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

#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>

#include "../../runtime/strings.h"
#include "../generic-proxy.h"

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
	self->s_str   = (DREF DeeStringObject *)Dee_EmptyString;
	self->s_siz   = 1;
	self->s_ptr   = (uint8_t *)DeeString_STR(Dee_EmptyString);
	self->s_end   = (uint8_t *)DeeString_STR(Dee_EmptyString);
	self->s_width = STRING_WIDTH_1BYTE;
	Dee_Incref(Dee_EmptyString);
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
	if (DeeArg_Unpack(argc, argv, "o:_StringSegmentsIterator", &seg))
		goto err;
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
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ssegiter_compare,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ssegiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ssegiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&ssegiter_init,
				TYPE_FIXED_ALLOCATOR(StringSegmentsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssegiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ssegiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ssegiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ssegiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssegiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ssegiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#undef READ_PTR





PRIVATE WUNUSED NONNULL((1)) int DCALL
sseg_ctor(StringSegments *__restrict self) {
	self->s_str = (DREF DeeStringObject *)Dee_EmptyString;
	self->s_siz = 1;
	Dee_Incref(Dee_EmptyString);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sseg_init(StringSegments *__restrict self,
          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_StringSegments", &self->s_str, &self->s_siz))
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
	union dcharptr my_str, my_end, ot_str;
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
	err_index_out_of_bounds((DeeObject *)self, index, length);
	return NULL;
}



PRIVATE struct type_seq sseg_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sseg_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sseg_contains,
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
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&sseg_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&sseg_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sseg_getitem_index,
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


PRIVATE struct type_member tpconst sseg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringSegments, s_str), "->?Dstring"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringSegments, s_siz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sseg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringSegmentsIterator_Type),
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sseg_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&sseg_init,
				TYPE_FIXED_ALLOCATOR(StringSegments)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sseg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sseg_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sseg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sseg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sseg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sseg_class_members
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_C_INL */
