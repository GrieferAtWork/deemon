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
#ifndef GUARD_DEEMON_SEQ_H
#define GUARD_DEEMON_SEQ_H 1

#include "api.h"
/**/

#include "alloc.h" /* Dee_MallocUsableSize */
#include "types.h"
#include "util/lock.h"
#ifndef __INTELLISENSE__
#include "object.h"
#endif /* !__INTELLISENSE__ */
/**/

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define return_empty_seq      Dee_return_empty_seq
#define return_empty_iterator Dee_return_empty_iterator
#endif /* DEE_SOURCE */



/* The following things are required from sub-class of `Sequence':
 *     - Must either implement `tp_iter' or `tp_sizeob' + `tp_getitem'
 * Many things are implemented by `Sequence'.
 * For a full list, see http://localhost:8080/modules/deemon/Sequence
 *
 * HINT: Instantiating `Sequence' as-is will return a copy of `Dee_EmptySeq' */
DDATDEF DeeTypeObject DeeSeq_Type; /* `Sequence from deemon' */
#define DeeSeq_Check(ob) DeeObject_Implements(ob, &DeeSeq_Type)

/* Similar to what `DeeSeq_Type' is for all sequence-style types,
 * `DeeIterator_Type' is the the same for all iterator-type types.
 * The following things are implemented by `Iterator':
 *     - Abstraction that automatically defines the following operators:
 *        - tp_bool
 *           - Copy the iterator and return true if invoking tp_iter_next() yields a value.
 *        - tp_repr
 *           - Copy the iterator and print its remaining elements.
 *        - tp_add
 *           - Create a copy of the iterator and invoke `tp_inplace_add' on it.
 *        - tp_inc
 *           - Same as `tp_inplace_add' when the second operand is `1'.
 *        - tp_inplace_add
 *           - Advance the iterator by the integer representation of the second operand.
 *             When the second operand is negative, throw an `Error.ValueError.ArithmeticError.IntegerOverflow'
 *     - Abstraction that automatically defines the following methods:
 *        - `next(): Object'
 *           - Literally the same as invoking `operator __next__()', but has a more userfriendly name.
 *             This function will throw a `Signal.StopIteration' when the iterator has been exhausted.
 * As should be apparent, in order to use the full auto-API provided for
 * iterators, any iterator sub-class should implement a copy-constructor
 * in addition to the `tp_iter_next()' operator, although it should be
 * noted that implementation of the copy-constructor is _NOT_ mandatory,
 * as in some cases it is even impossible to pull off (such as for yield-
 * functions not marked as copyable or using non-copyable local variables).
 *
 * TODO: The "seq" field of iterators is deprecated (for the sake of performance in order to
 *       allow forwarding of iterators objects from underlying sequences, without needing to
 *       wrap them just so that "seq" returns the correct object type). */
DDATDEF DeeTypeObject DeeIterator_Type; /* `Iterator from deemon' */

DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeIterator_Foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeIterator_ForeachPair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);


/* An empty instance of a generic sequence object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSeq_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSeq_Type' should be considered stub/empty,
 *       but obviously something like an empty tuple is also an empty sequence. */
DDATDEF DeeObject      DeeSeq_EmptyInstance;
#define Dee_EmptySeq (&DeeSeq_EmptyInstance)
#ifdef __INTELLISENSE__
#define Dee_return_empty_seq return Dee_EmptySeq
#else /* __INTELLISENSE__ */
#define Dee_return_empty_seq Dee_return_reference_(Dee_EmptySeq)
#endif /* !__INTELLISENSE__ */

DDATDEF DeeObject           DeeIterator_EmptyInstance;
#define Dee_EmptyIterator (&DeeIterator_EmptyInstance)
#ifdef __INTELLISENSE__
#define Dee_return_empty_iterator return Dee_EmptyIterator
#else /* __INTELLISENSE__ */
#define Dee_return_empty_iterator Dee_return_reference_(Dee_EmptyIterator)
#endif /* !__INTELLISENSE__ */



/*******************************************************************************************/
/*                                                                                         */
/* !!! BEGIN: DEPRECATED; PENDING REMOVAL AFTER CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
/*                                                                                         */
/*******************************************************************************************/

#ifdef DEE_SOURCE
#define Dee_type_nii                    type_nii
#define TYPE_ITERX_CLASS_UNIDIRECTIONAL Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL
#define TYPE_ITERX_CLASS_BIDIRECTIONAL  Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL
#define TYPE_ITERX_FNORMAL              Dee_TYPE_ITERX_FNORMAL
#endif /* DEE_SOURCE */

/* ==== NATIVE ITERATOR INTERFACE EXTENSIONS FOR TYPES ==== */
struct Dee_type_nii {
	/* Native Iterator Interface for types. */
#define Dee_TYPE_ITERX_CLASS_UNIDIRECTIONAL 0x0000 /* uni-directional iterator */
#define Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL  0x0001 /* bi-directional iterator */
#define Dee_TYPE_ITERX_FNORMAL              0x0000 /* Normal iterator flags. */
	__UINTPTR_HALF_TYPE__   nii_class; /* Iterator class (One of `TYPE_ITERX_CLASS_*') */
	__UINTPTR_HALF_TYPE__   nii_flags; /* Iterator flags (Set of `TYPE_ITERX_F*') */
	union {
		Dee_funptr_t       _nii_class_functions[10];

		struct {
			/* [0..1] Return the sequence associated with the iterator, or NULL on error.
			 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
			WUNUSED_T NONNULL_T((1))
			DREF DeeObject *(DCALL *nii_getseq)(DeeObject *__restrict self);

			/* [0..1] Get the iterator's position
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return: * :         The iterator's current position, where the a starting position is 0
			 * @return: (size_t)-2: The position is indeterminate (the iterator may have become detached
			 *                      from its sequence, as can happen in linked lists when the iterator's
			 *                      link entry gets removed)
			 * @return: (size_t)-1: Error */
			WUNUSED_T NONNULL_T((1))
			size_t          (DCALL *nii_getindex)(DeeObject *__restrict self);

			/* [0..1] Set the iterator's position
			 * If the given `new_index' is greater than the max allowed index,
			 * the iterator is set to an exhausted state (i.e. points at the
			 * end of the associated sequence)
			 * NOTE: Unbound sequence indices also count for this operation
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_setindex)(DeeObject *__restrict self, size_t new_index);

			/* [0..1] Rewind the iterator to its starting position
			 * @return:  0: Success
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_rewind)(DeeObject *__restrict self);

			/* [0..1] Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has reached its starting position)
			 * @return:  2: Success (the iterator hasn't reached its starting position)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_revert)(DeeObject *__restrict self, size_t step);

			/* [0..1] Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
			 * @return:  0: Success (new relative position couldn't be determined)
			 * @return:  1: Success (the iterator has become exhausted)
			 * @return:  2: Success (the iterator hasn't become exhausted)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_advance)(DeeObject *__restrict self, size_t step);

			/* [0..1] Decrement the iterator by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator was already at its starting location,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_prev)(DeeObject *__restrict self);

			/* [0..1] Increment the iterator, but don't generate a value
			 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
			 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
			 *       only ever be incremented by 1.
			 * @return:  0: Success
			 * @return:  1: The iterator had already been exhausted,
			 *              or the position couldn't be determined
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_next)(DeeObject *__restrict self);

			/* [0..1] Check if the iterator has a predecessor
			 * @return:  0: No, it doesn't have one (index == 0)
			 * @return:  1: Yes, it does have one (index != 0)
			 * @return: -1: Error */
			WUNUSED_T NONNULL_T((1))
			int             (DCALL *nii_hasprev)(DeeObject *__restrict self);

			/* NOTE: `nii_hasnext' should be provided through `tp_bool' (`operator bool()') */

			/* [0..1] Peek the next iterator value, but don't actually advance the iterator.
			 * @return: ITER_DONE: The iterator has already been exhausted. */
			WUNUSED_T NONNULL_T((1))
			DREF DeeObject *(DCALL *nii_peek)(DeeObject *__restrict self);
		}                   nii_common;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define nii_common _dee_aunion.nii_common
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

/*****************************************************************************************/
/*                                                                                       */
/* !!! END: DEPRECATED; PENDING REMOVAL AFTER CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */
/*                                                                                       */
/*****************************************************************************************/



struct Dee_seq_range {
	union {
#undef sr_start
#undef sr_istart
		size_t      sr_start;  /* Start index (clamped) */
		Dee_ssize_t sr_istart; /* Start index (input) */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion_s
#define sr_start  _dee_aunion_s.sr_start
#define sr_istart _dee_aunion_s.sr_istart
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	union {
#undef sr_end
#undef sr_iend
		size_t      sr_end;    /* End index (clamped) */
		Dee_ssize_t sr_iend;   /* End index (input) */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion_e
#define sr_end  _dee_aunion_e.sr_end
#define sr_iend _dee_aunion_e.sr_iend
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};


/* Clamp a range, as given to `operator [:]' & friends to the bounds
 * accepted by the associated sequence. This handles stuff like negative
 * index over-roll and past-the-end truncation. */
#define DeeSeqRange_Clamp(prange, istart, iend, size) \
	((prange)->sr_istart = (istart),                  \
	 (prange)->sr_iend   = (iend),                    \
	 _DeeSeqRange_Clamp(prange, size))
#define _DeeSeqRange_Clamp(prange, size)              \
	(void)(((prange)->sr_start <= (prange)->sr_end && \
	        (prange)->sr_end <= (size)) ||            \
	       (DeeSeqRange_DoClamp(prange, size), 0))
DFUNDEF ATTR_INOUT(1) void DCALL
DeeSeqRange_DoClamp(struct Dee_seq_range *__restrict self,
                    size_t size);

/* Specialized version of `DeeSeqRange_DoClamp()' for `[istart:none]' range expressions. */
#define DeeSeqRange_Clamp_n(istart, size) \
	((size_t)(istart) <= (size) ? (size_t)(istart) : DeeSeqRange_DoClamp_n(istart, size))
DFUNDEF ATTR_CONST WUNUSED size_t DCALL
DeeSeqRange_DoClamp_n(Dee_ssize_t start, size_t size);


/* Create new range sequence objects. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRange_New(DeeObject *begin, DeeObject *end, DeeObject *step);
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeRange_NewInt(Dee_ssize_t begin, Dee_ssize_t end, Dee_ssize_t step);

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Sum(DeeObject *__restrict self); /* DEPRECATED! -- use DeeObject_InvokeMethodHint(seq_sum, self) */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Any(DeeObject *__restrict self);             /* DEPRECATED! -- use DeeObject_InvokeMethodHint(seq_any, self) */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_All(DeeObject *__restrict self);             /* DEPRECATED! -- use DeeObject_InvokeMethodHint(seq_all, self) */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Min(DeeObject *self);            /* DEPRECATED! -- use DeeObject_InvokeMethodHint(seq_min, self) */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Max(DeeObject *self);            /* DEPRECATED! -- use DeeObject_InvokeMethodHint(seq_max, self) */

/* Unpack the given sequence `self' into `dst_length' items then stored within the `dst' vector.
 * This operator follows `DeeObject_Foreach()' semantics, in that unbound items are skipped.
 *
 * Alias for: `DeeObject_InvokeMethodHint(seq_unpack, self, dst_length, dst)'
 *
 * @return: 0 : Success (`dst' now contains exactly `dst_length' references to [1..1] objects)
 * @return: -1: An error was thrown (`dst' may have been modified, but contains no references) */
DFUNDEF WUNUSED ATTR_OUTS(3, 2) NONNULL((1)) int
(DCALL DeeSeq_Unpack)(DeeObject *__restrict self, size_t dst_length,
                      /*out*/ DREF DeeObject **__restrict dst);




/* Possible return values for `DeeType_GetSeqClass()' */
#define Dee_SEQCLASS_UNKNOWN 0 /* Never returned by `DeeType_GetSeqClass()' (used internally) */
#define Dee_SEQCLASS_NONE    1 /* Type does not inherit from "Sequence" */
#define Dee_SEQCLASS_SEQ     2 /* Type inherits from "Sequence" */
#define Dee_SEQCLASS_SET     3 /* Type inherits from "Sequence" and "Set" */
#define Dee_SEQCLASS_MAP     4 /* Type inherits from "Sequence" and "Mapping" */
#define Dee_SEQCLASS_COUNT   5
#define Dee_SEQCLASS_ISSETORMAP(x) ((x) >= Dee_SEQCLASS_SET)

/* Sequence type classification
 * @return: * : One of `Dee_SEQCLASS_*' */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_GetSeqClass(DeeTypeObject const *__restrict self);


/* Construct a new reference-vector object that can be iterated
 * and used to potentially modify the elements of a given `vector'.
 * NOTE: When write-access is granted, `vector' should be `[0..1][0..length]',
 *       whereas when write-access is not possible, then the disposition of
 *       elements of `vector' doesn't matter and can either be `[0..1]' or `[1..1]'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *owner, size_t length,
                 DeeObject **vector,
#ifndef CONFIG_NO_THREADS
                 Dee_atomic_rwlock_t *plock
#else /* !CONFIG_NO_THREADS */
                 bool writable
#endif /* CONFIG_NO_THREADS */
                 );

#ifdef __INTELLISENSE__
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeRefVector_NewReadonly(DeeObject *__restrict owner, size_t length,
                         DeeObject *const *__restrict vector);
#elif defined(CONFIG_NO_THREADS)
#define DeeRefVector_NewReadonly(owner, length, vector) \
	DeeRefVector_New(owner, length, (DeeObject **)(vector), false)
#else /* CONFIG_NO_THREADS */
#define DeeRefVector_NewReadonly(owner, length, vector) \
	DeeRefVector_New(owner, length, (DeeObject **)(vector), NULL)
#endif /* !CONFIG_NO_THREADS */


/* Type of object returned by `DeeSharedVector_NewShared()' */
DDATDEF DeeTypeObject DeeSharedVector_Type;

/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedVector_Decref:            The `vector' arg here is `DREF DeeObject *const *'
 *       - DeeSharedVector_DecrefNoGiftItems: The `vector' arg here is `DeeObject *const *'
 * NOTE: The returned object cannot be used to change out the elements
 *       of the given `vector', meaning that _it_ can still be [const] */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, /*maybe*/ DREF DeeObject *const *vector);

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained objects.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedVector_Decref(DREF DeeObject *__restrict self);

/* Same as `DeeSharedVector_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items.
 * s.a.: the "maybe DREF" annotated on the `vector' argument of
 *       `DeeSharedVector_NewShared()' */
DFUNDEF NONNULL((1)) void DCALL
DeeSharedVector_DecrefNoGiftItems(DREF DeeObject *__restrict self);



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
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeFastSeq_GetSize_deprecated(DeeObject *__restrict self); /* Deprecated */
#define DEE_FASTSEQ_NOTFAST_DEPRECATED ((size_t)-1)

/* Returns the `index'th item of `self'.
 * The caller is responsible that `index < DeeFastSeq_GetSize_deprecated(self)' when
 * `self' is an immutable sequence (anything other than `List' and `_SharedVector').
 * WARNING: This function may _ONLY_ be used if `DeeFastSeq_GetSize_deprecated(self)'
 *          returned something other than `DEE_FASTSEQ_NOTFAST_DEPRECATED'. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFastSeq_GetItem_deprecated(DeeObject *__restrict self, size_t index); /* Deprecated */





/* New fast-sequence interface */
typedef struct {
	DeeObject *fsq_self; /* [1..1] The sequence being enumerated. */
	size_t     fsq_size; /* The # of items in `fsq_self' */
	/* [1..1] Callback to get the index'th element of "fsq_self".
	 * NOTE: The caller must ensure that `index < fsq_size'
	 * HINT: This is the relevant "tp_getitem_index_fast" operator.
	 * @return: * :   A reference to the index'th element of "fsq_self".
	 * @return: NULL: The index'th element of "fsq_self" isn't bound (no error was thrown) */
	DREF DeeObject *(DCALL *fsq_getitem_index_fast)(DeeObject *__restrict self, size_t index);
} DeeFastSeq;

#define DeeFastSeq_GetSize(self)        ((self)->fsq_size)

/* Return the index'th element of "self".
 * @return: * :   A reference to the index'th element of "self".
 * @return: NULL: The index'th element of "self" isn't bound (no error was thrown) */
#define DeeFastSeq_GetItem(self, index) ((*(self)->fsq_getitem_index_fast)((self)->fsq_self, index))

/* Try to load index-based fast sequence controls for "seq".
 * @return: true:  Success. You may use other `DeeFastSeq_*' to access sequence elements.
 * @return: false: Failure. Given `seq' does not implement `tp_getitem_index_fast' */
#ifndef __OPTIMIZE_SIZE__
#define DeeFastSeq_Init(self, seq) (((self)->fsq_size = DeeFastSeq_Init_impl(self, seq)) != (size_t)-1)
#endif /* !__OPTIMIZE_SIZE__ */
DFUNDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeFastSeq_Init)(DeeFastSeq *__restrict self, DeeObject *__restrict seq);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t (DCALL DeeFastSeq_Init_impl)(DeeFastSeq *__restrict self, DeeObject *__restrict seq);



/* Allocate a suitable heap-vector for all the elements of a given sequence,
 * before returning that vector (then populated by [1..1] references), which
 * the caller must inherit upon success.
 * @return: * :   A vector of objects (with a length of `*p_length'),
 *                that must be freed using `Dee_Free', before inheriting
 *                a reference to each of its elements.
 * @return: NULL: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVector(DeeObject *__restrict self,
                    /*[out]*/ size_t *__restrict p_length);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc(DeeObject *__restrict self,
                             /*[out]*/ size_t *__restrict p_length,
                             /*[out]*/ size_t *__restrict p_allocated);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) /*owned(Dee_Free)*/ DREF DeeObject **DCALL
DeeSeq_AsHeapVectorWithAlloc2(DeeObject *__restrict self,
                              /*[out]*/ size_t *__restrict p_length);
#endif /* Dee_MallocUsableSize */

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
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse(DeeObject *__restrict self,
                                  /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                  /*in-out*/ size_t *__restrict p_allocated);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuse2(DeeObject *__restrict self,
                                   /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector);
#endif /* Dee_MallocUsableSize */

/* Same as `DeeSeq_AsHeapVectorWithAllocReuse()', but assume
 * that `IN(*p_allocated) >= offset', while also leaving the first
 * `offset' vector entries untouched and inserting the first enumerated
 * sequence element at `(*p_vector)[offset]', rather than `(*p_vector)[0]'
 * -> This function can be used to efficiently append elements to a
 *    vector which may already contain other objects upon entry. */
DFUNDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset(DeeObject *__restrict self,
                                        /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                        /*in-out*/ size_t *__restrict p_allocated,
                                        /*in*/ size_t offset);
#ifdef Dee_MallocUsableSize
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_AsHeapVectorWithAllocReuseOffset2(DeeObject *__restrict self,
                                         /*in-out, owned(Dee_Free)*/ DREF DeeObject ***__restrict p_vector,
                                         /*in*/ size_t offset);
#endif /* Dee_MallocUsableSize */




/************************************************************************/
/* Type for `Sequence.some'                                             */
/************************************************************************/
typedef struct {
	Dee_OBJECT_HEAD
	DREF DeeObject *se_seq; /* [1..1][const] The sequence being wrapped. */
} DeeSeqSomeObject;
#define DeeSeqSome_GetSeq(self) ((DeeSeqSomeObject const *)Dee_REQUIRES_OBJECT(self))->se_seq

DDATDEF DeeTypeObject DeeSeqSome_Type;
#define DeeSeqSome_Check(self)      DeeObject_InstanceOfExact(self, &DeeSeqSome_Type)
#define DeeSeqSome_CheckExact(self) DeeObject_InstanceOfExact(self, &DeeSeqSome_Type)

/* Construct a some-wrapper for `self' */
#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Some(DeeObject *__restrict self);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeSeq_Some(self) DeeObject_NewPack(&DeeSeqSome_Type, 1, self)
#endif /* !CONFIG_BUILDING_DEEMON */


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeSeq_Unpack(self, dst_length, objv) __builtin_expect(DeeSeq_Unpack(self, dst_length, objv), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_SEQ_H */
