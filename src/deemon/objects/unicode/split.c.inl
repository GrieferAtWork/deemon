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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack* */
#include <deemon/computed-operators.h>
#include <deemon/error.h>              /* DeeError_Throwf, DeeError_ValueError */
#include <deemon/object.h>             /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_AssertTypeExact, DeeObject_NewDefault, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_DecrefNokill, Dee_Incref, Dee_TYPE, Dee_hash_t, Dee_return_compare, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>             /* CASE_WIDTH_nBYTE, DeeString*, DeeUni_IsLF, Dee_STRING_DIV_SIZEOF_WIDTH, Dee_charptr_const, STRING_WIDTH_COMMON, SWITCH_SIZEOF_WIDTH, WSTR_LENGTH */
#include <deemon/super.h>              /* DeeSuper_New */
#include <deemon/system-features.h>    /* memmeml, memmemw */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, STRUCT_*, TF_NONE, TF_NONLOOPING, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak, atomic_read */
#include <deemon/util/hash.h>          /* Dee_HashPointer */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

INTDEF DeeTypeObject StringSplit_Type;
INTDEF DeeTypeObject StringSplitIterator_Type;
INTDEF DeeTypeObject StringCaseSplit_Type;
INTDEF DeeTypeObject StringCaseSplitIterator_Type;
INTDEF DeeTypeObject StringLineSplit_Type;
INTDEF DeeTypeObject StringLineSplitIterator_Type;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeStringObject, s_str, /* [1..1][const] The string that is being split. */
	                      DeeStringObject, s_sep) /* [1..1][const][!DeeString_IsEmpty] The string to search for. */
} StringSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(StringSplit, s_split) /* [1..1][const] The split descriptor object. */
	union Dee_charptr_const           s_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                                            *                (points into the s_enc-specific string of `s_split->s_str')
	                                            *                When the iterator is exhausted, this pointer is set to `NULL'. */
	union Dee_charptr_const           s_start; /* [1..1][const] The starting address of the width string of `s_split->s_str'. */
	union Dee_charptr_const           s_end;   /* [1..1][const] The end address of the width string of `s_split->s_str'. */
	union Dee_charptr_const           s_sep;   /* [1..1][const] The starting address of the `s_enc'-encoded string of `s_split->s_sep'. */
	size_t                            s_sepsz; /* [1..1][const][== WSTR_LENGTH(s_sep)] The length of separator string. */
	int                               s_width; /* [const] The width of `s_split->s_str' */
} StringSplitIterator;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
splititer_next(StringSplitIterator *__restrict self) {
	union Dee_charptr_const result_start, result_end, next_ptr;
	size_t result_len;
	do {
		result_start.ptr = atomic_read(&self->s_next.ptr);
		if (!result_start.ptr)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->s_width) {

		CASE_WIDTH_1BYTE:
			result_end.cp8 = memmemb(result_start.cp8,
			                         self->s_end.cp8 - result_start.cp8,
			                         self->s_sep.cp8, self->s_sepsz);
			if (!result_end.cp8) {
				result_end.cp8 = self->s_end.cp8;
				next_ptr.cp8   = NULL;
			} else {
				next_ptr.cp8 = result_end.cp8 + self->s_sepsz;
			}
			result_len = (size_t)(result_end.cp8 -
			                      result_start.cp8);
			break;

		CASE_WIDTH_2BYTE:
			result_end.cp16 = memmemw(result_start.cp16,
			                          self->s_end.cp16 - result_start.cp16,
			                          self->s_sep.cp16, self->s_sepsz);
			if (!result_end.cp16) {
				result_end.cp16 = self->s_end.cp16;
				next_ptr.cp16   = NULL;
			} else {
				next_ptr.cp16 = result_end.cp16 + self->s_sepsz;
			}
			result_len = (size_t)(result_end.cp16 -
			                      result_start.cp16);
			break;

		CASE_WIDTH_4BYTE:
			result_end.cp32 = memmeml(result_start.cp32,
			                          self->s_end.cp32 - result_start.cp32,
			                          self->s_sep.cp32, self->s_sepsz);
			if (!result_end.cp32) {
				result_end.cp32 = self->s_end.cp32;
				next_ptr.cp32   = NULL;
			} else {
				next_ptr.cp32 = result_end.cp32 + self->s_sepsz;
			}
			result_len = (size_t)(result_end.cp32 -
			                      result_start.cp32);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->s_next.ptr, result_start.ptr, next_ptr.ptr));

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start.ptr,
	                              result_len,
	                              self->s_width);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
casesplititer_next(StringSplitIterator *__restrict self) {
	/* Literally the same as the non-case version, but use `dee_memcasemem(b|w|l)' instead. */
	union Dee_charptr_const result_start, result_end, next_ptr;
	size_t result_len, match_length;
	do {
		result_start.ptr = atomic_read(&self->s_next.ptr);
		if (!result_start.ptr)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->s_width) {

		CASE_WIDTH_1BYTE:
			result_end.cp8 = dee_memcasememb(result_start.cp8,
			                                 self->s_end.cp8 - result_start.cp8,
			                                 self->s_sep.cp8, self->s_sepsz,
			                                 &match_length);
			if (!result_end.cp8) {
				result_end.cp8 = self->s_end.cp8;
				next_ptr.cp8   = NULL;
			} else {
				next_ptr.cp8 = result_end.cp8 + match_length;
			}
			result_len = (size_t)(result_end.cp8 -
			                      result_start.cp8);
			break;

		CASE_WIDTH_2BYTE:
			result_end.cp16 = dee_memcasememw(result_start.cp16,
			                                  self->s_end.cp16 - result_start.cp16,
			                                  self->s_sep.cp16, self->s_sepsz,
			                                  &match_length);
			if (!result_end.cp16) {
				result_end.cp16 = self->s_end.cp16;
				next_ptr.cp16   = NULL;
			} else {
				next_ptr.cp16 = result_end.cp16 + match_length;
			}
			result_len = (size_t)(result_end.cp16 -
			                      result_start.cp16);
			break;

		CASE_WIDTH_4BYTE:
			result_end.cp32 = dee_memcasememl(result_start.cp32,
			                                  self->s_end.cp32 - result_start.cp32,
			                                  self->s_sep.cp32, self->s_sepsz,
			                                  &match_length);
			if (!result_end.cp32) {
				result_end.cp32 = self->s_end.cp32;
				next_ptr.cp32   = NULL;
			} else {
				next_ptr.cp32 = result_end.cp32 + match_length;
			}
			result_len = (size_t)(result_end.cp32 -
			                      result_start.cp32);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->s_next.ptr, result_start.ptr, next_ptr.ptr));

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start.ptr,
	                              result_len,
	                              self->s_width);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
splititer_serialize(StringSplitIterator *__restrict self,
                    DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(StringSplitIterator, field))
	StringSplitIterator *out;
	out = DeeSerial_Addr2Mem(writer, addr, StringSplitIterator);
	out->s_sepsz = self->s_sepsz;
	out->s_width = self->s_width;
	if (DeeSerial_PutObject(writer, ADDROF(s_split), self->s_split))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(s_next.ptr), atomic_read(&self->s_next.ptr)))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(s_start.ptr), self->s_start.ptr))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(s_end.ptr), self->s_end.ptr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(s_sep.ptr), self->s_sep.ptr);
err:
	return -1;
#undef ADDROF
}

STATIC_ASSERT(offsetof(StringSplitIterator, s_split) == offsetof(ProxyObject, po_obj));
#define splititer_fini  generic_proxy__fini
#define splititer_visit generic_proxy__visit

#define GET_SPLIT_NEXT(x) atomic_read(&(x)->s_next.ptr)

PRIVATE WUNUSED NONNULL((1)) int DCALL
splititer_bool(StringSplitIterator *__restrict self) {
	return GET_SPLIT_NEXT(self) != NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
splititer_hash(StringSplitIterator *self) {
	return Dee_HashPointer(GET_SPLIT_NEXT(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
splititer_compare(StringSplitIterator *self, StringSplitIterator *other) {
	union Dee_charptr_const lhs, rhs;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	lhs.ptr = GET_SPLIT_NEXT(self);
	rhs.ptr = GET_SPLIT_NEXT(other);
	if (lhs.ptr == NULL)
		lhs.ptr = (void *)-1l;
	if (rhs.ptr == NULL)
		rhs.ptr = (void *)-1l;
	Dee_return_compare(lhs.cp_char, rhs.cp_char);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp splititer_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&splititer_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&splititer_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_member tpconst splititer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(StringSplitIterator, s_split), "->?Ert:StringSplit"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
splititer_setup(StringSplitIterator *__restrict self,
                StringSplit *__restrict split) {
	self->s_width = STRING_WIDTH_COMMON(DeeString_WIDTH(split->s_str),
	                                    DeeString_WIDTH(split->s_sep));
	SWITCH_SIZEOF_WIDTH(self->s_width) {

	CASE_WIDTH_1BYTE:
		self->s_start.cp8 = DeeString_As1Byte((DeeObject *)split->s_str);
		self->s_end.cp8   = self->s_start.cp8 + WSTR_LENGTH(self->s_start.cp8);
		self->s_sep.cp8   = DeeString_As1Byte((DeeObject *)split->s_sep);
		break;

	CASE_WIDTH_2BYTE:
		self->s_start.cp16 = DeeString_As2Byte((DeeObject *)split->s_str);
		if unlikely(!self->s_start.cp16)
			goto err;
		self->s_end.cp16 = self->s_start.cp16 + WSTR_LENGTH(self->s_start.cp16);
		self->s_sep.cp16 = DeeString_As2Byte((DeeObject *)split->s_sep);
		if unlikely(!self->s_sep.cp16)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		self->s_start.cp32 = DeeString_As4Byte((DeeObject *)split->s_str);
		if unlikely(!self->s_start.cp32)
			goto err;
		self->s_end.cp32 = self->s_start.cp32 + WSTR_LENGTH(self->s_start.cp32);
		self->s_sep.cp32 = DeeString_As4Byte((DeeObject *)split->s_sep);
		if unlikely(!self->s_sep.cp32)
			goto err;
		break;
	}
	self->s_next.ptr = self->s_start.ptr;
	if (self->s_next.ptr == self->s_end.ptr)
		self->s_next.ptr = NULL;
	self->s_sepsz = WSTR_LENGTH(self->s_sep.ptr);
	self->s_split = split;
	Dee_Incref(split);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
splititer_copy(StringSplitIterator *__restrict self,
               StringSplitIterator *__restrict other) {
	self->s_split     = other->s_split;
	self->s_next.ptr  = GET_SPLIT_NEXT(other);
	self->s_start.ptr = other->s_start.ptr;
	self->s_end.ptr   = other->s_end.ptr;
	self->s_sep.ptr   = other->s_sep.ptr;
	self->s_sepsz     = other->s_sepsz;
	self->s_width     = other->s_width;
	Dee_Incref(self->s_split);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
splititer_init(StringSplitIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *split_type;
	StringSplit *split;
	DeeArg_Unpack1(err, argc, argv, "_StringSplitIterator", &split);
	split_type = &StringSplit_Type;
	if (Dee_TYPE(self) == &StringCaseSplitIterator_Type)
		split_type = &StringCaseSplit_Type;
	if (DeeObject_AssertTypeExact(split, split_type))
		goto err;
	return splititer_setup(self, split);
err:
	return -1;
}

INTERN DeeTypeObject StringSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:StringSplit)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSplitIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &splititer_copy,
			/* tp_deep_ctor:   */ &splititer_copy,
			/* tp_any_ctor:    */ &splititer_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &splititer_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&splititer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&splititer_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&splititer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &splititer_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&splititer_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ splititer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject StringCaseSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:StringCaseSplit)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &StringSplitIterator_Type, /* Extend the regular split() iterator type. */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSplitIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &splititer_copy,
			/* tp_deep_ctor:   */ &splititer_copy,
			/* tp_any_ctor:    */ &splititer_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &splititer_serialize
		),
		/* .tp_dtor        = */ NULL, /* INHERITED */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&splititer_bool, /* INHERITED */
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL, /* INHERITED */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &splititer_cmp, /* INHERITED */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplititer_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



STATIC_ASSERT(offsetof(StringSplit, s_str) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(StringSplit, s_str) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(StringSplit, s_sep) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(StringSplit, s_sep) == offsetof(ProxyObject2, po_obj2));
#define split_fini      generic_proxy2__fini
#define split_visit     generic_proxy2__visit
#define split_serialize generic_proxy2__serialize

PRIVATE WUNUSED NONNULL((1)) int DCALL
split_bool(StringSplit *__restrict self) {
	return !DeeString_IsEmpty(self->s_str);
}

LOCAL WUNUSED DREF StringSplitIterator *DCALL
split_doiter(StringSplit *__restrict self,
             DeeTypeObject *__restrict iter_type) {
	DREF StringSplitIterator *result;
	result = DeeObject_MALLOC(StringSplitIterator);
	if unlikely(!result)
		goto done;
	result->s_width = STRING_WIDTH_COMMON(DeeString_WIDTH(self->s_str),
	                                      DeeString_WIDTH(self->s_sep));
	SWITCH_SIZEOF_WIDTH(result->s_width) {

	CASE_WIDTH_1BYTE:
		result->s_start.cp8 = DeeString_As1Byte(Dee_AsObject(self->s_str));
		result->s_end.cp8   = result->s_start.cp8 + WSTR_LENGTH(result->s_start.cp8);
		result->s_sep.cp8   = DeeString_As1Byte(Dee_AsObject(self->s_sep));
		break;

	CASE_WIDTH_2BYTE:
		result->s_start.cp16 = DeeString_As2Byte(Dee_AsObject(self->s_str));
		if unlikely(!result->s_start.cp16)
			goto err_r;
		result->s_end.cp16 = result->s_start.cp16 + WSTR_LENGTH(result->s_start.cp16);
		result->s_sep.cp16 = DeeString_As2Byte(Dee_AsObject(self->s_sep));
		if unlikely(!result->s_sep.cp16)
			goto err_r;
		break;

	CASE_WIDTH_4BYTE:
		result->s_start.cp32 = DeeString_As4Byte(Dee_AsObject(self->s_str));
		if unlikely(!result->s_start.cp32)
			goto err_r;
		result->s_end.cp32 = result->s_start.cp32 + WSTR_LENGTH(result->s_start.cp32);
		result->s_sep.cp32 = DeeString_As4Byte(Dee_AsObject(self->s_sep));
		if unlikely(!result->s_sep.cp32)
			goto err_r;
		break;
	}
	result->s_next.ptr = result->s_start.ptr;
	if (result->s_next.ptr == result->s_end.ptr)
		result->s_next.ptr = NULL;
	result->s_sepsz = WSTR_LENGTH(result->s_sep.ptr);
	/* Finalize the split iterator and return it. */
	Dee_Incref(self);
	result->s_split = self;
	DeeObject_Init(result, iter_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF StringSplitIterator *DCALL
split_iter(StringSplit *__restrict self) {
	return split_doiter(self, &StringSplitIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF StringSplitIterator *DCALL
casesplit_iter(StringSplit *__restrict self) {
	return split_doiter(self, &StringCaseSplitIterator_Type);
}

PRIVATE struct type_member tpconst split_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringSplit, s_str), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__sep__", STRUCT_OBJECT, offsetof(StringSplit, s_sep), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst split_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringSplitIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst casesplit_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringCaseSplitIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_seq split_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&split_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem  = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem  = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem  = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter), /* TODO */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq casesplit_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplit_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem  = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem  = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem  = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter), /* TODO */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
split_copy(StringSplit *__restrict self,
           StringSplit *__restrict other) {
	self->s_str = other->s_str;
	self->s_sep = other->s_sep;
	Dee_Incref(self->s_str);
	Dee_Incref(self->s_sep);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
split_init(StringSplit *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack2(err, argc, argv, "_StringSplit", &self->s_str, &self->s_sep);
	if (DeeObject_AssertTypeExact(self->s_str, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(self->s_sep, &DeeString_Type))
		goto err;
	if unlikely(DeeString_IsEmpty(self->s_sep))
		return DeeError_Throwf(&DeeError_ValueError, "Empty split separator");
	Dee_Incref(self->s_str);
	Dee_Incref(self->s_sep);
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject StringSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSplit",
	/* .tp_doc      = */ DOC("(s:?Dstring,sep:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSplit,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &split_copy,
			/* tp_deep_ctor:   */ &split_copy,
			/* tp_any_ctor:    */ &split_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &split_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&split_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&split_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&split_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &split_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ split_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ split_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject StringCaseSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseSplit",
	/* .tp_doc      = */ DOC("(s:?Dstring,sep:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &StringSplit_Type, /* Extend the regular split() type. */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSplit,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &split_copy,
			/* tp_deep_ctor:   */ &split_copy,
			/* tp_any_ctor:    */ &split_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &split_serialize
		),
		/* .tp_dtor        = */ NULL, /* INHERITED */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&split_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL, /* INHERITED */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &casesplit_seq,
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
	/* .tp_class_members = */ casesplit_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Split(DeeStringObject *self,
                DeeStringObject *separator) {
	DREF StringSplit *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(separator, &DeeString_Type);
	if unlikely(DeeString_IsEmpty(separator))
		goto handle_empty_sep;
	result = DeeObject_MALLOC(StringSplit);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &StringSplit_Type);
	Dee_Incref(self);
	Dee_Incref(separator);
	result->s_str = self;      /* Inherit */
	result->s_sep = separator; /* Inherit */
done:
	return Dee_AsObject(result);
handle_empty_sep:
	return DeeSuper_New(&DeeSeq_Type, Dee_AsObject(self));
}

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseSplit(DeeStringObject *self,
                    DeeStringObject *separator) {
	DREF StringSplit *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(separator, &DeeString_Type);
	if unlikely(DeeString_IsEmpty(separator))
		goto handle_empty_sep;
	result = DeeObject_MALLOC(StringSplit);
	if unlikely(!result)
		goto done;
	/* Same as the regular split(), but use the case-insensitive sequence type. */
	DeeObject_Init(result, &StringCaseSplit_Type);
	Dee_Incref(self);
	Dee_Incref(separator);
	result->s_str = self;      /* Inherit */
	result->s_sep = separator; /* Inherit */
done:
	return Dee_AsObject(result);
handle_empty_sep:
	return DeeSuper_New(&DeeSeq_Type, Dee_AsObject(self));
}



typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, ls_str)  /* [1..1][const] The string that is being split into lines. */
	bool                                  ls_keep; /* [const] True if line-ends should be kept in resulting strings. */
} LineSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(LineSplit, ls_split) /* [1..1][const] The split descriptor object. */
	union Dee_charptr_const         ls_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                                           *                (points into the s_enc-specific string of `ls_split->ls_str')
	                                           *                When the iterator is exhausted, this pointer is set to NULL. */
	union Dee_charptr_const         ls_begin; /* [1..1][const] The starting address of the width string of `ls_split->ls_str'. */
	union Dee_charptr_const         ls_end;   /* [1..1][const] The end address of the width string of `ls_split->ls_str'. */
	int                             ls_width; /* [const] The width of `ls_split->ls_str' */
	bool                            ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplitIterator;

LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) uint8_t const *DCALL
find_lfb(uint8_t const *__restrict start, size_t size) {
	for (;; --size, ++start) {
		if (!size)
			return NULL;
		if (DeeUni_IsLF(*start))
			break;
	}
	return start;
}

LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) uint16_t const *DCALL
find_lfw(uint16_t const *__restrict start, size_t size) {
	for (;; --size, ++start) {
		if (!size)
			return NULL;
		if (DeeUni_IsLF(*start))
			break;
	}
	return start;
}

LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) uint32_t const *DCALL
find_lfl(uint32_t const *__restrict start, size_t size) {
	for (;; --size, ++start) {
		if (!size)
			return NULL;
		if (DeeUni_IsLF(*start))
			break;
	}
	return start;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lineiter_next(LineSplitIterator *__restrict self) {
	union Dee_charptr_const result_start, result_end, next_ptr;
	size_t result_len;
	do {
		result_start.ptr = atomic_read(&self->ls_next.ptr);
		if (!result_start.ptr)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->ls_width) {

		CASE_WIDTH_1BYTE:
			result_end.cp8 = find_lfb(result_start.cp8, self->ls_end.cp8 - result_start.cp8);
			if (!result_end.cp8) {
				result_end.cp8 = self->ls_end.cp8;
				next_ptr.cp8   = NULL;
			} else {
				next_ptr.cp8 = result_end.cp8 + 1;
				if (*result_end.cp8 == UNICODE_CR && *next_ptr.cp8 == UNICODE_LF)
					++next_ptr.cp8;
			}
			result_len = (size_t)(result_end.cp8 -
			                      result_start.cp8);
			break;

		CASE_WIDTH_2BYTE:
			result_end.cp16 = find_lfw(result_start.cp16,
			                           self->ls_end.cp16 - result_start.cp16);
			if (!result_end.cp16) {
				result_end.cp16 = self->ls_end.cp16;
				next_ptr.cp16   = NULL;
			} else {
				next_ptr.cp16 = result_end.cp16 + 1;
				if (*result_end.cp16 == UNICODE_CR && *next_ptr.cp16 == UNICODE_LF)
					++next_ptr.cp16;
			}
			result_len = (size_t)(result_end.cp16 -
			                      result_start.cp16);
			break;

		CASE_WIDTH_4BYTE:
			result_end.cp32 = find_lfl(result_start.cp32,
			                           self->ls_end.cp32 - result_start.cp32);
			if (!result_end.cp32) {
				result_end.cp32 = self->ls_end.cp32;
				next_ptr.cp32   = NULL;
			} else {
				next_ptr.cp32 = result_end.cp32 + 1;
				if (*result_end.cp32 == UNICODE_CR && *next_ptr.cp32 == UNICODE_LF)
					++next_ptr.cp32;
			}
			result_len = (size_t)(result_end.cp32 -
			                      result_start.cp32);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->ls_next.cp32, result_start.cp32, next_ptr.cp32));

	/* Add the linefeed itself if we're supposed to include it. */
	if (self->ls_keep && next_ptr.ptr) {
		size_t len = (size_t)((byte_t const *)next_ptr.ptr -
		                      (byte_t const *)result_end.ptr);
		result_len += Dee_STRING_DIV_SIZEOF_WIDTH(len, self->ls_width);
	}

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start.ptr,
	                              result_len,
	                              self->ls_width);
}


/* Assert that we're allowed to re-use some helper functions from `strsplit' */
STATIC_ASSERT(offsetof(StringSplitIterator, s_split) == offsetof(LineSplitIterator, ls_split));
STATIC_ASSERT(offsetof(StringSplitIterator, s_next) == offsetof(LineSplitIterator, ls_next));

PRIVATE NONNULL((1, 2)) void DCALL
lineiter_setup(LineSplitIterator *__restrict self,
               LineSplit *__restrict split) {
	self->ls_width     = DeeString_WIDTH(split->ls_str);
	self->ls_begin.ptr = DeeString_WSTR(split->ls_str);
	self->ls_next.ptr  = self->ls_begin.ptr;
	self->ls_end.ptr   = DeeString_WEND(split->ls_str);
	if (self->ls_next.ptr == self->ls_end.ptr)
		self->ls_next.ptr = NULL;
	self->ls_keep = split->ls_keep;
	Dee_Incref(split);
	self->ls_split = split;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lineiter_ctor(LineSplitIterator *__restrict self) {
	DREF LineSplit *split;
	split = (DREF LineSplit *)DeeObject_NewDefault(&StringLineSplit_Type);
	if unlikely(!split)
		goto err;
	lineiter_setup(self, split);
	Dee_DecrefNokill(split);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lineiter_copy(LineSplitIterator *__restrict self,
              LineSplitIterator *__restrict other) {
	self->ls_split     = other->ls_split;
	self->ls_next.ptr  = atomic_read(&other->ls_next.ptr);
	self->ls_begin.ptr = other->ls_begin.ptr;
	self->ls_end.ptr   = other->ls_end.ptr;
	self->ls_width     = other->ls_width;
	self->ls_keep      = other->ls_keep;
	Dee_Incref(self->ls_split);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lineiter_init(LineSplitIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	LineSplit *split;
	DeeArg_Unpack1(err, argc, argv, "_StringLineSplitIterator", &split);
	if (DeeObject_AssertTypeExact(split, &StringLineSplit_Type))
		goto err;
	lineiter_setup(self, split);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lineiter_serialize(LineSplitIterator *__restrict self,
                   DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	LineSplitIterator *out;
#define ADDROF(field) (addr + offsetof(LineSplitIterator, field))
	out = DeeSerial_Addr2Mem(writer, addr, LineSplitIterator);
	out->ls_width = self->ls_width;
	out->ls_keep = self->ls_keep;
	if (DeeSerial_PutObject(writer, ADDROF(ls_split), self->ls_split))
		goto err;
	if (DeeSerial_XPutPointer(writer, ADDROF(ls_next.ptr), atomic_read(&self->ls_next.ptr)))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(ls_begin.ptr), self->ls_begin.ptr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(ls_end.ptr), self->ls_end.ptr);
err:
	return -1;
#undef ADDROF
}



INTERN DeeTypeObject StringLineSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringLineSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:StringLineSplit)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LineSplitIterator,
			/* tp_ctor:        */ &lineiter_ctor,
			/* tp_copy_ctor:   */ &lineiter_copy,
			/* tp_deep_ctor:   */ &lineiter_copy,
			/* tp_any_ctor:    */ &lineiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &lineiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&splititer_fini, /* offset:`s_split' == offset:`ls_split' */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&splititer_bool /* offset:`s_next' == offset:`ls_next' */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&splititer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &splititer_cmp, /* offset:`s_next' == offset:`ls_next' */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lineiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ splititer_members, /* offset:`s_split' == offset:`ls_split' */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

PRIVATE struct type_member tpconst linesplit_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringLineSplitIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF LineSplitIterator *DCALL
linesplit_iter(LineSplit *__restrict self) {
	DREF LineSplitIterator *result;
	result = DeeObject_MALLOC(LineSplitIterator);
	if unlikely(!result)
		goto done;
	result->ls_width     = DeeString_WIDTH(self->ls_str);
	result->ls_begin.ptr = DeeString_WSTR(self->ls_str);
	result->ls_next.ptr  = result->ls_begin.ptr;
	result->ls_end.ptr   = DeeString_WEND(self->ls_str);
	if (result->ls_next.ptr == result->ls_end.ptr)
		result->ls_next.ptr = NULL;
	result->ls_keep = self->ls_keep;
	Dee_Incref(self);
	result->ls_split = self;
	DeeObject_Init(result, &StringLineSplitIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq linesplit_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&linesplit_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter), /* TODO */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

STATIC_ASSERT(offsetof(LineSplit, ls_str) == offsetof(ProxyObject, po_obj));
#define linesplit_fini      generic_proxy__fini
#define linesplit_visit     generic_proxy__visit
#define linesplit_serialize generic_proxy__serialize_and_memcpy

STATIC_ASSERT(offsetof(LineSplit, ls_str) == offsetof(StringSplit, s_str));
#define linesplit_bool split_bool

PRIVATE struct type_member tpconst linesplit_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(LineSplit, ls_str), "->?Dstring"),
	TYPE_MEMBER_FIELD("__keeplf__", STRUCT_CONST | STRUCT_CBOOL, offsetof(LineSplit, ls_keep)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
linesplit_ctor(LineSplit *__restrict self) {
	self->ls_str  = (DREF DeeStringObject *)DeeString_NewEmpty();
	self->ls_keep = false;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
linesplit_copy(LineSplit *__restrict self,
               LineSplit *__restrict other) {
	self->ls_str  = other->ls_str;
	self->ls_keep = other->ls_keep;
	Dee_Incref(self->ls_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
linesplit_init(LineSplit *__restrict self,
               size_t argc, DeeObject *const *argv) {
	self->ls_keep = false;
	if (DeeArg_Unpack(argc, argv, "o|b:_StringLineSplit",
	                  &self->ls_str, &self->ls_keep))
		goto err;
	if (DeeObject_AssertTypeExact(self->ls_str, &DeeString_Type))
		goto err;
	Dee_Incref(self->ls_str);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject StringLineSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringLineSplit",
	/* .tp_doc      = */ DOC("(s:?Dstring,keepends=!f)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LineSplit,
			/* tp_ctor:        */ &linesplit_ctor,
			/* tp_copy_ctor:   */ &linesplit_copy,
			/* tp_deep_ctor:   */ &linesplit_copy,
			/* tp_any_ctor:    */ &linesplit_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &linesplit_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&linesplit_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&linesplit_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&linesplit_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &linesplit_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ linesplit_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ linesplit_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

/* @return: An abstract sequence type for enumerating
 *          the segments of a string split into lines. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_SplitLines(DeeObject *__restrict self,
                     bool keepends) {
	DREF LineSplit *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	result = DeeObject_MALLOC(LineSplit);
	if unlikely(!result)
		goto done;
	/* Same as the regular split(), but use the case-insensitive sequence type. */
	DeeObject_Init(result, &StringLineSplit_Type);
	Dee_Incref(self);
	result->ls_str  = (DREF DeeStringObject *)self; /* Inherit */
	result->ls_keep = keepends;
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL */
