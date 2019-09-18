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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include "../../runtime/strings.h"

DECL_BEGIN

INTDEF DeeTypeObject BytesSplitIterator_Type;
INTDEF DeeTypeObject BytesSplit_Type;
INTDEF DeeTypeObject BytesCaseSplitIterator_Type;
INTDEF DeeTypeObject BytesCaseSplit_Type;
INTDEF DeeTypeObject BytesLineSplitIterator_Type;
INTDEF DeeTypeObject BytesLineSplit_Type;

typedef struct {
	OBJECT_HEAD
	DREF Bytes          *bs_bytes;     /* [1..1][const] The Bytes object being split. */
	DREF DeeObject      *bs_sep_owner; /* [0..1][const] The owner of the split sequence. */
	uint8_t             *bs_sep_ptr;   /* [const] Pointer to the effective separation sequence. */
	size_t               bs_sep_len;   /* [const] Length of the separation sequence (in bytes). */
	uint8_t              bs_sep_buf[sizeof(void *)]; /* A small inline-buffer used for single-byte splits. */
} BytesSplit;

typedef struct {
	OBJECT_HEAD
	DREF BytesSplit     *bsi_split;    /* [1..1][const] The underlying split controller. */
	ATOMIC_DATA uint8_t *bsi_iter;     /* [0..1] Pointer to the start of the next split (When NULL, iteration is complete). */
	uint8_t             *bsi_end;      /* [1..1][== DeeBytes_TERM(bsi_split->bs_bytes)] Pointer to the end of input data. */
	Bytes               *bsi_bytes;    /* [1..1][const][== bsi_split] The Bytes object being split. */
	uint8_t             *bsi_sep_ptr;  /* [const][== bsi_split->bs_sep_ptr] Pointer to the effective separation sequence. */
	size_t               bsi_sep_len;  /* [const][== bsi_split->bs_sep_len] Length of the separation sequence (in bytes). */
} BytesSplitIterator;

#ifndef CONFIG_NO_THREADS
#define READ_BSI_ITER(x) ATOMIC_READ((x)->bsi_iter)
#else /* !CONFIG_NO_THREADS */
#define READ_BSI_ITER(x)            ((x)->bsi_iter)
#endif /* CONFIG_NO_THREADS */

PRIVATE int DCALL
bsi_init(BytesSplitIterator *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, "o:_BytesSplitIterator",
	                  &self->bsi_split))
		goto err;
	if (Dee_TYPE(self) == &BytesSplitIterator_Type) {
		if (DeeObject_AssertTypeExact((DeeObject *)self->bsi_split, &BytesSplit_Type))
			goto err;
	} else {
		if (DeeObject_AssertTypeExact((DeeObject *)self->bsi_split, &BytesCaseSplit_Type))
			goto err;
	}
	self->bsi_bytes = self->bsi_split->bs_bytes;
	self->bsi_iter  = DeeBytes_DATA(self->bsi_bytes);
	self->bsi_end   = self->bsi_iter + DeeBytes_SIZE(self->bsi_bytes);
	if (self->bsi_iter == self->bsi_end)
		self->bsi_iter = NULL;
	self->bsi_sep_ptr = self->bsi_split->bs_sep_ptr;
	self->bsi_sep_len = self->bsi_split->bs_sep_len;
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
bsi_copy(BytesSplitIterator *__restrict self,
         BytesSplitIterator *__restrict other) {
	self->bsi_split   = other->bsi_split;
	self->bsi_bytes   = other->bsi_bytes;
	self->bsi_iter    = READ_BSI_ITER(other);
	self->bsi_end     = other->bsi_end;
	self->bsi_sep_ptr = other->bsi_sep_ptr;
	self->bsi_sep_len = other->bsi_sep_len;
	Dee_Incref(self->bsi_split);
	return 0;
}

PRIVATE void DCALL
bsi_fini(BytesSplitIterator *__restrict self) {
	Dee_Decref(self->bsi_split);
}

PRIVATE void DCALL
bsi_visit(BytesSplitIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bsi_split);
}

PRIVATE int DCALL
bsi_bool(BytesSplitIterator *__restrict self) {
	return READ_BSI_ITER(self) != NULL;
}


#define DEFINE_BSI_COMPARE(name, op)                                       \
	PRIVATE DREF DeeObject *DCALL                                          \
	name(BytesSplitIterator *__restrict self,                              \
	     BytesSplitIterator *__restrict other) {                           \
		uint8_t *x, *y;                                                    \
		if (DeeObject_AssertTypeExact((DeeObject *)other, Dee_TYPE(self))) \
			goto err;                                                      \
		x = READ_BSI_ITER(self);                                           \
		y = READ_BSI_ITER(other);                                          \
		if (!x)                                                            \
			x = (uint8_t *)-1;                                             \
		if (!y)                                                            \
			y = (uint8_t *)-1;                                             \
		return_bool(x op y);                                               \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_BSI_COMPARE(bsi_eq, ==)
DEFINE_BSI_COMPARE(bsi_ne, !=)
DEFINE_BSI_COMPARE(bsi_lo, <)
DEFINE_BSI_COMPARE(bsi_le, <=)
DEFINE_BSI_COMPARE(bsi_gr, >)
DEFINE_BSI_COMPARE(bsi_ge, >=)
#undef DEFINE_BSI_COMPARE

PRIVATE struct type_cmp bsi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&bsi_ge,
};



PRIVATE DREF DeeObject *DCALL
bsi_next(BytesSplitIterator *__restrict self) {
	uint8_t *start, *end;
#ifdef CONFIG_NO_THREADS
	start = self->bsi_iter;
	if (!start)
		return ITER_DONE;
	end = (uint8_t *)memmemb(start,
	                         self->bsi_end - start,
	                         self->bsi_sep_ptr,
	                         self->bsi_sep_len);
	if (!end) {
		self->bsi_iter = NULL;
		return DeeBytes_NewView(self->bsi_bytes->b_orig,
		                        start, (size_t)(self->bsi_end - start),
		                        self->bsi_bytes->b_flags);
	}
	self->bsi_iter = end + self->bsi_sep_len;
#else  /* CONFIG_NO_THREADS */
	for (;;) {
		start = ATOMIC_READ(self->bsi_iter);
		if (!start)
			return ITER_DONE;
		end = (uint8_t *)memmemb(start,
		                         self->bsi_end - start,
		                         self->bsi_sep_ptr,
		                         self->bsi_sep_len);
		if (!end) {
			if (!ATOMIC_CMPXCH_WEAK(self->bsi_iter, start, NULL))
				continue;
			return DeeBytes_NewView(self->bsi_bytes->b_orig,
			                        start, (size_t)(self->bsi_end - start),
			                        self->bsi_bytes->b_flags);
		}
		if (ATOMIC_CMPXCH_WEAK(self->bsi_iter, start, end + self->bsi_sep_len))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return DeeBytes_NewView(self->bsi_bytes->b_orig,
	                        start, (size_t)(end - start),
	                        self->bsi_bytes->b_flags);
}

PRIVATE DREF DeeObject *DCALL
bsci_next(BytesSplitIterator *__restrict self) {
	uint8_t *start, *end;
#ifdef CONFIG_NO_THREADS
	start = self->bsi_iter;
	if (!start)
		return ITER_DONE;
	end = (uint8_t *)dee_memasciicasemem(start,
	                                     self->bsi_end - start,
	                                     self->bsi_sep_ptr,
	                                     self->bsi_sep_len);
	if (!end) {
		self->bsi_iter = NULL;
		return DeeBytes_NewView(self->bsi_bytes->b_orig,
		                        start, (size_t)(self->bsi_end - start),
		                        self->bsi_bytes->b_flags);
	}
	self->bsi_iter = end + self->bsi_sep_len;
#else  /* CONFIG_NO_THREADS */
	for (;;) {
		start = ATOMIC_READ(self->bsi_iter);
		if (!start)
			return ITER_DONE;
		end = (uint8_t *)dee_memasciicasemem(start,
		                                     self->bsi_end - start,
		                                     self->bsi_sep_ptr,
		                                     self->bsi_sep_len);
		if (!end) {
			if (!ATOMIC_CMPXCH_WEAK(self->bsi_iter, start, NULL))
				continue;
			return DeeBytes_NewView(self->bsi_bytes->b_orig,
			                        start, (size_t)(self->bsi_end - start),
			                        self->bsi_bytes->b_flags);
		}
		if (ATOMIC_CMPXCH_WEAK(self->bsi_iter, start, end + self->bsi_sep_len))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return DeeBytes_NewView(self->bsi_bytes->b_orig,
	                        start, (size_t)(end - start),
	                        self->bsi_bytes->b_flags);
}

PRIVATE struct type_member bsi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_split), "->?Ert:BytesSplit"),
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_bytes), "->?DBytes"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define bcsi_members bsi_members
#else  /* CONFIG_NO_DOC */
PRIVATE struct type_member bcsi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_split), "->?Ert:BytesCaseSplit"),
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_bytes), "->?DBytes"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */


INTERN DeeTypeObject BytesSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSplitIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &bsi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bsi_init,
				TYPE_FIXED_ALLOCATOR(BytesSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&bsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&bsi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /*  No visit, because it only ever references strings
                                    * (or rather an object that can only reference strings). */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bsi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject BytesCaseSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseSplitIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &bsi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bsi_init,
				TYPE_FIXED_ALLOCATOR(BytesSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&bsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&bsi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bsi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsci_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bcsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE int DCALL
bs_ctor(BytesSplit *__restrict self) {
	self->bs_bytes     = (DREF Bytes *)Dee_EmptyBytes;
	self->bs_sep_owner = NULL;
	self->bs_sep_ptr   = NULL;
	self->bs_sep_len   = 0;
	Dee_Incref(Dee_EmptyBytes);
	return 0;
}

PRIVATE int DCALL
bs_init(BytesSplit *__restrict self, size_t argc,
        DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_BytesSplit", &self->bs_bytes, &self->bs_sep_owner))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)self->bs_bytes, &DeeBytes_Type))
		goto err;
	if (DeeBytes_Check(self->bs_sep_owner)) {
		self->bs_sep_ptr = DeeBytes_DATA(self->bs_sep_owner);
		self->bs_sep_len = DeeBytes_SIZE(self->bs_sep_owner);
		Dee_Incref(self->bs_sep_owner);
	} else if (DeeString_Check(self->bs_sep_owner)) {
		self->bs_sep_ptr = DeeString_AsBytes(self->bs_sep_owner, false);
		if
			unlikely(!self->bs_sep_ptr)
		goto err;
		self->bs_sep_len = WSTR_LENGTH(self->bs_sep_ptr);
		Dee_Incref(self->bs_sep_owner);
	} else {
		if (DeeObject_AsUInt8(self->bs_sep_owner, &self->bs_sep_buf[0]))
			goto err;
		self->bs_sep_owner = NULL;
		self->bs_sep_ptr   = self->bs_sep_buf;
		self->bs_sep_len   = 1;
	}
	Dee_Incref(self->bs_bytes);
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
bs_fini(BytesSplit *__restrict self) {
	Dee_Decref(self->bs_bytes);
	Dee_XDecref(self->bs_sep_owner);
}

PRIVATE void DCALL
bs_visit(BytesSplit *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bs_bytes);
	Dee_XVisit(self->bs_sep_owner);
}

PRIVATE int DCALL
bs_bool(BytesSplit *__restrict self) {
	return DeeBytes_SIZE(self->bs_bytes) != 0;
}

PRIVATE DREF BytesSplitIterator *DCALL
bs_iter(BytesSplit *__restrict self) {
	DREF BytesSplitIterator *result;
	result = DeeObject_MALLOC(BytesSplitIterator);
	if
		unlikely(!result)
	goto done;
	result->bsi_split = self;
	result->bsi_bytes = self->bs_bytes;
	result->bsi_iter  = DeeBytes_DATA(self->bs_bytes);
	result->bsi_end   = result->bsi_iter + DeeBytes_SIZE(self->bs_bytes);
	if (result->bsi_iter == result->bsi_end)
		result->bsi_iter = NULL;
	result->bsi_sep_ptr = self->bs_sep_ptr;
	result->bsi_sep_len = self->bs_sep_len;
	Dee_Incref(self);
	DeeObject_Init(result, &BytesSplitIterator_Type);
done:
	return result;
}

PRIVATE DREF BytesSplitIterator *DCALL
bcs_iter(BytesSplit *__restrict self) {
	DREF BytesSplitIterator *result;
	result = DeeObject_MALLOC(BytesSplitIterator);
	if
		unlikely(!result)
	goto done;
	result->bsi_split = self;
	result->bsi_bytes = self->bs_bytes;
	result->bsi_iter  = DeeBytes_DATA(self->bs_bytes);
	result->bsi_end   = result->bsi_iter + DeeBytes_SIZE(self->bs_bytes);
	if (result->bsi_iter == result->bsi_end)
		result->bsi_iter = NULL;
	result->bsi_sep_ptr = self->bs_sep_ptr;
	result->bsi_sep_len = self->bs_sep_len;
	Dee_Incref(self);
	DeeObject_Init(result, &BytesCaseSplitIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq bs_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bs_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_seq bcs_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcs_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE DREF DeeObject *DCALL
bs_getsep(BytesSplit *__restrict self) {
	return DeeBytes_NewView(self->bs_sep_owner ? self->bs_sep_owner : (DeeObject *)self,
	                        self->bs_sep_ptr, self->bs_sep_len, Dee_BUFFER_FREADONLY);
}

PRIVATE struct type_getset bs_getsets[] = {
	{ "__sep__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & bs_getsep, NULL, NULL,
	  DOC("->?DBytes") },
	{ NULL }
};

PRIVATE struct type_member bs_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplit, bs_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__sep_owner__", STRUCT_OBJECT, offsetof(BytesSplit, bs_sep_owner)),
	TYPE_MEMBER_FIELD("__sep_length__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BytesSplit, bs_sep_len)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member bs_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BytesSplitIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member bcs_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BytesCaseSplitIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bs_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bs_init,
				TYPE_FIXED_ALLOCATOR(BytesSplit)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&bs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&bs_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bs_class_members
};

INTERN DeeTypeObject BytesCaseSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bs_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bs_init,
				TYPE_FIXED_ALLOCATOR(BytesSplit)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&bs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&bs_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bcs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bcs_class_members
};

INTERN DREF DeeObject *DCALL
DeeBytes_SplitByte(Bytes *__restrict self,
                   uint8_t sep) {
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if
		unlikely(!result)
	goto done;
	result->bs_bytes      = self;
	result->bs_sep_owner  = NULL;
	result->bs_sep_ptr    = result->bs_sep_buf;
	result->bs_sep_len    = 1;
	result->bs_sep_buf[0] = sep;
	Dee_Incref(self);
	DeeObject_Init(result, &BytesSplit_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
DeeBytes_Split(Bytes *__restrict self,
               DeeObject *__restrict sep) {
	ASSERT_OBJECT(sep);
	ASSERT(DeeString_Check(sep) || DeeBytes_Check(sep));
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if
		unlikely(!result)
	goto done;
	if (DeeString_Check(sep)) {
		result->bs_sep_ptr = DeeString_AsBytes(sep, false);
		if
			unlikely(!result->bs_sep_ptr)
		goto err_r;
		result->bs_sep_len = WSTR_LENGTH(result->bs_sep_ptr);
	} else {
		result->bs_sep_ptr = DeeBytes_DATA(sep);
		result->bs_sep_len = DeeBytes_SIZE(sep);
	}
	result->bs_bytes     = self;
	result->bs_sep_owner = sep;
	Dee_Incref(self);
	Dee_Incref(sep);
	DeeObject_Init(result, &BytesSplit_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeBytes_CaseSplitByte(Bytes *__restrict self,
                       uint8_t sep) {
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if
		unlikely(!result)
	goto done;
	result->bs_bytes      = self;
	result->bs_sep_owner  = NULL;
	result->bs_sep_ptr    = result->bs_sep_buf;
	result->bs_sep_len    = 1;
	result->bs_sep_buf[0] = sep;
	Dee_Incref(self);
	DeeObject_Init(result, &BytesCaseSplit_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
DeeBytes_CaseSplit(Bytes *__restrict self,
                   DeeObject *__restrict sep) {
	ASSERT_OBJECT(sep);
	ASSERT(DeeString_Check(sep) || DeeBytes_Check(sep));
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if
		unlikely(!result)
	goto done;
	if (DeeString_Check(sep)) {
		result->bs_sep_ptr = DeeString_AsBytes(sep, false);
		if
			unlikely(!result->bs_sep_ptr)
		goto err_r;
		result->bs_sep_len = WSTR_LENGTH(result->bs_sep_ptr);
	} else {
		result->bs_sep_ptr = DeeBytes_DATA(sep);
		result->bs_sep_len = DeeBytes_SIZE(sep);
	}
	result->bs_bytes     = self;
	result->bs_sep_owner = sep;
	Dee_Incref(self);
	Dee_Incref(sep);
	DeeObject_Init(result, &BytesCaseSplit_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}








typedef struct {
	OBJECT_HEAD
	DREF Bytes          *bls_bytes;    /* [1..1][const] The Bytes object being split. */
	bool                 bls_keepends; /* [const] If true, keep line endings. */
} BytesLineSplit;

typedef struct {
	OBJECT_HEAD
	DREF Bytes          *blsi_bytes;    /* [1..1][const] The Bytes object being split. */
	ATOMIC_DATA uint8_t *blsi_iter;     /* [0..1] Pointer to the start of the next split (When NULL, iteration is complete). */
	uint8_t             *blsi_end;      /* [1..1][== DeeBytes_TERM(blsi_bytes)] Pointer to the end of input data. */
	bool                 blsi_keepends; /* [const] If true, keep line endings. */
} BytesLineSplitIterator;

#ifndef CONFIG_NO_THREADS
#define READ_BLSI_ITER(x) ATOMIC_READ((x)->blsi_iter)
#else /* !CONFIG_NO_THREADS */
#define READ_BLSI_ITER(x)            ((x)->blsi_iter)
#endif /* CONFIG_NO_THREADS */

STATIC_ASSERT(COMPILER_OFFSETOF(BytesSplitIterator, bsi_split) ==
              COMPILER_OFFSETOF(BytesLineSplitIterator, blsi_bytes));
STATIC_ASSERT(COMPILER_OFFSETOF(BytesSplitIterator, bsi_iter) ==
              COMPILER_OFFSETOF(BytesLineSplitIterator, blsi_iter));

PRIVATE int DCALL
blsi_init(BytesLineSplitIterator *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	BytesLineSplit *ls;
	self->blsi_keepends = false;
	if (DeeArg_Unpack(argc, argv, "o|b:_BytesLineSplitIterator", &ls, &self->blsi_keepends) ||
	    DeeObject_AssertTypeExact((DeeObject *)ls, &BytesLineSplit_Type))
		goto err;
	self->blsi_bytes = ls->bls_bytes;
	self->blsi_iter  = DeeBytes_DATA(self->blsi_bytes);
	self->blsi_end   = self->blsi_iter + DeeBytes_SIZE(self->blsi_bytes);
	if (self->blsi_iter == self->blsi_end)
		self->blsi_iter = NULL;
	Dee_Incref(self->blsi_bytes);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
blsi_copy(BytesLineSplitIterator *__restrict self,
          BytesLineSplitIterator *__restrict other) {
	self->blsi_bytes = other->blsi_bytes;
	self->blsi_iter  = READ_BLSI_ITER(other);
	self->blsi_end   = other->blsi_end;
	Dee_Incref(self->blsi_bytes);
	return 0;
}

#define blsi_fini   bsi_fini
#define blsi_visit  bsi_visit
#define blsi_bool   bsi_bool
#define blsi_cmp    bsi_cmp

PRIVATE DREF DeeObject *DCALL
blsi_next(BytesLineSplitIterator *__restrict self) {
	uint8_t *start, *end;
#ifdef CONFIG_NO_THREADS
	start = self->blsi_iter;
	if (!start)
		return ITER_DONE;
	end = start;
	for (;;) {
		uint8_t ch;
		if (end >= self->blsi_end) {
			if (!ATOMIC_CMPXCH_WEAK(self->blsi_iter, start, NULL))
				goto again;
			return DeeBytes_NewView(self->blsi_bytes->b_orig,
			                        start, (size_t)(self->blsi_end - start),
			                        self->blsi_bytes->b_flags);
		}
		ch = *end;
		if (ch == '\n')
			break;
		if (ch == '\r') {
			if (end + 1 < self->blsi_end && end[1] == '\n') {
				self->blsi_iter = end + 2;
				goto return_view;
			}
			break;
		}
		++end;
	}
	self->blsi_iter = end + 1;
return_view:
#else  /* CONFIG_NO_THREADS */
again:
	for (;;) {
		start = ATOMIC_READ(self->blsi_iter);
		if (!start)
			return ITER_DONE;
		end = start;
		for (;;) {
			uint8_t ch;
			if (end >= self->blsi_end) {
				if (!ATOMIC_CMPXCH_WEAK(self->blsi_iter, start, NULL))
					goto again;
				return DeeBytes_NewView(self->blsi_bytes->b_orig,
				                        start, (size_t)(self->blsi_end - start),
				                        self->blsi_bytes->b_flags);
			}
			ch = *end;
			if (ch == '\n')
				break;
			if (ch == '\r') {
				if (end + 1 < self->blsi_end && end[1] == '\n') {
					if (!ATOMIC_CMPXCH_WEAK(self->blsi_iter, start, end + 2))
						goto again;
					if (self->blsi_keepends)
						end += 2;
					goto return_view;
				}
				break;
			}
			++end;
		}
		if (ATOMIC_CMPXCH_WEAK(self->blsi_iter, start, end + 1))
			break;
	}
	if (self->blsi_keepends)
		end += 1;
return_view:
#endif /* !CONFIG_NO_THREADS */
	return DeeBytes_NewView(self->blsi_bytes->b_orig,
	                        start, (size_t)(end - start),
	                        self->blsi_bytes->b_flags);
}


PRIVATE DREF DeeObject *DCALL
blsi_getseq(BytesLineSplitIterator *__restrict self) {
	return DeeBytes_SplitLines(self->blsi_bytes,
	                           self->blsi_keepends);
}

PRIVATE struct type_getset blsi_getsets[] = {
	{ DeeString_STR(&str_seq), (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & blsi_getseq },
	{ NULL }
};

PRIVATE struct type_member blsi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesLineSplitIterator, blsi_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__keepends__", STRUCT_BOOL | STRUCT_CONST, offsetof(BytesLineSplitIterator, blsi_keepends)),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject BytesLineSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesLineSplitIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &blsi_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &blsi_init,
				TYPE_FIXED_ALLOCATOR(BytesLineSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&blsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&blsi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL, /*  No visit, because it only ever references strings
                                    * (or rather an object that can only reference strings). */
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &blsi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blsi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */blsi_getsets,
	/* .tp_members       = */blsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(COMPILER_OFFSETOF(BytesLineSplitIterator, blsi_bytes) ==
              COMPILER_OFFSETOF(BytesLineSplit, bls_bytes));
STATIC_ASSERT(COMPILER_OFFSETOF(BytesSplit, bs_bytes) ==
              COMPILER_OFFSETOF(BytesLineSplit, bls_bytes));

PRIVATE int DCALL
bls_ctor(BytesLineSplit *__restrict self) {
	self->bls_bytes    = (DREF Bytes *)Dee_EmptyBytes;
	self->bls_keepends = false;
	Dee_Incref(Dee_EmptyBytes);
	return 0;
}

PRIVATE int DCALL
bls_init(BytesLineSplit *__restrict self, size_t argc,
         DeeObject **__restrict argv) {
	self->bls_keepends = false;
	if (DeeArg_Unpack(argc, argv, "o|b:_BytesLineSplit", &self->bls_bytes, &self->bls_keepends) ||
	    DeeObject_AssertTypeExact((DeeObject *)self->bls_bytes, &DeeBytes_Type))
		goto err;
	Dee_Incref(self->bls_bytes);
	return 0;
err:
	return -1;
}

#define bls_fini    blsi_fini
#define bls_visit   blsi_visit
#define bls_bool    bs_bool

PRIVATE DREF BytesLineSplitIterator *DCALL
bls_iter(BytesLineSplit *__restrict self) {
	DREF BytesLineSplitIterator *result;
	result = DeeObject_MALLOC(BytesLineSplitIterator);
	if
		unlikely(!result)
	goto done;
	result->blsi_bytes    = self->bls_bytes;
	result->blsi_keepends = self->bls_keepends;
	result->blsi_iter     = DeeBytes_DATA(self->bls_bytes);
	result->blsi_end      = result->blsi_iter + DeeBytes_SIZE(self->bls_bytes);
	if (result->blsi_iter == result->blsi_end)
		result->blsi_iter = NULL;
	Dee_Incref(result->blsi_bytes);
	DeeObject_Init(result, &BytesLineSplitIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq bls_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bls_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_member bls_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesLineSplit, bls_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__keepends__", STRUCT_BOOL | STRUCT_CONST, offsetof(BytesLineSplit, bls_keepends)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member bls_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BytesLineSplitIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesLineSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesLineSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &bls_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &bls_init,
				TYPE_FIXED_ALLOCATOR(BytesLineSplit)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&bls_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&bls_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bls_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bls_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bls_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bls_class_members
};



INTERN DREF DeeObject *DCALL
DeeBytes_SplitLines(Bytes *__restrict self,
                    bool keepends) {
	DREF BytesLineSplit *result;
	result = DeeObject_MALLOC(BytesLineSplit);
	if
		unlikely(!result)
	goto done;
	result->bls_bytes    = self;
	result->bls_keepends = keepends;
	Dee_Incref(self);
	DeeObject_Init(result, &BytesLineSplit_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_C_INL */
