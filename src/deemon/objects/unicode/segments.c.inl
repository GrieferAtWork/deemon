/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
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

#include <hybrid/atomic.h>

#include "../../runtime/strings.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *s_str;   /* [1..1][const] The string that is being segmented. */
	size_t                s_siz;   /* [!0][const] The size of a single segment. */
	ATOMIC_DATA uint8_t  *s_ptr;   /* [1..1][in(DeeString_WSTR(s_str))] Pointer to the start of the next segment. */
	uint8_t              *s_end;   /* [1..1][== DeeString_WEND(s_str)] End pointer. */
	unsigned int          s_width; /* [const] The width of a single character. */
} StringSegmentsIterator;

#ifdef CONFIG_NO_THREADS
#define READ_PTR(x)            ((x)->s_ptr)
#else /* CONFIG_NO_THREADS */
#define READ_PTR(x) ATOMIC_READ((x)->s_ptr)
#endif /* !CONFIG_NO_THREADS */


typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *s_str; /* [1..1][const] The string that is being segmented. */
	size_t                s_siz; /* [!0][const] The size of a single segment. */
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

PRIVATE int DCALL
ssegiter_init(StringSegmentsIterator *__restrict self,
              size_t argc, DeeObject **argv) {
	StringSegments *seg;
	if (DeeArg_Unpack(argc, argv, "o:_StringSegmentsIterator", &seg))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)seg, &StringSegments_Type))
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
#ifdef CONFIG_NO_THREADS
	ptr = self->s_ptr;
	if (ptr >= self->s_end)
		return ITER_DONE;
	new_ptr     = ptr + self->s_siz * STRING_SIZEOF_WIDTH(self->s_width);
	self->s_ptr = new_ptr;
#else /* CONFIG_NO_THREADS */
	do {
		ptr = READ_PTR(self);
		if (ptr >= self->s_end)
			return ITER_DONE;
		new_ptr = ptr + self->s_siz * STRING_SIZEOF_WIDTH(self->s_width);
	} while (!ATOMIC_CMPXCH(self->s_ptr, ptr, new_ptr));
#endif /* !CONFIG_NO_THREADS */
	part_size = self->s_siz;
	if (new_ptr > self->s_end) {
		part_size = (self->s_end - ptr) / STRING_SIZEOF_WIDTH(self->s_width);
		ASSERT(part_size < self->s_siz);
	}
	return DeeString_NewWithWidth(ptr, part_size, self->s_width);
}

PRIVATE NONNULL((1)) void DCALL
ssegiter_fini(StringSegmentsIterator *__restrict self) {
	Dee_Decref(self->s_str);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ssegiter_bool(StringSegmentsIterator *__restrict self) {
	return READ_PTR(self) < self->s_end;
}

PRIVATE WUNUSED NONNULL((1)) DREF StringSegments *DCALL
ssegiter_getseq(StringSegmentsIterator *__restrict self) {
	return (DREF StringSegments *)DeeString_Segments(self->s_str,
	                                                 self->s_siz);
}

PRIVATE struct type_getset ssegiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssegiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:StringSegments") },
	{ NULL }
};


#define DEFINE_STRINGSEGMENTSITERATOR_COMPARE(name, op)                    \
	PRIVATE DREF DeeObject *DCALL                                          \
	name(StringSegmentsIterator *__restrict self,                          \
	     StringSegmentsIterator *__restrict other) {                       \
		if (DeeObject_AssertTypeExact((DeeObject *)other, Dee_TYPE(self))) \
			return NULL;                                                   \
		return_bool(READ_PTR(self) op READ_PTR(other));                    \
	}
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_eq, ==)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_ne, !=)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_lo, <)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_le, <=)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_gr, >)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(ssegiter_ge, >=)
#undef DEFINE_STRINGSEGMENTSITERATOR_COMPARE


PRIVATE struct type_cmp ssegiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssegiter_ge,
};


INTERN DeeTypeObject StringSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSegmentsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &ssegiter_ctor,
				/* .tp_copy_ctor = */ &ssegiter_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &ssegiter_init,
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
	/* .tp_visit         = */ NULL, /* No visit, because it only ever references strings. */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ssegiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssegiter_next,
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

PRIVATE int DCALL
sseg_init(StringSegments *__restrict self,
          size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "oIu:_StringSegments", &self->s_str, &self->s_siz) ||
	    DeeObject_AssertTypeExact((DeeObject *)self->s_str, &DeeString_Type))
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

PRIVATE NONNULL((1)) void DCALL
sseg_fini(StringSegments *__restrict self) {
	Dee_Decref(self->s_str);
}

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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sseg_size(StringSegments *__restrict self) {
	size_t result = DeeString_WLEN(self->s_str);
	result += (self->s_siz - 1);
	result /= self->s_siz;
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sseg_contains(StringSegments *self,
              DeeStringObject *other) {
	DeeStringObject *str;
	union dcharptr my_str, my_end, ot_str;
	if (DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
sseg_get(StringSegments *__restrict self,
         DeeObject *__restrict index_ob) {
	size_t index, length;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	length = DeeString_WLEN(self->s_str);
	length += (self->s_siz - 1);
	length /= self->s_siz;
	if unlikely(index > length)
		goto err_index;
	index *= self->s_siz;
	return string_getsubstr(self->s_str, index, index + self->s_siz);
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, length);
err:
	return NULL;
}



PRIVATE struct type_seq sseg_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sseg_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sseg_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sseg_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sseg_get
};


PRIVATE struct type_member sseg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringSegments, s_str), "->?Dstring"),
	TYPE_MEMBER_FIELD("__siz__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringSegments, s_siz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member sseg_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &StringSegmentsIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject StringSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSegments",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &sseg_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &sseg_init,
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
	/* .tp_visit         = */ NULL, /* No visit, because it only ever references strings. */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sseg_seq,
	/* .tp_iter_next     = */ NULL,
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
