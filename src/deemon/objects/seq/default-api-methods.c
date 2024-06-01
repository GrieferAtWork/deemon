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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C 1

#include "default-api.h"
/**/

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/overflow.h>

/**/
#include "default-reversed.h"
#include "repeat.h"
#include "sort.h"

/**/
#include "../../runtime/kwlist.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#undef SSIZE_MIN
#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#define DeeType_RequireBool(tp_self)           (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))
#define DeeType_RequireSize(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size) || DeeType_InheritSize(tp_self))
#define DeeType_RequireIter(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeach(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerateIndex(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireGetItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItemIndex(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireGetRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index && (tp_self)->tp_seq->tp_getrange_index_n) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireDelRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index && (tp_self)->tp_seq->tp_delrange_index_n) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireSetRangeIndex(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index && (tp_self)->tp_seq->tp_setrange_index_n) || DeeType_InheritSetRange(tp_self))


PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_any_t DCALL DeeType_SeqCache_RequireAny_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_any_with_key_t DCALL DeeType_SeqCache_RequireAnyWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_any_with_range_t DCALL DeeType_SeqCache_RequireAnyWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_any_with_range_and_key_t DCALL DeeType_SeqCache_RequireAnyWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_all_t DCALL DeeType_SeqCache_RequireAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_all_with_key_t DCALL DeeType_SeqCache_RequireAllWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_all_with_range_t DCALL DeeType_SeqCache_RequireAllWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_all_with_range_and_key_t DCALL DeeType_SeqCache_RequireAllWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_parity_t DCALL DeeType_SeqCache_RequireParity_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_parity_with_key_t DCALL DeeType_SeqCache_RequireParityWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_parity_with_range_t DCALL DeeType_SeqCache_RequireParityWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_parity_with_range_and_key_t DCALL DeeType_SeqCache_RequireParityWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reduce_t DCALL DeeType_SeqCache_RequireReduce_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reduce_with_init_t DCALL DeeType_SeqCache_RequireReduceWithInit_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reduce_with_range_t DCALL DeeType_SeqCache_RequireReduceWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reduce_with_range_and_init_t DCALL DeeType_SeqCache_RequireReduceWithRangeAndInit_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_min_t DCALL DeeType_SeqCache_RequireMin_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_min_with_key_t DCALL DeeType_SeqCache_RequireMinWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_min_with_range_t DCALL DeeType_SeqCache_RequireMinWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_min_with_range_and_key_t DCALL DeeType_SeqCache_RequireMinWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_max_t DCALL DeeType_SeqCache_RequireMax_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_max_with_key_t DCALL DeeType_SeqCache_RequireMaxWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_max_with_range_t DCALL DeeType_SeqCache_RequireMaxWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_max_with_range_and_key_t DCALL DeeType_SeqCache_RequireMaxWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_count_t DCALL DeeType_SeqCache_RequireCount_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_count_with_key_t DCALL DeeType_SeqCache_RequireCountWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_count_with_range_t DCALL DeeType_SeqCache_RequireCountWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_count_with_range_and_key_t DCALL DeeType_SeqCache_RequireCountWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_contains_t DCALL DeeType_SeqCache_RequireContains_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_contains_with_key_t DCALL DeeType_SeqCache_RequireContainsWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_contains_with_range_t DCALL DeeType_SeqCache_RequireContainsWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_contains_with_range_and_key_t DCALL DeeType_SeqCache_RequireContainsWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_locate_t DCALL DeeType_SeqCache_RequireLocate_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_locate_with_key_t DCALL DeeType_SeqCache_RequireLocateWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_locate_with_range_t DCALL DeeType_SeqCache_RequireLocateWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_locate_with_range_and_key_t DCALL DeeType_SeqCache_RequireLocateWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rlocate_with_range_t DCALL DeeType_SeqCache_RequireRLocateWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rlocate_with_range_and_key_t DCALL DeeType_SeqCache_RequireRLocateWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_startswith_t DCALL DeeType_SeqCache_RequireStartsWith_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_startswith_with_key_t DCALL DeeType_SeqCache_RequireStartsWithWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_startswith_with_range_t DCALL DeeType_SeqCache_RequireStartsWithWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_startswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireStartsWithWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_endswith_t DCALL DeeType_SeqCache_RequireEndsWith_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_endswith_with_key_t DCALL DeeType_SeqCache_RequireEndsWithWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_endswith_with_range_t DCALL DeeType_SeqCache_RequireEndsWithWithRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_endswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireEndsWithWithRangeAndKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_find_t DCALL DeeType_SeqCache_RequireFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_find_with_key_t DCALL DeeType_SeqCache_RequireFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rfind_t DCALL DeeType_SeqCache_RequireRFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rfind_with_key_t DCALL DeeType_SeqCache_RequireRFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_bfind_t DCALL DeeType_SeqCache_RequireBFind_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_bfind_with_key_t DCALL DeeType_SeqCache_RequireBFindWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_bposition_t DCALL DeeType_SeqCache_RequireBPosition_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_bposition_with_key_t DCALL DeeType_SeqCache_RequireBPositionWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_brange_t DCALL DeeType_SeqCache_RequireBRange_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_brange_with_key_t DCALL DeeType_SeqCache_RequireBRangeWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_blocate_t DCALL DeeType_SeqCache_RequireBLocate_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_blocate_with_key_t DCALL DeeType_SeqCache_RequireBLocateWithKey_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_t DCALL DeeType_SeqCache_RequireAny_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_key_t DCALL DeeType_SeqCache_RequireAnyWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_range_t DCALL DeeType_SeqCache_RequireAnyWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_range_and_key_t DCALL DeeType_SeqCache_RequireAnyWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_t DCALL DeeType_SeqCache_RequireAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_key_t DCALL DeeType_SeqCache_RequireAllWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_range_t DCALL DeeType_SeqCache_RequireAllWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_range_and_key_t DCALL DeeType_SeqCache_RequireAllWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_t DCALL DeeType_SeqCache_RequireParity_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_key_t DCALL DeeType_SeqCache_RequireParityWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_range_t DCALL DeeType_SeqCache_RequireParityWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_range_and_key_t DCALL DeeType_SeqCache_RequireParityWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_t DCALL DeeType_SeqCache_RequireReduce_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_init_t DCALL DeeType_SeqCache_RequireReduceWithInit_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_range_t DCALL DeeType_SeqCache_RequireReduceWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_range_and_init_t DCALL DeeType_SeqCache_RequireReduceWithRangeAndInit_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_t DCALL DeeType_SeqCache_RequireMin_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_key_t DCALL DeeType_SeqCache_RequireMinWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_range_t DCALL DeeType_SeqCache_RequireMinWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_range_and_key_t DCALL DeeType_SeqCache_RequireMinWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_t DCALL DeeType_SeqCache_RequireMax_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_key_t DCALL DeeType_SeqCache_RequireMaxWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_range_t DCALL DeeType_SeqCache_RequireMaxWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_range_and_key_t DCALL DeeType_SeqCache_RequireMaxWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_t DCALL DeeType_SeqCache_RequireCount_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_key_t DCALL DeeType_SeqCache_RequireCountWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_range_t DCALL DeeType_SeqCache_RequireCountWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_range_and_key_t DCALL DeeType_SeqCache_RequireCountWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_t DCALL DeeType_SeqCache_RequireContains_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_key_t DCALL DeeType_SeqCache_RequireContainsWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_range_t DCALL DeeType_SeqCache_RequireContainsWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_range_and_key_t DCALL DeeType_SeqCache_RequireContainsWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_t DCALL DeeType_SeqCache_RequireLocate_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_key_t DCALL DeeType_SeqCache_RequireLocateWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_range_t DCALL DeeType_SeqCache_RequireLocateWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_range_and_key_t DCALL DeeType_SeqCache_RequireLocateWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rlocate_with_range_t DCALL DeeType_SeqCache_RequireRLocateWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rlocate_with_range_and_key_t DCALL DeeType_SeqCache_RequireRLocateWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_t DCALL DeeType_SeqCache_RequireStartsWith_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_key_t DCALL DeeType_SeqCache_RequireStartsWithWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_range_t DCALL DeeType_SeqCache_RequireStartsWithWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireStartsWithWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_t DCALL DeeType_SeqCache_RequireEndsWith_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_key_t DCALL DeeType_SeqCache_RequireEndsWithWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_range_t DCALL DeeType_SeqCache_RequireEndsWithWithRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireEndsWithWithRangeAndKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_t DCALL DeeType_SeqCache_RequireFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_with_key_t DCALL DeeType_SeqCache_RequireFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_t DCALL DeeType_SeqCache_RequireRFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_with_key_t DCALL DeeType_SeqCache_RequireRFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bfind_t DCALL DeeType_SeqCache_RequireBFind_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bfind_with_key_t DCALL DeeType_SeqCache_RequireBFindWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bposition_t DCALL DeeType_SeqCache_RequireBPosition_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bposition_with_key_t DCALL DeeType_SeqCache_RequireBPositionWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_brange_t DCALL DeeType_SeqCache_RequireBRange_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_brange_with_key_t DCALL DeeType_SeqCache_RequireBRangeWithKey_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_blocate_t DCALL DeeType_SeqCache_RequireBLocate_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_blocate_with_key_t DCALL DeeType_SeqCache_RequireBLocateWithKey_uncached(DeeTypeObject *__restrict self);


/* Mutable sequence functions */
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_seq_vunsupportedf(DeeObject *self, char const *method_format, va_list args) {
	int result;
	DREF DeeObject *message, *error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_printf(&printer, "Sequence type %k is does not support call: ", Dee_TYPE(self)) < 0)
		goto err_printer;
	if unlikely(unicode_printer_vprintf(&printer, method_format, args) < 0)
		goto err_printer;
	message = unicode_printer_pack(&printer);
	if unlikely(!message)
		goto err;
	error = DeeObject_New(&DeeError_SequenceError, 1, &message);
	Dee_Decref_unlikely(message);
	if unlikely(!error)
		goto err;
	result = DeeError_Throw(error);
	Dee_Decref_unlikely(error);
	return result;
err_printer:
	unicode_printer_fini(&printer);
err:
	return -1;
}

PRIVATE ATTR_COLD NONNULL((1)) int
err_seq_unsupportedf(DeeObject *self, char const *method_format, ...) {
	int result;
	va_list args;
	va_start(args, method_format);
	result = err_seq_vunsupportedf(self, method_format, args);
	va_end(args);
	return result;
}


/* Error implementations for `Sequence.enumerate()' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithError(DeeObject *self) {
	err_seq_unsupportedf(self, "enumerate()");
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithIntRangeWithError(DeeObject *self, size_t start, size_t end) {
	err_seq_unsupportedf(self, "enumerate(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultMakeEnumerationWithRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end) {
	err_seq_unsupportedf(self, "enumerate(%r, %r)", start, end);
	return NULL;
}





/* Special sequence functions that have dedicated operators. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Sum)(DeeObject *__restrict self) {
	return new_DeeSeq_Sum(self);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSeq_Any)(DeeObject *__restrict self) {
	return new_DeeSeq_Any(self);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSeq_All)(DeeObject *__restrict self) {
	return new_DeeSeq_All(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Min)(DeeObject *__restrict self) {
	return new_DeeSeq_Min(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeSeq_Max)(DeeObject *__restrict self) {
	return new_DeeSeq_Max(self);
}





/************************************************************************/
/* enumerate()                                                          */
/************************************************************************/
/* Helpers for enumerating a sequence by invoking a given callback. */

struct seq_enumerate_data {
	DeeObject      *sed_cb;     /* [1..1] Enumeration callback */
	DREF DeeObject *sed_result; /* [?..1][valid_if(return == -2)] Enumeration result */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_data *data;
	data    = (struct seq_enumerate_data *)arg;
	args[0] = index;
	args[1] = value;
	result  = DeeObject_Call(data->sed_cb, value ? 2 : 1, args);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sed_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
seq_enumerate_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_data *data;
	data    = (struct seq_enumerate_data *)arg;
	args[0] = DeeInt_NewSize(index);
	if unlikely(!args[0])
		goto err;
	args[1] = value;
	result  = DeeObject_Call(data->sed_cb, value ? 2 : 1, args);
	Dee_Decref(args[0]);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sed_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Enumerate(DeeObject *self, DeeObject *cb) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_Enumerate(self, &seq_enumerate_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

INTERN NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_EnumerateWithIntRange(DeeObject *self, DeeObject *cb, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb    = cb;
	foreach_status = DeeObject_EnumerateIndex(self, &seq_enumerate_index_cb, &data, start, end);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

struct seq_enumerate_with_filter_data {
	DeeObject      *sedwf_cb;     /* [1..1] Enumeration callback */
	DREF DeeObject *sedwf_result; /* [?..1][valid_if(return == -2)] Enumeration result */
	DeeObject      *sedwf_start;  /* [1..1] Filter start */
	DeeObject      *sedwf_end;    /* [1..1] Filter end */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_enumerate_with_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	int temp;
	DREF DeeObject *result;
	DeeObject *args[2];
	struct seq_enumerate_with_filter_data *data;
	data = (struct seq_enumerate_with_filter_data *)arg;
	/* if (data->sedwf_start <= index && data->sedwf_end > index) ... */
	temp = DeeObject_CmpLeAsBool(data->sedwf_start, index);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0;
	temp = DeeObject_CmpGrAsBool(data->sedwf_end, index);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0;
	args[0] = index;
	args[1] = value;
	result  = DeeObject_Call(data->sedwf_cb, value ? 2 : 1, args);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(Dee_None);
		return 0;
	}
	data->sedwf_result = result;
	return -2; /* Stop enumeration! */
err:
	return -1;
}

INTERN NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeSeq_EnumerateWithRange(DeeObject *self, DeeObject *cb, DeeObject *start, DeeObject *end) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_with_filter_data data;
	data.sedwf_cb    = cb;
	data.sedwf_start = start;
	data.sedwf_end   = end;
	foreach_status = DeeObject_Enumerate(self, &seq_enumerate_with_filter_cb, &data);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sedwf_result;
	return_none;
err:
	return NULL;
}





/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */

/************************************************************************/
/* any()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_any_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp > 0)
		temp = -2;
	return temp;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_any_foreach_with_key_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp > 0)
		temp = -2;
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_any_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_any_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_any_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_any_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAnyWithForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_any_foreach_cb, NULL);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAnyWithKeyWithForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_any_foreach_with_key_cb, key);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAnyWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_any_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultAnyWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_any_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}






/************************************************************************/
/* all()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_all_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp == 0)
		temp = -2;
	return temp;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_all_foreach_with_key_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp == 0)
		temp = -2;
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_all_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_all_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_all_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_all_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAllWithForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_all_foreach_cb, NULL);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAllWithKeyWithForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_all_foreach_with_key_cb, key);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultAllWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_all_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultAllWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_all_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}






/************************************************************************/
/* parity()                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_parity_foreach_cb(void *arg, DeeObject *item) {
	(void)arg;
	return (Dee_ssize_t)DeeObject_Bool(item);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_parity_foreach_with_key_cb(void *arg, DeeObject *item) {
	(void)arg;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	return (Dee_ssize_t)DeeObject_BoolInherited(item);
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_parity_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_parity_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_parity_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_parity_foreach_with_key_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithForeach(DeeObject *self) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeObject_Foreach(self, &default_seq_parity_foreach_cb, NULL);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultParityWithKeyWithForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeObject_Foreach(self, &default_seq_parity_foreach_with_key_cb, key);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultParityWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_parity_enumerate_cb, NULL, start, end);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultParityWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_parity_enumerate_with_key_cb, key, start, end);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}






/************************************************************************/
/* reduce()                                                             */
/************************************************************************/

struct default_seq_reduce_data {
	DeeObject      *gsr_combine; /* [1..1] Combinatory predicate (invoke as `gsr_combine(gsr_init, item)') */
	DREF DeeObject *gsr_result;  /* [0..1] Current reduction result, or NULL if no init given and at first item. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_reduce_foreach_with_init_cb(void *arg, DeeObject *item) {
	DeeObject *args[2];
	DREF DeeObject *reduced;
	struct default_seq_reduce_data *data;
	data    = (struct default_seq_reduce_data *)arg;
	args[0] = data->gsr_result;
	args[1] = item;
	reduced = DeeObject_Call(data->gsr_combine, 2, args);
	if unlikely(!reduced)
		goto err;
	Dee_Decref(data->gsr_result);
	data->gsr_result = reduced;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_reduce_foreach_cb(void *arg, DeeObject *item) {
	struct default_seq_reduce_data *data;
	data = (struct default_seq_reduce_data *)arg;
	if (data->gsr_result)
		return default_seq_reduce_foreach_with_init_cb(arg, item);
	data->gsr_result = item;
	Dee_Incref(item);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_reduce_enumerate_with_init_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return default_seq_reduce_foreach_with_init_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_reduce_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return default_seq_reduce_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithForeach(DeeObject *self, DeeObject *combine) {
	Dee_ssize_t foreach_status;
	struct default_seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = DeeObject_Foreach(self, &default_seq_reduce_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithInitWithForeach(DeeObject *self, DeeObject *combine, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct default_seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = DeeObject_Foreach(self, &default_seq_reduce_foreach_with_init_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_reduce_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		err_empty_sequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultReduceWithRangeAndInitWithEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct default_seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_reduce_enumerate_with_init_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}






/************************************************************************/
/* min()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_min_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp == 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}

struct default_seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_min_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct default_seq_minmax_with_key_data *data;
	data = (struct default_seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp > 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_min_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_min_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_min_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_min_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithForeach(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_min_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithKeyWithForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_Foreach(self, &default_seq_min_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_min_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultMinWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_min_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}






/************************************************************************/
/* max()                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_max_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp > 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_max_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct default_seq_minmax_with_key_data *data;
	data = (struct default_seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp == 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_max_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_max_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_max_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_max_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithForeach(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_max_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithKeyWithForeach(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_Foreach(self, &default_seq_max_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if (!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_max_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultMaxWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_max_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}






/************************************************************************/
/* sum()                                                                */
/************************************************************************/

struct default_seq_sum_data {
#define GENERIC_SEQ_SUM_MODE_FIRST  0 /* Mode not yet determined (expecting first item) */
#define GENERIC_SEQ_SUM_MODE_SECOND 1 /* Always comes after `GENERIC_SEQ_SUM_MODE_FIRST' and selects which mode to use */
#define GENERIC_SEQ_SUM_MODE_OBJECT 2 /* Generic object sum mode (using "operator +") */
#define GENERIC_SEQ_SUM_MODE_STRING 3 /* Use a unicode printer */
#define GENERIC_SEQ_SUM_MODE_BYTES  4 /* Use a bytes printer */
#define GENERIC_SEQ_SUM_MODE_INT    5 /* Got a single item, which had type "int" */
#ifndef CONFIG_NO_FPU
#define GENERIC_SEQ_SUM_MODE_FLOAT  6 /* Got a single item, which had type "float" */
#endif /* !CONFIG_NO_FPU */
	uintptr_t gss_mode; /* Sum-mode (one of `GENERIC_SEQ_SUM_MODE_*') */
	union {
		DREF DeeObject        *v_object; /* GENERIC_SEQ_SUM_MODE_SECOND, GENERIC_SEQ_SUM_MODE_OBJECT */
		struct unicode_printer v_string; /* GENERIC_SEQ_SUM_MODE_STRING */
		struct bytes_printer   v_bytes;  /* GENERIC_SEQ_SUM_MODE_BYTES */
		Dee_ssize_t            v_int;    /* GENERIC_SEQ_SUM_MODE_INT2 */
#ifndef CONFIG_NO_FPU
		double                 v_float;  /* GENERIC_SEQ_SUM_MODE_FLOAT2 */
#endif /* !CONFIG_NO_FPU */
		/* TODO: Special optimization for Sequence concat */
		/* TODO: Special optimization for Tuple */
		/* TODO: Special optimization for List */
	} gss_value;
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_sum_data_pack(struct default_seq_sum_data *__restrict self) {
	switch (self->gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
		return_none;
	case GENERIC_SEQ_SUM_MODE_SECOND:
	case GENERIC_SEQ_SUM_MODE_OBJECT:
		return self->gss_value.v_object;
	case GENERIC_SEQ_SUM_MODE_STRING:
		return unicode_printer_pack(&self->gss_value.v_string);
	case GENERIC_SEQ_SUM_MODE_BYTES:
		return bytes_printer_pack(&self->gss_value.v_bytes);
	case GENERIC_SEQ_SUM_MODE_INT:
		return DeeInt_NewSSize(self->gss_value.v_int);
#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT:
		return DeeFloat_New(self->gss_value.v_float);
#endif /* !CONFIG_NO_FPU */
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}

PRIVATE NONNULL((1)) void DCALL
default_seq_sum_data_fini(struct default_seq_sum_data *__restrict self) {
	switch (self->gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
	case GENERIC_SEQ_SUM_MODE_INT:
#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT:
#endif /* !CONFIG_NO_FPU */
		break;
	case GENERIC_SEQ_SUM_MODE_OBJECT:
	case GENERIC_SEQ_SUM_MODE_SECOND:
		Dee_Decref(self->gss_value.v_object);
		break;
	case GENERIC_SEQ_SUM_MODE_STRING:
		unicode_printer_fini(&self->gss_value.v_string);
		break;
	case GENERIC_SEQ_SUM_MODE_BYTES:
		bytes_printer_fini(&self->gss_value.v_bytes);
		break;
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_sum_foreach_cb(void *arg, DeeObject *item) {
	struct default_seq_sum_data *data;
	data = (struct default_seq_sum_data *)arg;
	switch (data->gss_mode) {

	case GENERIC_SEQ_SUM_MODE_FIRST:
		Dee_Incref(item);
		data->gss_value.v_object = item;
		data->gss_mode = GENERIC_SEQ_SUM_MODE_SECOND;
		break;

	case GENERIC_SEQ_SUM_MODE_SECOND: {
		DeeTypeObject *tp_elem;
		DREF DeeObject *first = data->gss_value.v_object;
		tp_elem = Dee_TYPE(first);
		if (tp_elem == &DeeString_Type) {
			Dee_ssize_t temp;
			data->gss_mode = GENERIC_SEQ_SUM_MODE_STRING;
			unicode_printer_init(&data->gss_value.v_string);
			temp = unicode_printer_printstring(&data->gss_value.v_string, first);
			Dee_Decref(first);
			if unlikely(temp < 0)
				return temp;
			goto do_print_string;
		} else if (tp_elem == &DeeBytes_Type) {
			Dee_ssize_t temp;
			data->gss_mode = GENERIC_SEQ_SUM_MODE_BYTES;
			bytes_printer_init(&data->gss_value.v_bytes);
			temp = Dee_bytes_printer_printbytes(&data->gss_value.v_bytes, first);
			Dee_Decref(first);
			if unlikely(temp < 0)
				return temp;
			goto do_print_bytes;
		} else if (tp_elem == &DeeInt_Type) {
			Dee_ssize_t a, b;
			if (Dee_TYPE(item) != &DeeInt_Type)
				goto generic_second;
			if unlikely(!DeeInt_TryAsSSize(first, &a))
				goto generic_second;
			if unlikely(!DeeInt_TryAsSSize(item, &b))
				goto generic_second;
			if unlikely(OVERFLOW_SADD(a, b, &a))
				goto generic_second;
			Dee_Decref(first);
			data->gss_mode = GENERIC_SEQ_SUM_MODE_INT;
			data->gss_value.v_int = a;
			break;
#ifndef CONFIG_NO_FPU
		} else if (tp_elem == &DeeFloat_Type) {
			double total;
			if (DeeObject_AsDouble(item, &total))
				goto err;
			total += DeeFloat_VALUE(first);
			Dee_Decref(first);
			data->gss_mode = GENERIC_SEQ_SUM_MODE_FLOAT;
			data->gss_value.v_float = total;
			break;
#endif /* !CONFIG_NO_FPU */
		} else {
			/* ... */
		}
generic_second:
		item = DeeObject_Add(first, item);
		if unlikely(!item)
			goto err;
		Dee_Decref(first);
		data->gss_value.v_object = item;
		data->gss_mode = GENERIC_SEQ_SUM_MODE_OBJECT;
	}	break;

	case GENERIC_SEQ_SUM_MODE_OBJECT: {
		DREF DeeObject *result;
do_handle_object:
		result = DeeObject_Add(data->gss_value.v_object, item);
		if unlikely(!result)
			goto err;
		Dee_Decref(data->gss_value.v_object);
		data->gss_value.v_object = result;
	}	break;

	case GENERIC_SEQ_SUM_MODE_STRING:
do_print_string:
		return unicode_printer_printobject(&data->gss_value.v_string, item);

	case GENERIC_SEQ_SUM_MODE_BYTES:
do_print_bytes:
		return bytes_printer_printobject(&data->gss_value.v_bytes, item);

	case GENERIC_SEQ_SUM_MODE_INT: {
		Dee_ssize_t total;
		if (!DeeInt_Check(item))
			goto switch_to_handle_object_from_int2;
		if unlikely(!DeeInt_TryAsSSize(item, &total))
			goto switch_to_handle_object_from_int2;
		if unlikely(OVERFLOW_SADD(data->gss_value.v_int, total, &total))
			goto switch_to_handle_object_from_int2;
		data->gss_value.v_int = total;
	}	break;

#ifndef CONFIG_NO_FPU
	case GENERIC_SEQ_SUM_MODE_FLOAT: {
		double total;
		if unlikely(!DeeObject_AsDouble(item, &total))
			goto err;
		data->gss_value.v_float += total;
	}	break;
#endif /* !CONFIG_NO_FPU */

	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
switch_to_handle_object_from_int2:
	data->gss_value.v_object = DeeInt_NewSSize(data->gss_value.v_int);
	if unlikely(!data->gss_value.v_object)
		goto err;
	data->gss_mode = GENERIC_SEQ_SUM_MODE_OBJECT;
	goto do_handle_object;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_sum_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_sum_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSumWithForeach(DeeObject *self) {
	Dee_ssize_t foreach_status;
	struct default_seq_sum_data data;
	data.gss_mode = GENERIC_SEQ_SUM_MODE_FIRST;
	foreach_status = DeeObject_Foreach(self, &default_seq_sum_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	return default_seq_sum_data_pack(&data);
err:
	default_seq_sum_data_fini(&data);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSumWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_seq_sum_data data;
	data.gss_mode = GENERIC_SEQ_SUM_MODE_FIRST;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_sum_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	return default_seq_sum_data_pack(&data);
err:
	default_seq_sum_data_fini(&data);
	return NULL;
}






/************************************************************************/
/* count()                                                              */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_count_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

struct default_seq_count_with_key_data {
	DeeObject *gscwk_key;   /* [1..1] Key predicate */
	DeeObject *gscwk_kelem; /* [1..1] Keyed search element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_count_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct default_seq_count_with_key_data *data;
	data = (struct default_seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_count_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_count_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_count_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_count_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithForeach(DeeObject *self, DeeObject *item) {
	return (size_t)DeeObject_Foreach(self, &default_seq_count_foreach_cb, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
DeeSeq_DefaultCountWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_Foreach(self, &default_seq_count_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultCountWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	return (size_t)DeeObject_EnumerateIndex(self, &default_seq_count_enumerate_cb, item, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultCountWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_count_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}






/************************************************************************/
/* contains()                                                           */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_contains_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_contains_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct default_seq_count_with_key_data *data;
	data = (struct default_seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_contains_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_contains_foreach_cb(arg, item);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_contains_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_contains_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithForeach(DeeObject *self, DeeObject *item) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_contains_foreach_cb, item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultContainsWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_Foreach(self, &default_seq_contains_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultContainsWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_contains_enumerate_cb, item, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultContainsWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_contains_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
err:
	return -1;
}






/************************************************************************/
/* locate()                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_locate_foreach_cb(void *arg, DeeObject *item) {
	DeeObject *elem_to_locate = *(DeeObject **)arg;
	int temp = DeeObject_TryCompareEq(elem_to_locate, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
}

struct default_seq_locate_with_key_data {
	DeeObject *gslwk_kelem; /* [1..1] Keyed search element. */
	DeeObject *gslwk_key;   /* [1..1][in] Search key predicate
	                         * [1..1][out:DREF] Located element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_locate_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct default_seq_locate_with_key_data *data;
	data = (struct default_seq_locate_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gslwk_kelem, item, data->gslwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		data->gslwk_key = item;
		return -2;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_locate_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_locate_foreach_cb(arg, item);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_locate_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_locate_with_key_foreach_cb(arg, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithForeach(DeeObject *self, DeeObject *item) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &default_seq_locate_foreach_cb, &item);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	foreach_status = DeeObject_Foreach(self, &default_seq_locate_with_key_foreach_cb, &data);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_locate_enumerate_cb, &item, start, end);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultLocateWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_locate_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}






/************************************************************************/
/* rlocate()                                                            */
/************************************************************************/
struct default_seq_rlocate_with_foreach_data {
	DeeObject      *gsrlwf_elem;   /* [1..1] Element to search for. */
	DREF DeeObject *gsrlwf_result; /* [0..1] Most recent match. */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_rlocate_with_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	int temp;
	struct default_seq_rlocate_with_foreach_data *data;
	(void)index;
	if (!item)
		return 0;
	data = (struct default_seq_rlocate_with_foreach_data *)arg;
	temp = DeeObject_TryCompareEq(data->gsrlwf_elem, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		Dee_XDecref(data->gsrlwf_result);
		data->gsrlwf_result = item;
	}
	return 0;
err:
	return -1;
}

struct default_seq_rlocate_with_key_and_foreach_data {
	DeeObject      *gsrlwkf_kelem;  /* [1..1] Keyed element to search for. */
	DREF DeeObject *gsrlwkf_result; /* [0..1] Most recent match. */
	DeeObject      *gsrlwkf_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq_rlocate_with_key_and_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	int temp;
	struct default_seq_rlocate_with_key_and_foreach_data *data;
	(void)index;
	if (!item)
		return 0;
	data = (struct default_seq_rlocate_with_key_and_foreach_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gsrlwkf_kelem, item, data->gsrlwkf_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(item);
		Dee_XDecref(data->gsrlwkf_result);
		data->gsrlwkf_result = item;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	Dee_tsc_enumerate_index_reverse_t op;
	op = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(op);
	foreach_status = (*op)(self, &default_seq_locate_enumerate_cb, &item, start, end);
	if likely(foreach_status == -2)
		return item;
	if (foreach_status == 0)
		err_item_not_found(self, item);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_seq_rlocate_with_foreach_data data;
	data.gsrlwf_elem   = item;
	data.gsrlwf_result = NULL;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_rlocate_with_enumerate_cb, &data, start, end);
	if likely(foreach_status == 0) {
		if (data.gsrlwf_result)
			return data.gsrlwf_result;
		err_item_not_found(self, item);
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeAndKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	Dee_tsc_enumerate_index_reverse_t op;
	struct default_seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	op = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(op);
	foreach_status = (*op)(self, &default_seq_locate_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultRLocateWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_seq_rlocate_with_key_and_foreach_data data;
	data.gsrlwkf_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrlwkf_kelem)
		goto err;
	data.gsrlwkf_key    = key;
	data.gsrlwkf_result = NULL;
	foreach_status = DeeObject_EnumerateIndex(self, &default_seq_rlocate_with_key_and_enumerate_cb, &data, start, end);
	Dee_Decref(data.gsrlwkf_kelem);
	if likely(foreach_status == 0) {
		if (data.gsrlwkf_result)
			return data.gsrlwkf_result;
		err_item_not_found(self, item);
	}
err:
	return NULL;
}






/************************************************************************/
/* startswith()                                                         */
/************************************************************************/

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "default-api.c" */
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *item);

/* Returns ITER_DONE if item doesn't exist. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_trygetfirst(DeeObject *self) {
	DREF DeeObject *result;
	Dee_tsc_getfirst_t getfirst;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	getfirst = DeeType_SeqCache_RequireGetFirst(tp_self);
	if (getfirst == &DeeSeq_DefaultGetFirstWithGetItemIndex) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
	} else if (getfirst == &DeeSeq_DefaultGetFirstWithGetItem) {
		int temp;
		DREF DeeObject *sizeob;
		sizeob = (*tp_self->tp_seq->tp_sizeob)(self);
		if unlikely(sizeob == NULL)
			goto err;
		temp = DeeObject_Bool(sizeob);
		Dee_Decref(sizeob);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			return ITER_DONE;
	} else if (getfirst == &DeeSeq_DefaultGetFirstWithForeachDefault) {
		Dee_ssize_t foreach_status;
		foreach_status = DeeObject_Foreach(self, &seq_default_getfirst_with_foreach_cb, &result);
		if likely(foreach_status == -2)
			return result;
		if (foreach_status == 0)
			return ITER_DONE;
		goto err;
	}
	result = (*getfirst)(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultStartsWithWithTSCFirst(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *first = default_seq_trygetfirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, first);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultStartsWithWithKeyWithTSCFirst(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *first = default_seq_trygetfirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(item, first, key);
	Dee_Decref(item);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultStartsWithWithRangeWithTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = DeeObject_TryGetItemIndex(self, start);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultStartsWithWithRangeAndKeyWithTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return 0;
	selfitem = DeeObject_TryGetItemIndex(self, start);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_selfitem;
	result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(item);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_selfitem:
	Dee_Decref(selfitem);
err:
	return -1;
}






/************************************************************************/
/* endswith()                                                           */
/************************************************************************/

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "default-api.c" */
seq_default_last_with_foreach_cb(void *arg, DeeObject *item);

/* Returns ITER_DONE if item doesn't exist. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_trygetlast(DeeObject *self) {
	DREF DeeObject *result;
	Dee_tsc_getlast_t getlast;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	getlast = DeeType_SeqCache_RequireGetLast(tp_self);
	if (getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
		result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, size - 1);
		if (!result)
			return ITER_DONE;
		return result;
	} else if (getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndex) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
		result = (*tp_self->tp_seq->tp_getitem_index)(self, size - 1);
		if (result)
			return result;
		if (DeeError_Catch(&DeeError_UnboundItem))
			return ITER_DONE;
		goto err;
	} else if (getlast == &DeeSeq_DefaultGetLastWithSizeObAndGetItem) {
		int temp;
		DREF DeeObject *sizeob;
		struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
		sizeob = (*seq->tp_sizeob)(self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_Bool(sizeob);
		if unlikely(temp < 0)
			goto err_sizeob;
		if unlikely(!temp) {
			Dee_Decref(sizeob);
			return ITER_DONE;
		}
		if unlikely(DeeObject_Dec(&sizeob))
			goto err_sizeob;
		result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
		Dee_Decref(sizeob);
		if (result)
			return result;
		if (DeeError_Catch(&DeeError_UnboundItem))
			return ITER_DONE;
		goto err;
err_sizeob:
		Dee_Decref(sizeob);
		goto err;
	} else if (getlast == &DeeSeq_DefaultGetLastWithForeachDefault) {
		Dee_ssize_t foreach_status;
		Dee_Incref(Dee_None);
		result = Dee_None;
		foreach_status = DeeObject_Foreach(self, &seq_default_last_with_foreach_cb, &result);
		if likely(foreach_status > 0)
			return result;
		Dee_Decref_unlikely(result);
		if (foreach_status == 0)
			return ITER_DONE;
		goto err;
	}
	result = (*getlast)(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultEndsWithWithTSCLast(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeObject *last = default_seq_trygetlast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, last);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeSeq_DefaultEndsWithWithKeyWithTSCLast(DeeObject *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *last = default_seq_trygetlast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(item, last, key);
	Dee_Decref(item);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultEndsWithWithRangeWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = DeeObject_Size(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeObject_TryGetItemIndex(self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultEndsWithWithRangeAndKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = DeeObject_Size(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeObject_TryGetItemIndex(self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_selfitem;
	result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(item);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_selfitem:
	Dee_Decref(selfitem);
err:
	return -1;
}






/************************************************************************/
/* find()                                                               */
/************************************************************************/
union default_seq_find_data {
	DeeObject *gsfd_elem;  /* [in][1..1] Element to search for */
	size_t     gsfd_index; /* [out] Located index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_find_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	union default_seq_find_data *data;
	data = (union default_seq_find_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsfd_elem, value);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultFindWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	union default_seq_find_data data;
	data.gsfd_elem = item;
	status = DeeSeq_EnumerateIndex(self, &default_seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* find() (with key)                                                    */
/************************************************************************/
struct default_seq_find_with_key_data {
	union default_seq_find_data gsfwk_base; /* Base find data */
	DeeObject                  *gsfwk_key;  /* Find element key */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_find_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_find_with_key_data *data;
	data = (struct default_seq_find_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareKeyEq(data->gsfwk_base.gsfd_elem, value, data->gsfwk_key);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfwk_base.gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultFindWithKeyWithEnumerateIndex(DeeObject *self, DeeObject *item,
                                            size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct default_seq_find_with_key_data data;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	status = DeeSeq_EnumerateIndex(self, &default_seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* rfind()                                                              */
/************************************************************************/
struct default_seq_rfind_data {
	DeeObject *gsrfd_elem;   /* [1..1] The element to search for */
	size_t     gsrfd_result; /* The last-matched index. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_rfind_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_rfind_data *data;
	data = (struct default_seq_rfind_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfd_elem, value);
	if (cmp == 0)
		data->gsrfd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	union default_seq_find_data data;
	Dee_tsc_enumerate_index_reverse_t renum;
	data.gsfd_elem = item;
	renum = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(renum);
	status = (*renum)(self, &default_seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRFindWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	struct default_seq_rfind_data data;
	data.gsrfd_elem   = item;
	data.gsrfd_result = (size_t)-1;
	status = DeeSeq_EnumerateIndex(self, &default_seq_rfind_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfd_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* rfind() (with key)                                                   */
/************************************************************************/
struct default_seq_rfind_with_key_data {
	DeeObject *gsrfwkd_kelem;   /* [1..1] The element to search for */
	size_t     gsrfwkd_result; /* The last-matched index. */
	DeeObject *gsrfwkd_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_rfind_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_rfind_with_key_data *data;
	data = (struct default_seq_rfind_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfwkd_kelem, value);
	if (cmp == 0)
		data->gsrfwkd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item,
                                                       size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct default_seq_find_with_key_data data;
	Dee_tsc_enumerate_index_reverse_t renum;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	renum = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(renum);
	status = (*renum)(self, &default_seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultRFindWithKeyWithEnumerateIndex(DeeObject *self, DeeObject *item,
                                             size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct default_seq_rfind_with_key_data data;
	data.gsrfwkd_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrfwkd_kelem)
		goto err;
	data.gsrfwkd_result = (size_t)-1;
	status = DeeSeq_EnumerateIndex(self, &default_seq_rfind_with_key_cb, &data, start, end);
	Dee_Decref(data.gsrfwkd_kelem);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfwkd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfwkd_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* erase()                                                              */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t index, size_t count) {
	struct type_seq *seq;
	size_t end_index;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	seq = Dee_TYPE(self)->tp_seq;
	return (*seq->tp_delrange_index)(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index);
err_overflow:
	return err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithPop(DeeObject *self, size_t index, size_t count) {
	size_t end_index;
	Dee_tsc_pop_t tsc_pop;
	if unlikely(OVERFLOW_UADD(index, count, &end_index))
		goto err_overflow;
	tsc_pop = DeeType_SeqCache_RequirePop(Dee_TYPE(self));
	while (end_index > index) {
		--end_index;
		if unlikely((*tsc_pop)(self, (Dee_ssize_t)end_index))
			goto err;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return 0;
err_overflow:
	err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultEraseWithError(DeeObject *self, size_t index, size_t count) {
	return err_seq_unsupportedf(self, "erase(%" PRFuSIZ ", %" PRFuSIZ ")", index, count);
}






/************************************************************************/
/* insert()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithTSCInsertAll(DeeObject *self, size_t index, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = (*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, index, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item) {
	return err_seq_unsupportedf(self, "insert(%" PRFuSIZ ", %r)", index, item);
}






/************************************************************************/
/* insertall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items) {
	size_t end_index;
	size_t items_size = DeeObject_Size(items);
	if unlikely(items_size == (size_t)-1)
		goto err;
	if unlikely(OVERFLOW_UADD(index, items_size, &end_index))
		goto err_overflow;
	if unlikely(end_index > SSIZE_MAX)
		goto err_overflow;
	return (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)index, (Dee_ssize_t)end_index, items);
err_overflow:
	err_integer_overflow_i((sizeof(size_t) * 8) - 1, true);
err:
	return -1;
}

struct default_insertall_with_foreach_insert_data {
	Dee_tsc_insert_t diawfid_insert; /* [1..1] Insert callback */
	DeeObject       *diawfid_self;   /* [1..1] The sequence to insert into */
	size_t           diawfid_index;  /* Next index for insertion */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
default_insertall_with_foreach_insert_cb(void *arg, DeeObject *item) {
	struct default_insertall_with_foreach_insert_data *data;
	data = (struct default_insertall_with_foreach_insert_data *)arg;
	return (Dee_ssize_t)(*data->diawfid_insert)(data->diawfid_self, data->diawfid_index++, item);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithTSCInsertForeach(DeeObject *self, size_t index, DeeObject *items) {
	struct default_insertall_with_foreach_insert_data data;
	data.diawfid_self   = self;
	data.diawfid_index  = index;
	data.diawfid_insert = DeeType_SeqCache_RequireInsert(Dee_TYPE(self));
	return (int)DeeObject_Foreach(items, &default_insertall_with_foreach_insert_cb, &data);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items) {
	return err_seq_unsupportedf(self, "insertall(%" PRFuSIZ ", %r)", index, items);
}






/************************************************************************/
/* pushfront()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultPushFrontWithTSCInsert(DeeObject *self, DeeObject *item) {
	return (*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, 0, item);
}






/************************************************************************/
/* append()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithTSCExtend(DeeObject *self, DeeObject *item) {
	int result;
	DREF DeeTupleObject *elem_tuple;
	elem_tuple = DeeTuple_NewUninitialized(1);
	if unlikely(!elem_tuple)
		goto err;
	DeeTuple_SET(elem_tuple, 0, item);
	result = (*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, (DeeObject *)elem_tuple);
	DeeTuple_DecrefSymbolic((DeeObject *)elem_tuple);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithSizeAndTSCInsert(DeeObject *self, DeeObject *item) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return (*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, selfsize, item);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item) {
	return err_seq_unsupportedf(self, "append(%r)", item);
}






/************************************************************************/
/* extend()                                                             */
/************************************************************************/
struct default_extend_with_foreach_append_data {
	Dee_tsc_append_t dewfad_append; /* [1..1] Append callback */
	DeeObject       *dewfad_self;   /* [1..1] The sequence to append to */
};

PRIVATE WUNUSED_T NONNULL_T((2)) Dee_ssize_t DCALL
default_extend_with_foreach_append_cb(void *arg, DeeObject *item) {
	struct default_extend_with_foreach_append_data *data;
	data = (struct default_extend_with_foreach_append_data *)arg;
	return (Dee_ssize_t)(*data->dewfad_append)(data->dewfad_self, item);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithTSCAppendForeach(DeeObject *self, DeeObject *items) {
	struct default_extend_with_foreach_append_data data;
	data.dewfad_self   = self;
	data.dewfad_append = DeeType_SeqCache_RequireAppend(Dee_TYPE(self));
	return (int)DeeObject_Foreach(items, &default_extend_with_foreach_append_cb, &data);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithSizeAndTSCInsertAll(DeeObject *self, DeeObject *items) {
	size_t selfsize;
	selfsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	return (*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, selfsize, items);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultExtendWithError(DeeObject *self, DeeObject *items) {
	return err_seq_unsupportedf(self, "extend(%r)", items);
}






/************************************************************************/
/* xchitem()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex(DeeObject *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	result = (*seq->tp_getitem_index)(self, index);
	if likely(result) {
		if unlikely((*seq->tp_setitem_index)(self, index, value))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeSeq_DefaultXchItemIndexWithError(DeeObject *self, size_t index, DeeObject *value) {
	err_seq_unsupportedf(self, "xchitem(%" PRFuSIZ ", %r)", index, value);
	return NULL;
}






/************************************************************************/
/* clear()                                                              */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithDelRangeIndexN(DeeObject *self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delrange_index_n)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithSetRangeIndexN(DeeObject *self) {
	return (*Dee_TYPE(self)->tp_seq->tp_setrange_index_n)(self, 0, Dee_EmptySeq);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithTSCErase(DeeObject *self) {
	return (*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, 0, (size_t)-1);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultClearWithError(DeeObject *self) {
	return err_seq_unsupportedf(self, "clear()");
}






/************************************************************************/
/* pop()                                                                */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithSizeAndGetItemIndexAndTSCErase(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *result;
	if (index < 0) {
		size_t selfsize;
		selfsize = (*seq->tp_size)(self);
		if unlikely(selfsize == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, selfsize);
	}
	result = (*seq->tp_getitem_index)(self, used_index);
	if likely(result) {
		if unlikely((*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, index, 1))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index) {
	size_t used_index = (size_t)index;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *result;
	if (index < 0) {
		size_t selfsize;
		selfsize = (*seq->tp_size)(self);
		if unlikely(selfsize == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, selfsize);
	}
	result = (*seq->tp_getitem_index)(self, used_index);
	if likely(result) {
		if unlikely((*seq->tp_delitem_index)(self, index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index) {
	err_seq_unsupportedf(self, "pop(" PRFdSIZ ")", index);
	return NULL;
}






/************************************************************************/
/* remove()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCRemoveAll(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end) {
	size_t result;
	result = (*DeeType_SeqCache_RequireRemoveAll(Dee_TYPE(self)))(self, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *srwrip_item; /* [1..1][const] Item to remove */
} SeqRemoveWithRemoveIfPredicate;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *srwripwk_item; /* [1..1][const] Keyed item to remove */
	DREF DeeObject *srwripwk_key;  /* [1..1][const] Key to use during compare */
} SeqRemoveWithRemoveIfPredicateWithKey;

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwrip_init(SeqRemoveWithRemoveIfPredicate *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &self->srwrip_item))
		goto err;
	Dee_Incref(self->srwrip_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
srwripwk_init(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqRemoveWithRemoveIfPredicateWithKey",
	                  &self->srwripwk_item, &self->srwripwk_key))
		goto err;
	Dee_Incref(self->srwripwk_item);
	Dee_Incref(self->srwripwk_key);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
srwrip_fini(SeqRemoveWithRemoveIfPredicate *__restrict self) {
	Dee_Decref(self->srwrip_item);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwrip_visit(SeqRemoveWithRemoveIfPredicate *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwrip_item);
}

PRIVATE NONNULL((1)) void DCALL
srwripwk_fini(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self) {
	Dee_Decref(self->srwripwk_item);
	Dee_Decref(self->srwripwk_key);
}

PRIVATE NONNULL((1, 2)) void DCALL
srwripwk_visit(SeqRemoveWithRemoveIfPredicateWithKey *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->srwripwk_item);
	Dee_Visit(self->srwripwk_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwrip_call(SeqRemoveWithRemoveIfPredicate *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicate", &item))
		goto err;
	equals = DeeObject_TryCompareEq(self->srwrip_item, item);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
srwripwk_call(SeqRemoveWithRemoveIfPredicateWithKey *self, size_t argc, DeeObject *const *argv) {
	int equals;
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveWithRemoveIfPredicateWithKey", &item))
		goto err;
	equals = DeeObject_TryCompareKeyEq(self->srwripwk_item, item, self->srwripwk_key);
	if unlikely(equals == Dee_COMPARE_ERR)
		goto err;
	return_bool_(equals == 0);
err:
	return NULL;
}

STATIC_ASSERT(offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item) ==
              offsetof(SeqRemoveWithRemoveIfPredicate, srwrip_item));
#define srwrip_members (srwripwk_members + 1)
PRIVATE struct type_member tpconst srwripwk_members[] = {
	TYPE_MEMBER_FIELD("__item__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_item)),
	TYPE_MEMBER_FIELD("__key__", STRUCT_OBJECT, offsetof(SeqRemoveWithRemoveIfPredicateWithKey, srwripwk_key)),
	TYPE_MEMBER_END
};

PRIVATE DeeTypeObject SeqRemoveWithRemoveIfPredicate_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicate",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwrip_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicate)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwrip_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwrip_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwrip_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwrip_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

PRIVATE DeeTypeObject SeqRemoveWithRemoveIfPredicateWithKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveWithRemoveIfPredicateWithKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&srwripwk_init,
				TYPE_FIXED_ALLOCATOR(SeqRemoveWithRemoveIfPredicateWithKey)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&srwripwk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&srwripwk_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&srwripwk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ srwripwk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                    size_t start, size_t end) {
	/* >> return !!self.removeif(x -> deemon.equals(item, x), start, end, 1); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicate *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicate);
	if unlikely(!pred)
		goto err;
	Dee_Incref(item);
	pred->srwrip_item = item;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicate_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, 1);
	Dee_Decref_likely(pred);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}



struct default_remove_with_enumerate_index_and_delitem_index_data {
	DeeObject *drweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drweiadiid_item; /* [1..1] The object to remove. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareEq(data->drweiadiid_item, value);
	if unlikely(equal == Dee_COMPARE_ERR)
		goto err;
	if (equal != 0)
		return 0;
	if unlikely((*Dee_TYPE(data->drweiadiid_self)->tp_seq->tp_delitem_index)(data->drweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithTSCFindAndDelItemIndex(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end) {
	int result;
	size_t index = (*DeeType_SeqCache_RequireFind(Dee_TYPE(self)))(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                      size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_enumerate_index)(self, &default_remove_with_enumerate_index_and_delitem_index_cb,
	                                                               &data, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item,
                              size_t start, size_t end) {
	return err_seq_unsupportedf(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* remove() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, DeeObject *key) {
	size_t result;
	result = (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                           size_t start, size_t end, DeeObject *key) {
	/* >> local keyedElem = key(item);
	 * >> return !!self.removeif(x -> deemon.equals(keyedElem, key(x)), start, end, 1); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicateWithKey *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicateWithKey);
	if unlikely(!pred)
		goto err;
	pred->srwripwk_item = DeeObject_Call(key, 1, &item);
	if unlikely(!pred->srwripwk_item)
		goto err_pred;
	Dee_Incref(key);
	pred->srwripwk_key = key;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicateWithKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, 1);
	Dee_Decref_likely(pred);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err_pred:
	DeeObject_FREE(pred);
err:
	return -1;
}

struct default_remove_with_key_with_enumerate_index_and_delitem_index_data {
	DeeObject *drwkweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drwkweiadiid_item; /* [1..1] The object to remove (already keyed). */
	DeeObject *drwkweiadiid_key;  /* [1..1] The key used for object compare. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_key_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareKeyEq(data->drwkweiadiid_item, value, data->drwkweiadiid_key);
	if unlikely(equal == Dee_COMPARE_ERR)
		goto err;
	if (equal != 0)
		return 0;
	if unlikely((*Dee_TYPE(data->drwkweiadiid_self)->tp_seq->tp_delitem_index)(data->drwkweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithTSCFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                             size_t start, size_t end, DeeObject *key) {
	int result;
	size_t index = (*DeeType_SeqCache_RequireFindWithKey(Dee_TYPE(self)))(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                             size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_enumerate_index)(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
	                                                               &data, start, end);
	Dee_Decref(data.drwkweiadiid_item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "remove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* rremove()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithTSCRFindAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                 size_t start, size_t end) {
	int result;
	size_t index = (*DeeType_SeqCache_RequireRFind(Dee_TYPE(self)))(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                 size_t start, size_t end) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	tsc_enumerate_index_reverse = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(tsc_enumerate_index_reverse);
	foreach_status = (*tsc_enumerate_index_reverse)(self, &default_remove_with_enumerate_index_and_delitem_index_cb,
	                                                &data, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                       size_t start, size_t end) {
	size_t index;
	index = DeeSeq_DefaultRFindWithEnumerateIndex(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item,
                               size_t start, size_t end) {
	return err_seq_unsupportedf(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* rremove() (with key)                                                 */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithTSCRFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                               size_t start, size_t end, DeeObject *key) {
	int result;
	size_t index = (*DeeType_SeqCache_RequireRFindWithKey(Dee_TYPE(self)))(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index);
	if unlikely(result)
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                        size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	tsc_enumerate_index_reverse = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self));
	ASSERT(tsc_enumerate_index_reverse);
	foreach_status = (*tsc_enumerate_index_reverse)(self, &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
	                                                &data, start, end);
	Dee_Decref(data.drwkweiadiid_item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, DeeObject *key) {
	size_t index;
	index = DeeSeq_DefaultRFindWithKeyWithEnumerateIndex(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		return 0;
	if unlikely((*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, index))
		goto err;
	return 1;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item,
                                      size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "rremove(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* removeall()                                                          */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                       size_t start, size_t end, size_t max) {
	/* >> return self.removeif(x -> deemon.equals(item, x), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicate *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicate);
	if unlikely(!pred)
		goto err;
	Dee_Incref(item);
	pred->srwrip_item = item;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicate_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithTSCRemove(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, size_t max) {
	size_t result = 0;
	Dee_tsc_remove_t tsc_remove;
	tsc_remove = DeeType_SeqCache_RequireRemove(Dee_TYPE(self));
	while (result < max) {
		int temp;
		temp = (*tsc_remove)(self, item, start, end);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                              size_t start, size_t end, size_t max) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareEq(item, elem);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end, size_t max) {
	return err_seq_unsupportedf(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
	                            item, start, end, max);
}






/************************************************************************/
/* removeall() (with key)                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item,
                                              size_t start, size_t end, size_t max,
                                              DeeObject *key) {
	/* >> local keyedElem = key(item);
	 * >> return self.removeif(x -> deemon.equals(keyedElem, key(x)), start, end, max); */
	size_t result;
	DREF SeqRemoveWithRemoveIfPredicateWithKey *pred;
	pred = DeeObject_MALLOC(SeqRemoveWithRemoveIfPredicateWithKey);
	if unlikely(!pred)
		goto err;
	pred->srwripwk_item = DeeObject_Call(key, 1, &item);
	if unlikely(!pred->srwripwk_item)
		goto err_pred;
	Dee_Incref(key);
	pred->srwripwk_key = key;
	DeeObject_Init(pred, &SeqRemoveWithRemoveIfPredicateWithKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, (DeeObject *)pred, start, end, max);
	Dee_Decref_likely(pred);
	return result;
err_pred:
	DeeObject_FREE(pred);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, size_t max,
                                                   DeeObject *key) {
	size_t result = 0;
	Dee_tsc_remove_with_key_t tsc_remove_with_key;
	tsc_remove_with_key = DeeType_SeqCache_RequireRemoveWithKey(Dee_TYPE(self));
	while (result < max) {
		int temp;
		temp = (*tsc_remove_with_key)(self, item, start, end, key);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			break;
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item,
                                                                     size_t start, size_t end, size_t max,
                                                                     DeeObject *key) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start >= end)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err;
	do {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err_item;
		if (elem != ITER_DONE) {
			int equal;
			equal = DeeObject_TryCompareKeyEq(item, elem, key);
			Dee_Decref(elem);
			if unlikely(equal == Dee_COMPARE_ERR)
				goto err_item;
			if (equal == 0) {
				/* Found one! (delete it) */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err_item;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err_item;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err_item;
	} while (start < end);
	Dee_Decref(item);
	return result;
err_item:
	Dee_Decref(item);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 6)) size_t DCALL
DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end, size_t max,
                                        DeeObject *key) {
	return err_seq_unsupportedf(self, "removeall(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ", %r)",
	                            item, start, end, max, key);
}






/************************************************************************/
/* removeif()                                                           */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_removeif_with_removeall_item_compare_eq(DeeObject *self, DeeObject *should_result) {
	int result;
	(void)self;
	result = DeeObject_Bool(should_result);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_removeif_with_removeall_item_eq(DeeObject *self, DeeObject *should_result) {
	(void)self;
	return_reference_(should_result);
}

PRIVATE struct type_cmp seq_removeif_with_removeall_item_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &seq_removeif_with_removeall_item_compare_eq,
	/* .tp_eq            = */ &seq_removeif_with_removeall_item_eq,
};

PRIVATE DeeTypeObject SeqRemoveIfWithRemoveAllItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &seq_removeif_with_removeall_item_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

PRIVATE DeeObject SeqRemoveIfWithRemoveAllItem_DummyInstance = {
	OBJECT_HEAD_INIT(&SeqRemoveIfWithRemoveAllItem_Type)
};

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *sriwrak_should; /* [1..1] Predicate to determine if an element should be removed. */
} SeqRemoveIfWithRemoveAllKey;

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_removeif_with_removeall_key_init(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveIfWithRemoveAllKey", &self->sriwrak_should))
		goto err;
	Dee_Incref(self->sriwrak_should);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
seq_removeif_with_removeall_key_fini(SeqRemoveIfWithRemoveAllKey *__restrict self) {
	Dee_Decref(self->sriwrak_should);
}

PRIVATE NONNULL((1, 2)) void DCALL
seq_removeif_with_removeall_key_visit(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                      dvisit_t proc, void *arg) {
	Dee_Visit(self->sriwrak_should);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_removeif_with_removeall_key_printrepr(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                          Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "rt.SeqRemoveIfWithRemoveAllKey(%r)", self->sriwrak_should);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_removeif_with_removeall_key_call(SeqRemoveIfWithRemoveAllKey *__restrict self,
                                     size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRemoveIfWithRemoveAllKey", &item))
		goto err;
	if (item == &SeqRemoveIfWithRemoveAllItem_DummyInstance)
		return_reference_(item);
	return DeeObject_Call(self->sriwrak_should, argc, argv);
err:
	return NULL;
}

PRIVATE DeeTypeObject SeqRemoveIfWithRemoveAllKey_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRemoveIfWithRemoveAllKey",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&seq_removeif_with_removeall_key_init,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *))&seq_removeif_with_removeall_key_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *, Dee_formatprinter_t, void *))&seq_removeif_with_removeall_key_printrepr,
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&seq_removeif_with_removeall_key_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *, dvisit_t, void *))&seq_removeif_with_removeall_key_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};



INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *should,
                                              size_t start, size_t end, size_t max) {
	/* >> global final class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> global final SeqRemoveIfWithRemoveAllItem_DummyInstance = SeqRemoveIfWithRemoveAllItem();
	 * >>
	 * >> class SeqRemoveIfWithRemoveAllItem { operator == (other) -> other; };
	 * >> return self.removeall(SeqRemoveIfWithRemoveAllItem_DummyInstance, start, end, max, key: x -> {
	 * >>     return x === SeqRemoveIfWithRemoveAllItem_DummyInstance ? x : should(x);
	 * >> }); */
	size_t result;
	DREF SeqRemoveIfWithRemoveAllKey *key;
	key = DeeObject_MALLOC(SeqRemoveIfWithRemoveAllKey);
	if unlikely(!key)
		goto err;
	Dee_Incref(should);
	key->sriwrak_should = should;
	DeeObject_Init(key, &SeqRemoveIfWithRemoveAllKey_Type);
	result = (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, &SeqRemoveIfWithRemoveAllItem_DummyInstance,
	                                                                     start, end, max, (DeeObject *)key);
	Dee_Decref_likely(key);
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *should,
                                                             size_t start, size_t end, size_t max) {
	int sequence_size_changes_after_delitem = -1;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize, result = 0;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while (start < end) {
		DREF DeeObject *elem;
		elem = (*seq->tp_trygetitem_index)(self, start);
		if unlikely(!elem)
			goto err;
		if (elem != ITER_DONE) {
			int should_remove;
			DREF DeeObject *pred_result;
			pred_result = DeeObject_Call(should, 1, &elem);
			Dee_Decref(elem);
			if unlikely(!pred_result)
				goto err;
			should_remove = DeeObject_BoolInherited(pred_result);
			if unlikely(should_remove < 0)
				goto err;
			if (should_remove) {
				/* Delete this one */
				if unlikely((*seq->tp_delitem_index)(self, start))
					goto err;
				++result;
				if (result >= max)
					break;
				if (sequence_size_changes_after_delitem == -1) {
					size_t new_selfsize = (*seq->tp_size)(self);
					if unlikely(new_selfsize == (size_t)-1)
						goto err;
					sequence_size_changes_after_delitem = selfsize > new_selfsize ? 1 : 0;
				}
				if (sequence_size_changes_after_delitem) {
					--end;
				} else {
					++start;
				}
				goto check_interrupt;
			}
		}
		++start;
check_interrupt:
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultRemoveIfWithError(DeeObject *self, DeeObject *should,
                                size_t start, size_t end, size_t max) {
	return err_seq_unsupportedf(self, "removeif(%r, %" PRFuSIZ ", %" PRFuSIZ ", %" PRFuSIZ ")",
	                            should, start, end, max);
}






/************************************************************************/
/* resize()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	oldsize = (*seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*seq->tp_delrange_index)(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndSetRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	oldsize = (*seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*seq->tp_setrange_index)(self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize, Dee_EmptySeq);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_DefaultResizeWithSizeAndTSCEraseAndTSCExtend(DeeObject *self, size_t newsize, DeeObject *filler) {
	size_t oldsize;
	oldsize = (*Dee_TYPE(self)->tp_seq->tp_size)(self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = (*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return (*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, newsize, oldsize - newsize);
	}
	return 0;
err:
	return -1;
}






/************************************************************************/
/* fill()                                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithSizeAndSetRangeIndex(DeeObject *self, size_t start,
                                           size_t end, DeeObject *filler) {
	int result;
	size_t selfsize;
	DREF DeeObject *repeat;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	repeat = DeeSeq_RepeatItem(filler, end - start);
	if unlikely(!repeat)
		goto err;
	result = (*seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, repeat);
	Dee_Decref(repeat);
	return result;
err:
	return -1;
}

struct default_fill_with_enumerate_index_and_setitem_index_data {
	DeeObject *dfweiasiid_seq;    /* [1..1] Sequence whose items to set. */
	DeeObject *dfweiasiid_filler; /* [1..1] Value to assign to indices. */
	WUNUSED_T NONNULL_T((1, 3)) int (DCALL *dfweiasiid_setitem_index)(DeeObject *self, size_t index, DeeObject *value);
};

PRIVATE WUNUSED Dee_ssize_t DCALL
default_fill_with_enumerate_index_and_setitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	struct default_fill_with_enumerate_index_and_setitem_index_data *data;
	(void)value;
	data = (struct default_fill_with_enumerate_index_and_setitem_index_data *)arg;
	return (*data->dfweiasiid_setitem_index)(data->dfweiasiid_seq, index, data->dfweiasiid_filler);
}


INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex(DeeObject *self, size_t start,
                                                    size_t end, DeeObject *filler) {
	struct default_fill_with_enumerate_index_and_setitem_index_data data;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	data.dfweiasiid_seq    = self;
	data.dfweiasiid_filler = filler;
	data.dfweiasiid_setitem_index = seq->tp_setitem_index;
	return (int)(*seq->tp_enumerate_index)(self, &default_fill_with_enumerate_index_and_setitem_index_cb,
	                                       &data, start, end);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultFillWithError(DeeObject *self, size_t start, size_t end, DeeObject *filler) {
	return err_seq_unsupportedf(self, "fill(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, filler);
}






/************************************************************************/
/* reverse()                                                            */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithTSCReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *reversed;
	reversed = (*DeeType_SeqCache_RequireReversed(Dee_TYPE(self)))(self, start, end);
	if unlikely(!reversed)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, reversed);
	Dee_Decref(reversed);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeObject *lo_elem;
	DREF DeeObject *hi_elem;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*seq->tp_getitem_index)(self, start);
		if unlikely(!lo_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		hi_elem = (*seq->tp_getitem_index)(self, end - 1);
		if unlikely(!hi_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err_lo_elem;
		if (hi_elem) {
			if unlikely((*seq->tp_setitem_index)(self, start, hi_elem))
				goto err_lo_elem_hi_elem;
			Dee_Decref(hi_elem);
		} else {
			if unlikely((*seq->tp_delitem_index)(self, start))
				goto err_lo_elem;
		}
		if (lo_elem) {
			if unlikely((*seq->tp_setitem_index)(self, end - 1, lo_elem))
				goto err_lo_elem;
			Dee_Decref(lo_elem);
		} else {
			if unlikely((*seq->tp_delitem_index)(self, end - 1))
				goto err;
		}
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_XDecref(hi_elem);
err_lo_elem:
	Dee_XDecref(lo_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeObject *lo_elem;
	DREF DeeObject *hi_elem;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	ASSERT(seq == Dee_TYPE(self)->tp_seq);
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = (*seq->tp_getitem_index)(self, start);
		if unlikely(!lo_elem)
			goto err;
		hi_elem = (*seq->tp_getitem_index)(self, end - 1);
		if unlikely(!hi_elem)
			goto err_lo_elem;
		if unlikely((*seq->tp_setitem_index)(self, start, hi_elem))
			goto err_lo_elem_hi_elem;
		Dee_Decref(hi_elem);
		if unlikely((*seq->tp_setitem_index)(self, end - 1, lo_elem))
			goto err_lo_elem;
		Dee_Decref(lo_elem);
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_Decref(hi_elem);
err_lo_elem:
	Dee_Decref(lo_elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultReverseWithError(DeeObject *self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "reverse(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}






/************************************************************************/
/* reversed()                                                           */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_getitem_index_fast;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_getitem_index;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end) {
	DREF DefaultReversed_WithGetItemIndex *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = seq->tp_trygetitem_index;
	Dee_Incref(self);
	result->drwgii_seq  = self;
	result->drwgii_max  = end - 1; /* It's ok if this underflows */
	result->drwgii_size = end - start;
	DeeObject_Init(result, &DefaultReversed_WithTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

struct foreach_subrange_as_tuple_data {
	DREF DeeTupleObject *fesrat_result;  /* [1..1] The tuple being constructed. */
	size_t               fesrat_used;    /* Used # of elements of `fesrat_result' */
	size_t               fesrat_maxsize; /* Max value for `fesrat_used' */
	size_t               fesrat_start;   /* # of elements that still need to be skipped. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
foreach_subrange_as_tuple_cb(void *arg, DeeObject *elem) {
	struct foreach_subrange_as_tuple_data *data;
	data = (struct foreach_subrange_as_tuple_data *)arg;
	if (data->fesrat_start) {
		--data->fesrat_start; /* Skip leading. */
		return 0;
	}
	if (data->fesrat_used >= DeeTuple_SIZE(data->fesrat_result)) {
		DREF DeeTupleObject *new_tuple;
		size_t new_size = DeeTuple_SIZE(data->fesrat_result) * 2;
		if (new_size < 16)
			new_size = 16;
		new_tuple = DeeTuple_TryResizeUninitialized(data->fesrat_result, new_size);
		if unlikely(!new_tuple) {
			new_size  = data->fesrat_used + 1;
			new_tuple = DeeTuple_ResizeUninitialized(data->fesrat_result, new_size);
			if unlikely(!new_tuple)
				goto err;
		}
		data->fesrat_result = new_tuple;
	}
	Dee_Incref(elem);
	data->fesrat_result->t_elem[data->fesrat_used++] = elem;
	if (data->fesrat_used >= data->fesrat_maxsize)
		return -2; /* Stop enumeration */
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetForeachSubRangeAsTuple(DeeObject *self, size_t start, size_t end) {
	size_t fast_size;
	Dee_ssize_t foreach_status;
	struct foreach_subrange_as_tuple_data data;
	if unlikely(start >= end)
		return_empty_tuple;
	fast_size = (*Dee_TYPE(self)->tp_seq->tp_size_fast)(self);
	if (fast_size != (size_t)-1) {
		data.fesrat_result = DeeTuple_NewUninitialized(fast_size);
		if unlikely(!data.fesrat_result)
			goto err;
	} else {
		Dee_Incref(Dee_EmptyTuple);
		data.fesrat_result = (DREF DeeTupleObject *)Dee_EmptyTuple;
	}
	data.fesrat_used    = 0;
	data.fesrat_maxsize = end - start;
	data.fesrat_start   = start;
	foreach_status = (*Dee_TYPE(self)->tp_seq->tp_foreach)(self, &foreach_subrange_as_tuple_cb, &data);
	ASSERT(foreach_status == 0 || foreach_status == -1);
	if unlikely(foreach_status < 0)
		goto err_r;
	data.fesrat_result = DeeTuple_TruncateUninitialized(data.fesrat_result, data.fesrat_used);
	return (DREF DeeObject *)data.fesrat_result;
err_r:
	Dee_Decrefv(data.fesrat_result->t_elem, data.fesrat_used);
	DeeTuple_FreeUninitialized(data.fesrat_result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultReversedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end) {
	DREF DeeObject *result;
	result = DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if likely(result) {
		DREF DeeObject **lo, **hi;
		lo = DeeTuple_ELEM(result);
		hi = lo + DeeTuple_SIZE(result);
		while (lo < hi) {
			DeeObject *temp;
			temp  = *lo;
			*lo++ = *--hi;
			*hi   = temp;
		}
	}
	return result;
}






/************************************************************************/
/* sort()                                                               */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end) {
	int result;
	DREF DeeObject *sorted;
	sorted = (*DeeType_SeqCache_RequireSorted(Dee_TYPE(self)))(self, start, end);
	if unlikely(!sorted)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, sorted);
	Dee_Decref(sorted);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithError(DeeObject *self, size_t start, size_t end) {
	return err_seq_unsupportedf(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ")", start, end);
}






/************************************************************************/
/* sort() (with key)                                                    */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start,
                                                       size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *sorted;
	sorted = (*DeeType_SeqCache_RequireSortedWithKey(Dee_TYPE(self)))(self, start, end, key);
	if unlikely(!sorted)
		goto err;
	result = (*Dee_TYPE(self)->tp_seq->tp_setrange_index)(self, (Dee_ssize_t)start, (Dee_ssize_t)end, sorted);
	Dee_Decref(sorted);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start,
                                                                size_t end, DeeObject *key) {
	/* TODO */
	(void)self;
	(void)start;
	(void)end;
	(void)key;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultSortWithKeyWithError(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	return err_seq_unsupportedf(self, "sort(%" PRFuSIZ ", %" PRFuSIZ ", %r)", start, end, key);
}






/************************************************************************/
/* sorted()                                                             */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFast(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                        self, start, seq->tp_getitem_index_fast))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */

	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndex(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                       self, start, seq->tp_trygetitem_index))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end) {
	DREF DeeTupleObject *base, *result;
	base = (DREF DeeTupleObject *)DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVector(DeeTuple_SIZE(result),
	                              DeeTuple_ELEM(result),
	                              DeeTuple_ELEM(base)))
		goto err_base_r;
	DeeTuple_FreeUninitialized(base);
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}






/************************************************************************/
/* sorted() (with key)                                                  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFastWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                               self, start, seq->tp_getitem_index_fast, key))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	size_t selfsize;
	DREF DeeTupleObject *result;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	result = DeeTuple_NewUninitialized(end - start);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndexWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result),
	                                              self, start, seq->tp_trygetitem_index, key))
		goto err_r;
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault(DeeObject *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *base;
	DREF DeeTupleObject *result;
	base = DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVectorWithKey(DeeTuple_SIZE(result),
	                                     DeeTuple_ELEM(result),
	                                     DeeTuple_ELEM(base),
	                                     key))
		goto err_base_r;
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}






/************************************************************************/
/* bfind()                                                              */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBFindWithTSCBRange(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end) {
	size_t result_range[2];
	if unlikely(new_DeeSeq_BRange(self, item, start, end, result_range))
		goto err;
	if (result_range[0] == result_range[1])
		return (size_t)-1; /* Not found */
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBFindWithError(DeeObject *self, DeeObject *item,
                             size_t start, size_t end) {
	err_seq_unsupportedf(self, "bfind(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bfind() (with key)                                                   */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBFindWithKeyWithTSCBRangeWithKey(DeeObject *self, DeeObject *item,
                                               size_t start, size_t end, DeeObject *key) {
	size_t result_range[2];
	if unlikely(new_DeeSeq_BRangeWithKey(self, item, start, end, key, result_range))
		goto err;
	if (result_range[0] == result_range[1])
		return (size_t)-1; /* Not found */
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBFindWithKeyWithError(DeeObject *self, DeeObject *item,
                                    size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "bfind(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bposition()                                                          */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBPositionWithTSCBRange(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end) {
	size_t result_range[2];
	if unlikely(new_DeeSeq_BRange(self, item, start, end, result_range))
		goto err;
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_DefaultBPositionWithError(DeeObject *self, DeeObject *item,
                                 size_t start, size_t end) {
	err_seq_unsupportedf(self, "bposition(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* bposition() (with key)                                               */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBPositionWithKeyWithTSCBRangeWithKey(DeeObject *self, DeeObject *item,
                                                   size_t start, size_t end, DeeObject *key) {
	size_t result_range[2];
	if unlikely(new_DeeSeq_BRangeWithKey(self, item, start, end, key, result_range))
		goto err;
	if unlikely(result_range[0] == (size_t)Dee_COMPARE_ERR ||
	            result_range[0] == (size_t)-1)
		goto err_overflow;
	return result_range[0];
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
DeeSeq_DefaultBPositionWithKeyWithError(DeeObject *self, DeeObject *item,
                                        size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "bposition(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
}






/************************************************************************/
/* brange()                                                             */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeSeq_DefaultBRangeWithError(DeeObject *self, DeeObject *item,
                              size_t start, size_t end,
                              size_t result_range[2]) {
	(void)result_range;
	return err_seq_unsupportedf(self, "brange(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
}






/************************************************************************/
/* brange() (with key)                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
DeeSeq_DefaultBRangeWithKeyWithError(DeeObject *self, DeeObject *item,
                                     size_t start, size_t end, DeeObject *key,
                                     size_t result_range[2]) {
	(void)result_range;
	return err_seq_unsupportedf(self, "brange(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
}






/************************************************************************/
/* blocate()                                                            */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithTSCBFindAndGetItemIndex(DeeObject *self, DeeObject *item,
                                                 size_t start, size_t end) {
	size_t index = new_DeeSeq_BFind(self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		goto err_not_found;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
err_not_found:
	err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithError(DeeObject *self, DeeObject *item,
                               size_t start, size_t end) {
	err_seq_unsupportedf(self, "blocate(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return NULL;
}






/************************************************************************/
/* blocate() (with key)                                                 */
/************************************************************************/

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithKeyWithTSCBFindWithKeyAndGetItemIndex(DeeObject *self, DeeObject *item,
                                                               size_t start, size_t end, DeeObject *key) {
	size_t index = new_DeeSeq_BFindWithKey(self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(index == (size_t)-1)
		goto err_not_found;
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, index);
err_not_found:
	err_item_not_found(self, item);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeSeq_DefaultBLocateWithKeyWithError(DeeObject *self, DeeObject *item,
                                      size_t start, size_t end, DeeObject *key) {
	err_seq_unsupportedf(self, "blocate(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return NULL;
}













/************************************************************************/
/* Deemon user-code wrappers                                            */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_getfirst(DeeObject *__restrict self) {
	return DeeSeq_GetFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_boundfirst(DeeObject *__restrict self) {
	return DeeSeq_BoundFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_delfirst(DeeObject *__restrict self) {
	return DeeSeq_DelFirst(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default_seq_setfirst(DeeObject *self, DeeObject *value) {
	return DeeSeq_SetFirst(self, value);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_getlast(DeeObject *__restrict self) {
	return DeeSeq_GetLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_boundlast(DeeObject *__restrict self) {
	return DeeSeq_BoundLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_dellast(DeeObject *__restrict self) {
	return DeeSeq_DelLast(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
default_seq_setlast(DeeObject *self, DeeObject *value) {
	return DeeSeq_SetLast(self, value);
}

INTERN WUNUSED NONNULL((1)) int DCALL
default_seq_bool(DeeObject *__restrict self) {
	return DeeSeq_NonEmpty(self);
}

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("cb");
print define_Dee_HashStr("start");
print define_Dee_HashStr("end");
]]]*/
#define Dee_HashStr__cb _Dee_HashSelectC(0x75ffadba, 0x2501dbb50208b92e)
#define Dee_HashStr__start _Dee_HashSelectC(0xa2ed6890, 0x80b621ce3c3982d5)
#define Dee_HashStr__end _Dee_HashSelectC(0x37fb4a05, 0x6de935c204dc3d01)
/*[[[end]]]*/

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
do_default_seq_enumerate_with_kw(DeeObject *self, size_t argc,
                                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeObject *cb, *startob, *endob;
	size_t start, end;
	DeeKwArgs kwds;
	if (DeeKwArgs_Init(&kwds, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0: {
		if unlikely((cb = DeeKwArgs_TryGetItemNRStringHash(&kwds, "cb", Dee_HashStr__cb)) == NULL)
			goto err;
		if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
			goto err;
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (cb != ITER_DONE) {
handle_with_cb:
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = DeeSeq_EnumerateWithIntRange(self, cb, start, end);
					} else {
						result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = DeeSeq_EnumerateWithIntRange(self, cb, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = DeeSeq_Enumerate(self, cb);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = DeeSeq_EnumerateWithIntRange(self, cb, start, (size_t)-1);
			}
		} else {
			if (endob != ITER_DONE) {
				if (startob != ITER_DONE) {
					if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
					    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
						result = DeeSeq_MakeEnumerationWithIntRange(self, start, end);
					} else {
						result = DeeSeq_MakeEnumerationWithRange(self, startob, endob);
					}
				} else if (DeeInt_Check(endob) && DeeInt_TryAsSize(endob, &end)) {
					result = DeeSeq_MakeEnumerationWithIntRange(self, 0, end);
				} else {
					startob = DeeObject_NewDefault(Dee_TYPE(endob));
					if unlikely(!startob)
						goto err;
					result = DeeSeq_MakeEnumerationWithRange(self, startob, endob);
					Dee_Decref(startob);
				}
			} else if (startob == ITER_DONE) {
				result = DeeSeq_MakeEnumeration(self);
			} else {
				ASSERT(startob != ITER_DONE);
				ASSERT(endob == ITER_DONE);
				if (DeeObject_AsSize(startob, &start))
					goto err;
				result = DeeSeq_MakeEnumerationWithIntRange(self, start, (size_t)-1);
			}
		}
	}	break;

	case 1: {
		cb = argv[0];
		if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
			goto err;
		if (DeeCallable_Check(cb)) {
			if unlikely((startob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "start", Dee_HashStr__start)) == NULL)
				goto err;
			goto handle_with_cb;
		}
		startob = cb;
		if (endob != ITER_DONE) {
			if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
			    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
				result = DeeSeq_MakeEnumerationWithIntRange(self, start, end);
			} else {
				result = DeeSeq_MakeEnumerationWithRange(self, startob, endob);
			}
		} else {
			if (DeeObject_AsSize(startob, &start))
				goto err;
			result = DeeSeq_MakeEnumerationWithIntRange(self, start, (size_t)-1);
		}
	}	break;

	case 2: {
		cb = argv[0];
		if (DeeCallable_Check(cb)) {
			if unlikely((endob = DeeKwArgs_TryGetItemNRStringHash(&kwds, "end", Dee_HashStr__end)) == NULL)
				goto err;
			startob = argv[1];
			goto handle_with_cb;
		}
		startob = argv[0];
		endob   = argv[1];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = DeeSeq_MakeEnumerationWithIntRange(self, start, end);
		} else {
			result = DeeSeq_MakeEnumerationWithRange(self, startob, endob);
		}
	}	break;

	case 3: {
		cb      = argv[0];
		startob = argv[1];
		endob   = argv[2];
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end))) {
			result = DeeSeq_EnumerateWithIntRange(self, cb, start, end);
		} else {
			result = DeeSeq_EnumerateWithRange(self, cb, startob, endob);
		}
	}	break;

	default:
		goto err_bad_args;
	}
	if unlikely(DeeKwArgs_Done(&kwds, argc, "enumerate"))
		goto err_r;
	return result;
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
	goto err;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_enumerate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start, end;
	if unlikely(kw)
		return do_default_seq_enumerate_with_kw(self, argc, argv, kw);
	if likely(argc == 0)
		return DeeSeq_MakeEnumeration(self);
	if (DeeCallable_Check(argv[0])) {
		if (argc == 1)
			return DeeSeq_Enumerate(self, argv[0]);
		if unlikely(argc == 2) {
			if (DeeObject_AsSize(argv[1], &start))
				goto err;
			return DeeSeq_EnumerateWithIntRange(self, argv[0], start, (size_t)-1);
		}
		if (argc != 3)
			goto err_bad_args;
		if ((DeeInt_Check(argv[1]) && DeeInt_Check(argv[2])) &&
		    (DeeInt_TryAsSize(argv[1], &start) && DeeInt_TryAsSize(argv[2], &end)))
			return DeeSeq_EnumerateWithIntRange(self, argv[0], start, end);
		return DeeSeq_EnumerateWithRange(self, argv[0], argv[1], argv[2]);
	} else {
		if unlikely(argc == 1) {
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			return DeeSeq_MakeEnumerationWithIntRange(self, start, (size_t)-1);
		}
		if (argc != 2)
			goto err_bad_args;
		if ((DeeInt_Check(argv[0]) && DeeInt_Check(argv[1])) &&
		    (DeeInt_TryAsSize(argv[0], &start) && DeeInt_TryAsSize(argv[1], &end)))
			return DeeSeq_MakeEnumerationWithIntRange(self, start, end);
		return DeeSeq_MakeEnumerationWithRange(self, argv[0], argv[1]);
	}
	__builtin_unreachable();
err_bad_args:
	err_invalid_argc("enumerate", argc, 0, 3);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_any(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:any",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_AnyWithKey(self, key)
		         : new_DeeSeq_Any(self);
	} else {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_AnyWithRangeAndKey(self, start, end, key)
		         : new_DeeSeq_AnyWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_all(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:all",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_AllWithKey(self, key)
		         : new_DeeSeq_All(self);
	} else {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_AllWithRangeAndKey(self, start, end, key)
		         : new_DeeSeq_AllWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_parity(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:parity",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_ParityWithKey(self, key)
		         : new_DeeSeq_Parity(self);
	} else {
		result = !DeeNone_Check(key)
		         ? new_DeeSeq_ParityWithRangeAndKey(self, start, end, key)
		         : new_DeeSeq_ParityWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_reduce(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *combine, *init = NULL;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__combine_start_end_init,
	                    "o|" UNPuSIZ UNPuSIZ "o:reduce",
	                    &combine, &start, &end, &init))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (init)
			return new_DeeSeq_ReduceWithInit(self, combine, init);
		return new_DeeSeq_Reduce(self, combine);
	}
	if (init)
		return new_DeeSeq_ReduceWithRangeAndInit(self, combine, start, end, init);
	return new_DeeSeq_ReduceWithRange(self, combine, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_min(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:min",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return new_DeeSeq_Min(self);
		return new_DeeSeq_MinWithKey(self, key);
	}
	if (DeeNone_Check(key))
		return new_DeeSeq_MinWithRange(self, start, end);
	return new_DeeSeq_MinWithRangeAndKey(self, start, end, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_max(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:max",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return new_DeeSeq_Max(self);
		return new_DeeSeq_MaxWithKey(self, key);
	}
	if (DeeNone_Check(key))
		return new_DeeSeq_MaxWithRange(self, start, end);
	return new_DeeSeq_MaxWithRangeAndKey(self, start, end, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_sum(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":sum",
	                    &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1)
		return new_DeeSeq_Sum(self);
	return new_DeeSeq_SumWithRange(self, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_count(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:count",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_Count(self, item);
		} else {
			result = new_DeeSeq_CountWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_CountWithRange(self, item, start, end);
		} else {
			result = new_DeeSeq_CountWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_contains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:contains",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_Contains(self, item);
		} else {
			result = new_DeeSeq_ContainsWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_ContainsWithRange(self, item, start, end);
		} else {
			result = new_DeeSeq_ContainsWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_locate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:locate",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key))
			return new_DeeSeq_Locate(self, item);
		return new_DeeSeq_LocateWithKey(self, item, key);
	}
	if (DeeNone_Check(key))
		return new_DeeSeq_LocateWithRange(self, item, start, end);
	return new_DeeSeq_LocateWithRangeAndKey(self, item, start, end, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_rlocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rlocate",
	                    &item, &start, &end, &key))
		goto err;
	if (DeeNone_Check(key))
		return new_DeeSeq_RLocateWithRange(self, item, start, end);
	return new_DeeSeq_RLocateWithRangeAndKey(self, item, start, end, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_startswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:startswith",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_StartsWith(self, item);
		} else {
			result = new_DeeSeq_StartsWithWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_StartsWithWithRange(self, item, start, end);
		} else {
			result = new_DeeSeq_StartsWithWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_endswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:endswith",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_EndsWith(self, item);
		} else {
			result = new_DeeSeq_EndsWithWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = new_DeeSeq_EndsWithWithRange(self, item, start, end);
		} else {
			result = new_DeeSeq_EndsWithWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}



INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_find(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:find",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_FindWithKey(self, item, start, end, key)
	         : new_DeeSeq_Find(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_rfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rfind",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_RFindWithKey(self, item, start, end, key)
	         : new_DeeSeq_RFind(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_count,
	                    UNPuSIZ "|" UNPuSIZ ":erase",
	                    &index, &count))
		goto err;
	if unlikely(new_DeeSeq_Erase(self, index, count))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "o:insert",
	                    &index, &item))
		goto err;
	if unlikely(new_DeeSeq_Insert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_insertall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_items,
	                    UNPuSIZ "o:insertall",
	                    &index, &items))
		goto err;
	if unlikely(new_DeeSeq_InsertAll(self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &item))
		goto err;
	if unlikely(new_DeeSeq_PushFront(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_append(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:append", &item))
		goto err;
	if unlikely(new_DeeSeq_Append(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:extend", &items))
		goto err;
	if unlikely(new_DeeSeq_Extend(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_xchitem(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_value,
	                    UNPuSIZ "o:xchitem", &index, &value))
		goto err;
	return new_DeeSeq_XchItemIndex(self, index, value);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	if unlikely(new_DeeSeq_Clear(self))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_pop(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index,
	                    "|" UNPdSIZ ":pop", &index))
		goto err;
	return new_DeeSeq_Pop(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	return new_DeeSeq_Pop(self, 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	return new_DeeSeq_Pop(self, -1);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:remove",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_RemoveWithKey(self, item, start, end, key)
	         : new_DeeSeq_Remove(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:rremove",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_RRemoveWithKey(self, item, start, end, key)
	         : new_DeeSeq_RRemove(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_max_key,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &item, &start, &end, &max, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_RemoveAllWithKey(self, item, start, end, max, key)
	         : new_DeeSeq_RemoveAll(self, item, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_removeif(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	DeeObject *should;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__should_start_end_max,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ ":removeif",
	                    &should, &start, &end, &max))
		goto err;
	result = new_DeeSeq_RemoveIf(self, should, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_resize(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t size;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__size_filler,
	                    UNPuSIZ "|o:resize", &size, &filler))
		goto err;
	if unlikely(new_DeeSeq_Resize(self, size, filler))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_fill(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_filler,
	                    "|" UNPuSIZ UNPuSIZ "o:fill",
	                    &start, &end, &filler))
		goto err;
	if unlikely(new_DeeSeq_Fill(self, start, end, filler))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reverse",
	                    &start, &end))
		goto err;
	if unlikely(new_DeeSeq_Reverse(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reversed",
	                    &start, &end))
		goto err;
	return new_DeeSeq_Reversed(self, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_sort(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sort",
	                    &start, &end, &key))
		goto err;
	if unlikely(!DeeNone_Check(key)
	            ? new_DeeSeq_SortWithKey(self, start, end, key)
	            : new_DeeSeq_Sort(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_sorted(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sorted",
	                    &start, &end, &key))
		goto err;
	return !DeeNone_Check(key)
	       ? new_DeeSeq_SortedWithKey(self, start, end, key)
	       : new_DeeSeq_Sorted(self, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_bfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bfind",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_BFindWithKey(self, item, start, end, key)
	         : new_DeeSeq_BFind(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_bposition(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bposition",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? new_DeeSeq_BPositionWithKey(self, item, start, end, key)
	         : new_DeeSeq_BPosition(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_brange(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, result_range[2];
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:brange",
	                    &item, &start, &end, &key))
		goto err;
	if (!DeeNone_Check(key)
	    ? new_DeeSeq_BRangeWithKey(self, item, start, end, key, result_range)
	    : new_DeeSeq_BRange(self, item, start, end, result_range))
		goto err;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, result_range[0], result_range[1]);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq_blocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:blocate",
	                    &item, &start, &end, &key))
		goto err;
	return !DeeNone_Check(key)
	       ? new_DeeSeq_BLocateWithKey(self, item, start, end, key)
	       : new_DeeSeq_BLocate(self, item, start, end);
err:
	return NULL;
}



#if 0
PRIVATE struct type_getset tpconst default_seq_getsets[] = {
	TYPE_GETSET_BOUND(STR_first, &default_seq_getfirst, &default_seq_delfirst, &default_seq_setfirst, &default_seq_boundfirst, "->"),
	TYPE_GETSET_BOUND(STR_last, &default_seq_getlast, &default_seq_dellast, &default_seq_setlast, &default_seq_boundlast, "->"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst default_seq_methods[] = {
	TYPE_KWMETHOD(STR_any, &default_seq_enumerate, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX)->?S?T2?Dint?O\n"
	                                               "(cb:?DCallable,start=!0,:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N"),
	TYPE_KWMETHOD(STR_any, &default_seq_any, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_all, &default_seq_all, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_parity, &default_seq_parity, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_reduce, &default_seq_reduce, "(combine:?DCallable,start=!0,:?Dint=!A!Dint!PSIZE_MAX,init?)->"),
	TYPE_KWMETHOD(STR_min, &default_seq_min, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N")
	TYPE_KWMETHOD(STR_max, &default_seq_max, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?O?N"),
	TYPE_KWMETHOD(STR_sum, &default_seq_sum, "(start=!0,:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N"),
	TYPE_KWMETHOD(STR_count, &default_seq_count, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_contains, &default_seq_contains, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_locate, &default_seq_locate, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_KWMETHOD(STR_rlocate, &default_seq_rlocate, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_KWMETHOD(STR_startswith, &default_seq_startswith, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_endswith, &default_seq_endswith, "(item,start=!0,:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_find, &default_seq_find, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_rfind, &default_seq_rfind, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_index, &default_seq_index, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_rindex, &default_seq_rindex, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_reversed, &default_seq_reversed, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence"),
	TYPE_KWMETHOD(STR_sorted, &default_seq_sorted, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?DSequence"),
	TYPE_KWMETHOD(STR_insert, &default_seq_insert, "(index:?Dint,item)"),
	TYPE_KWMETHOD(STR_insertall, &default_seq_insertall, "(index:?Dint,items:?DSequence)"),
	TYPE_METHOD(STR_append, &default_seq_append, "(item)"),
	TYPE_METHOD(STR_extend, &default_seq_extend, "(items:?DSequence)"),
	TYPE_KWMETHOD(STR_erase, &default_seq_erase, "(index:?Dint,count=!1)"),
	TYPE_KWMETHOD(STR_xchitem, &default_seq_xchitem, "(index:?Dint,value)->"),
	TYPE_KWMETHOD(STR_pop, &default_seq_pop, "(index=!-1)->"),
	TYPE_METHOD(STR_popfront, &default_seq_popfront, "->"),
	TYPE_METHOD(STR_popback, &default_seq_popback, "->"),
	TYPE_METHOD(STR_pushfront, &default_seq_pushfront, "(item)"),
	TYPE_KWMETHOD(STR_remove, &default_seq_remove, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_rremove, &default_seq_rremove, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool"),
	TYPE_KWMETHOD(STR_removeall, &default_seq_removeall, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_removeif, &default_seq_removeif, "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint"),
	TYPE_METHOD(STR_clear, &default_seq_clear, "()"),
	TYPE_KWMETHOD(STR_resize, &default_seq_resize, "(size:?Dint,filler=!N)"),
	TYPE_KWMETHOD(STR_fill, &default_seq_fill, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)"),
	TYPE_KWMETHOD(STR_reverse, &default_seq_reverse, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)"),
	TYPE_KWMETHOD(STR_sort, &default_seq_sort, "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)"),
	TYPE_KWMETHOD(STR_bfind, &default_seq_bfind, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_bposition, &default_seq_bposition, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint"),
	TYPE_KWMETHOD(STR_brange, &default_seq_brange, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?Dint"),
	TYPE_KWMETHOD(STR_blocate, &default_seq_blocate, "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->"),
	TYPE_METHOD_END
};
#endif

DECL_END

/* Define mutable sequence function implementation selectors */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_SeqCache_RequireAny
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAll
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParity
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduce
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithInit
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMin
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMax
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSum
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSumWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCount
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContains
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocate
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRLocateWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWith
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWith
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFind
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFindWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRFind
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRFindWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireErase
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsert
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsertAll
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePushFront
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAppend
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireExtend
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireXchItemIndex
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireClear
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePop
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemove
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemove
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAll
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveIf
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireResize
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFill
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReverse
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReversed
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSort
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSorted
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortedWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBFind
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBFindWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBPosition
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBPositionWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBRange
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBRangeWithKey
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBLocate
#include "default-api-methods-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBLocateWithKey
#include "default-api-methods-require-impl.c.inl"
#endif /* !__INTELLISENSE__ */


/* Define attribute proxy implementations */
#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_DefaultFooWithCallAttrFoo
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataFunction
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataMethod
#include "default-api-methods-attrproxy-impl.c.inl"
#define DEFINE_DeeSeq_DefaultFooWithCallFooDataKwMethod
#include "default-api-methods-attrproxy-impl.c.inl"
#endif /* !__INTELLISENSE__ */


/* Define implementations of bsearch functions */
#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#define DEFINE_DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex
#include "default-api-methods-bsearch-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_METHODS_C */
