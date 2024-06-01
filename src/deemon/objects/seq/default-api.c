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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C 1

#include "default-api.h"

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

/**/
#include "default-enumerate.h"

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
#include "../seq_functions.h"
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

DECL_BEGIN

#define DeeType_RequireBool(tp_self)           (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))
#define DeeType_RequireSize(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size && (tp_self)->tp_seq->tp_sizeob) || DeeType_InheritSize(tp_self))
#define DeeType_RequireIter(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeach(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerateIndex(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireGetItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItem(tp_self)      (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem && (tp_self)->tp_seq->tp_bounditem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_size(struct type_seq const *__restrict self) {
	return (self->tp_size != NULL) &&
	       !DeeType_IsDefaultSize(self->tp_size);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_sizeob(struct type_seq const *__restrict self) {
	return (self->tp_sizeob != NULL) &&
	       !DeeType_IsDefaultSizeOb(self->tp_sizeob);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_foreach(struct type_seq const *__restrict self) {
	return (self->tp_foreach != NULL) &&
	       !DeeType_IsDefaultForeach(self->tp_foreach);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_index(struct type_seq const *__restrict self) {
	return (self->tp_getitem_index != NULL) &&
	       !DeeType_IsDefaultGetItemIndex(self->tp_getitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_index(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_index != NULL) &&
	       !DeeType_IsDefaultBoundItemIndex(self->tp_bounditem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_delitem_index(struct type_seq const *__restrict self) {
	return (self->tp_delitem_index != NULL) &&
	       !DeeType_IsDefaultDelItemIndex(self->tp_delitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_setitem_index(struct type_seq const *__restrict self) {
	return (self->tp_setitem_index != NULL) &&
	       !DeeType_IsDefaultSetItemIndex(self->tp_setitem_index);
}

INTERN WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self) {
	struct Dee_type_seq_cache *sc;
	ASSERT(self->tp_seq);
	sc = self->tp_seq->_tp_seqcache;
	if (sc)
		return sc;
	sc = (struct Dee_type_seq_cache *)Dee_TryCalloc(sizeof(struct Dee_type_seq_cache));
	if likely(sc) {
		if unlikely(!atomic_cmpxch(&self->tp_seq->_tp_seqcache, NULL, sc)) {
			Dee_Free(sc);
			return self->tp_seq->_tp_seqcache;
		}

		/* Don't treat "sc" as a memory leak if it isn't freed. */
		sc = (struct Dee_type_seq_cache *)Dee_UntrackAlloc(sc);
	}
	return sc;
}


#define Dee_tsc_uslot_fini_function(self) Dee_Decref((self)->d_function)

/* Destroy a lazily allocated sequence operator cache table. */
INTERN NONNULL((1)) void DCALL
Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self) {
	/* Drop function references where they are present. */
	if (self->tsc_any == &DeeSeq_DefaultAnyWithCallAnyDataFunction ||
	    self->tsc_any_with_key == &DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSeq ||
	    self->tsc_any_with_key == &DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSetOrMap ||
	    self->tsc_any_with_range == &DeeSeq_DefaultAnyWithRangeWithCallAnyDataFunction ||
	    self->tsc_any_with_range_and_key == &DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_any_data);
	if (self->tsc_all == &DeeSeq_DefaultAllWithCallAllDataFunction ||
	    self->tsc_all_with_key == &DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSeq ||
	    self->tsc_all_with_key == &DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSetOrMap ||
	    self->tsc_all_with_range == &DeeSeq_DefaultAllWithRangeWithCallAllDataFunction ||
	    self->tsc_all_with_range_and_key == &DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_all_data);
	if (self->tsc_parity == &DeeSeq_DefaultParityWithCallParityDataFunction ||
	    self->tsc_parity_with_key == &DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSeq ||
	    self->tsc_parity_with_key == &DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSetOrMap ||
	    self->tsc_parity_with_range == &DeeSeq_DefaultParityWithRangeWithCallParityDataFunction ||
	    self->tsc_parity_with_range_and_key == &DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_parity_data);
	if (self->tsc_reduce == &DeeSeq_DefaultReduceWithCallReduceDataFunction ||
	    self->tsc_reduce_with_init == &DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSeq ||
	    self->tsc_reduce_with_init == &DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSetOrMap ||
	    self->tsc_reduce_with_range == &DeeSeq_DefaultReduceWithRangeWithCallReduceDataFunction ||
	    self->tsc_reduce_with_range_and_init == &DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_reduce_data);
	if (self->tsc_min == &DeeSeq_DefaultMinWithCallMinDataFunction ||
	    self->tsc_min_with_key == &DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSeq ||
	    self->tsc_min_with_key == &DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSetOrMap ||
	    self->tsc_min_with_range == &DeeSeq_DefaultMinWithRangeWithCallMinDataFunction ||
	    self->tsc_min_with_range_and_key == &DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_min_data);
	if (self->tsc_max == &DeeSeq_DefaultMaxWithCallMaxDataFunction ||
	    self->tsc_max_with_key == &DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSeq ||
	    self->tsc_max_with_key == &DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSetOrMap ||
	    self->tsc_max_with_range == &DeeSeq_DefaultMaxWithRangeWithCallMaxDataFunction ||
	    self->tsc_max_with_range_and_key == &DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_max_data);
	if (self->tsc_sum == &DeeSeq_DefaultSumWithCallSumDataFunction ||
	    self->tsc_sum_with_range == &DeeSeq_DefaultSumWithRangeWithCallSumDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_sum_data);
	if (self->tsc_count == &DeeSeq_DefaultCountWithCallCountDataFunction ||
	    self->tsc_count_with_key == &DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSeq ||
	    self->tsc_count_with_key == &DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSetOrMap ||
	    self->tsc_count_with_range == &DeeSeq_DefaultCountWithRangeWithCallCountDataFunction ||
	    self->tsc_count_with_range_and_key == &DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_count_data);
	if (self->tsc_contains == &DeeSeq_DefaultContainsWithCallContainsDataFunction ||
	    self->tsc_contains_with_key == &DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSeq ||
	    self->tsc_contains_with_key == &DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSetOrMap ||
	    self->tsc_contains_with_range == &DeeSeq_DefaultContainsWithRangeWithCallContainsDataFunction ||
	    self->tsc_contains_with_range_and_key == &DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_contains_data);
	if (self->tsc_locate == &DeeSeq_DefaultLocateWithCallLocateDataFunction ||
	    self->tsc_locate_with_key == &DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSeq ||
	    self->tsc_locate_with_key == &DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSetOrMap ||
	    self->tsc_locate_with_range == &DeeSeq_DefaultLocateWithRangeWithCallLocateDataFunction ||
	    self->tsc_locate_with_range_and_key == &DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_locate_data);
	if (self->tsc_rlocate_with_range == &DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataFunction ||
	    self->tsc_rlocate_with_range_and_key == &DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_rlocate_data);
	if (self->tsc_startswith == &DeeSeq_DefaultStartsWithWithCallStartsWithDataFunction ||
	    self->tsc_startswith_with_key == &DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSeq ||
	    self->tsc_startswith_with_key == &DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSetOrMap ||
	    self->tsc_startswith_with_range == &DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataFunction ||
	    self->tsc_startswith_with_range_and_key == &DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_startswith_data);
	if (self->tsc_endswith == &DeeSeq_DefaultEndsWithWithCallEndsWithDataFunction ||
	    self->tsc_endswith_with_key == &DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSeq ||
	    self->tsc_endswith_with_key == &DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSetOrMap ||
	    self->tsc_endswith_with_range == &DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataFunction ||
	    self->tsc_endswith_with_range_and_key == &DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_endswith_data);
	if (self->tsc_find == &DeeSeq_DefaultFindWithCallFindDataFunction ||
	    self->tsc_find_with_key == &DeeSeq_DefaultFindWithKeyWithCallFindDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_find_data);
	if (self->tsc_rfind == &DeeSeq_DefaultRFindWithCallRFindDataFunction ||
	    self->tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithCallRFindDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_rfind_data);
	if (self->tsc_erase == &DeeSeq_DefaultEraseWithCallEraseDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_erase_data);
	if (self->tsc_insert == &DeeSeq_DefaultInsertWithCallInsertDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_insert_data);
	if (self->tsc_insertall == &DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_insertall_data);
	if (self->tsc_pushfront == &DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_pushfront_data);
	if (self->tsc_append == &DeeSeq_DefaultAppendWithCallAppendDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_append_data);
	if (self->tsc_extend == &DeeSeq_DefaultExtendWithCallExtendDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_extend_data);
	if (self->tsc_xchitem_index == &DeeSeq_DefaultXchItemIndexWithCallXchItemDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_xchitem_data);
	if (self->tsc_clear == &DeeSeq_DefaultClearWithCallClearDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_clear_data);
	if (self->tsc_pop == &DeeSeq_DefaultPopWithCallPopDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_pop_data);
	if (self->tsc_remove == &DeeSeq_DefaultRemoveWithCallRemoveDataFunction ||
	    self->tsc_remove_with_key == &DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_remove_data);
	if (self->tsc_rremove == &DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction ||
	    self->tsc_rremove_with_key == &DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_rremove_data);
	if (self->tsc_removeall == &DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction ||
	    self->tsc_removeall_with_key == &DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_removeall_data);
	if (self->tsc_removeif == &DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_removeif_data);
	if (self->tsc_resize == &DeeSeq_DefaultResizeWithCallResizeDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_resize_data);
	if (self->tsc_fill == &DeeSeq_DefaultFillWithCallFillDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_fill_data);
	if (self->tsc_reverse == &DeeSeq_DefaultReverseWithCallReverseDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_reverse_data);
	if (self->tsc_reversed == &DeeSeq_DefaultReversedWithCallReversedDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_reversed_data);
	if (self->tsc_sort == &DeeSeq_DefaultSortWithCallSortDataFunction ||
	    self->tsc_sort_with_key == &DeeSeq_DefaultSortWithKeyWithCallSortDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_sort_data);
	if (self->tsc_sorted == &DeeSeq_DefaultSortedWithCallSortedDataFunction ||
	    self->tsc_sorted_with_key == &DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_sorted_data);
	Dee_Free(self);
}


INTERN WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL
DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_foreach_reverse)
			return sc->tsc_foreach_reverse;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		Dee_tsc_foreach_reverse_t result = NULL;
		if (DeeType_RequireSize(self) && DeeType_RequireGetItem(self)) {
			bool has_size = !DeeType_IsDefaultSize(self->tp_seq->tp_size);
			if (has_size && self->tp_seq->tp_getitem_index_fast) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast;
			} else if (has_size && !DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex;
			} else if (has_size && !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex;
			} else if (!DeeType_IsDefaultSizeOb(self->tp_seq->tp_sizeob) &&
			           !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeObAndGetItem;
			}
		}
		if (result) {
			struct Dee_type_seq_cache *sc;
			sc = DeeType_TryRequireSeqCache(self);
			if likely(sc)
				atomic_write(&sc->tsc_foreach_reverse, result);
		}
		return result;
	}
	return NULL; /* Not possible... */
}

INTERN WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_reverse_t DCALL
DeeType_SeqCache_TryRequireEnumerateIndexReverse(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_enumerate_index_reverse)
			return sc->tsc_enumerate_index_reverse;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		Dee_tsc_enumerate_index_reverse_t result = NULL;
		if (DeeType_RequireSize(self) && DeeType_RequireGetItem(self)) {
			bool has_size = !DeeType_IsDefaultSize(self->tp_seq->tp_size);
			if (has_size && self->tp_seq->tp_getitem_index_fast) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast;
			} else if (has_size && !DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex;
			} else if (has_size && !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex;
			} else if (!DeeType_IsDefaultSizeOb(self->tp_seq->tp_sizeob) &&
			           !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem;
			}
		}
		if (result) {
			struct Dee_type_seq_cache *sc;
			sc = DeeType_TryRequireSeqCache(self);
			if likely(sc)
				atomic_write(&sc->tsc_enumerate_index_reverse, result);
		}
		return result;
	}
	return NULL; /* Not possible... */
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_SeqCache_HasPrivateEnumerateIndexReverse(DeeTypeObject *orig_type, DeeTypeObject *self) {
	return DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
	       DeeType_HasPrivateOperator(self, OPERATOR_GETITEM);
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexWithError(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                      void *arg, size_t start, size_t end) {
	(void)self;
	(void)proc;
	(void)arg;
	(void)start;
	(void)end;
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_t DCALL
DeeType_SeqCache_RequireEnumerateIndex(DeeTypeObject *__restrict self) {
	Dee_tsc_enumerate_index_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_enumerate_index)
			return sc->tsc_enumerate_index;
	}
	if (DeeType_RequireEnumerateIndex(self)) {
		result = self->tp_seq->tp_enumerate_index;
	} else if (DeeType_RequireForeach(self) && !DeeType_IsDefaultForeach(self->tp_seq->tp_foreach)) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach;
	} else if (DeeType_RequireIter(self) && !DeeType_IsDefaultIter(self->tp_seq->tp_iter)) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndIter;
	} else if (self->tp_seq && self->tp_seq->tp_foreach) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault;
	} else {
		result = &DeeSeq_DefaultEnumerateIndexWithError;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_enumerate_index, result);
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_nonempty_t DCALL
DeeType_SeqCache_RequireNonEmpty(DeeTypeObject *__restrict self) {
	Dee_tsc_nonempty_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_nonempty)
			return sc->tsc_nonempty;
	}
	result = &DeeSeq_DefaultNonEmptyWithError;
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		if (DeeType_RequireBool(self) && self->tp_cast.tp_bool != &default_seq_bool) {
			result = self->tp_cast.tp_bool;
		} else if (Dee_type_seq_has_custom_tp_size(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithSize;
		} else if (Dee_type_seq_has_custom_tp_sizeob(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithSizeOb;
		} else if (Dee_type_seq_has_custom_tp_foreach(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithForeach;
		} else if (self->tp_cmp && self->tp_cmp->tp_compare_eq &&
		           !DeeType_IsDefaultCompareEq(self->tp_cmp->tp_compare_eq) &&
		           !DeeType_IsDefaultCompare(self->tp_cmp->tp_compare_eq)) {
			result = &DeeSeq_DefaultBoolWithCompareEq;
		} else if (self->tp_cmp && self->tp_cmp->tp_eq && !DeeType_IsDefaultEq(self->tp_cmp->tp_eq)) {
			result = &DeeSeq_DefaultBoolWithEq;
		} else if (self->tp_cmp && self->tp_cmp->tp_ne && !DeeType_IsDefaultNe(self->tp_cmp->tp_ne)) {
			result = &DeeSeq_DefaultBoolWithNe;
		} else if (self->tp_seq->tp_foreach || DeeType_InheritIter(self)) {
			result = &DeeSeq_DefaultBoolWithForeachDefault;
		}
	} else if (DeeType_RequireForeach(self)) {
		result = &DeeSeq_DefaultBoolWithForeach;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_nonempty, result);
	return result;
}


/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithError(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithError(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_t DCALL
DeeType_SeqCache_RequireMakeEnumeration_uncached(DeeTypeObject *__restrict self) {
	if (DeeType_HasOperator(self, OPERATOR_ITER)) {
		unsigned int seqclass;
		if (self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem ||
		           self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex ||
		           self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndForeach ||
		           self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndIter) {
			if (self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndGetItem ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			if (self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItem ||
			    self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItem ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeObAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItemIndex ||
			    self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (self->tp_seq->tp_enumerate == &DeeMap_DefaultEnumerateWithIter) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		}
		seqclass = DeeType_GetSeqClass(self);
		if (self->tp_seq->tp_iterkeys && !DeeType_IsDefaultIterKeys(self->tp_seq->tp_iterkeys)) {
			if (self->tp_seq->tp_trygetitem && !DeeType_IsDefaultTryGetItem(self->tp_seq->tp_trygetitem))
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
			if (self->tp_seq->tp_getitem && !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			if (self->tp_seq->tp_trygetitem)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
			if (self->tp_seq->tp_getitem)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
		}
		if (seqclass == Dee_SEQCLASS_SEQ) {
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (self->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
				if (DeeType_HasOperator(self, OPERATOR_GETITEM)) {
					if (!DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
					if (!DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
					return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
				}
			} else if (DeeType_HasOperator(self, OPERATOR_GETITEM)) {
				return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			}
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
		if (self->tp_seq->tp_enumerate != NULL &&
		    self->tp_seq->tp_enumerate != self->tp_seq->tp_foreach_pair &&
		    !DeeType_IsDefaultEnumerate(self->tp_seq->tp_enumerate)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		} else if (self->tp_seq->tp_enumerate_index != NULL &&
		           !DeeType_IsDefaultEnumerateIndex(self->tp_seq->tp_enumerate_index)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		}
		if (seqclass == Dee_SEQCLASS_MAP) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		} else if (seqclass == Dee_SEQCLASS_NONE) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
	}
	return &DeeSeq_DefaultMakeEnumerationWithError;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_t DCALL
DeeType_SeqCache_RequireMakeEnumeration(DeeTypeObject *__restrict self) {
	Dee_tsc_makeenumeration_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_makeenumeration)
			return sc->tsc_makeenumeration;
	}
	result = DeeType_SeqCache_RequireMakeEnumeration_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_makeenumeration, result);
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_int_range_t DCALL
DeeType_SeqCache_RequireMakeEnumerationWithIntRange(DeeTypeObject *__restrict self) {
	Dee_tsc_makeenumeration_with_int_range_t result;
	Dee_tsc_makeenumeration_t base;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_makeenumeration_with_int_range)
			return sc->tsc_makeenumeration_with_int_range;
	}
	base = DeeType_SeqCache_RequireMakeEnumeration(self);
	if (base == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexFastAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndTryGetItemIndexAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeObAndGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithGetItemIndexAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter) {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter;
	} else if (base == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack) {
		result = &DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithEnumerate) {
		result = &DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex;
	} else {
		result = &DeeSeq_DefaultMakeEnumerationWithIntRangeWithError;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_makeenumeration_with_int_range, result);
	return result;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_range_t DCALL
DeeType_SeqCache_RequireMakeEnumerationWithRange(DeeTypeObject *__restrict self) {
	Dee_tsc_makeenumeration_with_range_t result;
	Dee_tsc_makeenumeration_t base;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_makeenumeration_with_range)
			return sc->tsc_makeenumeration_with_range;
	}
	base = DeeType_SeqCache_RequireMakeEnumeration(self);
	if (base == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast ||
	    base == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex ||
	    base == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex ||
	    base == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex) {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem) {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndGetItemAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter) {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter;
	} else if (base == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack) {
		result = &DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter;
	} else if (base == &DeeSeq_DefaultMakeEnumerationWithEnumerate) {
		result = &DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter;
	} else {
		result = &DeeSeq_DefaultMakeEnumerationWithRangeWithError;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_makeenumeration_with_range, result);
	return result;
}






/* Possible implementations for sequence cache functions. */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self,
                                                        Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index_fast)(self, size);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self,
                                                    Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index)(self, size);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self,
                                                       Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self,
                                                 Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	for (;;) {
		DREF DeeObject *item;
		int size_is_nonzero = DeeObject_Bool(sizeob);
		if unlikely(size_is_nonzero < 0)
			goto err_sizeob;
		if (!size_is_nonzero)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob;
		item = (*seq->tp_getitem)(self, sizeob);
		if likely(item) {
			temp = (*proc)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp_sizeob;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob;
		}
		if (DeeThread_CheckInterrupt())
			goto err_sizeob;
	}
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob:
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                               void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index_fast)(self, size);
		temp = (*proc)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                           void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_getitem_index)(self, size);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                              void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*proc)(arg, size, item);
			Dee_Decref(item);
		} else {
			temp = (*proc)(arg, size, NULL);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                        void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	if (end != (size_t)-1) {
		int error;
		DREF DeeObject *wanted_end;
		wanted_end = DeeInt_NewSize(end);
		if unlikely(!wanted_end)
			goto err_sizeob;
		/* if (sizeob > wanted_end)
		 *     sizeob = wanted_end; */
		error = DeeObject_CmpGrAsBool(sizeob, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
		} else {
			Dee_Decref(sizeob);
			sizeob = wanted_end;
			if unlikely(error < 0)
				goto err_sizeob;
		}
	}
	if (start != 0) {
		startob = DeeInt_NewSize(start);
		if unlikely(!startob)
			goto err_sizeob;
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *item;
		int size_is_greater_start;
		if (startob) {
			size_is_greater_start = DeeObject_CmpGrAsBool(sizeob, startob);
		} else {
			size_is_greater_start = DeeObject_Bool(sizeob);
		}
		if unlikely(size_is_greater_start < 0)
			goto err_sizeob_startob;
		if (!size_is_greater_start)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob_startob;
		item = (*seq->tp_getitem)(self, sizeob);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_startob;
		}
		if unlikely(DeeObject_AsSize(sizeob, &index_value))
			goto err_sizeob_startob;
		temp = 0;
		if likely(index_value >= start && index_value < end)
			temp = (*proc)(arg, index_value, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp_sizeob_startob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_startob;
	}
	Dee_XDecref(startob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_startob:
	Dee_XDecref(startob);
/*err_temp_sizeob:*/
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob_startob:
	Dee_XDecref(startob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}



INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetAttr(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	*(DREF DeeObject **)arg = item;
	return -2;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithForeachDefault(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundAttr(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_boundfirst_with_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	(void)item;
	return -2;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithForeachDefault(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &seq_default_boundfirst_with_foreach_cb, NULL);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if likely(foreach_status == -2)
		return 1;
	if unlikely(foreach_status == -1)
		return -1;
	return -2;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelAttr(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self) {
	return err_cant_access_attribute(Dee_TYPE(self), &str_first, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetAttr(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_first, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, DeeInt_Zero, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, 0, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value) {
	(void)value;
	return err_cant_access_attribute(Dee_TYPE(self), &str_first, ATTR_ACCESS_SET);
}




INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithGetAttr(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	result = (*seq->tp_getitem_index_fast)(self, size);
	if unlikely(!result)
		err_unbound_index(self, size);
	return result;
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_getitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeObAndGetItem(DeeObject *__restrict self) {
	int temp;
	DREF DeeObject *result, *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_last_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	Dee_Decref_unlikely(*(DREF DeeObject **)arg);
	*(DREF DeeObject **)arg = item;
	return 1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithForeachDefault(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	Dee_Incref(Dee_None);
	result = Dee_None;
	foreach_status = DeeObject_Foreach(self, &seq_default_last_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	result = (*seq->tp_getitem_index_fast)(self, size);
	if unlikely(!result)
		return 0;
	Dee_Decref(result);
	return 1;
err_isempty:
	return -2;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_bounditem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithBoundAttr(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeObAndBoundItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithDelAttr(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeAndDelItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_delitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeObAndDelItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self) {
	return err_cant_access_attribute(Dee_TYPE(self), &str_last, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSetAttr(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_last, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeAndSetItemIndex(DeeObject *self, DeeObject *value) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_setitem_index)(self, size, value);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeObAndSetItem(DeeObject *self, DeeObject *value) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, sizeob, value);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value) {
	(void)value;
	return err_cant_access_attribute(Dee_TYPE(self), &str_last, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultNonEmptyWithError(DeeObject *__restrict self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}

DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_DeeType_SeqCache_RequireGetX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBoundX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireDelX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetX
#include "default-api-require-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C */
