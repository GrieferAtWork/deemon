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

#include "../../runtime/runtime_error.h"
#include "range.h"
#include "subrange.h"
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

/* Same as `DeeFastSeq_GetItem_deprecated()', but returns ITER_DONE if an error
 * occurred, and `NULL' if the item has been marked as unbound. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItemUnbound_deprecated(DeeObject *__restrict self, size_t index) {
	DREF DeeObject *result;
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
		result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
		ASSERT(result != ITER_DONE);
		return result;
	}
	ASSERT(nsi->nsi_seqlike.nsi_getitem);
	result = (*nsi->nsi_seqlike.nsi_getitem)(self, index);
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}



/* An alternative (and more restrictive) variant of the FastSeq-interface:
 *  - Semantically, these functions are used the same way as the regular interface
 *  - Unlike the functions above, these are guarantied to be non-blocking
 *    -> However, an atomic lock doesn't count as something that would block,
 *       yet because this means that `DeeFastSeq_GetItemNB_deprecated()' can never throw
 *       an exception, it also means that any sequence who's size could change
 *       at any time (such as `List') cannot be used here.
 * The following types function as fast-sequence-compatible-nb:
 *  - Tuple
 *  - _SharedVector   (If the sequence is cleared while being used here, `none' will be returned)
 *  - _SeqSubRange    (Only if the sub-ranged sequence is a fast-sequence-nb) */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSizeNB_deprecated(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeTuple_Type)
		return DeeTuple_SIZE(self);
	if (tp_self == &DeeSharedVector_Type)
		return ((SharedVector *)self)->sv_length;
	return DEE_FASTSEQ_NOTFAST_DEPRECATED;
}

PUBLIC ATTR_RETNONNULL DREF DeeObject *DCALL
DeeFastSeq_GetItemNB_deprecated(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeTuple_Type) {
		result = DeeTuple_GET(self, index);
		return_reference_(result);
	}
	ASSERT(tp_self == &DeeSharedVector_Type);
	SharedVector_LockRead((SharedVector *)self);
	if unlikely(index >= ((SharedVector *)self)->sv_length) {
		SharedVector_LockEndRead((SharedVector *)self);
		return_none;
	}
	result = ((SharedVector *)self)->sv_vector[index];
	Dee_Incref(result);
	SharedVector_LockEndRead((SharedVector *)self);
	return result;
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
		new_vector = (DREF DeeObject **)Dee_TryReallocc(data->sahvd_vector, new_alloc,
		                                                sizeof(DREF DeeObject *));
		if unlikely(!new_vector) {
			new_alloc  = data->sahvd_size + 1;
			new_vector = (DREF DeeObject **)Dee_Reallocc(data->sahvd_vector, new_alloc,
			                                             sizeof(DREF DeeObject *));
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
                    size_t *__restrict p_length) {
	struct foreach_seq_as_heap_vector_data data;
	data.sahvd_size = DeeFastSeq_GetSize_deprecated(self);
	if (data.sahvd_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		size_t i;
		/* Optimization for fast-sequence-compatible objects. */
		*p_length = data.sahvd_size;
		data.sahvd_vector = (DREF DeeObject **)Dee_Mallocc(data.sahvd_size, sizeof(DREF DeeObject *));
		if unlikely(!data.sahvd_vector)
			goto err;
		if (DeeTuple_Check(self)) {
			/* Further optimization: tuple can be memcpy'd */
			ASSERT(data.sahvd_size == DeeTuple_SIZE(self));
			return Dee_Movrefv(data.sahvd_vector, DeeTuple_ELEM(self), data.sahvd_size);
		}
		for (i = 0; i < data.sahvd_size; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItem_deprecated(self, i);
			if unlikely(!elem) {
				data.sahvd_size = i;
				goto err_data;
			}
			data.sahvd_vector[i] = elem;
		}
		return data.sahvd_vector;
	}

	/* Use `DeeObject_Foreach()' */
	data.sahvd_size   = 0;
	data.sahvd_alloc  = 16;
	data.sahvd_vector = (DREF DeeObject **)Dee_TryMallocc(16, sizeof(DREF DeeObject *));
	if unlikely(!data.sahvd_vector) {
		data.sahvd_alloc  = 1;
		data.sahvd_vector = (DREF DeeObject **)Dee_Mallocc(1, sizeof(DREF DeeObject *));
		if unlikely(!data.sahvd_vector)
			goto err;
	}
	if unlikely(DeeObject_Foreach(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);

	/* Free unused memory. */
	if (data.sahvd_size < data.sahvd_alloc) {
		DREF DeeObject **new_vector;
		new_vector = (DREF DeeObject **)Dee_TryReallocc(data.sahvd_vector,
		                                                data.sahvd_size,
		                                                sizeof(DREF DeeObject *));
		if likely(new_vector)
			data.sahvd_vector = new_vector;
	}

	/* Save the resulting length. */
	*p_length = data.sahvd_size;
	return data.sahvd_vector;
err_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
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
	*p_allocated = Dee_MallocUsableSize(result) / sizeof(DREF DeeObject *);
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
	struct foreach_seq_as_heap_vector_data data;
	data.sahvd_size = DeeFastSeq_GetSize_deprecated(self);
	if (data.sahvd_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		size_t i;
		/* Optimization for fast-sequence-compatible objects. */
#ifndef Dee_MallocUsableSize
		*p_allocated = data.sahvd_size;
#endif /* !Dee_MallocUsableSize */
		*p_length = data.sahvd_size;
		data.sahvd_vector = (DREF DeeObject **)Dee_Mallocc(data.sahvd_size, sizeof(DREF DeeObject *));
		if unlikely(!data.sahvd_vector)
			goto err;
		if (DeeTuple_Check(self)) {
			/* Further optimization: tuple can be memcpy'd */
			ASSERT(data.sahvd_size == DeeTuple_SIZE(self));
			return Dee_Movrefv(data.sahvd_vector, DeeTuple_ELEM(self), data.sahvd_size);
		}
		for (i = 0; i < data.sahvd_size; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItem_deprecated(self, i);
			if unlikely(!elem) {
				data.sahvd_size = i;
				goto err_data;
			}
			data.sahvd_vector[i] = elem;
		}
		return data.sahvd_vector;
	}

	/* Use `DeeObject_Foreach()' */
	data.sahvd_size   = 0;
	data.sahvd_alloc  = 16;
	data.sahvd_vector = (DREF DeeObject **)Dee_TryMallocc(16, sizeof(DREF DeeObject *));
	if unlikely(!data.sahvd_vector) {
		data.sahvd_alloc  = 1;
		data.sahvd_vector = (DREF DeeObject **)Dee_Mallocc(1, sizeof(DREF DeeObject *));
		if unlikely(!data.sahvd_vector)
			goto err;
	}
	if unlikely(DeeObject_Foreach(self, &foreach_seq_as_heap_vector_cb, &data))
		goto err_data;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);

	/* Save the resulting length, and allocation. */
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	*p_length = data.sahvd_size;
	return data.sahvd_vector;
err_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
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
	ASSERT(*p_allocated <= Dee_MallocUsableSize(*p_vector) / sizeof(DeeObject **));
	result       = DeeSeq_AsHeapVectorWithAllocReuse2(self, p_vector);
	*p_allocated = Dee_MallocUsableSize(*p_vector) / sizeof(DeeObject **);
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
	Dee_ssize_t error;
	struct foreach_seq_as_heap_vector_data data;
	data.sahvd_vector = *p_vector;
#ifdef Dee_MallocUsableSize
	data.sahvd_alloc = Dee_MallocUsableSize(data.sahvd_vector) / sizeof(DREF DeeObject *);
#else /* Dee_MallocUsableSize */
	data.sahvd_alloc = *p_allocated;
#endif /* !Dee_MallocUsableSize */
	ASSERT(!data.sahvd_alloc || data.sahvd_vector);

	data.sahvd_size = DeeFastSeq_GetSize_deprecated(self);
	if (data.sahvd_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		/* Fast sequence optimizations. */
		if (data.sahvd_size > data.sahvd_alloc) {
			DeeObject **new_vector;
			new_vector = (DeeObject **)Dee_Reallocc(data.sahvd_vector,
			                                        data.sahvd_size,
			                                        sizeof(DREF DeeObject *));
			if unlikely(!new_vector)
				goto err;
			data.sahvd_vector = new_vector;
			*p_vector         = new_vector;
#ifndef Dee_MallocUsableSize
			*p_allocated = data.sahvd_size;
#endif /* !Dee_MallocUsableSize */
		}
		if (DeeTuple_Check(self)) {
			/* Further optimization: tuple can be memcpy'd */
			ASSERT(data.sahvd_size == DeeTuple_SIZE(self));
			Dee_Movrefv(data.sahvd_vector, DeeTuple_ELEM(self), data.sahvd_size);
		} else {
			size_t i;
			for (i = 0; i < data.sahvd_size; ++i) {
				DREF DeeObject *elem;
				elem = DeeFastSeq_GetItem_deprecated(self, i);
				if unlikely(!elem) {
					data.sahvd_size = i;
					goto err_data;
				}
				data.sahvd_vector[i] = elem; /* Inherit reference. */
			}
		}
		return data.sahvd_size;
	}

	/* Use `DeeObject_Foreach()' */
	data.sahvd_size = 0;
	error = DeeObject_Foreach(self, &foreach_seq_as_heap_vector_cb, &data);
	ASSERT(data.sahvd_size <= data.sahvd_alloc);
	*p_vector = data.sahvd_vector;
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	if unlikely(error)
		goto err_data;
	return data.sahvd_size;
err_data:
	Dee_Decrefv(data.sahvd_vector, data.sahvd_size);
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
	ASSERT(*p_allocated <= Dee_MallocUsableSize(*p_vector) / sizeof(DREF DeeObject **));
	result       = DeeSeq_AsHeapVectorWithAllocReuseOffset2(self, p_vector, offset);
	*p_allocated = Dee_MallocUsableSize(*p_vector) / sizeof(DREF DeeObject **);
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
	Dee_ssize_t error;
	struct foreach_seq_as_heap_vector_data data;
	data.sahvd_vector = *p_vector;
#ifdef Dee_MallocUsableSize
	data.sahvd_alloc = Dee_MallocUsableSize(data.sahvd_vector) / sizeof(DREF DeeObject *);
#else /* Dee_MallocUsableSize */
	data.sahvd_alloc = *p_allocated;
#endif /* !Dee_MallocUsableSize */
	ASSERT(data.sahvd_alloc >= offset);
	ASSERT(!data.sahvd_alloc || data.sahvd_vector);

	data.sahvd_size = DeeFastSeq_GetSize_deprecated(self);
	if (data.sahvd_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		/* Fast sequence optimizations. */
		if (data.sahvd_size > (data.sahvd_alloc - offset)) {
			DeeObject **new_vector;
			data.sahvd_alloc = offset + data.sahvd_size;
			new_vector = (DeeObject **)Dee_Reallocc(data.sahvd_vector,
			                                        data.sahvd_alloc,
			                                        sizeof(DeeObject *));
			if unlikely(!new_vector)
				goto err;
			data.sahvd_vector = new_vector;
			*p_vector         = new_vector;
#ifndef Dee_MallocUsableSize
			*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
		}
		if (DeeTuple_Check(self)) {
			/* Further optimization: tuple can be memcpy'd */
			ASSERT(data.sahvd_size == DeeTuple_SIZE(self));
			Dee_Movrefv(data.sahvd_vector + offset, DeeTuple_ELEM(self), data.sahvd_size);
		} else {
			size_t i;
			for (i = 0; i < data.sahvd_size; ++i) {
				DREF DeeObject *elem;
				elem = DeeFastSeq_GetItem_deprecated(self, i);
				if unlikely(!elem) {
					data.sahvd_size = offset + i;
					goto err_data;
				}
				data.sahvd_vector[offset + i] = elem; /* Inherit reference. */
			}
		}
		return data.sahvd_size;
	}

	/* Use `DeeObject_Foreach()' */
	data.sahvd_size = offset;
	ASSERT(data.sahvd_size <= data.sahvd_alloc);
	error = DeeObject_Foreach(self, &foreach_seq_as_heap_vector_cb, &data);
	ASSERT(data.sahvd_size <= data.sahvd_alloc);
	ASSERT(data.sahvd_size >= offset);
	*p_vector = data.sahvd_vector;
#ifndef Dee_MallocUsableSize
	*p_allocated = data.sahvd_alloc;
#endif /* !Dee_MallocUsableSize */
	if unlikely(error)
		goto err_data;
	return data.sahvd_size - offset;
err_data:
	Dee_Decrefv(data.sahvd_vector + offset,
	            data.sahvd_size - offset);
err:
	return (size_t)-1;
}


/* Same as `DeeObject_Unpack()', but handle `DeeError_UnboundItem'
 * by filling in the resp. element from `objv[*]' with `NULL'.
 * This function is implemented to try the following things with `self' (in order):
 *  - Use `DeeFastSeq_GetSize_deprecated()' + `DeeFastSeq_GetItemUnbound_deprecated()'
 *    Try next when `DeeFastSeq_GetSize_deprecated() == DEE_FASTSEQ_NOTFAST_DEPRECATED'
 *  - Use `DeeObject_Size()' + `DeeObject_GetItemIndex()'
 *    Try next when `DeeObject_Size()' throws `DeeError_NotImplemented', or
 *    `DeeObject_GetItemIndex()' (first call only) throws `DeeError_NotImplemented'
 *    or `DeeError_TypeError'.
 *  - Use `DeeObject_Unpack()' (meaning that all elements written to `objv' will be non-NULL)
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED ATTR_OUTS(3, 2) NONNULL((1)) int
(DCALL DeeObject_UnpackWithUnbound)(DeeObject *__restrict self, size_t objc,
                                    /*out*/ DREF DeeObject **__restrict objv) {
	size_t i, size;
	size = DeeFastSeq_GetSize_deprecated(self);
	if (size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (size != objc)
			goto err_badsize;
		for (i = 0; i < objc; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItemUnbound_deprecated(self, i);
			if unlikely(elem == ITER_DONE) {
				Dee_XDecrefv(objv, i);
				goto err;
			}
			objv[i] = elem; /* Inherit reference */
		}
		return 0;
	}
	size = DeeObject_Size(self);
	if (size != (size_t)-1) {
		if (size != objc)
			goto err_badsize;
		for (i = 0; i < objc; ++i) {
			DREF DeeObject *elem;
			elem = DeeObject_GetItemIndex(self, i);
			if (!elem && !DeeError_Catch(&DeeError_UnboundItem)) {
				/* The first time around, try to handle these errors
				 * in which case we'll do the fallback method instead. */
				if (i == 0 &&
				    (DeeError_Catch(&DeeError_NotImplemented) ||
				     DeeError_Catch(&DeeError_TypeError)))
					goto fallback;
				goto err;
			}
			objv[i] = elem; /* Inherit reference */
		}
		return 0;
	}
	if (!DeeError_Catch(&DeeError_NotImplemented))
		goto err;
fallback:
	return DeeObject_Unpack(self, objc, objv);
err_badsize:
	return err_invalid_unpack_size(self, objc, size);
err:
	return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FASTSEQ_C */
