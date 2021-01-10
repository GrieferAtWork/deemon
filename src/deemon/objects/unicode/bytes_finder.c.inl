/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/seq.h>

#include <hybrid/atomic.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF Bytes     *bf_bytes;  /* [1..1][const] The bytes that is being searched. */
	DREF DeeObject *bf_other;  /* [1..1][const] The needle object. */
	Needle          bf_needle; /* [const] The needle being searched for. */
	uint8_t        *bf_start;  /* [1..1][const] Starting pointer. */
	uint8_t        *bf_end;    /* [1..1][const] End pointer. */
} BytesFind;

typedef struct {
	OBJECT_HEAD
	DREF BytesFind *bfi_find;       /* [1..1][const] The underlying find-controller. */
	uint8_t        *bfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK uint8_t  *bfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	uint8_t        *bfi_end;        /* [1..1][const] End pointer. */
	uint8_t        *bfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t          bfi_needle_len; /* [const] Length of the needle being searched. */
} BytesFindIterator;

INTDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *other,
                 size_t start, size_t end);
INTDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *other,
                     size_t start, size_t end);

#ifdef CONFIG_NO_THREADS
#define READ_PTR(x)            ((x)->bfi_ptr)
#else /* CONFIG_NO_THREADS */
#define READ_PTR(x) ATOMIC_READ((x)->bfi_ptr)
#endif /* !CONFIG_NO_THREADS */


INTDEF DeeTypeObject BytesFindIterator_Type;
INTDEF DeeTypeObject BytesFind_Type;
INTDEF DeeTypeObject BytesCaseFindIterator_Type;
INTDEF DeeTypeObject BytesCaseFind_Type;


PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_FindAll((Bytes *)Dee_EmptyBytes,
	                                                    Dee_EmptyBytes, 0, 0);
	if unlikely(!self->bfi_find)
		return -1;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_CaseFindAll((Bytes *)Dee_EmptyBytes,
	                                                        Dee_EmptyBytes, 0, 0);
	if unlikely(!self->bfi_find)
		return -1;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bfi_copy(BytesFindIterator *__restrict self,
         BytesFindIterator *__restrict other) {
	self->bfi_find       = other->bfi_find;
	self->bfi_start      = other->bfi_start;
	self->bfi_ptr        = READ_PTR(other);
	self->bfi_end        = other->bfi_end;
	self->bfi_needle_ptr = other->bfi_needle_ptr;
	self->bfi_needle_len = other->bfi_needle_len;
	Dee_Incref(self->bfi_find);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bfi_setup(BytesFindIterator *__restrict self,
          BytesFind *__restrict find) {
	self->bfi_find       = find;
	self->bfi_start      = find->bf_start;
	self->bfi_ptr        = find->bf_start;
	self->bfi_end        = find->bf_end;
	self->bfi_needle_ptr = find->bf_needle.n_data;
	self->bfi_needle_len = find->bf_needle.n_size;
	Dee_Incref(find);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_init(BytesFindIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	BytesFind *find;
	if (DeeArg_Unpack(argc, argv, "o:_BytesFindIterator", &find))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)find, &BytesFind_Type))
		goto err;
	return bfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_init(BytesFindIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	BytesFind *find;
	if (DeeArg_Unpack(argc, argv, "o:_BytesCaseFindIterator", &find))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)find, &BytesCaseFind_Type))
		goto err;
	return bfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bfi_next(BytesFindIterator *__restrict self) {
	uint8_t *ptr, *new_ptr;
again:
	ptr     = ATOMIC_READ(self->bfi_ptr);
	new_ptr = (uint8_t *)memmemb(ptr, (size_t)(self->bfi_end - ptr),
	                             self->bfi_needle_ptr,
	                             self->bfi_needle_len);
	if (new_ptr) {
		if (!ATOMIC_CMPXCH_WEAK(self->bfi_ptr, ptr, new_ptr + self->bfi_needle_len))
			goto again;
		return DeeInt_NewSize((size_t)(new_ptr - self->bfi_start));
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bcfi_next(BytesFindIterator *__restrict self) {
	uint8_t *ptr, *new_ptr;
again:
	ptr     = ATOMIC_READ(self->bfi_ptr);
	new_ptr = (uint8_t *)dee_memasciicasemem(ptr, (size_t)(self->bfi_end - ptr),
	                                         self->bfi_needle_ptr,
	                                         self->bfi_needle_len);
	if (new_ptr) {
		size_t result;
		if (!ATOMIC_CMPXCH_WEAK(self->bfi_ptr, ptr, new_ptr + self->bfi_needle_len))
			goto again;
		result = (size_t)(new_ptr - self->bfi_start);
		return DeeTuple_Newf(DEE_FMT_SIZE_T
		                     DEE_FMT_SIZE_T,
		                     result,
		                     result + self->bfi_needle_len);
	}
	return ITER_DONE;
}

PRIVATE NONNULL((1)) void DCALL
bfi_fini(BytesFindIterator *__restrict self) {
	Dee_Decref(self->bfi_find);
}

PRIVATE NONNULL((1, 2)) void DCALL
bfi_visit(BytesFindIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bfi_find);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_bool(BytesFindIterator *__restrict self) {
	uint8_t *ptr;
	ptr = ATOMIC_READ(self->bfi_ptr);
	ptr = memmemb(ptr, (size_t)(self->bfi_end - ptr),
	              self->bfi_needle_ptr,
	              self->bfi_needle_len);
	return ptr != NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_bool(BytesFindIterator *__restrict self) {
	uint8_t *ptr;
	ptr = ATOMIC_READ(self->bfi_ptr);
	ptr = dee_memasciicasemem(ptr, (size_t)(self->bfi_end - ptr),
	                          self->bfi_needle_ptr,
	                          self->bfi_needle_len);
	return ptr != NULL;
}

PRIVATE struct type_member bfi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BytesFindIterator, bfi_find), "->?Ert:BytesFind"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define bcfi_members bfi_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member bcfi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BytesFindIterator, bfi_find), "->?Ert:BytesCaseFind"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */


#define DEFINE_STRINGSEGMENTSITERATOR_COMPARE(name, op)                    \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                  \
	name(BytesFindIterator *self, BytesFindIterator *other) {              \
		if (DeeObject_AssertTypeExact((DeeObject *)other, Dee_TYPE(self))) \
			return NULL;                                                   \
		return_bool(READ_PTR(self) op READ_PTR(other));                    \
	}
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_eq, ==)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_ne, !=)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_lo, <)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_le, <=)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_gr, >)
DEFINE_STRINGSEGMENTSITERATOR_COMPARE(bfi_ge, >=)
#undef DEFINE_STRINGSEGMENTSITERATOR_COMPARE


PRIVATE struct type_cmp bfi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bfi_ge,
};


INTERN DeeTypeObject BytesFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesFindIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bfi_ctor,
				/* .tp_copy_ctor = */ &bfi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bfi_init,
				TYPE_FIXED_ALLOCATOR(BytesFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bfi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bfi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject BytesCaseFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFindIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bcfi_ctor,
				/* .tp_copy_ctor = */ &bfi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bcfi_init,
				TYPE_FIXED_ALLOCATOR(BytesFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bcfi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcfi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bcfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#undef READ_PTR





PRIVATE WUNUSED NONNULL((1)) int DCALL
bf_ctor(BytesFind *__restrict self) {
	self->bf_bytes         = (DREF Bytes *)Dee_EmptyBytes;
	self->bf_other         = Dee_EmptyBytes;
	self->bf_start         = DeeBytes_DATA(Dee_EmptyBytes);
	self->bf_end           = self->bf_start;
	self->bf_needle.n_data = self->bf_start;
	self->bf_needle.n_size = 0;
	Dee_Incref_n(Dee_EmptyBytes, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bf_init(BytesFind *__restrict self,
        size_t argc, DeeObject *const *argv) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:_BytesFind",
	                  &self->bf_bytes, &self->bf_other,
	                  &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)self->bf_bytes, &DeeBytes_Type) ||
	    get_needle(&self->bf_needle, self->bf_other))
		goto err;
	if (end > DeeBytes_SIZE(self->bf_bytes))
		end = DeeBytes_SIZE(self->bf_bytes);
	if (start > end)
		start = end;
	self->bf_start = DeeBytes_DATA(self->bf_bytes) + start;
	self->bf_end   = DeeBytes_DATA(self->bf_bytes) + end;
	Dee_Incref(self->bf_bytes);
	Dee_Incref(self->bf_other);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
bf_fini(BytesFind *__restrict self) {
	Dee_Decref(self->bf_bytes);
	Dee_Decref(self->bf_other);
}

PRIVATE NONNULL((1, 2)) void DCALL
bf_visit(BytesFind *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bf_bytes);
	Dee_Visit(self->bf_other);
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesFindIterator *DCALL
bf_iter(BytesFind *__restrict self) {
	DREF BytesFindIterator *result;
	result = DeeObject_MALLOC(BytesFindIterator);
	if unlikely(!result)
		goto done;
	if (bfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &BytesFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesFindIterator *DCALL
bcf_iter(BytesFind *__restrict self) {
	DREF BytesFindIterator *result;
	result = DeeObject_MALLOC(BytesFindIterator);
	if unlikely(!result)
		goto done;
	if (bfi_setup(result, self))
		goto err_r;
	DeeObject_Init(result, &BytesCaseFindIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE struct type_seq bf_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bf_iter,
	/* .tp_size      = */ NULL, /* TODO: bytes.count() */
	/* .tp_contains  = */ NULL, /* TODO: bytes.substr() == needle */
};

PRIVATE struct type_seq bcf_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcf_iter,
	/* .tp_size      = */ NULL, /* TODO: bytes.casecount() */
	/* .tp_contains  = */ NULL, /* TODO: bytes.substr(...).casecompare(needle) == 0 */
};


PRIVATE struct type_member bf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesFind, bf_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__needle__", STRUCT_OBJECT, offsetof(BytesFind, bf_needle), "->?DBytes"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesFind, bf_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesFind, bf_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member bf_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BytesFindIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesFind",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bf_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bf_init,
				TYPE_FIXED_ALLOCATOR(BytesFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL  /* TODO: bytes.contains() */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bf_class_members
};

INTERN DeeTypeObject BytesCaseFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFind",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bf_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bf_init,
				TYPE_FIXED_ALLOCATOR(BytesFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL  /* TODO: bytes.contains() */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bcf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bf_class_members
};


INTERN WUNUSED DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *other,
                 size_t start, size_t end) {
	DREF BytesFind *result;
	result = DeeObject_MALLOC(BytesFind);
	if unlikely(!result)
		goto done;
	if (get_needle(&result->bf_needle, other))
		goto err_r;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_empty_seq;
	result->bf_bytes = self;
	result->bf_other = other;
	result->bf_start = DeeBytes_DATA(self) + start;
	result->bf_end   = DeeBytes_DATA(self) + end;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &BytesFind_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *other,
                     size_t start, size_t end) {
	DREF BytesFind *result;
	result = DeeObject_MALLOC(BytesFind);
	if unlikely(!result)
		goto done;
	if (get_needle(&result->bf_needle, other))
		goto err_r;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_empty_seq;
	result->bf_bytes = self;
	result->bf_other = other;
	result->bf_start = DeeBytes_DATA(self) + start;
	result->bf_end   = DeeBytes_DATA(self) + end;
	Dee_Incref(self);
	Dee_Incref(other);
	DeeObject_Init(result, &BytesCaseFind_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL */
