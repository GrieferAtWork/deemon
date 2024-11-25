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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL 1

#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#endif /* __INTELLISENSE__ */

#include <deemon/format.h>

#include "../../runtime/operator-require.h"
#include "default-sequences.h"
#include "range.h"

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

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorBoundItemWithEmpty(DeeObject *self, DeeObject *index) {
	(void)self;
	(void)index;
	return -2;
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
		return 1;
	return -2;
err:
	return -1;
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
DeeSeq_DefaultOperatorInplaceAddWithTSCExtend(DREF DeeObject **__restrict p_self,
                                              DeeObject *some_object) {
	return DeeSeq_InvokeExtend(*p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend(DREF DeeObject **__restrict p_self,
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
	/* .tp_nsi                        = */ NULL,
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
(DCALL DeeSeq_OperatorInplaceAdd)(DREF DeeObject **__restrict p_self, DeeObject *some_object) {
	return DeeSeq_OperatorInplaceAdd(p_self, some_object);
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeSeq_OperatorInplaceMul)(DREF DeeObject **__restrict p_self, DeeObject *some_object) {
	return DeeSeq_OperatorInplaceMul(p_self, some_object);
}







/************************************************************************/
/* SET                                                                  */
/************************************************************************/

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
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ &DeeSet_OperatorForeach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ &DeeSet_OperatorSize,
	/* .tp_size_fast                  = */ &DeeSet_OperatorSizeFast,
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
	return -2;
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
	return -2;
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
	/* .tp_nsi                        = */ NULL,
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

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL */
