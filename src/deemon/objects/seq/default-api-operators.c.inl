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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
	value = DeeObject_TryGetItem(self, wanted_key_value[0]); /* TODO: `DeeMap_OperatorTryGetItem' */
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
	err_seq_unsupportedf(self, "operator [](%r)", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorDelItemWithEmpty(DeeObject *self, DeeObject *index) {
	return err_index_out_of_bounds_ob_x(self, index, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index) {
	return err_seq_unsupportedf(self, "operator del[](%r)", index);
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemWithEmpty(DeeObject *self, DeeObject *index, DeeObject *value) {
	(void)value;
	return err_index_out_of_bounds_ob_x(self, index, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%r, %r)", index, value);
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
	return err_seq_unsupportedf(self, "operator [:]=(%r, %r, %r)", start, end, values);
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
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelItemIndexWithEmpty(DeeObject *self, size_t index) {
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index) {
	return err_seq_unsupportedf(self, "operator del[](%" PRFuSIZ ")", index);
}


INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemIndexWithEmpty(DeeObject *self, size_t index, DeeObject *value) {
	(void)value;
	return err_index_out_of_bounds(self, index, 0);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value) {
	return err_seq_unsupportedf(self, "operator []=(%" PRFdSIZ ", %r)", index, value);
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
	result->dsial_tp_iter = DeeType_SeqCache_RequireOperatorIter(Dee_TYPE(self));
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
	return err_seq_unsupportedf(self, "operator [:]=(%" PRFdSIZ ", %" PRFdSIZ ", %r)", start, end, values);
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
	result->dsial_tp_iter = DeeType_SeqCache_RequireOperatorIter(Dee_TYPE(self));
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
	return err_seq_unsupportedf(self, "operator [:]=(%" PRFdSIZ ", none, %r)", start, values);
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
	return new_DeeSeq_Extend(*p_self, some_object);
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
		return new_DeeSeq_Clear(*p_self);
	if (repeat == 1)
		return 0;
	extend_with_this = DeeSeq_Repeat(*p_self, repeat - 1);
	if unlikely(!extend_with_this)
		goto err;
	result = new_DeeSeq_Extend(*p_self, extend_with_this);
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
(DCALL DeeSeq_OperatorEnumerate)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg) {
	return DeeSeq_OperatorEnumerate(self, proc, arg);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeSeq_OperatorEnumerateIndex)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	return DeeSeq_OperatorEnumerateIndex(self, proc, arg, start, end);
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
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ &DeeSeq_OperatorEnumerate,
	/* .tp_enumerate_index            = */ &DeeSeq_OperatorEnumerateIndex,
	/* .tp_iterkeys                   = */ NULL,
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

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATORS_C_INL */
