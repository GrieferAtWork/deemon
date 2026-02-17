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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#include "segments.c.inl"
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack, DeeArg_Unpack1, UNPuSIZ */
#include <deemon/bool.h>               /* Dee_True, return_bool, return_false, return_true */
#include <deemon/bytes.h>              /* DeeBytes* */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_ErrIndexOutOfBounds */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_AssertTypeExact, DeeObject_TypeAssertFailed, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Incref, Dee_foreach_t, Dee_ssize_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString_AsBytes, DeeString_Check, WSTR_LENGTH */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */

#include <hybrid/align.h>    /* CEILDIV */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "string_functions.h"

#include <stdbool.h> /* false */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeBytesObject, b_str) /* [1..1][const] The Bytes object that is being segmented. */
	size_t                               b_siz; /* [!0][const] The size of a single segment. */
	byte_t const                        *b_ptr; /* [1..1][in(DeeBytes_WSTR(b_str))][lock(ATOMIC)] Pointer to the start of the next segment. */
	byte_t const                        *b_end; /* [1..1][== DeeBytes_WEND(b_str)][const] End pointer. */
} BytesSegmentsIterator;
#define READ_PTR(x) atomic_read(&(x)->b_ptr)

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeBytesObject, b_str) /* [1..1][const] The Bytes object that is being segmented. */
	size_t                               b_siz; /* [!0][const] The size of a single segment. */
} BytesSegments;

INTDEF DeeTypeObject BytesSegmentsIterator_Type;
INTDEF DeeTypeObject BytesSegments_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self,
                  size_t segment_size);


PRIVATE WUNUSED NONNULL((1)) int DCALL
bsegiter_ctor(BytesSegmentsIterator *__restrict self) {
	self->b_str = (DREF DeeBytesObject *)DeeBytes_NewEmpty();
	self->b_siz = 1;
	self->b_ptr = DeeBytes_DATA(self->b_str);
	self->b_end = DeeBytes_DATA(self->b_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsegiter_copy(BytesSegmentsIterator *__restrict self,
              BytesSegmentsIterator *__restrict other) {
	self->b_str = other->b_str;
	self->b_siz = other->b_siz;
	self->b_ptr = READ_PTR(other);
	self->b_end = other->b_end;
	Dee_Incref(self->b_str);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsegiter_init(BytesSegmentsIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	BytesSegments *seg;
	DeeArg_Unpack1(err, argc, argv, "_BytesSegmentsIterator", &seg);
	if (DeeObject_AssertTypeExact(seg, &BytesSegments_Type))
		goto err;
	self->b_str = seg->b_str;
	self->b_siz = seg->b_siz;
	self->b_ptr = DeeBytes_DATA(seg->b_str);
	self->b_end = self->b_ptr + DeeBytes_SIZE(seg->b_str);
	Dee_Incref(self->b_str);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsegiter_next(BytesSegmentsIterator *__restrict self) {
	size_t part_size;
	byte_t const *new_ptr, *ptr;
	do {
		ptr = atomic_read(&self->b_ptr);
		if (ptr >= self->b_end)
			return ITER_DONE;
		new_ptr = ptr + self->b_siz;
	} while (!atomic_cmpxch_weak_or_write(&self->b_ptr, ptr, new_ptr));
	part_size = self->b_siz;
	if (new_ptr > self->b_end) {
		part_size = (size_t)(self->b_end - ptr);
		ASSERT(part_size < self->b_siz);
	}
	return DeeBytes_NewSubView(self->b_str, (void *)ptr, part_size);
}

STATIC_ASSERT(offsetof(BytesSegmentsIterator, b_str) == offsetof(ProxyObject, po_obj));
#define bsegiter_fini  generic_proxy__fini
#define bsegiter_visit generic_proxy__visit

STATIC_ASSERT(offsetof(StringSegmentsIterator, s_str) == offsetof(BytesSegmentsIterator, b_str));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_siz) == offsetof(BytesSegmentsIterator, b_siz));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_ptr) == offsetof(BytesSegmentsIterator, b_ptr));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_end) == offsetof(BytesSegmentsIterator, b_end));
#define bsegiter_bool ssegiter_bool
#define bsegiter_cmp  ssegiter_cmp

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsegiter_serialize(BytesSegmentsIterator *__restrict self,
                   DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(BytesSegmentsIterator, field))
	BytesSegmentsIterator *out;
	out = DeeSerial_Addr2Mem(writer, addr, BytesSegmentsIterator);
	out->b_siz = self->b_siz;
	if (DeeSerial_PutObject(writer, ADDROF(b_str), self->b_str))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(b_ptr), atomic_read(&self->b_ptr)))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(b_end), self->b_end);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesSegments *DCALL
bsegiter_getseq(BytesSegmentsIterator *__restrict self) {
	return (DREF BytesSegments *)DeeBytes_Segments(self->b_str,
	                                               self->b_siz);
}

PRIVATE struct type_getset tpconst bsegiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &bsegiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:BytesSegments"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject BytesSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSegmentsIterator",
	/* .tp_doc      = */ DOC("(seg:?Ert:BytesSegments)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesSegmentsIterator,
			/* tp_ctor:        */ &bsegiter_ctor,
			/* tp_copy_ctor:   */ &bsegiter_copy,
			/* tp_any_ctor:    */ &bsegiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bsegiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsegiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bsegiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bsegiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bsegiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsegiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bsegiter_getsets,
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
bseg_ctor(BytesSegments *__restrict self) {
	self->b_str = (DREF DeeBytesObject *)DeeBytes_NewEmpty();
	self->b_siz = 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bseg_init(BytesSegments *__restrict self,
          size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_BytesSegments", &self->b_str, &self->b_siz))
		goto err;
	if (DeeObject_AssertTypeExact(self->b_str, &DeeBytes_Type))
		goto err;
	if (!self->b_siz) {
		err_invalid_segment_size(self->b_siz);
		goto err;
	}
	Dee_Incref(self->b_str);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(BytesSegments, b_str) == offsetof(ProxyObject, po_obj));
#define bseg_fini      generic_proxy__fini
#define bseg_visit     generic_proxy__visit
#define bseg_serialize generic_proxy__serialize_and_memcpy

PRIVATE WUNUSED NONNULL((1)) int DCALL
bseg_bool(BytesSegments *__restrict self) {
	return !DeeBytes_IsEmpty(self->b_str);
}

PRIVATE WUNUSED NONNULL((1)) DREF BytesSegmentsIterator *DCALL
bseg_iter(BytesSegments *__restrict self) {
	DREF BytesSegmentsIterator *result;
	result = DeeObject_MALLOC(BytesSegmentsIterator);
	if unlikely(!result)
		goto done;
	Dee_Incref(self->b_str);
	result->b_str = self->b_str;
	result->b_siz = self->b_siz;
	result->b_ptr = DeeBytes_DATA(self->b_str);
	result->b_end = result->b_ptr + DeeBytes_SIZE(self->b_str);
	DeeObject_Init(result, &BytesSegmentsIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bseg_size(BytesSegments *__restrict self) {
	size_t length;
	length = DeeBytes_SIZE(self->b_str);
	length = CEILDIV(length, self->b_siz);
	return length;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bseg_contains(BytesSegments *self, DeeObject *other) {
	byte_t const *other_data, *iter, *end;
	DeeBytesObject *str;
	size_t other_size;
	if (DeeBytes_Check(other)) {
		other_data = DeeBytes_DATA(other);
		other_size = DeeBytes_SIZE(other);
	} else if (DeeString_Check(other)) {
		other_data = DeeString_AsBytes(other, false);
		if unlikely(!other_data)
			goto err;
		other_size = WSTR_LENGTH(other_data);
	} else {
		DeeObject_TypeAssertFailed(other, &DeeBytes_Type);
		goto err;
	}
	str = self->b_str;
	if (other_size != self->b_siz) {
		size_t last_part, last_index;
		if (other_size > self->b_siz)
			goto do_return_false;
		last_part = DeeBytes_SIZE(str);
		last_part %= self->b_siz;
		if (other_size != last_part)
			goto do_return_false;
		last_index = DeeBytes_SIZE(str) - last_part;
		return_bool(MEMEQB(DeeBytes_DATA(str) + last_index, other_data, last_part));
	}
	iter = DeeBytes_DATA(str);
	end  = iter + DeeBytes_SIZE(str) - self->b_siz;
	for (; iter <= end; iter += self->b_siz) {
		if (MEMEQB(iter, other_data, self->b_siz))
			goto do_return_true;
	}
do_return_false:
	return_false;
do_return_true:
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
bseg_getitem_index(BytesSegments *__restrict self, size_t index) {
	size_t length;
	length = DeeBytes_SIZE(self->b_str);
	length = CEILDIV(length, self->b_siz);
	if unlikely(index >= length)
		goto err_index;
	index *= self->b_siz;
	return bytes_getsubstr(self->b_str, index, index + self->b_siz);
err_index:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, length);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bseg_foreach(BytesSegments *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, length;
	length = DeeBytes_SIZE(self->b_str);
	length = CEILDIV(length, self->b_siz);
	for (i = 0; i < length; ++i) {
		DREF Bytes *elem;
		size_t start = i * self->b_siz;
		size_t end   = start + self->b_siz;
		elem = bytes_getsubstr(self->b_str, start, end);
		if unlikely(!elem)
			goto err;
		temp = (*proc)(arg, (DeeObject *)elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}



PRIVATE struct type_seq bseg_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bseg_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bseg_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&bseg_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bseg_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&bseg_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bseg_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
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


PRIVATE struct type_member tpconst bseg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT_AB, offsetof(BytesSegments, b_str), "->?DBytes"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesSegments, b_siz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bseg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesSegmentsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeBytes_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject BytesSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSegments",
	/* .tp_doc      = */ DOC("(s:?DBytes,siz:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesSegments,
			/* tp_ctor:        */ &bseg_ctor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bseg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bseg_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bseg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bseg_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bseg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__D73F4EB3E327B6B5),
	/* .tp_seq           = */ &bseg_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bseg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bseg_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self,
                  size_t segment_size) {
	DREF BytesSegments *result;
	ASSERT(segment_size != 0);
	result = DeeObject_MALLOC(BytesSegments);
	if unlikely(!result)
		goto done;
	result->b_str = self;
	Dee_Incref(self);
	result->b_siz = segment_size;
	DeeObject_Init(result, &BytesSegments_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL */
