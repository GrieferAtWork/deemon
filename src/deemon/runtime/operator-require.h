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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_REQUIRE_H
#define GUARD_DEEMON_RUNTIME_OPERATOR_REQUIRE_H 1

#include <deemon/api.h>

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#include <deemon/object.h>

DECL_BEGIN

#define DeeType_RequireBool(tp_self)                    (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))
#define DeeType_RequireIter(tp_self)                    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireSizeOb(tp_self)                  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_sizeob) || DeeType_InheritSize(tp_self))
#define DeeType_RequireSize(tp_self)                    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size) || DeeType_InheritSize(tp_self))
#define DeeType_RequireSizeFast(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size_fast) || DeeType_InheritSize(tp_self))
#define DeeType_RequireContains(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_contains) || DeeType_InheritContains(tp_self))
#define DeeType_RequireForeach(tp_self)                 (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeachPair(tp_self)             (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerate(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate))
#define DeeType_RequireEnumerateIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireForeachAndForeachPair(tp_self)   (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach && (tp_self)->tp_seq->tp_foreach_pair) || DeeType_InheritIter(tp_self))
#define DeeType_RequireIterKeys(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iterkeys) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_iterkeys))
#define DeeType_RequireGetItem(tp_self)                 (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireGetItemIndex(tp_self)            (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireGetItemStringHash(tp_self)       (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_string_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireGetItemStringLenHash(tp_self)    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem_string_len_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItem(tp_self)              (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItemIndex(tp_self)         (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItemStringHash(tp_self)    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem_string_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireTryGetItemStringLenHash(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_trygetitem_string_len_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItem(tp_self)               (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItemIndex(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItemStringHash(tp_self)     (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem_string_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItemStringLenHash(tp_self)  (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem_string_len_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItem(tp_self)                 (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItemIndex(tp_self)            (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItemStringHash(tp_self)       (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem_string_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireHasItemStringLenHash(tp_self)    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_hasitem_string_len_hash) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItem(tp_self)                 (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireDelItemIndex(tp_self)            (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireDelItemStringHash(tp_self)       (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_string_hash) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireDelItemStringLenHash(tp_self)    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem_string_len_hash) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItem(tp_self)                 (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireSetItemIndex(tp_self)            (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireSetItemStringHash(tp_self)       (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_string_hash) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireSetItemStringLenHash(tp_self)    (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem_string_len_hash) || DeeType_InheritSetItem(tp_self))
#define DeeType_RequireGetRange(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireGetRangeIndex(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireGetRangeIndexN(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getrange_index_n) || DeeType_InheritGetRange(tp_self))
#define DeeType_RequireDelRange(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireDelRangeIndex(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireDelRangeIndexN(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delrange_index_n) || DeeType_InheritDelRange(tp_self))
#define DeeType_RequireSetRange(tp_self)                (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireSetRangeIndex(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireSetRangeIndexN(tp_self)          (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setrange_index_n) || DeeType_InheritSetRange(tp_self))
#define DeeType_RequireHash(tp_self)                    (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_hash) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireCompareEq(tp_self)               (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_compare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireCompare(tp_self)                 (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_compare) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireTryCompareEq(tp_self)            (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_trycompare_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireEq(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_eq) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireNe(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ne) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLo(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_lo) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireLe(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_le) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGr(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_gr) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireGe(tp_self)                      (((tp_self)->tp_cmp && (tp_self)->tp_cmp->tp_ge) || DeeType_InheritCompare(tp_self))
#define DeeType_RequireInplaceAdd(tp_self)              (((tp_self)->tp_math && (tp_self)->tp_math->tp_inplace_add) || DeeType_InheritAdd(tp_self))
#define DeeType_RequireInplaceMul(tp_self)              (((tp_self)->tp_math && (tp_self)->tp_math->tp_inplace_mul) || DeeType_InheritMul(tp_self))

DECL_END
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_REQUIRE_H */
