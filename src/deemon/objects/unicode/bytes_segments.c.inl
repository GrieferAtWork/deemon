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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL 1

#ifdef __INTELLISENSE__
#include "bytes_functions.c.inl"
#include "segments.c.inl"
#include "string_functions.c"
#endif /* __INTELLISENSE__ */

#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>

#include "../../runtime/strings.h"

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

typedef struct {
	OBJECT_HEAD
	DREF DeeBytesObject *b_str;   /* [1..1][const] The Bytes object that is being segmented. */
	size_t               b_siz;   /* [!0][const] The size of a single segment. */
	DWEAK byte_t        *b_ptr;   /* [1..1][in(DeeBytes_WSTR(b_str))] Pointer to the start of the next segment. */
	byte_t              *b_end;   /* [1..1][== DeeBytes_WEND(b_str)] End pointer. */
} BytesSegmentsIterator;

#define READ_PTR(x) atomic_read(&(x)->b_ptr)

typedef struct {
	OBJECT_HEAD
	DREF DeeBytesObject *b_str; /* [1..1][const] The Bytes object that is being segmented. */
	size_t               b_siz; /* [!0][const] The size of a single segment. */
} BytesSegments;

STATIC_ASSERT(offsetof(StringSegmentsIterator, s_str) == offsetof(BytesSegmentsIterator, b_str));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_siz) == offsetof(BytesSegmentsIterator, b_siz));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_ptr) == offsetof(BytesSegmentsIterator, b_ptr));
STATIC_ASSERT(offsetof(StringSegmentsIterator, s_end) == offsetof(BytesSegmentsIterator, b_end));
STATIC_ASSERT(offsetof(StringSegments, s_str) == offsetof(BytesSegments, b_str));
STATIC_ASSERT(offsetof(StringSegments, s_siz) == offsetof(BytesSegments, b_siz));



INTDEF DeeTypeObject BytesSegmentsIterator_Type;
INTDEF DeeTypeObject BytesSegments_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self,
                  size_t segment_size);


PRIVATE WUNUSED NONNULL((1)) int DCALL
bsegiter_ctor(BytesSegmentsIterator *__restrict self) {
	self->b_str = (DREF DeeBytesObject *)Dee_EmptyBytes;
	self->b_siz = 1;
	self->b_ptr = DeeBytes_DATA(Dee_EmptyBytes);
	self->b_end = DeeBytes_DATA(Dee_EmptyBytes);
	Dee_Incref(Dee_EmptyBytes);
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
	if (DeeArg_Unpack(argc, argv, "o:_BytesSegmentsIterator", &seg))
		goto err;
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
	byte_t *new_ptr, *ptr;
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
	return DeeBytes_NewSubView(self->b_str, ptr, part_size);
}

#define bsegiter_fini ssegiter_fini
#define bsegiter_bool ssegiter_bool

PRIVATE NONNULL((1, 2)) void DCALL
bsegiter_visit(BytesSegmentsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->b_str);
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

#define bsegiter_cmp ssegiter_cmp

INTERN DeeTypeObject BytesSegmentsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSegmentsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bsegiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bsegiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bsegiter_init,
				TYPE_FIXED_ALLOCATOR(BytesSegmentsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsegiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bsegiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsegiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bsegiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsegiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ bsegiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#undef READ_PTR



PRIVATE WUNUSED NONNULL((1)) int DCALL
bseg_ctor(BytesSegments *__restrict self) {
	self->b_str = (DREF DeeBytesObject *)Dee_EmptyBytes;
	self->b_siz = 1;
	Dee_Incref(Dee_EmptyBytes);
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

#define bseg_fini sseg_fini
PRIVATE NONNULL((1, 2)) void DCALL
bseg_visit(BytesSegments *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->b_str);
}

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
	byte_t *other_data, *iter, *end;
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
	err_index_out_of_bounds((DeeObject *)self, index, length);
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
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bseg_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&bseg_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bseg_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&bseg_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bseg_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};


PRIVATE struct type_member tpconst bseg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(BytesSegments, b_str), "->?DBytes"),
	TYPE_MEMBER_FIELD("__len__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(BytesSegments, b_siz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bseg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesSegmentsIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject BytesSegments_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesSegments",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bseg_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bseg_init,
				TYPE_FIXED_ALLOCATOR(BytesSegments)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bseg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bseg_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bseg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &bseg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bseg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bseg_class_members
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SEGMENTS_C_INL */
