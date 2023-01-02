/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C
#define GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C 1

#include "regroups.h"

#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "../../runtime/runtime_error.h"

/* Proxy sequence objects for `struct DeeRegexMatch'-arrays */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
rgiter_ctor(ReGroupsIterator *__restrict self) {
	self->rgi_groups = (DREF ReGroups *)DeeObject_NewDefault(&ReGroups_Type);
	if unlikely(!self->rgi_groups)
		goto err;
	self->rgi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rgiter_copy(ReGroupsIterator *__restrict self,
            ReGroupsIterator *__restrict other) {
	self->rgi_index  = ReGroupsIterator_GetIndex(other);
	self->rgi_groups = other->rgi_groups;
	Dee_Incref(self->rgi_groups);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rgiter_init(ReGroupsIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ReGroupsIterator", &self->rgi_groups))
		goto err;
	if (DeeObject_AssertTypeExact(self->rgi_groups, &ReGroups_Type))
		goto err;
	Dee_Incref(self->rgi_groups);
	self->rgi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
rgiter_fini(ReGroupsIterator *__restrict self) {
	Dee_Decref(self->rgi_groups);
}

PRIVATE NONNULL((1, 2)) void DCALL
rgiter_visit(ReGroupsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rgi_groups);
}

PRIVATE NONNULL((1)) int DCALL
rgiter_bool(ReGroupsIterator *__restrict self) {
	return ReGroupsIterator_GetIndex(self) < self->rgi_groups->rg_ngroups;
}

#define DEFINE_RGITER_COMPARE(name, op)                               \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL             \
	name(ReGroupsIterator *self, ReGroupsIterator *other) {           \
		size_t x, y;                                                  \
		if (DeeObject_AssertTypeExact(other, &ReGroupsIterator_Type)) \
			goto err;                                                 \
		x = ReGroupsIterator_GetIndex(self);                          \
		y = ReGroupsIterator_GetIndex(other);                         \
		return_bool(x op y);                                          \
	err:                                                              \
		return NULL;                                                  \
	}
DEFINE_RGITER_COMPARE(rgiter_eq, ==)
DEFINE_RGITER_COMPARE(rgiter_ne, !=)
DEFINE_RGITER_COMPARE(rgiter_lo, <)
DEFINE_RGITER_COMPARE(rgiter_le, <=)
DEFINE_RGITER_COMPARE(rgiter_gr, >)
DEFINE_RGITER_COMPARE(rgiter_ge, >=)
#undef DEFINE_RGITER_COMPARE

PRIVATE struct type_cmp rgiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rgiter_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rgiter_next(ReGroupsIterator *__restrict self) {
	DREF DeeObject *result;
	size_t index;
#ifndef CONFIG_NO_THREADS
again:
#endif /* CONFIG_NO_THREADS */
	index = ReGroupsIterator_GetIndex(self);
	if unlikely(index >= self->rgi_groups->rg_ngroups)
		return ITER_DONE;
	result = DeeRegexMatch_ToObject(&self->rgi_groups->rg_groups[index]);
	if likely(result) {
#ifdef CONFIG_NO_THREADS
		self->rgi_index = index + 1;
#else  /* CONFIG_NO_THREADS */
		if unlikely (!ATOMIC_CMPXCH(self->rgi_index, index, index + 1))
			goto again;
#endif /* !CONFIG_NO_THREADS */
	}
	return result;
}

PRIVATE struct type_member tpconst rgiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(ReGroupsIterator, rgi_groups), "->?Ert:ReGroups"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReGroupsIterator, rgi_index)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ReGroupsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReGroupsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rgiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rgiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rgiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rgiter_init,
				TYPE_FIXED_ALLOCATOR(ReGroupsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rgiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rgiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rgiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rgiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rgiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rgiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED DREF ReGroups *DCALL rg_ctor(void) {
	DREF ReGroups *result = ReGroups_Malloc(1);
	if likely(result) {
		result->rg_groups[0].rm_so = 0;
		result->rg_groups[0].rm_eo = 0;
		ReGroups_Init(result, 1);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rg_bool(ReGroups *__restrict UNUSED(self)) {
	return 1; /* Always non-empty */
}

PRIVATE WUNUSED NONNULL((1)) DREF ReGroupsIterator *DCALL
rg_iter(ReGroups *__restrict self) {
	DREF ReGroupsIterator *result;
	result = DeeObject_MALLOC(ReGroupsIterator);
	if likely(result) {
		result->rgi_index  = 0;
		result->rgi_groups = self;
		Dee_Incref(self);
		DeeObject_Init(result, &ReGroupsIterator_Type);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rg_nsi_getsize(ReGroups *__restrict self) {
	return self->rg_ngroups;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rg_size(ReGroups *__restrict self) {
	return DeeInt_NewSize(self->rg_ngroups);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rg_nsi_getitem_fast(ReGroups *__restrict self, size_t index) {
	ASSERT(index < self->rg_ngroups);
	return DeeRegexMatch_ToObject(&self->rg_groups[index]);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rg_nsi_getitem(ReGroups *__restrict self, size_t index) {
	if unlikely(index >= self->rg_ngroups)
		goto err_bounds;
	return rg_nsi_getitem_fast(self, index);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, self->rg_ngroups);
	return NULL;
}

PRIVATE struct type_nsi tpconst rg_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&rg_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&rg_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&rg_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&rg_nsi_getitem_fast,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq rg_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rg_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rg_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &rg_nsi
};

PRIVATE struct type_member tpconst rg_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ReGroupsIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ReGroups_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReGroups",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rg_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rg_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rg_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C */
