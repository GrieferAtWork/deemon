/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C
#define GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* _Dee_MallococBufsize */
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/regex.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
/**/

#include "regroups.h"

#include <stddef.h> /* NULL, offsetof, size_t */

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

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
rg_serialize(ReGroups *__restrict self, DeeSerial *__restrict writer) {
	ReGroups *out;
	size_t sizeof_self = _Dee_MallococBufsize(offsetof(ReGroups, rg_groups),
	                                          self->rg_ngroups,
	                                          sizeof(struct DeeRegexMatch));
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, ReGroups);
	out->rg_ngroups = self->rg_ngroups;
	memcpyc(out->rg_groups, self->rg_groups, self->rg_ngroups, sizeof(ReGroups));
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}

#define rg_bool _DeeNone_reti1_1 /* Always non-empty (iow: return "1") */

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rg_size(ReGroups *__restrict self) {
	return self->rg_ngroups;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rg_getitem_index(ReGroups *__restrict self, size_t index) {
	if unlikely(index >= self->rg_ngroups)
		goto err_bounds;
	return DeeRegexMatch_AsRangeObject(&self->rg_groups[index]);
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rg_ngroups);
	return NULL;
}

PRIVATE struct type_seq rg_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rg_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rg_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rg_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
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

#define rsb_serialize rss_serialize
PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
rss_serialize(ReSubStrings *__restrict self,
              DeeSerial *__restrict writer) {
	ReSubStrings *out;
	size_t sizeof_self = _Dee_MallococBufsize(offsetof(ReSubStrings, rss_groups),
	                                          self->rss_ngroups,
	                                          sizeof(struct DeeRegexMatch));
#define ADDROF(field) (out_addr + offsetof(ReSubStrings, field))
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(rss_baseown), self->rss_baseown))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(rss_baseptr), self->rss_baseptr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, ReSubStrings);
	out->rss_ngroups = self->rss_ngroups;
	memcpyc(out->rss_groups, self->rss_groups, self->rss_ngroups, sizeof(ReGroups));
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
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

#define rss_size rg_size
#define rsb_nsi_getsize rg_size
#define rss_size rg_size
#define rsb_size rg_size

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rss_getitem_index(ReSubStrings *__restrict self, size_t index) {
	if unlikely(index >= self->rss_ngroups)
		goto err_bounds;
	return DeeRegexMatch_AsSubString(&self->rss_groups[index],
	                                 self->rss_baseown,
	                                 self->rss_baseptr);
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rss_ngroups);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rsb_getitem_index(ReSubBytes *__restrict self, size_t index) {
	if unlikely(index >= self->rss_ngroups)
		goto err_bounds;
	return DeeRegexMatch_AsSubBytes(&self->rss_groups[index],
	                                self->rss_baseown,
	                                self->rss_baseptr);
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rss_ngroups);
	return NULL;
}

PRIVATE struct type_seq rss_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rss_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rss_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rss_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_seq rsb_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rsb_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rsb_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rsb_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
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

PRIVATE struct type_member tpconst rg_class_members[] = {
#define rss_class_members rg_class_members
#define rsb_class_members rg_class_members
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ReGroups_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReGroups",
	/* .tp_doc      = */ DOC("getitem->?X2?T2?Dint?Dint?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &rg_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rg_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rg_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &rg_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rg_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ReSubStrings_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubStrings",
	/* .tp_doc      = */ DOC("getitem->?X2?Dstring?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &rss_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rss_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rss_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rss_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &rss_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rss_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rss_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ReSubBytes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSubBytes",
	/* .tp_doc      = */ DOC("getitem->?X2?DBytes?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &rsb_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rsb_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rsb_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rsb_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &rsb_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rsb_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rsb_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REGROUPS_C */
