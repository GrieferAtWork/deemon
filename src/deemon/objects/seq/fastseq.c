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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C
#define GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "range.h"
#include "svec.h"

DECL_BEGIN

STATIC_ASSERT_MSG(DEE_FASTSEQ_NOTFAST_DEPRECATED == (size_t)-1,
                  "`nsi_getsize_fast' assumes this correlation");


/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST_DEPRECATED' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - Tuple
 *  - List
 *  - _SharedVector      (Created by a `ASM_CALL_SEQ' instruction -- `call top, [#X]')
 *  - _SeqSubRange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _SeqSubRangeN      (*ditto*)
 *  - _SeqTransformation (Only if the sequence being transformed is a fast-sequence)
 *  - _SeqIntRange
 *  - string
 *  - Bytes
 * Sub-classes of these types are not fast-sequence-compatible. */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSize_deprecated(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	struct type_seq *seq;
	struct type_nsi const *nsi;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	seq     = tp_self->tp_seq;
	if (!seq)
		goto nope;
	nsi = seq->tp_nsi;
	if (!nsi)
		goto nope;
	if (nsi->nsi_class != TYPE_SEQX_CLASS_SEQ)
		goto nope;
	if (!nsi->nsi_seqlike.nsi_getsize_fast)
		goto nope;
	ASSERT(nsi->nsi_seqlike.nsi_getitem ||
	       nsi->nsi_seqlike.nsi_getitem_fast);
	return (*nsi->nsi_seqlike.nsi_getsize_fast)(self);
nope:
	return DEE_FASTSEQ_NOTFAST_DEPRECATED;
}


/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize_deprecated(self)' when
 * `self' is an immutable sequence (anything other than `List' and `_SharedVector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize_deprecated(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST_DEPRECATED'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItem_deprecated(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	struct type_seq *seq;
	struct type_nsi const *nsi;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	seq     = tp_self->tp_seq;
	ASSERT(seq != NULL);
	nsi = seq->tp_nsi;
	ASSERT(nsi != NULL);
	ASSERT(nsi->nsi_class == TYPE_SEQX_CLASS_SEQ);
	if (nsi->nsi_seqlike.nsi_getitem_fast) {
		DREF DeeObject *result;
		result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
		ASSERT(result != ITER_DONE);
		if unlikely(!result)
			err_unbound_index(self, index);
		return result;
	}
	ASSERT(nsi->nsi_seqlike.nsi_getitem);
	return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
}


/* Try to load index-based fast sequence controls for "seq".
 * @return: true:  Success. You may use other `DeeFastSeq_*' to access sequence elements.
 * @return: false: Failure. Given `seq' does not implement `tp_getitem_index_fast' */
PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeFastSeq_Init)(DeeFastSeq *__restrict self, DeeObject *__restrict seq) {
	self->fsq_size = DeeFastSeq_Init_impl(self, seq);
	return self->fsq_size != (size_t)-1;
}

PUBLIC WUNUSED NONNULL((1, 2)) size_t
(DCALL DeeFastSeq_Init_impl)(DeeFastSeq *__restrict self, DeeObject *__restrict seq) {
	DeeTypeObject *tp_seq;
	tp_seq = Dee_TYPE(seq);
again:
	if (tp_seq->tp_seq &&
	    tp_seq->tp_seq->tp_getitem_index_fast &&
	    tp_seq->tp_seq->tp_size_fast) {
have_operators:
		self->fsq_self               = seq;
		self->fsq_getitem_index_fast = tp_seq->tp_seq->tp_getitem_index_fast;
		return (*tp_seq->tp_seq->tp_size_fast)(seq);
	} else if (tp_seq == &DeeSuper_Type) {
		tp_seq = DeeSuper_TYPE(seq);
		seq    = DeeSuper_SELF(seq);
		goto again;
	} else if (!tp_seq->tp_seq && (DeeType_InheritSize(tp_seq) ||
	                               DeeType_InheritGetItem(tp_seq))) {
		if (tp_seq->tp_seq &&
		    tp_seq->tp_seq->tp_getitem_index_fast &&
		    tp_seq->tp_seq->tp_size_fast)
			goto have_operators;
	}
	return (size_t)-1;
}







struct foreach_seq_as_heap_vector_data {
	size_t           sahvd_size;   /* Used vector size */
	size_t           sahvd_alloc;  /* Allocated vector size */
	DREF DeeObject **sahvd_vector; /* [1..1][0..sahvd_size|ALLOC(sahvd_alloc)][owned] Sequence vector. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
foreach_seq_as_heap_vector_cb(void *arg, DeeObject *__restrict elem) {
	struct foreach_seq_as_heap_vector_data *data;
	data = (struct foreach_seq_as_heap_vector_data *)arg;
	ASSERT(data->sahvd_size <= data->sahvd_alloc);
	if unlikely(data->sahvd_size >= data->sahvd_alloc) {
		/* Must allocate more memory. */
		size_t new_alloc;
		DREF DeeObject **new_vector;
		new_alloc = data->sahvd_alloc * 2;
		if unlikely(new_alloc <= data->sahvd_size) {
			new_alloc = 16;
			if unlikely(new_alloc <= data->sahvd_size)
				new_alloc = data->sahvd_size + 1;
		}
		new_vector = Dee_objectlist_elemv_tryrealloc(data->sahvd_vector, new_alloc);
		if unlikely(!new_vector) {
			new_alloc  = data->sahvd_size + 1;
			new_vector = Dee_objectlist_elemv_realloc(data->sahvd_vector, new_alloc);
			if unlikely(!new_vector)
				goto err;
		}
		data->sahvd_vector = new_vector;
		data->sahvd_alloc  = new_alloc;
	}
	data->sahvd_vector[data->sahvd_size++] = elem;
	Dee_Incref(elem);
	return 0;
err:
	return -1;
}


/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*p_length'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    /*[out]*/ size_t *__restrict p_length) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	struct foreach_seq_as_heap_vector_data data;
	if unlikely(!(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	              unlikely(DeeType_InheritIter(tp_self)))) {
		err_unimplemented_operator(tp_self, OPERATOR_ITER);
		goto err;
	}

	/* Try to use "tp_size_fast" to get a good guess regarding the initial buffer size. */
	if likely(tp_self->tp_seq->tp_size_fast) {
		data.sahvd_alloc = (*tp_self->tp_seq->tp_size_fast)(self);
		if (data.sahvd_alloc == (size_t)-1)
			data.sahvd_alloc = 16;
	} else {
		data.sahvd_alloc = 16;
	}
	data.sahvd_vector = Dee_objectlist_elemv_trymalloc(data.sahvd_alloc);
	if unlikely(!data.sahvd_vector) {
		data.sahvd_alloc  = 1;
		data.sahvd_vector = Dee_objectlist_elemv_malloc(1);
		if unlikely(!data.sahvd_vector)
			goto err;
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_self->tp_seq->tp_asvector) {
		DREF DeeObject **new_vector;
again_asvector:
		data.sahvd_size = (*tp_self->tp_seq->tp_asvector)(self, data.sahvd_alloc, data.sahvd_vector);
		if likely(data.sahvd_size <= data.sahvd_alloc) {
			if unlikely(data.sahvd_size < data.sahvd_alloc)
				goto resize_data_and_done;
			goto done;
		}
		if unlikely(data.sahvd_size == (size_t)-1)
			goto err_vector;
		/* Need a larger vector buffer. */
		data.sahvd_alloc = data.sahvd_size;
		new_vector = Dee_objectlist_elemv_realloc(data.sahvd_vector,
		                                          data.sahvd_alloc);
		if unlikely(!new_vector)
			goto err_vector;
		data.sahvd_vector = new_vector;
		goto again_asvector;
	}

	/* Fallback: use "tp_foreach" to enumerate sequence items. */
	data.sahvd_size = 0;
	if unlikely((*tp_self->tp_seq->tp_foreach)(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_vector_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);

	/* Free unused memory. */
	if (data.sahvd_size < data.sahvd_alloc) {
		DREF DeeObject **new_vector;
resize_data_and_done:
		new_vector = Dee_objectlist_elemv_tryrealloc(data.sahvd_vector,
		                                             data.sahvd_size);
		if likely(new_vector)
			data.sahvd_vector = new_vector;
	}

	/* Save the resulting length. */
done:
	*p_length = data.sahvd_size;
	return data.sahvd_vector;
err_vector_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
err_vector:
	Dee_Free(data.sahvd_vector);
err:
	return NULL;
}

#ifdef Dee_MallocUsableSize
PUBLIC WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             /*[out]*/ size_t *__restrict p_length,
                             /*[out]*/ size_t *__restrict p_allocated) {
	DREF DeeObject **result;
	result       = DeeSeq_AsHeapVectorWithAlloc2(self, p_length);
	*p_allocated = Dee_objectlist_elemv_usable_size(result);
	return result;
}
PUBLIC WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc2(DeeObject *__restrict self,
                              /*[out]*/ size_t *__restrict p_length)
#else /* Dee_MallocUsableSize */
PUBLIC WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc2(DeeObject *__restrict self,
                              /*[out]*/ size_t *__restrict p_length) {
	size_t allocated; /* For binary compatibility... */
	return DeeSeq_AsHeapVectorWithAlloc(self, p_length, &allocated);
}
PUBLIC WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             /*[out]*/ size_t *__restrict p_length,
                             /*[out]*/ size_t *__restrict p_allocated)
#endif /* !Dee_MallocUsableSize */
{
	DeeTypeObject *tp_self = Dee_TYPE(self);
	struct foreach_seq_as_heap_vector_data data;
	if unlikely(!(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	              unlikely(DeeType_InheritIter(tp_self)))) {
		err_unimplemented_operator(tp_self, OPERATOR_ITER);
		goto err;
	}

	/* Try to use "tp_size_fast" to get a good guess regarding the initial buffer size. */
	if likely(tp_self->tp_seq->tp_size_fast) {
		data.sahvd_alloc = (*tp_self->tp_seq->tp_size_fast)(self);
		if (data.sahvd_alloc == (size_t)-1)
			data.sahvd_alloc = 16;
	} else {
		data.sahvd_alloc = 16;
	}
	data.sahvd_vector = Dee_objectlist_elemv_trymalloc(data.sahvd_alloc);
	if unlikely(!data.sahvd_vector) {
		data.sahvd_alloc  = 1;
		data.sahvd_vector = Dee_objectlist_elemv_malloc(1);
		if unlikely(!data.sahvd_vector)
			goto err;
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_self->tp_seq->tp_asvector) {
		DREF DeeObject **new_vector;
again_asvector:
		data.sahvd_size = (*tp_self->tp_seq->tp_asvector)(self, data.sahvd_alloc, data.sahvd_vector);
		if likely(data.sahvd_size <= data.sahvd_alloc)
			goto done;
		if unlikely(data.sahvd_size == (size_t)-1)
			goto err_vector;
		/* Need a larger vector buffer. */
		data.sahvd_alloc = data.sahvd_size;
		new_vector = Dee_objectlist_elemv_realloc(data.sahvd_vector,
		                                          data.sahvd_alloc);
		if unlikely(!new_vector)
			goto err_vector;
		data.sahvd_vector = new_vector;
		goto again_asvector;
	}

	/* Fallback: use "tp_foreach" to enumerate sequence items. */
	data.sahvd_size = 0;
	if unlikely((*tp_self->tp_seq->tp_foreach)(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_vector_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);

	/* Save the resulting length. */
done:
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	*p_length = data.sahvd_size;
	return data.sahvd_vector;
err_vector_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
err_vector:
	Dee_Free(data.sahvd_vector);
err:
	return NULL;
}


/* Same as `DeeSeq_AsHeapVectorWithAlloc()', however also inherit
 * a pre-allocated heap-vector `*p_vector' with an allocated size
 * of `IN(*p_allocated) * sizeof(DeeObject *)', which is updated
 * as more memory needs to be allocated.
 * NOTE: `*p_vector' may be updated to point to a new vector, even
 *       when the function fails (i.e. (size_t)-1 is returned)
 * @param: p_vector:    A pointer to a preallocated object-vector `[0..IN(*p_allocated)]'
 *                      May only point to a `NULL' vector when `IN(*p_allocated)' is ZERO(0).
 *                      Upon return, this pointer may have been updated to point to a
 *                      realloc()-ated vector, should the need to allocate more memory
 *                      have arisen.
 * @param: p_allocated: A pointer to an information field describing how much pointers
 *                      are allocated upon entry / how much are allocated upon exit.
 *                      Just as `p_vector', this pointer may be updated, even upon error.
 * @return: * :         The amount of filled in objects in `*p_vector'
 * @return: (size_t)-1: An error occurred. Note that both `*p_vector' and `*p_allocated'
 *                      may have been modified since entry, with their original values
 *                      no longer being valid! */
#ifdef Dee_MallocUsableSize
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                  /*in-out*/ size_t *__restrict p_allocated) {
	size_t result;
	ASSERT(*p_allocated <= Dee_objectlist_elemv_usable_size(*p_vector));
	result       = DeeSeq_AsHeapVectorWithAllocReuse2(self, p_vector);
	*p_allocated = Dee_objectlist_elemv_usable_size(*p_vector);
	return result;
}
PUBLIC WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse2(DeeObject *__restrict self,
                                   /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector)
#else /* Dee_MallocUsableSize */
PUBLIC WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse2(DeeObject *__restrict self,
                                   /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector) {
	/* For binary compatibility... */
	size_t allocated = 0;
	return DeeSeq_AsHeapVectorWithAllocReuse(self, p_vector, &allocated);
}
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                  /*in-out*/ size_t *__restrict p_allocated)
#endif /* !Dee_MallocUsableSize */
{
	DeeTypeObject *tp_self = Dee_TYPE(self);
	struct foreach_seq_as_heap_vector_data data;
	if unlikely(!(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	              unlikely(DeeType_InheritIter(tp_self)))) {
		err_unimplemented_operator(tp_self, OPERATOR_ITER);
		goto err;
	}

	data.sahvd_vector = *p_vector;
#ifdef Dee_MallocUsableSize
	data.sahvd_alloc = Dee_objectlist_elemv_usable_size(data.sahvd_vector);
#else /* Dee_MallocUsableSize */
	data.sahvd_alloc = *p_allocated;
#endif /* !Dee_MallocUsableSize */
	ASSERT(!data.sahvd_alloc || data.sahvd_vector);

	/* Try to use "tp_size_fast" to get a good guess regarding the initial buffer size. */
	if likely(tp_self->tp_seq->tp_size_fast) {
		size_t min_alloc;
		min_alloc = (*tp_self->tp_seq->tp_size_fast)(self);
		if (min_alloc != (size_t)-1 && min_alloc > data.sahvd_alloc) {
			DREF DeeObject **new_vector;
			new_vector = Dee_objectlist_elemv_tryrealloc(data.sahvd_vector, min_alloc);
			if likely(new_vector) {
				data.sahvd_vector = new_vector;
				data.sahvd_alloc  = min_alloc;
			}
		}
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_self->tp_seq->tp_asvector) {
		DREF DeeObject **new_vector;
again_asvector:
		data.sahvd_size = (*tp_self->tp_seq->tp_asvector)(self, data.sahvd_alloc, data.sahvd_vector);
		if likely(data.sahvd_size <= data.sahvd_alloc)
			goto done;
		if unlikely(data.sahvd_size == (size_t)-1)
			goto done;
		/* Need a larger vector buffer. */
		data.sahvd_alloc = data.sahvd_size;
		new_vector = Dee_objectlist_elemv_realloc(data.sahvd_vector,
		                                          data.sahvd_alloc);
		if unlikely(!new_vector)
			goto err_writeback;
		data.sahvd_vector = new_vector;
		goto again_asvector;
	}

	/* Fallback: use "tp_foreach" to enumerate sequence items. */
	data.sahvd_size = 0;
	if unlikely((*tp_self->tp_seq->tp_foreach)(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_writeback_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);

done:
	*p_vector = data.sahvd_vector;
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	return data.sahvd_size;
err_writeback_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
err_writeback:
	data.sahvd_size = (size_t)-1;
	goto done;
err:
	return (size_t)-1;
}


/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*p_allocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*p_vector)[offset]', rather than `(*p_vector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
#ifdef Dee_MallocUsableSize
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                        /*in-out*/ size_t *__restrict p_allocated,
                                        /*in*/ size_t offset) {
	size_t result;
	ASSERT(*p_allocated <= Dee_objectlist_elemv_usable_size(*p_vector));
	result       = DeeSeq_AsHeapVectorWithAllocReuseOffset2(self, p_vector, offset);
	*p_allocated = Dee_objectlist_elemv_usable_size(*p_vector);
	return result;
}
PUBLIC WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset2(DeeObject *__restrict self,
                                         /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                         /*in*/ size_t offset)
#else /* Dee_MallocUsableSize */
PUBLIC WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset2(DeeObject *__restrict self,
                                         /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                         /*in*/ size_t offset) {
	/* For binary compatibility... */
	size_t allocated = offset;
	return DeeSeq_AsHeapVectorWithAllocReuseOffset(self, p_vector, &allocated, offset);
}
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                        /*in-out*/ size_t *__restrict p_allocated,
                                        /*in*/ size_t offset)
#endif /* !Dee_MallocUsableSize */
{
	DeeTypeObject *tp_self = Dee_TYPE(self);
	struct foreach_seq_as_heap_vector_data data;
	if unlikely(!(likely(tp_self->tp_seq && tp_self->tp_seq->tp_foreach) ||
	              unlikely(DeeType_InheritIter(tp_self)))) {
		err_unimplemented_operator(tp_self, OPERATOR_ITER);
		goto err;
	}

	data.sahvd_vector = *p_vector;
#ifdef Dee_MallocUsableSize
	data.sahvd_alloc = Dee_objectlist_elemv_usable_size(data.sahvd_vector);
#else /* Dee_MallocUsableSize */
	data.sahvd_alloc = *p_allocated;
#endif /* !Dee_MallocUsableSize */
	ASSERT(data.sahvd_alloc >= offset);
	ASSERT(!data.sahvd_alloc || data.sahvd_vector);

	/* Try to use "tp_size_fast" to get a good guess regarding the initial buffer size. */
	if likely(tp_self->tp_seq->tp_size_fast) {
		size_t min_alloc;
		min_alloc = (*tp_self->tp_seq->tp_size_fast)(self);
		if (min_alloc != (size_t)-1 && !OVERFLOW_UADD(min_alloc, offset, &min_alloc)) {
			if (min_alloc > data.sahvd_alloc) {
				DREF DeeObject **new_vector;
				new_vector = Dee_objectlist_elemv_tryrealloc(data.sahvd_vector, min_alloc);
				if likely(new_vector) {
					data.sahvd_vector = new_vector;
					data.sahvd_alloc  = min_alloc;
				}
			}
		}
	}

	/* Check if the type supports the "tp_asvector" extension. */
	if (tp_self->tp_seq->tp_asvector) {
		DREF DeeObject **new_vector;
again_asvector:
		data.sahvd_size = (*tp_self->tp_seq->tp_asvector)(self,
		                                                  data.sahvd_alloc - offset,
		                                                  data.sahvd_vector + offset);
		if likely(data.sahvd_size <= (data.sahvd_alloc - offset)) {
			data.sahvd_size += offset;
			goto done;
		}
		if unlikely(data.sahvd_size == (size_t)-1) {
			data.sahvd_size += offset;
			goto done;
		}
		/* Need a larger vector buffer. */
		if unlikely(OVERFLOW_UADD(data.sahvd_size, offset, &data.sahvd_alloc)) {
			Dee_BadAlloc((size_t)-1);
			goto err_writeback;
		}
		new_vector = Dee_objectlist_elemv_realloc(data.sahvd_vector,
		                                          data.sahvd_alloc);
		if unlikely(!new_vector)
			goto err_writeback;
		data.sahvd_vector = new_vector;
		goto again_asvector;
	}

	/* Use `DeeObject_Foreach()' */
	data.sahvd_size = offset;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);
	if unlikely((*tp_self->tp_seq->tp_foreach)(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_writeback_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);
	ASSERT(data.sahvd_size >= offset);

done:
	*p_vector = data.sahvd_vector;
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	return data.sahvd_size - offset;
err_writeback_data:
	Dee_Decrefv(data.sahvd_vector + offset,
	            data.sahvd_size - offset);
err_writeback:
	data.sahvd_size = (size_t)-1;
	goto done;
err:
	return (size_t)-1;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C */
