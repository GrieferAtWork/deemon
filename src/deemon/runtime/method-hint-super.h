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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/super.h>

/**/
#include "method-hints.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printSuperMethodHintWrappers from "..method-hints.method-hints")(impl: false);]]]*/
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_bool(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_sizeob(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL super_mh__seq_operator_size(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_iter(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__seq_operator_foreach(DeeSuperObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__seq_operator_foreach_pair(DeeSuperObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_getitem(DeeSuperObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_getitem_index(DeeSuperObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_trygetitem(DeeSuperObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_trygetitem_index(DeeSuperObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_hasitem(DeeSuperObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_hasitem_index(DeeSuperObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_bounditem(DeeSuperObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_bounditem_index(DeeSuperObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_delitem(DeeSuperObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_delitem_index(DeeSuperObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__seq_operator_setitem(DeeSuperObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_operator_setitem_index(DeeSuperObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__seq_operator_getrange(DeeSuperObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_getrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_operator_getrange_index_n(DeeSuperObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__seq_operator_delrange(DeeSuperObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_delrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_operator_delrange_index_n(DeeSuperObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL super_mh__seq_operator_setrange(DeeSuperObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_operator_setrange_index(DeeSuperObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_operator_setrange_index_n(DeeSuperObject *self, Dee_ssize_t start, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_assign(DeeSuperObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL super_mh__seq_operator_hash(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_compare(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_ne(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_lo(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_le(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_gr(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_ge(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_add(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_mul(DeeSuperObject *self, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_inplace_add(DREF DeeSuperObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_operator_inplace_mul(DREF DeeSuperObject **__restrict p_lhs, DeeObject *repeat);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__seq_enumerate(DeeSuperObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__seq_enumerate_index(DeeSuperObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_makeenumeration(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__seq_makeenumeration_with_range(DeeSuperObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_makeenumeration_with_intrange(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_unpack(DeeSuperObject *__restrict self, size_t count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL super_mh__seq_unpack_ex(DeeSuperObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL super_mh__seq_unpack_ub(DeeSuperObject *__restrict self, size_t min_count, size_t max_count, DREF DeeObject *result[]);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_trygetfirst(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_getfirst(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_boundfirst(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_delfirst(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_setfirst(DeeSuperObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_trygetlast(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_getlast(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_boundlast(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_dellast(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_setlast(DeeSuperObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_cached(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_frozen(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_any(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_any_with_key(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_any_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_any_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_all(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_all_with_key(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_all_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_all_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_parity(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_parity_with_key(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_parity_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_parity_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_reduce(DeeSuperObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__seq_reduce_with_init(DeeSuperObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_reduce_with_range(DeeSuperObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL super_mh__seq_reduce_with_range_and_init(DeeSuperObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_min(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_min_with_key(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_min_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL super_mh__seq_min_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_max(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_max_with_key(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_max_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL super_mh__seq_max_with_range_and_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_sum(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_sum_with_range(DeeSuperObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_count(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL super_mh__seq_count_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_count_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL super_mh__seq_count_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_contains(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__seq_contains_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_contains_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_contains_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__seq_operator_contains(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__seq_locate(DeeSuperObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL super_mh__seq_locate_with_range(DeeSuperObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__seq_rlocate(DeeSuperObject *self, DeeObject *match, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL super_mh__seq_rlocate_with_range(DeeSuperObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_startswith(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__seq_startswith_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_startswith_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_startswith_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_endswith(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__seq_endswith_with_key(DeeSuperObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_endswith_with_range(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_endswith_with_range_and_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_find(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL super_mh__seq_find_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_rfind(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL super_mh__seq_rfind_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_erase(DeeSuperObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_insert(DeeSuperObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_insertall(DeeSuperObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_pushfront(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_append(DeeSuperObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_extend(DeeSuperObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL super_mh__seq_xchitem_index(DeeSuperObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_clear(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_pop(DeeSuperObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_remove(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_remove_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__seq_rremove(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_rremove_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_removeall(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL super_mh__seq_removeall_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_removeif(DeeSuperObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__seq_resize(DeeSuperObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_fill(DeeSuperObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_reverse(DeeSuperObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_reversed(DeeSuperObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__seq_sort(DeeSuperObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL super_mh__seq_sort_with_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__seq_sorted(DeeSuperObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL super_mh__seq_sorted_with_key(DeeSuperObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_bfind(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL super_mh__seq_bfind_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL super_mh__seq_bposition(DeeSuperObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL super_mh__seq_bposition_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL super_mh__seq_brange(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL super_mh__seq_brange_with_key(DeeSuperObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__set_operator_iter(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__set_operator_foreach(DeeSuperObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__set_operator_sizeob(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL super_mh__set_operator_size(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL super_mh__set_operator_hash(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_ne(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_lo(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_le(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_gr(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_ge(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__set_operator_inv(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_add(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_sub(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_and(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_operator_xor(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_inplace_add(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_inplace_sub(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_inplace_and(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_operator_inplace_xor(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__set_frozen(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_unify(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_insert(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_insertall(DeeSuperObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_remove(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__set_removeall(DeeSuperObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__set_pop(DeeSuperObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__set_pop_with_default(DeeSuperObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_iter(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__map_operator_foreach_pair(DeeSuperObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_sizeob(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL super_mh__map_operator_size(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_getitem(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_trygetitem(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_getitem_index(DeeSuperObject *self, size_t key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_trygetitem_index(DeeSuperObject *self, size_t key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_getitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_trygetitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_getitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_operator_trygetitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_bounditem(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_bounditem_index(DeeSuperObject *self, size_t key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_bounditem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_bounditem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_hasitem(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_hasitem_index(DeeSuperObject *self, size_t key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_hasitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_hasitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_delitem(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_delitem_index(DeeSuperObject *self, size_t key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_delitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) int DCALL super_mh__map_operator_delitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__map_operator_setitem(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL super_mh__map_operator_setitem_index(DeeSuperObject *self, size_t key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL super_mh__map_operator_setitem_string_hash(DeeSuperObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 5)) int DCALL super_mh__map_operator_setitem_string_len_hash(DeeSuperObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_contains(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_keys(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_iterkeys(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_values(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_itervalues(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL super_mh__map_enumerate(DeeSuperObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 4, 5)) Dee_ssize_t DCALL super_mh__map_enumerate_range(DeeSuperObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_makeenumeration(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__map_makeenumeration_with_range(DeeSuperObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_compare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_trycompare_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_eq(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_ne(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_lo(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_le(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_gr(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_ge(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_add(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_sub(DeeSuperObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_and(DeeSuperObject *lhs, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_operator_xor(DeeSuperObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_inplace_add(DREF DeeSuperObject **__restrict p_self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_inplace_sub(DREF DeeSuperObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_inplace_and(DREF DeeSuperObject **__restrict p_self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_operator_inplace_xor(DREF DeeSuperObject **__restrict p_self, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_frozen(DeeSuperObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__map_setold(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__map_setold_ex(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_mh__map_setnew(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__map_setnew_ex(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__map_setdefault(DeeSuperObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_update(DeeSuperObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_remove(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_mh__map_removekeys(DeeSuperObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_mh__map_pop(DeeSuperObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL super_mh__map_pop_with_default(DeeSuperObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL super_mh__map_popitem(DeeSuperObject *self);
/*[[[end]]]*/
/* clang-format on */

/* Fully initialized method hint cache for "Super" */
INTDEF struct Dee_type_mh_cache super_mhcache;

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SUPER_H */
