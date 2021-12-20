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
#ifndef GUARD_DEEMON_OBJECTS_BYTES_C
#define GUARD_DEEMON_OBJECTS_BYTES_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeBytesObject Bytes;

typedef struct {
	OBJECT_HEAD
	DREF Bytes    *bi_bytes; /* [1..1][const] The Bytes object being iterated. */
	DWEAK uint8_t *bi_iter;  /* [1..1][in(bi_bytes->b_base)] Pointer to the next byte to-be iterated. */
	uint8_t       *bi_end;   /* [1..1][const][== bi_bytes->b_base + bi_bytes->b_size]
	                          * Pointer to one byte past the end of the Bytes object being iterated. */
} BytesIterator;

INTDEF DeeTypeObject BytesIterator_Type;


PRIVATE NONNULL((1)) void DCALL
bytesiter_fini(BytesIterator *__restrict self) {
	Dee_Decref(self->bi_bytes);
}

PRIVATE NONNULL((1, 2)) void DCALL
bytesiter_visit(BytesIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bi_bytes);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytesiter_next(BytesIterator *__restrict self) {
	uint8_t *pos;
	do {
		pos = self->bi_iter;
		if (pos >= self->bi_end)
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->bi_iter, pos, pos + 1));
	return DeeInt_NewU8(*pos);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytesiter_ctor(BytesIterator *__restrict self) {
	self->bi_bytes = (DREF Bytes *)Dee_EmptyBytes;
	self->bi_iter  = self->bi_bytes->b_data;
	self->bi_end   = self->bi_bytes->b_data;
	Dee_Incref(Dee_EmptyBytes);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytesiter_copy(BytesIterator *__restrict self,
               BytesIterator *__restrict other) {
	self->bi_bytes = other->bi_bytes;
#ifdef CONFIG_NO_THREADS
	self->bi_iter = other->bi_iter;
#else /* CONFIG_NO_THREADS */
	self->bi_iter = ATOMIC_READ(other->bi_iter);
#endif /* !CONFIG_NO_THREADS */
	self->bi_end = other->bi_end;
	Dee_Incref(self->bi_bytes);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytesiter_init(BytesIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	Bytes *bytes;
	if (DeeArg_Unpack(argc, argv, "o:_BytesIterator", &bytes))
		goto err;
	if (DeeObject_AssertTypeExact(bytes, &DeeBytes_Type))
		goto err;
	self->bi_bytes = bytes;
	Dee_Incref(bytes);
	self->bi_iter = DeeBytes_DATA(bytes);
	self->bi_end  = DeeBytes_DATA(bytes) + DeeBytes_SIZE(bytes);
	return 0;
err:
	return -1;
}

#define DEFINE_BYTESITER_COMPARE(name, expr)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL          \
	name(BytesIterator *self, BytesIterator *other) {              \
		if (DeeObject_AssertTypeExact(other, &BytesIterator_Type)) \
			goto err;                                              \
		return_bool(expr);                                         \
	err:                                                           \
		return NULL;                                               \
	}
DEFINE_BYTESITER_COMPARE(bytesiter_eq, (self->bi_bytes == other->bi_bytes && self->bi_iter == other->bi_iter));
DEFINE_BYTESITER_COMPARE(bytesiter_ne, (self->bi_bytes != other->bi_bytes || self->bi_iter != other->bi_iter));
DEFINE_BYTESITER_COMPARE(bytesiter_lo, (self->bi_bytes < other->bi_bytes || (self->bi_bytes == other->bi_bytes && self->bi_iter < other->bi_iter)));
DEFINE_BYTESITER_COMPARE(bytesiter_le, (self->bi_bytes < other->bi_bytes || (self->bi_bytes == other->bi_bytes && self->bi_iter <= other->bi_iter)));
DEFINE_BYTESITER_COMPARE(bytesiter_gr, (self->bi_bytes > other->bi_bytes || (self->bi_bytes == other->bi_bytes && self->bi_iter > other->bi_iter)));
DEFINE_BYTESITER_COMPARE(bytesiter_ge, (self->bi_bytes > other->bi_bytes || (self->bi_bytes == other->bi_bytes && self->bi_iter >= other->bi_iter)));
#undef DEFINE_BYTESITER_COMPARE

PRIVATE struct type_cmp bytesiter_cmp = {
    /* Compare operators. */
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytesiter_ge,
};

PRIVATE struct type_member tpconst bytesiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BytesIterator, bi_bytes), "->?DBytes"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject BytesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_BytesIterator",
	/* .tp_doc      = */ DOC("next->?Dint"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bytesiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bytesiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bytesiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bytesiter_init,
				TYPE_FIXED_ALLOCATOR(BytesIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bytesiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bytesiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bytesiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytesiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bytesiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSeq_ItemsToBytes)(uint8_t *__restrict dst, size_t num_bytes,
                            DeeObject *__restrict seq) {
	size_t i, fast_size;
	int error;
	DREF DeeObject *iter, *elem;
	if (DeeNone_Check(seq)) {
		/* Special case: `none' */
		bzero(dst, num_bytes);
		return 0;
	}

	if (DeeString_Check(seq)) {
		/* Special case: `string' */
		uint8_t *data = DeeString_AsBytes(seq, false);
		if unlikely(!data)
			goto err;
		if (WSTR_LENGTH(data) != num_bytes) {
			err_invalid_unpack_size(seq, num_bytes, WSTR_LENGTH(data));
			goto err;
		}
		memcpy(dst, data, num_bytes);
		return 0;
	}

	if (DeeBytes_Check(seq)) {
		/* Optional optimization for `Bytes' (though this one
		 * would also function using the fallback code below). */
		if (DeeBytes_SIZE(seq) != num_bytes) {
			err_invalid_unpack_size(seq, num_bytes, DeeBytes_SIZE(seq));
			goto err;
		}
		/* Use `memmove', because `seq' may be a view of `dst' */
		memmove(dst, DeeBytes_DATA(seq), num_bytes);
		return 0;
	}

	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		if (fast_size != num_bytes) {
			err_invalid_unpack_size(seq, num_bytes, fast_size);
			goto err;
		}
		/* Fast-sequence optimizations. */
		for (i = 0; i < fast_size; ++i) {
			elem = DeeFastSeq_GetItem(seq, i);
			if unlikely(!elem)
				goto err;
			error = DeeObject_AsUInt8(elem, &dst[i]);
			Dee_Decref(elem);
			if unlikely(error)
				goto err;
		}
		return 0;
	}

	/* Fallback: use an iterator. */
	iter = DeeObject_IterSelf(seq);
	if unlikely(!iter)
		goto err;
	i = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		if (i >= num_bytes) {
			err_invalid_unpack_iter_size(seq, iter, num_bytes);
			Dee_Decref(elem);
			goto err_iter;
		}
		error = DeeObject_AsUInt8(elem, &dst[i]);
		Dee_Decref(elem);
		if unlikely(error)
			goto err_iter;
		++i;
	}
	if unlikely(!elem)
		goto err_iter;
	if unlikely(i != num_bytes) {
		err_invalid_unpack_size(seq, num_bytes, i);
		goto err_iter;
	}
	Dee_Decref(iter);
	return 0;
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_FromSequence(DeeObject *__restrict seq) {
	DREF Bytes *result;
	size_t i, bufsize;
	DREF DeeObject *iter, *elem;
	if (DeeNone_Check(seq))
		return_empty_bytes;
	bufsize = DeeFastSeq_GetSize(seq);
	if (bufsize != DEE_FASTSEQ_NOTFAST) {
		result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data) +
		                                        bufsize);
		if unlikely(!result)
			goto err;
		/* Fast-sequence optimizations. */
		for (i = 0; i < bufsize; ++i) {
			int error;
			elem = DeeFastSeq_GetItem(seq, i);
			if unlikely(!elem)
				goto err_r;
			error = DeeObject_AsUInt8(elem, &result->b_data[i]);
			Dee_Decref(elem);
			if unlikely(error)
				goto err_r;
		}
		goto done;
	}
	/* Fallback: use an iterator. */
	bufsize = 256;
	result  = (DREF Bytes *)DeeObject_TryMalloc(offsetof(Bytes, b_data) + bufsize);
	if unlikely(!bufsize) {
		bufsize = 1;
		result  = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data) + 1);
		if unlikely(!result)
			goto err;
	}
	iter = DeeObject_IterSelf(seq);
	if unlikely(!iter)
		goto err_r;
	i = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		ASSERT(i <= bufsize);
		if (i >= bufsize) {
			DREF Bytes *new_result;
			size_t new_bufsize = bufsize * 2;
			/* Must allocate more memory. */
			new_result = (DREF Bytes *)DeeObject_TryRealloc(result,
			                                                offsetof(Bytes, b_data) +
			                                                new_bufsize);
			if unlikely(!new_result) {
				new_bufsize = i + 1;
				new_result = (DREF Bytes *)DeeObject_Realloc(result,
				                                             offsetof(Bytes, b_data) +
				                                             new_bufsize);
				if unlikely(!new_result)
					goto err_r_elem;
			}
			result  = new_result;
			bufsize = new_bufsize;
		}
		if unlikely(DeeObject_AsUInt8(elem, &result->b_data[i]))
			goto err_r_elem;
		Dee_Decref(elem);
		++i;
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iter);
	/* Free unused buffer memory. */
	if likely(i < bufsize) {
		DREF Bytes *new_result;
		new_result = (DREF Bytes *)DeeObject_TryRealloc(result,
		                                                offsetof(Bytes, b_data) +
		                                                i);
		if likely(new_result)
			result = new_result;
	}
done:
	result->b_base           = result->b_data;
	result->b_size           = i;
	result->b_orig           = (DREF DeeObject *)result;
	result->b_flags          = Dee_BUFFER_FWRITABLE;
	result->b_buffer.bb_base = result->b_data;
	result->b_buffer.bb_size = i;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	DeeObject_Init(result, &DeeBytes_Type);
	return (DREF DeeObject *)result;
err_r_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_Decref(iter);
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}



/* Construct a bytes-buffer from `self', using the generic object-buffer interface. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Bytes(DeeObject *__restrict self,
                unsigned int flags,
                size_t start, size_t end) {
	DREF Bytes *result;
	ASSERTF(!(flags & ~(Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE)),
	        "Invalid flags %x", flags);
	ASSERT_OBJECT(self);
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data));
	if unlikely(!result)
		goto done;
	if (DeeObject_GetBuf(self, &result->b_buffer, flags))
		goto err_r;
	if (start > result->b_buffer.bb_size)
		start = result->b_buffer.bb_size;
	if (end > result->b_buffer.bb_size)
		end = result->b_buffer.bb_size;
	result->b_base  = (uint8_t *)result->b_buffer.bb_base + start;
	result->b_size  = (size_t)(end - start);
	result->b_orig  = self;
	result->b_flags = flags;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_Free(result);
	return NULL;
}

/* Construct a writable bytes-buffer, consisting of a total of `num_bytes' bytes. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeBytes_NewBuffer(size_t num_bytes, uint8_t init) {
	DREF Bytes *result;
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data) +
	                                        num_bytes);
	if unlikely(!result)
		goto done;
	memset(result->b_data, init, num_bytes);
	result->b_base           = result->b_data;
	result->b_size           = num_bytes;
	result->b_orig           = (DREF DeeObject *)result;
	result->b_flags          = Dee_BUFFER_FWRITABLE;
	result->b_buffer.bb_base = result->b_data;
	result->b_buffer.bb_size = num_bytes;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeBytes_NewBufferUninitialized(size_t num_bytes) {
	DREF Bytes *result;
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data) +
	                                        num_bytes);
	if unlikely(!result)
		goto done;
	result->b_base           = result->b_data;
	result->b_size           = num_bytes;
	result->b_orig           = (DREF DeeObject *)result;
	result->b_flags          = Dee_BUFFER_FWRITABLE;
	result->b_buffer.bb_base = result->b_data;
	result->b_buffer.bb_size = num_bytes;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_NewBufferData(void const *__restrict data, size_t num_bytes) {
	DREF Bytes *result;
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data) +
	                                        num_bytes);
	if unlikely(!result)
		goto done;
	result->b_base           = (uint8_t *)memcpy(result->b_data, data, num_bytes);
	result->b_size           = num_bytes;
	result->b_orig           = (DREF DeeObject *)result;
	result->b_flags          = Dee_BUFFER_FWRITABLE;
	result->b_buffer.bb_base = result->b_data;
	result->b_buffer.bb_size = num_bytes;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_ResizeBuffer(/*inherit(on_success)*/ DREF DeeObject *__restrict self,
                      size_t num_bytes) {
	DREF Bytes *result, *new_result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeBytes_Type);
	ASSERT(!DeeObject_IsShared(self));
	result = (DREF Bytes *)self;
	ASSERT(result->b_base == result->b_data);
	ASSERT(result->b_orig == (DREF DeeObject *)result);
	ASSERT(result->b_flags == Dee_BUFFER_FWRITABLE);
	ASSERT(result->b_buffer.bb_base == result->b_data);
	ASSERT(result->b_buffer.bb_size == result->b_size);
again:
	new_result = (DREF Bytes *)DeeObject_TryRealloc(result,
	                                                offsetof(Bytes, b_data) +
	                                                num_bytes);
	if unlikely(!new_result) {
		if (num_bytes <= result->b_size) {
			result->b_size = result->b_buffer.bb_size = num_bytes;
			return (DREF DeeObject *)result;
		}
		if (Dee_CollectMemory(offsetof(Bytes, b_data) + num_bytes))
			goto again;
		return NULL;
	}
	result->b_orig               = (DREF DeeObject *)result;
	new_result->b_buffer.bb_base = new_result->b_base = new_result->b_data;
	new_result->b_buffer.bb_size = new_result->b_size = num_bytes;
	return (DREF DeeObject *)new_result;
}

PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_TruncateBuffer(/*inherit(on_success)*/ DREF DeeObject *__restrict self,
                        size_t num_bytes) {
	DREF Bytes *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeBytes_Type);
	ASSERT(!DeeObject_IsShared(self));
	result = (DREF Bytes *)self;
	ASSERT(result->b_base == result->b_data);
	ASSERT(result->b_orig == (DREF DeeObject *)result);
	ASSERT(result->b_flags == Dee_BUFFER_FWRITABLE);
	ASSERT(result->b_buffer.bb_base == result->b_data);
	ASSERT(result->b_buffer.bb_size == result->b_size);
	ASSERT(num_bytes <= result->b_size);
	if (num_bytes != result->b_size) {
		result = (DREF Bytes *)DeeObject_TryRealloc(result,
		                                            offsetof(Bytes, b_data) +
		                                            num_bytes);
		if unlikely(!result) {
			result         = (DREF Bytes *)self;
			result->b_size = result->b_buffer.bb_size = num_bytes;
			return (DREF DeeObject *)result;
		}
		result->b_orig           = (DREF DeeObject *)result;
		result->b_buffer.bb_base = result->b_base = result->b_data;
		result->b_buffer.bb_size = result->b_size = num_bytes;
	}
	return (DREF DeeObject *)result;
}


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_NewView(DeeObject *owner, void *base,
                 size_t num_bytes, unsigned int flags) {
	DREF Bytes *result;
	ASSERTF(!(flags & ~(Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE)),
	        "Invalid flags %x", flags);
	ASSERT_OBJECT(owner);
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data));
	if unlikely(!result)
		goto done;
	result->b_base           = (uint8_t *)base;
	result->b_size           = num_bytes;
	result->b_orig           = owner;
	result->b_flags          = flags;
	result->b_buffer.bb_base = (uint8_t *)base;
	result->b_buffer.bb_size = num_bytes;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif
	Dee_Incref(owner);
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return (DREF DeeObject *)result;
}


PRIVATE NONNULL((1)) void DCALL
bytes_fini(Bytes *__restrict self) {
	/* Check for special case: we're owning the object buffer outself. */
	if (self->b_orig == (DREF DeeObject *)self)
		return;
	/* Release the object buffer held on `b_orig' */
	DeeObject_PutBuf(self->b_orig, &self->b_buffer, self->b_flags);
	Dee_Decref(self->b_orig);
}

PRIVATE NONNULL((1, 2)) void DCALL
bytes_visit(Bytes *__restrict self, dvisit_t proc, void *arg) {
	/* Check for special case: we're owning the object buffer outself. */
	if (self->b_orig == (DREF DeeObject *)self)
		return;
	Dee_Visit(self->b_orig);
}

PRIVATE WUNUSED DREF Bytes *DCALL bytes_ctor(void) {
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_copy(Bytes *__restrict other) {
	return (DREF Bytes *)DeeBytes_NewBufferData(DeeBytes_DATA(other),
	                                            DeeBytes_SIZE(other));
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_init(size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	unsigned int flags = Dee_BUFFER_FREADONLY;
	size_t start = 0, end = (size_t)-1;
	if (argc >= 2) {
		ob = argv[0];
		if (DeeInt_Check(ob)) {
			uint8_t init;
			if (DeeObject_AsSize(ob, &start))
				goto err;
			if (DeeObject_AsUInt8(argv[1], &init))
				goto err;
			return (DREF Bytes *)DeeBytes_NewBuffer(start, init);
		}
		if (DeeString_Check(argv[1])) {
			char *str = DeeString_STR(argv[1]);
			if (WSTR_LENGTH(str) != 1)
				goto err_invalid_mode;
			if (str[0] == 'r')
				;
			else if (str[0] == 'w')
				flags = Dee_BUFFER_FWRITABLE;
			else {
				goto err_invalid_mode;
			}
			if (argc >= 3) {
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&start))
					goto err;
				if (argc >= 4) {
					if unlikely(argc > 4)
						goto err_args;
					if (DeeObject_AsSSize(argv[3], (dssize_t *)&end))
						goto err;
				}
			}
		} else {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&start))
				goto err;
			if (argc >= 3) {
				if unlikely(argc > 3)
					goto err_args;
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&end))
					goto err;
			}
		}
	} else if (!argc) {
err_args:
		err_invalid_argc(DeeString_STR(&str_Bytes),
		                 argc,
		                 1,
		                 4);
		goto err;
	} else {
		DeeTypeObject *tp_iter;
		DREF Bytes *result;
		ob = argv[0];
		if (DeeInt_Check(ob)) {
			if (DeeObject_AsSize(ob, &start))
				goto err;
			return (DREF Bytes *)DeeBytes_NewBufferUninitialized(start);
		}
		tp_iter = Dee_TYPE(ob);
		if (tp_iter == &DeeSuper_Type) {
			tp_iter = DeeSuper_TYPE(ob);
			ob      = DeeSuper_SELF(ob);
		}
		do {
			struct type_buffer *buf = tp_iter->tp_buffer;
			if (buf && buf->tp_getbuf) {
				result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_data));
				if unlikely(!result)
					goto err;
				/* Construct a Bytes object using the buffer interface provided by `ob' */
#ifndef __INTELLISENSE__
				result->b_buffer.bb_put = buf->tp_putbuf;
#endif /* !__INTELLISENSE__ */
				if unlikely((*buf->tp_getbuf)(ob, &result->b_buffer, Dee_BUFFER_FREADONLY))
					goto err_r;
				if (start > result->b_buffer.bb_size)
					start = result->b_buffer.bb_size;
				if (end > result->b_buffer.bb_size)
					end = result->b_buffer.bb_size;
				result->b_base  = (uint8_t *)result->b_buffer.bb_base + start;
				result->b_size  = (size_t)(end - start);
				result->b_orig  = ob;
				result->b_flags = Dee_BUFFER_FREADONLY;
				Dee_Incref(ob);
				DeeObject_Init(result, &DeeBytes_Type);
				return result;
			}
		} while (type_inherit_buffer(tp_iter));
		/* The object does not implement the buffer interface.
		 * -> Instead, construct the Bytes object as a sequence. */
		result = (DREF Bytes *)DeeBytes_FromSequence(ob);
		if likely(result)
			return result;
		/* Translate a NotImplement error to indicate that the buffer
		 * interface is missing, rather than the sequence interface. */
		if (DeeError_Catch(&DeeError_NotImplemented)) {
			tp_iter = Dee_TYPE(argv[0]);
			if (tp_iter == &DeeSuper_Type)
				tp_iter = DeeSuper_TYPE(argv[0]);
			err_unimplemented_operator(tp_iter, OPERATOR_GETBUF);
		}
		goto err;
err_r:
		DeeObject_Free(result);
		goto err;
	}
	return (DREF Bytes *)DeeObject_Bytes(ob, flags, start, end);
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid buffer mode %r",
	                argv[1]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_assign(Bytes *__restrict self,
             DeeObject *__restrict values) {
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	return DeeSeq_ItemsToBytes(DeeBytes_DATA(self),
	                           DeeBytes_SIZE(self),
	                           values);
err_readonly:
	return err_bytes_not_writable((DeeObject *)self);
}


INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeBytes_PrintUtf8(DeeObject *__restrict self,
                   dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
	uint8_t *iter, *end, *flush_start;
	/* Optimizations for special printer targets. */
	if (printer == &bytes_printer_print)
		return bytes_printer_append((struct bytes_printer *)arg, DeeBytes_DATA(self), DeeBytes_SIZE(self));
	if (printer == &unicode_printer_print)
		return unicode_printer_print8((struct unicode_printer *)arg, DeeBytes_DATA(self), DeeBytes_SIZE(self));

	end = (iter = flush_start = DeeBytes_DATA(self)) + DeeBytes_SIZE(self);
	while (iter < end) {
		uint8_t escape_buf[2];
		uint8_t ch = *iter++;
		if (ch < 0x80)
			continue;
		if (flush_start < iter - 1) {
			temp = (*printer)(arg,
			                  (char const *)flush_start,
			                  (size_t)((iter - 1) - flush_start));
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		escape_buf[0] = 0xc0 | ((ch & 0xc0) >> 6);
		escape_buf[1] = 0x80 | (ch & 0x3f);
		temp = (*printer)(arg, (char const *)escape_buf, 2);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		flush_start = iter;
	}
	if (flush_start < end) {
		temp = (*printer)(arg,
		                  (char const *)flush_start,
		                  (size_t)(end - flush_start));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_str(Bytes *__restrict self) {
	return DeeString_NewSized((char *)DeeBytes_DATA(self),
	                          DeeBytes_SIZE(self));
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeBytes_PrintRepr(DeeObject *__restrict self,
                   dformatprinter printer,
                   void *arg) {
	dssize_t temp, result;
	Bytes *me;
	me = (Bytes *)self;
#if 1
	temp = DeeFormat_Quote8(printer, arg,
	                        DeeBytes_DATA(me),
	                        DeeBytes_SIZE(me),
	                        FORMAT_QUOTE_FNORMAL);
	if unlikely(temp < 0)
		goto err;
	result = temp;
	temp   = DeeFormat_PRINT(printer, arg, ".bytes()");
#else
	temp = DeeFormat_PRINT(printer, arg, "Bytes(");
	if unlikely(temp < 0)
		goto err;
	result = temp;
	temp = DeeFormat_Quote8(printer, arg,
	                        DeeBytes_DATA(me),
	                        DeeBytes_SIZE(me),
	                        FORMAT_QUOTE_FNORMAL);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")");
#endif
	if unlikely(temp < 0)
		goto err;
	return result + temp;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_repr(Bytes *__restrict self) {
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(DeeBytes_PrintRepr((DeeObject *)self,
	                               &ascii_printer_print,
	                               &printer) < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_bool(Bytes *__restrict self) {
	return !DeeBytes_IsEmpty(self);
}


PRIVATE WUNUSED NONNULL((1)) DREF BytesIterator *DCALL
bytes_iter(Bytes *__restrict self) {
	DREF BytesIterator *result;
	result = DeeObject_MALLOC(BytesIterator);
	if unlikely(!result)
		goto done;
	result->bi_bytes = self;
	result->bi_iter  = DeeBytes_DATA(self);
	result->bi_end   = DeeBytes_DATA(self) + DeeBytes_SIZE(self);
	Dee_Incref(self);
	DeeObject_Init(result, &BytesIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_size(Bytes *__restrict self) {
	return DeeInt_NewSize(DeeBytes_SIZE(self));
}
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_contains(Bytes *self,
               DeeObject *needle);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_getitem(Bytes *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	if unlikely(i >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, i, DeeBytes_SIZE(self));
		goto err;
	}
	return DeeInt_NewU8(DeeBytes_DATA(self)[i]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_delitem(Bytes *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	if unlikely(i >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, i, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	DeeBytes_DATA(self)[i] = 0;
	return 0;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
bytes_setitem(Bytes *self, DeeObject *index, DeeObject *value) {
	size_t i;
	uint8_t val;
	if (DeeObject_AsSize(index, &i))
		goto err;
	if (DeeObject_AsUInt8(value, &val))
		goto err;
	if unlikely(i >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, i, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	DeeBytes_DATA(self)[i] = val;
	return 0;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF Bytes *DCALL
bytes_getrange(Bytes *self, DeeObject *begin, DeeObject *end) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(begin, &start_index))
		goto err;
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if (DeeNone_Check(end)) {
		end_index = (dssize_t)DeeBytes_SIZE(self);
	} else {
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
		if unlikely(end_index < 0)
			end_index += DeeBytes_SIZE(self);
		if ((size_t)end_index > DeeBytes_SIZE(self))
			end_index = (dssize_t)DeeBytes_SIZE(self);
	}
	if ((size_t)start_index >= (size_t)end_index)
		return_reference_((Bytes *)Dee_EmptyBytes);
	if ((size_t)start_index == 0 &&
	    (size_t)end_index == DeeBytes_SIZE(self))
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
	                                      self->b_base + (size_t)start_index,
	                                      (size_t)(end_index - start_index),
	                                      self->b_flags);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
bytes_setrange(Bytes *self, DeeObject *begin,
               DeeObject *end, DeeObject *value) {
	dssize_t start_index;
	dssize_t end_index = (dssize_t)DeeBytes_SIZE(self);
	uint8_t *dst;
	size_t size;
	if (DeeObject_AsSSize(begin, &start_index))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if unlikely(end_index < 0)
		end_index += DeeBytes_SIZE(self);
	if unlikely((size_t)start_index >= DeeBytes_SIZE(self) ||
	            (size_t)start_index >= (size_t)end_index) {
		start_index = 0;
		end_index   = 0;
	} else if unlikely((size_t)end_index > DeeBytes_SIZE(self)) {
		end_index = (dssize_t)DeeBytes_SIZE(self);
	}
	size = (size_t)(end_index - start_index);
	dst  = DeeBytes_DATA(self) + (size_t)start_index;
	return DeeSeq_ItemsToBytes(dst, size, value);
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
bytes_delrange(Bytes *self, DeeObject *begin, DeeObject *end) {
	return bytes_setrange(self, begin, end, Dee_None);
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
bytes_hash(Bytes *__restrict self) {
	return Dee_HashPtr(DeeBytes_DATA(self),
	                   DeeBytes_SIZE(self));
}

INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
string_eq_bytes(DeeStringObject *self,
                DeeBytesObject *other);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
compare_string_bytes(DeeStringObject *lhs,
                     DeeBytesObject *rhs);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_eq(Bytes *self, DeeObject *other) {
	uint8_t *other_data;
	size_t other_size;
	if (DeeString_Check(other))
		return_bool(string_eq_bytes((DeeStringObject *)other, self));
	if (DeeObject_AssertTypeExact(other, &DeeBytes_Type))
		goto err;
	other_data = DeeBytes_DATA(other);
	other_size = DeeBytes_SIZE(other);
	if (DeeBytes_SIZE(self) != other_size)
		return_false;
	return_bool(memcmp(DeeBytes_DATA(self), other_data, other_size) == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_ne(Bytes *self, DeeObject *other) {
	uint8_t *other_data;
	size_t other_size;
	if (DeeString_Check(other))
		return_bool(!string_eq_bytes((DeeStringObject *)other, self));
	if (DeeObject_AssertTypeExact(other, &DeeBytes_Type))
		goto err;
	other_data = DeeBytes_DATA(other);
	other_size = DeeBytes_SIZE(other);
	if (DeeBytes_SIZE(self) != other_size)
		return_true;
	return_bool(memcmp(DeeBytes_DATA(self), other_data, other_size) != 0);
err:
	return NULL;
}

#undef memxcmp
#define memxcmp dee_memxcmp
LOCAL int DCALL
dee_memxcmp(void const *a, size_t asiz,
            void const *b, size_t bsiz) {
	int result = memcmp(a, b, MIN(asiz, bsiz));
	if (result)
		return result;
	if (asiz == bsiz)
		return 0;
	if (asiz < bsiz)
		return -1;
	return 1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
bytes_compare(Bytes *lhs, DeeObject *rhs) {
	uint8_t *other_data;
	size_t other_size;
	if (DeeString_Check(rhs))
		return -compare_string_bytes((DeeStringObject *)rhs, lhs);
	if (DeeObject_AssertTypeExact(rhs, &DeeBytes_Type))
		goto err;
	other_data = DeeBytes_DATA(rhs);
	other_size = DeeBytes_SIZE(rhs);
	return memxcmp(DeeBytes_DATA(lhs),
	               DeeBytes_SIZE(lhs),
	               other_data,
	               other_size);
err:
	return -2;
}


#ifdef __OPTIMIZE_SIZE__
#define DEFINE_BYTES_COMPARE(name, op)                   \
	INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Bytes *self, DeeObject *other) {                \
		int diff = bytes_compare(self, other);           \
		if unlikely(diff == -2)                          \
			goto err;                                    \
		return_bool(diff op 0);                          \
	err:                                                 \
		return NULL;                                     \
	}
#else /* __OPTIMIZE_SIZE__ */
#define DEFINE_BYTES_COMPARE(name, op)                                              \
	INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                            \
	name(Bytes *self, DeeObject *other) {                                           \
		uint8_t *other_data;                                                        \
		size_t other_size;                                                          \
		if (DeeString_Check(other))                                                 \
			return_bool(0 op compare_string_bytes((DeeStringObject *)other, self)); \
		if (DeeObject_AssertTypeExact(other, &DeeBytes_Type))                       \
			goto err;                                                               \
		other_data = DeeBytes_DATA(other);                                          \
		other_size = DeeBytes_SIZE(other);                                          \
		return_bool(memxcmp(DeeBytes_DATA(self),                                    \
		                    DeeBytes_SIZE(self),                                    \
		                    other_data,                                             \
		                    other_size) op 0);                                      \
	err:                                                                            \
		return NULL;                                                                \
	}
#endif /* !__OPTIMIZE_SIZE__ */
DEFINE_BYTES_COMPARE(bytes_lo, <)
DEFINE_BYTES_COMPARE(bytes_le, <=)
DEFINE_BYTES_COMPARE(bytes_gr, >)
DEFINE_BYTES_COMPARE(bytes_ge, >=)
#undef DEFINE_BYTES_COMPARE

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_add(Bytes *self, DeeObject *other) {
	DREF Bytes *result;
	DeeBuffer buffer;
	if (DeeObject_GetBuf(other, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self) +
	                                                       buffer.bb_size);
	if likely(result) {
		memcpy(result->b_data, DeeBytes_DATA(self), DeeBytes_SIZE(self));
		memcpy(result->b_data + DeeBytes_SIZE(self), buffer.bb_base, buffer.bb_size);
	}
	DeeObject_PutBuf(other, &buffer, Dee_BUFFER_FREADONLY);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_mul(Bytes *self, DeeObject *other) {
	DREF Bytes *result;
	uint8_t *dst, *src;
	size_t my_length, total_length, repeat;
	if (DeeObject_AsSize(other, &repeat))
		goto err;
	if (!repeat)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	if (repeat == 1)
		return bytes_copy(self);
	my_length = DeeString_SIZE(self);
	if (OVERFLOW_UMUL(my_length, repeat, &total_length))
		goto err_overflow;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(total_length);
	if unlikely(!result)
		goto err;
	src = DeeBytes_DATA(self);
	dst = DeeBytes_DATA(result);
	while (repeat--) {
		memcpy(dst, src, my_length);
		dst += my_length;
	}
	return result;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
DeeString_CFormat(dformatprinter printer,
                  dformatprinter format_printer, void *arg,
                  /*utf-8*/ char const *__restrict format, size_t format_len,
                  size_t argc, DeeObject *const *argv);

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_mod(Bytes *self, DeeObject *args) {
	struct bytes_printer printer = BYTES_PRINTER_INIT;
	DeeObject *const *argv;
	size_t argc;
	/* C-style string formating */
	if (DeeTuple_Check(args)) {
		argv = DeeTuple_ELEM(args);
		argc = DeeTuple_SIZE(args);
	} else {
		argv = (DeeObject **)&args;
		argc = 1;
	}
	/* Use a different printer for format-copy-characters, thus allowing
	 * us to not need to both encoding the bytes from `self' as UTF-8. */
	if unlikely(DeeString_CFormat(&bytes_printer_print,
		                           (dformatprinter)&bytes_printer_append,
		                           &printer,
		                           (char const *)DeeBytes_DATA(self),
		                           DeeBytes_SIZE(self),
		                           argc,
		                           argv) < 0)
	goto err;
	return (DREF Bytes *)bytes_printer_pack(&printer);
err:
	bytes_printer_fini(&printer);
	return NULL;
}

PRIVATE struct type_math bytes_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_add,
	/* .tp_sub    = */ NULL,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_mul,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_mod,
	/* .tp_shl    = */ NULL,
	/* .tp_shr    = */ NULL,
	/* .tp_and    = */ NULL,
	/* .tp_or     = */ NULL,
	/* .tp_xor    = */ NULL,
	/* .tp_pow    = */ NULL
};

PRIVATE struct type_cmp bytes_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&bytes_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_ge
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bytes_nsi_getsize(Bytes *__restrict self) {
	ASSERT(DeeBytes_SIZE(self) != (size_t)-1);
	return DeeBytes_SIZE(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_nsi_getitem(Bytes *__restrict self, size_t index) {
	if unlikely(index >= DeeBytes_SIZE(self))
		goto err_bounds;
	return DeeInt_NewU8(DeeBytes_DATA(self)[index]);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, DeeBytes_SIZE(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_nsi_delitem(Bytes *__restrict self, size_t index) {
	if unlikely(index >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, index, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	DeeBytes_DATA(self)[index] = 0;
	return 0;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bytes_nsi_setitem(Bytes *self, size_t index, DeeObject *value) {
	uint8_t val;
	if (DeeObject_AsUInt8(value, &val))
		goto err;
	if unlikely(index >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, index, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	DeeBytes_DATA(self)[index] = val;
	return 0;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_nsi_getrange_i(Bytes *__restrict self,
                     dssize_t start_index,
                     dssize_t end_index) {
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if unlikely(end_index < 0)
		end_index += DeeBytes_SIZE(self);
	if ((size_t)end_index > DeeBytes_SIZE(self))
		end_index = (dssize_t)DeeBytes_SIZE(self);
	if ((size_t)start_index >= (size_t)end_index)
		return_reference_((Bytes *)Dee_EmptyBytes);
	if ((size_t)start_index == 0 &&
	    (size_t)end_index == DeeBytes_SIZE(self))
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
	                                      self->b_base + (size_t)start_index,
	                                      (size_t)(end_index - start_index),
	                                      self->b_flags);
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_nsi_getrange_in(Bytes *__restrict self,
                      dssize_t start_index) {
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if ((size_t)start_index >= DeeBytes_SIZE(self))
		return_reference_((Bytes *)Dee_EmptyBytes);
	if (start_index == 0)
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
	                                      self->b_base + (size_t)start_index,
	                                      (size_t)(DeeBytes_SIZE(self) - start_index),
	                                      self->b_flags);
}



PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
bytes_nsi_setrange_i(Bytes *self,
                     dssize_t start_index,
                     dssize_t end_index,
                     DeeObject *value) {
	uint8_t *dst;
	size_t size;
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if unlikely(end_index < 0)
		end_index += DeeBytes_SIZE(self);
	if unlikely((size_t)start_index >= DeeBytes_SIZE(self) ||
	            (size_t)start_index >= (size_t)end_index) {
		start_index = 0;
		end_index   = 0;
	} else if unlikely((size_t)end_index > DeeBytes_SIZE(self)) {
		end_index = (dssize_t)DeeBytes_SIZE(self);
	}
	size = (size_t)(end_index - start_index);
	dst  = DeeBytes_DATA(self) + (size_t)start_index;
	return DeeSeq_ItemsToBytes(dst, size, value);
err_readonly:
	return err_bytes_not_writable((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bytes_nsi_setrange_in(Bytes *self,
                      dssize_t start_index,
                      DeeObject *value) {
	uint8_t *dst;
	size_t size;
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	if unlikely(start_index < 0)
		start_index += DeeBytes_SIZE(self);
	if unlikely((size_t)start_index >= DeeBytes_SIZE(self))
		start_index = DeeBytes_SIZE(self);
	size = (size_t)(DeeBytes_SIZE(self) - start_index);
	dst  = DeeBytes_DATA(self) + (size_t)start_index;
	return DeeSeq_ItemsToBytes(dst, size, value);
err_readonly:
	return err_bytes_not_writable((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
bytes_nsi_xch(Bytes *self, size_t index, DeeObject *value) {
	uint8_t val, result;
	if (DeeObject_AsUInt8(value, &val))
		goto err;
	if unlikely(index >= DeeBytes_SIZE(self)) {
		err_index_out_of_bounds((DeeObject *)self, index, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
#ifdef CONFIG_NO_THREADS
	result                     = DeeBytes_DATA(self)[index];
	DeeBytes_DATA(self)[index] = val;
#else /* CONFIG_NO_THREADS */
	result = ATOMIC_XCH(DeeBytes_DATA(self)[index], val);
#endif /* !CONFIG_NO_THREADS */
	return DeeInt_NewU8(result);
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return NULL;
}


PRIVATE struct type_nsi tpconst bytes_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&bytes_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&bytes_nsi_getsize,
			/* .nsi_getitem      = */ (dfunptr_t)&bytes_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)&bytes_nsi_delitem,
			/* .nsi_setitem      = */ (dfunptr_t)&bytes_nsi_setitem,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)&bytes_nsi_getrange_i,
			/* .nsi_getrange_n   = */ (dfunptr_t)&bytes_nsi_getrange_in,
			/* .nsi_setrange     = */ (dfunptr_t)&bytes_nsi_setrange_i,
			/* .nsi_setrange_n   = */ (dfunptr_t)&bytes_nsi_setrange_in,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&bytes_nsi_xch,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq bytes_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytes_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bytes_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bytes_getrange,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bytes_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&bytes_setrange,
	/* .tp_nsi       = */ &bytes_nsi
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_isreadonly(Bytes *__restrict self) {
	return_bool_(!(self->b_flags & Dee_BUFFER_FWRITABLE));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_iswritable(Bytes *__restrict self) {
	return_bool_(self->b_flags & Dee_BUFFER_FWRITABLE);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getfirst(Bytes *__restrict self) {
	if unlikely(DeeBytes_IsEmpty(self)) {
		err_empty_sequence((DeeObject *)self);
		return NULL;
	}
	return DeeInt_NewU8(DeeBytes_DATA(self)[0]);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_setfirst(Bytes *__restrict self, DeeObject *__restrict value) {
	uint8_t int_value;
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	if unlikely(DeeObject_AsUInt8(value, &int_value))
		goto err;
	DeeBytes_DATA(self)[0] = int_value;
	return 0;
err_empty:
	err_empty_sequence((DeeObject *)self);
	goto err;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getlast(Bytes *__restrict self) {
	if unlikely(DeeBytes_IsEmpty(self)) {
		err_empty_sequence((DeeObject *)self);
		return NULL;
	}
	return DeeInt_NewU8(DeeBytes_DATA(self)[DeeBytes_SIZE(self) - 1]);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_setlast(Bytes *__restrict self, DeeObject *__restrict value) {
	uint8_t int_value;
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	if unlikely(!DeeBytes_WRITABLE(self))
		goto err_readonly;
	if unlikely(DeeObject_AsUInt8(value, &int_value))
		goto err;
	DeeBytes_DATA(self)[DeeBytes_SIZE(self) - 1] = int_value;
	return 0;
err_empty:
	err_empty_sequence((DeeObject *)self);
	goto err;
err_readonly:
	err_bytes_not_writable((DeeObject *)self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_delfirst(Bytes *__restrict self) {
	return bytes_setfirst(self, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_dellast(Bytes *__restrict self) {
	return bytes_setlast(self, Dee_None);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_sizeof(Bytes *self) {
	size_t result = offsetof(Bytes, b_data);
	if (self->b_buffer.bb_base == self->b_data)
		result += self->b_buffer.bb_size;
	return DeeInt_NewSize(result);
}


PRIVATE struct type_getset tpconst bytes_getsets[] = {
	{ "isreadonly",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_isreadonly, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to ?t if @this Bytes object cannot be written to") },
	{ "iswritable",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_iswritable, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to ?t if @this Bytes object not be written to (the inverse of ?#isreadonly)") },
	{ "ismutable",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_iswritable, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Alias for ?#iswritable, overriding :Sequence.ismutable") },
	{ DeeString_STR(&str_first),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_getfirst,
	  (int (DCALL *)(DeeObject *__restrict))&bytes_delfirst,
	  (int (DCALL *)(DeeObject *, DeeObject *))&bytes_setfirst,
	  DOC("->?Dint\n"
	      "@throw ValueError @this Bytes object is empty\n"
	      "@throw BufferError Attempted to modify the byte when @this Bytes object is not writable\n"
	      "Access the first byte of @this Bytes object") },
	{ DeeString_STR(&str_last),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_getlast,
	  (int (DCALL *)(DeeObject *__restrict))&bytes_dellast,
	  (int (DCALL *)(DeeObject *, DeeObject *))&bytes_setlast,
	  DOC("->?Dint\n"
	      "@throw ValueError @this Bytes object is empty\n"
	      "@throw BufferError Attempted to modify the byte when @this Bytes object is not writable\n"
	      "Access the last byte of @this Bytes object") },
	{ STR___sizeof__,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_sizeof, NULL, NULL,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member tpconst bytes_members[] = {
	TYPE_MEMBER_FIELD("length", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Bytes, b_size)),
	TYPE_MEMBER_END
};


PRIVATE int DCALL
bytes_getbuf(Bytes *__restrict self,
             DeeBuffer *__restrict info,
             unsigned int flags) {
	if ((flags & Dee_BUFFER_FWRITABLE) &&
	    !(self->b_flags & Dee_BUFFER_FWRITABLE))
		goto err_readonly;
	info->bb_base = DeeBytes_DATA(self);
	info->bb_size = DeeBytes_SIZE(self);
	return 0;
err_readonly:
	return err_bytes_not_writable((DeeObject *)self);
}

PRIVATE struct type_buffer bytes_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bytes_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};


INTDEF struct type_method tpconst bytes_methods[];


PRIVATE WUNUSED DREF Bytes *DCALL
bytes_fromseq(DeeTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:fromseq", &seq))
		goto err;
	return (DREF Bytes *)DeeBytes_FromSequence(seq);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_fromhex(DeeTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject *const *argv) {
	DeeObject *hex_str;
	DREF Bytes *result;
	uint8_t *dst;
	union dcharptr iter, end;
	size_t length;
	if (DeeArg_Unpack(argc, argv, "o:fromhex", &hex_str))
		goto err;
	if (DeeObject_AssertTypeExact(hex_str, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(hex_str)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte(hex_str);
		length   = WSTR_LENGTH(iter.cp8);
		result   = (DREF Bytes *)DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst     = DeeBytes_DATA(result);
		end.cp8 = iter.cp8 + length;
		for (;;) {
			uint8_t byte_value;
			uint8_t ch;
			for (;;) {
				if (iter.cp8 == end.cp8)
					goto done;
				if (!DeeUni_IsSpace(iter.cp8[0]))
					break;
				++iter.cp8;
			}
			ch = *iter.cp8++;
			if (ch >= '0' && ch <= '9') {
				byte_value = (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value = (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value = (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value = trt->ut_digit;
			}
			if (iter.cp8 == end.cp8)
				goto err_unbalanced;
			ch = *iter.cp8++;
			byte_value <<= 4;
			if (ch >= '0' && ch <= '9') {
				byte_value |= (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value |= (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value |= (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value |= trt->ut_digit;
			}
			ASSERT(dst < DeeBytes_TERM(result));
			*dst++ = byte_value;
		}
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16 = DeeString_Get2Byte(hex_str);
		length    = WSTR_LENGTH(iter.cp16);
		result    = (DREF Bytes *)DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst      = DeeBytes_DATA(result);
		end.cp16 = iter.cp16 + length;
		for (;;) {
			uint8_t byte_value;
			uint16_t ch;
			for (;;) {
				if (iter.cp16 == end.cp16)
					goto done;
				if (!DeeUni_IsSpace(iter.cp16[0]))
					break;
				++iter.cp16;
			}
			ch = *iter.cp16++;
			if (ch >= '0' && ch <= '9') {
				byte_value = (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value = (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value = (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value = trt->ut_digit;
			}
			if (iter.cp16 == end.cp16)
				goto err_unbalanced;
			ch = *iter.cp16++;
			byte_value <<= 4;
			if (ch >= '0' && ch <= '9') {
				byte_value |= (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value |= (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value |= (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value |= trt->ut_digit;
			}
			ASSERT(dst < DeeBytes_TERM(result));
			*dst++ = byte_value;
		}
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32 = DeeString_Get4Byte(hex_str);
		length    = WSTR_LENGTH(iter.cp32);
		result    = (DREF Bytes *)DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst      = DeeBytes_DATA(result);
		end.cp32 = iter.cp32 + length;
		for (;;) {
			uint8_t byte_value;
			uint32_t ch;
			for (;;) {
				if (iter.cp32 == end.cp32)
					goto done;
				if (!DeeUni_IsSpace(iter.cp32[0]))
					break;
				++iter.cp32;
			}
			ch = *iter.cp32++;
			if (ch >= '0' && ch <= '9') {
				byte_value = (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value = (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value = (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value = trt->ut_digit;
			}
			if (iter.cp32 == end.cp32)
				goto err_unbalanced;
			ch = *iter.cp32++;
			byte_value <<= 4;
			if (ch >= '0' && ch <= '9') {
				byte_value |= (uint8_t)(ch - '0');
			} else if (ch >= 'a' && ch <= 'f') {
				byte_value |= (uint8_t)(10 + (ch - 'a'));
			} else if (ch >= 'A' && ch <= 'F') {
				byte_value |= (uint8_t)(10 + (ch - 'A'));
			} else {
				struct unitraits *trt;
				trt = DeeUni_Descriptor(ch);
				if (!(trt->ut_flags & UNICODE_FDECIMAL))
					goto err_invalid;
				byte_value |= trt->ut_digit;
			}
			ASSERT(dst < DeeBytes_TERM(result));
			*dst++ = byte_value;
		}
		break;
	}
done:
	{
		size_t used;
		used = (size_t)(dst - DeeBytes_DATA(result));
		ASSERT(used <= DeeBytes_SIZE(result));
		if unlikely(used != DeeBytes_SIZE(result)) {
			result = (DREF Bytes *)DeeBytes_TruncateBuffer((DeeObject *)result, used);
		}
	}
	return result;
err_invalid:
	DeeError_Throwf(&DeeError_ValueError,
	                "Non-hexadecimal character in %r",
	                hex_str);
	goto err;
err_unbalanced:
	DeeError_Throwf(&DeeError_ValueError,
	                "Unbalanced hexadecimal character in %r",
	                hex_str);
err:
	return NULL;
}



PRIVATE struct type_method tpconst bytes_class_methods[] = {
	{ "fromseq", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_fromseq,
	  DOC("(seq:?S?Dint)->?.\n"
	      "@throw NotImplemented The given @seq cannot be iterated, or contains at "
	      "least one item that cannot be converted into an integer\n"
	      "@throw IntegerOverflow At least one of the integers found in @seq is lower "
	      "than $0, or greater than $0xff\n"
	      "Convert the items of the given sequence @seq into integers, "
	      "and construct a writable Bytes object from their values\n"
	      "Passing ?N for @seq will return an empty Bytes object") },
	{ "fromhex", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_fromhex,
	  DOC("(hex_string:?Dstring)->?.\n"
	      "@throw ValueError The given @hex_string contains non-hexadecimal and non-space characters\n"
	      "@throw ValueError The given @hex_string contains an unbalanced hexadecimal digit\n"
	      "Decode a given string containing only digit characters, characters between $\"a\" and $\"f\" "
	      "or $\"A\" and $\"F\", or optional space characters separating pairs of such characters.\n"
	      "Each pair of hexadecimal digits is then interpreted as a byte that is then used to construct "
	      "the resulting Bytes object.\n"
	      "Note that this function is also called by the $\"hex\" codec, meaning that ${string.decode(\"hex\")} "
	      "is the same as calling this functions, while ${Bytes.encode(\"hex\")} is the same as calling ?#hex\n"
	      "${"
	      "local data = \"DEAD BEEF\".decode(\"hex\");\n"
	      "print repr data; /* \"\\xDE\\xAD\\xBE\\xEF\".bytes() */"
	      "}") },
	{ NULL }
};

PRIVATE struct type_member tpconst bytes_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &BytesIterator_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeBytesObject DeeBytes_Empty = {
	OBJECT_HEAD_INIT(&DeeBytes_Type),
	/* .b_base   = */ NULL,
	/* .b_size   = */ 0,
	/* .b_orig   = */ Dee_EmptyBytes,
	/* .b_buffer = */ {
		/* .bb_base = */ NULL,
		/* .bb_size = */ 0,
	},
	/* .b_flags  = */ Dee_BUFFER_FWRITABLE,
	/* .b_data   = */ { 0 }
};


PUBLIC DeeTypeObject DeeBytes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Bytes),
	/* .tp_doc      = */ DOC("An string-like abstract buffer object type for viewing & editing "
	                         "the memory of any object implementing the buffer interface, or "
	                         "to allocate fixed-length buffers for raw bytes which "
	                         "can then be loaded with arbitrary data (most notably "
	                         "through use of :File.readinto)\n"
	                         "A Bytes object implements the sequence interface as a ${{int...}}-like sequence, "
	                         "with each integer being a value between $0 and $0xff\n"

	                         "\n"
	                         "()\n"
	                         "Construct an empty Bytes object\n"

	                         "\n"
	                         "(ob:?O,start=!0,end=!-1)\n"
	                         "(ob:?O,mode=!Pr,start=!0,end=!-1)\n"
	                         "@throw NotImplemented The given @ob does not implement the buffer protocol\n"
	                         "Construct a Bytes object for viewing the memory of @ob, either "
	                         "as read-only when @mode is set to $\"r\" or omitted, or as read-write "
	                         "when set to $\"w\". The section of memory then addressed by the "
	                         "Bytes view starts after @start bytes, and ends at @end bytes.\n"
	                         "If either @start or @end is a negative number, it will refer to the "
	                         "end of the memory found in @ob, using an intentional integer underflow\n"
	                         "Whether or not a Bytes object is read-only, or writable "
	                         "can be determined using ?#iswritable and ?#isreadonly\n"
	                         "Additionally, a Bytes object can be made writable by creating a copy (a.s. ?#{op:copy}), "
	                         "or by using ?#makewritable, which will not create a new Bytes object, but re-return "
	                         "the same object, if it was already writable\n"

	                         "\n"
	                         "(num_bytes:?Dint)\n"
	                         "(num_bytes:?Dint,init:?Dint)\n"
	                         "@throw IntegerOverflow The given @init is negative, or greater than $0xff\n"
	                         "Construct a writable, self-contained Bytes object for a total "
	                         "of @num_bytes bytes of memory, pre-initialized to @init, or left "
	                         "undefined when @init isn't specified\n"

	                         "\n"
	                         "(items:?S?Dint)\n"
	                         "For compatibility with other sequence types, as well as the expectation "
	                         "of a sequence-like object implementing a sequence-cast-constructor, Bytes "
	                         "objects also implement this overload\n"
	                         "However, given the reason for the existence of a Bytes object, as well as "
	                         "the fact that a sequence object may also implement a buffer interface, which "
	                         "the ${Bytes(ob: Object)} overload above makes use of, that overload is always "
	                         "preferred over this one. In situations where this might cause ambiguity, "
	                         "it is recommended that the ?#fromseq class method be used to construct the "
	                         "Bytes object instead.\n"

	                         "\n"
	                         "copy->\n"
	                         "Returns a writable copy of @this Bytes object, containing "
	                         "a copy of all data as a kind of snapshot\n"
	                         "${"
	                         "local x = \"foobar\";\n"
	                         "local y = x.bytes();\n"
	                         "print y[0]; /* 102 */\n"
	                         "y = copy y;\n"
	                         "print y[0]; /* 102 */\n"
	                         "y[0] = 42;  /* Only allowed because the `copy' */\n"
	                         "print y;    /* *oobar */"
	                         "}\n"

	                         "\n"
	                         "str->\n"
	                         "Returns a string variant of @this Bytes object, interpreting each byte as "
	                         "a unicode character from the range U+0000-U+00FF. This is identical "
	                         "to encoding the Bytes object as a latin-1 (or iso-8859-1) string\n"
	                         "This is identical to ${this.encode(\"latin-1\")}\n"

	                         "\n"
	                         "repr->\n"
	                         "Returns the representation of @this Bytes object in the form "
	                         "of ${\"{!r}.bytes()\".format({ this.encode(\"latin-1\") })}\n"

	                         "\n"
	                         "contains(needle:?X3?.?Dstring?Dint)->\n"
	                         "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	                         "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	                         "Check if @needle appears within @this Bytes object\n"

	                         "\n"
	                         ":=(data:?X3?.?Dstring?S?Dint)->\n"
	                         "@throw BufferError @this Bytes object is not writable\n"
	                         "@throw UnpackError The length of the given @data does not equal ${##this}\n"
	                         "Assign the contents of @data to @this Bytes object\n"
	                         "You may pass ?N for @data to clear all bytes of @this buffer\n"

	                         "\n"
	                         "[](index:?Dint)->?Dint\n"
	                         "Returns the byte found at @index as an integer between $0 and $0xff\n"

	                         "\n"
	                         "[]=(index:?Dint,value:?Dint)->\n"
	                         "@throw BufferError @this Bytes object is not writable\n"
	                         "@throw IndexError The given @index is negative, or greater than the length of @this \n"
	                         "@throw IntegerOverflow @value is negative or greater than $0xff\n"
	                         "Taking the integer value of @value, assign that value to the byte found at @index\n"

	                         "\n"
	                         "del[]->\n"
	                         "@throw BufferError @this Bytes object is not writable\n"
	                         "@throw IndexError The given @index is negative, or greater than the length of @this \n"
	                         "Same as ${this[index] = 0}, assigning a zero-byte at the given @index\n"

	                         "\n"
	                         "[:]->?.\n"
	                         "Returns another Bytes view for viewing a sub-range of bytes\n"
	                         "Modifications then made to the returned Bytes object will affect the "
	                         "same memory already described by @this Bytes object\n"

	                         "\n"
	                         "[:]=(start:?Dint,end:?Dint,data:?X3?.?Dstring?S?Dint)->\n"
	                         "@throw BufferError @this Bytes object is not writable\n"
	                         "@throw IntegerOverflow One of the integers extracted from @data is negative, or greater than $0xff\n"
	                         "@throw ValueError @data is a string containing characters ${> 0xff}\n"
	                         "@throw UnpackError The length of the given sequence @data does not match the number of bytes, that is ${end-start}. "
	                         /*              */ "If any, and how many bytes of @this Bytes object were already modified is undefined\n"
	                         "Assign values to a given range of bytes\n"
	                         "You may pass ?N to fill the entire range with zero-bytes\n"

	                         "\n"
	                         "del[:]->\n"
	                         "Same as ${this[start:end] = none}, assigning zero-bytes within the given range\n"

	                         "\n"
	                         "iter->\n"
	                         "Allows for iteration of the individual bytes as integers between $0 and $0xff\n"

	                         "\n"
	                         "<(other:?X2?.?Dstring)->\n"
	                         "<=(other:?X2?.?Dstring)->\n"
	                         "==(other:?X2?.?Dstring)->\n"
	                         "!=(other:?X2?.?Dstring)->\n"
	                         ">(other:?X2?.?Dstring)->\n"
	                         ">=(other:?X2?.?Dstring)->\n"
	                         "@throw ValueError The given @other is a string containing characters ${> 0xff}\n"
	                         "Perform a lexicographical comparison between @this and @other, and return the result\n"

	                         "\n"
	                         "#->\n"
	                         "Returns the number of bytes represented by @this Bytes object\n"

	                         "\n"
	                         "bool->\n"
	                         "Returns ?t if @this Bytes object is non-empty\n"

	                         "\n"
	                         "add->\n"
	                         "Construct a new writable Bytes object that at is the concatenation of @this and @other\n"
	                         "+(other:?X3?.?Dstring?O)->\n"
	                         "Return a new writable Bytes object that is the concatenation of @this and ${str(other).bytes()}\n"

	                         "\n"
	                         "*(times:?Dint)->\n"
	                         "@throw IntegerOverflow @times is negative, or too large\n"
	                         "Returns @this Bytes object repeated @times number of times\n"

	                         "\n"
	                         "%(args:?DTuple)->\n"
	                         "%(arg:?O)->\n"
	                         "@throw UnicodeEncodeError Attempted to print a unicode character ${> 0xff}\n"
	                         "After interpreting the bytes from @this Bytes object as LATIN-1, "
	                         "generate a printf-style format string containing only LATIN-1 characters.\n"
	                         "If @arg isn't a tuple, it is packed into one and the call is identical "
	                         "to ${this.operator % (pack(arg))}\n"
	                         "The returned Bytes object is writable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bytes_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bytes_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bytes_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bytes_init,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bytes_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytes_assign,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bytes_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bytes_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bytes_math,
	/* .tp_cmp           = */ &bytes_cmp,
	/* .tp_seq           = */ &bytes_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &bytes_buffer,
	/* .tp_methods       = */ bytes_methods,
	/* .tp_getsets       = */ bytes_getsets,
	/* .tp_members       = */ bytes_members,
	/* .tp_class_methods = */ bytes_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bytes_class_members
};








/* ================================================================================= */
/*   BYTES PRINTER API                                                               */
/* ================================================================================= */

/* _Always_ inherit all byte data (even upon error) saved in
 * `self', and construct a new Bytes object from all that data, before
 * returning a reference to that object.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `bytes_printer_fini()'
 * @return: * :   A reference to the packed Bytes object.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_bytes_printer_pack(/*inherit(always)*/ struct bytes_printer *__restrict self) {
	DREF Bytes *result = self->bp_bytes;
	if unlikely(!result)
		return_empty_bytes;
	/* Deallocate unused memory. */
	if likely(self->bp_length != result->b_size) {
		DREF Bytes *reloc;
		reloc = (DREF Bytes *)DeeObject_TryRealloc(result,
		                                           offsetof(Bytes, b_data) +
		                                           self->bp_length);
		if likely(reloc)
			result         = reloc;
		result->b_size = self->bp_length;
	}
	/* Do final object initialization. */
	result->b_base           = result->b_data;
	result->b_orig           = (DREF DeeObject *)result;
	result->b_flags          = Dee_BUFFER_FWRITABLE;
	result->b_buffer.bb_base = result->b_data;
	result->b_buffer.bb_size = self->bp_length;
#ifndef __INTELLISENSE__
	result->b_buffer.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	DeeObject_Init(result, &DeeBytes_Type);
	return (DREF DeeObject *)result;
}

/* Append raw byte data to the given bytes-printer, without concern
 * about any kind of encoding. - Just copy over the raw bytes.
 * -> A far as unicode support goes, this function has _nothing_ to
 *    do with any kind of encoding. - It just blindly copies the given
 *    data into the buffer of the resulting Bytes object.
 * -> The equivalent unicode_printer function is `unicode_printer_print8' */
PUBLIC WUNUSED NONNULL((1)) dssize_t DPRINTER_CC
Dee_bytes_printer_append(struct bytes_printer *__restrict self,
                         uint8_t const *__restrict data, size_t datalen) {
	Bytes *bytes;
	size_t alloc_size;
	ASSERT(data || !datalen);
	if ((bytes = self->bp_bytes) == NULL) {
		/* Make sure not to allocate a bytes when the used length remains ZERO.
		 * >> Must be done to assure the expectation of `if(bp_length == 0) bp_bytes == NULL' */
		if unlikely(!datalen)
			return 0;
		/* Allocate the initial bytes. */
		alloc_size = 8;
		while (alloc_size < datalen)
			alloc_size *= 2;
alloc_again:
		bytes = (Bytes *)DeeObject_TryMalloc(offsetof(Bytes, b_data) + alloc_size);
		if unlikely(!bytes) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemory(offsetof(Bytes, b_data) + alloc_size))
				goto alloc_again;
			return -1;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
		memcpy(bytes->b_data, data, datalen);
		self->bp_length = datalen;
		goto done;
	}
	alloc_size = bytes->b_size;
	ASSERT(alloc_size >= self->bp_length);
	alloc_size -= self->bp_length;
	if unlikely(alloc_size < datalen) {
		size_t min_alloc = self->bp_length + datalen;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		bytes = (Bytes *)DeeObject_TryRealloc(bytes, offsetof(Bytes, b_data) + alloc_size);
		if unlikely(!bytes) {
			bytes = self->bp_bytes;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemory(offsetof(Bytes, b_data) + alloc_size))
				goto realloc_again;
			return -1;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
	}
	/* Copy data into the dynamic bytes. */
	memcpy(bytes->b_data + self->bp_length, data, datalen);
	self->bp_length += datalen;
done:
	return (dssize_t)datalen;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_bytes_printer_putb)(struct bytes_printer *__restrict self, uint8_t ch) {
	/* Quick check: Can we print to an existing buffer. */
	if (self->bp_bytes &&
	    self->bp_length < self->bp_bytes->b_size) {
		self->bp_bytes->b_data[self->bp_length++] = ch;
		goto done;
	}
	/* Fallback: go the long route. */
	if (bytes_printer_append(self, (uint8_t *)&ch, 1) < 0)
		goto err;
done:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) dssize_t
(DCALL Dee_bytes_printer_repeat)(struct bytes_printer *__restrict self,
                                 uint8_t ch, size_t count) {
	uint8_t *buffer;
	buffer = bytes_printer_alloc(self, count);
	if unlikely(!buffer)
		goto err;
	/* Simply do a memset to initialize bytes data. */
	memset(buffer, ch, count);
	return (dssize_t)count;
err:
	return -1;
}

PUBLIC NONNULL((1)) void
(DCALL Dee_bytes_printer_release)(struct bytes_printer *__restrict self,
                                  size_t datalen) {
	ASSERT(self->bp_length >= datalen);
	/* This's actually all that needs to be
	 * done with the current implementation. */
	self->bp_length -= datalen;
}

PUBLIC WUNUSED NONNULL((1)) uint8_t *
(DCALL Dee_bytes_printer_alloc)(struct bytes_printer *__restrict self, size_t datalen) {
	Bytes *bytes;
	size_t alloc_size;
	uint8_t *result;
	if ((bytes = self->bp_bytes) == NULL) {
		/* Make sure not to allocate new bytes when the used length remains ZERO.
		 * >> Must be done to assure the expectation of `if(bp_length == 0) bp_bytes == NULL' */
		if unlikely(!datalen)
			return 0;
		/* Allocate the initial bytes. */
		alloc_size = 8;
		while (alloc_size < datalen)
			alloc_size *= 2;
alloc_again:
		bytes = (Bytes *)DeeObject_TryMalloc(offsetof(Bytes, b_data) +
		                                     (alloc_size + 1) * sizeof(char));
		if unlikely(!bytes) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemory(offsetof(Bytes, b_data) +
			                      (alloc_size + 1) * sizeof(char)))
				goto alloc_again;
			return NULL;
		}
		self->bp_bytes  = bytes;
		bytes->b_size   = alloc_size;
		self->bp_length = datalen;
		return bytes->b_data;
	}
	alloc_size = bytes->b_size;
	ASSERT(alloc_size >= self->bp_length);
	alloc_size -= self->bp_length;
	if unlikely(alloc_size < datalen) {
		size_t min_alloc = self->bp_length + datalen;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		bytes = (Bytes *)DeeObject_TryRealloc(bytes, offsetof(Bytes, b_data) +
		                                             (alloc_size + 1) * sizeof(char));
		if unlikely(!bytes) {
			bytes = self->bp_bytes;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemory(offsetof(Bytes, b_data) +
			                      (alloc_size + 1) * sizeof(char)))
				goto realloc_again;
			return NULL;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
	}
	/* Append text at the end. */
	result = bytes->b_data + self->bp_length;
	self->bp_length += datalen;
	return result;
}




DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_BYTES_C */
