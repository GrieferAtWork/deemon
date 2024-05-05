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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>

DECL_BEGIN

INTDEF DeeTypeObject StringSplit_Type;
INTDEF DeeTypeObject StringSplitIterator_Type;
INTDEF DeeTypeObject StringCaseSplit_Type;
INTDEF DeeTypeObject StringCaseSplitIterator_Type;
INTDEF DeeTypeObject StringLineSplit_Type;
INTDEF DeeTypeObject StringLineSplitIterator_Type;

typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *s_str; /* [1..1][const] The string that is being split. */
	DREF DeeStringObject *s_sep; /* [1..1][const] The string to search for. */
} StringSplit;

typedef struct {
	OBJECT_HEAD
	DREF StringSplit *s_split; /* [1..1][const] The split descriptor object. */
	uint8_t          *s_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                            *                (points into the s_enc-specific string of `s_split->s_str')
	                            *                When the iterator is exhausted, this pointer is set to `NULL'. */
	uint8_t          *s_begin; /* [1..1][const] The starting address of the width string of `s_split->s_str'. */
	uint8_t          *s_end;   /* [1..1][const] The end address of the width string of `s_split->s_str'. */
	uint8_t          *s_sep;   /* [1..1][const] The starting address of the `s_enc'-encoded string of `s_split->s_sep'. */
	size_t            s_sepsz; /* [1..1][const][== WSTR_LENGTH(s_sep)] The length of separator string. */
	int               s_width; /* [const] The width of `s_split->s_str' */
} StringSplitIterator;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
splititer_next(StringSplitIterator *__restrict self) {
	uint8_t *result_start, *result_end;
	uint8_t *next_ptr;
	size_t result_len;
	do {
		result_start = atomic_read(&self->s_next);
		if (!result_start)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->s_width) {

		CASE_WIDTH_1BYTE:
			result_end = (uint8_t *)memmemb((uint8_t *)result_start,
			                                (uint8_t *)self->s_end - (uint8_t *)result_start,
			                                (uint8_t *)self->s_sep, self->s_sepsz);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + self->s_sepsz * 1;
			}
			result_len = (size_t)((uint8_t *)result_end - (uint8_t *)result_start);
			break;

		CASE_WIDTH_2BYTE:
			result_end = (uint8_t *)memmemw((uint16_t *)result_start,
			                                (uint16_t *)self->s_end - (uint16_t *)result_start,
			                                (uint16_t *)self->s_sep, self->s_sepsz);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + self->s_sepsz * 2;
			}
			result_len = (size_t)((uint16_t *)result_end - (uint16_t *)result_start);
			break;

		CASE_WIDTH_4BYTE:
			result_end = (uint8_t *)memmeml((uint32_t *)result_start,
			                                (uint32_t *)self->s_end - (uint32_t *)result_start,
			                                (uint32_t *)self->s_sep, self->s_sepsz);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + self->s_sepsz * 4;
			}
			result_len = (size_t)((uint32_t *)result_end - (uint32_t *)result_start);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->s_next, result_start, next_ptr));

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start,
	                              result_len,
	                              self->s_width);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
casesplititer_next(StringSplitIterator *__restrict self) {
	/* Literally the same as the non-case version, but use `memcasemem(b|w|l)' instead. */
	uint8_t *result_start, *result_end;
	uint8_t *next_ptr;
	size_t result_len, match_length;
	do {
		result_start = atomic_read(&self->s_next);
		if (!result_start)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->s_width) {

		CASE_WIDTH_1BYTE:
			result_end = (uint8_t *)memcasememb((uint8_t *)result_start,
			                                    (uint8_t *)self->s_end - (uint8_t *)result_start,
			                                    (uint8_t *)self->s_sep, self->s_sepsz,
			                                    &match_length);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + match_length * 1;
			}
			result_len = (size_t)((uint8_t *)result_end - (uint8_t *)result_start);
			break;

		CASE_WIDTH_2BYTE:
			result_end = (uint8_t *)memcasememw((uint16_t *)result_start,
			                                    (uint16_t *)self->s_end - (uint16_t *)result_start,
			                                    (uint16_t *)self->s_sep, self->s_sepsz,
			                                    &match_length);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + match_length * 2;
			}
			result_len = (size_t)((uint16_t *)result_end - (uint16_t *)result_start);
			break;

		CASE_WIDTH_4BYTE:
			result_end = (uint8_t *)memcasememl((uint32_t *)result_start,
			                                    (uint32_t *)self->s_end - (uint32_t *)result_start,
			                                    (uint32_t *)self->s_sep, self->s_sepsz,
			                                    &match_length);
			if (!result_end) {
				result_end = self->s_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + match_length * 4;
			}
			result_len = (size_t)((uint32_t *)result_end - (uint32_t *)result_start);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->s_next, result_start, next_ptr));

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start,
	                              result_len,
	                              self->s_width);
}


PRIVATE NONNULL((1)) void DCALL
splititer_fini(StringSplitIterator *__restrict self) {
	Dee_Decref(self->s_split);
}

PRIVATE NONNULL((1, 2)) void DCALL
splititer_visit(StringSplitIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->s_split);
}

#define GET_SPLIT_NEXT(x) atomic_read(&(x)->s_next)

PRIVATE WUNUSED NONNULL((1)) int DCALL
splititer_bool(StringSplitIterator *__restrict self) {
	return GET_SPLIT_NEXT(self) != NULL;
}

#define DEFINE_STRINGSPLITITER_COMPARE(name, op)                  \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL         \
	name(StringSplitIterator *self, StringSplitIterator *other) { \
		uint8_t *x, *y;                                           \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))     \
			goto err;                                             \
		x = GET_SPLIT_NEXT(self);                                 \
		y = GET_SPLIT_NEXT(other);                                \
		if (!x)                                                   \
			x = (uint8_t *)(uintptr_t)-1;                         \
		if (!y)                                                   \
			y = (uint8_t *)(uintptr_t)-1;                         \
		return_bool(x op y);                                      \
	err:                                                          \
		return NULL;                                              \
	}
DEFINE_STRINGSPLITITER_COMPARE(splititer_eq, ==)
DEFINE_STRINGSPLITITER_COMPARE(splititer_ne, !=)
DEFINE_STRINGSPLITITER_COMPARE(splititer_lo, <)
DEFINE_STRINGSPLITITER_COMPARE(splititer_le, <=)
DEFINE_STRINGSPLITITER_COMPARE(splititer_gr, >)
DEFINE_STRINGSPLITITER_COMPARE(splititer_ge, >=)
#undef DEFINE_STRINGSPLITITER_COMPARE

PRIVATE struct type_cmp splititer_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ NULL,
	/* .tp_eq         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_eq,
	/* .tp_ne         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_ne,
	/* .tp_lo         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_lo,
	/* .tp_le         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_le,
	/* .tp_gr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_gr,
	/* .tp_ge         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&splititer_ge
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
		self->s_begin = DeeString_As1Byte((DeeObject *)split->s_str);
		self->s_end   = self->s_begin + WSTR_LENGTH(self->s_begin);
		self->s_sep   = DeeString_As1Byte((DeeObject *)split->s_sep);
		break;

	CASE_WIDTH_2BYTE:
		self->s_begin = (uint8_t *)DeeString_As2Byte((DeeObject *)split->s_str);
		if unlikely(!self->s_begin)
			goto err;
		self->s_end = self->s_begin + WSTR_LENGTH(self->s_begin) * 2;
		self->s_sep = (uint8_t *)DeeString_As2Byte((DeeObject *)split->s_sep);
		if unlikely(!self->s_sep)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		self->s_begin = (uint8_t *)DeeString_As4Byte((DeeObject *)split->s_str);
		if unlikely(!self->s_begin)
			goto err;
		self->s_end = self->s_begin + WSTR_LENGTH(self->s_begin) * 4;
		self->s_sep = (uint8_t *)DeeString_As4Byte((DeeObject *)split->s_sep);
		if unlikely(!self->s_sep)
			goto err;
		break;
	}
	self->s_next = self->s_begin;
	if (self->s_next == self->s_end)
		self->s_next = NULL;
	self->s_sepsz = WSTR_LENGTH(self->s_sep);
	self->s_split = split;
	Dee_Incref(split);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
splititer_ctor(StringSplitIterator *__restrict self) {
	DREF StringSplit *split;
	split = (DREF StringSplit *)DeeObject_NewDefault(&StringSplit_Type);
	if unlikely(!split)
		goto err;
	if unlikely(splititer_setup(self, split))
		goto err_split;
	Dee_DecrefNokill(split);
	return 0;
err_split:
	Dee_Decref(split);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
splititer_copy(StringSplitIterator *__restrict self,
               StringSplitIterator *__restrict other) {
	self->s_split = other->s_split;
	self->s_next  = GET_SPLIT_NEXT(other);
	self->s_begin = other->s_begin;
	self->s_end   = other->s_end;
	self->s_sep   = other->s_sep;
	self->s_sepsz = other->s_sepsz;
	self->s_width = other->s_width;
	Dee_Incref(self->s_split);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
splititer_init(StringSplitIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *split_type;
	StringSplit *split;
	if (DeeArg_Unpack(argc, argv, "o:_StringSplitIterator", &split))
		goto err;
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
	/* .tp_doc      = */ DOC("next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&splititer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&splititer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&splititer_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&splititer_init,
				TYPE_FIXED_ALLOCATOR(StringSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&splititer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&splititer_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&splititer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &splititer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&splititer_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ splititer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject StringCaseSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseSplitIterator",
	/* .tp_doc      = */ DOC("next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &StringSplitIterator_Type, /* Extend the regular split() iterator type. */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&splititer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&splititer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&splititer_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&splititer_init,
				TYPE_FIXED_ALLOCATOR(StringSplitIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* INHERITED */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL  /* INHERITED */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /* INHERITED */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* INHERITED */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplititer_next,
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



PRIVATE NONNULL((1)) void DCALL
split_fini(StringSplit *__restrict self) {
	Dee_Decref(self->s_str);
	Dee_Decref(self->s_sep);
}

PRIVATE NONNULL((1, 2)) void DCALL
split_visit(StringSplit *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->s_str);
	Dee_Visit(self->s_sep);
}

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
		result->s_begin = DeeString_As1Byte((DeeObject *)self->s_str);
		result->s_end   = result->s_begin + WSTR_LENGTH(result->s_begin);
		result->s_sep   = DeeString_As1Byte((DeeObject *)self->s_sep);
		break;

	CASE_WIDTH_2BYTE:
		result->s_begin = (uint8_t *)DeeString_As2Byte((DeeObject *)self->s_str);
		if unlikely(!result->s_begin)
			goto err_r;
		result->s_end = result->s_begin + WSTR_LENGTH(result->s_begin) * 2;
		result->s_sep = (uint8_t *)DeeString_As2Byte((DeeObject *)self->s_sep);
		if unlikely(!result->s_sep)
			goto err_r;
		break;

	CASE_WIDTH_4BYTE:
		result->s_begin = (uint8_t *)DeeString_As4Byte((DeeObject *)self->s_str);
		if unlikely(!result->s_begin)
			goto err_r;
		result->s_end = result->s_begin + WSTR_LENGTH(result->s_begin) * 4;
		result->s_sep = (uint8_t *)DeeString_As4Byte((DeeObject *)self->s_sep);
		if unlikely(!result->s_sep)
			goto err_r;
		break;
	}
	result->s_next = result->s_begin;
	if (result->s_next == result->s_end)
		result->s_next = NULL;
	result->s_sepsz = WSTR_LENGTH(result->s_sep);
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
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst casesplit_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringCaseSplitIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_seq split_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&split_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE struct type_seq casesplit_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&casesplit_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
split_ctor(StringSplit *__restrict self) {
	self->s_str = (DREF DeeStringObject *)Dee_EmptyString;
	self->s_sep = (DREF DeeStringObject *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

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
	if (DeeArg_Unpack(argc, argv, "oo:_StringSplit", &self->s_str, &self->s_sep))
		goto err;
	if (DeeObject_AssertTypeExact(self->s_str, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(self->s_sep, &DeeString_Type))
		goto err;
	Dee_Incref(self->s_str);
	Dee_Incref(self->s_sep);
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject StringSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringSplit",
	/* .tp_doc      = */ DOC("()\n"
	                         "(s:?Dstring,sep:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&split_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&split_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&split_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&split_init,
				TYPE_FIXED_ALLOCATOR(StringSplit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&split_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&split_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&split_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &split_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ split_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ split_class_members
};

INTERN DeeTypeObject StringCaseSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseSplit",
	/* .tp_doc      = */ DOC("()\n"
	                         "(s:?Dstring,sep:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &StringSplit_Type, /* Extend the regular split() type. */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&split_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&split_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&split_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&split_init,
				TYPE_FIXED_ALLOCATOR(StringSplit)
			}
		},
		/* .tp_dtor        = */ NULL, /* INHERITED */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&split_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /* INHERITED */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &casesplit_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ casesplit_class_members
};

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Split(DeeStringObject *self,
                DeeStringObject *separator) {
	DREF StringSplit *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(separator, &DeeString_Type);
	result = DeeObject_MALLOC(StringSplit);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &StringSplit_Type);
	Dee_Incref(self);
	Dee_Incref(separator);
	result->s_str = self;      /* Inherit */
	result->s_sep = separator; /* Inherit */
done:
	return (DREF DeeObject *)result;
}

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseSplit(DeeStringObject *self,
                    DeeStringObject *separator) {
	DREF StringSplit *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(separator, &DeeString_Type);
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
	return (DREF DeeObject *)result;
}



typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *ls_str;   /* [1..1][const] The string that is being split into lines. */
	bool                  ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplit;

typedef struct {
	OBJECT_HEAD
	DREF LineSplit *ls_split; /* [1..1][const] The split descriptor object. */
	uint8_t        *ls_next;  /* [0..1][atomic] Pointer to the starting address of the next split
	                           *                (points into the s_enc-specific string of `ls_split->ls_str')
	                           *                When the iterator is exhausted, this pointer is set to NULL. */
	uint8_t        *ls_begin; /* [1..1][const] The starting address of the width string of `ls_split->ls_str'. */
	uint8_t        *ls_end;   /* [1..1][const] The end address of the width string of `ls_split->ls_str'. */
	int             ls_width; /* [const] The width of `ls_split->ls_str' */
	bool            ls_keep;  /* [const] True if line-ends should be kept in resulting strings. */
} LineSplitIterator;

LOCAL uint8_t *DCALL
find_lfb(uint8_t *__restrict start, size_t size) {
	for (;; --size, ++start) {
		if (!size)
			return NULL;
		if (DeeUni_IsLF(*start))
			break;
	}
	return start;
}

LOCAL uint16_t *DCALL
find_lfw(uint16_t *__restrict start, size_t size) {
	for (;; --size, ++start) {
		if (!size)
			return NULL;
		if (DeeUni_IsLF(*start))
			break;
	}
	return start;
}

LOCAL uint32_t *DCALL
find_lfl(uint32_t *__restrict start, size_t size) {
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
	uint8_t *result_start, *result_end;
	uint8_t *next_ptr;
	size_t result_len;
	do {
		result_start = atomic_read(&self->ls_next);
		if (!result_start)
			return ITER_DONE;
		SWITCH_SIZEOF_WIDTH(self->ls_width) {

		CASE_WIDTH_1BYTE:
			result_end = (uint8_t *)find_lfb((uint8_t *)result_start,
			                                 (uint8_t *)self->ls_end - (uint8_t *)result_start);
			if (!result_end) {
				result_end = self->ls_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + 1;
				if (*(uint8_t *)result_end == UNICODE_CR &&
				    *(uint8_t *)next_ptr == UNICODE_LF)
					next_ptr += 1;
			}
			result_len = (size_t)((uint8_t *)result_end -
			                      (uint8_t *)result_start);
			break;

		CASE_WIDTH_2BYTE:
			result_end = (uint8_t *)find_lfw((uint16_t *)result_start,
			                                 (uint16_t *)self->ls_end - (uint16_t *)result_start);
			if (!result_end) {
				result_end = self->ls_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + 2;
				if (*(uint16_t *)result_end == UNICODE_CR &&
				    *(uint16_t *)next_ptr == UNICODE_LF)
					next_ptr += 2;
			}
			result_len = (size_t)((uint16_t *)result_end -
			                      (uint16_t *)result_start);
			break;

		CASE_WIDTH_4BYTE:
			result_end = (uint8_t *)find_lfl((uint32_t *)result_start,
			                                 (uint32_t *)self->ls_end - (uint32_t *)result_start);
			if (!result_end) {
				result_end = self->ls_end;
				next_ptr   = NULL;
			} else {
				next_ptr = result_end + 4;
				if (*(uint32_t *)result_end == UNICODE_CR &&
				    *(uint32_t *)next_ptr == UNICODE_LF)
					next_ptr += 4;
			}
			result_len = (size_t)((uint32_t *)result_end -
			                      (uint32_t *)result_start);
			break;
		}
	} while (!atomic_cmpxch_weak(&self->ls_next, result_start, next_ptr));

	/* Add the linefeed itself if we're supposed to include it. */
	if (self->ls_keep && next_ptr)
		result_len += (size_t)(next_ptr - result_end) / STRING_SIZEOF_WIDTH(self->ls_width);

	/* Return the part-string. */
	return DeeString_NewWithWidth(result_start,
	                              result_len,
	                              self->ls_width);
}


/* Assert that we're allowed to re-use some helper functions from `strsplit' */
STATIC_ASSERT(offsetof(StringSplitIterator, s_split) == offsetof(LineSplitIterator, ls_split));
STATIC_ASSERT(offsetof(StringSplitIterator, s_next) == offsetof(LineSplitIterator, ls_next));

PRIVATE NONNULL((1, 2)) void DCALL
lineiter_setup(LineSplitIterator *__restrict self,
               LineSplit *__restrict split) {
	self->ls_width = DeeString_WIDTH(split->ls_str);
	self->ls_begin = (uint8_t *)DeeString_WSTR(split->ls_str);
	self->ls_next  = self->ls_begin;
	self->ls_end   = (uint8_t *)DeeString_WEND(split->ls_str);
	if (self->ls_next == self->ls_end)
		self->ls_next = NULL;
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
	self->ls_split = other->ls_split;
	self->ls_next  = atomic_read(&other->ls_next);
	self->ls_begin = other->ls_begin;
	self->ls_end   = other->ls_end;
	self->ls_width = other->ls_width;
	self->ls_keep  = other->ls_keep;
	Dee_Incref(self->ls_split);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lineiter_init(LineSplitIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	LineSplit *split;
	if (DeeArg_Unpack(argc, argv, "o:_StringLineSplitIterator", &split))
		goto err;
	if (DeeObject_AssertTypeExact(split, &StringLineSplit_Type))
		goto err;
	lineiter_setup(self, split);
	return 0;
err:
	return -1;
}



INTERN DeeTypeObject StringLineSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringLineSplitIterator",
	/* .tp_doc      = */ DOC("next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&lineiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&lineiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&lineiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&lineiter_init,
				TYPE_FIXED_ALLOCATOR(LineSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&splititer_fini, /* offset:`s_split' == offset:`ls_split' */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&splititer_bool /* offset:`s_next' == offset:`ls_next' */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&splititer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &splititer_cmp, /* offset:`s_next' == offset:`ls_next' */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&lineiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ splititer_members, /* offset:`s_split' == offset:`ls_split' */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE struct type_member tpconst linesplit_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringLineSplitIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF LineSplitIterator *DCALL
linesplit_iter(LineSplit *__restrict self) {
	DREF LineSplitIterator *result;
	result = DeeObject_MALLOC(LineSplitIterator);
	if unlikely(!result)
		goto done;
	result->ls_width = DeeString_WIDTH(self->ls_str);
	result->ls_begin = (uint8_t *)DeeString_WSTR(self->ls_str);
	result->ls_next  = result->ls_begin;
	result->ls_end   = (uint8_t *)DeeString_WEND(self->ls_str);
	if (result->ls_next == result->ls_end)
		result->ls_next = NULL;
	result->ls_keep = self->ls_keep;
	Dee_Incref(self);
	result->ls_split = self;
	DeeObject_Init(result, &StringLineSplitIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq linesplit_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&linesplit_iter
};

PRIVATE NONNULL((1)) void DCALL
linesplit_fini(LineSplit *__restrict self) {
	Dee_Decref(self->ls_str);
}

PRIVATE NONNULL((1, 2)) void DCALL
linesplit_visit(LineSplit *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ls_str);
}

STATIC_ASSERT(offsetof(LineSplit, ls_str) == offsetof(StringSplit, s_str));
#define linesplit_bool split_bool

PRIVATE struct type_member tpconst linesplit_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(LineSplit, ls_str), "->?Dstring"),
	TYPE_MEMBER_FIELD("__keeplf__", STRUCT_CONST | STRUCT_CBOOL, offsetof(LineSplit, ls_keep)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
linesplit_ctor(LineSplit *__restrict self) {
	self->ls_str  = (DREF DeeStringObject *)Dee_EmptyString;
	self->ls_keep = false;
	Dee_Incref(Dee_EmptyString);
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
	if (DeeArg_Unpack(argc, argv, "o|b:_StringLineSplit", &self->ls_str, &self->ls_keep))
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
	/* .tp_doc      = */ DOC("()\n"
	                         "(s:?Dstring,keepends=!f)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&linesplit_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&linesplit_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&linesplit_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&linesplit_init,
				TYPE_FIXED_ALLOCATOR(LineSplit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&linesplit_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&linesplit_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&linesplit_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &linesplit_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ linesplit_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ linesplit_class_members
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SPLIT_C_INL */
