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

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/arg.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include "../../runtime/strings.h"
#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	/* A proxy object for viewing the characters of a string as an array of unsigned
	 * integers representing the unicode character codes for each character. */
	PROXY_OBJECT_HEAD_EX(DeeStringObject, so_str)   /* [1..1][const] The string who's character ordinals are being viewed. */
	unsigned int                          so_width; /* [const][== DeeString_WIDTH(so_str)] The string's character width. */
	union dcharptr                        so_ptr;   /* [const][== DeeString_WSTR(so_str)] The effective character array. */
} StringOrdinals;

INTDEF DeeTypeObject StringOrdinals_Type;

STATIC_ASSERT(offsetof(StringOrdinals, so_str) == offsetof(ProxyObject, po_obj));
#define stringordinals_fini  generic_proxy_fini
#define stringordinals_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_ctor(StringOrdinals *__restrict self) {
	self->so_str     = (DREF DeeStringObject *)Dee_EmptyString;
	self->so_width   = STRING_WIDTH_1BYTE;
	self->so_ptr.ptr = DeeString_STR(Dee_EmptyString);
	Dee_Incref(Dee_EmptyString);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_init(StringOrdinals *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_StringOrdinals", &self->so_str))
		goto err;
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
	err_index_out_of_bounds((DeeObject *)self, index,
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
	/* .tp_iter               = */ NULL,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&stringordinals_contains,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&stringordinals_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&stringordinals_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&stringordinals_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_trygetitem         = */ NULL,
	/* .tp_trygetitem_index   = */ NULL,
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
				/* .tp_ctor      = */ (dfunptr_t)&stringordinals_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&stringordinals_init,
				TYPE_FIXED_ALLOCATOR(StringOrdinals)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stringordinals_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&stringordinals_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&stringordinals_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &stringordinals_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ stringordinals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
