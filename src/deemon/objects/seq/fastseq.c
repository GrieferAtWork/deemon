/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
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
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "../../runtime/runtime_error.h"
#include "range.h"
#include "subrange.h"
#include "svec.h"
#include "transform.h"

DECL_BEGIN

STATIC_ASSERT_MSG(DEE_FASTSEQ_NOTFAST == (size_t)-1,
                  "`nsi_getsize_fast' assumes this correlation");


/* Check if `self' is a fast-sequence object, and return its (current)
 * length if it is, or return `DEE_FASTSEQ_NOTFAST' if it isn't.
 * A fast-sequence object is a vector-based object implemented by the
 * deemon C-core, meaning that its size can quickly be determined,
 * and items can quickly be accessed, given their index.
 * The following types function as fast-sequence-compatible:
 *  - Tuple
 *  - List
 *  - _SharedVector      (Created by a `ASM_CALL_SEQ' instruction -- `call top, {#X}')
 *  - _SeqSubRange       (Only if the sub-ranged sequence is a fast-sequence)
 *  - _SeqSubRangeN      (*ditto*)
 *  - _SeqTransformation (Only if the sequence being transformed is a fast-sequence)
 *  - _SeqIntRange
 *  - string
 *  - Bytes
 * Sub-classes of these types are not fast-sequence-compatible. */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSize(DeeObject *__restrict self) {
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
	return DEE_FASTSEQ_NOTFAST;
}


/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize(self)' when
 * `self' is an immutable sequence (anything other than `List' and `_SharedVector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItem(DeeObject *__restrict self, size_t index) {
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

/* Same as `DeeFastSeq_GetItem()', but returns ITER_DONE if an error
 * occurred, and `NULL' if the item has been marked as unbound. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItemUnbound(DeeObject *__restrict self, size_t index) {
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
 *       yet because this means that `DeeFastSeq_GetItemNB()' can never throw
 *       an exception, it also means that any sequence who's size could change
 *       at any time (such as `List') cannot be used here.
 * The following types function as fast-sequence-compatible-nb:
 *  - Tuple
 *  - _SharedVector   (If the sequence is cleared while being used here, `none' will be returned)
 *  - _SeqSubRange    (Only if the sub-ranged sequence is a fast-sequence-nb) */
PUBLIC WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSizeNB(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeTuple_Type)
		return DeeTuple_SIZE(self);
	if (tp_self == &SharedVector_Type)
		return ((SharedVector *)self)->sv_length;
	return DEE_FASTSEQ_NOTFAST;
}

PUBLIC ATTR_RETNONNULL DREF DeeObject *DCALL
DeeFastSeq_GetItemNB(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeTuple_Type) {
		result = DeeTuple_GET(self, index);
		return_reference_(result);
	}
	ASSERT(tp_self == &SharedVector_Type);
	rwlock_read(&((SharedVector *)self)->sv_lock);
	if unlikely(index >= ((SharedVector *)self)->sv_length) {
		rwlock_endread(&((SharedVector *)self)->sv_lock);
		return_none;
	}
	result = ((SharedVector *)self)->sv_vector[index];
	Dee_Incref(result);
	rwlock_endread(&((SharedVector *)self)->sv_lock);
	return result;
}


/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*plength'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    size_t *__restrict plength) {
	size_t i, fastsize, alloc_size;
	DREF DeeObject **result, *iter, *elem, **new_result;
	fastsize = DeeFastSeq_GetSize(self);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		/* Optimization for fast-sequence-compatible objects. */
		*plength = fastsize;
		result   = (DREF DeeObject **)Dee_Malloc(fastsize * sizeof(DREF DeeObject *));
		if unlikely(!result)
			goto err;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_r_i;
			result[i] = elem;
		}
		goto done;
	}
	/* Must use iterators. */
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	alloc_size = 16, i = 0;
	result = (DREF DeeObject **)Dee_TryMalloc(alloc_size * sizeof(DREF DeeObject *));
	if unlikely(!result) {
		alloc_size = 1;
		result = (DREF DeeObject **)Dee_Malloc(alloc_size * sizeof(DREF DeeObject *));
		goto err_r_iter;
	}
	/* Iterate items. */
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		ASSERT(i <= alloc_size);
		if unlikely(i >= alloc_size) {
			/* Must allocate more memory. */
			size_t new_alloc_size;
			new_alloc_size = alloc_size * 2;
			new_result = (DREF DeeObject **)Dee_TryRealloc(result,
			                                               new_alloc_size *
			                                               sizeof(DREF DeeObject *));
			if unlikely(!new_result) {
				new_alloc_size = i + 1;
				new_result = (DREF DeeObject **)Dee_Realloc(result,
				                                            new_alloc_size *
				                                            sizeof(DREF DeeObject *));
				if unlikely(!new_result)
					goto err_r_iter_elem;
			}
			result     = new_result;
			alloc_size = new_alloc_size;
		}
		result[i++] = elem; /* Inherit reference. */
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iter);
	ASSERT(i <= alloc_size);
	/* Free unused memory. */
	if (i != alloc_size) {
		new_result = (DREF DeeObject **)Dee_TryRealloc(result,
		                                               i * sizeof(DREF DeeObject *));
		if likely(new_result)
			result = new_result;
	}
	/* Save the resulting length. */
	*plength = i;
done:
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_Decref(iter);
err_r_i:
	while (i--)
		Dee_Decref(result[i]);
	Dee_Free(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             size_t *__restrict plength,
                             size_t *__restrict pallocated) {
	size_t i, fastsize, alloc_size;
	DREF DeeObject **result, *iter, *elem, **new_result;
	fastsize = DeeFastSeq_GetSize(self);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		/* Optimization for fast-sequence-compatible objects. */
		*plength = *pallocated = fastsize;
		result                 = (DREF DeeObject **)Dee_Malloc(fastsize * sizeof(DREF DeeObject *));
		if unlikely(!result)
			goto err;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_r_i;
			result[i] = elem;
		}
		goto done;
	}
	/* Must use iterators. */
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	alloc_size = 16, i = 0;
	result = (DREF DeeObject **)Dee_TryMalloc(alloc_size * sizeof(DREF DeeObject *));
	if unlikely(!result) {
		alloc_size = 1;
		result     = (DREF DeeObject **)Dee_Malloc(alloc_size * sizeof(DREF DeeObject *));
		goto err_r_iter;
	}
	/* Iterate items. */
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		ASSERT(i <= alloc_size);
		if unlikely(i >= alloc_size) {
			/* Must allocate more memory. */
			size_t new_alloc_size = alloc_size * 2;
			new_result            = (DREF DeeObject **)Dee_TryRealloc(result, new_alloc_size * sizeof(DREF DeeObject *));
			if unlikely(!new_result) {
				new_alloc_size = i + 1;
				new_result     = (DREF DeeObject **)Dee_Realloc(result, new_alloc_size * sizeof(DREF DeeObject *));
				if unlikely(!new_result)
					goto err_r_iter_elem;
			}
			result     = new_result;
			alloc_size = new_alloc_size;
		}
		result[i++] = elem; /* Inherit reference. */
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iter);
	ASSERT(i <= alloc_size);
	/* Save the resulting length, and allocation. */
	*pallocated = alloc_size;
	*plength    = i;
done:
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_Decref(iter);
err_r_i:
	while (i--)
		Dee_Decref(result[i]);
	Dee_Free(result);
err:
	return NULL;
}


/* Same as `DeeSeq_AsHeapVectorWithAlloc()', however also inherit
 * a pre-allocated heap-vector `*pvector' with an allocated size
 * of `IN(*pallocated) * sizeof(DeeObject *)', which is updated
 * as more memory needs to be allocated.
 * NOTE: `*pvector' may be updated to point to a new vector, even
 *       when the function fails (i.e. (size_t)-1 is returned)
 * @param: pvector:     A pointer to a preallocated object-vector `[0..IN(*pallocated)]'
 *                      May only point to a `NULL' vector when `IN(*pallocated)' is ZERO(0).
 *                      Upon return, this pointer may have been updated to point to a
 *                      realloc()-ated vector, should the need to allocate more memory
 *                      have arisen.
 * @param: pallocated:  A pointer to an information field describing how much pointers
 *                      are allocated upon entry / how much are allocated upon exit.
 *                      Just as `pvector', this pointer may be updated, even upon error.
 * @return: * :         The amount of filled in objects in `*pvector'
 * @return: (size_t)-1: An error occurred. Note that both `*pvector' and `*pallocated'
 *                      may have been modified since entry, with their original values
 *                      no longer being valid! */
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DeeObject ***__restrict pvector,
                                  /*in-out*/ size_t *__restrict pallocated) {
	DeeObject **new_elemv, **elemv = *pvector;
	DREF DeeObject *iterator, *elem;
	size_t elema = *pallocated;
	size_t elemc, i;
	ASSERT(!elema || elemv);
	elemc = DeeFastSeq_GetSize(self);
	if (elemc != DEE_FASTSEQ_NOTFAST) {
		/* Fast sequence optimizations. */
		if (elemc > elema) {
			new_elemv = (DeeObject **)Dee_Realloc(elemv, elemc * sizeof(DeeObject *));
			if unlikely(!new_elemv)
				goto err;
			*pvector = elemv = new_elemv;
			*pallocated      = elemc;
		}
		for (i = 0; i < elemc; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_i;
			elemv[i] = elem; /* Inherit reference. */
		}
	} else {
		/* Use iterators. */
		iterator = DeeObject_IterSelf(self);
		if unlikely(!iterator)
			goto err;
		elemc = 0;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			ASSERT(elemc <= elema);
			if (elemc >= elema) {
				/* Allocate more memory. */
				size_t new_elema = elema * 2;
				if unlikely(new_elema < 16)
					new_elema = 16;
				new_elemv = (DeeObject **)Dee_TryRealloc(elemv, new_elema *
				                                                sizeof(DeeObject *));
				if unlikely(!new_elemv) {
					new_elema = elemc + 1;
					new_elemv = (DeeObject **)Dee_Realloc(elemv, new_elema *
					                                             sizeof(DeeObject *));
					if unlikely(!new_elemv)
						goto err_iterator_elemc;
				}
				elemv = new_elemv;
				elema = new_elema;
			}
			elemv[elemc] = elem; /* Inherit reference. */
			++elemc;
		}
		*pvector    = elemv;
		*pallocated = elema;
		if unlikely(!elem)
			goto err_iterator;
		Dee_Decref(iterator);
	}
	return elemc;
err_iterator_elemc:
	while (elemc--)
		Dee_Decref(elemv[elemc]);
	*pvector    = elemv;
	*pallocated = elema;
err_iterator:
	Dee_Decref(iterator);
	goto err;
err_i:
	while (i--)
		Dee_Decref(elemv[i]);
err:
	return (size_t)-1;
}


/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*pallocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*pvector)[offset]', rather than `(*pvector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DeeObject ***__restrict pvector,
                                        /*in-out*/ size_t *__restrict pallocated,
                                        /*in*/ size_t offset) {
	DeeObject **new_elemv, **elemv = *pvector;
	DREF DeeObject *iterator, *elem;
	size_t elema = *pallocated;
	size_t elemc, i;
	ASSERT(elema >= offset);
	ASSERT(!elema || elemv);
	elemc = DeeFastSeq_GetSize(self);
	if (elemc != DEE_FASTSEQ_NOTFAST) {
		/* Fast sequence optimizations. */
		if (elemc > (elema - offset)) {
			new_elemv = (DeeObject **)Dee_Realloc(elemv,
			                                      (offset + elemc) *
			                                      sizeof(DeeObject *));
			if unlikely(!new_elemv)
				goto err;
			*pvector = elemv = new_elemv;
			*pallocated      = offset + elemc;
		}
		for (i = 0; i < elemc; ++i) {
			elem = DeeFastSeq_GetItem(self, i);
			if unlikely(!elem)
				goto err_i;
			elemv[offset + i] = elem; /* Inherit reference. */
		}
	} else {
		/* Use iterators. */
		iterator = DeeObject_IterSelf(self);
		if unlikely(!iterator)
			goto err;
		elemc = 0;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			ASSERT(elemc <= (elema - offset));
			if (elemc >= (elema - offset)) {
				/* Allocate more memory. */
				size_t new_elema = elema * 2;
				if unlikely(new_elema < 16)
					new_elema = 16;
				new_elemv = (DeeObject **)Dee_TryRealloc(elemv, new_elema *
				                                                sizeof(DeeObject *));
				if unlikely(!new_elemv) {
					new_elema = offset + elemc + 1;
					new_elemv = (DeeObject **)Dee_Realloc(elemv, new_elema *
					                                             sizeof(DeeObject *));
					if unlikely(!new_elemv)
						goto err_iterator_elemc;
				}
				elemv = new_elemv;
				elema = new_elema;
			}
			elemv[offset + elemc] = elem; /* Inherit reference. */
			++elemc;
		}
		*pvector    = elemv;
		*pallocated = elema;
		if unlikely(!elem)
			goto err_iterator;
		Dee_Decref(iterator);
	}
	return elemc;
err_iterator_elemc:
	while (elemc--)
		Dee_Decref(elemv[offset + elemc]);
	*pvector    = elemv;
	*pallocated = elema;
err_iterator:
	Dee_Decref(iterator);
	goto err;
err_i:
	while (i--)
		Dee_Decref(elemv[offset + i]);
err:
	return (size_t)-1;
}


/* Same as `DeeObject_Unpack()', but handle `DeeError_UnboundItem'
 * by filling in the resp. element from `objv[*]' with `NULL'.
 * This function is implemented to try the following things with `self' (in order):
 *  - Use `DeeFastSeq_GetSize()' + `DeeFastSeq_GetItemUnbound()'
 *    Try next when `DeeFastSeq_GetSize() == DEE_FASTSEQ_NOTFAST'
 *  - Use `DeeObject_Size()' + `DeeObject_GetItemIndex()'
 *    Try next when `DeeObject_Size()' throws `DeeError_NotImplemented', or
 *    `DeeObject_GetItemIndex()' (first call only) throws `DeeError_NotImplemented'
 *    or `DeeError_TypeError'.
 *  - Use `DeeObject_Unpack()' (meaning that all elements written to `objv' will be non-NULL)
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_UnpackWithUnbound)(DeeObject *__restrict self, size_t objc,
                                    /*out*/ DREF DeeObject **__restrict objv) {
	size_t i, size;
	size = DeeFastSeq_GetSize(self);
	if (size != DEE_FASTSEQ_NOTFAST) {
		if (size != objc)
			goto err_badsize;
		for (i = 0; i < objc; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItemUnbound(self, i);
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
