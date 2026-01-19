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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H
#define GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H 1

#include <deemon/api.h>

#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/util/atomic.h>

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct seq_combinations SeqCombinations;
typedef struct seq_combinations_iterator SeqCombinationsIterator;
typedef struct seq_combinations_view SeqCombinationsView;

struct seq_combinations {
	PROXY_OBJECT_HEAD(sc_seq)                                  /* [1..1][const] Underlying sequence */
	DeeMH_seq_operator_trygetitem_index_t sc_trygetitem_index; /* [1..1][const] trygetitem_index operator for "sc_seq" */
	size_t                                sc_seqsize;          /* [valid_if(!= (size_t)-1)][lock(WRITE_ONCE)] Cache for `DeeObject_InvokeMethodHint(seq_operator_size, sc_seq)' */
	size_t                                sc_rparam;           /* [!0][valid_if(!= (size_t)-1)][lock(WRITE_ONCE)] the "r" parameter (or `(size_t)-1' when `sc_seqsize ?: 1' should be used)
	                                                            * NOTE: Guarantied to be valid when accessed by an iterator. */
};

struct seq_combinations_iterator {
	PROXY_OBJECT_HEAD_EX(SeqCombinations, sci_com)  /* [1..1][const] Underlying sequence combinations controller */
	WEAKREF(SeqCombinationsView)          sci_view; /* [0..1] View that is aliasing "sci_idx" */
	COMPILER_FLEXIBLE_ARRAY(size_t,       sci_idx); /* [lock(ATOMIC)][sci_com->sc_rparam] Index matrix */
};

struct seq_combinations_view {
	PROXY_OBJECT_HEAD_EX(SeqCombinationsIterator, scv_iter) /* [1..1] Iterator being viewed */
	SeqCombinations                              *scv_com;  /* [1..1][== scv_iter->sci_com][const] Shallow alias */
	size_t                                       *scv_idx;  /* [1..scv_com->sc_rparam][owned_if(!= scv_iter->sci_idx)]
	                                                         * [lock(ATOMIC && WRITE_ONCE && KEEP_VALID)]
	                                                         * Used table of indices. You can (atomically) read this pointer
	                                                         * and dereference it without needing to hold any locks, because
	                                                         * at the moment that this pointer is exchanged, the old location
	                                                         * will remain valid. */
	WEAKREF_SUPPORT
};

/* Get/Set(once) the "scv_idx" field of "SeqCombinationsView" */
#define SeqCombinationsView_GetIdx(self)    atomic_read(&(self)->scv_idx)
#define SeqCombinationsView_SetIdx(self, p) atomic_cmpxch(&(self)->scv_idx, (self)->scv_iter->sci_idx, p)


INTDEF DeeTypeObject SeqCombinations_Type;
INTDEF DeeTypeObject SeqCombinationsIterator_Type;
INTDEF DeeTypeObject SeqRepeatCombinations_Type;
INTDEF DeeTypeObject SeqRepeatCombinationsIterator_Type;
INTDEF DeeTypeObject SeqPermutations_Type;
INTDEF DeeTypeObject SeqPermutationsIterator_Type;
INTDEF DeeTypeObject SeqCombinationsView_Type;

/* Sequence combinatoric functions:
 * >> DeeSeq_Combinations("ABCD", 2)      -> AB AC AD BC BD CD
 * >> DeeSeq_RepeatCombinations("ABC", 2) -> AA AB AC BB BC CC
 * >> DeeSeq_Permutations2("ABCD", 2)     -> AB AC BA BC CA CB
 * >> DeeSeq_Permutations2("ABC", 2)      -> AB AC BA BC CA CB
 * >> DeeSeq_Permutations("ABC")          -> ABC ACA ACB BAA BAC BCA CAA CAB CBA */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Combinations(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_RepeatCombinations(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Permutations2(/*inherit(always)*/ DREF DeeObject *__restrict self, size_t r);
#define DeeSeq_Permutations(self) DeeSeq_Permutations2(self, (size_t)-1)

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_H */
