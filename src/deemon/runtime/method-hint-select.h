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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/method-hints.h>
#include <deemon/object.h>

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMhInitSelectDecls from "..method-hints.method-hints")();]]]*/
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bool_t DCALL mh_select_seq_operator_bool(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_sizeob_t DCALL mh_select_seq_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_size_t DCALL mh_select_seq_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iter_t DCALL mh_select_seq_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_t DCALL mh_select_seq_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_t DCALL mh_select_seq_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_index_t DCALL mh_select_seq_operator_getitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_t DCALL mh_select_seq_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_index_t DCALL mh_select_seq_operator_trygetitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_t DCALL mh_select_seq_operator_hasitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_index_t DCALL mh_select_seq_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_t DCALL mh_select_seq_operator_bounditem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_index_t DCALL mh_select_seq_operator_bounditem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_t DCALL mh_select_seq_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_index_t DCALL mh_select_seq_operator_delitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_t DCALL mh_select_seq_operator_setitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_index_t DCALL mh_select_seq_operator_setitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_t DCALL mh_select_seq_operator_getrange(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_t DCALL mh_select_seq_operator_getrange_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_n_t DCALL mh_select_seq_operator_getrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_t DCALL mh_select_seq_operator_delrange(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_index_t DCALL mh_select_seq_operator_delrange_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_index_n_t DCALL mh_select_seq_operator_delrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_t DCALL mh_select_seq_operator_setrange(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_index_t DCALL mh_select_seq_operator_setrange_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_index_n_t DCALL mh_select_seq_operator_setrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_assign_t DCALL mh_select_seq_operator_assign(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hash_t DCALL mh_select_seq_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_compare_t DCALL mh_select_seq_operator_compare(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_compare_eq_t DCALL mh_select_seq_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trycompare_eq_t DCALL mh_select_seq_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_eq_t DCALL mh_select_seq_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ne_t DCALL mh_select_seq_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_lo_t DCALL mh_select_seq_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_le_t DCALL mh_select_seq_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_gr_t DCALL mh_select_seq_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ge_t DCALL mh_select_seq_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_add_t DCALL mh_select_seq_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_mul_t DCALL mh_select_seq_operator_inplace_mul(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_t DCALL mh_select_seq_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_index_t DCALL mh_select_seq_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_t DCALL mh_select_seq_makeenumeration(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_intrange_t DCALL mh_select_seq_makeenumeration_with_intrange(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_range_t DCALL mh_select_seq_makeenumeration_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_foreach_reverse_t DCALL mh_select_seq_foreach_reverse(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_index_reverse_t DCALL mh_select_seq_enumerate_index_reverse(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetfirst_t DCALL mh_select_seq_trygetfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_getfirst_t DCALL mh_select_seq_getfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_boundfirst_t DCALL mh_select_seq_boundfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_delfirst_t DCALL mh_select_seq_delfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_setfirst_t DCALL mh_select_seq_setfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetlast_t DCALL mh_select_seq_trygetlast(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_getlast_t DCALL mh_select_seq_getlast(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_boundlast_t DCALL mh_select_seq_boundlast(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_dellast_t DCALL mh_select_seq_dellast(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_setlast_t DCALL mh_select_seq_setlast(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_cached_t DCALL mh_select_seq_cached(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_frozen_t DCALL mh_select_seq_frozen(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_t DCALL mh_select_seq_any(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_key_t DCALL mh_select_seq_any_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_t DCALL mh_select_seq_any_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_and_key_t DCALL mh_select_seq_any_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_t DCALL mh_select_seq_all(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_key_t DCALL mh_select_seq_all_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_t DCALL mh_select_seq_all_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_and_key_t DCALL mh_select_seq_all_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_t DCALL mh_select_seq_parity(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_key_t DCALL mh_select_seq_parity_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_range_t DCALL mh_select_seq_parity_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_range_and_key_t DCALL mh_select_seq_parity_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_t DCALL mh_select_seq_reduce(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_init_t DCALL mh_select_seq_reduce_with_init(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_range_t DCALL mh_select_seq_reduce_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_range_and_init_t DCALL mh_select_seq_reduce_with_range_and_init(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_t DCALL mh_select_seq_min(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_key_t DCALL mh_select_seq_min_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_range_t DCALL mh_select_seq_min_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_range_and_key_t DCALL mh_select_seq_min_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_t DCALL mh_select_seq_max(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_key_t DCALL mh_select_seq_max_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_range_t DCALL mh_select_seq_max_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_range_and_key_t DCALL mh_select_seq_max_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sum_t DCALL mh_select_seq_sum(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sum_with_range_t DCALL mh_select_seq_sum_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_t DCALL mh_select_seq_count(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_key_t DCALL mh_select_seq_count_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_range_t DCALL mh_select_seq_count_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_range_and_key_t DCALL mh_select_seq_count_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_t DCALL mh_select_seq_contains(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_key_t DCALL mh_select_seq_contains_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_range_t DCALL mh_select_seq_contains_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_range_and_key_t DCALL mh_select_seq_contains_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_contains_t DCALL mh_select_seq_operator_contains(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_locate_t DCALL mh_select_seq_locate(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_locate_with_range_t DCALL mh_select_seq_locate_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rlocate_t DCALL mh_select_seq_rlocate(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rlocate_with_range_t DCALL mh_select_seq_rlocate_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_t DCALL mh_select_seq_startswith(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_key_t DCALL mh_select_seq_startswith_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_range_t DCALL mh_select_seq_startswith_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_range_and_key_t DCALL mh_select_seq_startswith_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_t DCALL mh_select_seq_endswith(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_key_t DCALL mh_select_seq_endswith_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_range_t DCALL mh_select_seq_endswith_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_range_and_key_t DCALL mh_select_seq_endswith_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_t DCALL mh_select_seq_find(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_with_key_t DCALL mh_select_seq_find_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rfind_t DCALL mh_select_seq_rfind(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rfind_with_key_t DCALL mh_select_seq_rfind_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_erase_t DCALL mh_select_seq_erase(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_insert_t DCALL mh_select_seq_insert(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_insertall_t DCALL mh_select_seq_insertall(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_pushfront_t DCALL mh_select_seq_pushfront(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_append_t DCALL mh_select_seq_append(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_extend_t DCALL mh_select_seq_extend(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_xchitem_index_t DCALL mh_select_seq_xchitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_clear_t DCALL mh_select_seq_clear(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_pop_t DCALL mh_select_seq_pop(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_remove_t DCALL mh_select_seq_remove(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_remove_with_key_t DCALL mh_select_seq_remove_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rremove_t DCALL mh_select_seq_rremove(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rremove_with_key_t DCALL mh_select_seq_rremove_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeall_t DCALL mh_select_seq_removeall(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeall_with_key_t DCALL mh_select_seq_removeall_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeif_t DCALL mh_select_seq_removeif(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_resize_t DCALL mh_select_seq_resize(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_fill_t DCALL mh_select_seq_fill(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reverse_t DCALL mh_select_seq_reverse(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reversed_t DCALL mh_select_seq_reversed(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sort_t DCALL mh_select_seq_sort(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sort_with_key_t DCALL mh_select_seq_sort_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sorted_t DCALL mh_select_seq_sorted(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sorted_with_key_t DCALL mh_select_seq_sorted_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bfind_t DCALL mh_select_seq_bfind(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bfind_with_key_t DCALL mh_select_seq_bfind_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bposition_t DCALL mh_select_seq_bposition(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bposition_with_key_t DCALL mh_select_seq_bposition_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_brange_t DCALL mh_select_seq_brange(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_brange_with_key_t DCALL mh_select_seq_brange_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_iter_t DCALL mh_select_set_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_t DCALL mh_select_set_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_pair_t DCALL mh_select_set_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_sizeob_t DCALL mh_select_set_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_size_t DCALL mh_select_set_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_hash_t DCALL mh_select_set_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_compare_eq_t DCALL mh_select_set_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_trycompare_eq_t DCALL mh_select_set_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_eq_t DCALL mh_select_set_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_ne_t DCALL mh_select_set_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_lo_t DCALL mh_select_set_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_le_t DCALL mh_select_set_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_gr_t DCALL mh_select_set_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_ge_t DCALL mh_select_set_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_add_t DCALL mh_select_set_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_sub_t DCALL mh_select_set_operator_inplace_sub(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_and_t DCALL mh_select_set_operator_inplace_and(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_xor_t DCALL mh_select_set_operator_inplace_xor(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_frozen_t DCALL mh_select_set_frozen(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_unify_t DCALL mh_select_set_unify(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_insert_t DCALL mh_select_set_insert(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_insertall_t DCALL mh_select_set_insertall(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_remove_t DCALL mh_select_set_remove(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_removeall_t DCALL mh_select_set_removeall(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_pop_t DCALL mh_select_set_pop(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_pop_with_default_t DCALL mh_select_set_pop_with_default(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_t DCALL mh_select_map_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_t DCALL mh_select_map_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_index_t DCALL mh_select_map_operator_getitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_index_t DCALL mh_select_map_operator_trygetitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_string_hash_t DCALL mh_select_map_operator_getitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_string_hash_t DCALL mh_select_map_operator_trygetitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_string_len_hash_t DCALL mh_select_map_operator_getitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_string_len_hash_t DCALL mh_select_map_operator_trygetitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_t DCALL mh_select_map_operator_bounditem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_index_t DCALL mh_select_map_operator_bounditem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_string_hash_t DCALL mh_select_map_operator_bounditem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_string_len_hash_t DCALL mh_select_map_operator_bounditem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_t DCALL mh_select_map_operator_hasitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_index_t DCALL mh_select_map_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_hash_t DCALL mh_select_map_operator_hasitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_len_hash_t DCALL mh_select_map_operator_hasitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_t DCALL mh_select_map_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_index_t DCALL mh_select_map_operator_delitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_string_hash_t DCALL mh_select_map_operator_delitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_string_len_hash_t DCALL mh_select_map_operator_delitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_t DCALL mh_select_map_operator_setitem(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_index_t DCALL mh_select_map_operator_setitem_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_string_hash_t DCALL mh_select_map_operator_setitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_string_len_hash_t DCALL mh_select_map_operator_setitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_contains_t DCALL mh_select_map_operator_contains(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_keys_t DCALL mh_select_map_keys(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_iterkeys_t DCALL mh_select_map_iterkeys(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_values_t DCALL mh_select_map_values(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_itervalues_t DCALL mh_select_map_itervalues(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_enumerate_t DCALL mh_select_map_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_enumerate_range_t DCALL mh_select_map_enumerate_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_compare_eq_t DCALL mh_select_map_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trycompare_eq_t DCALL mh_select_map_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_eq_t DCALL mh_select_map_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_ne_t DCALL mh_select_map_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_lo_t DCALL mh_select_map_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_le_t DCALL mh_select_map_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_gr_t DCALL mh_select_map_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_ge_t DCALL mh_select_map_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_add_t DCALL mh_select_map_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_sub_t DCALL mh_select_map_operator_inplace_sub(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_and_t DCALL mh_select_map_operator_inplace_and(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_xor_t DCALL mh_select_map_operator_inplace_xor(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_frozen_t DCALL mh_select_map_frozen(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setold_t DCALL mh_select_map_setold(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setold_ex_t DCALL mh_select_map_setold_ex(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setnew_t DCALL mh_select_map_setnew(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setnew_ex_t DCALL mh_select_map_setnew_ex(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setdefault_t DCALL mh_select_map_setdefault(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_update_t DCALL mh_select_map_update(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_remove_t DCALL mh_select_map_remove(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_removekeys_t DCALL mh_select_map_removekeys(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_pop_t DCALL mh_select_map_pop(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_pop_with_default_t DCALL mh_select_map_pop_with_default(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_popitem_t DCALL mh_select_map_popitem(DeeTypeObject *self, DeeTypeObject *orig_type);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H */
