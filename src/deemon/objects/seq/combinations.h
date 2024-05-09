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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H
#define GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject  *c_seq;        /* [1..1][const] The underlying sequence that is being combined. */
	DREF DeeObject **c_elem;       /* [1..1][0..c_seqlen][const][owned_if(!= DeeTuple_ELEM(c_seq))]
	                                * The vector of elements found in `c_seq'
	                                * NOTE: When `NULL', elements from `c_seq' are accessed through
	                                *       the GETITEM interface, as those items are being used. */
	size_t           c_seqlen;     /* [const][!0] The length of the sequence (in items) */
	size_t           c_comlen;     /* [const][< c_seqlen] The amount of elements per combination. */
	struct type_seq *c_getitem;    /* [0..1][if(!c_elem, [1..1])][const] The seq-interface of the type
	                                * to-be used to access the items of `c_seq' */
	DeeTypeObject   *c_getitem_tp; /* [1..1][valid_if(c_getitem != NULL)] The type used to invoke the getitem operator. */
} Combinations;

typedef struct {
	OBJECT_HEAD
	DREF Combinations  *ci_combi;   /* [1..1][const] The underlying combinations sequence proxy. */
	size_t             *ci_indices; /* [1..ci_combi->c_comlen][lock(ci_lock)][owned]
	                                 * Indices to-be used for the next set of combinations to-be
	                                 * combined to generate the next item. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t ci_lock;    /* Lock for this combinations iterator. */
#endif /* !CONFIG_NO_THREADS */
	bool                ci_first;   /* [lock(ci_lock)] True prior to the first iteration. */
} CombinationsIterator;

#define CombinationsIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->ci_lock)
#define CombinationsIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ci_lock)
#define CombinationsIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ci_lock)
#define CombinationsIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ci_lock)
#define CombinationsIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ci_lock)
#define CombinationsIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ci_lock)
#define CombinationsIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ci_lock)
#define CombinationsIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ci_lock)
#define CombinationsIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->ci_lock)
#define CombinationsIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->ci_lock)
#define CombinationsIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ci_lock)
#define CombinationsIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ci_lock)
#define CombinationsIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ci_lock)
#define CombinationsIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ci_lock)
#define CombinationsIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ci_lock)
#define CombinationsIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->ci_lock)


INTDEF DeeTypeObject SeqCombinations_Type;
INTDEF DeeTypeObject SeqCombinationsIterator_Type;
INTDEF DeeTypeObject SeqRepeatCombinations_Type;
INTDEF DeeTypeObject SeqRepeatCombinationsIterator_Type;
INTDEF DeeTypeObject SeqPermutations_Type;
INTDEF DeeTypeObject SeqPermutationsIterator_Type;


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Combinations(DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatCombinations(DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Permutations(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Permutations2(DeeObject *__restrict self, size_t r);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H */
