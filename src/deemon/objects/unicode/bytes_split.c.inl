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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpy */
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>
/**/

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

INTDEF DeeTypeObject BytesSplitIterator_Type;
INTDEF DeeTypeObject BytesSplit_Type;
INTDEF DeeTypeObject BytesCaseSplitIterator_Type;
INTDEF DeeTypeObject BytesCaseSplit_Type;
INTDEF DeeTypeObject BytesLineSplitIterator_Type;
INTDEF DeeTypeObject BytesLineSplit_Type;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(Bytes,     bs_bytes,     /* [1..1][const] The Bytes object being split. */
	                      DeeObject, bs_sep_owner) /* [0..1][const] The owner of the split sequence. */
	byte_t const                    *bs_sep_ptr;   /* [const] Pointer to the effective separation sequence. */
	size_t                           bs_sep_len;   /* [const][!0] Length of the separation sequence (in bytes). */
	byte_t                           bs_sep_buf[sizeof(void *)]; /* A small inline-buffer used for single-byte splits. */
} BytesSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(BytesSplit, bsi_split)    /* [1..1][const] The underlying split controller. */
	DWEAK byte_t const              *bsi_iter;     /* [0..1] Pointer to the start of the next split (When NULL, iteration is complete). */
	byte_t const                    *bsi_end;      /* [1..1][== DeeBytes_TERM(bsi_split->bs_bytes)] Pointer to the end of input data. */
	Bytes                           *bsi_bytes;    /* [1..1][const][== bsi_split->bs_bytes] The Bytes object being split. */
	byte_t const                    *bsi_sep_ptr;  /* [const][== bsi_split->bs_sep_ptr] Pointer to the effective separation sequence. */
	size_t                           bsi_sep_len;  /* [const][== bsi_split->bs_sep_len] Length of the separation sequence (in bytes). */
} BytesSplitIterator;

#define READ_BSI_ITER(x) atomic_read(&(x)->bsi_iter)

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsi_init(BytesSplitIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_BytesSplitIterator",
	                  &self->bsi_split);
	if (DeeObject_AssertTypeExact(self->bsi_split,
	                              Dee_TYPE(self) == &BytesSplitIterator_Type
	                              ? &BytesSplit_Type
	                              : &BytesCaseSplit_Type))
		goto err;
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsi_deepcopy(BytesSplitIterator *__restrict self,
             BytesSplitIterator *__restrict other) {
	byte_t const *iterpos;
	self->bsi_split = (DREF BytesSplit *)DeeObject_DeepCopy((DeeObject *)other->bsi_split);
	if unlikely(!self->bsi_split)
		goto err;
	iterpos = READ_BSI_ITER(other);
	if (iterpos) {
		iterpos = DeeBytes_DATA(self->bsi_split->bs_bytes) +
		          (iterpos - DeeBytes_DATA(other->bsi_bytes));
	}
	self->bsi_iter    = iterpos;
	self->bsi_end     = DeeBytes_TERM(self->bsi_split->bs_bytes);
	self->bsi_bytes   = self->bsi_split->bs_bytes;
	self->bsi_sep_ptr = self->bsi_split->bs_sep_ptr;
	self->bsi_sep_len = self->bsi_split->bs_sep_len;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(BytesSplitIterator, bsi_split) == offsetof(ProxyObject, po_obj));
#define bsi_fini  generic_proxy__fini
#define bsi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsi_bool(BytesSplitIterator *__restrict self) {
	return READ_BSI_ITER(self) != NULL;
}


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bsi_hash(BytesSplitIterator *self) {
	return Dee_HashPointer(READ_BSI_ITER(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsi_compare(BytesSplitIterator *self, BytesSplitIterator *other) {
	byte_t const *lhs, *rhs;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	lhs = READ_BSI_ITER(self);
	rhs = READ_BSI_ITER(other);
	if (!lhs)
		lhs = (byte_t const *)(void const *)-1l;
	if (!rhs)
		rhs = (byte_t const *)(void const *)-1l;
	Dee_return_compare(lhs, rhs);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp bsi_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&bsi_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&bsi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsi_next(BytesSplitIterator *__restrict self) {
	byte_t const *start, *end;
	for (;;) {
		start = atomic_read(&self->bsi_iter);
		if (!start)
			return ITER_DONE;
		end = (byte_t *)memmemb(start,
		                        self->bsi_end - start,
		                        self->bsi_sep_ptr,
		                        self->bsi_sep_len);
		if (!end) {
			if (!atomic_cmpxch_weak_or_write(&self->bsi_iter, start, NULL))
				continue;
			return DeeBytes_NewView(self->bsi_bytes->b_orig,
			                        (void *)start,
			                        (size_t)(self->bsi_end - start),
			                        self->bsi_bytes->b_flags);
		}
		if (atomic_cmpxch_weak_or_write(&self->bsi_iter, start, end + self->bsi_sep_len))
			break;
	}
	return DeeBytes_NewView(self->bsi_bytes->b_orig,
	                        (void *)start,
	                        (size_t)(end - start),
	                        self->bsi_bytes->b_flags);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsci_next(BytesSplitIterator *__restrict self) {
	byte_t const *start, *end;
	for (;;) {
		start = atomic_read(&self->bsi_iter);
		if (!start)
			return ITER_DONE;
		end = (byte_t *)memasciicasemem(start,
		                                self->bsi_end - start,
		                                self->bsi_sep_ptr,
		                                self->bsi_sep_len);
		if (!end) {
			if (!atomic_cmpxch_weak_or_write(&self->bsi_iter, start, NULL))
				continue;
			return DeeBytes_NewView(self->bsi_bytes->b_orig,
			                        (void *)start,
			                        (size_t)(self->bsi_end - start),
			                        self->bsi_bytes->b_flags);
		}
		if (atomic_cmpxch_weak_or_write(&self->bsi_iter, start, end + self->bsi_sep_len))
			break;
	}
	return DeeBytes_NewView(self->bsi_bytes->b_orig,
	                        (void *)start,
	                        (size_t)(end - start),
	                        self->bsi_bytes->b_flags);
}

PRIVATE struct type_member tpconst bsi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_split), "->?Ert:BytesSplit"),
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_bytes), "->?DBytes"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define bcsi_members bsi_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst bcsi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_split), "->?Ert:BytesCaseSplit"),
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplitIterator, bsi_bytes), "->?DBytes"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */


INTERN DeeTypeObject BytesSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:BytesSplit)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&bsi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bsi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bsi_init,
				TYPE_FIXED_ALLOCATOR(BytesSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bsi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bsi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject BytesCaseSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:BytesCaseSplit)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&bsi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bsi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bsi_init,
				TYPE_FIXED_ALLOCATOR(BytesSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bsi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bsi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsci_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bcsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_copy(BytesSplit *__restrict self,
        BytesSplit *__restrict other) {
	self->bs_bytes     = other->bs_bytes;
	self->bs_sep_owner = other->bs_sep_owner;
	self->bs_sep_ptr   = other->bs_sep_ptr;
	self->bs_sep_len   = other->bs_sep_len;
	if (self->bs_sep_ptr == other->bs_sep_buf) {
		self->bs_sep_ptr = (byte_t *)memcpy(self->bs_sep_buf,
		                                    other->bs_sep_buf,
		                                    sizeof(self->bs_sep_buf));
	}
	Dee_Incref(self->bs_bytes);
	Dee_XIncref(self->bs_sep_owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_deepcopy(BytesSplit *__restrict self,
            BytesSplit *__restrict other) {
	self->bs_bytes = (DREF Bytes *)DeeObject_DeepCopy((DeeObject *)other->bs_bytes);
	if unlikely(!self->bs_bytes)
		goto err;
	self->bs_sep_owner = other->bs_sep_owner;
	self->bs_sep_ptr   = other->bs_sep_ptr;
	self->bs_sep_len   = other->bs_sep_len;
	if (self->bs_sep_ptr == other->bs_sep_buf) {
		self->bs_sep_ptr = (byte_t *)memcpy(self->bs_sep_buf,
		                                    other->bs_sep_buf,
		                                    sizeof(self->bs_sep_buf));
	}
	if (!self->bs_sep_owner) {
		/* ... */
	} else if (DeeBytes_Check(self->bs_sep_owner)) {
		self->bs_sep_owner = DeeObject_DeepCopy(self->bs_sep_owner);
		if unlikely(!self->bs_sep_owner)
			goto err_bytes;
		self->bs_sep_ptr = DeeBytes_DATA(self->bs_sep_owner);
		self->bs_sep_len = DeeBytes_SIZE(self->bs_sep_owner);
	} else {
		/* Can't copy the sep-owner in this case... */
		Dee_Incref(self->bs_sep_owner);
	}
	return 0;
err_bytes:
	Dee_Decref(self->bs_bytes);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_init(BytesSplit *__restrict self, size_t argc,
        DeeObject *const *argv) {
	_DeeArg_Unpack2(err, argc, argv, "_BytesSplit", &self->bs_bytes, &self->bs_sep_owner);
	if (DeeObject_AssertTypeExact(self->bs_bytes, &DeeBytes_Type))
		goto err;
	if (DeeBytes_Check(self->bs_sep_owner)) {
		self->bs_sep_ptr = DeeBytes_DATA(self->bs_sep_owner);
		self->bs_sep_len = DeeBytes_SIZE(self->bs_sep_owner);
		if unlikely(!self->bs_sep_len)
			goto err_empty_sep;
		Dee_Incref(self->bs_sep_owner);
	} else if (DeeString_Check(self->bs_sep_owner)) {
		self->bs_sep_ptr = DeeString_AsBytes(self->bs_sep_owner, false);
		if unlikely(!self->bs_sep_ptr)
			goto err;
		self->bs_sep_len = WSTR_LENGTH(self->bs_sep_ptr);
		if unlikely(!self->bs_sep_len)
			goto err_empty_sep;
		Dee_Incref(self->bs_sep_owner);
	} else {
		if (DeeObject_AsUIntX(self->bs_sep_owner, &self->bs_sep_buf[0]))
			goto err;
		self->bs_sep_owner = NULL;
		self->bs_sep_ptr   = self->bs_sep_buf;
		self->bs_sep_len   = 1;
	}
	Dee_Incref(self->bs_bytes);
	return 0;
err_empty_sep:
	DeeError_Throwf(&DeeError_ValueError, "Empty split separator");
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
bs_fini(BytesSplit *__restrict self) {
	Dee_Decref(self->bs_bytes);
	Dee_XDecref(self->bs_sep_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
bs_visit(BytesSplit *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bs_bytes);
	Dee_XVisit(self->bs_sep_owner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_bool(BytesSplit *__restrict self) {
	return DeeBytes_SIZE(self->bs_bytes) != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesSplitIterator *DCALL
bs_iter(BytesSplit *__restrict self) {
	DREF BytesSplitIterator *result;
	result = DeeObject_MALLOC(BytesSplitIterator);
	if unlikely(!result)
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

PRIVATE WUNUSED NONNULL((1)) DREF BytesSplitIterator *DCALL
bcs_iter(BytesSplit *__restrict self) {
	DREF BytesSplitIterator *result;
	result = DeeObject_MALLOC(BytesSplitIterator);
	if unlikely(!result)
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
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bs_iter,
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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
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

PRIVATE struct type_seq bcs_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcs_iter,
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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_getsep(BytesSplit *__restrict self) {
	return DeeBytes_NewViewRo(self->bs_sep_owner ? self->bs_sep_owner
	                                             : (DeeObject *)self,
	                          self->bs_sep_ptr, self->bs_sep_len);
}

PRIVATE struct type_getset tpconst bs_getsets[] = {
	TYPE_GETTER_AB("__sep__", &bs_getsep, "->?DBytes"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst bs_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSplit, bs_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__sep_owner__", STRUCT_OBJECT, offsetof(BytesSplit, bs_sep_owner)),
	TYPE_MEMBER_FIELD("__sep_length__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BytesSplit, bs_sep_len)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bs_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesSplitIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bcs_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesCaseSplitIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSplit",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,sep:?X3?DBytes?Dstring?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&bs_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bs_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bs_init,
				TYPE_FIXED_ALLOCATOR(BytesSplit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bs_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bs_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bs_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject BytesCaseSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseSplit",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,sep:?X3?DBytes?Dstring?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&bs_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bs_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bs_init,
				TYPE_FIXED_ALLOCATOR(BytesSplit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bs_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bs_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bs_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bcs_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bcs_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_SplitByte(Bytes *__restrict self, byte_t sep) {
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if unlikely(!result)
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_Split(Bytes *self, DeeObject *sep) {
	ASSERT_OBJECT(sep);
	ASSERT(DeeString_Check(sep) || DeeBytes_Check(sep));
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if unlikely(!result)
		goto done;
	if (DeeString_Check(sep)) {
		result->bs_sep_ptr = DeeString_AsBytes(sep, false);
		if unlikely(!result->bs_sep_ptr)
			goto err_r;
		result->bs_sep_len = WSTR_LENGTH(result->bs_sep_ptr);
	} else {
		result->bs_sep_ptr = DeeBytes_DATA(sep);
		result->bs_sep_len = DeeBytes_SIZE(sep);
	}
	if unlikely(!result->bs_sep_len)
		goto handle_empty_sep;
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
handle_empty_sep:
	DeeObject_FREE(result);
	return DeeSuper_New(&DeeSeq_Type, (DeeObject *)self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_CaseSplitByte(Bytes *__restrict self, byte_t sep) {
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if unlikely(!result)
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseSplit(Bytes *self, DeeObject *sep) {
	ASSERT_OBJECT(sep);
	ASSERT(DeeString_Check(sep) || DeeBytes_Check(sep));
	DREF BytesSplit *result;
	result = DeeObject_MALLOC(BytesSplit);
	if unlikely(!result)
		goto done;
	if (DeeString_Check(sep)) {
		result->bs_sep_ptr = DeeString_AsBytes(sep, false);
		if unlikely(!result->bs_sep_ptr)
			goto err_r;
		result->bs_sep_len = WSTR_LENGTH(result->bs_sep_ptr);
	} else {
		result->bs_sep_ptr = DeeBytes_DATA(sep);
		result->bs_sep_len = DeeBytes_SIZE(sep);
	}
	if unlikely(!result->bs_sep_len)
		goto handle_empty_sep;
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
handle_empty_sep:
	DeeObject_FREE(result);
	return DeeSuper_New(&DeeSeq_Type, (DeeObject *)self);
}








typedef struct {
	PROXY_OBJECT_HEAD_EX(Bytes, bls_bytes)    /* [1..1][const] The Bytes object being split. */
	bool                        bls_keepends; /* [const] If true, keep line endings. */
} BytesLineSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(Bytes, blsi_bytes)    /* [1..1][const] The Bytes object being split. */
	DWEAK byte_t               *blsi_iter;     /* [0..1] Pointer to the start of the next split (When NULL, iteration is complete). */
	byte_t                     *blsi_end;      /* [1..1][== DeeBytes_TERM(blsi_bytes)] Pointer to the end of input data. */
	bool                        blsi_keepends; /* [const] If true, keep line endings. */
} BytesLineSplitIterator;
#define READ_BLSI_ITER(x) atomic_read(&(x)->blsi_iter)

STATIC_ASSERT(offsetof(BytesSplitIterator, bsi_split) == offsetof(BytesLineSplitIterator, blsi_bytes));
STATIC_ASSERT(offsetof(BytesSplitIterator, bsi_iter) == offsetof(BytesLineSplitIterator, blsi_iter));

PRIVATE WUNUSED NONNULL((1)) int DCALL
blsi_init(BytesLineSplitIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	BytesLineSplit *ls;
	_DeeArg_Unpack1(err, argc, argv, "_BytesLineSplitIterator", &ls);
	if (DeeObject_AssertTypeExact(ls, &BytesLineSplit_Type))
		goto err;
	self->blsi_bytes    = ls->bls_bytes;
	self->blsi_iter     = DeeBytes_DATA(self->blsi_bytes);
	self->blsi_end      = self->blsi_iter + DeeBytes_SIZE(self->blsi_bytes);
	self->blsi_keepends = ls->bls_keepends;
	if (self->blsi_iter == self->blsi_end)
		self->blsi_iter = NULL;
	Dee_Incref(self->blsi_bytes);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
blsi_ctor(BytesLineSplitIterator *__restrict self) {
	self->blsi_bytes    = (DREF Bytes *)DeeBytes_NewEmpty();
	self->blsi_iter     = NULL;
	self->blsi_end      = DeeBytes_DATA(self->blsi_bytes);
	self->blsi_keepends = false;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blsi_copy(BytesLineSplitIterator *__restrict self,
          BytesLineSplitIterator *__restrict other) {
	self->blsi_bytes = other->blsi_bytes;
	self->blsi_iter  = READ_BLSI_ITER(other);
	self->blsi_end   = other->blsi_end;
	Dee_Incref(self->blsi_bytes);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
blsi_deepcopy(BytesLineSplitIterator *__restrict self,
              BytesLineSplitIterator *__restrict other) {
	byte_t *other_pos;
	self->blsi_bytes = (DREF Bytes *)DeeObject_DeepCopy((DeeObject *)other->blsi_bytes);
	if unlikely(!self->blsi_bytes)
		goto err;
	other_pos = READ_BLSI_ITER(other);
	if (other_pos) {
		other_pos = DeeBytes_DATA(self->blsi_bytes) +
		            (other_pos - DeeBytes_DATA(other->blsi_bytes));
	}
	self->blsi_iter = other_pos;
	self->blsi_end  = DeeBytes_TERM(self->blsi_bytes);
	Dee_Incref(self->blsi_bytes);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(BytesLineSplitIterator, blsi_bytes) == offsetof(ProxyObject, po_obj));
#define blsi_fini  generic_proxy__fini
#define blsi_visit generic_proxy__visit

#define blsi_bool bsi_bool
#define blsi_cmp  bsi_cmp

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blsi_next(BytesLineSplitIterator *__restrict self) {
	byte_t *start, *end;
again:
	for (;;) {
		start = atomic_read(&self->blsi_iter);
		if (!start)
			return ITER_DONE;
		end = start;
		for (;;) {
			byte_t ch;
			if (end >= self->blsi_end) {
				if (!atomic_cmpxch_weak_or_write(&self->blsi_iter, start, NULL))
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
					if (!atomic_cmpxch_weak_or_write(&self->blsi_iter, start, end + 2))
						goto again;
					if (self->blsi_keepends)
						end += 2;
					goto return_view;
				}
				break;
			}
			++end;
		}
		if (atomic_cmpxch_weak_or_write(&self->blsi_iter, start, end + 1))
			break;
	}
	if (self->blsi_keepends)
		end += 1;
return_view:
	return DeeBytes_NewView(self->blsi_bytes->b_orig,
	                        start, (size_t)(end - start),
	                        self->blsi_bytes->b_flags);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
blsi_getseq(BytesLineSplitIterator *__restrict self) {
	return DeeBytes_SplitLines(self->blsi_bytes,
	                           self->blsi_keepends);
}

PRIVATE struct type_getset tpconst blsi_getsets[] = {
	TYPE_GETTER_F(STR_seq, &blsi_getseq, METHOD_FNOREFESCAPE, "->?Ert:BytesLineSplit"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst blsi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesLineSplitIterator, blsi_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__keepends__", STRUCT_CONST | STRUCT_CBOOL, offsetof(BytesLineSplitIterator, blsi_keepends)),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject BytesLineSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesLineSplitIterator",
	/* .tp_doc      = */ DOC("(split:?Ert:BytesLineSplit)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&blsi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&blsi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&blsi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&blsi_init,
				TYPE_FIXED_ALLOCATOR(BytesLineSplitIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&blsi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&blsi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&blsi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &blsi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&blsi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ blsi_getsets,
	/* .tp_members       = */ blsi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
bls_ctor(BytesLineSplit *__restrict self) {
	self->bls_bytes    = (DREF Bytes *)DeeBytes_NewEmpty();
	self->bls_keepends = false;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bls_copy(BytesLineSplit *__restrict self,
         BytesLineSplit *__restrict other) {
	self->bls_bytes    = other->bls_bytes;
	self->bls_keepends = other->bls_keepends;
	Dee_Incref(self->bls_bytes);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bls_deepcopy(BytesLineSplit *__restrict self,
             BytesLineSplit *__restrict other) {
	self->bls_bytes = (DREF Bytes *)DeeObject_DeepCopy((DeeObject *)other->bls_bytes);
	if unlikely(!self->bls_bytes)
		goto err;
	self->bls_keepends = other->bls_keepends;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bls_init(BytesLineSplit *__restrict self, size_t argc,
         DeeObject *const *argv) {
	self->bls_keepends = false;
	if (DeeArg_Unpack(argc, argv, "o|b:_BytesLineSplit", &self->bls_bytes, &self->bls_keepends))
		goto err;
	if (DeeObject_AssertTypeExact(self->bls_bytes, &DeeBytes_Type))
		goto err;
	Dee_Incref(self->bls_bytes);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(BytesLineSplit, bls_bytes) == offsetof(ProxyObject, po_obj));
#define bls_fini  generic_proxy__fini
#define bls_visit generic_proxy__visit

STATIC_ASSERT(offsetof(BytesSplit, bs_bytes) == offsetof(BytesLineSplit, bls_bytes));
#define bls_bool bs_bool

PRIVATE WUNUSED NONNULL((1)) DREF BytesLineSplitIterator *DCALL
bls_iter(BytesLineSplit *__restrict self) {
	DREF BytesLineSplitIterator *result;
	result = DeeObject_MALLOC(BytesLineSplitIterator);
	if unlikely(!result)
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
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bls_iter,
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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
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

PRIVATE struct type_member tpconst bls_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesLineSplit, bls_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD("__keepends__", STRUCT_CONST | STRUCT_CBOOL, offsetof(BytesLineSplit, bls_keepends)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bls_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesLineSplitIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesLineSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesLineSplit",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,keepends=!f)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bls_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bls_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bls_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bls_init,
				TYPE_FIXED_ALLOCATOR(BytesLineSplit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bls_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bls_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bls_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bls_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bls_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bls_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_SplitLines(Bytes *__restrict self,
                    bool keepends) {
	DREF BytesLineSplit *result;
	result = DeeObject_MALLOC(BytesLineSplit);
	if unlikely(!result)
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
