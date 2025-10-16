/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>

#include "../../runtime/runtime_error.h"
#include "../generic-proxy.h"
#include "string_functions.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

DECL_BEGIN

typedef struct {
	/* A proxy object for viewing the characters of a string as an array of unsigned
	 * integers representing the unicode character codes for each character. */
	PROXY_OBJECT_HEAD_EX(DeeStringObject, so_str)   /* [1..1][const] The string who's character ordinals are being viewed. */
	unsigned int                          so_width; /* [const][== DeeString_WIDTH(so_str)] The string's character width. */
	union dcharptr_const                  so_ptr;   /* [const][== DeeString_WSTR(so_str)] The effective character array. */
} StringOrdinals;

INTDEF DeeTypeObject StringOrdinals_Type;

STATIC_ASSERT(offsetof(StringOrdinals, so_str) == offsetof(ProxyObject, po_obj));
#define stringordinals_fini  generic_proxy__fini
#define stringordinals_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_ctor(StringOrdinals *__restrict self) {
	self->so_str     = (DREF DeeStringObject *)DeeString_NewEmpty();
	self->so_width   = STRING_WIDTH_1BYTE;
	self->so_ptr.ptr = DeeString_STR(self->so_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_init(StringOrdinals *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_StringOrdinals", &self->so_str);
	if (DeeObject_AssertTypeExact(self->so_str, &DeeString_Type))
		goto err;
	self->so_width   = DeeString_WIDTH(self->so_str);
	self->so_ptr.ptr = DeeString_WSTR(self->so_str);
	Dee_Incref(self->so_str);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_bool(StringOrdinals *__restrict self) {
	return !DeeString_IsEmpty(self->so_str);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
stringordinals_size(StringOrdinals *__restrict self) {
	return WSTR_LENGTH(self->so_ptr.ptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stringordinals_getitem_index(StringOrdinals *__restrict self, size_t index) {
	if unlikely(index >= WSTR_LENGTH(self->so_ptr.ptr))
		goto err_oob;
	SWITCH_SIZEOF_WIDTH(self->so_width) {
	CASE_WIDTH_1BYTE:
		return DeeInt_NewUInt8(self->so_ptr.cp8[index]);
	CASE_WIDTH_2BYTE:
		return DeeInt_NewUInt16(self->so_ptr.cp16[index]);
	CASE_WIDTH_4BYTE:
		return DeeInt_NewUInt32(self->so_ptr.cp32[index]);
	}
	__builtin_unreachable();
err_oob:
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index,
	                          WSTR_LENGTH(self->so_ptr.ptr));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
stringordinals_contains(StringOrdinals *self,
                        DeeObject *ord_ob) {
	uint32_t ord;
	if (DeeObject_AsUInt32(ord_ob, &ord))
		goto err;
	SWITCH_SIZEOF_WIDTH(self->so_width) {

	CASE_WIDTH_1BYTE:
		if (ord > 0xff)
			break;
		return_bool(memchrb(self->so_ptr.cp8, (uint8_t)ord, WSTR_LENGTH(self->so_ptr.cp8)) != NULL);

	CASE_WIDTH_2BYTE:
		if (ord > 0xffff)
			break;
		return_bool(memchrw(self->so_ptr.cp16, (uint16_t)ord, WSTR_LENGTH(self->so_ptr.cp16)) != NULL);

	CASE_WIDTH_4BYTE:
		return_bool(memchrl(self->so_ptr.cp32, ord, WSTR_LENGTH(self->so_ptr.cp32)) != NULL);
	}
	return_false;
err:
	return NULL;
}


PRIVATE struct type_seq stringordinals_seq = {
	/* .tp_iter               = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&stringordinals_contains,
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&stringordinals_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&stringordinals_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&stringordinals_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
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

PRIVATE struct type_member tpconst stringordinals_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringOrdinals, so_str), "->?Dstring"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject StringOrdinals_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringOrdinals",
	/* .tp_doc      = */ DOC("(s:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&stringordinals_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&stringordinals_init,
				TYPE_FIXED_ALLOCATOR(StringOrdinals)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stringordinals_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&stringordinals_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&stringordinals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__DC202CECA797EF15),
	/* .tp_seq           = */ &stringordinals_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ stringordinals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Ordinals(DeeObject *__restrict self) {
	DREF StringOrdinals *result;
	result = DeeObject_MALLOC(StringOrdinals);
	if unlikely(!result)
		goto done;
	result->so_str     = (DREF DeeStringObject *)self;
	result->so_width   = DeeString_WIDTH(self);
	result->so_ptr.ptr = DeeString_WSTR(self);
	Dee_Incref(self);
	DeeObject_Init(result, &StringOrdinals_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL */
