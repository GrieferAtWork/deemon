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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"
/**/

#include <stddef.h> /* size_t, offsetof */
#include <stdint.h> /* uint8_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, s_str)   /* [1..1][const] The string that is being segmented. */
	size_t                                s_siz;   /* [!0][const] The size of a single segment. */
	DWEAK uint8_t                        *s_ptr;   /* [1..1][in(DeeString_WSTR(s_str))] Pointer to the start of the next segment. */
	uint8_t                              *s_end;   /* [1..1][== DeeString_WEND(s_str)] End pointer. */
	unsigned int                          s_width; /* [const] The width of a single character. */
} StringSegmentsIterator;
#define READ_PTR(x) atomic_read(&(x)->s_ptr)

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, s_str) /* [1..1][const] The string that is being segmented. */
	size_t                                s_siz; /* [!0][const] The size of a single segment. */
} StringSegments;



INTDEF DeeTypeObject StringSegmentsIterator_Type;
INTDEF DeeTypeObject StringSegments_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Segments(String *__restrict self,
                   size_t segment_size);


PRIVATE WUNUSED NONNULL((1)) int DCALL
ssegiter_ctor(StringSegmentsIterator *__restrict self) {
	self->s_str   = (DREF DeeStringObject *)DeeString_NewEmpty();
	self->s_siz   = 1;
	self->s_ptr   = (uint8_t *)DeeString_STR(self->s_str);
	self->s_end   = (uint8_t *)DeeString_STR(self->s_str);
	self->s_width = STRING_WIDTH_1BYTE;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ssegiter_copy(StringSegmentsIterator *__restrict self,
              StringSegmentsIterator *__restrict other) {
	self->s_str   = other->s_str;
	self->s_siz   = other->s_siz;
	self->s_ptr   = READ_PTR(other);
	self->s_end   = other->s_end;
	self->s_width = other->s_width;
	Dee_Incref(self->s_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ssegiter_init(StringSegmentsIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	StringSegments *seg;
	DeeArg_Unpack1(err, argc, argv, "_StringSegmentsIterator", &seg);
	if (DeeObject_AssertTypeExact(seg, &StringSegments_Type))
		goto err;
	self->s_str   = seg->s_str;
	self->s_siz   = seg->s_siz;
	self->s_ptr   = (uint8_t *)DeeString_WSTR(seg->s_str);
	self->s_end   = (uint8_t *)DeeString_WEND(seg->s_str);
	self->s_width = DeeString_WIDTH(seg->s_str);
	Dee_Incref(self->s_str);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ssegiter_next(StringSegmentsIterator *__restrict self) {
	size_t part_size;
	uint8_t *new_ptr, *ptr;
	do {
		ptr = READ_PTR(self);
		if (ptr >= self->s_end)
			return ITER_DONE;
		new_ptr = ptr + self->s_siz * STRING_SIZEOF_WIDTH(self->s_width);
	} while (!atomic_cmpxch_weak_or_write(&self->s_ptr, ptr, new_ptr));
	part_size = self->s_siz;
	if (new_ptr > self->s_end) {
		part_size = (self->s_end - ptr) / STRING_SIZEOF_WIDTH(self->s_width);
		ASSERT(part_size < self->s_siz);
	}
	return DeeString_NewWithWidth(ptr, part_size, self->s_width);
}

STATIC_ASSERT(offsetof(StringSegmentsIterator, s_str) == offsetof(ProxyObject, po_obj));
#define ssegiter_fini  generic_proxy__fini
#define ssegiter_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
ssegiter_bool(StringSegmentsIterator *__restrict self) {
	return READ_PTR(self) < self->s_end;
}

PRIVATE WUNUSED NONNULL((1)) DREF StringSegments *DCALL
ssegiter_getseq(StringSegmentsIterator *__restrict self) {
	return (DREF StringSegments *)DeeString_Segments(self->s_str,
	                                                 self->s_siz);
}

PRIVATE struct type_getset tpconst ssegiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &ssegiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:StringSegments"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
ssegiter_hash(StringSegmentsIterator *self) {
	return Dee_HashPointer(READ_PTR(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ssegiter_compare(StringSegmentsIterator *self, StringSegmentsIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(uint8_t *, READ_PTR(self),
	                    /*      */ READ_PTR(other));
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp ssegiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&ssegiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ssegiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


INTERN DeeTypeObject StringSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSegmentsIterator",
	/* .tp_doc      = */ DOC("(seg:?Ert:StringSegments)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSegmentsIterator,
			/* tp_ctor:        */ &ssegiter_ctor,
			/* tp_copy_ctor:   */ &ssegiter_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &ssegiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssegiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ssegiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ssegiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &ssegiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssegiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ssegiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};
#undef READ_PTR





PRIVATE WUNUSED NONNULL((1)) int DCALL
sseg_ctor(StringSegments *__restrict self) {
	self->s_str = (DREF DeeStringObject *)DeeString_NewEmpty();
	self->s_siz = 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sseg_init(StringSegments *__restrict self,
          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_StringSegments",
	                  &self->s_str, &self->s_siz))
		goto err;
	if (DeeObject_AssertTypeExact(self->s_str, &DeeString_Type))
		goto err;
	if (!self->s_siz) {
		err_invalid_segment_size(self->s_siz);
		goto err;
	}
	Dee_Incref(self->s_str);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(StringSegments, s_str) == offsetof(ProxyObject, po_obj));
#define sseg_fini  generic_proxy__fini
#define sseg_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
sseg_bool(StringSegments *__restrict self) {
	return !DeeString_IsEmpty(self->s_str);
}


PRIVATE WUNUSED NONNULL((1)) DREF StringSegmentsIterator *DCALL
sseg_iter(StringSegments *__restrict self) {
	DREF StringSegmentsIterator *result;
	result = DeeObject_MALLOC(StringSegmentsIterator);
	if unlikely(!result)
		goto done;
	Dee_Incref(self->s_str);
	result->s_str   = self->s_str;
	result->s_siz   = self->s_siz;
	result->s_ptr   = (uint8_t *)DeeString_WSTR(self->s_str);
	result->s_end   = (uint8_t *)DeeString_WEND(self->s_str);
	result->s_width = DeeString_WIDTH(self->s_str);
	DeeObject_Init(result, &StringSegmentsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sseg_size(StringSegments *__restrict self) {
	size_t length;
	length = DeeString_WLEN(self->s_str);
	length = CEILDIV(length, self->s_siz);
	return length;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sseg_contains(StringSegments *self, DeeStringObject *other) {
	DeeStringObject *str;
	union dcharptr_const my_str, my_end, ot_str;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	str = self->s_str;
	if (DeeString_WLEN(other) != self->s_siz) {
		size_t last_part, last_index;
		if (DeeString_WLEN(other) > self->s_siz)
			goto do_return_false;
		last_part = DeeString_WLEN(str);
		last_part %= self->s_siz;
		if (DeeString_WLEN(other) != last_part)
			goto do_return_false;
		last_index = DeeString_WLEN(str) - last_part;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(str),
		                                        DeeString_WIDTH(other))) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_As1Byte((DeeObject *)str);
			ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
			return_bool(MEMEQB(my_str.cp8 + last_index, ot_str.cp8, last_part));

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_As2Byte((DeeObject *)str);
			if unlikely(!my_str.cp16)
				goto err;
			ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
			if unlikely(!ot_str.cp16)
				goto err;
			return_bool(MEMEQW(my_str.cp16 + last_index, ot_str.cp16, last_part));

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_As4Byte((DeeObject *)str);
			if unlikely(!my_str.cp32)
				goto err;
			ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
			if unlikely(!ot_str.cp32)
				goto err;
			return_bool(MEMEQL(my_str.cp32 + last_index, ot_str.cp32, last_part));
		}
	}
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(str),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)str);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
		my_end.cp8 = my_str.cp8 + WSTR_LENGTH(my_str.cp8) - self->s_siz;
		for (; my_str.cp8 <= my_end.cp8; my_str.cp8 += self->s_siz) {
			if (MEMEQB(my_str.cp8, ot_str.cp8, self->s_siz))
				goto do_return_true;
		}
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)str);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_end.cp16 = my_str.cp16 + WSTR_LENGTH(my_str.cp16) - self->s_siz;
		for (; my_str.cp16 <= my_end.cp16; my_str.cp16 += self->s_siz) {
			if (MEMEQW(my_str.cp16, ot_str.cp16, self->s_siz))
				goto do_return_true;
		}
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)str);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_end.cp32 = my_str.cp32 + WSTR_LENGTH(my_str.cp32) - self->s_siz;
		for (; my_str.cp32 <= my_end.cp32; my_str.cp32 += self->s_siz) {
			if (MEMEQL(my_str.cp32, ot_str.cp32, self->s_siz))
				goto do_return_true;
		}
		break;
	}
do_return_false:
	return_false;
do_return_true:
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
sseg_getitem_index(StringSegments *__restrict self, size_t index) {
	size_t length;
	length = DeeString_WLEN(self->s_str);
	length = CEILDIV(length, self->s_siz);
	if unlikely(index >= length)
		goto err_index;
	index *= self->s_siz;
	return string_getsubstr(self->s_str, index, index + self->s_siz);
err_index:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, length);
	return NULL;
}



PRIVATE struct type_seq sseg_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sseg_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sseg_contains,
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&sseg_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&sseg_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sseg_getitem_index,
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


PRIVATE struct type_member tpconst sseg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringSegments, s_str), "->?Dstring"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringSegments, s_siz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sseg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringSegmentsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject StringSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSegments",
	/* .tp_doc      = */ DOC("(s:?Dstring,siz:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StringSegments,
			/* tp_ctor:        */ &sseg_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &sseg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sseg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sseg_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sseg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__26B2EC529683DE3C),
	/* .tp_seq           = */ &sseg_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sseg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sseg_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Segments(String *__restrict self,
                   size_t segment_size) {
	DREF StringSegments *result;
	ASSERT(segment_size != 0);
	result = DeeObject_MALLOC(StringSegments);
	if unlikely(!result)
		goto done;
	result->s_str = (DREF DeeStringObject *)self;
	Dee_Incref(self);
	result->s_siz = segment_size;
	DeeObject_Init(result, &StringSegments_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_C_INL */
