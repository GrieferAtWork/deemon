/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/seq.h>
#include <deemon/string.h>

#include <hybrid/atomic.h>

#include "../../runtime/strings.h"

DECL_BEGIN

typedef struct {
	/* A proxy object for viewing the characters of a string as an array of unsigned
	 * integers representing the unicode character codes for each character. */
	OBJECT_HEAD
	DREF DeeStringObject *soi_str;   /* [1..1][const] The string who's character ordinals are being viewed. */
	unsigned int          soi_width; /* [const][== DeeString_WIDTH(so_str)] The string's character width. */
	DWEAK union dcharptr  soi_ptr;   /* Pointer to the next character. */
	union dcharptr        soi_end;   /* [const][== DeeString_WEND(so_str)] The end of the string. */
} StringOrdinalsIterator;
#ifdef CONFIG_NO_THREADS
#define READ_PTR(x)               (x)->soi_ptr.ptr
#else /* CONFIG_NO_THREADS */
#define READ_PTR(x)   ATOMIC_READ((x)->soi_ptr.ptr)
#endif /* !CONFIG_NO_THREADS */

typedef struct {
	/* A proxy object for viewing the characters of a string as an array of unsigned
	 * integers representing the unicode character codes for each character. */
	OBJECT_HEAD
	DREF DeeStringObject *so_str;   /* [1..1][const] The string who's character ordinals are being viewed. */
	unsigned int          so_width; /* [const][== DeeString_WIDTH(so_str)] The string's character width. */
	union dcharptr        so_ptr;   /* [const][== DeeString_WSTR(so_str)] The effective character array. */
} StringOrdinals;

INTDEF DeeTypeObject StringOrdinals_Type;
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



PRIVATE NONNULL((1)) void DCALL stringordinals_fini(StringOrdinals *__restrict self);
PRIVATE NONNULL((1, 2)) void DCALL stringordinals_visit(StringOrdinals *__restrict self, dvisit_t proc, void *arg);
STATIC_ASSERT(offsetof(StringOrdinals, so_str) == offsetof(StringOrdinalsIterator, soi_str));
#define stringordinalsiter_fini  stringordinals_fini
#define stringordinalsiter_visit stringordinals_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinalsiter_ctor(StringOrdinalsIterator *__restrict self) {
	self->soi_str     = (DREF DeeStringObject *)Dee_EmptyString;
	self->soi_width   = STRING_WIDTH_1BYTE;
	self->soi_ptr.ptr = DeeString_STR(Dee_EmptyString);
	self->soi_end.ptr = DeeString_STR(Dee_EmptyString);
	Dee_Incref(Dee_EmptyString);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
stringordinalsiter_copy(StringOrdinalsIterator *__restrict self,
                        StringOrdinalsIterator *__restrict other) {
	self->soi_str     = other->soi_str;
	self->soi_width   = other->soi_width;
	self->soi_ptr.ptr = READ_PTR(other);
	self->soi_end.ptr = other->soi_end.ptr;
	Dee_Incref(self->soi_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinalsiter_init(StringOrdinalsIterator *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	StringOrdinals *ords;
	if (DeeArg_Unpack(argc, argv, "o:_StringOrdinalsIterator", &ords))
		goto err;
	if (DeeObject_AssertTypeExact(ords, &StringOrdinals_Type))
		goto err;
	self->soi_str     = ords->so_str;
	self->soi_width   = ords->so_width;
	self->soi_ptr.ptr = ords->so_ptr.ptr;
	self->soi_end.ptr = DeeString_WEND(ords->so_str);
	Dee_Incref(self->soi_str);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinalsiter_bool(StringOrdinalsIterator *__restrict self) {
	return READ_PTR(self) < self->soi_end.ptr;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stringordinalsiter_next(StringOrdinalsIterator *__restrict self) {
	union dcharptr pchar;
#ifdef CONFIG_NO_THREADS
	pchar.ptr = READ_PTR(self);
	if (pchar.ptr >= self->soi_end.ptr)
		return ITER_DONE;
	self->soi_ptr.cp8 = STRING_SIZEOF_WIDTH(self->soi_width);
#else /* CONFIG_NO_THREADS */
	do {
		pchar.ptr = READ_PTR(self);
		if (pchar.ptr >= self->soi_end.ptr)
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->soi_ptr.cp8, pchar.cp8, pchar.cp8 + STRING_SIZEOF_WIDTH(self->soi_width)));
#endif /* !CONFIG_NO_THREADS */
	SWITCH_SIZEOF_WIDTH(self->soi_width) {

	CASE_WIDTH_1BYTE:
		return DeeInt_NewU8(*pchar.cp8);

	CASE_WIDTH_2BYTE:
		return DeeInt_NewU16(*pchar.cp16);

	CASE_WIDTH_4BYTE:
		return DeeInt_NewU32(*pchar.cp32);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF StringOrdinals *DCALL
stringordinalsiter_seq(StringOrdinalsIterator *__restrict self) {
	return (DREF StringOrdinals *)DeeString_Ordinals((DeeObject *)self->soi_str);
}

PRIVATE struct type_getset tpconst stringordinalsiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringordinalsiter_seq,
	  NULL,
	  NULL,
	  DOC("->?Ert:StringOrdinals") },
	{ NULL }
};


INTERN DeeTypeObject StringOrdinalsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringOrdinalsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&stringordinalsiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&stringordinalsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&stringordinalsiter_init,
				TYPE_FIXED_ALLOCATOR(StringOrdinalsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stringordinalsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&stringordinalsiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&stringordinalsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringordinalsiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ stringordinalsiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE NONNULL((1)) void DCALL
stringordinals_fini(StringOrdinals *__restrict self) {
	Dee_Decref(self->so_str);
}

PRIVATE NONNULL((1, 2)) void DCALL
stringordinals_visit(StringOrdinals *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->so_str);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringordinals_bool(StringOrdinals *__restrict self) {
	return !DeeString_IsEmpty(self->so_str);
}

PRIVATE WUNUSED NONNULL((1)) DREF StringOrdinalsIterator *DCALL
stringordinals_iter(StringOrdinals *__restrict self) {
	DREF StringOrdinalsIterator *result;
	result = DeeObject_MALLOC(StringOrdinalsIterator);
	if unlikely(!result)
		goto done;
	result->soi_str     = self->so_str;
	result->soi_width   = self->so_width;
	result->soi_ptr     = self->so_ptr;
	result->soi_end.ptr = DeeString_WEND(self->so_str);
	Dee_Incref(self->so_str);
	DeeObject_Init(result, &StringOrdinalsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stringordinals_size(StringOrdinals *__restrict self) {
	return DeeInt_NewSize(WSTR_LENGTH(self->so_ptr.ptr));
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
stringordinals_get(StringOrdinals *self,
                   DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	if (index >= WSTR_LENGTH(self->so_ptr.ptr)) {
		err_index_out_of_bounds((DeeObject *)self, index,
		                        WSTR_LENGTH(self->so_ptr.ptr));
		goto err;
	}
	return DeeInt_NewU32(STRING_WIDTH_GETCHAR(self->so_width,
	                                          self->so_ptr.ptr,
	                                          index));
err:
	return NULL;
}


PRIVATE struct type_seq stringordinals_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringordinals_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringordinals_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&stringordinals_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&stringordinals_get,
};

PRIVATE struct type_member tpconst stringordinals_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringOrdinals, so_str), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst stringordinals_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &StringOrdinalsIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject StringOrdinals_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringOrdinals",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ stringordinals_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ stringordinals_class_members
};


#undef READ_PTR

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ORDINALS_C_INL */
