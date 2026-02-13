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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack, DeeArg_Unpack1, UNPuSIZ, UNPxSIZ */
#include <deemon/bytes.h>              /* DeeBytes*, Dee_EmptyBytes */
#include <deemon/computed-operators.h>
#include <deemon/format.h>             /* PCKuSIZ */
#include <deemon/int.h>                /* DeeInt_NewSize, DeeInt_Type */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_AssertTypeExact, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_hash_t, Dee_return_compareT, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_NewEmpty, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/system-features.h>    /* memcasemem, memmem */
#include <deemon/tuple.h>              /* DeeTuple_Newf */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak, atomic_read */
#include <deemon/util/hash.h>          /* Dee_HashPointer */

#include <hybrid/overflow.h> /* OVERFLOW_USUB */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

typedef struct {
	PROXY_OBJECT_HEAD2_EX(Bytes,     bf_bytes,  /* [1..1][const] The bytes that is being searched. */
	                      DeeObject, bf_other); /* [1..1][const] The needle object. */
	Needle                           bf_needle; /* [const] The needle being searched for. */
	byte_t const                    *bf_start;  /* [1..1][const] Starting pointer. */
	byte_t const                    *bf_end;    /* [1..1][const] End pointer. */
	bool                             bf_ovrlap; /* [const] When true, "bfi_find_delta = 1", else "bfi_find_delta = max(bf_needle.n_size, 1)" */
} BytesFind;

typedef struct {
	PROXY_OBJECT_HEAD_EX(BytesFind, bfi_find)       /* [1..1][const] The underlying find-controller. */
	byte_t const                   *bfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK byte_t const             *bfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	byte_t const                   *bfi_end;        /* [1..1][const] End pointer. */
	byte_t const                   *bfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t                          bfi_needle_len; /* [const] Length of the needle being searched. */
	size_t                          bfi_find_delta; /* [const] Delta added to `sfi_ptr' after each match */
} BytesFindIterator;

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *needle,
                 size_t start, size_t end,
                 bool overlapping);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *needle,
                     size_t start, size_t end,
                     bool overlapping);
#define READ_PTR(x) atomic_read(&(x)->bfi_ptr)

INTDEF DeeTypeObject BytesFindIterator_Type;
INTDEF DeeTypeObject BytesFind_Type;
INTDEF DeeTypeObject BytesCaseFindIterator_Type;
INTDEF DeeTypeObject BytesCaseFind_Type;


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bf_serialize(BytesFind *__restrict self,
             DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(BytesFind, field))
	BytesFind *out = DeeSerial_Addr2Mem(writer, addr, BytesFind);
	out->bf_ovrlap = self->bf_ovrlap;
	if (DeeSerial_PutObject(writer, ADDROF(bf_bytes), self->bf_bytes))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(bf_other), self->bf_other))
		goto err;
	if (Needle_Serialize(&self->bf_needle, writer, ADDROF(bf_needle)))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(bf_start), self->bf_start))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(bf_end), self->bf_end);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bfi_serialize(BytesFindIterator *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(BytesFindIterator, field))
	BytesFindIterator *out = DeeSerial_Addr2Mem(writer, addr, BytesFindIterator);
	out->bfi_find_delta = self->bfi_find_delta;
	out->bfi_needle_len = self->bfi_needle_len;
	if (DeeSerial_PutObject(writer, ADDROF(bfi_find), self->bfi_find))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(bfi_start), self->bfi_start))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(bfi_end), self->bfi_end))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(bfi_ptr), atomic_read(&self->bfi_ptr)))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(bfi_needle_ptr), self->bfi_needle_ptr);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_FindAll((Bytes *)Dee_EmptyBytes,
	                                                    Dee_EmptyBytes, 0, 0, false);
	if unlikely(!self->bfi_find)
		goto err;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	self->bfi_find_delta = 1;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_ctor(BytesFindIterator *__restrict self) {
	self->bfi_find = (DREF BytesFind *)DeeBytes_CaseFindAll((Bytes *)Dee_EmptyBytes,
	                                                        Dee_EmptyBytes, 0, 0, false);
	if unlikely(!self->bfi_find)
		goto err;
	self->bfi_start      = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_ptr        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_end        = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->bfi_needle_len = 0;
	self->bfi_find_delta = 1;
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
	self->bfi_find_delta = other->bfi_find_delta;
	Dee_Incref(self->bfi_find);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
bfi_setup(BytesFindIterator *__restrict self,
          BytesFind *__restrict find) {
	self->bfi_find       = find;
	self->bfi_start      = find->bf_start;
	self->bfi_ptr        = find->bf_start;
	self->bfi_end        = find->bf_end;
	self->bfi_needle_ptr = find->bf_needle.n_data;
	self->bfi_needle_len = find->bf_needle.n_size;
	self->bfi_find_delta = find->bf_needle.n_size;
	if (self->bfi_find_delta == 0 || find->bf_ovrlap)
		self->bfi_find_delta = 1;
	Dee_Incref(find);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_init(BytesFindIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	BytesFind *find;
	DeeArg_Unpack1(err, argc, argv, "_BytesFindIterator", &find);
	if (DeeObject_AssertTypeExact(find, &BytesFind_Type))
		goto err;
	bfi_setup(self, find);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_init(BytesFindIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	BytesFind *find;
	DeeArg_Unpack1(err, argc, argv, "_BytesCaseFindIterator", &find);
	if (DeeObject_AssertTypeExact(find, &BytesCaseFind_Type))
		goto err;
	bfi_setup(self, find);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bfi_next(BytesFindIterator *__restrict self) {
	size_t scan_size;
	byte_t const *ptr, *new_ptr;
again:
	ptr = atomic_read(&self->bfi_ptr);
	if (OVERFLOW_USUB((uintptr_t)self->bfi_end, (uintptr_t)ptr, &scan_size))
		goto iter_done;
	new_ptr = memmemb(ptr, scan_size, self->bfi_needle_ptr, self->bfi_needle_len);
	if (new_ptr) {
		if (!atomic_cmpxch_weak(&self->bfi_ptr, ptr, new_ptr + self->bfi_find_delta))
			goto again;
		return DeeInt_NewSize((size_t)(new_ptr - self->bfi_start));
	}
iter_done:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bcfi_next(BytesFindIterator *__restrict self) {
	size_t scan_size;
	byte_t const *ptr, *new_ptr;
again:
	ptr = atomic_read(&self->bfi_ptr);
	if (OVERFLOW_USUB((uintptr_t)self->bfi_end, (uintptr_t)ptr, &scan_size))
		goto iter_done;
	new_ptr = memasciicasemem(ptr, scan_size,
	                          self->bfi_needle_ptr,
	                          self->bfi_needle_len);
	if (new_ptr) {
		size_t result;
		if (!atomic_cmpxch_weak(&self->bfi_ptr, ptr, new_ptr + self->bfi_find_delta))
			goto again;
		result = (size_t)(new_ptr - self->bfi_start);
		return DeeTuple_Newf(PCKuSIZ PCKuSIZ,
		                     result, result + self->bfi_needle_len);
	}
iter_done:
	return ITER_DONE;
}

STATIC_ASSERT(offsetof(BytesFindIterator, bfi_find) == offsetof(ProxyObject, po_obj));
#define bfi_fini  generic_proxy__fini
#define bfi_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
bfi_bool(BytesFindIterator *__restrict self) {
	size_t scan_size;
	byte_t const *ptr = atomic_read(&self->bfi_ptr);
	if (OVERFLOW_USUB((uintptr_t)self->bfi_end, (uintptr_t)ptr, &scan_size))
		goto iter_done;
	ptr = memmemb(ptr, scan_size, self->bfi_needle_ptr, self->bfi_needle_len);
	return ptr != NULL;
iter_done:
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcfi_bool(BytesFindIterator *__restrict self) {
	size_t scan_size;
	byte_t const *ptr = atomic_read(&self->bfi_ptr);
	if (OVERFLOW_USUB((uintptr_t)self->bfi_end, (uintptr_t)ptr, &scan_size))
		goto iter_done;
	ptr = memasciicasemem(ptr, scan_size, self->bfi_needle_ptr, self->bfi_needle_len);
	return ptr != NULL;
iter_done:
	return 0;
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
	Dee_return_compareT(byte_t const *, READ_PTR(self),
	                    /*           */ READ_PTR(other));
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesFindIterator,
			/* tp_ctor:        */ &bfi_ctor,
			/* tp_copy_ctor:   */ &bfi_copy,
			/* tp_any_ctor:    */ &bfi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bfi_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bfi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bfi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject BytesCaseFindIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFindIterator",
	/* .tp_doc      = */ DOC("(find:?Ert:BytesCaseFind)\n"
	                         "\n"
	                         "next->?T2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesFindIterator,
			/* tp_ctor:        */ &bcfi_ctor,
			/* tp_copy_ctor:   */ &bcfi_copy,
			/* tp_any_ctor:    */ &bcfi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bfi_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bfi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bcfi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bcfi_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bcfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
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
	if (DeeArg_Unpack(argc, argv, "oo|" UNPuSIZ UNPxSIZ ":_BytesFind",
	                  &self->bf_bytes, &self->bf_other,
	                  &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(self->bf_bytes, &DeeBytes_Type))
		goto err;
	if (acquire_needle(&self->bf_needle, self->bf_other)) /* TODO: release_needle() */
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
	bfi_setup(result, self);
	DeeObject_Init(result, &BytesFindIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesFindIterator *DCALL
bcf_iter(BytesFind *__restrict self) {
	DREF BytesFindIterator *result;
	result = DeeObject_MALLOC(BytesFindIterator);
	if unlikely(!result)
		goto done;
	bfi_setup(result, self);
	DeeObject_Init(result, &BytesCaseFindIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bf_size(BytesFind *__restrict self) {
	return memcnt(self->bf_start, (size_t)(self->bf_end - self->bf_start),
	              self->bf_needle.n_data, self->bf_needle.n_size);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bcf_size(BytesFind *__restrict self) {
	return memcasecnt(self->bf_start, (size_t)(self->bf_end - self->bf_start),
	                  self->bf_needle.n_data, self->bf_needle.n_size);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bf_bool(BytesFind *__restrict self) {
	return memmem(self->bf_start, (size_t)(self->bf_end - self->bf_start),
	              self->bf_needle.n_data, self->bf_needle.n_size)
	       ? 1
	       : 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bcf_bool(BytesFind *__restrict self) {
	return memcasemem(self->bf_start, (size_t)(self->bf_end - self->bf_start),
	                  self->bf_needle.n_data, self->bf_needle.n_size)
	       ? 1
	       : 0;
}

PRIVATE struct type_seq bf_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bf_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__sizeob__with__size),
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
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bf_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
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
	/* .tp_sizeob   = */ DEFIMPL(&default__sizeob__with__size),
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
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bcf_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
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
	TYPE_MEMBER_CONST(STR_ItemType, &DeeInt_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bcf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesCaseFindIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BytesFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesFind",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,needle:?DBytes,start=!0,end=!-1)\n"
	                         "\n"
	                         "size->\n"
	                         "Same as ${this.__str__.count(this.__needle__, this.__start__, this.__end__)}\n"
	                         "\n"
	                         "bool->\n"
	                         "Same as ${this.__str__.contains(this.__needle__, this.__start__, this.__end__)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesFind,
			/* tp_ctor:        */ &bf_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bf_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bf_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bf_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject BytesCaseFind_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesCaseFind",
	/* .tp_doc      = */ DOC("(bytes:?DBytes,needle:?DBytes,start=!0,end=!-1)\n"
	                         "\n"
	                         "#->\n"
	                         "Same as ${this.__str__.casecount(this.__needle__, this.__start__, this.__end__)}\n"
	                         "\n"
	                         "bool->\n"
	                         "Same as ${this.__str__.casecontains(this.__needle__, this.__start__, this.__end__)}\n"
	                         "\n"
	                         "[](index:?Dint)->?T2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesFind,
			/* tp_ctor:        */ &bf_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bf_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bcf_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &bcf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bcf_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_FindAll(Bytes *self, DeeObject *needle,
                 size_t start, size_t end,
                 bool overlapping) {
	DREF BytesFind *result;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeSeq_NewEmpty();
	result = DeeObject_MALLOC(BytesFind);
	if unlikely(!result)
		goto done;
	if (acquire_needle(&result->bf_needle, needle)) /* TODO: release_needle() */
		goto err_r;
	result->bf_bytes  = self;
	result->bf_other  = needle;
	result->bf_start  = DeeBytes_DATA(self) + start;
	result->bf_end    = DeeBytes_DATA(self) + end;
	result->bf_ovrlap = overlapping;
	Dee_Incref(self);
	Dee_Incref(needle);
	DeeObject_Init(result, &BytesFind_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseFindAll(Bytes *self, DeeObject *needle,
                     size_t start, size_t end,
                     bool overlapping) {
	DREF BytesFind *result;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeSeq_NewEmpty();
	result = DeeObject_MALLOC(BytesFind);
	if unlikely(!result)
		goto done;
	if (acquire_needle(&result->bf_needle, needle)) /* TODO: release_needle() */
		goto err_r;
	result->bf_bytes  = self;
	result->bf_other  = needle;
	result->bf_start  = DeeBytes_DATA(self) + start;
	result->bf_end    = DeeBytes_DATA(self) + end;
	result->bf_ovrlap = overlapping;
	Dee_Incref(self);
	Dee_Incref(needle);
	DeeObject_Init(result, &BytesCaseFind_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_C_INL */
