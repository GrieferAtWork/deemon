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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/seq.h>
#include <deemon/util/atomic.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF String *sf_str;    /* [1..1][const] The string that is being searched. */
	DREF String *sf_needle; /* [1..1][const] The needle being searched for. */
	size_t       sf_start;  /* [const] Starting search index. */
	size_t       sf_end;    /* [const] End search index. */
} StringFind;

typedef struct {
	OBJECT_HEAD
	DREF StringFind     *sfi_find;       /* [1..1][const] The underlying find-controller. */
	union dcharptr       sfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK union dcharptr sfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	union dcharptr       sfi_end;        /* [1..1][const] End pointer. */
	union dcharptr       sfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t               sfi_needle_len; /* [const] Length of the needle being searched. */
	unsigned int         sfi_width;      /* [const] The common width of the searched, and needle string. */
} StringFindIterator;

INTDEF WUNUSED DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end);
INTDEF WUNUSED DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end);
#define READ_PTR(x) atomic_read(&(x)->sfi_ptr.ptr)


INTDEF DeeTypeObject StringFindIterator_Type;
INTDEF DeeTypeObject StringFind_Type;
INTDEF DeeTypeObject StringCaseFindIterator_Type;
INTDEF DeeTypeObject StringCaseFind_Type;


PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_ctor(StringFindIterator *__restrict self) {
	self->sfi_find = (DREF StringFind *)DeeString_FindAll((String *)Dee_EmptyString,
	                                                      (String *)Dee_EmptyString,
	                                                      0, 0);
	if unlikely(!self->sfi_find)
		goto err;
	self->sfi_start.cp8      = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_ptr.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_end.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_ptr.cp8 = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_len     = 0;
	self->sfi_width          = STRING_WIDTH_1BYTE;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_ctor(StringFindIterator *__restrict self) {
	self->sfi_find = (DREF StringFind *)DeeString_CaseFindAll((String *)Dee_EmptyString,
	                                                          (String *)Dee_EmptyString,
	                                                          0, 0);
	if unlikely(!self->sfi_find)
		goto err;
	self->sfi_start.cp8      = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_ptr.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_end.cp8        = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_ptr.cp8 = DeeString_As1Byte(Dee_EmptyString);
	self->sfi_needle_len     = 0;
	self->sfi_width          = STRING_WIDTH_1BYTE;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_copy(StringFindIterator *__restrict self,
         StringFindIterator *__restrict other) {
	self->sfi_find           = other->sfi_find;
	self->sfi_start.ptr      = other->sfi_start.ptr;
	self->sfi_ptr.ptr        = READ_PTR(other);
	self->sfi_end.ptr        = other->sfi_end.ptr;
	self->sfi_needle_ptr.ptr = other->sfi_needle_ptr.ptr;
	self->sfi_needle_len     = other->sfi_needle_len;
	self->sfi_width          = other->sfi_width;
	Dee_Incref(self->sfi_find);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_setup(StringFindIterator *__restrict self,
          StringFind *__restrict find) {
	size_t my_start, my_end;
	self->sfi_width = STRING_WIDTH_COMMON(DeeString_WIDTH(find->sf_str),
	                                      DeeString_WIDTH(find->sf_needle));
	my_start        = find->sf_start;
	my_end          = find->sf_end;
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		self->sfi_ptr.cp8   = DeeString_As1Byte((DeeObject *)find->sf_str);
		self->sfi_start.cp8 = self->sfi_ptr.cp8;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp8))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp8);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp8 = self->sfi_ptr.cp8 + my_end;
		self->sfi_ptr.cp8 += my_start;
		self->sfi_needle_ptr.cp8 = DeeString_As1Byte((DeeObject *)find->sf_needle);
		self->sfi_needle_len     = WSTR_LENGTH(self->sfi_needle_ptr.cp8);
		break;

	CASE_WIDTH_2BYTE:
		self->sfi_ptr.cp16 = DeeString_As2Byte((DeeObject *)find->sf_str);
		if unlikely(!self->sfi_ptr.cp16)
			goto err;
		self->sfi_start.cp16 = self->sfi_ptr.cp16;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp16))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp16);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp16 = self->sfi_ptr.cp16 + my_end;
		self->sfi_ptr.cp16 += my_start;
		self->sfi_needle_ptr.cp16 = DeeString_As2Byte((DeeObject *)find->sf_needle);
		if unlikely(!self->sfi_needle_ptr.cp16)
			goto err;
		self->sfi_needle_len = WSTR_LENGTH(self->sfi_needle_ptr.cp16);
		break;

	CASE_WIDTH_4BYTE:
		self->sfi_ptr.cp32 = DeeString_As4Byte((DeeObject *)find->sf_str);
		if unlikely(!self->sfi_ptr.cp32)
			goto err;
		self->sfi_start.cp32 = self->sfi_ptr.cp32;
		if (my_end > WSTR_LENGTH(self->sfi_ptr.cp32))
			my_end = WSTR_LENGTH(self->sfi_ptr.cp32);
		if (my_start > my_end)
			my_start = my_end;
		self->sfi_end.cp32 = self->sfi_ptr.cp32 + my_end;
		self->sfi_ptr.cp32 += my_start;
		self->sfi_needle_ptr.cp32 = DeeString_As4Byte((DeeObject *)find->sf_needle);
		if unlikely(!self->sfi_needle_ptr.cp32)
			goto err;
		self->sfi_needle_len = WSTR_LENGTH(self->sfi_needle_ptr.cp32);
		break;
	}
	self->sfi_find = find;
	Dee_Incref(find);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_init(StringFindIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	StringFind *find;
	if (DeeArg_Unpack(argc, argv, "o:_StringFindIterator", &find))
		goto err;
	if (DeeObject_AssertTypeExact(find, &StringFind_Type))
		goto err;
	return sfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_init(StringFindIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	StringFind *find;
	if (DeeArg_Unpack(argc, argv, "o:_StringCaseFindIterator", &find))
		goto err;
	if (DeeObject_AssertTypeExact(find, &StringCaseFind_Type))
		goto err;
	return sfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_next(StringFindIterator *__restrict self) {
	union dcharptr ptr, new_ptr;
again:
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		new_ptr.cp8 = memmemb(ptr.cp8, (size_t)(self->sfi_end.cp8 - ptr.cp8),
		                      self->sfi_needle_ptr.cp8,
		                      self->sfi_needle_len);
		if (new_ptr.cp8) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp8, ptr.cp8, new_ptr.cp8 + self->sfi_needle_len))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp8 - self->sfi_start.cp8));
		}
		break;

	CASE_WIDTH_2BYTE:
		new_ptr.cp16 = memmemw(ptr.cp16, (size_t)(self->sfi_end.cp16 - ptr.cp16),
		                       self->sfi_needle_ptr.cp16,
		                       self->sfi_needle_len);
		if (new_ptr.cp16) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp16, ptr.cp16, new_ptr.cp16 + self->sfi_needle_len))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp16 - self->sfi_start.cp16));
		}
		break;

	CASE_WIDTH_4BYTE:
		new_ptr.cp32 = memmeml(ptr.cp32, (size_t)(self->sfi_end.cp32 - ptr.cp32),
		                       self->sfi_needle_ptr.cp32,
		                       self->sfi_needle_len);
		if (new_ptr.cp32) {
			if (!atomic_cmpxch_weak(&self->sfi_ptr.cp32, ptr.cp32, new_ptr.cp32 + self->sfi_needle_len))
				goto again;
			return DeeInt_NewSize((size_t)(new_ptr.cp32 - self->sfi_start.cp32));
		}
		break;
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
scfi_next(StringFindIterator *__restrict self) {
	union dcharptr ptr, new_ptr;
	size_t match_length, result;
again:
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		new_ptr.cp8 = memcasememb(ptr.cp8, (size_t)(self->sfi_end.cp8 - ptr.cp8),
		                          self->sfi_needle_ptr.cp8,
		                          self->sfi_needle_len,
		                          &match_length);
		if (!new_ptr.cp8)
			goto iter_done;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp8, ptr.cp8, new_ptr.cp8 + match_length))
			goto again;
		result = (size_t)(new_ptr.cp8 - self->sfi_start.cp8);
		break;

	CASE_WIDTH_2BYTE:
		new_ptr.cp16 = memcasememw(ptr.cp16, (size_t)(self->sfi_end.cp16 - ptr.cp16),
		                           self->sfi_needle_ptr.cp16,
		                           self->sfi_needle_len,
		                           &match_length);
		if (!new_ptr.cp16)
			goto iter_done;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp16, ptr.cp16, new_ptr.cp16 + match_length))
			goto again;
		result = (size_t)(new_ptr.cp16 - self->sfi_start.cp16);
		break;

	CASE_WIDTH_4BYTE:
		new_ptr.cp32 = memcasememl(ptr.cp32, (size_t)(self->sfi_end.cp32 - ptr.cp32),
		                           self->sfi_needle_ptr.cp32,
		                           self->sfi_needle_len,
		                           &match_length);
		if (!new_ptr.cp32)
			goto iter_done;
		if (!atomic_cmpxch_weak(&self->sfi_ptr.cp32, ptr.cp32, new_ptr.cp32 + match_length))
			goto again;
		result = (size_t)(new_ptr.cp32 - self->sfi_start.cp32);
		break;
	}
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + match_length);
iter_done:
	return ITER_DONE;
}

PRIVATE NONNULL((1)) void DCALL
sfi_fini(StringFindIterator *__restrict self) {
	Dee_Decref(self->sfi_find);
}

PRIVATE NONNULL((1, 2)) void DCALL
sfi_visit(StringFindIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sfi_find);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sfi_bool(StringFindIterator *__restrict self) {
	union dcharptr ptr;
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = memmemb(ptr.cp8, (size_t)(self->sfi_end.cp8 - ptr.cp8),
		                  self->sfi_needle_ptr.cp8,
		                  self->sfi_needle_len);
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = memmemw(ptr.cp16, (size_t)(self->sfi_end.cp16 - ptr.cp16),
		                   self->sfi_needle_ptr.cp16,
		                   self->sfi_needle_len);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = memmeml(ptr.cp32, (size_t)(self->sfi_end.cp32 - ptr.cp32),
		                   self->sfi_needle_ptr.cp32,
		                   self->sfi_needle_len);
		break;
	}
	return ptr.ptr != NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
scfi_bool(StringFindIterator *__restrict self) {
	union dcharptr ptr;
	ptr.ptr = atomic_read(&self->sfi_ptr.ptr);
	SWITCH_SIZEOF_WIDTH(self->sfi_width) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = memcasememb(ptr.cp8, (size_t)(self->sfi_end.cp8 - ptr.cp8),
		                      self->sfi_needle_ptr.cp8,
		                      self->sfi_needle_len, NULL);
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = memcasememw(ptr.cp16, (size_t)(self->sfi_end.cp16 - ptr.cp16),
		                       self->sfi_needle_ptr.cp16,
		                       self->sfi_needle_len, NULL);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = memcasememl(ptr.cp32, (size_t)(self->sfi_end.cp32 - ptr.cp32),
		                       self->sfi_needle_ptr.cp32,
		                       self->sfi_needle_len, NULL);
		break;
	}
	return ptr.ptr != NULL;
}

PRIVATE struct type_member tpconst sfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(StringFindIterator, sfi_find),
	                      "->?Ert:StringFind"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst scfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT,
	                      offsetof(StringFindIterator, sfi_find),
	                      "->?Ert:StringCaseFind"),
	TYPE_MEMBER_END
};


#define DEFINE_STRINGFINDITERATOR_COMPARE(name, op)             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL       \
	name(StringFindIterator *self, StringFindIterator *other) { \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))   \
			goto err;                                           \
		return_bool(READ_PTR(self) op READ_PTR(other));         \
	err:                                                        \
		return NULL;                                            \
	}
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_eq, ==)
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_ne, !=)
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_lo, <)
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_le, <=)
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_gr, >)
DEFINE_STRINGFINDITERATOR_COMPARE(sfi_ge, >=)
#undef DEFINE_STRINGFINDITERATOR_COMPARE


PRIVATE struct type_cmp sfi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sfi_ge,
};


INTERN DeeTypeObject StringFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringFindIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&sfi_init,
				TYPE_FIXED_ALLOCATOR(StringFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sfi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject StringCaseFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseFindIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&scfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&scfi_init,
				TYPE_FIXED_ALLOCATOR(StringFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&scfi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scfi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ scfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#undef READ_PTR





PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_ctor(StringFind *__restrict self) {
	self->sf_str    = (DREF String *)Dee_EmptyString;
	self->sf_needle = (DREF String *)Dee_EmptyString;
	self->sf_start  = 0;
	self->sf_end    = 0;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_init(StringFind *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->sf_start = 0;
	self->sf_end   = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":_StringFind",
	                  &self->sf_str, &self->sf_needle,
	                  &self->sf_start, &self->sf_end))
		goto err;
	if (DeeObject_AssertTypeExact(self->sf_str, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(self->sf_needle, &DeeString_Type))
		goto err;
	Dee_Incref(self->sf_str);
	Dee_Incref(self->sf_needle);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sf_fini(StringFind *__restrict self) {
	Dee_Decref(self->sf_str);
	Dee_Decref(self->sf_needle);
}

PRIVATE NONNULL((1, 2)) void DCALL
sf_visit(StringFind *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sf_str);
	Dee_Visit(self->sf_needle);
}

PRIVATE WUNUSED NONNULL((1)) DREF StringFindIterator *DCALL
sf_iter(StringFind *__restrict self) {
	DREF StringFindIterator *result;
	result = DeeObject_MALLOC(StringFindIterator);
	if unlikely(!result)
		goto done;
	if (sfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &StringFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF StringFindIterator *DCALL
scf_iter(StringFind *__restrict self) {
	DREF StringFindIterator *result;
	result = DeeObject_MALLOC(StringFindIterator);
	if unlikely(!result)
		goto done;
	if (sfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &StringCaseFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE struct type_seq sf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sf_iter,
	/* .tp_sizeob   = */ NULL, /* TODO: string.count() */
	/* .tp_contains = */ NULL, /* TODO: string.substr() == needle */
};

PRIVATE struct type_seq scf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&scf_iter,
	/* .tp_sizeob   = */ NULL, /* TODO: string.casecount() */
	/* .tp_contains = */ NULL, /* TODO: string.substr(...).casecompare(needle) == 0 */
};


PRIVATE struct type_member tpconst sf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringFind, sf_str), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__needle__", STRUCT_OBJECT, offsetof(StringFind, sf_needle), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringFind, sf_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(StringFind, sf_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringFindIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject StringFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringFind",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&sf_init,
				TYPE_FIXED_ALLOCATOR(StringFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL  /* TODO: string.contains() */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sf_class_members
};

INTERN DeeTypeObject StringCaseFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringCaseFind",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&sf_init,
				TYPE_FIXED_ALLOCATOR(StringFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL  /* TODO: string.casecontains() */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &scf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sf_class_members
};


INTERN WUNUSED DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end) {
	DREF StringFind *result;
	result = DeeObject_MALLOC(StringFind);
	if unlikely(!result)
		goto done;
	result->sf_str    = self;
	result->sf_needle = other;
	result->sf_start  = start;
	result->sf_end    = end;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &StringFind_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end) {
	DREF StringFind *result;
	result = DeeObject_MALLOC(StringFind);
	if unlikely(!result)
		goto done;
	result->sf_str    = self;
	result->sf_needle = other;
	result->sf_start  = start;
	result->sf_end    = end;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &StringCaseFind_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_ISINDER_C_INL */
