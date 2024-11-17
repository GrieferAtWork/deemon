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

#define DeeType_RequireIter(tp_self)                  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireSizeOb(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_sizeob) || DeeType_InheritSize(tp_self))
#define DeeType_RequireSize(tp_self)                  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size) || DeeType_InheritSize(tp_self))
#define DeeType_RequireContains(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_contains) || DeeType_InheritContains(tp_self))
#define DeeType_RequireForeach(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeachPair(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerate(tp_self)             (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate))
#define DeeType_RequireEnumerateIndex(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireForeachAndForeachPair(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireGetItem(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireGetItemIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItem(tp_self)            (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItemIndex(tp_self)       (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItem(tp_self)             (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItemIndex(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItem(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItemIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItem(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireDelItemIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItem(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireSetItemIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireGetRange(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireGetRangeIndex(tp_self)         (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireGetRangeIndexN(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index_n) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireDelRange(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireDelRangeIndex(tp_self)         (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireDelRangeIndexN(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index_n) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireSetRange(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireSetRangeIndex(tp_self)         (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireSetRangeIndexN(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index_n) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireHash(tp_self)                  (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_hash) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireCompareEq(tp_self)             (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_compare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireCompare(tp_self)               (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_compare) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireTryCompareEq(tp_self)          (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_trycompare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireEq(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireNe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ne) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLo(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_lo) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_le) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGr(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_gr) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGe(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ge) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireBool(tp_self)                  (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))

PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_trygetfirst_t DCALL DeeType_SeqCache_RequireTryGetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_getfirst_t DCALL DeeType_SeqCache_RequireGetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_boundfirst_t DCALL DeeType_SeqCache_RequireBoundFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_delfirst_t DCALL DeeType_SeqCache_RequireDelFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_setfirst_t DCALL DeeType_SeqCache_RequireSetFirst_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_trygetlast_t DCALL DeeType_SeqCache_RequireTryGetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_getlast_t DCALL DeeType_SeqCache_RequireGetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_boundlast_t DCALL DeeType_SeqCache_RequireBoundLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_dellast_t DCALL DeeType_SeqCache_RequireDelLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_setlast_t DCALL DeeType_SeqCache_RequireSetLast_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
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
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_insert_t DCALL DeeType_SeqCache_RequireSetInsert_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_remove_t DCALL DeeType_SeqCache_RequireSetRemove_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_unify_t DCALL DeeType_SeqCache_RequireSetUnify_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_insertall_t DCALL DeeType_SeqCache_RequireSetInsertAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_removeall_t DCALL DeeType_SeqCache_RequireSetRemoveAll_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_pop_t DCALL DeeType_SeqCache_RequireSetPop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_set_pop_with_default_t DCALL DeeType_SeqCache_RequireSetPopWithDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_setold_t DCALL DeeType_SeqCache_RequireMapSetOld_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_setold_ex_t DCALL DeeType_SeqCache_RequireMapSetOldEx_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_setnew_t DCALL DeeType_SeqCache_RequireMapSetNew_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_setnew_ex_t DCALL DeeType_SeqCache_RequireMapSetNewEx_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_setdefault_t DCALL DeeType_SeqCache_RequireMapSetDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_update_t DCALL DeeType_SeqCache_RequireMapUpdate_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_removekeys_t DCALL DeeType_SeqCache_RequireMapRemoveKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_pop_t DCALL DeeType_SeqCache_RequireMapPop_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_pop_with_default_t DCALL DeeType_SeqCache_RequireMapPopWithDefault_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_popitem_t DCALL DeeType_SeqCache_RequireMapPopItem_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_keys_t DCALL DeeType_SeqCache_RequireMapKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_values_t DCALL DeeType_SeqCache_RequireMapValues_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_iterkeys_t DCALL DeeType_SeqCache_RequireMapIterKeys_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_tsc_map_itervalues_t DCALL DeeType_SeqCache_RequireMapIterValues_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self);

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_trygetfirst_t DCALL DeeType_SeqCache_RequireTryGetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getfirst_t DCALL DeeType_SeqCache_RequireGetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundfirst_t DCALL DeeType_SeqCache_RequireBoundFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_delfirst_t DCALL DeeType_SeqCache_RequireDelFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setfirst_t DCALL DeeType_SeqCache_RequireSetFirst_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_trygetlast_t DCALL DeeType_SeqCache_RequireTryGetLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getlast_t DCALL DeeType_SeqCache_RequireGetLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundlast_t DCALL DeeType_SeqCache_RequireBoundLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_dellast_t DCALL DeeType_SeqCache_RequireDelLast_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setlast_t DCALL DeeType_SeqCache_RequireSetLast_uncached(DeeTypeObject *__restrict self);
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
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insert_t DCALL DeeType_SeqCache_RequireSetInsert_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_remove_t DCALL DeeType_SeqCache_RequireSetRemove_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_unify_t DCALL DeeType_SeqCache_RequireSetUnify_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insertall_t DCALL DeeType_SeqCache_RequireSetInsertAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_removeall_t DCALL DeeType_SeqCache_RequireSetRemoveAll_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_t DCALL DeeType_SeqCache_RequireSetPop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_with_default_t DCALL DeeType_SeqCache_RequireSetPopWithDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_t DCALL DeeType_SeqCache_RequireMapSetOld_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_ex_t DCALL DeeType_SeqCache_RequireMapSetOldEx_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_t DCALL DeeType_SeqCache_RequireMapSetNew_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_ex_t DCALL DeeType_SeqCache_RequireMapSetNewEx_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setdefault_t DCALL DeeType_SeqCache_RequireMapSetDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_update_t DCALL DeeType_SeqCache_RequireMapUpdate_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_removekeys_t DCALL DeeType_SeqCache_RequireMapRemoveKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_t DCALL DeeType_SeqCache_RequireMapPop_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_with_default_t DCALL DeeType_SeqCache_RequireMapPopWithDefault_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_popitem_t DCALL DeeType_SeqCache_RequireMapPopItem_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_keys_t DCALL DeeType_SeqCache_RequireMapKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_values_t DCALL DeeType_SeqCache_RequireMapValues_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_iterkeys_t DCALL DeeType_SeqCache_RequireMapIterKeys_uncached(DeeTypeObject *__restrict self);
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_itervalues_t DCALL DeeType_SeqCache_RequireMapIterValues_uncached(DeeTypeObject *__restrict self);


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
	if (self->tsc_getfirst == &DeeSeq_DefaultGetFirstWithCallGetFirstDataFunction ||
	    self->tsc_boundfirst == &DeeSeq_DefaultBoundFirstWithCallGetFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_getfirst_data);
	if (self->tsc_delfirst == &DeeSeq_DefaultDelFirstWithCallDelFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_delfirst_data);
	if (self->tsc_setfirst == &DeeSeq_DefaultSetFirstWithCallSetFirstDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_setfirst_data);
	if (self->tsc_getlast == &DeeSeq_DefaultGetLastWithCallGetLastDataFunction ||
	    self->tsc_boundlast == &DeeSeq_DefaultBoundLastWithCallGetLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_getlast_data);
	if (self->tsc_dellast == &DeeSeq_DefaultDelLastWithCallDelLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_dellast_data);
	if (self->tsc_setlast == &DeeSeq_DefaultSetLastWithCallSetLastDataFunction)
		Dee_tsc_uslot_fini_function(&self->tsc_setlast_data);
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

DECL_END

/* Define sequence function implementation selectors */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_SeqCache_RequireTryGetFirst
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireGetFirst
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBoundFirst
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireDelFirst
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetFirst
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireTryGetLast
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireGetLast
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBoundLast
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireDelLast
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetLast
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAny
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAll
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParity
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduce
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithInit
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMin
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMax
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSum
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSumWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCount
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContains
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocate
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRLocateWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWith
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWith
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFind
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFindWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRFind
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRFindWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireErase
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsert
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireInsertAll
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePushFront
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireAppend
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireExtend
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireXchItemIndex
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireClear
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequirePop
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemove
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemove
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAll
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireRemoveIf
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireResize
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireFill
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReverse
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireReversed
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSort
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSorted
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSortedWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBFind
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBFindWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBPosition
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBPositionWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBRange
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBRangeWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBLocate
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBLocateWithKey
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetInsert
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetRemove
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetUnify
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetInsertAll
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetRemoveAll
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetPop
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetPopWithDefault
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapSetOld
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapSetOldEx
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapSetNew
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapSetNewEx
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapSetDefault
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapUpdate
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapRemove
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapRemoveKeys
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapPop
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapPopWithDefault
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapPopItem
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapKeys
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapValues
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapIterKeys
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireMapIterValues
#include "default-api-require-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C */
