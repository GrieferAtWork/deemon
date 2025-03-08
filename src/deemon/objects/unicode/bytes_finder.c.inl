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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>
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

typedef struct {
	PROXY_OBJECT_HEAD2_EX(Bytes,     bf_bytes,  /* [1..1][const] The bytes that is being searched. */
	                      DeeObject, bf_other); /* [1..1][const] The needle object. */
	Needle                           bf_needle; /* [const] The needle being searched for. */
	byte_t                          *bf_start;  /* [1..1][const] Starting pointer. */
	byte_t                          *bf_end;    /* [1..1][const] End pointer. */
} BytesFind;

typedef struct {
	PROXY_OBJECT_HEAD_EX(BytesFind, bfi_find)       /* [1..1][const] The underlying find-controller. */
	byte_t                         *bfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK byte_t                   *bfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	byte_t                         *bfi_end;        /* [1..1][const] End pointer. */
	byte_t                         *bfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t                          bfi_needle_len; /* [const] Length of the needle being searched. */
} BytesFindIterator;

INTDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *other,
                 size_t start, size_t end);
INTDEF WUNUSED DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *other,
                     size_t start, size_t end);
#define READ_PTR(x) atomic_read(&(x)->bfi_ptr)

INTDEF DeeTypeObject BytesFindIterator_Type;
INTDEF DeeTypeObject BytesFind_Type;
INTDEF DeeTypeObject BytesCaseFindIterator_Type;
INTDEF DeeTypeObject BytesCaseFind_Type;


PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_FindAll((Bytes *)Dee_EmptyBytes,
	                                                    Dee_EmptyBytes, 0, 0);
	if unlikely(!self->bfi_find)
		goto err;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_CaseFindAll((Bytes *)Dee_EmptyBytes,
	                                                        Dee_EmptyBytes, 0, 0);
	if unlikely(!self->bfi_find)
		goto err;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	return 0;
err:
	return -1;
}

#define bcfi_copy bfi_copy
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
	if (DeeObject_AssertTypeExact(find, &BytesFind_Type))
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
	if (DeeObject_AssertTypeExact(find, &BytesCaseFind_Type))
		goto err;
	return bfi_setup(self, find);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bfi_next(BytesFindIterator *__restrict self) {
	byte_t *ptr, *new_ptr;
again:
	ptr     = atomic_read(&self->bfi_ptr);
	new_ptr = (byte_t *)memmemb(ptr, (size_t)(self->bfi_end - ptr),
	                            self->bfi_needle_ptr,
	                            self->bfi_needle_len);
	if (new_ptr) {
		if (!atomic_cmpxch_weak(&self->bfi_ptr, ptr, new_ptr + self->bfi_needle_len))
			goto again;
		return DeeInt_NewSize((size_t)(new_ptr - self->bfi_start));
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bcfi_next(BytesFindIterator *__restrict self) {
	byte_t *ptr, *new_ptr;
again:
	ptr     = atomic_read(&self->bfi_ptr);
	new_ptr = (byte_t *)memasciicasemem(ptr, (size_t)(self->bfi_end - ptr),
	                                    self->bfi_needle_ptr,
	                                    self->bfi_needle_len);
	if (new_ptr) {
		size_t result;
		if (!atomic_cmpxch_weak(&self->bfi_ptr, ptr, new_ptr + self->bfi_needle_len))
			goto again;
		result = (size_t)(new_ptr - self->bfi_start);
		return DeeTuple_Newf(PCKuSIZ
		                     PCKuSIZ,
		                     result,
		                     result + self->bfi_needle_len);
	}
	return ITER_DONE;
}

STATIC_ASSERT(offsetof(BytesFindIterator, bfi_find) == offsetof(ProxyObject, po_obj));
#define bfi_fini  generic_proxy__fini
#define bfi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_bool(BytesFindIterator *__restrict self) {
	byte_t *ptr;
	ptr = atomic_read(&self->bfi_ptr);
	ptr = memmemb(ptr, (size_t)(self->bfi_end - ptr),
	              self->bfi_needle_ptr,
	              self->bfi_needle_len);
	return ptr != NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_bool(BytesFindIterator *__restrict self) {
	byte_t *ptr;
	ptr = atomic_read(&self->bfi_ptr);
	ptr = (byte_t *)memasciicasemem(ptr, (size_t)(self->bfi_end - ptr),
	                                self->bfi_needle_ptr,
	                                self->bfi_needle_len);
	return ptr != NULL;
}

PRIVATE struct type_member tpconst bfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BytesFindIterator, bfi_find), "->?Ert:BytesFind"),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_DOC
#define bcfi_members bfi_members
#else /* CONFIG_NO_DOC */
PRIVATE struct type_member tpconst bcfi_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BytesFindIterator, bfi_find), "->?Ert:BytesCaseFind"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bfi_hash(BytesFindIterator *self) {
	return Dee_HashPointer(READ_PTR(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bfi_compare(BytesFindIterator *self, BytesFindIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(byte_t *, READ_PTR(self),
	                    /*     */ READ_PTR(other));
err:
	return Dee_COMPARE_ERR;
}

#define bcfi_cmp bfi_cmp
PRIVATE struct type_cmp bfi_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&bfi_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&bfi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


INTERN DeeTypeObject BytesFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesFindIterator",
	/* .tp_doc      = */ DOC("(find:?Ert:BytesFind)\n"
	                         "\n"
	                         "next->?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bfi_init,
				TYPE_FIXED_ALLOCATOR(BytesFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bfi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &bfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bfi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN DeeTypeObject BytesCaseFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFindIterator",
	/* .tp_doc      = */ DOC("(find:?Ert:BytesCaseFind)\n"
	                         "\n"
	                         "next->?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bcfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bcfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bcfi_init,
				TYPE_FIXED_ALLOCATOR(BytesFindIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bcfi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &bcfi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcfi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bcfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
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
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":_BytesFind",
	                  &self->bf_bytes, &self->bf_other,
	                  &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(self->bf_bytes, &DeeBytes_Type))
		goto err;
	if (get_needle(&self->bf_needle, self->bf_other))
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

STATIC_ASSERT(offsetof(BytesFind, bf_bytes) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(BytesFind, bf_bytes) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(BytesFind, bf_other) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(BytesFind, bf_other) == offsetof(ProxyObject2, po_obj2));
#define bf_fini  generic_proxy2__fini
#define bf_visit generic_proxy2__visit

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
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bf_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size), /* TODO: Bytes.count() */
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), /* TODO: Bytes.substr() == needle */
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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
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

PRIVATE struct type_seq bcf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcf_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size), /* TODO: Bytes.casecount() */
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), /* TODO: Bytes.substr(...).casecompare(needle) == 0 */
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
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
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


PRIVATE struct type_member tpconst bf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesFind, bf_bytes), "->?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__needle__", STRUCT_OBJECT, offsetof(BytesFind, bf_needle), "->?DBytes"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesFind, bf_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesFind, bf_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesFindIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bcf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesCaseFindIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesFind",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,needle:?DBytes,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bf_init,
				TYPE_FIXED_ALLOCATOR(BytesFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL  /* TODO: Bytes.contains() */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bf_class_members,
};

INTERN DeeTypeObject BytesCaseFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFind",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,needle:?DBytes,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bf_init,
				TYPE_FIXED_ALLOCATOR(BytesFind)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ NULL  /* TODO: Bytes.casecontains() */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bcf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bcf_class_members,
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
