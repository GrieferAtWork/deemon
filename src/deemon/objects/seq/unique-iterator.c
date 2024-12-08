/* Copyright (c) 2018-2024 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C
#define GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/util/simple-hashset.h>

/**/
#include "../../runtime/runtime_error.h"

/**/
#include "unique-iterator.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
uqi_ctor(UniqueIterator *__restrict self) {
	Dee_Incref(Dee_EmptyIterator);
	self->ui_iter = Dee_EmptyIterator;
	self->ui_tp_next = Dee_TYPE(Dee_EmptyIterator)->tp_iter_next;
	ASSERT(self->ui_tp_next);
	Dee_simple_hashset_with_lock_init(&self->ui_encountered);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uqi_copy(UniqueIterator *__restrict self,
         UniqueIterator *__restrict other) {
	int result;
	result = Dee_simple_hashset_with_lock_copy(&self->ui_encountered,
	                                           &other->ui_encountered);
	if likely(result == 0) {
		Dee_Incref(other->ui_iter);
		self->ui_iter    = other->ui_iter;
		self->ui_tp_next = other->ui_tp_next;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uqi_init(UniqueIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_UniqueIterator", &self->ui_iter))
		goto err;
	self->ui_tp_next = Dee_TYPE(self->ui_iter)->tp_iter_next;
	if unlikely(!self->ui_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(self->ui_iter))) {
			err_unimplemented_operator(Dee_TYPE(self->ui_iter), OPERATOR_ITERNEXT);
			goto err;
		}
		self->ui_tp_next = Dee_TYPE(self->ui_iter)->tp_iter_next;
		ASSERT(self->ui_tp_next);
	}
	Dee_Incref(self->ui_iter);
	Dee_simple_hashset_with_lock_init(&self->ui_encountered);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
uqi_fini(UniqueIterator *__restrict self) {
	Dee_Decref(self->ui_iter);
	Dee_simple_hashset_with_lock_fini(&self->ui_encountered);
}

PRIVATE NONNULL((1, 2)) void DCALL
uqi_visit(UniqueIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ui_iter);
	Dee_simple_hashset_with_lock_visit(&self->ui_encountered, proc, arg);
}

PRIVATE NONNULL((1)) void DCALL
uqi_clear(UniqueIterator *__restrict self) {
	Dee_simple_hashset_with_lock_clear(&self->ui_encountered);
}

PRIVATE struct type_gc uqi_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&uqi_clear
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uqi_next(UniqueIterator *__restrict self) {
	DREF DeeObject *result;
	for (;;) {
		int exists;
		result = (*self->ui_tp_next)(self->ui_iter);
		if (!ITER_ISOK(result))
			break;
		exists = Dee_simple_hashset_with_lock_insert(&self->ui_encountered, result);
		if (exists > 0)
			break;
		Dee_Decref(result);
		if unlikely(exists < 0)
			goto err;

		/* Must check for interrupts in case "iter" is something
		 * like "Sequence.repeatitem(x).operator iter()" */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE struct type_member tpconst uqi_members[] = {
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(UniqueIterator, ui_iter)),
	TYPE_MEMBER_FIELD("__num_encountered__", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SIZE_T,
	                  offsetof(UniqueIterator, ui_encountered.shswl_set.shs_size)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject UniqueIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&uqi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&uqi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&uqi_init,
				TYPE_FIXED_ALLOCATOR_GC(UniqueIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uqi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uqi_visit,
	/* .tp_gc            = */ &uqi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uqi_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: "__encountered__->?DSet" (using a custom wrapper object) */
	/* .tp_members       = */ uqi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C */
