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
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

/**/
#include "../../runtime/operator-require.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

/**/
#include "default-enumerate.h"

DECL_BEGIN

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_trygetfirst_t DCALL DeeType_RequireSeqTryGetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_getfirst_t DCALL DeeType_RequireSeqGetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_boundfirst_t DCALL DeeType_RequireSeqBoundFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_delfirst_t DCALL DeeType_RequireSeqDelFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_setfirst_t DCALL DeeType_RequireSeqSetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_trygetlast_t DCALL DeeType_RequireSeqTryGetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_getlast_t DCALL DeeType_RequireSeqGetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_boundlast_t DCALL DeeType_RequireSeqBoundLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_dellast_t DCALL DeeType_RequireSeqDelLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_setlast_t DCALL DeeType_RequireSeqSetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_any_t DCALL DeeType_RequireSeqAny_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_any_with_key_t DCALL DeeType_RequireSeqAnyWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_any_with_range_t DCALL DeeType_RequireSeqAnyWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_any_with_range_and_key_t DCALL DeeType_RequireSeqAnyWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_all_t DCALL DeeType_RequireSeqAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_all_with_key_t DCALL DeeType_RequireSeqAllWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_all_with_range_t DCALL DeeType_RequireSeqAllWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_all_with_range_and_key_t DCALL DeeType_RequireSeqAllWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_parity_t DCALL DeeType_RequireSeqParity_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_parity_with_key_t DCALL DeeType_RequireSeqParityWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_parity_with_range_t DCALL DeeType_RequireSeqParityWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_parity_with_range_and_key_t DCALL DeeType_RequireSeqParityWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reduce_t DCALL DeeType_RequireSeqReduce_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reduce_with_init_t DCALL DeeType_RequireSeqReduceWithInit_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reduce_with_range_t DCALL DeeType_RequireSeqReduceWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reduce_with_range_and_init_t DCALL DeeType_RequireSeqReduceWithRangeAndInit_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_min_t DCALL DeeType_RequireSeqMin_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_min_with_key_t DCALL DeeType_RequireSeqMinWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_min_with_range_t DCALL DeeType_RequireSeqMinWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_min_with_range_and_key_t DCALL DeeType_RequireSeqMinWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_max_t DCALL DeeType_RequireSeqMax_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_max_with_key_t DCALL DeeType_RequireSeqMaxWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_max_with_range_t DCALL DeeType_RequireSeqMaxWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_max_with_range_and_key_t DCALL DeeType_RequireSeqMaxWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_count_t DCALL DeeType_RequireSeqCount_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_count_with_key_t DCALL DeeType_RequireSeqCountWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_count_with_range_t DCALL DeeType_RequireSeqCountWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_count_with_range_and_key_t DCALL DeeType_RequireSeqCountWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_contains_t DCALL DeeType_RequireSeqContains_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_contains_with_key_t DCALL DeeType_RequireSeqContainsWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_contains_with_range_t DCALL DeeType_RequireSeqContainsWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_contains_with_range_and_key_t DCALL DeeType_RequireSeqContainsWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_locate_t DCALL DeeType_RequireSeqLocate_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_locate_with_range_t DCALL DeeType_RequireSeqLocateWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_rlocate_with_range_t DCALL DeeType_RequireSeqRLocateWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_startswith_t DCALL DeeType_RequireSeqStartsWith_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_startswith_with_key_t DCALL DeeType_RequireSeqStartsWithWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_startswith_with_range_t DCALL DeeType_RequireSeqStartsWithWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_startswith_with_range_and_key_t DCALL DeeType_RequireSeqStartsWithWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_endswith_t DCALL DeeType_RequireSeqEndsWith_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_endswith_with_key_t DCALL DeeType_RequireSeqEndsWithWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_endswith_with_range_t DCALL DeeType_RequireSeqEndsWithWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_endswith_with_range_and_key_t DCALL DeeType_RequireSeqEndsWithWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_find_t DCALL DeeType_RequireSeqFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_find_with_key_t DCALL DeeType_RequireSeqFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_rfind_t DCALL DeeType_RequireSeqRFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_rfind_with_key_t DCALL DeeType_RequireSeqRFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_erase_t DCALL DeeType_RequireSeqErase_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_insert_t DCALL DeeType_RequireSeqInsert_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_insertall_t DCALL DeeType_RequireSeqInsertAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_pushfront_t DCALL DeeType_RequireSeqPushFront_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_append_t DCALL DeeType_RequireSeqAppend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_extend_t DCALL DeeType_RequireSeqExtend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_xchitem_index_t DCALL DeeType_RequireSeqXchItemIndex_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_clear_t DCALL DeeType_RequireSeqClear_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_pop_t DCALL DeeType_RequireSeqPop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_remove_t DCALL DeeType_RequireSeqRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_remove_with_key_t DCALL DeeType_RequireSeqRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_rremove_t DCALL DeeType_RequireSeqRRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_rremove_with_key_t DCALL DeeType_RequireSeqRRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_removeall_t DCALL DeeType_RequireSeqRemoveAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_removeall_with_key_t DCALL DeeType_RequireSeqRemoveAllWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_removeif_t DCALL DeeType_RequireSeqRemoveIf_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_resize_t DCALL DeeType_RequireSeqResize_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_fill_t DCALL DeeType_RequireSeqFill_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reverse_t DCALL DeeType_RequireSeqReverse_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_reversed_t DCALL DeeType_RequireSeqReversed_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_sort_t DCALL DeeType_RequireSeqSort_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_sort_with_key_t DCALL DeeType_RequireSeqSortWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_sorted_t DCALL DeeType_RequireSeqSorted_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_sorted_with_key_t DCALL DeeType_RequireSeqSortedWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_bfind_t DCALL DeeType_RequireSeqBFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_bfind_with_key_t DCALL DeeType_RequireSeqBFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_bposition_t DCALL DeeType_RequireSeqBPosition_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_bposition_with_key_t DCALL DeeType_RequireSeqBPositionWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_brange_t DCALL DeeType_RequireSeqBRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_seq_brange_with_key_t DCALL DeeType_RequireSeqBRangeWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_insert_t DCALL DeeType_RequireSetInsert_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_remove_t DCALL DeeType_RequireSetRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_unify_t DCALL DeeType_RequireSetUnify_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_insertall_t DCALL DeeType_RequireSetInsertAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_removeall_t DCALL DeeType_RequireSetRemoveAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_pop_t DCALL DeeType_RequireSetPop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_set_pop_with_default_t DCALL DeeType_RequireSetPopWithDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_setold_t DCALL DeeType_RequireMapSetOld_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_setold_ex_t DCALL DeeType_RequireMapSetOldEx_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_setnew_t DCALL DeeType_RequireMapSetNew_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_setnew_ex_t DCALL DeeType_RequireMapSetNewEx_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_setdefault_t DCALL DeeType_RequireMapSetDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_update_t DCALL DeeType_RequireMapUpdate_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_remove_t DCALL DeeType_RequireMapRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_removekeys_t DCALL DeeType_RequireMapRemoveKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_pop_t DCALL DeeType_RequireMapPop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_pop_with_default_t DCALL DeeType_RequireMapPopWithDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_popitem_t DCALL DeeType_RequireMapPopItem_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_keys_t DCALL DeeType_RequireMapKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_values_t DCALL DeeType_RequireMapValues_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_iterkeys_t DCALL DeeType_RequireMapIterKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_mh_map_itervalues_t DCALL DeeType_RequireMapIterValues_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetfirst_t DCALL DeeType_RequireSeqTryGetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_getfirst_t DCALL DeeType_RequireSeqGetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_boundfirst_t DCALL DeeType_RequireSeqBoundFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_delfirst_t DCALL DeeType_RequireSeqDelFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_setfirst_t DCALL DeeType_RequireSeqSetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetlast_t DCALL DeeType_RequireSeqTryGetLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_getlast_t DCALL DeeType_RequireSeqGetLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_boundlast_t DCALL DeeType_RequireSeqBoundLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_dellast_t DCALL DeeType_RequireSeqDelLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_setlast_t DCALL DeeType_RequireSeqSetLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_t DCALL DeeType_RequireSeqAny_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_key_t DCALL DeeType_RequireSeqAnyWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_t DCALL DeeType_RequireSeqAnyWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_and_key_t DCALL DeeType_RequireSeqAnyWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_t DCALL DeeType_RequireSeqAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_key_t DCALL DeeType_RequireSeqAllWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_t DCALL DeeType_RequireSeqAllWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_and_key_t DCALL DeeType_RequireSeqAllWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_t DCALL DeeType_RequireSeqParity_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_key_t DCALL DeeType_RequireSeqParityWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_t DCALL DeeType_RequireSeqParityWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_and_key_t DCALL DeeType_RequireSeqParityWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_t DCALL DeeType_RequireSeqReduce_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_init_t DCALL DeeType_RequireSeqReduceWithInit_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_t DCALL DeeType_RequireSeqReduceWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_and_init_t DCALL DeeType_RequireSeqReduceWithRangeAndInit_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_t DCALL DeeType_RequireSeqMin_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_key_t DCALL DeeType_RequireSeqMinWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_t DCALL DeeType_RequireSeqMinWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_and_key_t DCALL DeeType_RequireSeqMinWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_t DCALL DeeType_RequireSeqMax_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_key_t DCALL DeeType_RequireSeqMaxWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_t DCALL DeeType_RequireSeqMaxWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_and_key_t DCALL DeeType_RequireSeqMaxWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_t DCALL DeeType_RequireSeqCount_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_key_t DCALL DeeType_RequireSeqCountWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_t DCALL DeeType_RequireSeqCountWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_and_key_t DCALL DeeType_RequireSeqCountWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_t DCALL DeeType_RequireSeqContains_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_key_t DCALL DeeType_RequireSeqContainsWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_t DCALL DeeType_RequireSeqContainsWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_and_key_t DCALL DeeType_RequireSeqContainsWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_t DCALL DeeType_RequireSeqLocate_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_with_range_t DCALL DeeType_RequireSeqLocateWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rlocate_with_range_t DCALL DeeType_RequireSeqRLocateWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_t DCALL DeeType_RequireSeqStartsWith_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_key_t DCALL DeeType_RequireSeqStartsWithWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_t DCALL DeeType_RequireSeqStartsWithWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_and_key_t DCALL DeeType_RequireSeqStartsWithWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_t DCALL DeeType_RequireSeqEndsWith_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_key_t DCALL DeeType_RequireSeqEndsWithWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_t DCALL DeeType_RequireSeqEndsWithWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_and_key_t DCALL DeeType_RequireSeqEndsWithWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_t DCALL DeeType_RequireSeqFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_with_key_t DCALL DeeType_RequireSeqFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_t DCALL DeeType_RequireSeqRFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_with_key_t DCALL DeeType_RequireSeqRFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_erase_t DCALL DeeType_RequireSeqErase_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insert_t DCALL DeeType_RequireSeqInsert_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insertall_t DCALL DeeType_RequireSeqInsertAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pushfront_t DCALL DeeType_RequireSeqPushFront_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_append_t DCALL DeeType_RequireSeqAppend_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_extend_t DCALL DeeType_RequireSeqExtend_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_xchitem_index_t DCALL DeeType_RequireSeqXchItemIndex_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_clear_t DCALL DeeType_RequireSeqClear_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pop_t DCALL DeeType_RequireSeqPop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_t DCALL DeeType_RequireSeqRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_with_key_t DCALL DeeType_RequireSeqRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_t DCALL DeeType_RequireSeqRRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_with_key_t DCALL DeeType_RequireSeqRRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_t DCALL DeeType_RequireSeqRemoveAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_with_key_t DCALL DeeType_RequireSeqRemoveAllWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeif_t DCALL DeeType_RequireSeqRemoveIf_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_resize_t DCALL DeeType_RequireSeqResize_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_fill_t DCALL DeeType_RequireSeqFill_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reverse_t DCALL DeeType_RequireSeqReverse_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reversed_t DCALL DeeType_RequireSeqReversed_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_t DCALL DeeType_RequireSeqSort_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_with_key_t DCALL DeeType_RequireSeqSortWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_t DCALL DeeType_RequireSeqSorted_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_with_key_t DCALL DeeType_RequireSeqSortedWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_t DCALL DeeType_RequireSeqBFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_with_key_t DCALL DeeType_RequireSeqBFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_t DCALL DeeType_RequireSeqBPosition_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_with_key_t DCALL DeeType_RequireSeqBPositionWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_t DCALL DeeType_RequireSeqBRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_with_key_t DCALL DeeType_RequireSeqBRangeWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insert_t DCALL DeeType_RequireSetInsert_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_remove_t DCALL DeeType_RequireSetRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_unify_t DCALL DeeType_RequireSetUnify_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insertall_t DCALL DeeType_RequireSetInsertAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_removeall_t DCALL DeeType_RequireSetRemoveAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_t DCALL DeeType_RequireSetPop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_with_default_t DCALL DeeType_RequireSetPopWithDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_t DCALL DeeType_RequireMapSetOld_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_ex_t DCALL DeeType_RequireMapSetOldEx_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_t DCALL DeeType_RequireMapSetNew_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_ex_t DCALL DeeType_RequireMapSetNewEx_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setdefault_t DCALL DeeType_RequireMapSetDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_update_t DCALL DeeType_RequireMapUpdate_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_remove_t DCALL DeeType_RequireMapRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_removekeys_t DCALL DeeType_RequireMapRemoveKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_t DCALL DeeType_RequireMapPop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_with_default_t DCALL DeeType_RequireMapPopWithDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_popitem_t DCALL DeeType_RequireMapPopItem_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_keys_t DCALL DeeType_RequireMapKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_values_t DCALL DeeType_RequireMapValues_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_iterkeys_t DCALL DeeType_RequireMapIterKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_itervalues_t DCALL DeeType_RequireMapIterValues_uncached(DeeTypeObject *__restrict self);


/* Query for custom attributes */
#define DeeType_HasPrivateCustomSeqInsert(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_insert, Dee_HashStr__insert, (Dee_objmethod_t)&DeeMH_seq_insert, Dee_TMH_seq_insert)
#define DeeType_HasPrivateCustomSeqPushBack(self)   DeeType_HasPrivateCustomMethodOrHint(self, STR_pushback, Dee_HashStr__pushback, (Dee_objmethod_t)&DeeMH_seq_append, Dee_TMH_seq_append)
#define DeeType_HasPrivateCustomSeqAppend(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_append, Dee_HashStr__append, (Dee_objmethod_t)&DeeMH_seq_append, Dee_TMH_seq_append)
#define DeeType_HasPrivateCustomSeqErase(self)      DeeType_HasPrivateCustomMethodOrHint(self, STR_erase, Dee_HashStr__erase, (Dee_objmethod_t)&DeeMH_seq_erase, Dee_TMH_seq_erase)
#define DeeType_HasPrivateCustomSeqRemoveAll(self)  DeeType_HasPrivateCustomMethodOrHint(self, STR_removeall, Dee_HashStr__removeall, (Dee_objmethod_t)&DeeMH_seq_removeall, Dee_TMH_seq_removeall)
#define DeeType_HasPrivateCustomSeqRemove(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_remove, Dee_HashStr__remove, (Dee_objmethod_t)&DeeMH_seq_remove, Dee_TMH_seq_remove)
#define DeeType_HasPrivateCustomSetInsertAll(self)  DeeType_HasPrivateCustomMethodOrHint(self, STR_insertall, Dee_HashStr__insertall, &DeeMH_set_insertall, Dee_TMH_set_insertall)
#define DeeType_HasPrivateCustomSetRemoveAll(self)  DeeType_HasPrivateCustomMethodOrHint(self, STR_removeall, Dee_HashStr__removeall, &DeeMH_set_removeall, Dee_TMH_set_removeall)
#define DeeType_HasPrivateCustomMapSetDefault(self) DeeType_HasPrivateCustomMethodOrHint(self, STR_setdefault, Dee_HashStr__setdefault, &DeeMH_map_setdefault, Dee_TMH_map_setdefault)
#define DeeType_HasPrivateCustomMapSetNew(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_setnew, Dee_HashStr__setnew, &DeeMH_map_setnew, Dee_TMH_map_setnew)
#define DeeType_HasPrivateCustomMapSetOld(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_setold, Dee_HashStr__setold, &DeeMH_map_setold, Dee_TMH_map_setold)
#define DeeType_HasPrivateCustomMapSetNewEx(self)   DeeType_HasPrivateCustomMethodOrHint(self, STR_setnew_ex, Dee_HashStr__setnew_ex, &DeeMH_map_setnew_ex, Dee_TMH_map_setnew_ex)
#define DeeType_HasPrivateCustomMapSetOldEx(self)   DeeType_HasPrivateCustomMethodOrHint(self, STR_setold_ex, Dee_HashStr__setold_ex, &DeeMH_map_setold_ex, Dee_TMH_map_setold_ex)
#define DeeType_HasPrivateCustomMapRemoveKeys(self) DeeType_HasPrivateCustomMethodOrHint(self, STR_removekeys, Dee_HashStr__removekeys, &DeeMH_map_removekeys, Dee_TMH_map_removekeys)
#define DeeType_HasPrivateCustomMapRemove(self)     DeeType_HasPrivateCustomMethodOrHint(self, STR_remove, Dee_HashStr__remove, &DeeMH_map_remove, Dee_TMH_map_remove)
#define DeeType_HasPrivateCustomMapKeys(self)       DeeType_HasPrivateCustomGetter(self, STR_keys, Dee_HashStr__keys, &default_map_keys)
#define DeeType_HasPrivateCustomMapValues(self)     DeeType_HasPrivateCustomGetter(self, STR_values, Dee_HashStr__values, &default_map_values)

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2, 4)) bool DCALL
DeeType_HasPrivateCustomMethodOrHint(DeeTypeObject *__restrict self, char const *method_name,
                                     Dee_hash_t method_name_hash, Dee_objmethod_t mh_default,
                                     enum Dee_tmh_id mh_id) {
	struct Dee_attrinfo info;
	if (!DeeObject_TFindPrivateAttrInfoStringHash(self, NULL, method_name, method_name_hash, &info))
		goto nope;
	if (info.ai_type == Dee_ATTRINFO_METHOD) {
		if (info.ai_value.v_method->m_func == mh_default) {
			if (!DeeType_GetPrivateMethodHint((DeeTypeObject *)info.ai_decl, mh_id))
				goto nope;
		}
	} else {
		/* When it's some other kind of attribute -> always use it! */
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2, 4)) bool DCALL
DeeType_HasPrivateCustomGetter(DeeTypeObject *__restrict self, char const *getset_name,
                               Dee_hash_t getset_name_hash, Dee_getmethod_t mh_default) {
	struct Dee_attrinfo info;
	if (!DeeObject_TFindPrivateAttrInfoStringHash(self, NULL, getset_name, getset_name_hash, &info))
		goto nope;
	if (info.ai_type == Dee_ATTRINFO_GETSET) {
		if (info.ai_value.v_getset->gs_get == mh_default)
			goto nope;
	} else {
		/* When it's some other kind of attribute -> always use it! */
	}
	return true;
nope:
	return false;
}


/* Check if "self" provide a private implementation of "name",
 * and make sure that "orig_type" has inherited the operator
 * when that is the case. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasPrivateOperator_in(DeeTypeObject *orig_type,
                              DeeTypeObject *self, Dee_operator_t name) {
	if (DeeType_HasPrivateOperator(self, name))
		return DeeType_InheritOperator(orig_type, name);
	return false;
}

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

INTERN WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self) {
	struct Dee_type_seq_cache *sc;
	if unlikely(!self->tp_seq)
		return NULL;
	sc = self->tp_seq->_tp_seqcache;
	if (sc)
		return sc;
	sc = (struct Dee_type_seq_cache *)Dee_TryCalloc(sizeof(struct Dee_type_seq_cache));
	if likely(sc) {
		if unlikely(!atomic_cmpxch(&self->tp_seq->_tp_seqcache, NULL, sc)) {
			Dee_Free(sc);
			return self->tp_seq->_tp_seqcache;
		}

		/* Don't treat "sc" as a memory leak if it isn't freed (which
		 * will probably be the case when "self" is statically allocated). */
		sc = (struct Dee_type_seq_cache *)Dee_UntrackAlloc(sc);
	}
	return sc;
}


#define Dee_tsc_uslot_fini_function(self) Dee_Decref((self)->d_function)

/* Destroy a lazily allocated sequence operator cache table. */
INTERN NONNULL((1)) void DCALL
Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self) {
	/* Drop function references where they are present. */
	if (self->tsc_seq_getfirst == &DeeSeq_DefaultGetFirstWithCallGetFirstDataFunction ||
	    self->tsc_seq_boundfirst == &DeeSeq_DefaultBoundFirstWithCallGetFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_getfirst_data);
	if (self->tsc_seq_delfirst == &DeeSeq_DefaultDelFirstWithCallDelFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_delfirst_data);
	if (self->tsc_seq_setfirst == &DeeSeq_DefaultSetFirstWithCallSetFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_setfirst_data);
	if (self->tsc_seq_getlast == &DeeSeq_DefaultGetLastWithCallGetLastDataFunction ||
	    self->tsc_seq_boundlast == &DeeSeq_DefaultBoundLastWithCallGetLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_getlast_data);
	if (self->tsc_seq_dellast == &DeeSeq_DefaultDelLastWithCallDelLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_dellast_data);
	if (self->tsc_seq_setlast == &DeeSeq_DefaultSetLastWithCallSetLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_setlast_data);
	if (self->tsc_seq_any == &DeeSeq_DefaultAnyWithCallAnyDataFunction ||
	    self->tsc_seq_any_with_key == &DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSeq ||
	    self->tsc_seq_any_with_key == &DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSetOrMap ||
	    self->tsc_seq_any_with_range == &DeeSeq_DefaultAnyWithRangeWithCallAnyDataFunction ||
	    self->tsc_seq_any_with_range_and_key == &DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_any_data);
	if (self->tsc_seq_all == &DeeSeq_DefaultAllWithCallAllDataFunction ||
	    self->tsc_seq_all_with_key == &DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSeq ||
	    self->tsc_seq_all_with_key == &DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSetOrMap ||
	    self->tsc_seq_all_with_range == &DeeSeq_DefaultAllWithRangeWithCallAllDataFunction ||
	    self->tsc_seq_all_with_range_and_key == &DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_all_data);
	if (self->tsc_seq_parity == &DeeSeq_DefaultParityWithCallParityDataFunction ||
	    self->tsc_seq_parity_with_key == &DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSeq ||
	    self->tsc_seq_parity_with_key == &DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSetOrMap ||
	    self->tsc_seq_parity_with_range == &DeeSeq_DefaultParityWithRangeWithCallParityDataFunction ||
	    self->tsc_seq_parity_with_range_and_key == &DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_parity_data);
	if (self->tsc_seq_reduce == &DeeSeq_DefaultReduceWithCallReduceDataFunction ||
	    self->tsc_seq_reduce_with_init == &DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSeq ||
	    self->tsc_seq_reduce_with_init == &DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSetOrMap ||
	    self->tsc_seq_reduce_with_range == &DeeSeq_DefaultReduceWithRangeWithCallReduceDataFunction ||
	    self->tsc_seq_reduce_with_range_and_init == &DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_reduce_data);
	if (self->tsc_seq_min == &DeeSeq_DefaultMinWithCallMinDataFunction ||
	    self->tsc_seq_min_with_key == &DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSeq ||
	    self->tsc_seq_min_with_key == &DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSetOrMap ||
	    self->tsc_seq_min_with_range == &DeeSeq_DefaultMinWithRangeWithCallMinDataFunction ||
	    self->tsc_seq_min_with_range_and_key == &DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_min_data);
	if (self->tsc_seq_max == &DeeSeq_DefaultMaxWithCallMaxDataFunction ||
	    self->tsc_seq_max_with_key == &DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSeq ||
	    self->tsc_seq_max_with_key == &DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSetOrMap ||
	    self->tsc_seq_max_with_range == &DeeSeq_DefaultMaxWithRangeWithCallMaxDataFunction ||
	    self->tsc_seq_max_with_range_and_key == &DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_max_data);
	if (self->tsc_seq_sum == &DeeSeq_DefaultSumWithCallSumDataFunction ||
	    self->tsc_seq_sum_with_range == &DeeSeq_DefaultSumWithRangeWithCallSumDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_sum_data);
	if (self->tsc_seq_count == &DeeSeq_DefaultCountWithCallCountDataFunction ||
	    self->tsc_seq_count_with_key == &DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSeq ||
	    self->tsc_seq_count_with_key == &DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSetOrMap ||
	    self->tsc_seq_count_with_range == &DeeSeq_DefaultCountWithRangeWithCallCountDataFunction ||
	    self->tsc_seq_count_with_range_and_key == &DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_count_data);
	if (self->tsc_seq_contains == &DeeSeq_DefaultContainsWithCallContainsDataFunction ||
	    self->tsc_seq_contains_with_key == &DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSeq ||
	    self->tsc_seq_contains_with_key == &DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSetOrMap ||
	    self->tsc_seq_contains_with_range == &DeeSeq_DefaultContainsWithRangeWithCallContainsDataFunction ||
	    self->tsc_seq_contains_with_range_and_key == &DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_contains_data);
	if (self->tsc_seq_locate == &DeeSeq_DefaultLocateWithCallLocateDataFunctionForSeq ||
	    self->tsc_seq_locate == &DeeSeq_DefaultLocateWithCallLocateDataFunctionForSetOrMap ||
	    self->tsc_seq_locate_with_range == &DeeSeq_DefaultLocateWithRangeWithCallLocateDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_locate_data);
	if (self->tsc_seq_rlocate_with_range == &DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_rlocate_data);
	if (self->tsc_seq_startswith == &DeeSeq_DefaultStartsWithWithCallStartsWithDataFunction ||
	    self->tsc_seq_startswith_with_key == &DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSeq ||
	    self->tsc_seq_startswith_with_key == &DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSetOrMap ||
	    self->tsc_seq_startswith_with_range == &DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataFunction ||
	    self->tsc_seq_startswith_with_range_and_key == &DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_startswith_data);
	if (self->tsc_seq_endswith == &DeeSeq_DefaultEndsWithWithCallEndsWithDataFunction ||
	    self->tsc_seq_endswith_with_key == &DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSeq ||
	    self->tsc_seq_endswith_with_key == &DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSetOrMap ||
	    self->tsc_seq_endswith_with_range == &DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataFunction ||
	    self->tsc_seq_endswith_with_range_and_key == &DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_endswith_data);
	if (self->tsc_seq_find == &DeeSeq_DefaultFindWithCallFindDataFunction ||
	    self->tsc_seq_find_with_key == &DeeSeq_DefaultFindWithKeyWithCallFindDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_find_data);
	if (self->tsc_seq_rfind == &DeeSeq_DefaultRFindWithCallRFindDataFunction ||
	    self->tsc_seq_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithCallRFindDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_rfind_data);
	if (self->tsc_seq_erase == &DeeSeq_DefaultEraseWithCallEraseDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_erase_data);
	if (self->tsc_seq_insert == &DeeSeq_DefaultInsertWithCallInsertDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_insert_data);
	if (self->tsc_seq_insertall == &DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_insertall_data);
	if (self->tsc_seq_pushfront == &DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_pushfront_data);
	if (self->tsc_seq_append == &DeeSeq_DefaultAppendWithCallAppendDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_append_data);
	if (self->tsc_seq_extend == &DeeSeq_DefaultExtendWithCallExtendDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_extend_data);
	if (self->tsc_seq_xchitem_index == &DeeSeq_DefaultXchItemIndexWithCallXchItemDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_xchitem_data);
	if (self->tsc_seq_clear == &DeeSeq_DefaultClearWithCallClearDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_clear_data);
	if (self->tsc_seq_pop == &DeeSeq_DefaultPopWithCallPopDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_pop_data);
	if (self->tsc_seq_remove == &DeeSeq_DefaultRemoveWithCallRemoveDataFunction ||
	    self->tsc_seq_remove_with_key == &DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_remove_data);
	if (self->tsc_seq_rremove == &DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction ||
	    self->tsc_seq_rremove_with_key == &DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_rremove_data);
	if (self->tsc_seq_removeall == &DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction ||
	    self->tsc_seq_removeall_with_key == &DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_removeall_data);
	if (self->tsc_seq_removeif == &DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_removeif_data);
	if (self->tsc_seq_resize == &DeeSeq_DefaultResizeWithCallResizeDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_resize_data);
	if (self->tsc_seq_fill == &DeeSeq_DefaultFillWithCallFillDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_fill_data);
	if (self->tsc_seq_reverse == &DeeSeq_DefaultReverseWithCallReverseDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_reverse_data);
	if (self->tsc_seq_reversed == &DeeSeq_DefaultReversedWithCallReversedDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_reversed_data);
	if (self->tsc_seq_sort == &DeeSeq_DefaultSortWithCallSortDataFunction ||
	    self->tsc_seq_sort_with_key == &DeeSeq_DefaultSortWithKeyWithCallSortDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_sort_data);
	if (self->tsc_seq_sorted == &DeeSeq_DefaultSortedWithCallSortedDataFunction ||
	    self->tsc_seq_sorted_with_key == &DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_seq_sorted_data);
	if (self->tsc_set_insert == &DeeSet_DefaultInsertWithCallInsertDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_insert_data);
	if (self->tsc_set_remove == &DeeSet_DefaultRemoveWithCallRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_remove_data);
	if (self->tsc_set_unify == &DeeSet_DefaultUnifyWithCallUnifyDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_unify_data);
	if (self->tsc_set_insertall == &DeeSet_DefaultInsertAllWithCallInsertAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_insertall_data);
	if (self->tsc_set_removeall == &DeeSet_DefaultRemoveAllWithCallRemoveAllDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_removeall_data);
	if (self->tsc_set_pop == &DeeSet_DefaultPopWithCallPopDataFunction ||
	    self->tsc_set_pop_with_default == &DeeSet_DefaultPopWithDefaultWithCallPopDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_set_pop_data);
	if (self->tsc_map_setold == &DeeMap_DefaultSetOldWithCallSetOldDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_setold_data);
	if (self->tsc_map_setold_ex == &DeeMap_DefaultSetOldExWithCallSetOldExDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_setold_ex_data);
	if (self->tsc_map_setnew == &DeeMap_DefaultSetNewWithCallSetNewDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_setnew_data);
	if (self->tsc_map_setnew_ex == &DeeMap_DefaultSetNewExWithCallSetNewExDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_setnew_ex_data);
	if (self->tsc_map_setdefault == &DeeMap_DefaultSetDefaultWithCallSetDefaultDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_setdefault_data);
	if (self->tsc_map_update == &DeeMap_DefaultUpdateWithCallUpdateDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_update_data);
	if (self->tsc_map_remove == &DeeMap_DefaultRemoveWithCallRemoveDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_remove_data);
	if (self->tsc_map_removekeys == &DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_removekeys_data);
	if (self->tsc_map_pop == &DeeMap_DefaultPopWithCallPopDataFunction ||
	    self->tsc_map_pop_with_default == &DeeMap_DefaultPopWithDefaultWithCallPopDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_pop_data);
	if (self->tsc_map_popitem == &DeeMap_DefaultPopItemWithCallPopItemDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_popitem_data);
	if (self->tsc_map_keys == &DeeMap_DefaultKeysWithCallKeysDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_keys_data);
	if (self->tsc_map_values == &DeeMap_DefaultValuesWithCallValuesDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_values_data);
	if (self->tsc_map_iterkeys == &DeeMap_DefaultIterKeysWithCallIterKeysDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_iterkeys_data);
	if (self->tsc_map_itervalues == &DeeMap_DefaultIterValuesWithCallIterValuesDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_map_itervalues_data);

	Dee_Free(self);
}


INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_foreach_reverse_t DCALL
DeeType_TryRequireSeqForeachReverse(DeeTypeObject *__restrict self) {
	Dee_mh_seq_foreach_reverse_t result;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_seq_foreach_reverse)
			return sc->tsc_seq_foreach_reverse;
	}
	result = (Dee_mh_seq_foreach_reverse_t)DeeType_GetPrivateMethodHint(self, Dee_TMH_seq_foreach_reverse);
	if (!result) {
		if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
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
		}
	}
	if (result) {
		struct Dee_type_seq_cache *sc;
		sc = DeeType_TryRequireSeqCache(self);
		if likely(sc)
			atomic_write(&sc->tsc_seq_foreach_reverse, result);
	}
	return result;
}

INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_enumerate_index_reverse_t DCALL
DeeType_TryRequireSeqEnumerateIndexReverse(DeeTypeObject *__restrict self) {
	Dee_mh_seq_enumerate_index_reverse_t result;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_seq_enumerate_index_reverse)
			return sc->tsc_seq_enumerate_index_reverse;
	}
	result = (Dee_mh_seq_enumerate_index_reverse_t)DeeType_GetPrivateMethodHint(self, Dee_TMH_seq_enumerate_index_reverse);
	if (!result) {
		if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
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
		}
	}
	if (result) {
		struct Dee_type_seq_cache *sc;
		sc = DeeType_TryRequireSeqCache(self);
		if likely(sc)
			atomic_write(&sc->tsc_seq_enumerate_index_reverse, result);
	}
	return result;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasPrivateSeqEnumerateIndexReverse(DeeTypeObject *orig_type, DeeTypeObject *self) {
	if (DeeType_GetPrivateMethodHint(self, Dee_TMH_seq_enumerate_index_reverse))
		return true;
	return DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
	       DeeType_HasPrivateOperator(self, OPERATOR_GETITEM);
}



/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithError(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithError(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_t DCALL
DeeType_RequireSeqMakeEnumeration_uncached(DeeTypeObject *__restrict self) {
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
					if (!DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
						return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
				}
			} else if (DeeType_HasOperator(self, OPERATOR_GETITEM)) {
				if (!DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index) ||
				    !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index) ||
				    !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
					return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			}
			if (!DeeType_IsDefaultIter(self->tp_seq->tp_iter))
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
		if (seqclass == Dee_SEQCLASS_SEQ) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (seqclass == Dee_SEQCLASS_MAP) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		} else if (seqclass == Dee_SEQCLASS_NONE) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
	}
	return &DeeSeq_DefaultMakeEnumerationWithError;
}

INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_t DCALL
DeeType_RequireSeqMakeEnumeration(DeeTypeObject *__restrict self) {
	Dee_mh_seq_makeenumeration_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_seq_makeenumeration)
			return sc->tsc_seq_makeenumeration;
	}
	result = DeeType_RequireSeqMakeEnumeration_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_seq_makeenumeration, result);
	return result;
}

INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_int_range_t DCALL
DeeType_RequireSeqMakeEnumerationWithIntRange(DeeTypeObject *__restrict self) {
	Dee_mh_seq_makeenumeration_with_int_range_t result;
	Dee_mh_seq_makeenumeration_t base;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_seq_makeenumeration_with_int_range)
			return sc->tsc_seq_makeenumeration_with_int_range;
	}
	base = DeeType_RequireSeqMakeEnumeration(self);
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
		atomic_write(&sc->tsc_seq_makeenumeration_with_int_range, result);
	return result;
}

INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_range_t DCALL
DeeType_RequireSeqMakeEnumerationWithRange(DeeTypeObject *__restrict self) {
	Dee_mh_seq_makeenumeration_with_range_t result;
	Dee_mh_seq_makeenumeration_t base;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_seq_makeenumeration_with_range)
			return sc->tsc_seq_makeenumeration_with_range;
	}
	base = DeeType_RequireSeqMakeEnumeration(self);
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
		atomic_write(&sc->tsc_seq_makeenumeration_with_range, result);
	return result;
}






/* Special sequence functions that have dedicated operators. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Sum)(DeeObject *__restrict self) {
	return DeeSeq_InvokeSum(self);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSeq_Any)(DeeObject *__restrict self) {
	return DeeSeq_InvokeAny(self);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSeq_All)(DeeObject *__restrict self) {
	return DeeSeq_InvokeAll(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Min)(DeeObject *__restrict self) {
	return DeeSeq_InvokeMin(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Max)(DeeObject *__restrict self) {
	return DeeSeq_InvokeMax(self);
}

DECL_END

/* Define sequence operator implementation selectors */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_RequireSeqOperatorBool
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorIter
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSizeOb
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorContains
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRange
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRange
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRange
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorForeach
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorForeachPair
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEnumerate
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorIterKeys
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorBoundItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHasItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSize
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSizeFast
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorBoundItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHasItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryGetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorCompare
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorTryCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorNe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorLo
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorLe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGr
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorGe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorInplaceAdd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSeqOperatorInplaceMul
#include "default-api-require-operator-impl.c.inl"

#define DEFINE_DeeType_RequireSetOperatorIter
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorForeach
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorSize
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorSizeOb
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorTryCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorNe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorLo
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorLe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorGr
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorGe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInv
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorAdd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorSub
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorAnd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorXor
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceAdd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceSub
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceAnd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireSetOperatorInplaceXor
#include "default-api-require-operator-impl.c.inl"

#define DEFINE_DeeType_RequireMapOperatorContains
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEnumerate
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEnumerateIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItem
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemIndex
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemStringHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorTryCompareEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorEq
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorNe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorLo
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorLe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGr
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorGe
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorAdd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorSub
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorAnd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorXor
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceAdd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceSub
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceAnd
#include "default-api-require-operator-impl.c.inl"
#define DEFINE_DeeType_RequireMapOperatorInplaceXor
#include "default-api-require-operator-impl.c.inl"
#endif /* !__INTELLISENSE__ */

/* Define sequence function implementation selectors */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_RequireSeqTryGetFirst
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqGetFirst
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBoundFirst
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqDelFirst
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetFirst
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqTryGetLast
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqGetLast
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBoundLast
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqDelLast
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetLast
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAny
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAnyWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAnyWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAnyWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAll
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAllWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAllWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAllWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqParity
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqParityWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqParityWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqParityWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReduce
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReduceWithInit
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReduceWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReduceWithRangeAndInit
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMin
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMinWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMinWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMinWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMax
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMaxWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMaxWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqMaxWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSum
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSumWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqCount
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqCountWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqCountWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqCountWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqContains
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqContainsWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqContainsWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqContainsWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqLocate
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqLocateWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRLocateWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqStartsWith
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqStartsWithWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqStartsWithWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqEndsWith
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqEndsWithWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqEndsWithWithRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqFind
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqFindWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRFind
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRFindWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqErase
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqInsert
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqInsertAll
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqPushFront
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqAppend
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqExtend
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqXchItemIndex
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqClear
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqPop
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRemove
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRemoveWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRRemove
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRRemoveWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRemoveAll
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRemoveAllWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqRemoveIf
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqResize
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqFill
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReverse
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqReversed
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSort
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSortWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSorted
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqSortedWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBFind
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBFindWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBPosition
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBPositionWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBRange
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSeqBRangeWithKey
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetInsert
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetRemove
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetUnify
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetInsertAll
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetRemoveAll
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetPop
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireSetPopWithDefault
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapSetOld
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapSetOldEx
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapSetNew
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapSetNewEx
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapSetDefault
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapUpdate
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapRemove
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapRemoveKeys
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapPop
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapPopWithDefault
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapPopItem
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapKeys
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapValues
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapIterKeys
#include "default-api-require-method-impl.c.inl"
#define DEFINE_DeeType_RequireMapIterValues
#include "default-api-require-method-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C */
