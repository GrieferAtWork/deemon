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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINTS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINTS_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/method-hints.h>

/**/
#include "../objects/seq/default-api.h"

DECL_BEGIN

#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	PUBLIC_CONST char const DeeMHDefault_##attr_name##_doc[] = doc;                                  \
	PUBLIC_CONST char const DeeMHDefault_##attr_name##_name[] = method_name;
#include "../../../include/deemon/method-hints.def"


typedef ATTR_PURE_T WUNUSED_T NONNULL_T((1)) Dee_funptr_t
(DCALL *require_tsc_cb_t)(DeeTypeObject *__restrict self);

#define REQUIRE_TSC_seq_foreach_reverse                     DeeType_TryRequireSeqForeachReverse
#define REQUIRE_TSC_seq_enumerate_index_reverse             DeeType_TryRequireSeqEnumerateIndexReverse
#define REQUIRE_TSC_seq_makeenumeration                     DeeType_RequireSeqMakeEnumeration
#define REQUIRE_TSC_seq_makeenumeration_with_int_range      DeeType_RequireSeqMakeEnumerationWithIntRange
#define REQUIRE_TSC_seq_makeenumeration_with_range          DeeType_RequireSeqMakeEnumerationWithRange
#define REQUIRE_TSC_seq_operator_bool                       DeeType_RequireSeqOperatorBool
#define REQUIRE_TSC_seq_operator_iter                       DeeType_RequireSeqOperatorIter
#define REQUIRE_TSC_seq_operator_sizeob                     DeeType_RequireSeqOperatorSizeOb
#define REQUIRE_TSC_seq_operator_contains                   DeeType_RequireSeqOperatorContains
#define REQUIRE_TSC_seq_operator_getitem                    DeeType_RequireSeqOperatorGetItem
#define REQUIRE_TSC_seq_operator_delitem                    DeeType_RequireSeqOperatorDelItem
#define REQUIRE_TSC_seq_operator_setitem                    DeeType_RequireSeqOperatorSetItem
#define REQUIRE_TSC_seq_operator_getrange                   DeeType_RequireSeqOperatorGetRange
#define REQUIRE_TSC_seq_operator_delrange                   DeeType_RequireSeqOperatorDelRange
#define REQUIRE_TSC_seq_operator_setrange                   DeeType_RequireSeqOperatorSetRange
#define REQUIRE_TSC_seq_operator_foreach                    DeeType_RequireSeqOperatorForeach
#define REQUIRE_TSC_seq_operator_foreach_pair               DeeType_RequireSeqOperatorForeachPair
#define REQUIRE_TSC_seq_operator_enumerate                  DeeType_RequireSeqOperatorEnumerate
#define REQUIRE_TSC_seq_operator_enumerate_index            DeeType_RequireSeqOperatorEnumerateIndex
#define REQUIRE_TSC_seq_operator_iterkeys                   DeeType_RequireSeqOperatorIterKeys
#define REQUIRE_TSC_seq_operator_bounditem                  DeeType_RequireSeqOperatorBoundItem
#define REQUIRE_TSC_seq_operator_hasitem                    DeeType_RequireSeqOperatorHasItem
#define REQUIRE_TSC_seq_operator_size                       DeeType_RequireSeqOperatorSize
#define REQUIRE_TSC_seq_operator_size_fast                  DeeType_RequireSeqOperatorSizeFast
#define REQUIRE_TSC_seq_operator_getitem_index              DeeType_RequireSeqOperatorGetItemIndex
#define REQUIRE_TSC_seq_operator_delitem_index              DeeType_RequireSeqOperatorDelItemIndex
#define REQUIRE_TSC_seq_operator_setitem_index              DeeType_RequireSeqOperatorSetItemIndex
#define REQUIRE_TSC_seq_operator_bounditem_index            DeeType_RequireSeqOperatorBoundItemIndex
#define REQUIRE_TSC_seq_operator_hasitem_index              DeeType_RequireSeqOperatorHasItemIndex
#define REQUIRE_TSC_seq_operator_getrange_index             DeeType_RequireSeqOperatorGetRangeIndex
#define REQUIRE_TSC_seq_operator_delrange_index             DeeType_RequireSeqOperatorDelRangeIndex
#define REQUIRE_TSC_seq_operator_setrange_index             DeeType_RequireSeqOperatorSetRangeIndex
#define REQUIRE_TSC_seq_operator_getrange_index_n           DeeType_RequireSeqOperatorGetRangeIndexN
#define REQUIRE_TSC_seq_operator_delrange_index_n           DeeType_RequireSeqOperatorDelRangeIndexN
#define REQUIRE_TSC_seq_operator_setrange_index_n           DeeType_RequireSeqOperatorSetRangeIndexN
#define REQUIRE_TSC_seq_operator_trygetitem                 DeeType_RequireSeqOperatorTryGetItem
#define REQUIRE_TSC_seq_operator_trygetitem_index           DeeType_RequireSeqOperatorTryGetItemIndex
#define REQUIRE_TSC_seq_operator_hash                       DeeType_RequireSeqOperatorHash
#define REQUIRE_TSC_seq_operator_compare_eq                 DeeType_RequireSeqOperatorCompareEq
#define REQUIRE_TSC_seq_operator_compare                    DeeType_RequireSeqOperatorCompare
#define REQUIRE_TSC_seq_operator_trycompare_eq              DeeType_RequireSeqOperatorTryCompareEq
#define REQUIRE_TSC_seq_operator_eq                         DeeType_RequireSeqOperatorEq
#define REQUIRE_TSC_seq_operator_ne                         DeeType_RequireSeqOperatorNe
#define REQUIRE_TSC_seq_operator_lo                         DeeType_RequireSeqOperatorLo
#define REQUIRE_TSC_seq_operator_le                         DeeType_RequireSeqOperatorLe
#define REQUIRE_TSC_seq_operator_gr                         DeeType_RequireSeqOperatorGr
#define REQUIRE_TSC_seq_operator_ge                         DeeType_RequireSeqOperatorGe
#define REQUIRE_TSC_seq_operator_inplace_add                DeeType_RequireSeqOperatorInplaceAdd
#define REQUIRE_TSC_seq_operator_inplace_mul                DeeType_RequireSeqOperatorInplaceMul
#define REQUIRE_TSC_set_operator_iter                       DeeType_RequireSetOperatorIter
#define REQUIRE_TSC_set_operator_foreach                    DeeType_RequireSetOperatorForeach
#define REQUIRE_TSC_set_operator_size                       DeeType_RequireSetOperatorSize
#define REQUIRE_TSC_set_operator_sizeob                     DeeType_RequireSetOperatorSizeOb
#define REQUIRE_TSC_set_operator_hash                       DeeType_RequireSetOperatorHash
#define REQUIRE_TSC_set_operator_compare_eq                 DeeType_RequireSetOperatorCompareEq
#define REQUIRE_TSC_set_operator_trycompare_eq              DeeType_RequireSetOperatorTryCompareEq
#define REQUIRE_TSC_set_operator_eq                         DeeType_RequireSetOperatorEq
#define REQUIRE_TSC_set_operator_ne                         DeeType_RequireSetOperatorNe
#define REQUIRE_TSC_set_operator_lo                         DeeType_RequireSetOperatorLo
#define REQUIRE_TSC_set_operator_le                         DeeType_RequireSetOperatorLe
#define REQUIRE_TSC_set_operator_gr                         DeeType_RequireSetOperatorGr
#define REQUIRE_TSC_set_operator_ge                         DeeType_RequireSetOperatorGe
#define REQUIRE_TSC_set_operator_inv                        DeeType_RequireSetOperatorInv
#define REQUIRE_TSC_set_operator_add                        DeeType_RequireSetOperatorAdd
#define REQUIRE_TSC_set_operator_sub                        DeeType_RequireSetOperatorSub
#define REQUIRE_TSC_set_operator_and                        DeeType_RequireSetOperatorAnd
#define REQUIRE_TSC_set_operator_xor                        DeeType_RequireSetOperatorXor
#define REQUIRE_TSC_set_operator_inplace_add                DeeType_RequireSetOperatorInplaceAdd
#define REQUIRE_TSC_set_operator_inplace_sub                DeeType_RequireSetOperatorInplaceSub
#define REQUIRE_TSC_set_operator_inplace_and                DeeType_RequireSetOperatorInplaceAnd
#define REQUIRE_TSC_set_operator_inplace_xor                DeeType_RequireSetOperatorInplaceXor
#define REQUIRE_TSC_map_operator_contains                   DeeType_RequireMapOperatorContains
#define REQUIRE_TSC_map_operator_getitem                    DeeType_RequireMapOperatorGetItem
#define REQUIRE_TSC_map_operator_delitem                    DeeType_RequireMapOperatorDelItem
#define REQUIRE_TSC_map_operator_setitem                    DeeType_RequireMapOperatorSetItem
#define REQUIRE_TSC_map_operator_enumerate                  DeeType_RequireMapOperatorEnumerate
#define REQUIRE_TSC_map_operator_enumerate_index            DeeType_RequireMapOperatorEnumerateIndex
#define REQUIRE_TSC_map_operator_bounditem                  DeeType_RequireMapOperatorBoundItem
#define REQUIRE_TSC_map_operator_hasitem                    DeeType_RequireMapOperatorHasItem
#define REQUIRE_TSC_map_operator_getitem_index              DeeType_RequireMapOperatorGetItemIndex
#define REQUIRE_TSC_map_operator_delitem_index              DeeType_RequireMapOperatorDelItemIndex
#define REQUIRE_TSC_map_operator_setitem_index              DeeType_RequireMapOperatorSetItemIndex
#define REQUIRE_TSC_map_operator_bounditem_index            DeeType_RequireMapOperatorBoundItemIndex
#define REQUIRE_TSC_map_operator_hasitem_index              DeeType_RequireMapOperatorHasItemIndex
#define REQUIRE_TSC_map_operator_trygetitem                 DeeType_RequireMapOperatorTryGetItem
#define REQUIRE_TSC_map_operator_trygetitem_index           DeeType_RequireMapOperatorTryGetItemIndex
#define REQUIRE_TSC_map_operator_trygetitem_string_hash     DeeType_RequireMapOperatorTryGetItemStringHash
#define REQUIRE_TSC_map_operator_getitem_string_hash        DeeType_RequireMapOperatorGetItemStringHash
#define REQUIRE_TSC_map_operator_delitem_string_hash        DeeType_RequireMapOperatorDelItemStringHash
#define REQUIRE_TSC_map_operator_setitem_string_hash        DeeType_RequireMapOperatorSetItemStringHash
#define REQUIRE_TSC_map_operator_bounditem_string_hash      DeeType_RequireMapOperatorBoundItemStringHash
#define REQUIRE_TSC_map_operator_hasitem_string_hash        DeeType_RequireMapOperatorHasItemStringHash
#define REQUIRE_TSC_map_operator_trygetitem_string_len_hash DeeType_RequireMapOperatorTryGetItemStringLenHash
#define REQUIRE_TSC_map_operator_getitem_string_len_hash    DeeType_RequireMapOperatorGetItemStringLenHash
#define REQUIRE_TSC_map_operator_delitem_string_len_hash    DeeType_RequireMapOperatorDelItemStringLenHash
#define REQUIRE_TSC_map_operator_setitem_string_len_hash    DeeType_RequireMapOperatorSetItemStringLenHash
#define REQUIRE_TSC_map_operator_bounditem_string_len_hash  DeeType_RequireMapOperatorBoundItemStringLenHash
#define REQUIRE_TSC_map_operator_hasitem_string_len_hash    DeeType_RequireMapOperatorHasItemStringLenHash
#define REQUIRE_TSC_map_operator_compare_eq                 DeeType_RequireMapOperatorCompareEq
#define REQUIRE_TSC_map_operator_trycompare_eq              DeeType_RequireMapOperatorTryCompareEq
#define REQUIRE_TSC_map_operator_eq                         DeeType_RequireMapOperatorEq
#define REQUIRE_TSC_map_operator_ne                         DeeType_RequireMapOperatorNe
#define REQUIRE_TSC_map_operator_lo                         DeeType_RequireMapOperatorLo
#define REQUIRE_TSC_map_operator_le                         DeeType_RequireMapOperatorLe
#define REQUIRE_TSC_map_operator_gr                         DeeType_RequireMapOperatorGr
#define REQUIRE_TSC_map_operator_ge                         DeeType_RequireMapOperatorGe
#define REQUIRE_TSC_map_operator_add                        DeeType_RequireMapOperatorAdd
#define REQUIRE_TSC_map_operator_sub                        DeeType_RequireMapOperatorSub
#define REQUIRE_TSC_map_operator_and                        DeeType_RequireMapOperatorAnd
#define REQUIRE_TSC_map_operator_xor                        DeeType_RequireMapOperatorXor
#define REQUIRE_TSC_map_operator_inplace_add                DeeType_RequireMapOperatorInplaceAdd
#define REQUIRE_TSC_map_operator_inplace_sub                DeeType_RequireMapOperatorInplaceSub
#define REQUIRE_TSC_map_operator_inplace_and                DeeType_RequireMapOperatorInplaceAnd
#define REQUIRE_TSC_map_operator_inplace_xor                DeeType_RequireMapOperatorInplaceXor
#define REQUIRE_TSC_seq_trygetfirst                         DeeType_RequireSeqTryGetFirst
#define REQUIRE_TSC_seq_getfirst                            DeeType_RequireSeqGetFirst
#define REQUIRE_TSC_seq_boundfirst                          DeeType_RequireSeqBoundFirst
#define REQUIRE_TSC_seq_delfirst                            DeeType_RequireSeqDelFirst
#define REQUIRE_TSC_seq_setfirst                            DeeType_RequireSeqSetFirst
#define REQUIRE_TSC_seq_trygetlast                          DeeType_RequireSeqTryGetLast
#define REQUIRE_TSC_seq_getlast                             DeeType_RequireSeqGetLast
#define REQUIRE_TSC_seq_boundlast                           DeeType_RequireSeqBoundLast
#define REQUIRE_TSC_seq_dellast                             DeeType_RequireSeqDelLast
#define REQUIRE_TSC_seq_setlast                             DeeType_RequireSeqSetLast
#define REQUIRE_TSC_seq_any                                 DeeType_RequireSeqAny
#define REQUIRE_TSC_seq_any_with_key                        DeeType_RequireSeqAnyWithKey
#define REQUIRE_TSC_seq_any_with_range                      DeeType_RequireSeqAnyWithRange
#define REQUIRE_TSC_seq_any_with_range_and_key              DeeType_RequireSeqAnyWithRangeAndKey
#define REQUIRE_TSC_seq_all                                 DeeType_RequireSeqAll
#define REQUIRE_TSC_seq_all_with_key                        DeeType_RequireSeqAllWithKey
#define REQUIRE_TSC_seq_all_with_range                      DeeType_RequireSeqAllWithRange
#define REQUIRE_TSC_seq_all_with_range_and_key              DeeType_RequireSeqAllWithRangeAndKey
#define REQUIRE_TSC_seq_parity                              DeeType_RequireSeqParity
#define REQUIRE_TSC_seq_parity_with_key                     DeeType_RequireSeqParityWithKey
#define REQUIRE_TSC_seq_parity_with_range                   DeeType_RequireSeqParityWithRange
#define REQUIRE_TSC_seq_parity_with_range_and_key           DeeType_RequireSeqParityWithRangeAndKey
#define REQUIRE_TSC_seq_reduce                              DeeType_RequireSeqReduce
#define REQUIRE_TSC_seq_reduce_with_init                    DeeType_RequireSeqReduceWithInit
#define REQUIRE_TSC_seq_reduce_with_range                   DeeType_RequireSeqReduceWithRange
#define REQUIRE_TSC_seq_reduce_with_range_and_init          DeeType_RequireSeqReduceWithRangeAndInit
#define REQUIRE_TSC_seq_min                                 DeeType_RequireSeqMin
#define REQUIRE_TSC_seq_min_with_key                        DeeType_RequireSeqMinWithKey
#define REQUIRE_TSC_seq_min_with_range                      DeeType_RequireSeqMinWithRange
#define REQUIRE_TSC_seq_min_with_range_and_key              DeeType_RequireSeqMinWithRangeAndKey
#define REQUIRE_TSC_seq_max                                 DeeType_RequireSeqMax
#define REQUIRE_TSC_seq_max_with_key                        DeeType_RequireSeqMaxWithKey
#define REQUIRE_TSC_seq_max_with_range                      DeeType_RequireSeqMaxWithRange
#define REQUIRE_TSC_seq_max_with_range_and_key              DeeType_RequireSeqMaxWithRangeAndKey
#define REQUIRE_TSC_seq_sum                                 DeeType_RequireSeqSum
#define REQUIRE_TSC_seq_sum_with_range                      DeeType_RequireSeqSumWithRange
#define REQUIRE_TSC_seq_count                               DeeType_RequireSeqCount
#define REQUIRE_TSC_seq_count_with_key                      DeeType_RequireSeqCountWithKey
#define REQUIRE_TSC_seq_count_with_range                    DeeType_RequireSeqCountWithRange
#define REQUIRE_TSC_seq_count_with_range_and_key            DeeType_RequireSeqCountWithRangeAndKey
#define REQUIRE_TSC_seq_contains                            DeeType_RequireSeqContains
#define REQUIRE_TSC_seq_contains_with_key                   DeeType_RequireSeqContainsWithKey
#define REQUIRE_TSC_seq_contains_with_range                 DeeType_RequireSeqContainsWithRange
#define REQUIRE_TSC_seq_contains_with_range_and_key         DeeType_RequireSeqContainsWithRangeAndKey
#define REQUIRE_TSC_seq_locate                              DeeType_RequireSeqLocate
#define REQUIRE_TSC_seq_locate_with_key                     DeeType_RequireSeqLocateWithKey
#define REQUIRE_TSC_seq_locate_with_range                   DeeType_RequireSeqLocateWithRange
#define REQUIRE_TSC_seq_locate_with_range_and_key           DeeType_RequireSeqLocateWithRangeAndKey
#define REQUIRE_TSC_seq_rlocate_with_range                  DeeType_RequireSeqRLocateWithRange
#define REQUIRE_TSC_seq_rlocate_with_range_and_key          DeeType_RequireSeqRLocateWithRangeAndKey
#define REQUIRE_TSC_seq_startswith                          DeeType_RequireSeqStartsWith
#define REQUIRE_TSC_seq_startswith_with_key                 DeeType_RequireSeqStartsWithWithKey
#define REQUIRE_TSC_seq_startswith_with_range               DeeType_RequireSeqStartsWithWithRange
#define REQUIRE_TSC_seq_startswith_with_range_and_key       DeeType_RequireSeqStartsWithWithRangeAndKey
#define REQUIRE_TSC_seq_endswith                            DeeType_RequireSeqEndsWith
#define REQUIRE_TSC_seq_endswith_with_key                   DeeType_RequireSeqEndsWithWithKey
#define REQUIRE_TSC_seq_endswith_with_range                 DeeType_RequireSeqEndsWithWithRange
#define REQUIRE_TSC_seq_endswith_with_range_and_key         DeeType_RequireSeqEndsWithWithRangeAndKey
#define REQUIRE_TSC_seq_find                                DeeType_RequireSeqFind
#define REQUIRE_TSC_seq_find_with_key                       DeeType_RequireSeqFindWithKey
#define REQUIRE_TSC_seq_rfind                               DeeType_RequireSeqRFind
#define REQUIRE_TSC_seq_rfind_with_key                      DeeType_RequireSeqRFindWithKey
#define REQUIRE_TSC_seq_erase                               DeeType_RequireSeqErase
#define REQUIRE_TSC_seq_insert                              DeeType_RequireSeqInsert
#define REQUIRE_TSC_seq_insertall                           DeeType_RequireSeqInsertAll
#define REQUIRE_TSC_seq_pushfront                           DeeType_RequireSeqPushFront
#define REQUIRE_TSC_seq_append                              DeeType_RequireSeqAppend
#define REQUIRE_TSC_seq_extend                              DeeType_RequireSeqExtend
#define REQUIRE_TSC_seq_xchitem_index                       DeeType_RequireSeqXchItemIndex
#define REQUIRE_TSC_seq_clear                               DeeType_RequireSeqClear
#define REQUIRE_TSC_seq_pop                                 DeeType_RequireSeqPop
#define REQUIRE_TSC_seq_remove                              DeeType_RequireSeqRemove
#define REQUIRE_TSC_seq_remove_with_key                     DeeType_RequireSeqRemoveWithKey
#define REQUIRE_TSC_seq_rremove                             DeeType_RequireSeqRRemove
#define REQUIRE_TSC_seq_rremove_with_key                    DeeType_RequireSeqRRemoveWithKey
#define REQUIRE_TSC_seq_removeall                           DeeType_RequireSeqRemoveAll
#define REQUIRE_TSC_seq_removeall_with_key                  DeeType_RequireSeqRemoveAllWithKey
#define REQUIRE_TSC_seq_removeif                            DeeType_RequireSeqRemoveIf
#define REQUIRE_TSC_seq_resize                              DeeType_RequireSeqResize
#define REQUIRE_TSC_seq_fill                                DeeType_RequireSeqFill
#define REQUIRE_TSC_seq_reverse                             DeeType_RequireSeqReverse
#define REQUIRE_TSC_seq_reversed                            DeeType_RequireSeqReversed
#define REQUIRE_TSC_seq_sort                                DeeType_RequireSeqSort
#define REQUIRE_TSC_seq_sort_with_key                       DeeType_RequireSeqSortWithKey
#define REQUIRE_TSC_seq_sorted                              DeeType_RequireSeqSorted
#define REQUIRE_TSC_seq_sorted_with_key                     DeeType_RequireSeqSortedWithKey
#define REQUIRE_TSC_seq_bfind                               DeeType_RequireSeqBFind
#define REQUIRE_TSC_seq_bfind_with_key                      DeeType_RequireSeqBFindWithKey
#define REQUIRE_TSC_seq_bposition                           DeeType_RequireSeqBPosition
#define REQUIRE_TSC_seq_bposition_with_key                  DeeType_RequireSeqBPositionWithKey
#define REQUIRE_TSC_seq_brange                              DeeType_RequireSeqBRange
#define REQUIRE_TSC_seq_brange_with_key                     DeeType_RequireSeqBRangeWithKey
#define REQUIRE_TSC_seq_blocate                             DeeType_RequireSeqBLocate
#define REQUIRE_TSC_seq_blocate_with_key                    DeeType_RequireSeqBLocateWithKey
#define REQUIRE_TSC_set_insert                              DeeType_RequireSetInsert
#define REQUIRE_TSC_set_remove                              DeeType_RequireSetRemove
#define REQUIRE_TSC_set_unify                               DeeType_RequireSetUnify
#define REQUIRE_TSC_set_insertall                           DeeType_RequireSetInsertAll
#define REQUIRE_TSC_set_removeall                           DeeType_RequireSetRemoveAll
#define REQUIRE_TSC_set_pop                                 DeeType_RequireSetPop
#define REQUIRE_TSC_set_pop_with_default                    DeeType_RequireSetPopWithDefault
#define REQUIRE_TSC_map_setold                              DeeType_RequireMapSetOld
#define REQUIRE_TSC_map_setold_ex                           DeeType_RequireMapSetOldEx
#define REQUIRE_TSC_map_setnew                              DeeType_RequireMapSetNew
#define REQUIRE_TSC_map_setnew_ex                           DeeType_RequireMapSetNewEx
#define REQUIRE_TSC_map_setdefault                          DeeType_RequireMapSetDefault
#define REQUIRE_TSC_map_update                              DeeType_RequireMapUpdate
#define REQUIRE_TSC_map_remove                              DeeType_RequireMapRemove
#define REQUIRE_TSC_map_removekeys                          DeeType_RequireMapRemoveKeys
#define REQUIRE_TSC_map_pop                                 DeeType_RequireMapPop
#define REQUIRE_TSC_map_pop_with_default                    DeeType_RequireMapPopWithDefault
#define REQUIRE_TSC_map_popitem                             DeeType_RequireMapPopItem
#define REQUIRE_TSC_map_keys                                DeeType_RequireMapKeys
#define REQUIRE_TSC_map_values                              DeeType_RequireMapValues
#define REQUIRE_TSC_map_iterkeys                            DeeType_RequireMapIterKeys
#define REQUIRE_TSC_map_itervalues                          DeeType_RequireMapIterValues


PRIVATE require_tsc_cb_t tpconst require_tsc_table[] = {
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	(require_tsc_cb_t)&REQUIRE_TSC_##func_name,
#define Dee_DEFINE_TYPE_METHOD_HINT_TSC_ONLY
#include "../../../include/deemon/method-hints.def"
#undef Dee_DEFINE_TYPE_METHOD_HINT_TSC_ONLY
};


/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
PUBLIC ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *__restrict self, uintptr_t id) {
	struct type_method_hint const *hints = self->tp_method_hints;
	if unlikely(!hints)
		goto done;
	for (; hints->tmh_func; ++hints) {
		if (hints->tmh_id == id)
			return hints->tmh_func;
	}
done:
	return NULL;
}

/* Same as `DeeType_GetPrivateMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions. */
PUBLIC ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetMethodHint(DeeTypeObject *__restrict self, uintptr_t id) {
	Dee_funptr_t result;

	/* Special handling for TSC (TypeSequenceCache) related method hints. */
	if (id < COMPILER_LENOF(require_tsc_table))
		return (*require_tsc_table[id])(self);

	/* Search the type's MRO for this method hint. */
	/* TODO: This is wrong -- must search *only* the type that is the origin
	 *       of whatever attribute/operator the method hint is for. */
	DeeType_mro_foreach_start(self) {
		result = DeeType_GetPrivateMethodHint(self, id);
		if (result != NULL)
			goto done;
	}
	DeeType_mro_foreach_end(self);
done:
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINTS_C */
