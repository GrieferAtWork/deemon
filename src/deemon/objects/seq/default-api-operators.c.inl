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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL 1

#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#endif /* __INTELLISENSE__ */

#include <deemon/format.h>
#include <deemon/map.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/set.h>
#include <deemon/super.h>
#include <deemon/util/simple-hashset.h>

#include "../../runtime/operator-require.h"
#include "default-maps.h"
#include "default-sequences.h"
#include "default-sets.h"
#include "range.h"
#include "unique-iterator.h"

DECL_BEGIN

/* Possible implementations for sequence cache operators. */
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorBoolWithError(DeeObject *__restrict self) {
	return err_seq_unsupportedf(self, "operator bool");
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorIterWithEmpty(DeeObject *__restrict self) {
	(void)self;
	return_empty_iterator;
}

INTERN /*WUNUSED*/ NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorIterWithError(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "operator iter");
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize(DeeObject *__restrict self) {
	size_t result = DeeSeq_OperatorSize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorSizeObWithEmpty(DeeObject *__restrict self) {
	(void)self;
	return_reference_(DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorSizeObWithError(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "operator size");
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorContainsWithMapTryGetItem(DeeObject *self, DeeObject *some_object) {
	DREF DeeObject *wanted_key_value[2];
	DREF DeeObject *value, *result;
	if (DeeObject_Unpack(some_object, 2, wanted_key_value))
		goto err;
	value = DeeMap_OperatorTryGetItem(self, wanted_key_value[0]);
	Dee_Decref(wanted_key_value[0]);
	if unlikely(!value) {
		Dee_Decref(wanted_key_value[1]);
		goto err;
	}
	if (value == ITER_DONE) {
		Dee_Decref(wanted_key_value[1]);
		return_false;
	}
	result = DeeObject_CmpEq(wanted_key_value[1], value);
	Dee_Decref(wanted_key_value[1]);
	Dee_Decref(value);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorContainsWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorContainsWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator contains(%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorGetItemIndex(self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetItemWithEmpty(DeeObject *self, DeeObject *index) {
	err_index_out_of_bounds_ob_x(self, index, DeeInt_Zero);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetItemWithError(DeeObject *self, DeeObject *index) {
	err_seq_unsupportedf(self, "operator [] (%r)", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorDelItemWithEmpty(DeeObject *self, DeeObject *index) {
	return err_index_out_of_bounds_ob_x(self, index, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator del[] (%r)", index);
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemWithEmpty(DeeObject *self, DeeObject *index, DeeObject *value) {
	(void)value;
	return err_index_out_of_bounds_ob_x(self, index, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []= (%r, %r)", index, value);
}


INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndGetRangeIndexN(DeeObject *self,
                                                                    DeeObject *start,
                                                                    DeeObject *end) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorGetRangeIndexN(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorGetRangeIndex(self, start_index, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeWithEmpty(DeeObject *self, DeeObject *start, DeeObject *end) {
	(void)self;
	(void)start;
	(void)end;
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "operator [:](%r, %r)", start, end);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultOperatorDelRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end) {
	return err_seq_unsupportedf(self, "operator del[:](%r, %r)", start, end);
}


INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeSeq_DefaultOperatorSetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]= (%r, %r, %r)", start, end, values);
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultOperatorForeachWithEmpty(DeeObject *__restrict self,
                                       Dee_foreach_t proc, void *arg) {
	(void)self;
	(void)proc;
	(void)arg;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultOperatorForeachWithError(DeeObject *__restrict self,
                                       Dee_foreach_t proc, void *arg) {
	(void)proc;
	(void)arg;
	DeeSeq_DefaultOperatorIterWithError(self);
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultOperatorEnumerateIndexWithEmpty(DeeObject *__restrict self,
                                              Dee_enumerate_index_t proc,
                                              void *arg, size_t start, size_t end) {
	(void)self;
	(void)proc;
	(void)arg;
	(void)start;
	(void)end;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultOperatorEnumerateIndexWithError(DeeObject *__restrict self,
                                              Dee_enumerate_index_t proc, void *arg,
                                              size_t start, size_t end) {
	(void)proc;
	(void)arg;
	return err_seq_unsupportedf(self, "enumerate(start: %" PRFuSIZ ", end: %" PRFuSIZ ")", start, end);
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorIterKeysWithSeqSize(DeeObject *__restrict self) {
	size_t size;
	DREF IntRangeIterator *result;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(IntRangeIterator);
	if unlikely(!result)
		goto err;
	result->iri_index = 0;
	result->iri_end   = (Dee_ssize_t)size; /* TODO: Need another range iterator type that uses unsigned indices */
	result->iri_step  = 1;
	DeeObject_Init(result, &SeqIntRangeIterator_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorIterKeysWithError(DeeObject *__restrict self) {
	err_seq_unsupportedf(self, "__iterkeys__()");
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorBoundItemIndex(self, index_value);
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorBoundItemWithEmpty(DeeObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorBoundItemWithError(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "__bounditem__(%r)", index);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorHasItemIndex(self, index_value);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorHasItemWithError(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "__hasitem__(%r)", index);
}


INTERN WUNUSED NONNULL((1)) size_t DCALL
DeeSeq_DefaultOperatorSizeWithEmpty(DeeObject *__restrict self) {
	COMPILER_IMPURE();
	(void)self;
	return 0;
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
DeeSeq_DefaultOperatorSizeWithError(DeeObject *__restrict self) {
	return (size_t)err_seq_unsupportedf(self, "operator #");
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetItemIndexWithEmpty(DeeObject *self, size_t index) {
	err_index_out_of_bounds(self, index, 0);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetItemIndexWithError(DeeObject *self, size_t index) {
	err_seq_unsupportedf(self, "operator [] (%" PRFuSIZ ")", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelItemIndexWithEmpty(DeeObject *self, size_t index) {
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index) {
	return err_seq_unsupportedf(self, "operator del[] (%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemIndexWithEmpty(DeeObject *self, size_t index, DeeObject *value) {
	(void)value;
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []= (%" PRFdSIZ ", %r)", index, value);
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize(DeeObject *self, size_t index) {
	size_t seqsize;
	seqsize = DeeSeq_OperatorSize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	if (index < seqsize)
		return Dee_BOUND_YES;
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorBoundItemIndexWithError(DeeObject *self, size_t index) {
	return err_seq_unsupportedf(self, "__bounditem__(%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorHasItemIndexWithSeqSize(DeeObject *self, size_t index) {
	size_t seqsize;
	seqsize = DeeSeq_OperatorSize(self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	if (index < seqsize)
		return 1;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorHasItemIndexWithError(DeeObject *self, size_t index) {
	return err_seq_unsupportedf(self, "__hasitem__(%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeIndexWithIterAndSeqSize(DeeObject *self,
                                                      Dee_ssize_t start,
                                                      Dee_ssize_t end) {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = DeeSeq_OperatorSize(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		return_empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsial_seq     = self;
	result->dsial_start   = range.sr_start;
	result->dsial_limit   = range.sr_end - range.sr_start;
	result->dsial_tp_iter = DeeType_RequireSeqOperatorIter(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	err_seq_unsupportedf(self, "operator [:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return err_seq_unsupportedf(self, "del operator [:](%" PRFdSIZ ", %" PRFdSIZ ")", start, end);
}


INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultOperatorSetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]= (%" PRFdSIZ ", %" PRFdSIZ ", %r)", start, end, values);
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeIndexNWithIterAndSeqSize(DeeObject *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithIterAndLimit *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = DeeSeq_OperatorSize(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsial_seq     = self;
	result->dsial_start   = used_start;
	result->dsial_limit   = (size_t)-1;
	result->dsial_tp_iter = DeeType_RequireSeqOperatorIter(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start) {
	(void)self;
	(void)start;
	return_empty_seq;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start) {
	err_seq_unsupportedf(self, "operator [:](%" PRFdSIZ ", none)", start);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelRangeIndexNWithError(DeeObject *self, Dee_ssize_t start) {
	return err_seq_unsupportedf(self, "operator del[:](%" PRFdSIZ ", none)", start);
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultOperatorSetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	return err_seq_unsupportedf(self, "operator [:]= (%" PRFdSIZ ", none, %r)", start, values);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex(DeeObject *self, DeeObject *index) {
	size_t index_value;
	if unlikely(DeeObject_AsSize(index, &index_value))
		goto err;
	return DeeSeq_OperatorTryGetItemIndex(self, index_value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorTryGetItemWithEmpty(DeeObject *self, DeeObject *index) {
	COMPILER_IMPURE();
	(void)self;
	(void)index;
	return ITER_DONE;
}


INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeSeq_DefaultOperatorHashWithEmpty(DeeObject *__restrict self) {
	(void)self;
	return DEE_HASHOF_EMPTY_SEQUENCE;
}

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeSeq_DefaultOperatorHashWithError(DeeObject *__restrict self) {
	return DeeObject_HashGeneric(self);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_seq_unsupportedf(self, "operator == (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorCompareWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_some_object;
	(void)self;
	tp_some_object = Dee_TYPE(some_object);
	if (DeeType_GetSeqClass(tp_some_object) == Dee_SEQCLASS_SEQ &&
	    DeeType_RequireBool(tp_some_object)) {
		result = (*tp_some_object->tp_cast.tp_bool)(some_object);
	} else {
		result = DeeSeq_DefaultBoolWithForeachDefault(some_object);
	}
	if unlikely(result < 0) {
		result = Dee_COMPARE_ERR;
	} else if (result) {
		result = -1;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorCompareWithError(DeeObject *self, DeeObject *some_object) {
	return err_seq_unsupportedf(self, "operator <=> (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorTryCompareEqWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result;
	DeeTypeObject *tp_some_object;
	(void)self;
	tp_some_object = Dee_TYPE(some_object);
	if (DeeType_GetSeqClass(tp_some_object) == Dee_SEQCLASS_SEQ &&
	    DeeType_RequireBool(tp_some_object)) {
		result = (*tp_some_object->tp_cast.tp_bool)(some_object);
	} else {
		result = DeeSeq_DefaultBoolWithForeachDefault(some_object);
	}
	if unlikely(result < 0) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			return -1;
		result = Dee_COMPARE_ERR;
	} else if (result) {
		result = -1;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_seq_unsupportedf(self, "__equals__(%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorEqWithSeqCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorEqWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator == (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorNeWithSeqCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorNeWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator != (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLoWithSeqCompare(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompare(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLoWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator < (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLeWithSeqCompare(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompare(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLeWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator <= (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGrWithSeqCompare(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompare(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGrWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator > (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGeWithSeqCompare(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_OperatorCompare(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGeWithEmpty(DeeObject *self, DeeObject *some_object) {
	int result = DeeSeq_DefaultOperatorCompareWithEmpty(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object) {
	err_seq_unsupportedf(self, "operator >= (%r)", some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorInplaceAddWithSeqExtend(DREF DeeObject **__restrict p_self,
                                              DeeObject *some_object) {
	return DeeSeq_InvokeExtend(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorInplaceMulWithSeqClearAndSeqExtend(DREF DeeObject **__restrict p_self,
                                                         DeeObject *some_object) {
	int result;
	size_t repeat;
	DREF DeeObject *extend_with_this;
	if (DeeObject_AsSize(some_object, &repeat))
		goto err;
	if (repeat == 0)
		return DeeSeq_InvokeClear(*p_self);
	if (repeat == 1)
		return 0;
	extend_with_this = DeeSeq_Repeat(*p_self, repeat - 1);
	if unlikely(!extend_with_this)
		goto err;
	result = DeeSeq_InvokeExtend(*p_self, extend_with_this);
	Dee_Decref_likely(extend_with_this);
	return result;
err:
	return -1;
}






/************************************************************************/
/* LINKED SEQUENCE OPERATORS                                            */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorContainsAsBool)(DeeObject *self, DeeObject *some_object) {
	DREF DeeObject *result = DeeSeq_OperatorContains(self, some_object);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorBool)(DeeObject *__restrict self) {
	return DeeSeq_OperatorBool(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorIter)(DeeObject *__restrict self) {
	return DeeSeq_OperatorIter(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorSizeOb)(DeeObject *__restrict self) {
	return DeeSeq_OperatorSizeOb(self);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorContains)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorContains(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorGetItem)(DeeObject *self, DeeObject *index) {
	return DeeSeq_OperatorGetItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorDelItem)(DeeObject *self, DeeObject *index) {
	return DeeSeq_OperatorDelItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeSeq_OperatorSetItem)(DeeObject *self, DeeObject *index, DeeObject *value) {
	return DeeSeq_OperatorSetItem(self, index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_OperatorGetRange)(DeeObject *self, DeeObject *start, DeeObject *end) {
	return DeeSeq_OperatorGetRange(self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeSeq_OperatorDelRange)(DeeObject *self, DeeObject *start, DeeObject *end) {
	return DeeSeq_OperatorDelRange(self, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeSeq_OperatorSetRange)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	return DeeSeq_OperatorSetRange(self, start, end, values);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeSeq_OperatorForeach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg) {
	return DeeSeq_OperatorForeach(self, proc, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeSeq_OperatorForeachPair)(DeeObject *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	return DeeSeq_OperatorForeachPair(self, proc, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeSeq_OperatorEnumerate)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	return DeeSeq_OperatorEnumerate(self, proc, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeSeq_OperatorEnumerateIndex)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	return DeeSeq_OperatorEnumerateIndex(self, proc, arg, start, end);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorIterKeys)(DeeObject *__restrict self) {
	return DeeSeq_OperatorIterKeys(self);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorBoundItem)(DeeObject *self, DeeObject *index) {
	return DeeSeq_OperatorBoundItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorHasItem)(DeeObject *self, DeeObject *index) {
	return DeeSeq_OperatorHasItem(self, index);
}

INTERN WUNUSED NONNULL((1)) size_t
(DCALL DeeSeq_OperatorSize)(DeeObject *__restrict self) {
	return DeeSeq_OperatorSize(self);
}

INTERN WUNUSED NONNULL((1)) size_t
(DCALL DeeSeq_OperatorSizeFast)(DeeObject *__restrict self) {
	return DeeSeq_OperatorSizeFast(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorGetItemIndex)(DeeObject *self, size_t index) {
	return DeeSeq_OperatorGetItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorDelItemIndex)(DeeObject *self, size_t index) {
	return DeeSeq_OperatorDelItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL DeeSeq_OperatorSetItemIndex)(DeeObject *self, size_t index, DeeObject *value) {
	return DeeSeq_OperatorSetItemIndex(self, index, value);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorBoundItemIndex)(DeeObject *self, size_t index) {
	return DeeSeq_OperatorBoundItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorHasItemIndex)(DeeObject *self, size_t index) {
	return DeeSeq_OperatorHasItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorGetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return DeeSeq_OperatorGetRangeIndex(self, start, end);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorDelRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return DeeSeq_OperatorDelRangeIndex(self, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) int
(DCALL DeeSeq_OperatorSetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return DeeSeq_OperatorSetRangeIndex(self, start, end, values);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorGetRangeIndexN)(DeeObject *self, Dee_ssize_t start) {
	return DeeSeq_OperatorGetRangeIndexN(self, start);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeSeq_OperatorDelRangeIndexN)(DeeObject *self, Dee_ssize_t start) {
	return DeeSeq_OperatorDelRangeIndexN(self, start);
}

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL DeeSeq_OperatorSetRangeIndexN)(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	return DeeSeq_OperatorSetRangeIndexN(self, start, values);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorTryGetItem)(DeeObject *self, DeeObject *index) {
	return DeeSeq_OperatorTryGetItem(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_OperatorTryGetItemIndex)(DeeObject *self, size_t index) {
	return DeeSeq_OperatorTryGetItemIndex(self, index);
}


INTERN struct type_seq DeeSeq_OperatorSeq = {
	/* .tp_iter                       = */ &DeeSeq_OperatorIter,
	/* .tp_sizeob                     = */ &DeeSeq_OperatorSizeOb,
	/* .tp_contains                   = */ &DeeSeq_OperatorContains,
	/* .tp_getitem                    = */ &DeeSeq_OperatorGetItem,
	/* .tp_delitem                    = */ &DeeSeq_OperatorDelItem,
	/* .tp_setitem                    = */ &DeeSeq_OperatorSetItem,
	/* .tp_getrange                   = */ &DeeSeq_OperatorGetRange,
	/* .tp_delrange                   = */ &DeeSeq_OperatorDelRange,
	/* .tp_setrange                   = */ &DeeSeq_OperatorSetRange,
	/* .tp_foreach                    = */ &DeeSeq_OperatorForeach,
	/* .tp_foreach_pair               = */ &DeeSeq_OperatorForeachPair,
	/* .tp_enumerate                  = */ &DeeSeq_OperatorEnumerate,
	/* .tp_enumerate_index            = */ &DeeSeq_OperatorEnumerateIndex,
	/* .tp_iterkeys                   = */ &DeeSeq_OperatorIterKeys,
	/* .tp_bounditem                  = */ &DeeSeq_OperatorBoundItem,
	/* .tp_hasitem                    = */ &DeeSeq_OperatorHasItem,
	/* .tp_size                       = */ &DeeSeq_OperatorSize,
	/* .tp_size_fast                  = */ &DeeSeq_OperatorSizeFast,
	/* .tp_getitem_index              = */ &DeeSeq_OperatorGetItemIndex,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ &DeeSeq_OperatorDelItemIndex,
	/* .tp_setitem_index              = */ &DeeSeq_OperatorSetItemIndex,
	/* .tp_bounditem_index            = */ &DeeSeq_OperatorBoundItemIndex,
	/* .tp_hasitem_index              = */ &DeeSeq_OperatorHasItemIndex,
	/* .tp_getrange_index             = */ &DeeSeq_OperatorGetRangeIndex,
	/* .tp_delrange_index             = */ &DeeSeq_OperatorDelRangeIndex,
	/* .tp_setrange_index             = */ &DeeSeq_OperatorSetRangeIndex,
	/* .tp_getrange_index_n           = */ &DeeSeq_OperatorGetRangeIndexN,
	/* .tp_delrange_index_n           = */ &DeeSeq_OperatorDelRangeIndexN,
	/* .tp_setrange_index_n           = */ &DeeSeq_OperatorSetRangeIndexN,
	/* .tp_trygetitem                 = */ &DeeSeq_OperatorTryGetItem,
	/* .tp_trygetitem_index           = */ &DeeSeq_OperatorTryGetItemIndex,
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



INTERN WUNUSED NONNULL((1)) Dee_hash_t
(DCALL DeeSeq_OperatorHash)(DeeObject *__restrict self) {
	return DeeSeq_OperatorHash(self);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorCompare)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorCompare(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorTryCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorTryCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorNe)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorNe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorLo)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorLo(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorLe)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorLe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorGr)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorGr(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_OperatorGe)(DeeObject *self, DeeObject *some_object) {
	return DeeSeq_OperatorGe(self, some_object);
}

INTERN struct type_cmp DeeSeq_OperatorCmp = {
	/* .tp_hash          = */ &DeeSeq_OperatorHash,
	/* .tp_compare_eq    = */ &DeeSeq_OperatorCompareEq,
	/* .tp_compare       = */ &DeeSeq_OperatorCompare,
	/* .tp_trycompare_eq = */ &DeeSeq_OperatorTryCompareEq,
	/* .tp_eq            = */ &DeeSeq_OperatorEq,
	/* .tp_ne            = */ &DeeSeq_OperatorNe,
	/* .tp_lo            = */ &DeeSeq_OperatorLo,
	/* .tp_le            = */ &DeeSeq_OperatorLe,
	/* .tp_gr            = */ &DeeSeq_OperatorGr,
	/* .tp_ge            = */ &DeeSeq_OperatorGe,
};





INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorInplaceAdd)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSeq_OperatorInplaceAdd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorInplaceMul)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSeq_OperatorInplaceMul(p_self, some_object);
}







/************************************************************************/
/* SET                                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorIterWithDistinctIter(DeeObject *__restrict self) {
	DREF DeeObject *iter;
	DREF DistinctIterator *result;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctIterator);
	if unlikely(!result)
		goto err_iter;
	result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
	if unlikely(!result->di_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(iter))) {
			err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
			goto err_iter_result;
		}
		result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
		ASSERT(result->di_tp_next);
	}
	result->di_iter = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->di_encountered);
	DeeObject_Init(result, &DistinctIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_iter_result:
	DeeGCObject_FREE(result);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

INTERN /*WUNUSED*/ NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorIterWithError(DeeObject *__restrict self) {
	err_set_unsupportedf(self, "operator iter");
	return NULL;
}

struct default_set_foreach_unique_cb_data {
	Dee_foreach_t dsfucd_cb;  /* [1..1] user-defined callback */
	void         *dsfucd_arg; /* [?..?] Cookie for `dsfucd_cb' */
};

struct default_set_foreach_unique_data {
	struct Dee_simple_hashset                 dsfud_encountered; /* Set of objects already encountered. */
	struct default_set_foreach_unique_cb_data dsfud_cb;          /* Callback data */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_foreach_unique_cb(void *arg, DeeObject *item) {
	int insert_status;
	struct default_set_foreach_unique_data *data;
	data = (struct default_set_foreach_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dsfud_encountered, item);
	if likely(insert_status > 0)
		return (*data->dsfud_cb.dsfucd_cb)(data->dsfud_cb.dsfucd_arg, item);
	return insert_status; /* error, or already-exists */
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSet_DefaultOperatorForeachWithDistinctForeach(DeeObject *__restrict self,
                                                 Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	struct default_set_foreach_unique_data data;
	data.dsfud_cb.dsfucd_cb  = cb;
	data.dsfud_cb.dsfucd_arg = arg;
	Dee_simple_hashset_init(&data.dsfud_encountered);
	result = DeeSeq_OperatorForeach(self, &default_set_foreach_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dsfud_encountered);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSet_DefaultOperatorForeachWithError(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	(void)cb;
	(void)arg;
	DeeSet_DefaultOperatorIterWithError(self);
	return -1;
}


INTDEF WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_size_with_foreach_cb(void *arg, DeeObject *elem);

INTERN WUNUSED NONNULL((1)) size_t DCALL
DeeSet_DefaultOperatorSizeWithSetForeach(DeeObject *__restrict self) {
	return (size_t)DeeSet_OperatorForeach(self, &default_size_with_foreach_cb, NULL);
}

INTERN /*WUNUSED*/ NONNULL((1)) size_t DCALL
DeeSet_DefaultOperatorSizeWithError(DeeObject *__restrict self) {
	return (size_t)err_set_unsupportedf(self, "operator size");
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorSizeObWithSetSize(DeeObject *__restrict self) {
	size_t result = DeeSet_OperatorSize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorSizeObWithError(DeeObject *__restrict self) {
	DeeSet_DefaultOperatorSizeWithError(self);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSet_OperatorIter)(DeeObject *__restrict self) {
	return DeeSet_OperatorIter(self);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t 
(DCALL DeeSet_OperatorForeach)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	return DeeSet_OperatorForeach(self, cb, arg);
}

INTERN WUNUSED NONNULL((1)) size_t 
(DCALL DeeSet_OperatorSize)(DeeObject *__restrict self) {
	return DeeSet_OperatorSize(self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSet_OperatorSizeOb)(DeeObject *__restrict self) {
	return DeeSet_OperatorSizeOb(self);
}


INTERN struct type_seq DeeSet_OperatorSeq = {
	/* .tp_iter                       = */ &DeeSet_OperatorIter,
	/* .tp_sizeob                     = */ &DeeSet_OperatorSizeOb,
	/* .tp_contains                   = */ &DeeSet_OperatorContains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ &DeeSet_OperatorForeach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ &DeeSet_OperatorSize,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
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
	/* .tp_trygetitem_index           = */ NULL,
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


INTERN /*WUNUSED*/ NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_set_unsupportedf(self, "operator == (%r)", some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_set_unsupportedf(self, "__equals__(%r)", some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorEqWithSetCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object) {
	DeeSet_DefaultOperatorCompareEqWithError(self, some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorNeWithSetCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeSet_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator != (%r)", some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator < (%r)", some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorLeWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return_true;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator <= (%r)", some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorGrWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return_false;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator > (%r)", some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator >= (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorInvWithEmpty(DeeObject *self) {
	(void)self;
	return_reference_(Dee_UniversalSet);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorInvWithForeach(DeeObject *self) {
	DREF SetInversion *result;
	result = DeeObject_MALLOC(SetInversion);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SetInversion_Type);
	result->si_set = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorInvWithError(DeeObject *self) {
	err_set_unsupportedf(self, "operator ~");
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorAddWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)self;
	if (DeeType_GetSeqClass(Dee_TYPE(some_object)) == Dee_SEQCLASS_SET)
		return_reference_(some_object);
	return DeeSuper_New(&DeeSet_Type, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorAddWithForeach(DeeObject *self, DeeObject *some_object) {
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a | ~b' --> `~(~a & b)'
		 * -> Keep the inversion on the outside, since it prevents enumeration. */
		SetInversion *xrhs = (SetInversion *)some_object;
		DREF SetInversion *inv_lhs;
		DREF SetIntersection *intersection;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return_reference_(self);
		inv_lhs = SetInversion_New(self);
		if unlikely(!inv_lhs)
			goto err;
		intersection = SetIntersection_New_inherit_b(inv_lhs, xrhs->si_set);
		if unlikely(!intersection)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(intersection);
	}
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(self);
	return (DREF DeeObject *)SetUnion_New(self, some_object);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorAddWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator + (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorSubWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)some_object;
	return_reference_(self);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorSubWithForeach(DeeObject *self, DeeObject *some_object) {
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a - ~b' -> `a & b' */
		SetInversion *xrhs = (SetInversion *)some_object;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return_reference_(Dee_EmptySet); /* `a - ~{}' -> `{}' */
		return (DREF DeeObject *)SetIntersection_New(self, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(self); /* `a - {}' -> `a' */
	return (DREF DeeObject *)SetDifference_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorSubWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator - (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorAndWithForeach(DeeObject *self, DeeObject *some_object) {
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a & ~b' -> `a - b' */
		SetInversion *xrhs = (SetInversion *)some_object;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return_reference_(self); /* `a & ~{}' -> `a' */
		return (DREF DeeObject *)SetDifference_New(self, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(Dee_EmptySet); /* `a & {}' -> `{}' */
	return (DREF DeeObject *)SetIntersection_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorAndWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator & (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorXorWithForeach(DeeObject *self, DeeObject *some_object) {
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a ^ ~b' -> `~(a ^ b)'
		 * -> Keep the inversion on the outside, since it prevents enumeration. */
		SetInversion *xrhs = (SetInversion *)some_object;
		DREF SetSymmetricDifference *symdiff;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return (DREF DeeObject *)SetInversion_New(self); /* `a ^ ~{}' -> `~a' */
		symdiff = SetSymmetricDifference_New(self, xrhs->si_set);
		if unlikely(!symdiff)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(symdiff);
	}
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(self); /* `a ^ {}' -> `a' */
	return (DREF DeeObject *)SetSymmetricDifference_New(self, some_object);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_DefaultOperatorXorWithError(DeeObject *self, DeeObject *some_object) {
	err_set_unsupportedf(self, "operator ^ (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceAddWithSetInsertAll(DREF DeeObject **__restrict p_self,
                                                 DeeObject *some_object) {
	return DeeSet_InvokeInsertAll(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceAddWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_set_unsupportedf(*p_self, "operator += (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceSubWithSetRemoveAll(DREF DeeObject **__restrict p_self,
                                                 DeeObject *some_object) {
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xrhs = (SetInversion *)some_object;
		return DeeSet_OperatorInplaceAnd(p_self, xrhs->si_set);
	}
	return DeeSet_InvokeRemoveAll(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceSubWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_set_unsupportedf(*p_self, "operator -= (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceAndWithForeachAndSetRemoveAll(DREF DeeObject **__restrict p_self,
                                                           DeeObject *some_object) {
	int result;
	DREF SetDifference *keys_to_remove_proxy;
	DREF DeeObject *keys_to_remove;
	if (SetInversion_CheckExact(some_object)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xrhs = (SetInversion *)some_object;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return 0;
		return DeeSet_OperatorInplaceSub(p_self, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(some_object))
		return DeeSeq_InvokeClear(*p_self);

	/* `a &= b' -> `a.removeall((a - b).frozen)' */
	keys_to_remove_proxy = SetDifference_New(*p_self, some_object);
	if unlikely(!keys_to_remove_proxy)
		goto err;
	keys_to_remove = DeeRoSet_FromSequence((DeeObject *)keys_to_remove_proxy);
	Dee_Decref(keys_to_remove_proxy);
	if unlikely(!keys_to_remove)
		goto err;
	result = DeeSet_InvokeRemoveAll(*p_self, keys_to_remove);
	Dee_Decref(keys_to_remove);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceAndWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_set_unsupportedf(*p_self, "operator &= (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceXorWithForeachAndSetInsertAllAndSetRemoveAll(DREF DeeObject **__restrict p_self,
                                                                          DeeObject *some_object) {
	DREF DeeObject *only_in_b_proxy;
	DREF DeeObject *only_in_b;
	if (DeeSet_CheckEmpty(some_object))
		return 0;

	/* >> a ^= b
	 * <=>
	 * >> local only_in_b = (b - a).frozen;
	 * >> a.removeall(b);
	 * >> a.insertall(only_in_b); */
	only_in_b_proxy = DeeSet_OperatorSub(some_object, *p_self);
	if unlikely(!only_in_b_proxy)
		goto err;
	only_in_b = DeeRoSet_FromSequence((DeeObject *)only_in_b_proxy);/* TODO: DeeSeq_InvokeFrozen */
	Dee_Decref(only_in_b_proxy);
	if unlikely(!only_in_b)
		goto err;
	if unlikely(DeeSet_InvokeRemoveAll(*p_self, some_object))
		goto err_only_in_b;
	if unlikely(DeeSet_InvokeInsertAll(*p_self, only_in_b))
		goto err_only_in_b;
	Dee_Decref(only_in_b);
	return 0;
err_only_in_b:
	Dee_Decref(only_in_b);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSet_DefaultOperatorInplaceXorWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_set_unsupportedf(*p_self, "operator ^= (%r)", some_object);
}



INTERN WUNUSED NONNULL((1)) Dee_hash_t
(DCALL DeeSet_OperatorHash)(DeeObject *__restrict self) {
	return DeeSet_OperatorHash(self);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSet_OperatorCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSet_OperatorTryCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorTryCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorEq)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorNe)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorNe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorLo)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorLo(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorLe)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorLe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorGr)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorGr(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorGe)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorGe(self, some_object);
}

INTERN struct type_cmp DeeSet_OperatorCmp = {
	/* .tp_hash          = */ &DeeSet_OperatorHash,
	/* .tp_compare_eq    = */ &DeeSet_OperatorCompareEq,
	/* .tp_compare       = */ NULL, /* Not possible */
	/* .tp_trycompare_eq = */ &DeeSet_OperatorTryCompareEq,
	/* .tp_eq            = */ &DeeSet_OperatorEq,
	/* .tp_ne            = */ &DeeSet_OperatorNe,
	/* .tp_lo            = */ &DeeSet_OperatorLo,
	/* .tp_le            = */ &DeeSet_OperatorLe,
	/* .tp_gr            = */ &DeeSet_OperatorGr,
	/* .tp_ge            = */ &DeeSet_OperatorGe,
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSet_OperatorInv)(DeeObject *self) {
	return DeeSet_OperatorInv(self);
}

/* {"a"} + {"b"}         -> {"a","b"} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorAdd)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorAdd(self, some_object);
}

/* {"a","b"} - {"b"}     -> {"a"} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorSub)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorSub(self, some_object);
}

/* {"a","b"} & {"a"}     -> {"a"} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorAnd)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorAnd(self, some_object);
}

/* {"a","b"} ^ {"a","c"} -> {"b","c"} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSet_OperatorXor)(DeeObject *self, DeeObject *some_object) {
	return DeeSet_OperatorXor(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeSet_OperatorInplaceAdd)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSet_OperatorInplaceAdd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeSet_OperatorInplaceSub)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSet_OperatorInplaceSub(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeSet_OperatorInplaceAnd)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSet_OperatorInplaceAnd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeSet_OperatorInplaceXor)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeSet_OperatorInplaceXor(p_self, some_object);
}

INTERN struct type_math DeeSet_OperatorMath = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ &DeeSet_OperatorInv,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeSet_OperatorAdd,
	/* .tp_sub         = */ &DeeSet_OperatorSub,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ &DeeSet_OperatorAnd,
	/* .tp_or          = */ &DeeSet_OperatorAdd,
	/* .tp_xor         = */ &DeeSet_OperatorXor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &DeeSet_OperatorInplaceAdd,
	/* .tp_inplace_sub = */ &DeeSet_OperatorInplaceSub,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ &DeeSet_OperatorInplaceAnd,
	/* .tp_inplace_or  = */ &DeeSet_OperatorInplaceAdd,
	/* .tp_inplace_xor = */ &DeeSet_OperatorInplaceXor,
	/* .tp_inplace_pow = */ NULL,
};









/************************************************************************/
/* MAP                                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorContainsWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator contains(%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemWithEmpty(DeeObject *self, DeeObject *index) {
	err_unknown_key(self, index);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemWithError(DeeObject *self, DeeObject *index) {
	err_map_unsupportedf(self, "operator [] (%r)", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index) {
	return err_map_unsupportedf(self, "operator del[] (%r)", index);
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeMap_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []= (%r, %r)", index, value);
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeMap_DefaultOperatorEnumerateWithError(DeeObject *__restrict self,
                                         Dee_enumerate_t proc, void *arg) {
	(void)proc;
	(void)arg;
	return err_map_unsupportedf(self, "enumerate()");
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeMap_DefaultOperatorEnumerateIndexWithError(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                              void *arg, size_t start, size_t end) {
	(void)proc;
	(void)arg;
	return err_map_unsupportedf(self, "enumerate(start: %" PRFuSIZ ", end: %" PRFuSIZ ")", start, end);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorBoundItemWithError(DeeObject *self, DeeObject *index) {
	return err_map_unsupportedf(self, "__bounditem__(%r)", index);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorHasItemWithError(DeeObject *self, DeeObject *index) {
	return err_map_unsupportedf(self, "__hasitem__(%r)", index);
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemIndexWithEmpty(DeeObject *self, size_t index) {
	err_unknown_key_int(self, index);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemIndexWithError(DeeObject *self, size_t index) {
	err_map_unsupportedf(self, "operator [] (%" PRFuSIZ ")", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeMap_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index) {
	return err_map_unsupportedf(self, "operator del[] (%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeMap_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value) {
	return err_map_unsupportedf(self, "operator []= (%" PRFuSIZ ", %r)", index, value);
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeMap_DefaultOperatorBoundItemIndexWithError(DeeObject *self, size_t index) {
	return err_map_unsupportedf(self, "__bounditem__(%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeMap_DefaultOperatorHasItemIndexWithError(DeeObject *self, size_t index) {
	return err_map_unsupportedf(self, "__hasitem__(%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorTryGetItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash) {
	COMPILER_IMPURE();
	(void)self;
	(void)key;
	(void)hash;
	return ITER_DONE;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash) {
	(void)hash;
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash) {
	(void)hash;
	err_map_unsupportedf(self, "operator [] (%q)", key);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorDelItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "operator del[] (%q)", key);
}


INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeMap_DefaultOperatorSetItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	return err_map_unsupportedf(self, "operator []= (%q, %r)", key, value);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorBoundItemStringHashWithEmpty(DeeObject *self, char const *key, Dee_hash_t hash) {
	COMPILER_IMPURE();
	(void)self;
	(void)key;
	(void)hash;
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorBoundItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "__bounditem__(%q)", key);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorHasItemStringHashWithError(DeeObject *self, char const *key, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "__hasitem__(%q)", key);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorTryGetItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	COMPILER_IMPURE();
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return ITER_DONE;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)hash;
	err_unknown_key_str_len(self, key, keylen);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGetItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)hash;
	err_map_unsupportedf(self, "operator [] (%$q)", keylen, key);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorDelItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "operator del[] (%$q)", keylen, key);
}


INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeMap_DefaultOperatorSetItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	(void)hash;
	return err_map_unsupportedf(self, "operator []= (%$q, %r)", keylen, key, value);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorBoundItemStringLenHashWithEmpty(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	COMPILER_IMPURE();
	(void)self;
	(void)key;
	(void)keylen;
	(void)hash;
	return Dee_BOUND_MISSING;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorBoundItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "__bounditem__(%$q)", keylen, key);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorHasItemStringLenHashWithError(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	(void)hash;
	return err_map_unsupportedf(self, "__hasitem__(%$q)", keylen, key);
}


INTERN /*WUNUSED*/ NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_map_unsupportedf(self, "operator == (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object) {
	return err_map_unsupportedf(self, "__equals__(%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorEqWithMapCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeMap_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object) {
	DeeMap_DefaultOperatorCompareEqWithError(self, some_object);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorNeWithMapCompareEq(DeeObject *self, DeeObject *some_object) {
	int result = DeeMap_OperatorCompareEq(self, some_object);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator != (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator < (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator <= (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator > (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator >= (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorAddWithEmpty(DeeObject *self, DeeObject *some_object) {
	(void)self;
	if (DeeType_GetSeqClass(Dee_TYPE(some_object)) == Dee_SEQCLASS_MAP)
		return_reference_(some_object);
	return DeeSuper_New(&DeeMapping_Type, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorAddWithForeach(DeeObject *self, DeeObject *some_object) {
	if (DeeMap_CheckEmpty(some_object))
		return_reference_(self);
	return (DREF DeeObject *)MapUnion_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorAddWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator + (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorSubWithForeach(DeeObject *self, DeeObject *some_object) {
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(self);
	return (DREF DeeObject *)MapDifference_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorSubWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator - (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorAndWithForeach(DeeObject *self, DeeObject *some_object) {
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(Dee_EmptyMapping);
	return (DREF DeeObject *)MapIntersection_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorAndWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator & (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorXorWithForeach(DeeObject *self, DeeObject *some_object) {
	if (DeeSet_CheckEmpty(some_object))
		return_reference_(self);
	return (DREF DeeObject *)MapSymmetricDifference_New(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeMap_DefaultOperatorXorWithError(DeeObject *self, DeeObject *some_object) {
	err_map_unsupportedf(self, "operator ^ (%r)", some_object);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceAddWithMapUpdate(DREF DeeObject **__restrict p_self,
                                              DeeObject *some_object) {
	return DeeMap_InvokeUpdate(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceAddWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_map_unsupportedf(*p_self, "operator += (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceSubWithMapRemoveKeys(DREF DeeObject **__restrict p_self,
                                                  DeeObject *some_object) {
	return DeeMap_InvokeRemoveKeys(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceSubWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_map_unsupportedf(*p_self, "operator -= (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceAndWithForeachAndMapRemoveKeys(DREF DeeObject **__restrict p_self,
                                                            DeeObject *some_object) {
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *a_keys_without_b_proxy;
	DREF DeeObject *a_keys_without_b;
	/* `a &= b' -> `a.removekeys(((a.keys as Set) - b).frozen)' */
	a_keys = DeeMap_InvokeKeys(*p_self);
	if unlikely(!a_keys)
		goto err;
	a_keys_without_b_proxy = DeeSet_OperatorSub(a_keys, some_object);
	Dee_Decref(a_keys);
	if unlikely(!a_keys_without_b_proxy)
		goto err;
	a_keys_without_b = DeeRoSet_FromSequence((DeeObject *)a_keys_without_b_proxy); /* TODO: DeeSeq_InvokeFrozen */
	Dee_Decref(a_keys_without_b_proxy);
	if unlikely(!a_keys_without_b)
		goto err;
	result = DeeMap_InvokeRemoveKeys(*p_self, a_keys_without_b);
	Dee_Decref(a_keys_without_b);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceAndWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_map_unsupportedf(*p_self, "operator &= (%r)", some_object);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceXorWithForeachAndMapUpdatAndMapRemoveKeys(DREF DeeObject **__restrict p_self,
                                                                       DeeObject *some_object) {
	/* >> a ^= b
	 * <=>
	 * >> local a_keys = (a as Mapping).keys;
	 * >> local b_keys = (b as Mapping).keys;
	 * >> local a_and_b_keys = (a_keys & b_keys).frozen;
	 * >> a.removekeys(b_keys);
	 * >> a.update((b as Mapping) - a_and_b_keys); */
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *b_keys;
	DREF DeeObject *a_and_b_keys_proxy;
	DREF DeeObject *a_and_b_keys;
	DREF DeeObject *b_without_a_keys;
	a_keys = DeeMap_InvokeKeys(*p_self);
	if unlikely(!a_keys)
		goto err;
	b_keys = DeeMap_InvokeKeys(some_object);
	if unlikely(!b_keys)
		goto err_a_keys;
	a_and_b_keys_proxy = DeeSet_OperatorAnd(a_keys, b_keys);
	Dee_Decref(a_keys);
	if unlikely(!a_and_b_keys_proxy)
		goto err_b_keys;
	a_and_b_keys = DeeRoSet_FromSequence(a_and_b_keys_proxy); /* TODO: DeeSeq_InvokeFrozen */
	Dee_Decref(a_and_b_keys_proxy);
	if unlikely(!a_and_b_keys)
		goto err_b_keys;
	result = DeeMap_InvokeRemoveKeys(*p_self, b_keys);
	Dee_Decref(b_keys);
	if unlikely(result)
		goto err_a_and_b_keys;
	b_without_a_keys = DeeMap_OperatorSub(some_object, a_and_b_keys);
	Dee_Decref(a_and_b_keys);
	if unlikely(!b_without_a_keys)
		goto err;
	result = DeeMap_InvokeUpdate(*p_self, b_without_a_keys);
	Dee_Decref(b_without_a_keys);
	return result;
err_a_and_b_keys:
	Dee_Decref(a_and_b_keys);
err:
	return -1;
err_a_keys:
	Dee_Decref(a_keys);
	goto err;
err_b_keys:
	Dee_Decref(b_keys);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeMap_DefaultOperatorInplaceXorWithError(DREF DeeObject **__restrict p_self,
                                          DeeObject *some_object) {
	return err_map_unsupportedf(*p_self, "operator ^= (%r)", some_object);
}






INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorContainsAsBool)(DeeObject *self, DeeObject *some_object) {
	DREF DeeObject *result = DeeMap_OperatorContains(self, some_object);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorContains)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorContains(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorGetItem)(DeeObject *self, DeeObject *index) {
	return DeeMap_OperatorGetItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorDelItem)(DeeObject *self, DeeObject *index) {
	return DeeMap_OperatorDelItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeMap_OperatorSetItem)(DeeObject *self, DeeObject *index, DeeObject *value) {
	return DeeMap_OperatorSetItem(self, index, value);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeMap_OperatorEnumerate)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	return DeeMap_OperatorEnumerate(self, proc, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeMap_OperatorEnumerateIndex)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	return DeeMap_OperatorEnumerateIndex(self, proc, arg, start, end);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorBoundItem)(DeeObject *self, DeeObject *index) {
	return DeeMap_OperatorBoundItem(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorHasItem)(DeeObject *self, DeeObject *index) {
	return DeeMap_OperatorHasItem(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeMap_OperatorGetItemIndex)(DeeObject *self, size_t index) {
	return DeeMap_OperatorGetItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeMap_OperatorDelItemIndex)(DeeObject *self, size_t index) {
	return DeeMap_OperatorDelItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL DeeMap_OperatorSetItemIndex)(DeeObject *self, size_t index, DeeObject *value) {
	return DeeMap_OperatorSetItemIndex(self, index, value);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeMap_OperatorBoundItemIndex)(DeeObject *self, size_t index) {
	return DeeMap_OperatorBoundItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL DeeMap_OperatorHasItemIndex)(DeeObject *self, size_t index) {
	return DeeMap_OperatorHasItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorTryGetItem)(DeeObject *self, DeeObject *index) {
	return DeeMap_OperatorTryGetItem(self, index);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeMap_OperatorTryGetItemIndex)(DeeObject *self, size_t index) {
	return DeeMap_OperatorTryGetItemIndex(self, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorTryGetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	return DeeMap_OperatorTryGetItemStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorGetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	return DeeMap_OperatorGetItemStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorDelItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	return DeeMap_OperatorDelItemStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeMap_OperatorSetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	return DeeMap_OperatorSetItemStringHash(self, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorBoundItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	return DeeMap_OperatorBoundItemStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorHasItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	return DeeMap_OperatorHasItemStringHash(self, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorTryGetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeMap_OperatorTryGetItemStringLenHash(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorGetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeMap_OperatorGetItemStringLenHash(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorDelItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeMap_OperatorDelItemStringLenHash(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeMap_OperatorSetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return DeeMap_OperatorSetItemStringLenHash(self, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorBoundItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeMap_OperatorBoundItemStringLenHash(self, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorHasItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeMap_OperatorHasItemStringLenHash(self, key, keylen, hash);
}

INTERN struct type_seq DeeMap_OperatorSeq = {
	/* .tp_iter                       = */ &DeeMap_OperatorIter,
	/* .tp_sizeob                     = */ &DeeMap_OperatorSizeOb,
	/* .tp_contains                   = */ &DeeMap_OperatorContains,
	/* .tp_getitem                    = */ &DeeMap_OperatorGetItem,
	/* .tp_delitem                    = */ &DeeMap_OperatorDelItem,
	/* .tp_setitem                    = */ &DeeMap_OperatorSetItem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ &DeeMap_OperatorForeach,
	/* .tp_foreach_pair               = */ &DeeMap_OperatorForeachPair,
	/* .tp_enumerate                  = */ &DeeMap_OperatorEnumerate,
	/* .tp_enumerate_index            = */ &DeeMap_OperatorEnumerateIndex,
	/* .tp_iterkeys                   = */ &DeeMap_OperatorIterKeys,
	/* .tp_bounditem                  = */ &DeeMap_OperatorBoundItem,
	/* .tp_hasitem                    = */ &DeeMap_OperatorHasItem,
	/* .tp_size                       = */ &DeeMap_OperatorSize,
	/* .tp_size_fast                  = */ &DeeMap_OperatorSizeFast,
	/* .tp_getitem_index              = */ &DeeMap_OperatorGetItemIndex,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ &DeeMap_OperatorDelItemIndex,
	/* .tp_setitem_index              = */ &DeeMap_OperatorSetItemIndex,
	/* .tp_bounditem_index            = */ &DeeMap_OperatorBoundItemIndex,
	/* .tp_hasitem_index              = */ &DeeMap_OperatorHasItemIndex,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ &DeeMap_OperatorTryGetItem,
	/* .tp_trygetitem_index           = */ &DeeMap_OperatorTryGetItemIndex,
	/* .tp_trygetitem_string_hash     = */ &DeeMap_OperatorTryGetItemStringHash,
	/* .tp_getitem_string_hash        = */ &DeeMap_OperatorGetItemStringHash,
	/* .tp_delitem_string_hash        = */ &DeeMap_OperatorDelItemStringHash,
	/* .tp_setitem_string_hash        = */ &DeeMap_OperatorSetItemStringHash,
	/* .tp_bounditem_string_hash      = */ &DeeMap_OperatorBoundItemStringHash,
	/* .tp_hasitem_string_hash        = */ &DeeMap_OperatorHasItemStringHash,
	/* .tp_trygetitem_string_len_hash = */ &DeeMap_OperatorTryGetItemStringLenHash,
	/* .tp_getitem_string_len_hash    = */ &DeeMap_OperatorGetItemStringLenHash,
	/* .tp_delitem_string_len_hash    = */ &DeeMap_OperatorDelItemStringLenHash,
	/* .tp_setitem_string_len_hash    = */ &DeeMap_OperatorSetItemStringLenHash,
	/* .tp_bounditem_string_len_hash  = */ &DeeMap_OperatorBoundItemStringLenHash,
	/* .tp_hasitem_string_len_hash    = */ &DeeMap_OperatorHasItemStringLenHash,
};




INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeMap_OperatorTryCompareEq)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorTryCompareEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorEq)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorEq(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorNe)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorNe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorLo)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorLo(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorLe)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorLe(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorGr)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorGr(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorGe)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorGe(self, some_object);
}

INTERN struct type_cmp DeeMap_OperatorCmp = {
	/* .tp_hash          = */ &DeeMap_OperatorHash,
	/* .tp_compare_eq    = */ &DeeMap_OperatorCompareEq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &DeeMap_OperatorTryCompareEq,
	/* .tp_eq            = */ &DeeMap_OperatorEq,
	/* .tp_ne            = */ &DeeMap_OperatorNe,
	/* .tp_lo            = */ &DeeMap_OperatorLo,
	/* .tp_le            = */ &DeeMap_OperatorLe,
	/* .tp_gr            = */ &DeeMap_OperatorGr,
	/* .tp_ge            = */ &DeeMap_OperatorGe,
};

/* {"a":1} + {"b":2}       -> {"a":1,"b":2} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorAdd)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorAdd(self, some_object);
}

/* {"a":1,"b":2} - {"a"}   -> {"b":2} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorSub)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorSub(self, some_object);
}

/* {"a":1,"b":2} & {"a"}   -> {"a":1} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorAnd)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorAnd(self, some_object);
}

/* {"a":1,"b":2} ^ {"a":3} -> {"b":2} */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeMap_OperatorXor)(DeeObject *self, DeeObject *some_object) {
	return DeeMap_OperatorXor(self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeMap_OperatorInplaceAdd)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeMap_OperatorInplaceAdd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeMap_OperatorInplaceSub)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeMap_OperatorInplaceSub(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeMap_OperatorInplaceAnd)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeMap_OperatorInplaceAnd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int 
(DCALL DeeMap_OperatorInplaceXor)(DREF DeeObject **__restrict p_self,
                                  DeeObject *some_object) {
	return DeeMap_OperatorInplaceXor(p_self, some_object);
}

INTERN struct type_math DeeMap_OperatorMath = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ NULL,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeMap_OperatorAdd,
	/* .tp_sub         = */ &DeeMap_OperatorSub,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ &DeeMap_OperatorAnd,
	/* .tp_or          = */ &DeeMap_OperatorAdd,
	/* .tp_xor         = */ &DeeMap_OperatorXor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &DeeMap_OperatorInplaceAdd,
	/* .tp_inplace_sub = */ &DeeMap_OperatorInplaceSub,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ &DeeMap_OperatorInplaceAnd,
	/* .tp_inplace_or  = */ &DeeMap_OperatorInplaceAdd,
	/* .tp_inplace_xor = */ &DeeMap_OperatorInplaceXor,
	/* .tp_inplace_pow = */ NULL,
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL */
