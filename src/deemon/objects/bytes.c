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
#ifndef GUARD_DEEMON_OBJECTS_BYTES_C
#define GUARD_DEEMON_OBJECTS_BYTES_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_CollectMemory, Dee_CollectMemoryoc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* return_bool, return_false, return_true */
#include <deemon/bytes.h>              /* DeeBytes*, Dee_BYTES_PRINTER_INIT, Dee_EmptyBytes, Dee_bytes_* */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_* */
#include <deemon/int.h>                /* DeeInt_* */
#include <deemon/method-hints.h>       /* type_method_hint */
#include <deemon/none.h>               /* DeeNone_Check, Dee_None */
#include <deemon/object.h>
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_Type, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* CASE_WIDTH_nBYTE, DeeString*, DeeUni_AsDigit, DeeUni_IsSpace, Dee_ASCII_PRINTER_INIT, Dee_ascii_printer*, Dee_charptr_const, SWITCH_SIZEOF_WIDTH, WSTR_LENGTH */
#include <deemon/stringutils.h>        /* DeeString_GetChar */
#include <deemon/super.h>              /* DeeObject_TGetBuf, DeeSuper* */
#include <deemon/system-features.h>    /* CONFIG_HAVE_*, bcmp, bzero, memcmp, memcpy, memmove, mempcpy, memset, memsetb, memsetl, memsetq, memsetw */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_weak_or_write, atomic_read */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/minmax.h>    /* MIN */
#include <hybrid/overflow.h>  /* OVERFLOW_UADD, OVERFLOW_UMUL */
#include <hybrid/typecore.h>  /* __BYTE_TYPE__ */
#include <hybrid/unaligned.h> /* UNALIGNED_GET* */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "int-8bit.h"

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeBytesObject Bytes;

typedef struct {
	PROXY_OBJECT_HEAD_EX(Bytes, bi_bytes); /* [1..1][const] The Bytes object being iterated. */
	DWEAK byte_t const         *bi_iter;   /* [1..1][in(bi_bytes->b_base)][lock(ATOMIC)] Pointer to the next byte to-be iterated. */
} BytesIterator;

#define BytesIterator_GetIter(self) atomic_read(&(self)->bi_iter)

INTDEF DeeTypeObject BytesIterator_Type;

#define bytesiter_fini  generic_proxy__fini
#define bytesiter_visit generic_proxy__visit

STATIC_ASSERT(offsetof(BytesIterator, bi_bytes) == offsetof(ProxyObjectWithPointer, po_obj));
STATIC_ASSERT(offsetof(BytesIterator, bi_iter) == offsetof(ProxyObjectWithPointer, po_ptr));
#define bytesiter_serialize generic_proxy_with_pointer__serialize_atomic

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytesiter_next(BytesIterator *__restrict self) {
	Bytes *bytes = self->bi_bytes;
	byte_t const *pos, *end;
	byte_t result;
	end = DeeBytes_END(bytes);
	do {
		pos = BytesIterator_GetIter(self);
		if (pos >= end)
			return ITER_DONE;
	} while unlikely(!atomic_cmpxch_weak_or_write(&self->bi_iter,
	                                              pos, pos + 1));
	result = *pos;
	return DeeInt_NEWU(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytesiter_ctor(BytesIterator *__restrict self) {
	self->bi_bytes = (DREF Bytes *)DeeBytes_NewEmpty();
	self->bi_iter  = DeeBytes_BUFFER_DATA(&DeeBytes_Empty);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytesiter_copy(BytesIterator *__restrict self,
               BytesIterator *__restrict other) {
	self->bi_bytes = other->bi_bytes;
	self->bi_iter  = BytesIterator_GetIter(other);
	Dee_Incref(self->bi_bytes);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytesiter_init(BytesIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	Bytes *bytes;
	DeeArg_Unpack1(err, argc, argv, "_BytesIterator", &bytes);
	if (DeeObject_AssertTypeExact(bytes, &DeeBytes_Type))
		goto err;
	self->bi_bytes = bytes;
	Dee_Incref(bytes);
	self->bi_iter = DeeBytes_DATA(bytes);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bytesiter_hash(BytesIterator *self) {
	return Dee_HashCombine(Dee_HashPointer(self->bi_bytes),
	                       Dee_HashPointer(BytesIterator_GetIter(self)));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytesiter_compare(BytesIterator *self, BytesIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BytesIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->bi_bytes, other->bi_bytes);
	Dee_return_compareT(byte_t const *, BytesIterator_GetIter(self),
	                    /*           */ BytesIterator_GetIter(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp bytesiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&bytesiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytesiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst bytesiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(BytesIterator, bi_bytes), "->?DBytes"),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BytesIterator,
			/* tp_ctor:        */ &bytesiter_ctor,
			/* tp_copy_ctor:   */ &bytesiter_copy,
			/* tp_deep_ctor:   */ &bytesiter_copy,
			/* tp_any_ctor:    */ &bytesiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bytesiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bytesiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bytesiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &bytesiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytesiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bytesiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



struct seq_tobytes_foreach_data {
	byte_t *stbf_dst;       /* [0..stbf_num_bytes] Output buffer */
	size_t  stbf_num_bytes; /* # of remaining bytes in `stbf_dst' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_tobytes_foreach_cb(void *arg, DeeObject *item) {
	struct seq_tobytes_foreach_data *data;
	data = (struct seq_tobytes_foreach_data *)arg;
	if likely(data->stbf_num_bytes) {
		if (DeeObject_AsByte(item, data->stbf_dst))
			goto err;
		--data->stbf_num_bytes;
	}
	++data->stbf_dst;
	return 0;
err:
	return -1;
}


/* Unpack the given sequence `seq' into `num_bytes', invoking the
 * `operator int' on each, converting their values into bytes, before
 * storing those bytes in the given `dst' vector.
 * If the length of `seq' doesn't match `num_bytes', an UnpackError is thrown.
 * If `seq' is the none-singleton, `dst...+=num_bytes' is zero-initialized. */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSeq_ItemsToBytes)(byte_t *__restrict dst, size_t num_bytes,
                            DeeObject *__restrict seq) {
	if (DeeNone_Check(seq)) {
		/* Special case: `none' */
		bzero(dst, num_bytes);
	} else if (DeeString_Check(seq)) {
		/* Special case: `string' */
		byte_t const *data = DeeString_AsBytes(seq, false);
		if unlikely(!data)
			goto err;
		if (WSTR_LENGTH(data) != num_bytes) {
			DeeRT_ErrUnpackError(seq, num_bytes, WSTR_LENGTH(data));
			goto err;
		}
		memcpy(dst, data, num_bytes);
	} else if (DeeBytes_Check(seq)) {
		/* Optional optimization for `Bytes' (though this one
		 * would also function using the fallback code below). */
		if (DeeBytes_SIZE(seq) != num_bytes) {
			DeeRT_ErrUnpackError(seq, num_bytes, DeeBytes_SIZE(seq));
			goto err;
		}

		/* Use `memmove', because `seq' may be a view of `dst' */
		memmove(dst, DeeBytes_DATA(seq), num_bytes);
	} else {
		/* Fallback: use DeeObject_Foreach() */
		size_t req_bytes;
		struct seq_tobytes_foreach_data data;
		data.stbf_dst       = dst;
		data.stbf_num_bytes = num_bytes;
		if unlikely(DeeObject_Foreach(seq, &seq_tobytes_foreach_cb, &data))
			goto err;
		req_bytes = (size_t)(data.stbf_dst - dst);
		if unlikely(req_bytes != num_bytes) {
			DeeRT_ErrUnpackError(seq, num_bytes, req_bytes);
			goto err;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
bytes_fromseq_foreach_cb(void *arg, DeeObject *item) {
	byte_t byte;
	struct Dee_bytes_printer *printer = (struct Dee_bytes_printer *)arg;
	int result = DeeObject_AsByte(item, &byte);
	if likely(result == 0)
		result = Dee_bytes_printer_putb(printer, byte);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_FromSequence(DeeObject *__restrict seq) {
	size_t bsize;
	struct Dee_bytes_printer printer;
	if (DeeBytes_Check(seq))
		return Dee_AsObject(DeeBytes_NewBufferData(DeeBytes_DATA(seq), DeeBytes_SIZE(seq)));

	/* Fallback: use an iterator. */
	bsize = DeeObject_SizeFast(seq);
	if (bsize == (size_t)-1)
		bsize = 256;
	Dee_bytes_printer_init_ex(&printer, bsize);
	if unlikely(DeeObject_Foreach(seq, &bytes_fromseq_foreach_cb, &printer))
		goto err_printer;
	return Dee_bytes_printer_pack(&printer);
err_printer:
	Dee_bytes_printer_fini(&printer);
/*err:*/
	return NULL;
}



/* Construct a bytes-buffer from `self', using the generic object-buffer interface. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_Bytes(DeeObject *__restrict self,
                unsigned int flags,
                size_t start, size_t end) {
	DREF Bytes *result;
	DeeBuffer buffer;
	ASSERTF(!(flags & ~(Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE)),
	        "Invalid flags %x", flags);
	ASSERT_OBJECT(self);
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_buffer));
	if unlikely(!result)
		goto done;
	if (DeeObject_GetBuf(self, &buffer, flags))
		goto err_r;
	if (start > buffer.bb_size)
		start = buffer.bb_size;
	if (end > buffer.bb_size)
		end = buffer.bb_size;
	result->b_base  = (byte_t *)buffer.bb_base + start;
	result->b_size  = (size_t)(end - start);
	result->b_orig  = self;
	result->b_flags = flags;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_Free(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_TBytes(DeeTypeObject *tp_self,
                 DeeObject *__restrict self,
                 unsigned int flags,
                 size_t start, size_t end) {
	DREF Bytes *result;
	DeeBuffer buffer;
	ASSERTF(!(flags & ~(Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE)),
	        "Invalid flags %x", flags);
	ASSERT_OBJECT_TYPE(self, tp_self);
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_buffer));
	if unlikely(!result)
		goto done;
	if (DeeObject_TGetBuf(tp_self, self, &buffer, flags))
		goto err_r;
	if (start > buffer.bb_size)
		start = buffer.bb_size;
	if (end > buffer.bb_size)
		end = buffer.bb_size;
	result->b_base  = (byte_t *)buffer.bb_base + start;
	result->b_size  = (size_t)(end - start);
	result->b_orig  = self;
	result->b_flags = flags;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_Free(result);
	return NULL;
}

/* Construct a writable bytes-buffer, consisting of a total of `num_bytes' bytes. */
PUBLIC WUNUSED DREF DeeBytesObject *DCALL
DeeBytes_NewBuffer(size_t num_bytes, byte_t init) {
	DREF DeeBytesObject *result;
	result = (DREF DeeBytesObject *)DeeObject_MalloccSafe(offsetof(DeeBytesObject, b_buffer),
	                                                      num_bytes, sizeof(byte_t));
	if unlikely(!result)
		goto done;
#if 1 /* This one allows for more efficient register streaming */
	result->b_size = num_bytes;
	result->b_base = (byte_t *)memset(DeeBytes_BUFFER_DATA(result), init, num_bytes);
	result->b_orig = Dee_AsObject(result);
#else
	memset(DeeBytes_BUFFER_DATA(result), init, num_bytes);
	_DeeBytes_InitBuffer(result, num_bytes);
#endif
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return result;
}

PUBLIC WUNUSED DREF DeeBytesObject *DCALL
DeeBytes_NewBufferUninitialized(size_t num_bytes) {
	DREF DeeBytesObject *result;
	result = (DREF DeeBytesObject *)DeeObject_MalloccSafe(offsetof(DeeBytesObject, b_buffer),
	                                                      num_bytes, sizeof(byte_t));
	if unlikely(!result)
		goto done;
	_DeeBytes_InitBuffer(result, num_bytes);
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return result;
}

PUBLIC WUNUSED DREF DeeBytesObject *DCALL
DeeBytes_TryNewBufferUninitialized(size_t num_bytes) {
	DREF DeeBytesObject *result;
	result = (DREF DeeBytesObject *)DeeObject_TryMalloccSafe(offsetof(DeeBytesObject, b_buffer),
	                                                         num_bytes, sizeof(byte_t));
	if unlikely(!result)
		goto done;
	_DeeBytes_InitBuffer(result, num_bytes);
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return result;
}

PUBLIC WUNUSED ATTR_INS(1, 2) DREF DeeBytesObject *DCALL
DeeBytes_NewBufferData(void const *__restrict data, size_t num_bytes) {
	DREF DeeBytesObject *result;
	result = (DREF DeeBytesObject *)DeeObject_Mallocc(offsetof(DeeBytesObject, b_buffer),
	                                                  num_bytes, sizeof(byte_t));
	if unlikely(!result)
		goto done;
#if 1 /* This one allows for more efficient register streaming */
	result->b_size = num_bytes;
	result->b_base = (byte_t *)memcpy(DeeBytes_BUFFER_DATA(result), data, num_bytes);
	result->b_orig = Dee_AsObject(result);
#else
	memcpy(DeeBytes_BUFFER_DATA(result), data, num_bytes);
	_DeeBytes_InitBuffer(result, num_bytes);
#endif
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return result;
}

PUBLIC WUNUSED ATTR_INS(1, 2) DREF DeeBytesObject *DCALL
DeeBytes_TryNewBufferData(void const *__restrict data, size_t num_bytes) {
	DREF DeeBytesObject *result;
	result = (DREF DeeBytesObject *)DeeObject_TryMallocc(offsetof(DeeBytesObject, b_buffer),
	                                                     num_bytes, sizeof(byte_t));
	if unlikely(!result)
		goto done;
#if 1 /* This one allows for more efficient register streaming */
	result->b_size = num_bytes;
	result->b_base = (byte_t *)memcpy(DeeBytes_BUFFER_DATA(result), data, num_bytes);
	result->b_orig = Dee_AsObject(result);
#else
	memcpy(DeeBytes_BUFFER_DATA(result), data, num_bytes);
	_DeeBytes_InitBuffer(result, num_bytes);
#endif
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
DeeBytes_ResizeBuffer(/*inherit(on_success)*/ DREF DeeBytesObject *__restrict self,
                      size_t num_bytes) {
	size_t total_bytes;
	DREF DeeBytesObject *result, *new_result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeBytes_Type);
	ASSERT(!DeeObject_IsShared(self));
	if unlikely(OVERFLOW_UADD(offsetof(DeeBytesObject, b_buffer), num_bytes, &total_bytes))
		total_bytes = (size_t)-1;
	result = self;
	ASSERT(result->b_base == DeeBytes_BUFFER_DATA(result));
	ASSERT(result->b_orig == Dee_AsObject(result));
	ASSERT(result->b_flags == Dee_BUFFER_FWRITABLE);
again:
	new_result = (DREF DeeBytesObject *)DeeObject_TryRealloc(result, total_bytes);
	if unlikely(!new_result) {
		if unlikely(num_bytes <= result->b_size) {
			result->b_size = num_bytes;
			return result;
		}
		if (Dee_CollectMemory(total_bytes))
			goto again;
		return NULL;
	}
	_DeeBytes_InitBuffer(new_result, num_bytes);
	return new_result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
DeeBytes_TryResizeBuffer(/*inherit(on_success)*/ DREF DeeBytesObject *__restrict self,
                         size_t num_bytes) {
	size_t total_bytes;
	DREF DeeBytesObject *result, *new_result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeBytes_Type);
	ASSERT(!DeeObject_IsShared(self));
	if unlikely(OVERFLOW_UADD(offsetof(DeeBytesObject, b_buffer), num_bytes, &total_bytes))
		total_bytes = (size_t)-1;
	result = self;
	ASSERT(result->b_base == DeeBytes_BUFFER_DATA(result));
	ASSERT(result->b_orig == Dee_AsObject(result));
	ASSERT(result->b_flags == Dee_BUFFER_FWRITABLE);
	new_result = (DREF DeeBytesObject *)DeeObject_TryRealloc(result, total_bytes);
	if unlikely(!new_result) {
		if unlikely(num_bytes <= result->b_size) {
			result->b_size = num_bytes;
			return result;
		}
		return NULL;
	}
	_DeeBytes_InitBuffer(new_result, num_bytes);
	return new_result;
}


PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeBytesObject *DCALL
DeeBytes_TruncateBuffer(/*inherit(on_success)*/ DREF DeeBytesObject *__restrict self,
                        size_t num_bytes) {
	DREF DeeBytesObject *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeBytes_Type);
	ASSERT(!DeeObject_IsShared(self));
	result = self;
	ASSERT(result->b_base == DeeBytes_BUFFER_DATA(result));
	ASSERT(result->b_orig == Dee_AsObject(result));
	ASSERT(result->b_flags == Dee_BUFFER_FWRITABLE);
	ASSERT(num_bytes <= result->b_size);
	if (num_bytes != result->b_size) {
		result = (DREF DeeBytesObject *)DeeObject_TryReallocc(result, offsetof(DeeBytesObject, b_buffer),
		                                                      num_bytes, sizeof(byte_t));
		if unlikely(!result) {
			result = self;
			result->b_size = num_bytes;
			return result;
		}
		_DeeBytes_InitBuffer(result, num_bytes);
	}
	return result;
}


/* Constructs a byte-view for data in `base...+=num_bytes' held by `owner'.
 * The given `flags' determines if the view is read-only, or can be modified.
 * @param: flags: Set of `Dee_BUFFER_F*' */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_NewView(DeeObject *owner, void *base,
                 size_t num_bytes, unsigned int flags) {
	DREF Bytes *result;
	ASSERTF(!(flags & ~(Dee_BUFFER_FREADONLY | Dee_BUFFER_FWRITABLE)),
	        "Invalid flags %x", flags);
	ASSERT_OBJECT(owner);
	result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_buffer));
	if unlikely(!result)
		goto done;
	result->b_base  = (byte_t *)base;
	result->b_size  = num_bytes;
	result->b_orig  = owner;
	result->b_flags = flags;
	Dee_Incref(owner);
	DeeObject_Init(result, &DeeBytes_Type);
done:
	return Dee_AsObject(result);
}


PRIVATE NONNULL((1)) void DCALL
bytes_fini(Bytes *__restrict self) {
	if (self->b_orig != Dee_AsObject(self))
		Dee_Decref(self->b_orig);
}

PRIVATE NONNULL((1, 2)) void DCALL
bytes_visit(Bytes *__restrict self, Dee_visit_t proc, void *arg) {
	if (self->b_orig != Dee_AsObject(self))
		Dee_Visit(self->b_orig);
}

PRIVATE WUNUSED DREF Bytes *DCALL bytes_ctor(void) {
	return_reference_((DREF Bytes *)Dee_EmptyBytes);
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_copy(Bytes *__restrict other) {
	return DeeBytes_NewBufferData(DeeBytes_DATA(other),
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
			byte_t init;
			if (DeeObject_AsSize(ob, &start))
				goto err;
			if (DeeObject_AsByte(argv[1], &init))
				goto err;
			return DeeBytes_NewBuffer(start, init);
		}
		if (DeeString_Check(argv[1])) {
			char const *str = DeeString_STR(argv[1]);
			if (WSTR_LENGTH(str) != 1)
				goto err_invalid_mode;
			if (str[0] == 'r') {
				/* ... */
			} else if (str[0] == 'w') {
				flags = Dee_BUFFER_FWRITABLE;
			} else {
				goto err_invalid_mode;
			}
			if (argc >= 3) {
				if (DeeObject_AsSize(argv[2], &start))
					goto err;
				if (argc >= 4) {
					if unlikely(argc > 4)
						goto err_args;
					if (DeeObject_AsSizeM1(argv[3], &end))
						goto err;
				}
			}
		} else {
			if (DeeObject_AsSize(argv[1], &start))
				goto err;
			if (argc >= 3) {
				if unlikely(argc > 3)
					goto err_args;
				if (DeeObject_AsSizeM1(argv[2], &end))
					goto err;
			}
		}
	} else if (!argc) {
err_args:
		err_invalid_argc(STR_Bytes, argc, 1, 4);
		goto err;
	} else {
		DeeTypeObject *tp_iter;
		DREF Bytes *result;
		ob = argv[0];
		if (DeeInt_Check(ob)) {
			if (DeeObject_AsSize(ob, &start))
				goto err;
#if 1 /* XXX: Should we really expose uninitialized memory to user-code? */
			return DeeBytes_NewBufferUninitialized(start);
#else
			return DeeBytes_NewBuffer(start, 0);
#endif
		}
		tp_iter = Dee_TYPE(ob);
		if (tp_iter == &DeeSuper_Type) {
			tp_iter = DeeSuper_TYPE(ob);
			ob      = DeeSuper_SELF(ob);
		}
		do {
			struct type_buffer *buf = tp_iter->tp_buffer;
			if (buf && buf->tp_getbuf) {
				DeeBuffer buffer;
				result = (DREF Bytes *)DeeObject_Malloc(offsetof(Bytes, b_buffer));
				if unlikely(!result)
					goto err;
				/* Construct a Bytes object using the buffer interface provided by `ob' */
				if unlikely((*buf->tp_getbuf)(ob, &buffer, Dee_BUFFER_FREADONLY))
					goto err_r;
				if (start > buffer.bb_size)
					start = buffer.bb_size;
				if (end > buffer.bb_size)
					end = buffer.bb_size;
				result->b_base  = (byte_t *)buffer.bb_base + start;
				result->b_size  = (size_t)(end - start);
				result->b_orig  = ob;
				result->b_flags = Dee_BUFFER_FREADONLY;
				Dee_Incref(ob);
				DeeObject_Init(result, &DeeBytes_Type);
				return result;
			}
		} while (DeeType_InheritBuffer(tp_iter));
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
			err_unimplemented_operator(tp_iter, OPERATOR_GETBUF); /* TODO: Pass orig error as "cause" */
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
bytes_serialize(Bytes *__restrict self, DeeSerial *__restrict writer) {
	Bytes *out;
	Dee_seraddr_t out_addr;
#define ADDROF(field) (out_addr + offsetof(Bytes, field))
	if (DeeBytes_IsBuffer(self)) {
		/* Self-owned bytes object */
		size_t sizeof_bytes, buffer_offset;
		ASSERT(self->b_orig == (DeeObject *)self);
		buffer_offset = DeeBytes_GetBufferDataOffset(self);
		sizeof_bytes = offsetof(Bytes, b_buffer);
		sizeof_bytes += DeeBytes_SIZE(self);
		sizeof_bytes += buffer_offset;
		out_addr = DeeSerial_ObjectMalloc(writer, sizeof_bytes, self);
		if (!Dee_SERADDR_ISOK(out_addr))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(b_orig), out_addr))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(b_base), ADDROF(b_buffer) + buffer_offset))
			goto err;
	} else {
		/* Externally-referenced bytes object */
		ASSERT(self->b_orig != (DeeObject *)self);
		out_addr = DeeSerial_ObjectMalloc(writer, offsetof(Bytes, b_buffer), self);
		if (!Dee_SERADDR_ISOK(out_addr))
			goto err;
		if (DeeSerial_PutObject(writer, ADDROF(b_orig), self->b_orig))
			goto err;
		if (DeeSerial_PutPointer(writer, ADDROF(b_base), self->b_base))
			goto err;
	}
	out = DeeSerial_Addr2Mem(writer, out_addr, Bytes);
	out->b_size  = self->b_size;
	out->b_flags = self->b_flags;
#undef ADDROF
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_assign(Bytes *__restrict self,
             DeeObject *__restrict values) {
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	return DeeSeq_ItemsToBytes(DeeBytes_DATA(self),
	                           DeeBytes_SIZE(self),
	                           values);
err_readonly:
	return err_bytes_not_writable(Dee_AsObject(self));
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeBytes_Print(DeeObject *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Print8(printer, arg,
	                        DeeBytes_DATA(self),
	                        DeeBytes_SIZE(self));
}

#define DO(expr)                         \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeBytes_PrintRepr(DeeObject *__restrict self,
                   Dee_formatprinter_t printer,
                   void *arg) {
	Dee_ssize_t temp, result;
	Bytes *me = (Bytes *)self;
	result = DeeFormat_PRINT(printer, arg, "\"");
	if unlikely(result < 0)
		goto done;
	DO(DeeFormat_QuoteBytes(printer, arg, DeeBytes_DATA(me), DeeBytes_SIZE(me)));
	DO(DeeFormat_PRINT(printer, arg, "\".bytes()"));
done:
	return result;
err:
	return temp;
}

#undef DO

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_str(Bytes *__restrict self) {
	return DeeString_NewSized((char const *)DeeBytes_DATA(self),
	                          DeeBytes_SIZE(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_repr(Bytes *__restrict self) {
	/* Optimization: can use a Dee_ascii_printer, instead of the default Dee_unicode_printer */
	struct Dee_ascii_printer printer = Dee_ASCII_PRINTER_INIT;
	if unlikely(DeeBytes_PrintRepr(Dee_AsObject(self),
	                               &Dee_ascii_printer_print,
	                               &printer) < 0)
		goto err;
	return Dee_ascii_printer_pack(&printer);
err:
	Dee_ascii_printer_fini(&printer);
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
	Dee_Incref(self);
	DeeObject_Init(result, &BytesIterator_Type);
done:
	return result;
}

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_contains(Bytes *self, DeeObject *needle);

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
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


struct bytes_compare_seq_data {
	byte_t               *bcsd_data;  /* [1..bcsd_size] The LHS byte-block */
	size_t                bcsd_size;  /* # of bytes in `bcsd_data' */
	size_t                bcsd_index; /* [<= bcsd_size] Index to next byte */
};


/* @return: 0 : lhs == rhs (for now...)
 * @return: -1: Error
 * @return: -2: lhs < rhs
 * @return: -3: lhs > rhs */
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bytes_compare_seq_cb(void *arg, DeeObject *rhs_elem) {
	byte_t lhs_byte, rhs_byte;
	struct bytes_compare_seq_data *data;
	data = (struct bytes_compare_seq_data *)arg;
	if (data->bcsd_index >= data->bcsd_size)
		return -2; /* lhs < rhs */
	if (DeeString_Check(rhs_elem)) {
		uint32_t rhs_word;
		if (DeeString_WLEN(rhs_elem) != 1)
			goto err_wrong_length;
		rhs_word = DeeString_GetChar(rhs_elem, 0);
		if unlikely(rhs_word > 0xff)
			return -2;
		rhs_byte = (byte_t)rhs_word;
	} else if (DeeBytes_Check(rhs_elem)) {
		if (DeeBytes_SIZE(rhs_elem) != 1)
			goto err_wrong_length;
		rhs_byte = DeeBytes_GetByte(rhs_elem, 0);
	} else {
		uint32_t rhs_word;
		if (DeeObject_AsUInt32(rhs_elem, &rhs_word))
			goto err;
		if unlikely(rhs_word > 0xff)
			return -2;
		rhs_byte = (byte_t)rhs_word;
	}
	lhs_byte = data->bcsd_data[data->bcsd_index++];
	if (lhs_byte == rhs_byte)
		return 0;
	if (lhs_byte < rhs_byte)
		return -2; /* lhs < rhs */
	return -3;     /* lhs > rhs */
err_wrong_length:
	err_expected_single_character_string(rhs_elem);
err:
	return -1;
}

/* Also accept "rhs" complaying with `{(string | Bytes | int)...}'
 * - string: Must be a single latin-1 character
 * - Bytes:  Must be a single byte
 * - int:    Must be in the range [0,256)
 * @return: -1: lhs < rhs
 * @return:  0: Equal
 * @return:  1: lhs > rhs
 * @return: Dee_COMPARE_ERR: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_compare_seq(Bytes *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct bytes_compare_seq_data data;
	data.bcsd_data  = DeeBytes_DATA(lhs);
	data.bcsd_size  = DeeBytes_SIZE(lhs);
	data.bcsd_index = 0;
	foreach_status = DeeObject_Foreach(rhs, &bytes_compare_seq_cb, &data);
	ASSERT(foreach_status == 0 || foreach_status == -1 ||
	       foreach_status == -2 || foreach_status == -3);
	switch (foreach_status) {
	case 0: break;
	case -1: goto err;
	case -2: return Dee_COMPARE_LO; /* lhs < rhs */
	case -3: return Dee_COMPARE_GR; /* lhs > rhs */
	default: __builtin_unreachable();
	}
	if (data.bcsd_index < data.bcsd_size)
		return Dee_COMPARE_GR; /* lhs > rhs */
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_compare_eq(Bytes *lhs, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return !string_eq_bytes((DeeStringObject *)rhs, lhs);
	if (DeeBytes_Check(rhs)) {
		void *rhs_data = DeeBytes_DATA(rhs);
		size_t rhs_size = DeeBytes_SIZE(rhs);
		if (DeeBytes_SIZE(lhs) != rhs_size)
			return Dee_COMPARE_NE;
		return !!bcmp(DeeBytes_DATA(lhs), rhs_data, rhs_size);
	}
	return bytes_compare_seq(lhs, rhs);
}

#undef memxcmp
#define memxcmp dee_memxcmp
LOCAL WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
dee_memxcmp(void const *a, size_t asiz,
            void const *b, size_t bsiz) {
	int result = memcmp(a, b, MIN(asiz, bsiz));
	if (result)
		return Dee_CompareFromDiff(result);
	return Dee_Compare(asiz, bsiz);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_compare(Bytes *lhs, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return -compare_string_bytes((DeeStringObject *)rhs, lhs);
	if (DeeBytes_Check(rhs)) {
		return memxcmp(DeeBytes_DATA(lhs), DeeBytes_SIZE(lhs),
		               DeeBytes_DATA(rhs), DeeBytes_SIZE(rhs));
	}
	return bytes_compare_seq(lhs, rhs);
}



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_eq(Bytes *lhs, DeeObject *rhs) {
	if (DeeBytes_Check(rhs)) {
		byte_t *rhs_data = DeeBytes_DATA(rhs);
		size_t rhs_size  = DeeBytes_SIZE(rhs);
		if (DeeBytes_SIZE(lhs) != rhs_size)
			return_false;
		return_bool(bcmp(DeeBytes_DATA(lhs), rhs_data, rhs_size) == 0);
	}
	if (DeeString_Check(rhs))
		return_bool(string_eq_bytes((DeeStringObject *)rhs, lhs));
	{
		int temp = bytes_compare_seq(lhs, rhs);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		return_bool(Dee_COMPARE_ISEQ(temp));
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_ne(Bytes *lhs, DeeObject *rhs) {
	if (DeeBytes_Check(rhs)) {
		byte_t *rhs_data = DeeBytes_DATA(rhs);
		size_t rhs_size  = DeeBytes_SIZE(rhs);
		if (DeeBytes_SIZE(lhs) != rhs_size)
			return_true;
		return_bool(bcmp(DeeBytes_DATA(lhs), rhs_data, rhs_size) != 0);
	}
	if (DeeString_Check(rhs))
		return_bool(!string_eq_bytes((DeeStringObject *)rhs, lhs));
	{
		int temp = bytes_compare_seq(lhs, rhs);
		if (Dee_COMPARE_ISERR(temp))
			goto err;
		return_bool(Dee_COMPARE_ISNE(temp));
	}
err:
	return NULL;
}


#define DEFINE_BYTES_COMPARE(name, op)                    \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Bytes *self, DeeObject *other) {                 \
		int diff = bytes_compare(self, other);            \
		if (Dee_COMPARE_ISERR(diff))              \
			goto err;                                     \
		return_bool(diff op 0);                           \
	err:                                                  \
		return NULL;                                      \
	}
DEFINE_BYTES_COMPARE(bytes_lo, <)
DEFINE_BYTES_COMPARE(bytes_le, <=)
DEFINE_BYTES_COMPARE(bytes_gr, >)
DEFINE_BYTES_COMPARE(bytes_ge, >=)
#undef DEFINE_BYTES_COMPARE

INTERN WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_add(Bytes *self, DeeObject *other) {
	DREF Bytes *result;
	DeeBuffer buffer;
	if (DeeObject_GetBuf(other, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self) + buffer.bb_size);
	if likely(result) {
		memcpy(mempcpy(DeeBytes_BUFFER_DATA(result),
		               DeeBytes_DATA(self),
		               DeeBytes_SIZE(self)),
		       buffer.bb_base, buffer.bb_size);
	}
	DeeBuffer_Fini(&buffer);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_mul(Bytes *self, DeeObject *other) {
	DREF Bytes *result;
	byte_t *dst, *src;
	size_t my_length, total_length, repeat;
	if (DeeObject_AsSize(other, &repeat))
		goto err;
	if (repeat == 1)
		return bytes_copy(self);
	my_length = DeeBytes_SIZE(self);
	if (OVERFLOW_UMUL(my_length, repeat, &total_length))
		goto err_overflow;
	if unlikely(total_length == 0)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	result = DeeBytes_NewBufferUninitialized(total_length);
	if unlikely(!result)
		goto err;
	src = DeeBytes_DATA(self);
	dst = DeeBytes_BUFFER_DATA(result);
	switch (my_length) {
	case 1:
		memsetb(dst, UNALIGNED_GET8(src), repeat);
		break;
#ifdef CONFIG_HAVE_memsetw
	case 2:
		memsetw(dst, UNALIGNED_GET16(src), repeat);
		break;
#endif /* CONFIG_HAVE_memsetw */
#ifdef CONFIG_HAVE_memsetl
	case 4:
		memsetl(dst, UNALIGNED_GET32(src), repeat);
		break;
#endif /* CONFIG_HAVE_memsetl */
#ifdef CONFIG_HAVE_memsetq
	case 8:
		memsetq(dst, UNALIGNED_GET64(src), repeat);
		break;
#endif /* CONFIG_HAVE_memsetq */
	default:
		while (repeat--)
			dst = (byte_t *)mempcpy(dst, src, my_length);
		break;
	}
	return result;
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(my_length, repeat);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
DeeString_CFormat(Dee_formatprinter_t printer,
                  Dee_formatprinter_t format_printer, void *arg,
                  /*utf-8*/ char const *__restrict format, size_t format_len,
                  size_t argc, DeeObject *const *argv);

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bytes *DCALL
bytes_mod(Bytes *self, DeeObject *args) {
	struct Dee_bytes_printer printer = Dee_BYTES_PRINTER_INIT;
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
	if unlikely(DeeString_CFormat(&Dee_bytes_printer_print,
	                              (Dee_formatprinter_t)&Dee_bytes_printer_append,
	                              &printer,
	                              (char const *)DeeBytes_DATA(self),
	                              DeeBytes_SIZE(self),
	                              argc,
	                              argv) < 0)
		goto err;
	return (DREF Bytes *)Dee_bytes_printer_pack(&printer);
err:
	Dee_bytes_printer_fini(&printer);
	return NULL;
}

PRIVATE struct type_math bytes_math = {
	/* .tp_int32  = */ DEFIMPL_UNSUPPORTED(&default__int32__unsupported),
	/* .tp_int64  = */ DEFIMPL_UNSUPPORTED(&default__int64__unsupported),
	/* .tp_double = */ DEFIMPL_UNSUPPORTED(&default__double__unsupported),
	/* .tp_int    = */ DEFIMPL_UNSUPPORTED(&default__int__unsupported),
	/* .tp_inv    = */ DEFIMPL(&default__set_operator_inv),
	/* .tp_pos    = */ DEFIMPL_UNSUPPORTED(&default__pos__unsupported),
	/* .tp_neg    = */ DEFIMPL_UNSUPPORTED(&default__neg__unsupported),
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_add,
	/* .tp_sub    = */ DEFIMPL(&default__set_operator_sub),
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_mul,
	/* .tp_div    = */ DEFIMPL_UNSUPPORTED(&default__div__unsupported),
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_mod,
	/* .tp_shl    = */ DEFIMPL_UNSUPPORTED(&default__shl__unsupported),
	/* .tp_shr    = */ DEFIMPL_UNSUPPORTED(&default__shr__unsupported),
	/* .tp_and    = */ DEFIMPL(&default__set_operator_and),
	/* .tp_or     = */ DEFIMPL(&default__set_operator_add),
	/* .tp_xor    = */ DEFIMPL(&default__set_operator_xor),
	/* .tp_pow    = */ DEFIMPL_UNSUPPORTED(&default__pow__unsupported),
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL_UNSUPPORTED(&default__dec__unsupported),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__set_operator_inplace_sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL_UNSUPPORTED(&default__inplace_div__unsupported),
	/* .tp_inplace_mod = */ DEFIMPL(&default__inplace_mod__with__mod),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL(&default__set_operator_inplace_and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__set_operator_inplace_add),
	/* .tp_inplace_xor = */ DEFIMPL(&default__set_operator_inplace_xor),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

PRIVATE struct type_cmp bytes_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&bytes_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytes_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytes_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_ge,
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bytes_size(Bytes *__restrict self) {
	ASSERT(DeeBytes_SIZE(self) != (size_t)-1);
	return DeeBytes_SIZE(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getitem_index(Bytes *__restrict self, size_t index) {
	byte_t value;
	if unlikely(index >= DeeBytes_SIZE(self))
		goto err_bounds;
	value = DeeBytes_GetByte(self, index);
#ifdef DeeInt_8bit
	return_reference(DeeInt_8bit + value);
#else /* DeeInt_8bit */
	return DeeInt_NewUInt8(value);
#endif /* !DeeInt_8bit */
err_bounds:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, DeeBytes_SIZE(self));
	return NULL;
}

#ifdef DeeInt_8bit
#define bytes_getitem_index_fast_PTR &bytes_getitem_index_fast
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getitem_index_fast(Bytes *__restrict self, size_t index) {
	byte_t value;
	ASSERT(index < DeeBytes_SIZE(self));
	value = DeeBytes_GetByte(self, index);
	return_reference(DeeInt_8bit + value);
}
#endif /* DeeInt_8bit */

#ifndef bytes_getitem_index_fast_PTR
#define bytes_getitem_index_fast_PTR NULL
#endif /* !bytes_getitem_index_fast_PTR */

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_delitem_index(Bytes *__restrict self, size_t index) {
	if unlikely(index >= DeeBytes_SIZE(self)) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	DeeBytes_SetByte(self, index, 0);
	return 0;
err_readonly:
	err_bytes_not_writable(Dee_AsObject(self));
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bytes_setitem_index(Bytes *self, size_t index, DeeObject *value) {
	byte_t val;
	if (DeeObject_AsByte(value, &val))
		goto err;
	if unlikely(index >= DeeBytes_SIZE(self)) {
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, DeeBytes_SIZE(self));
		goto err;
	}
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	DeeBytes_SetByte(self, index, val);
	return 0;
err_readonly:
	err_bytes_not_writable(Dee_AsObject(self));
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_getrange_index(Bytes *__restrict self,
                     Dee_ssize_t i_begin,
                     Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	size_t range_size;
	DeeSeqRange_Clamp(&range, i_begin, i_end, DeeBytes_SIZE(self));
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0)
		return_reference_((Bytes *)Dee_EmptyBytes);
	if unlikely(range_size == DeeBytes_SIZE(self))
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
	                                      self->b_base + range.sr_start,
	                                      range_size,
	                                      self->b_flags);
}

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_getrange_index_n(Bytes *__restrict self,
                       Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return bytes_getrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t start, range_size;
	start = DeeSeqRange_Clamp_n(i_begin, DeeBytes_SIZE(self));
	if unlikely(start == 0)
		return_reference_(self);
	range_size = DeeBytes_SIZE(self) - start;
	if unlikely(range_size <= 0)
		return_reference_((Bytes *)Dee_EmptyBytes);
	return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
	                                      self->b_base + start,
	                                      range_size,
	                                      self->b_flags);
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
bytes_setrange_index(Bytes *self,
                     Dee_ssize_t i_begin,
                     Dee_ssize_t i_end,
                     DeeObject *value) {
	struct Dee_seq_range range;
	size_t range_size;
	byte_t *dst;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, i_begin, i_end, DeeBytes_SIZE(self));
	range_size = range.sr_end - range.sr_start;
	dst = DeeBytes_DATA(self) + range.sr_start;
	return DeeSeq_ItemsToBytes(dst, range_size, value);
err_readonly:
	return err_bytes_not_writable(Dee_AsObject(self));
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bytes_setrange_index_n(Bytes *self, Dee_ssize_t i_begin,
                       DeeObject *value) {
#ifdef __OPTIMIZE_SIZE__
	return bytes_setrange_index(self, i_begin, SSIZE_MAX, value);
#else /* __OPTIMIZE_SIZE__ */
	size_t start, range_size;
	byte_t *dst;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	start      = DeeSeqRange_Clamp_n(i_begin, DeeBytes_SIZE(self));
	range_size = DeeBytes_SIZE(self) - start;
	dst = DeeBytes_DATA(self) + start;
	return DeeSeq_ItemsToBytes(dst, range_size, value);
err_readonly:
	return err_bytes_not_writable(Dee_AsObject(self));
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_delrange_index(Bytes *self, Dee_ssize_t i_begin, Dee_ssize_t i_end) {
#ifdef __OPTIMIZE_SIZE__
	return bytes_setrange_index(self, i_begin, i_end, Dee_None);
#else /* __OPTIMIZE_SIZE__ */
	struct Dee_seq_range range;
	size_t range_size;
	byte_t *dst;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, i_begin, i_end, DeeBytes_SIZE(self));
	range_size = range.sr_end - range.sr_start;
	dst = DeeBytes_DATA(self) + range.sr_start;
	bzero(dst, range_size);
	return 0;
err_readonly:
	return err_bytes_not_writable(Dee_AsObject(self));
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_delrange_index_n(Bytes *self, Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return bytes_delrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t start, range_size;
	byte_t *dst;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	start      = DeeSeqRange_Clamp_n(i_begin, DeeBytes_SIZE(self));
	range_size = DeeBytes_SIZE(self) - start;
	dst = DeeBytes_DATA(self) + start;
	bzero(dst, range_size);
	return 0;
err_readonly:
	return err_bytes_not_writable(Dee_AsObject(self));
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bytes_foreach(Bytes *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i, size = DeeBytes_SIZE(self);
	byte_t *data = DeeBytes_DATA(self);
	for (i = 0; i < size; ++i) {
		byte_t value = data[i];
#ifdef DeeInt_8bit
		temp = (*proc)(arg, (DeeObject *)(DeeInt_8bit + value));
#else /* DeeInt_8bit */
		DREF DeeObject *int_value = DeeInt_NewUInt8(value);
		if unlikely(!int_value)
			return -1;
		temp = (*proc)(arg, (DeeObject *)(DeeInt_8bit + value));
		Dee_Decref(int_value);
#endif /* !DeeInt_8bit */
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

#ifdef DeeInt_8bit
#define bytes_asvector_nothrow     bytes_asvector
#define bytes_asvector_nothrow_PTR &bytes_asvector_nothrow
#endif /* DeeInt_8bit */
#ifndef bytes_asvector_nothrow_PTR
#define bytes_asvector_nothrow_PTR NULL
#endif /* !bytes_asvector_nothrow_PTR */

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bytes_asvector(Bytes *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t size = DeeBytes_SIZE(self);
	if likely(dst_length >= size) {
		size_t i;
		byte_t *data = DeeBytes_DATA(self);
		for (i = 0; i < size; ++i) {
			DREF DeeObject *int_value;
			byte_t value = data[i];
#ifdef DeeInt_8bit
			int_value = Dee_AsObject(DeeInt_8bit + value);
			Dee_Incref(int_value);
#else /* DeeInt_8bit */
			int_value = DeeInt_NewUInt8(value);
			if unlikely(!int_value) {
				Dee_Decrefv(dst, i);
				return (size_t)-1;
			}
#endif /* !DeeInt_8bit */
			dst[i] = int_value; /* Inherit reference */
		}
	}
	return size;
}

PRIVATE struct type_seq bytes_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bytes_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__delrange__with__delrange_index__and__delrange_index_n),
	/* .tp_setrange                   = */ DEFIMPL(&default__setrange__with__setrange_index__and__setrange_index_n),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&bytes_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bytes_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&bytes_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bytes_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))bytes_getitem_index_fast_PTR,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&bytes_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&bytes_setitem_index,
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__size__and__getitem_index_fast),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__size__and__getitem_index_fast),
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bytes_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bytes_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&bytes_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&bytes_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&bytes_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&bytes_setrange_index_n,
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&bytes_asvector,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))bytes_asvector_nothrow_PTR,
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_isreadonly(Bytes *__restrict self) {
	return_bool(!(self->b_flags & Dee_BUFFER_FWRITABLE));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bytes_nonempty_as_bound(Bytes *__restrict self) {
	return Dee_BOUND_FROMBOOL(!DeeBytes_IsEmpty(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getfirst(Bytes *__restrict self) {
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	return DeeInt_NEWU(DeeBytes_GetByte(self, 0));
err_empty:
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_setfirst(Bytes *__restrict self, DeeObject *__restrict value) {
	byte_t byte_value;
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	if unlikely(DeeObject_AsByte(value, &byte_value))
		goto err;
	DeeBytes_SetByte(self, 0, byte_value);
	return 0;
err_empty:
	DeeRT_ErrEmptySequence(self);
	goto err;
err_readonly:
	err_bytes_not_writable(Dee_AsObject(self));
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_getlast(Bytes *__restrict self) {
	byte_t byte_value;
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	byte_value = DeeBytes_GetByte(self, DeeBytes_SIZE(self) - 1);
	return DeeInt_NEWU(byte_value);
err_empty:
	DeeRT_ErrEmptySequence(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bytes_setlast(Bytes *__restrict self, DeeObject *__restrict value) {
	byte_t byte_value;
	if unlikely(DeeBytes_IsEmpty(self))
		goto err_empty;
	if unlikely(!DeeBytes_IsWritable(self))
		goto err_readonly;
	if unlikely(DeeObject_AsByte(value, &byte_value))
		goto err;
	DeeBytes_SetByte(self, DeeBytes_SIZE(self) - 1, byte_value);
	return 0;
err_empty:
	DeeRT_ErrEmptySequence(self);
	goto err;
err_readonly:
	err_bytes_not_writable(Dee_AsObject(self));
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
	size_t result = offsetof(Bytes, b_buffer);
	if (DeeBytes_IsBuffer(self))
		result += DeeBytes_SIZE(self);
	return DeeInt_NewSize(result);
}


PRIVATE struct type_getset tpconst bytes_getsets[] = {
	TYPE_GETTER_AB_F("isreadonly", &bytes_isreadonly,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this ?. object cannot be written to"),
	TYPE_GETSET_BOUND_F(STR_first,
	                    &bytes_getfirst,
	                    &bytes_delfirst,
	                    &bytes_setfirst,
	                    &bytes_nonempty_as_bound,
	                    METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#tValueError{@this ?. object is empty}"
	                    "#tBufferError{Attempted to modify the byte when @this ?. object is not writable}"
	                    "Access the first byte of @this ?. object"),
	TYPE_GETSET_BOUND_F(STR_last,
	                    &bytes_getlast,
	                    &bytes_dellast,
	                    &bytes_setlast,
	                    &bytes_nonempty_as_bound,
	                    METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#tValueError{@this ?. object is empty}"
	                    "#tBufferError{Attempted to modify the byte when @this ?. object is not writable}"
	                    "Access the last byte of @this ?. object"),
	TYPE_GETTER_AB_F("__sizeof__", &bytes_sizeof,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst bytes_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("iswritable", STRUCT_CONST, Bytes, b_flags, Dee_BUFFER_FWRITABLE,
	                         "Evaluates to ?t if @this ?. object not be written to (the inverse of ?#isreadonly)"),
	TYPE_MEMBER_BITFIELD_DOC("ismutable", STRUCT_CONST, Bytes, b_flags, Dee_BUFFER_FWRITABLE,
	                         "Alias for ?#iswritable, overriding ?Aismutable?DSequence"),
	TYPE_MEMBER_FIELD("length", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Bytes, b_size)),
	TYPE_MEMBER_FIELD("__flags__", STRUCT_CONST | STRUCT_UINT, offsetof(Bytes, b_flags)),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
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
	return err_bytes_not_writable(Dee_AsObject(self));
}

PRIVATE struct type_buffer bytes_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bytes_getbuf,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};


PRIVATE WUNUSED DREF Bytes *DCALL
bytes_fromseq(DeeTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "fromseq", &seq);
	return (DREF Bytes *)DeeBytes_FromSequence(seq);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_fromhex(DeeTypeObject *__restrict UNUSED(self),
              size_t argc, DeeObject *const *argv) {
	DeeObject *hex_str;
	DREF Bytes *result;
	byte_t *dst;
	union Dee_charptr_const iter, end;
	size_t length;
	DeeArg_Unpack1(err, argc, argv, "fromhex", &hex_str);
	if (DeeObject_AssertTypeExact(hex_str, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(hex_str)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte(hex_str);
		length   = WSTR_LENGTH(iter.cp8);
		result   = DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst     = DeeBytes_BUFFER_DATA(result);
		end.cp8 = iter.cp8 + length;
		for (;;) {
			byte_t byte_value, nibble;
			uint8_t ch;
			for (;;) {
				if (iter.cp8 == end.cp8)
					goto done;
				if (!DeeUni_IsSpace(iter.cp8[0]))
					break;
				++iter.cp8;
			}
			ch = *iter.cp8++;
			if (!DeeUni_AsDigit(ch, 16, &byte_value))
				goto err_invalid;
			if (iter.cp8 == end.cp8)
				goto err_unbalanced;
			ch = *iter.cp8++;
			if (!DeeUni_AsDigit(ch, 16, &nibble))
				goto err_invalid;
			byte_value <<= 4;
			byte_value |= nibble;
			ASSERT(dst < DeeBytes_END(result));
			*dst++ = byte_value;
		}
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16 = DeeString_Get2Byte(hex_str);
		length    = WSTR_LENGTH(iter.cp16);
		result    = DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst      = DeeBytes_BUFFER_DATA(result);
		end.cp16 = iter.cp16 + length;
		for (;;) {
			byte_t byte_value, nibble;
			uint16_t ch;
			for (;;) {
				if (iter.cp16 == end.cp16)
					goto done;
				if (!DeeUni_IsSpace(iter.cp16[0]))
					break;
				++iter.cp16;
			}
			ch = *iter.cp16++;
			if (!DeeUni_AsDigit(ch, 16, &byte_value))
				goto err_invalid;
			if (iter.cp16 == end.cp16)
				goto err_unbalanced;
			ch = *iter.cp16++;
			if (!DeeUni_AsDigit(ch, 16, &nibble))
				goto err_invalid;
			byte_value <<= 4;
			byte_value |= nibble;
			ASSERT(dst < DeeBytes_END(result));
			*dst++ = byte_value;
		}
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32 = DeeString_Get4Byte(hex_str);
		length    = WSTR_LENGTH(iter.cp32);
		result    = DeeBytes_NewBufferUninitialized(length / 2);
		if unlikely(!result)
			goto err;
		dst      = DeeBytes_BUFFER_DATA(result);
		end.cp32 = iter.cp32 + length;
		for (;;) {
			byte_t byte_value, nibble;
			uint32_t ch;
			for (;;) {
				if (iter.cp32 == end.cp32)
					goto done;
				if (!DeeUni_IsSpace(iter.cp32[0]))
					break;
				++iter.cp32;
			}
			ch = *iter.cp32++;
			if (!DeeUni_AsDigit(ch, 16, &byte_value))
				goto err_invalid;
			if (iter.cp32 == end.cp32)
				goto err_unbalanced;
			ch = *iter.cp32++;
			if (!DeeUni_AsDigit(ch, 16, &nibble))
				goto err_invalid;
			byte_value <<= 4;
			byte_value |= nibble;
			ASSERT(dst < DeeBytes_END(result));
			*dst++ = byte_value;
		}
		break;
	}
done:
	{
		size_t used = (size_t)(dst - DeeBytes_BUFFER_DATA(result));
		ASSERT(used <= DeeBytes_SIZE(result));
		if unlikely(used != DeeBytes_SIZE(result))
			result = DeeBytes_TruncateBuffer(result, used);
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
	TYPE_METHOD_F("fromseq", &bytes_fromseq,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because returns writable buffer */
	              "(seq:?S?Dint)->?.\n"
	              "#tNotImplemented{The given @seq cannot be iterated, or contains at "
	              /*            */ "least one item that cannot be converted into an integer}"
	              "#tIntegerOverflow{At least one of the integers found in @seq is lower "
	              /*             */ "than $0, or greater than $0xff}"
	              "Convert the items of the given sequence @seq into integers, "
	              /**/ "and construct a writable ?. object from their values\n"
	              "Passing ?N for @seq will return an empty ?. object"),
	TYPE_METHOD_F("fromhex", &bytes_fromhex,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because returns writable buffer */
	              "(hex_string:?Dstring)->?.\n"
	              "#tValueError{The given @hex_string contains non-hexadecimal and non-space characters}"
	              "#tValueError{The given @hex_string contains an unbalanced hexadecimal digit}"
	              "Decode a given string containing only digit characters, characters between $\"a\" and $\"f\" "
	              /**/ "or $\"A\" and $\"F\", or optional space characters separating pairs of such characters.\n"
	              "Each pair of hexadecimal digits is then interpreted as a byte that is then used to construct "
	              /**/ "the resulting ?. object.\n"
	              "Note that this function is also called by the $\"hex\" codec, meaning that ${string.decode(\"hex\")} "
	              /**/ "is the same as calling this functions, while ${Bytes.encode(\"hex\")} is the same as calling ?#hex\n"
	              "${"
	              /**/ "local data = \"DEAD BEEF\".decode(\"hex\");\n"
	              /**/ "print repr data; /* \"\\xDE\\xAD\\xBE\\xEF\".bytes() */"
	              "}"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst bytes_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &BytesIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeInt_Type),
	TYPE_MEMBER_END
};

PUBLIC struct _Dee_empty_bytes_struct DeeBytes_Empty = {
	OBJECT_HEAD_INIT(&DeeBytes_Type),
	/* .b_base   = */ NULL,
	/* .b_size   = */ 0,
	/* .b_orig   = */ Dee_EmptyBytes,
	/* .b_flags  = */ Dee_BUFFER_FWRITABLE,
	/* .b_buffer = */ { 0 }
};

PRIVATE struct type_operator const bytes_operators[] = {
//	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL), /* TODO: Allow when the result is a read-only buffer */
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISARG_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISARG_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISARG_ROBYTES | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISARG_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES),
	TYPE_OPERATOR_FLAGS(OPERATOR_8003_GETBUF, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISARG_ROBYTES),
};

INTDEF struct type_method tpconst bytes_methods[];
INTDEF struct type_method_hint tpconst bytes_method_hints[];

PUBLIC DeeTypeObject DeeBytes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Bytes),
	/* .tp_doc      = */ DOC("An string-like abstract buffer object type for viewing & editing "
	                         /**/ "the memory of any object implementing the buffer interface, "
	                         /**/ "or to allocate fixed-length buffers for raw bytes which can "
	                         /**/ "then be loaded with arbitrary data (most notably through use "
	                         /**/ "of ?Areadinto?DFile)\n"
	                         "A ?. object implements the sequence interface as a ${{int...}}-like "
	                         /**/ "sequence, with each integer being a value between $0 and $0xff\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?. object\n"
	                         "\n"

	                         "(ob:?O,start=!0,end=!-1)\n"
	                         "(ob:?O,mode=!Pr,start=!0,end=!-1)\n"
	                         "#tNotImplemented{The given @ob does not implement the buffer protocol}"
	                         "Construct a ?. object for viewing the memory of @ob, either as read-only "
	                         /**/ "when @mode is set to $\"r\" or omitted, or as read-write when set to "
	                         /**/ "$\"w\". The section of memory then addressed by the ?. view starts "
	                         /**/ "after @start bytes, and ends at @end bytes.\n"
	                         "If either @start or @end is a negative number, it will refer to the "
	                         /**/ "end of the memory found in @ob, using an intentional integer underflow\n"
	                         "Whether or not a ?. object is read-only, or writable can be determined "
	                         /**/ "using ?#iswritable and ?#isreadonly\n"
	                         "Additionally, a ?. object can be made writable by creating a copy (a.s. ?#{op:copy}), "
	                         /**/ "or by using ?#makewritable, which will not create a new ?. object, but re-return "
	                         /**/ "the same object, if it was already writable\n"
	                         "\n"

	                         "(num_bytes:?Dint)\n"
	                         "(num_bytes:?Dint,init:?Dint)\n"
	                         "#tIntegerOverflow{The given @init is negative, or greater than $0xff}"
	                         "Construct a writable, self-contained ?. object for a total of @num_bytes "
	                         /**/ "bytes of memory, pre-initialized to @init, or left undefined when "
	                         /**/ "@init isn't specified\n"
	                         "\n"

	                         "(items:?S?Dint)\n"
	                         "For compatibility with other sequence types, as well as the expectation "
	                         /**/ "of a sequence-like object implementing a sequence-cast-constructor, ?. "
	                         /**/ "objects also implement this overload\n"
	                         "However, given the reason for the existence of a ?. object, as well as the "
	                         /**/ "fact that a sequence object may also implement a buffer interface, which "
	                         /**/ "the ${Bytes(ob: Object)} overload above makes use of, that overload is "
	                         /**/ "always preferred over this one. In situations where this might cause "
	                         /**/ "ambiguity, it is recommended that the ?#fromseq class method be used to "
	                         /**/ "construct the ?. object instead.\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a writable copy of @this ?. object, containing "
	                         /**/ "a copy of all data as a kind of snapshot\n"
	                         "${"
	                         /**/ "local x = \"foobar\";\n"
	                         /**/ "local y = x.bytes();\n"
	                         /**/ "print y[0]; /* 102 */\n"
	                         /**/ "y = copy y;\n"
	                         /**/ "print y[0]; /* 102 */\n"
	                         /**/ "y[0] = 42;  /* Only allowed because the `copy' */\n"
	                         /**/ "print y;    /* *oobar */"
	                         "}\n"
	                         "\n"

	                         "str->\n"
	                         "Returns a string variant of @this ?. object, interpreting each byte as "
	                         /**/ "a unicode character from the range U+0000-U+00FF. This is identical "
	                         /**/ "to encoding the ?. object as a latin-1 (or iso-8859-1) string\n"
	                         "The returned string is equal to ${this.encode(\"latin-1\")}\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of @this ?. object in the form "
	                         /**/ "of ${f\"{repr this.encode(\"latin-1\")}.bytes()\"}\n"
	                         "\n"

	                         "contains(needle:?X3?.?Dstring?Dint)->\n"
	                         "#tValueError{The given @needle is a string containing characters ${> 0xff}}"
	                         "#tIntegerOverflow{The given @needle is an integer lower than $0, or greater than $0xff}"
	                         "Check if @needle appears within @this ?. object\n"
	                         "\n"

	                         ":=(data:?X3?.?Dstring?S?Dint)->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tUnpackError{The length of the given @data does not equal ${##this}}"
	                         "Assign the contents of @data to @this ?. object\n"
	                         "You may pass ?N for @data to clear all bytes of @this buffer\n"
	                         "\n"

	                         "[](index:?Dint)->?Dint\n"
	                         "Returns the byte found at @index as an integer between $0 and $0xff\n"
	                         "\n"

	                         "[]=(index:?Dint,value:?Dint)->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tIndexError{The given @index is negative, or greater than the length of @this }"
	                         "#tIntegerOverflow{@value is negative or greater than $0xff}"
	                         "Taking the integer value of @value, assign that value to the byte found at @index\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tIndexError{The given @index is negative, or greater than the length of @this }"
	                         "Same as ${this[index] = 0}, assigning a zero-byte at the given @index\n"
	                         "\n"

	                         "[:]->?.\n"
	                         "Returns another ?. view for viewing a sub-range of bytes\n"
	                         "Modifications then made to the returned ?. object will affect the "
	                         /**/ "same memory already described by @this ?. object\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,data:?X3?.?Dstring?S?Dint)->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tIntegerOverflow{One of the integers extracted from @data is negative, "
	                         /*             */ "or greater than $0xff}"
	                         "#tValueError{@data is a string containing characters ${> 0xff}}"
	                         "#tUnpackError{The length of the given sequence @data does not match the "
	                         /*         */ "number of bytes, that is ${end-start}. If any, and how many "
	                         /*         */ "bytes of @this ?. object were already modified is undefined}"
	                         "Assign values to a given range of bytes\n"
	                         "You may pass ?N to fill the entire range with zero-bytes\n"
	                         "\n"

	                         "del[:]->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "Same as ${this[start:end] = none}, assigning zero-bytes within the given range\n"
	                         "\n"

	                         "iter->\n"
	                         "Allows for iteration of the individual bytes as integers between $0 and $0xff\n"
	                         "\n"

	                         "<(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         "<=(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         "==(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         "!=(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         ">(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         ">=(other:?X3?.?Dstring?S?X3?.?Dstring?Dint)->\n"
	                         "#tValueError{The given @other is a string containing characters ${> 0xff}}"
	                         "Perform a lexicographical comparison between @this and @other, and return the result\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of bytes represented by @this ?. object\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. object is non-empty\n"
	                         "\n"

	                         "add->\n"
	                         "Construct a new writable ?. object that at is the concatenation of @this and @other\n"
	                         "+(other:?X3?.?Dstring?O)->\n"
	                         "Return a new writable ?. object that is the concatenation of @this and ${str(other).bytes()}\n"
	                         "\n"

	                         "*(times:?Dint)->\n"
	                         "#tIntegerOverflow{@times is negative, or too large}"
	                         "Returns @this ?. object repeated @times number of times\n"
	                         "\n"

	                         "%(args:?DTuple)->\n"
	                         "%(arg:?O)->\n"
	                         "#tUnicodeEncodeError{Attempted to print a unicode character ${> 0xff}}"
	                         "After interpreting the bytes from @this ?. object as LATIN-1, generate "
	                         /**/ "a printf-style format string containing only LATIN-1 characters.\n"
	                         "If @arg isn't a tuple, it is packed into one and the call is identical "
	                         /**/ "to ${this.operator % (pack(arg))}\n"
	                         "The returned ?. object is writable"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &bytes_ctor,
			/* tp_copy_ctor:   */ &bytes_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bytes_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bytes_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bytes_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&bytes_assign,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bytes_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&bytes_bool,
		/* .tp_print     = */ &DeeBytes_Print,
		/* .tp_printrepr = */ &DeeBytes_PrintRepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bytes_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bytes_math,
	/* .tp_cmp           = */ &bytes_cmp,
	/* .tp_seq           = */ &bytes_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ &bytes_buffer,
	/* .tp_methods       = */ bytes_methods,
	/* .tp_getsets       = */ bytes_getsets,
	/* .tp_members       = */ bytes_members,
	/* .tp_class_methods = */ bytes_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bytes_class_members,
	/* .tp_method_hints  = */ bytes_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bytes_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bytes_operators),
};








/* ================================================================================= */
/*   BYTES PRINTER API                                                               */
/* ================================================================================= */

/* Same as `Dee_bytes_printer_init()', but try to pre-allocate memory for `hint' bytes. */
PUBLIC NONNULL((1)) void
(DCALL Dee_bytes_printer_init_ex)(/*inherit(always)*/ struct Dee_bytes_printer *__restrict self,
                                  size_t hint) {
	Bytes *bytes;
	bytes = (Bytes *)DeeObject_TryMallocc(offsetof(Bytes, b_buffer), hint, sizeof(byte_t));
	if likely(bytes)
		bytes->b_size = hint;
	self->bp_bytes   = bytes;
	self->bp_length  = 0;
	self->bp_numpend = 0;
}


/* _Always_ inherit all byte data (even upon error) saved in
 * `self', and construct a new Bytes object from all that data, before
 * returning a reference to that object.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `Dee_bytes_printer_fini()'
 * @return: * :   A reference to the packed Bytes object.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_bytes_printer_pack(/*inherit(always)*/ struct Dee_bytes_printer *__restrict self) {
	DREF Bytes *result = self->bp_bytes;
	if unlikely(!result)
		return DeeBytes_NewEmpty();

	/* Deallocate unused memory. */
	if likely(self->bp_length != result->b_size) {
		DREF Bytes *reloc;
		reloc = (DREF Bytes *)DeeObject_TryReallocc(result, offsetof(Bytes, b_buffer),
		                                            self->bp_length, sizeof(byte_t));
		if likely(reloc)
			result = reloc;
		result->b_size = self->bp_length;
	}

	/* Do final object initialization. */
	result->b_base  = DeeBytes_BUFFER_DATA(result);
	result->b_orig  = Dee_AsObject(result);
	result->b_flags = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &DeeBytes_Type);
	DBG_memset(self, 0xcc, sizeof(*self));
	return Dee_AsObject(result);
}

/* Append raw byte data to the given bytes-printer, without concern
 * about any kind of encoding. - Just copy over the raw bytes.
 * -> A far as unicode support goes, this function has _nothing_ to
 *    do with any kind of encoding. - It just blindly copies the given
 *    data into the buffer of the resulting Bytes object.
 * -> The equivalent Dee_unicode_printer function is `Dee_unicode_printer_print8' */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DPRINTER_CC
Dee_bytes_printer_append(struct Dee_bytes_printer *__restrict self,
                         byte_t const *__restrict data, size_t datalen) {
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
		bytes = (Bytes *)DeeObject_TryMallocc(offsetof(Bytes, b_buffer),
		                                      alloc_size, sizeof(byte_t));
		if unlikely(!bytes) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(Bytes, b_buffer), alloc_size, sizeof(byte_t)))
				goto alloc_again;
			return -1;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
		memcpy(DeeBytes_BUFFER_DATA(bytes), data, datalen);
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
		bytes = (Bytes *)DeeObject_TryReallocc(bytes, offsetof(Bytes, b_buffer),
		                                       alloc_size, sizeof(byte_t));
		if unlikely(!bytes) {
			bytes = self->bp_bytes;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(Bytes, b_buffer),
			                        alloc_size, sizeof(byte_t)))
				goto realloc_again;
			return -1;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
	}

	/* Copy data into the dynamic bytes. */
	memcpy(DeeBytes_BUFFER_DATA(bytes) + self->bp_length, data, datalen);
	self->bp_length += datalen;
done:
	return (Dee_ssize_t)datalen;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_bytes_printer_putb)(struct Dee_bytes_printer *__restrict self, byte_t ch) {
	/* Quick check: Can we print to an existing buffer. */
	if (self->bp_bytes &&
	    self->bp_length < self->bp_bytes->b_size) {
		DeeBytes_SetBufferByte(self->bp_bytes, self->bp_length, ch);
		++self->bp_length;
		goto done;
	}

	/* Fallback: go the long route. */
	if (Dee_bytes_printer_append(self, &ch, 1) < 0)
		goto err;
done:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL Dee_bytes_printer_repeat)(struct Dee_bytes_printer *__restrict self,
                                 byte_t ch, size_t count) {
	byte_t *buffer;
	buffer = Dee_bytes_printer_alloc(self, count);
	if unlikely(!buffer)
		goto err;

	/* Simply do a memset to initialize bytes data. */
	memset(buffer, ch, count);
	return (Dee_ssize_t)count;
err:
	return -1;
}

PUBLIC NONNULL((1)) void
(DCALL Dee_bytes_printer_release)(struct Dee_bytes_printer *__restrict self,
                                  size_t datalen) {
	ASSERT(self->bp_length >= datalen);

	/* This's actually all that needs to be
	 * done with the current implementation. */
	self->bp_length -= datalen;
}

PUBLIC WUNUSED NONNULL((1)) byte_t *
(DCALL Dee_bytes_printer_alloc)(struct Dee_bytes_printer *__restrict self, size_t datalen) {
	Bytes *bytes;
	size_t alloc_size;
	byte_t *result;
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
		bytes = (Bytes *)DeeObject_TryMallocc(offsetof(Bytes, b_buffer),
		                                      alloc_size + 1, sizeof(byte_t));
		if unlikely(!bytes) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(Bytes, b_buffer),
			                        alloc_size + 1, sizeof(byte_t)))
				goto alloc_again;
			return NULL;
		}
		self->bp_bytes  = bytes;
		bytes->b_size   = alloc_size;
		self->bp_length = datalen;
		return DeeBytes_BUFFER_DATA(bytes);
	}
	alloc_size = bytes->b_size;
	ASSERT(alloc_size >= self->bp_length);
	alloc_size -= self->bp_length;
	if unlikely(alloc_size < datalen) {
		size_t min_alloc = self->bp_length + datalen;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		bytes = (Bytes *)DeeObject_TryReallocc(bytes, offsetof(Bytes, b_buffer),
		                                       alloc_size + 1, sizeof(byte_t));
		if unlikely(!bytes) {
			bytes = self->bp_bytes;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(Bytes, b_buffer),
			                        alloc_size + 1, sizeof(byte_t)))
				goto realloc_again;
			return NULL;
		}
		self->bp_bytes = bytes;
		bytes->b_size  = alloc_size;
	}

	/* Append text at the end. */
	result = DeeBytes_BUFFER_DATA(bytes) + self->bp_length;
	self->bp_length += datalen;
	return result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_BYTES_C */
