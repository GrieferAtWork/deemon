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
#include <deemon/bytes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

/* Proxy sequence objects for `struct DeeRegexMatch'-arrays */

DECL_BEGIN

#undef sizeof_field
#define sizeof_field(T, m) sizeof(((T *)0)->m)

/* Assert assumptions made by reused code below. */
STATIC_ASSERT(sizeof(ReGroupsIterator) == sizeof(ReSubStringsIterator));
STATIC_ASSERT(offsetof(ReGroupsIterator, rgi_groups) == offsetof(ReSubStringsIterator, rssi_strings));
STATIC_ASSERT(sizeof_field(ReGroupsIterator, rgi_groups) == sizeof_field(ReSubStringsIterator, rssi_strings));
STATIC_ASSERT(offsetof(ReGroupsIterator, rgi_index) == offsetof(ReSubStringsIterator, rssi_index));
STATIC_ASSERT(sizeof_field(ReGroupsIterator, rgi_index) == sizeof_field(ReSubStringsIterator, rssi_index));
STATIC_ASSERT(offsetof(ReGroups, rg_ngroups) == offsetof(ReSubStrings, rss_ngroups));
STATIC_ASSERT(sizeof_field(ReGroups, rg_ngroups) == sizeof_field(ReSubStrings, rss_ngroups));


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

PRIVATE WUNUSED NONNULL((1)) int DCALL
rssiter_ctor(ReSubStringsIterator *__restrict self) {
	self->rssi_strings = (DREF ReSubStrings *)DeeObject_NewDefault(&ReSubStrings_Type);
	if unlikely(!self->rssi_strings)
		goto err;
	self->rssi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rsbiter_ctor(ReSubBytesIterator *__restrict self) {
	self->rssi_strings = (DREF ReSubBytes *)DeeObject_NewDefault(&ReSubBytes_Type);
	if unlikely(!self->rssi_strings)
		goto err;
	self->rssi_index = 0;
	return 0;
err:
	return -1;
}

#define rssiter_copy rgiter_copy
#define rsbiter_copy rgiter_copy
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
rssiter_init(ReSubStringsIterator *__restrict self,
             size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ReSubStringsIterator", &self->rssi_strings))
		goto err;
	if (DeeObject_AssertTypeExact(self->rssi_strings, &ReSubStrings_Type))
		goto err;
	Dee_Incref(self->rssi_strings);
	self->rssi_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rsbiter_init(ReSubBytesIterator *__restrict self,
             size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_ReSubBytesIterator", &self->rssi_strings))
		goto err;
	if (DeeObject_AssertTypeExact(self->rssi_strings, &ReSubBytes_Type))
		goto err;
	Dee_Incref(self->rssi_strings);
	self->rssi_index = 0;
	return 0;
err:
	return -1;
}

#define rssiter_fini rgiter_fini
#define rsbiter_fini rgiter_fini
PRIVATE NONNULL((1)) void DCALL
rgiter_fini(ReGroupsIterator *__restrict self) {
	Dee_Decref(self->rgi_groups);
}

#define rssiter_visit rgiter_visit
#define rsbiter_visit rgiter_visit
PRIVATE NONNULL((1, 2)) void DCALL
rgiter_visit(ReGroupsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rgi_groups);
}

#define rssiter_bool rgiter_bool
#define rsbiter_bool rgiter_bool
PRIVATE NONNULL((1)) int DCALL
rgiter_bool(ReGroupsIterator *__restrict self) {
	return ReGroupsIterator_GetIndex(self) < self->rgi_groups->rg_ngroups;
}

#define DEFINE_RGITER_COMPARE(name, op)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL     \
	name(ReGroupsIterator *self, ReGroupsIterator *other) {   \
		size_t x, y;                                          \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                         \
		x = ReGroupsIterator_GetIndex(self);                  \
		y = ReGroupsIterator_GetIndex(other);                 \
		return_bool(x op y);                                  \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_RGITER_COMPARE(rgiter_eq, ==)
DEFINE_RGITER_COMPARE(rgiter_ne, !=)
DEFINE_RGITER_COMPARE(rgiter_lo, <)
DEFINE_RGITER_COMPARE(rgiter_le, <=)
DEFINE_RGITER_COMPARE(rgiter_gr, >)
DEFINE_RGITER_COMPARE(rgiter_ge, >=)
#undef DEFINE_RGITER_COMPARE

#define rssiter_cmp rgiter_cmp
#define rsbiter_cmp rgiter_cmp
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
again:
	index = ReGroupsIterator_GetIndex(self);
	if unlikely(index >= self->rgi_groups->rg_ngroups)
		return ITER_DONE;
	result = DeeRegexMatch_AsRangeObject(&self->rgi_groups->rg_groups[index]);
	if likely(result) {
		if unlikely(!atomic_cmpxch_weak_or_write(&self->rgi_index, index, index + 1))
			goto again;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rssiter_next(ReSubStringsIterator *__restrict self) {
	DREF DeeObject *result;
	size_t index;
again:
	index = ReSubStringsIterator_GetIndex(self);
	if unlikely(index >= self->rssi_strings->rss_ngroups)
		return ITER_DONE;
	result = DeeRegexMatch_AsSubString(&self->rssi_strings->rss_groups[index],
	                                   self->rssi_strings->rss_baseown,
	                                   self->rssi_strings->rss_baseptr);
	if likely(result) {
		if unlikely(!atomic_cmpxch_weak_or_write(&self->rssi_index, index, index + 1))
			goto again;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rsbiter_next(ReSubBytesIterator *__restrict self) {
	DREF DeeObject *result;
	size_t index;
again:
	index = ReSubBytesIterator_GetIndex(self);
	if unlikely(index >= self->rssi_strings->rss_ngroups)
		return ITER_DONE;
	result = DeeRegexMatch_AsSubBytes(&self->rssi_strings->rss_groups[index],
	                                   self->rssi_strings->rss_baseown,
	                                   self->rssi_strings->rss_baseptr);
	if likely(result) {
		if unlikely(!atomic_cmpxch_weak_or_write(&self->rssi_index, index, index + 1))
			goto again;
	}
	return result;
}

PRIVATE struct type_member tpconst rgiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(ReGroupsIterator, rgi_groups), "->?Ert:ReGroups"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReGroupsIterator, rgi_index)),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define rssiter_members rgiter_members
#define rsbiter_members rgiter_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst rssiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(ReSubStringsIterator, rssi_strings), "->?Ert:ReSubStrings"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSubStringsIterator, rssi_index)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rsbiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(ReSubStringsIterator, rssi_strings), "->?Ert:ReSubBytes"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSubStringsIterator, rssi_index)),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

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

INTERN DeeTypeObject ReSubStringsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubStringsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rssiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rssiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rssiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rssiter_init,
				TYPE_FIXED_ALLOCATOR(ReGroupsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rssiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rssiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rssiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rssiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rssiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rssiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject ReSubBytesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubBytesIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rsbiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rsbiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rsbiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rsbiter_init,
				TYPE_FIXED_ALLOCATOR(ReGroupsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rsbiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rsbiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rsbiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rsbiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rsbiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rsbiter_members,
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
	return DeeRegexMatch_AsRangeObject(&self->rg_groups[index]);
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

PRIVATE WUNUSED DREF ReSubStrings *DCALL rss_ctor(void) {
	DREF ReSubStrings *result = ReSubStrings_Malloc(0);
	if likely(result)
		ReSubStrings_Init(result, Dee_EmptyString, NULL, 0);
	return result;
}

PRIVATE WUNUSED DREF ReSubBytes *DCALL rsb_ctor(void) {
	DREF ReSubBytes *result = ReSubBytes_Malloc(0);
	if likely(result)
		ReSubBytes_Init(result, Dee_EmptyBytes, NULL, 0);
	return result;
}

#define rsb_bool rss_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
rss_bool(ReSubStrings *__restrict self) {
	return self->rss_ngroups != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSubStringsIterator *DCALL
rss_iter(ReSubStrings *__restrict self) {
	DREF ReSubStringsIterator *result;
	result = DeeObject_MALLOC(ReSubStringsIterator);
	if likely(result) {
		result->rssi_index   = 0;
		result->rssi_strings = self;
		Dee_Incref(self);
		DeeObject_Init(result, &ReSubStringsIterator_Type);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSubBytesIterator *DCALL
rsb_iter(ReSubBytes *__restrict self) {
	DREF ReSubBytesIterator *result;
	result = DeeObject_MALLOC(ReSubBytesIterator);
	if likely(result) {
		result->rssi_index   = 0;
		result->rssi_strings = self;
		Dee_Incref(self);
		DeeObject_Init(result, &ReSubBytesIterator_Type);
	}
	return result;
}

#define rss_nsi_getsize rg_nsi_getsize
#define rsb_nsi_getsize rg_nsi_getsize
#define rss_size rg_size
#define rsb_size rg_size

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rss_nsi_getitem_fast(ReSubStrings *__restrict self, size_t index) {
	ASSERT(index < self->rss_ngroups);
	return DeeRegexMatch_AsSubString(&self->rss_groups[index],
	                                 self->rss_baseown,
	                                 self->rss_baseptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rsb_nsi_getitem_fast(ReSubBytes *__restrict self, size_t index) {
	ASSERT(index < self->rss_ngroups);
	return DeeRegexMatch_AsSubBytes(&self->rss_groups[index],
	                                self->rss_baseown,
	                                self->rss_baseptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rss_nsi_getitem(ReSubStrings *__restrict self, size_t index) {
	if unlikely(index >= self->rss_ngroups)
		goto err_bounds;
	return rss_nsi_getitem_fast(self, index);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, self->rss_ngroups);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rsb_nsi_getitem(ReSubBytes *__restrict self, size_t index) {
	if unlikely(index >= self->rss_ngroups)
		goto err_bounds;
	return rsb_nsi_getitem_fast(self, index);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, self->rss_ngroups);
	return NULL;
}

PRIVATE struct type_nsi tpconst rss_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&rss_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&rss_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&rss_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&rss_nsi_getitem_fast,
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

PRIVATE struct type_nsi tpconst rsb_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&rsb_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&rsb_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&rsb_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&rsb_nsi_getitem_fast,
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

PRIVATE struct type_seq rss_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rss_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rss_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &rss_nsi
};

PRIVATE struct type_seq rsb_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rsb_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rsb_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &rsb_nsi
};

PRIVATE struct type_member tpconst rg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReGroupsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rss_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReSubStringsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rsb_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReSubBytesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rss_members[] = {
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(ReSubStrings, rss_baseown), "->?Dstring"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define rsb_members rss_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst rsb_members[] = {
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(ReSubStrings, rss_baseown), "->?DBytes"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

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

INTERN DeeTypeObject ReSubStrings_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubStrings",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rss_ctor,
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
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rss_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rss_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rss_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rss_class_members
};

INTERN DeeTypeObject ReSubBytes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubBytes",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rsb_ctor,
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
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rsb_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rsb_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rsb_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rsb_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C */
