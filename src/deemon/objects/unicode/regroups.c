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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rg_nsi_getsize(ReGroups *__restrict self) {
	return self->rg_ngroups;
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
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
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

#define rsb_fini rss_fini
PRIVATE NONNULL((1)) void DCALL
rss_fini(ReSubStrings *__restrict self) {
	Dee_Decref(self->rss_baseown);
}

#define rsb_bool rss_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
rss_bool(ReSubStrings *__restrict self) {
	return self->rss_ngroups != 0;
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
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
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
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
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
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_nsi      = */ &rg_nsi
};

PRIVATE struct type_seq rss_seq = {
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_nsi      = */ &rss_nsi
};

PRIVATE struct type_seq rsb_seq = {
	/* .tp_iter     = */ NULL,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_nsi      = */ &rsb_nsi
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
	/* .tp_class_members = */ NULL
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
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rss_fini,
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
	/* .tp_class_members = */ NULL
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
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rsb_fini,
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
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C */
