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
#include <deemon/set.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/util/simple-hashset.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "default-api.h"

/**/
#include "unique-iterator.h"

DECL_BEGIN

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
		 * like "Sequence.repeatitem(item).operator iter()" */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE struct type_member tpconst uqiwk_members[] = {
	TYPE_MEMBER_FIELD("__key__", STRUCT_OBJECT, offsetof(UniqueIteratorWithKey, uiwk_key)),
#define uqi_members (uqiwk_members + 1)
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(UniqueIteratorWithKey, uiwk_iter)),
	TYPE_MEMBER_FIELD("__num_encountered__", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SIZE_T,
	                  offsetof(UniqueIteratorWithKey, uiwk_encountered.shswl_set.shs_size)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uqi_getseq(UniqueIterator *__restrict self) {
	DREF DeeObject *iter_seq, *result;
	iter_seq = DeeObject_GetAttr(self->ui_iter, (DeeObject *)&str_seq);
	if unlikely(!iter_seq)
		goto err;
	result = DeeSuper_New(&DeeSet_Type, iter_seq);
	Dee_Decref(iter_seq);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst uqi_getsets[] = {
	TYPE_GETTER(STR_seq, &uqi_getseq,
	            "->?DSet\n"
	            "Returns ${this.__iter__.seq as Set}"),
	/* TODO: "__encountered__->?DSet" (using a custom wrapper object) */
	TYPE_GETSET_END
};

INTERN DeeTypeObject UniqueIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueIterator",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
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
	/* .tp_getsets       = */ uqi_getsets,
	/* .tp_members       = */ uqi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



/************************************************************************/
/* WITH KEY                                                             */
/************************************************************************/
STATIC_ASSERT(offsetof(UniqueIteratorWithKey, uiwk_iter) == offsetof(UniqueIterator, ui_iter));
STATIC_ASSERT(offsetof(UniqueIteratorWithKey, uiwk_tp_next) == offsetof(UniqueIterator, ui_tp_next));
STATIC_ASSERT(offsetof(UniqueIteratorWithKey, uiwk_encountered) == offsetof(UniqueIterator, ui_encountered));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
uqiwk_copy(UniqueIteratorWithKey *__restrict self,
           UniqueIteratorWithKey *__restrict other) {
	int result;
	result = Dee_simple_hashset_with_lock_copy(&self->uiwk_encountered,
	                                           &other->uiwk_encountered);
	if likely(result == 0) {
		Dee_Incref(other->uiwk_iter);
		self->uiwk_iter    = other->uiwk_iter;
		self->uiwk_tp_next = other->uiwk_tp_next;
		Dee_Incref(other->uiwk_key);
		self->uiwk_key = other->uiwk_key;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
uqiwk_init(UniqueIteratorWithKey *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_UniqueIteratorWithKey",
	                  &self->uiwk_iter, &self->uiwk_key))
		goto err;
	self->uiwk_tp_next = Dee_TYPE(self->uiwk_iter)->tp_iter_next;
	if unlikely(!self->uiwk_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(self->uiwk_iter))) {
			err_unimplemented_operator(Dee_TYPE(self->uiwk_iter), OPERATOR_ITERNEXT);
			goto err;
		}
		self->uiwk_tp_next = Dee_TYPE(self->uiwk_iter)->tp_iter_next;
		ASSERT(self->uiwk_tp_next);
	}
	Dee_Incref(self->uiwk_iter);
	Dee_Incref(self->uiwk_key);
	Dee_simple_hashset_with_lock_init(&self->uiwk_encountered);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
uqiwk_fini(UniqueIteratorWithKey *__restrict self) {
	Dee_Decref(self->uiwk_iter);
	Dee_simple_hashset_with_lock_fini(&self->uiwk_encountered);
	Dee_Decref(self->uiwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
uqiwk_visit(UniqueIteratorWithKey *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->uiwk_iter);
	Dee_simple_hashset_with_lock_visit(&self->uiwk_encountered, proc, arg);
	Dee_Visit(self->uiwk_key);
}

#define uqiwk_clear uqi_clear
#define uqiwk_gc    uqi_gc

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
uqiwk_next(UniqueIteratorWithKey *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *keyed_result;
	for (;;) {
		int exists;
		result = (*self->uiwk_tp_next)(self->uiwk_iter);
		if (!ITER_ISOK(result))
			break;
		keyed_result = DeeObject_Call(self->uiwk_key, 1, &result);
		if unlikely(!keyed_result)
			goto err_r;
		exists = Dee_simple_hashset_with_lock_insert(&self->uiwk_encountered, keyed_result);
		Dee_Decref_unlikely(keyed_result);
		if (exists > 0)
			break;
		Dee_Decref(result);
		if unlikely(exists < 0)
			goto err;

		/* Must check for interrupts in case "iter" is something
		 * like "Sequence.repeatitem(item).unique(x -> x.lower())" */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF UniqueSetWithKey *DCALL
uqiwk_getseq(UniqueIteratorWithKey *__restrict self) {
	DREF DeeObject *iter_seq;
	DREF UniqueSetWithKey *result;
	iter_seq = DeeObject_GetAttr(self->uiwk_iter, (DeeObject *)&str_seq);
	if unlikely(!iter_seq)
		goto err;
	result = DeeObject_MALLOC(UniqueSetWithKey);
	if unlikely(!result)
		goto err_iter_seq;
	result->uswk_seq = iter_seq; /* Inherit reference */
	result->uswk_key = self->uiwk_key;
	Dee_Incref(self->uiwk_key);
	DeeObject_Init(result, &UniqueSetWithKey_Type);
	return result;
err_iter_seq:
	Dee_Decref(iter_seq);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst uqiwk_getsets[] = {
	TYPE_GETTER(STR_seq, &uqiwk_getseq,
	            "->?Ert:UniqueSetWithKey"),
	/* TODO: "__encountered__->?DSet" (using a custom wrapper object) */
	TYPE_GETSET_END
};

INTERN DeeTypeObject UniqueIteratorWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueIteratorWithKey",
	/* .tp_doc      = */ DOC("(objWithNext,key:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&uqiwk_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&uqiwk_init,
				TYPE_FIXED_ALLOCATOR_GC(UniqueIteratorWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uqiwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uqiwk_visit,
	/* .tp_gc            = */ &uqiwk_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uqiwk_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ uqiwk_getsets,
	/* .tp_members       = */ uqiwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



/************************************************************************/
/* UNIQUE SET W/ KEY                                                    */
/************************************************************************/
STATIC_ASSERT(offsetof(UniqueSetWithKey, uswk_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(UniqueSetWithKey, uswk_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(UniqueSetWithKey, uswk_key) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(UniqueSetWithKey, uswk_key) == offsetof(ProxyObject2, po_obj2));
#define uswk_copy  generic_proxy2_copy_alias12
#define uswk_deep  generic_proxy2_deepcopy
#define uswk_fini  generic_proxy2_fini
#define uswk_visit generic_proxy2_visit

STATIC_ASSERT(offsetof(UniqueSetWithKey, uswk_seq) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(UniqueSetWithKey, uswk_key) == offsetof(ProxyObject2, po_obj2));
#define uswk_init generic_proxy2_init

PRIVATE WUNUSED NONNULL((1)) DREF UniqueIteratorWithKey *DCALL
uswk_iter(UniqueSetWithKey *__restrict self) {
	DREF UniqueIteratorWithKey *result;
	result = DeeGCObject_MALLOC(UniqueIteratorWithKey);
	if unlikely(!result)
		goto err;
	result->uiwk_iter = DeeSeq_OperatorIter(self->uswk_seq);
	if unlikely(!result->uiwk_iter)
		goto err_r;
	result->uiwk_tp_next = Dee_TYPE(result->uiwk_iter)->tp_iter_next;
	if unlikely(!result->uiwk_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(result->uiwk_iter))) {
			err_unimplemented_operator(Dee_TYPE(result->uiwk_iter), OPERATOR_ITERNEXT);
			goto err_r_iter;
		}
		result->uiwk_tp_next = Dee_TYPE(result->uiwk_iter)->tp_iter_next;
		ASSERT(result->uiwk_tp_next);
	}
	result->uiwk_key = self->uswk_key;
	Dee_Incref(self->uswk_key);
	Dee_simple_hashset_with_lock_init(&result->uiwk_encountered);
	DeeObject_Init(result, &UniqueIteratorWithKey_Type);
	return (DREF UniqueIteratorWithKey *)DeeGC_Track((DREF DeeObject *)result);
err_r_iter:
	Dee_Decref(result->uiwk_iter);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq uswk_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&uswk_iter,
};

PRIVATE struct type_member tpconst uswk_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(UniqueSetWithKey, uswk_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__key__", STRUCT_OBJECT, offsetof(UniqueSetWithKey, uswk_key), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst uswk_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &UniqueIteratorWithKey_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject UniqueSetWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueSetWithKey",
	/* .tp_doc      = */ DOC("(objWithIter,key:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&uswk_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&uswk_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&uswk_init,
				TYPE_FIXED_ALLOCATOR(UniqueSetWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&uswk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&uswk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &uswk_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ uswk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ uswk_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_C */
