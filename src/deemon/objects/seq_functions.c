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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C 1

#include "seq_functions.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
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

#include <hybrid/minmax.h>

#include "../runtime/runtime_error.h"
#include "seq/default-api.h"

DECL_BEGIN

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an Iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows usercode to query
 *       for a general purpose sequence by writing `x is Sequence from deemon' */
INTERN WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self) {
	DREF DeeObject *iter, *elem;
	size_t result = 0;
	/* Count the number of elements, given an iterator. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter) {
		if (DeeError_Catch(&DeeError_NotImplemented))
			err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
		goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		Dee_Decref(elem);
		if unlikely(result == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_get_nth(DeeObject *__restrict self,
                 DeeObject *__restrict sequence,
                 size_t index) {
	DREF DeeObject *elem;
	size_t current_index = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		if (current_index == (size_t)index)
			return elem;
		Dee_Decref(elem);
		++current_index;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	err_index_out_of_bounds(sequence, index, current_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetItem(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type) {
		err_index_out_of_bounds(self, index, 0);
		goto err;
	}
	DeeTypeMRO_Init(&mro, tp_self);
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem)
					return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					size_t mylen = (*nsi->nsi_common.nsi_getsize)(self);
					if unlikely(mylen == (size_t)-1)
						goto err;
					if unlikely(index >= mylen) {
						err_index_out_of_bounds(self, index, mylen);
						goto err;
					}
					result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
					if unlikely(!result)
						err_unbound_index(self, index);
					return result;
				}
				if (nsi->nsi_seqlike.nsi_getrange) {
					result = (*nsi->nsi_seqlike.nsi_getrange)(self, index, index + 1);
					if unlikely(!result)
						goto err;
					goto return_result_first;
				}
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				DREF DeeObject *index_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				result = (*seq->tp_getitem)(self, index_ob);
				Dee_Decref(index_ob);
				return result;
			}
			if (has_noninherited_getrange(tp_self, seq)) {
				DREF DeeObject *real_result;
				DREF DeeObject *index_ob, *index_plus1_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				index_plus1_ob = DeeInt_NewSize(index + 1);
				if unlikely(!index_plus1_ob) {
					Dee_Decref(index_ob);
					goto err;
				}
				result = (*seq->tp_getrange)(self, index_ob, index_plus1_ob);
				Dee_Decref(index_plus1_ob);
				Dee_Decref(index_ob);
				if unlikely(!result)
					goto err;
return_result_first:
				real_result = default_seq_getfirst(result);
				Dee_Decref(result);
				if unlikely(!real_result) {
					/* Translate the empty-sequence error into an index-out-of-bounds */
					if (DeeError_Catch(&DeeError_ValueError)) {
						size_t mylen = DeeObject_Size(self);
						if unlikely(mylen == (size_t)-1)
							goto err;
						err_index_out_of_bounds(self, index, mylen);
					}
				}
				return real_result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *iterator;
				iterator = (*seq->tp_iter)(self);
				if unlikely(!iterator)
					goto err;
				result = iterator_get_nth(iterator, self, index);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator3(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_GETRANGE,
	                            OPERATOR_ITER);
err:
	return NULL;
}

#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */





/************************************************************************/
/* Sequence compare                                                     */
/************************************************************************/
#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS

/* @return: == Dee_COMPARE_ERR: An error occurred.
 * @return: == -1: `self < some_object'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `self > some_object' */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_CompareIV(DeeObject *lhs,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return -1; /* COUNT(LHS) < COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(lhs_item);
	return 1; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareII(DeeObject *lhs,
                 DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return Dee_COMPARE_ERR;
				return 0; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return -1; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			return 1; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareIS(DeeObject *lhs,
                 DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}



/* @return: == -1: An error occurred.
 * @return: == 0:  Sequences differ
 * @return: == 1:  Sequences are equal */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_EqIV(DeeObject *lhs,
            DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) < COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(lhs_item);
	return 0; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqII(DeeObject *lhs, DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return -1;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return -1;
				return 1; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return 0; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return -1;
			return 0; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIS(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_EqIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C */
