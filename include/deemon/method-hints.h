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
#ifndef GUARD_DEEMON_METHOD_HINTS_H
#define GUARD_DEEMON_METHOD_HINTS_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_type_method_hint type_method_hint
#endif /* DEE_SOURCE */

/*
 * Method hints are declaration pairs that come in the form of an entry in a
 * type's `tp_methods' array using a pre-defined function pointer (declared
 * in this header), as well as an entry in a type's `tp_method_hints' array,
 * which then points to the low-level C implementation of the function.
 *
 * These method hints are only available for certain, commonly overwritten,
 * default functions of objects, and are used to:
 * - Make it easier to override default functions without needing to re-write
 *   the same argument processing stub every time the function gets defined.
 * - Allow for fasting dispatching to the actual, underlying function within
 *   optimized _hostasm code.
 *
 *
 * >> PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
 * >> myob_setdefault(MyObject *self, DeeObject *key, DeeObject *value) {
 * >>     // This gets called by:
 * >>     // - `MyObject().setdefault(...)'
 * >>     // - `Mapping.setdefault(MyObject(), ...)'
 * >>     ...
 * >> }
 * >>
 * >> PRIVATE struct type_method_hint myob_method_hints[] = {
 * >>     TYPE_METHOD_HINT(map_setdefault, &myob_setdefault),
 * >>     TYPE_METHOD_HINT_END
 * >> };
 * >>
 * >> PRIVATE struct type_method myob_methods[] = {
 * >>     TYPE_METHOD_HINTREF(map_setdefault),
 * >>     TYPE_METHOD_END
 * >> };
 * >>
 * >> PRIVATE DeeTypeObject MyObject_Type = {
 * >>     ...
 * >>     .tp_methods      = myob_methods,
 * >>     .tp_method_hints = myob_method_hints,
 * >> };
 */


#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)

typedef WUNUSED_T NONNULL_T((2)) Dee_ssize_t (DCALL *Dee_seq_enumerate_t)(void *arg, DeeObject *index, /*nullable*/ DeeObject *value);
typedef WUNUSED_T Dee_ssize_t (DCALL *Dee_seq_enumerate_index_t)(void *arg, size_t index, /*nullable*/ DeeObject *value);

/* clang-format off */
/*[[[deemon (print_Dee_tmh_id from "...src.deemon.method-hints.method-hints")();]]]*/
/* !!! CAUTION !!! Method hint IDs are prone to arbitrarily change !!!
 *
 * Do not make use of these IDs if you're developing a DEX module and
 * wish to remain compatible with the deemon core across many version.
 * If that's want you're trying to accomplish, you should instead define
 * regular `tp_methods' with names recognized as method hints. */
enum Dee_tmh_id {
	Dee_TMH_seq_operator_bool,
	Dee_TMH_seq_operator_sizeob,
	Dee_TMH_seq_operator_size,
	Dee_TMH_seq_operator_iter,
	Dee_TMH_seq_operator_foreach,
	Dee_TMH_seq_operator_foreach_pair,
	Dee_TMH_seq_operator_getitem,
	Dee_TMH_seq_operator_getitem_index,
	Dee_TMH_seq_operator_trygetitem,
	Dee_TMH_seq_operator_trygetitem_index,
	Dee_TMH_seq_operator_hasitem,
	Dee_TMH_seq_operator_hasitem_index,
	Dee_TMH_seq_operator_bounditem,
	Dee_TMH_seq_operator_bounditem_index,
	Dee_TMH_seq_operator_delitem,
	Dee_TMH_seq_operator_delitem_index,
	Dee_TMH_seq_operator_setitem,
	Dee_TMH_seq_operator_setitem_index,
	Dee_TMH_seq_operator_getrange,
	Dee_TMH_seq_operator_getrange_index,
	Dee_TMH_seq_operator_getrange_index_n,
	Dee_TMH_seq_operator_delrange,
	Dee_TMH_seq_operator_delrange_index,
	Dee_TMH_seq_operator_delrange_index_n,
	Dee_TMH_seq_operator_setrange,
	Dee_TMH_seq_operator_setrange_index,
	Dee_TMH_seq_operator_setrange_index_n,
	Dee_TMH_seq_operator_assign,
	Dee_TMH_seq_operator_hash,
	Dee_TMH_seq_operator_compare,
	Dee_TMH_seq_operator_compare_eq,
	Dee_TMH_seq_operator_trycompare_eq,
	Dee_TMH_seq_operator_eq,
	Dee_TMH_seq_operator_ne,
	Dee_TMH_seq_operator_lo,
	Dee_TMH_seq_operator_le,
	Dee_TMH_seq_operator_gr,
	Dee_TMH_seq_operator_ge,
	Dee_TMH_seq_operator_inplace_add,
	Dee_TMH_seq_operator_inplace_mul,
	Dee_TMH_seq_enumerate,
	Dee_TMH_seq_enumerate_index,
	Dee_TMH_seq_makeenumeration,
	Dee_TMH_seq_makeenumeration_with_int_range,
	Dee_TMH_seq_makeenumeration_with_range,
	Dee_TMH_seq_foreach_reverse,
	Dee_TMH_seq_enumerate_index_reverse,
	Dee_TMH_seq_trygetfirst,
	Dee_TMH_seq_getfirst,
	Dee_TMH_seq_boundfirst,
	Dee_TMH_seq_delfirst,
	Dee_TMH_seq_setfirst,
	Dee_TMH_seq_trygetlast,
	Dee_TMH_seq_getlast,
	Dee_TMH_seq_boundlast,
	Dee_TMH_seq_dellast,
	Dee_TMH_seq_setlast,
	Dee_TMH_seq_any,
	Dee_TMH_seq_any_with_key,
	Dee_TMH_seq_any_with_range,
	Dee_TMH_seq_any_with_range_and_key,
	Dee_TMH_seq_all,
	Dee_TMH_seq_all_with_key,
	Dee_TMH_seq_all_with_range,
	Dee_TMH_seq_all_with_range_and_key,
	Dee_TMH_seq_parity,
	Dee_TMH_seq_parity_with_key,
	Dee_TMH_seq_parity_with_range,
	Dee_TMH_seq_parity_with_range_and_key,
	Dee_TMH_seq_min,
	Dee_TMH_seq_min_with_key,
	Dee_TMH_seq_min_with_range,
	Dee_TMH_seq_min_with_range_and_key,
	Dee_TMH_seq_max,
	Dee_TMH_seq_max_with_key,
	Dee_TMH_seq_max_with_range,
	Dee_TMH_seq_max_with_range_and_key,
	Dee_TMH_seq_sum,
	Dee_TMH_seq_sum_with_range,
	Dee_TMH_seq_count,
	Dee_TMH_seq_count_with_key,
	Dee_TMH_seq_count_with_range,
	Dee_TMH_seq_count_with_range_and_key,
	Dee_TMH_seq_contains,
	Dee_TMH_seq_contains_with_key,
	Dee_TMH_seq_contains_with_range,
	Dee_TMH_seq_contains_with_range_and_key,
	Dee_TMH_seq_operator_contains,
	Dee_TMH_seq_locate,
	Dee_TMH_seq_locate_with_range,
	Dee_TMH_seq_rlocate,
	Dee_TMH_seq_rlocate_with_range,
	Dee_TMH_seq_startswith,
	Dee_TMH_seq_startswith_with_key,
	Dee_TMH_seq_startswith_with_range,
	Dee_TMH_seq_startswith_with_range_and_key,
	Dee_TMH_seq_endswith,
	Dee_TMH_seq_endswith_with_key,
	Dee_TMH_seq_endswith_with_range,
	Dee_TMH_seq_endswith_with_range_and_key,
	Dee_TMH_seq_find,
	Dee_TMH_seq_find_with_key,
	Dee_TMH_seq_rfind,
	Dee_TMH_seq_rfind_with_key,
	Dee_TMH_seq_erase,
	Dee_TMH_seq_insert,
	Dee_TMH_seq_insertall,
	Dee_TMH_seq_pushfront,
	Dee_TMH_seq_append,
	Dee_TMH_seq_extend,
	Dee_TMH_seq_xchitem_index,
	Dee_TMH_seq_clear,
	Dee_TMH_seq_pop,
	Dee_TMH_set_operator_iter,
	Dee_TMH_set_operator_foreach,
	Dee_TMH_set_operator_foreach_pair,
	Dee_TMH_set_operator_sizeob,
	Dee_TMH_set_operator_size,
	Dee_TMH_set_operator_hash,
	Dee_TMH_set_operator_compare_eq,
	Dee_TMH_set_operator_trycompare_eq,
	Dee_TMH_set_operator_eq,
	Dee_TMH_set_operator_ne,
	Dee_TMH_map_operator_getitem,
	Dee_TMH_map_operator_trygetitem,
	Dee_TMH_map_operator_getitem_index,
	Dee_TMH_map_operator_trygetitem_index,
	Dee_TMH_map_operator_getitem_string_hash,
	Dee_TMH_map_operator_trygetitem_string_hash,
	Dee_TMH_map_operator_getitem_string_len_hash,
	Dee_TMH_map_operator_trygetitem_string_len_hash,
	Dee_TMH_map_operator_bounditem,
	Dee_TMH_map_operator_bounditem_index,
	Dee_TMH_map_operator_bounditem_string_hash,
	Dee_TMH_map_operator_bounditem_string_len_hash,
	Dee_TMH_map_operator_hasitem,
	Dee_TMH_map_operator_hasitem_index,
	Dee_TMH_map_operator_hasitem_string_hash,
	Dee_TMH_map_operator_hasitem_string_len_hash,
	Dee_TMH_map_operator_delitem,
	Dee_TMH_map_operator_delitem_index,
	Dee_TMH_map_operator_delitem_string_hash,
	Dee_TMH_map_operator_delitem_string_len_hash,
	Dee_TMH_map_operator_setitem,
	Dee_TMH_map_operator_setitem_index,
	Dee_TMH_map_operator_setitem_string_hash,
	Dee_TMH_map_operator_setitem_string_len_hash,
	Dee_TMH_map_operator_contains,
	Dee_TMH_map_iterkeys,
	Dee_TMH_map_enumerate,
	Dee_TMH_map_enumerate_range,
	Dee_TMH_COUNT
};
/*[[[end]]]*/

/*[[[deemon (printMHTypedefs from "...src.deemon.method-hints.method-hints")();]]]*/
/* __seq_bool__ */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_bool_t)(DeeObject *__restrict self);

/* __seq_size__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_sizeob_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeMH_seq_operator_size_t)(DeeObject *__restrict self);

/* __seq_iter__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_iter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_operator_foreach_t)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_operator_foreach_pair_t)(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* __seq_getitem__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_getitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_getitem_index_t)(DeeObject *__restrict self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_trygetitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_trygetitem_index_t)(DeeObject *__restrict self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_hasitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_hasitem_index_t)(DeeObject *__restrict self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_bounditem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_bounditem_index_t)(DeeObject *__restrict self, size_t index);

/* __seq_delitem__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_delitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_delitem_index_t)(DeeObject *__restrict self, size_t index);

/* __seq_setitem__ */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_seq_operator_setitem_t)(DeeObject *self, DeeObject *index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeMH_seq_operator_setitem_index_t)(DeeObject *self, size_t index, DeeObject *value);

/* __seq_getrange__ */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeMH_seq_operator_getrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_getrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_operator_getrange_index_n_t)(DeeObject *self, Dee_ssize_t start);

/* __seq_delrange__ */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_seq_operator_delrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_delrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_operator_delrange_index_n_t)(DeeObject *self, Dee_ssize_t start);

/* __seq_setrange__ */
typedef WUNUSED_T NONNULL_T((1, 2, 3, 4)) int (DCALL *DeeMH_seq_operator_setrange_t)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeMH_seq_operator_setrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeMH_seq_operator_setrange_index_n_t)(DeeObject *self, Dee_ssize_t start, DeeObject *items);

/* __seq_assign__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_assign_t)(DeeObject *self, DeeObject *items);

/* __seq_hash__ */
typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *DeeMH_seq_operator_hash_t)(DeeObject *__restrict self);

/* __seq_compare__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_compare_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_compare_eq__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_compare_eq_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_trycompare_eq__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_trycompare_eq_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_eq__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_eq_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_ne__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_ne_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_lo__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_lo_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_le__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_le_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_gr__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_gr_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_ge__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_ge_t)(DeeObject *lhs, DeeObject *rhs);

/* __seq_inplace_add__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_inplace_add_t)(DREF DeeObject **__restrict p_self, DeeObject *rhs);

/* __seq_inplace_mul__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_operator_inplace_mul_t)(DREF DeeObject **__restrict p_self, DeeObject *repeat);

/* __seq_enumerate__ */
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_enumerate_t)(DeeObject *__restrict self, Dee_seq_enumerate_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_enumerate_index_t)(DeeObject *__restrict self, Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end);

/* __seq_enumerate_items__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_makeenumeration_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_makeenumeration_with_int_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeMH_seq_makeenumeration_with_range_t)(DeeObject *self, DeeObject *start, DeeObject *end);

/* Sequence_first, __seq_first__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_trygetfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_getfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_boundfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_delfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_setfirst_t)(DeeObject *self, DeeObject *value);

/* Sequence_last, __seq_last__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_trygetlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_getlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_boundlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_dellast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_setlast_t)(DeeObject *self, DeeObject *value);

/* Sequence_any, seq_any, __seq_any__, explicit_seq_any */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_any_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_any_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_any_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeMH_seq_any_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Sequence_all, seq_all, __seq_all__, explicit_seq_all */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_all_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_all_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_all_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeMH_seq_all_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Sequence_parity, seq_parity, __seq_parity__, explicit_seq_parity */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_parity_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_parity_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_parity_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeMH_seq_parity_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Sequence_min, seq_min, __seq_min__, explicit_seq_min */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_min_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_min_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_min_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *DeeMH_seq_min_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Sequence_max, seq_max, __seq_max__, explicit_seq_max */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_max_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_max_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_max_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *DeeMH_seq_max_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Sequence_sum, seq_sum, __seq_sum__, explicit_seq_sum */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_sum_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_sum_with_range_t)(DeeObject *__restrict self, size_t start, size_t end);

/* Sequence_count, seq_count, __seq_count__, explicit_seq_count */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *DeeMH_seq_count_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) size_t (DCALL *DeeMH_seq_count_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *DeeMH_seq_count_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *DeeMH_seq_count_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* __seq_contains__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_contains_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_seq_contains_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_contains_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeMH_seq_contains_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_seq_operator_contains_t)(DeeObject *self, DeeObject *item);

/* Sequence_locate, seq_locate, __seq_locate__, explicit_seq_locate */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeMH_seq_locate_t)(DeeObject *self, DeeObject *match, DeeObject *def);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *DeeMH_seq_locate_with_range_t)(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* Sequence_rlocate, seq_rlocate, __seq_rlocate__, explicit_seq_rlocate */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeMH_seq_rlocate_t)(DeeObject *self, DeeObject *match, DeeObject *def);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *DeeMH_seq_rlocate_with_range_t)(DeeObject *self, DeeObject *match, size_t start, size_t end, DeeObject *def);

/* Sequence_startswith, seq_startswith, __seq_startswith__, explicit_seq_startswith */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_startswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_seq_startswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_startswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeMH_seq_startswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Sequence_endswith, seq_endswith, __seq_endswith__, explicit_seq_endswith */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_endswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_seq_endswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_endswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeMH_seq_endswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Sequence_find, seq_find, __seq_find__, explicit_seq_find */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *DeeMH_seq_find_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *DeeMH_seq_find_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Sequence_rfind, seq_rfind, __seq_rfind__, explicit_seq_rfind */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *DeeMH_seq_rfind_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *DeeMH_seq_rfind_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Sequence_erase, seq_erase, __seq_erase__, explicit_seq_erase */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_erase_t)(DeeObject *__restrict self, size_t index, size_t count);

/* Sequence_insert, seq_insert, __seq_insert__, explicit_seq_insert */
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeMH_seq_insert_t)(DeeObject *self, size_t index, DeeObject *item);

/* Sequence_insertall, seq_insertall, __seq_insertall__, explicit_seq_insertall */
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeMH_seq_insertall_t)(DeeObject *self, size_t index, DeeObject *items);

/* Sequence_pushfront, seq_pushfront, __seq_pushfront__, explicit_seq_pushfront */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_pushfront_t)(DeeObject *self, DeeObject *item);

/* Sequence_append, seq_append, Sequence_pushback, seq_pushback, __seq_append__, explicit_seq_append */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_append_t)(DeeObject *self, DeeObject *item);

/* Sequence_extend, seq_extend, __seq_extend__, explicit_seq_extend */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_seq_extend_t)(DeeObject *self, DeeObject *items);

/* Sequence_xchitem, seq_xchitem, __seq_xchitem__, explicit_seq_xchitem */
typedef WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *DeeMH_seq_xchitem_index_t)(DeeObject *self, size_t index, DeeObject *item);

/* Sequence_clear, seq_clear, Set_clear, Mapping_clear, __seq_clear__, explicit_seq_clear */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_seq_clear_t)(DeeObject *__restrict self);

/* Sequence_pop, seq_pop, __seq_pop__, explicit_seq_pop */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_seq_pop_t)(DeeObject *self, Dee_ssize_t index);

/* __set_iter__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_set_operator_iter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_set_operator_foreach_t)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_set_operator_foreach_pair_t)(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);

/* __set_size__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_set_operator_sizeob_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeMH_set_operator_size_t)(DeeObject *__restrict self);

/* __set_hash__ */
typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *DeeMH_set_operator_hash_t)(DeeObject *__restrict self);

/* __set_compare_eq__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_set_operator_compare_eq_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_set_operator_trycompare_eq_t)(DeeObject *lhs, DeeObject *rhs);

/* __set_eq__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_set_operator_eq_t)(DeeObject *lhs, DeeObject *rhs);

/* __set_ne__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_set_operator_ne_t)(DeeObject *lhs, DeeObject *rhs);

/* __map_getitem__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_map_operator_getitem_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_map_operator_trygetitem_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_map_operator_getitem_index_t)(DeeObject *self, size_t key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_map_operator_trygetitem_index_t)(DeeObject *self, size_t key);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_map_operator_getitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_map_operator_trygetitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_map_operator_getitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_map_operator_trygetitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_bounditem_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_bounditem_index_t)(DeeObject *self, size_t key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_bounditem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_bounditem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_hasitem_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_hasitem_index_t)(DeeObject *self, size_t key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_hasitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_hasitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* __map_delitem__ */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_delitem_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_delitem_index_t)(DeeObject *self, size_t key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeMH_map_operator_delitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeMH_map_operator_delitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);

/* __map_setitem__ */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeMH_map_operator_setitem_t)(DeeObject *self, DeeObject *key, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeMH_map_operator_setitem_index_t)(DeeObject *self, size_t key, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 4)) int (DCALL *DeeMH_map_operator_setitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 5)) int (DCALL *DeeMH_map_operator_setitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);

/* __map_contains__ */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeMH_map_operator_contains_t)(DeeObject *self, DeeObject *key);

/* __map_iterkeys__ */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeMH_map_iterkeys_t)(DeeObject *__restrict self);

/* __map_enumerate__ */
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_map_enumerate_t)(DeeObject *__restrict self, Dee_seq_enumerate_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2, 4, 5)) Dee_ssize_t (DCALL *DeeMH_map_enumerate_range_t)(DeeObject *self, Dee_seq_enumerate_t cb, void *arg, DeeObject *start, DeeObject *end);

/* Anonymous method hints */
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_foreach_reverse_t)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeMH_seq_enumerate_index_reverse_t)(DeeObject *__restrict self, Dee_seq_enumerate_index_t cb, void *arg, size_t start, size_t end);
/*[[[end]]]*/

/*[[[deemon (printMethodAttributeDecls from "...src.deemon.method-hints.method-hints")();]]]*/
#define DeeMA___seq_bool___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_bool___name[]; /* "__seq_bool__" */
DDATDEF char const DeeMA___seq_bool___doc[];  /* "->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_bool__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_size___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_size___name[]; /* "__seq_size__" */
DDATDEF char const DeeMA___seq_size___doc[];  /* "->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_iter___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_iter___name[]; /* "__seq_iter__" */
DDATDEF char const DeeMA___seq_iter___doc[];  /* "->?DIterator" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_getitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_getitem___name[]; /* "__seq_getitem__" */
DDATDEF char const DeeMA___seq_getitem___doc[];  /* "(index:?Dint)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_delitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_delitem___name[]; /* "__seq_delitem__" */
DDATDEF char const DeeMA___seq_delitem___doc[];  /* "(index:?Dint)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_setitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_setitem___name[]; /* "__seq_setitem__" */
DDATDEF char const DeeMA___seq_setitem___doc[];  /* "(index:?Dint,value)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_getrange___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_getrange___name[]; /* "__seq_getrange__" */
DDATDEF char const DeeMA___seq_getrange___doc[];  /* "(start?:?X2?Dint?N,end?:?X2?Dint?N)->?S" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_getrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_delrange___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_delrange___name[]; /* "__seq_delrange__" */
DDATDEF char const DeeMA___seq_delrange___doc[];  /* "(start?:?X2?Dint?N,end?:?X2?Dint?N)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_delrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_setrange___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_setrange___name[]; /* "__seq_setrange__" */
DDATDEF char const DeeMA___seq_setrange___doc[];  /* "(start:?X2?Dint?N,end:?X2?Dint?N,items:?X2?DSequence?S?O)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_setrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_assign___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_assign___name[]; /* "__seq_assign__" */
DDATDEF char const DeeMA___seq_assign___doc[];  /* "(items:?X2?DSequence?S?O)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_assign__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_hash___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_hash___name[]; /* "__seq_hash__" */
DDATDEF char const DeeMA___seq_hash___doc[];  /* "->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_compare___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_compare___name[]; /* "__seq_compare__" */
DDATDEF char const DeeMA___seq_compare___doc[];  /* "(rhs:?S?O)->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_compare__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_compare_eq___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_compare_eq___name[]; /* "__seq_compare_eq__" */
DDATDEF char const DeeMA___seq_compare_eq___doc[];  /* "(rhs:?S?O)->?X2?Dbool?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_trycompare_eq___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_trycompare_eq___name[]; /* "__seq_trycompare_eq__" */
DDATDEF char const DeeMA___seq_trycompare_eq___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_trycompare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_eq___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_eq___name[]; /* "__seq_eq__" */
DDATDEF char const DeeMA___seq_eq___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_ne___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_ne___name[]; /* "__seq_ne__" */
DDATDEF char const DeeMA___seq_ne___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_lo___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_lo___name[]; /* "__seq_lo__" */
DDATDEF char const DeeMA___seq_lo___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_le___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_le___name[]; /* "__seq_le__" */
DDATDEF char const DeeMA___seq_le___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_gr___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_gr___name[]; /* "__seq_gr__" */
DDATDEF char const DeeMA___seq_gr___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_ge___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_ge___name[]; /* "__seq_ge__" */
DDATDEF char const DeeMA___seq_ge___doc[];  /* "(rhs:?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_inplace_add___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_inplace_add___name[]; /* "__seq_inplace_add__" */
DDATDEF char const DeeMA___seq_inplace_add___doc[];  /* "(rhs:?S?O)->?." */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_inplace_mul___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_inplace_mul___name[]; /* "__seq_inplace_mul__" */
DDATDEF char const DeeMA___seq_inplace_mul___doc[];  /* "(repeat:?Dint)->?." */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_inplace_mul__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_enumerate___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_enumerate___name[]; /* "__seq_enumerate__" */
DDATDEF char const DeeMA___seq_enumerate___doc[];  /* "(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_enumerate_items___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_enumerate_items___name[]; /* "__seq_enumerate_items__" */
DDATDEF char const DeeMA___seq_enumerate_items___doc[];  /* "(start?:?X2?Dint?O,end?:?X2?Dint?O)->?S?T2?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___seq_any___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_any___name[]; /* "__seq_any__" */
DDATDEF char const DeeMA___seq_any___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_any__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_any_flags DeeMA___seq_any___flags
#define DeeMA_Sequence_any_doc   DeeMA___seq_any___doc
#define DeeMA_Sequence_any       DeeMA___seq_any__
DDATDEF char const DeeMA_Sequence_any_name[]; /* "any" */
#define DeeMA_seq_any_flags DeeMA_Sequence_any_flags
#define DeeMA_seq_any_name  DeeMA_Sequence_any_name
#define DeeMA_seq_any_doc   DeeMA_Sequence_any_doc
#define DeeMA_seq_any       DeeMA_Sequence_any
#define DeeMA_explicit_seq_any_flags DeeMA___seq_any___flags
#define DeeMA_explicit_seq_any_name  DeeMA___seq_any___name
#define DeeMA_explicit_seq_any_doc   DeeMA___seq_any___doc
#define DeeMA_explicit_seq_any       DeeMA___seq_any__

#define DeeMA___seq_all___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_all___name[]; /* "__seq_all__" */
DDATDEF char const DeeMA___seq_all___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_all__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_all_flags DeeMA___seq_all___flags
#define DeeMA_Sequence_all_doc   DeeMA___seq_all___doc
#define DeeMA_Sequence_all       DeeMA___seq_all__
DDATDEF char const DeeMA_Sequence_all_name[]; /* "all" */
#define DeeMA_seq_all_flags DeeMA_Sequence_all_flags
#define DeeMA_seq_all_name  DeeMA_Sequence_all_name
#define DeeMA_seq_all_doc   DeeMA_Sequence_all_doc
#define DeeMA_seq_all       DeeMA_Sequence_all
#define DeeMA_explicit_seq_all_flags DeeMA___seq_all___flags
#define DeeMA_explicit_seq_all_name  DeeMA___seq_all___name
#define DeeMA_explicit_seq_all_doc   DeeMA___seq_all___doc
#define DeeMA_explicit_seq_all       DeeMA___seq_all__

#define DeeMA___seq_parity___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_parity___name[]; /* "__seq_parity__" */
DDATDEF char const DeeMA___seq_parity___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_parity__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_parity_flags DeeMA___seq_parity___flags
#define DeeMA_Sequence_parity_doc   DeeMA___seq_parity___doc
#define DeeMA_Sequence_parity       DeeMA___seq_parity__
DDATDEF char const DeeMA_Sequence_parity_name[]; /* "parity" */
#define DeeMA_seq_parity_flags DeeMA_Sequence_parity_flags
#define DeeMA_seq_parity_name  DeeMA_Sequence_parity_name
#define DeeMA_seq_parity_doc   DeeMA_Sequence_parity_doc
#define DeeMA_seq_parity       DeeMA_Sequence_parity
#define DeeMA_explicit_seq_parity_flags DeeMA___seq_parity___flags
#define DeeMA_explicit_seq_parity_name  DeeMA___seq_parity___name
#define DeeMA_explicit_seq_parity_doc   DeeMA___seq_parity___doc
#define DeeMA_explicit_seq_parity       DeeMA___seq_parity__

#define DeeMA___seq_min___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_min___name[]; /* "__seq_min__" */
DDATDEF char const DeeMA___seq_min___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_min__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_min_flags DeeMA___seq_min___flags
#define DeeMA_Sequence_min_doc   DeeMA___seq_min___doc
#define DeeMA_Sequence_min       DeeMA___seq_min__
DDATDEF char const DeeMA_Sequence_min_name[]; /* "min" */
#define DeeMA_seq_min_flags DeeMA_Sequence_min_flags
#define DeeMA_seq_min_name  DeeMA_Sequence_min_name
#define DeeMA_seq_min_doc   DeeMA_Sequence_min_doc
#define DeeMA_seq_min       DeeMA_Sequence_min
#define DeeMA_explicit_seq_min_flags DeeMA___seq_min___flags
#define DeeMA_explicit_seq_min_name  DeeMA___seq_min___name
#define DeeMA_explicit_seq_min_doc   DeeMA___seq_min___doc
#define DeeMA_explicit_seq_min       DeeMA___seq_min__

#define DeeMA___seq_max___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_max___name[]; /* "__seq_max__" */
DDATDEF char const DeeMA___seq_max___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_max__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_max_flags DeeMA___seq_max___flags
#define DeeMA_Sequence_max_doc   DeeMA___seq_max___doc
#define DeeMA_Sequence_max       DeeMA___seq_max__
DDATDEF char const DeeMA_Sequence_max_name[]; /* "max" */
#define DeeMA_seq_max_flags DeeMA_Sequence_max_flags
#define DeeMA_seq_max_name  DeeMA_Sequence_max_name
#define DeeMA_seq_max_doc   DeeMA_Sequence_max_doc
#define DeeMA_seq_max       DeeMA_Sequence_max
#define DeeMA_explicit_seq_max_flags DeeMA___seq_max___flags
#define DeeMA_explicit_seq_max_name  DeeMA___seq_max___name
#define DeeMA_explicit_seq_max_doc   DeeMA___seq_max___doc
#define DeeMA_explicit_seq_max       DeeMA___seq_max__

#define DeeMA___seq_sum___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_sum___name[]; /* "__seq_sum__" */
DDATDEF char const DeeMA___seq_sum___doc[];  /* "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_sum__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_sum_flags DeeMA___seq_sum___flags
#define DeeMA_Sequence_sum_doc   DeeMA___seq_sum___doc
#define DeeMA_Sequence_sum       DeeMA___seq_sum__
DDATDEF char const DeeMA_Sequence_sum_name[]; /* "sum" */
#define DeeMA_seq_sum_flags DeeMA_Sequence_sum_flags
#define DeeMA_seq_sum_name  DeeMA_Sequence_sum_name
#define DeeMA_seq_sum_doc   DeeMA_Sequence_sum_doc
#define DeeMA_seq_sum       DeeMA_Sequence_sum
#define DeeMA_explicit_seq_sum_flags DeeMA___seq_sum___flags
#define DeeMA_explicit_seq_sum_name  DeeMA___seq_sum___name
#define DeeMA_explicit_seq_sum_doc   DeeMA___seq_sum___doc
#define DeeMA_explicit_seq_sum       DeeMA___seq_sum__

#define DeeMA___seq_count___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_count___name[]; /* "__seq_count__" */
DDATDEF char const DeeMA___seq_count___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_count__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_count_flags DeeMA___seq_count___flags
#define DeeMA_Sequence_count_doc   DeeMA___seq_count___doc
#define DeeMA_Sequence_count       DeeMA___seq_count__
DDATDEF char const DeeMA_Sequence_count_name[]; /* "count" */
#define DeeMA_seq_count_flags DeeMA_Sequence_count_flags
#define DeeMA_seq_count_name  DeeMA_Sequence_count_name
#define DeeMA_seq_count_doc   DeeMA_Sequence_count_doc
#define DeeMA_seq_count       DeeMA_Sequence_count
#define DeeMA_explicit_seq_count_flags DeeMA___seq_count___flags
#define DeeMA_explicit_seq_count_name  DeeMA___seq_count___name
#define DeeMA_explicit_seq_count_doc   DeeMA___seq_count___doc
#define DeeMA_explicit_seq_count       DeeMA___seq_count__

#define DeeMA___seq_contains___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_contains___name[]; /* "__seq_contains__" */
DDATDEF char const DeeMA___seq_contains___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);

#define DeeMA___seq_locate___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_locate___name[]; /* "__seq_locate__" */
DDATDEF char const DeeMA___seq_locate___doc[];  /* "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_locate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_locate_flags DeeMA___seq_locate___flags
#define DeeMA_Sequence_locate_doc   DeeMA___seq_locate___doc
#define DeeMA_Sequence_locate       DeeMA___seq_locate__
DDATDEF char const DeeMA_Sequence_locate_name[]; /* "locate" */
#define DeeMA_seq_locate_flags DeeMA_Sequence_locate_flags
#define DeeMA_seq_locate_name  DeeMA_Sequence_locate_name
#define DeeMA_seq_locate_doc   DeeMA_Sequence_locate_doc
#define DeeMA_seq_locate       DeeMA_Sequence_locate
#define DeeMA_explicit_seq_locate_flags DeeMA___seq_locate___flags
#define DeeMA_explicit_seq_locate_name  DeeMA___seq_locate___name
#define DeeMA_explicit_seq_locate_doc   DeeMA___seq_locate___doc
#define DeeMA_explicit_seq_locate       DeeMA___seq_locate__

#define DeeMA___seq_rlocate___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_rlocate___name[]; /* "__seq_rlocate__" */
DDATDEF char const DeeMA___seq_rlocate___doc[];  /* "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_rlocate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_rlocate_flags DeeMA___seq_rlocate___flags
#define DeeMA_Sequence_rlocate_doc   DeeMA___seq_rlocate___doc
#define DeeMA_Sequence_rlocate       DeeMA___seq_rlocate__
DDATDEF char const DeeMA_Sequence_rlocate_name[]; /* "rlocate" */
#define DeeMA_seq_rlocate_flags DeeMA_Sequence_rlocate_flags
#define DeeMA_seq_rlocate_name  DeeMA_Sequence_rlocate_name
#define DeeMA_seq_rlocate_doc   DeeMA_Sequence_rlocate_doc
#define DeeMA_seq_rlocate       DeeMA_Sequence_rlocate
#define DeeMA_explicit_seq_rlocate_flags DeeMA___seq_rlocate___flags
#define DeeMA_explicit_seq_rlocate_name  DeeMA___seq_rlocate___name
#define DeeMA_explicit_seq_rlocate_doc   DeeMA___seq_rlocate___doc
#define DeeMA_explicit_seq_rlocate       DeeMA___seq_rlocate__

#define DeeMA___seq_startswith___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_startswith___name[]; /* "__seq_startswith__" */
DDATDEF char const DeeMA___seq_startswith___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_startswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_startswith_flags DeeMA___seq_startswith___flags
#define DeeMA_Sequence_startswith_doc   DeeMA___seq_startswith___doc
#define DeeMA_Sequence_startswith       DeeMA___seq_startswith__
DDATDEF char const DeeMA_Sequence_startswith_name[]; /* "startswith" */
#define DeeMA_seq_startswith_flags DeeMA_Sequence_startswith_flags
#define DeeMA_seq_startswith_name  DeeMA_Sequence_startswith_name
#define DeeMA_seq_startswith_doc   DeeMA_Sequence_startswith_doc
#define DeeMA_seq_startswith       DeeMA_Sequence_startswith
#define DeeMA_explicit_seq_startswith_flags DeeMA___seq_startswith___flags
#define DeeMA_explicit_seq_startswith_name  DeeMA___seq_startswith___name
#define DeeMA_explicit_seq_startswith_doc   DeeMA___seq_startswith___doc
#define DeeMA_explicit_seq_startswith       DeeMA___seq_startswith__

#define DeeMA___seq_endswith___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_endswith___name[]; /* "__seq_endswith__" */
DDATDEF char const DeeMA___seq_endswith___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_endswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_endswith_flags DeeMA___seq_endswith___flags
#define DeeMA_Sequence_endswith_doc   DeeMA___seq_endswith___doc
#define DeeMA_Sequence_endswith       DeeMA___seq_endswith__
DDATDEF char const DeeMA_Sequence_endswith_name[]; /* "endswith" */
#define DeeMA_seq_endswith_flags DeeMA_Sequence_endswith_flags
#define DeeMA_seq_endswith_name  DeeMA_Sequence_endswith_name
#define DeeMA_seq_endswith_doc   DeeMA_Sequence_endswith_doc
#define DeeMA_seq_endswith       DeeMA_Sequence_endswith
#define DeeMA_explicit_seq_endswith_flags DeeMA___seq_endswith___flags
#define DeeMA_explicit_seq_endswith_name  DeeMA___seq_endswith___name
#define DeeMA_explicit_seq_endswith_doc   DeeMA___seq_endswith___doc
#define DeeMA_explicit_seq_endswith       DeeMA___seq_endswith__

#define DeeMA___seq_find___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_find___name[]; /* "__seq_find__" */
DDATDEF char const DeeMA___seq_find___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_find__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_find_flags DeeMA___seq_find___flags
#define DeeMA_Sequence_find_doc   DeeMA___seq_find___doc
#define DeeMA_Sequence_find       DeeMA___seq_find__
DDATDEF char const DeeMA_Sequence_find_name[]; /* "find" */
#define DeeMA_seq_find_flags DeeMA_Sequence_find_flags
#define DeeMA_seq_find_name  DeeMA_Sequence_find_name
#define DeeMA_seq_find_doc   DeeMA_Sequence_find_doc
#define DeeMA_seq_find       DeeMA_Sequence_find
#define DeeMA_explicit_seq_find_flags DeeMA___seq_find___flags
#define DeeMA_explicit_seq_find_name  DeeMA___seq_find___name
#define DeeMA_explicit_seq_find_doc   DeeMA___seq_find___doc
#define DeeMA_explicit_seq_find       DeeMA___seq_find__

#define DeeMA___seq_rfind___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_rfind___name[]; /* "__seq_rfind__" */
DDATDEF char const DeeMA___seq_rfind___doc[];  /* "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_rfind__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_rfind_flags DeeMA___seq_rfind___flags
#define DeeMA_Sequence_rfind_doc   DeeMA___seq_rfind___doc
#define DeeMA_Sequence_rfind       DeeMA___seq_rfind__
DDATDEF char const DeeMA_Sequence_rfind_name[]; /* "rfind" */
#define DeeMA_seq_rfind_flags DeeMA_Sequence_rfind_flags
#define DeeMA_seq_rfind_name  DeeMA_Sequence_rfind_name
#define DeeMA_seq_rfind_doc   DeeMA_Sequence_rfind_doc
#define DeeMA_seq_rfind       DeeMA_Sequence_rfind
#define DeeMA_explicit_seq_rfind_flags DeeMA___seq_rfind___flags
#define DeeMA_explicit_seq_rfind_name  DeeMA___seq_rfind___name
#define DeeMA_explicit_seq_rfind_doc   DeeMA___seq_rfind___doc
#define DeeMA_explicit_seq_rfind       DeeMA___seq_rfind__

#define DeeMA___seq_erase___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_erase___name[]; /* "__seq_erase__" */
DDATDEF char const DeeMA___seq_erase___doc[];  /* "(index:?Dint,count=!1)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_erase__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_erase_flags DeeMA___seq_erase___flags
#define DeeMA_Sequence_erase_doc   DeeMA___seq_erase___doc
#define DeeMA_Sequence_erase       DeeMA___seq_erase__
DDATDEF char const DeeMA_Sequence_erase_name[]; /* "erase" */
#define DeeMA_seq_erase_flags DeeMA_Sequence_erase_flags
#define DeeMA_seq_erase_name  DeeMA_Sequence_erase_name
#define DeeMA_seq_erase_doc   DeeMA_Sequence_erase_doc
#define DeeMA_seq_erase       DeeMA_Sequence_erase
#define DeeMA_explicit_seq_erase_flags DeeMA___seq_erase___flags
#define DeeMA_explicit_seq_erase_name  DeeMA___seq_erase___name
#define DeeMA_explicit_seq_erase_doc   DeeMA___seq_erase___doc
#define DeeMA_explicit_seq_erase       DeeMA___seq_erase__

#define DeeMA___seq_insert___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_insert___name[]; /* "__seq_insert__" */
DDATDEF char const DeeMA___seq_insert___doc[];  /* "(index:?Dint,item)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_insert__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_insert_flags DeeMA___seq_insert___flags
#define DeeMA_Sequence_insert_doc   DeeMA___seq_insert___doc
#define DeeMA_Sequence_insert       DeeMA___seq_insert__
DDATDEF char const DeeMA_Sequence_insert_name[]; /* "insert" */
#define DeeMA_seq_insert_flags DeeMA_Sequence_insert_flags
#define DeeMA_seq_insert_name  DeeMA_Sequence_insert_name
#define DeeMA_seq_insert_doc   DeeMA_Sequence_insert_doc
#define DeeMA_seq_insert       DeeMA_Sequence_insert
#define DeeMA_explicit_seq_insert_flags DeeMA___seq_insert___flags
#define DeeMA_explicit_seq_insert_name  DeeMA___seq_insert___name
#define DeeMA_explicit_seq_insert_doc   DeeMA___seq_insert___doc
#define DeeMA_explicit_seq_insert       DeeMA___seq_insert__

#define DeeMA___seq_insertall___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_insertall___name[]; /* "__seq_insertall__" */
DDATDEF char const DeeMA___seq_insertall___doc[];  /* "(index:?Dint,items:?S?O)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_insertall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_insertall_flags DeeMA___seq_insertall___flags
#define DeeMA_Sequence_insertall_doc   DeeMA___seq_insertall___doc
#define DeeMA_Sequence_insertall       DeeMA___seq_insertall__
DDATDEF char const DeeMA_Sequence_insertall_name[]; /* "insertall" */
#define DeeMA_seq_insertall_flags DeeMA_Sequence_insertall_flags
#define DeeMA_seq_insertall_name  DeeMA_Sequence_insertall_name
#define DeeMA_seq_insertall_doc   DeeMA_Sequence_insertall_doc
#define DeeMA_seq_insertall       DeeMA_Sequence_insertall
#define DeeMA_explicit_seq_insertall_flags DeeMA___seq_insertall___flags
#define DeeMA_explicit_seq_insertall_name  DeeMA___seq_insertall___name
#define DeeMA_explicit_seq_insertall_doc   DeeMA___seq_insertall___doc
#define DeeMA_explicit_seq_insertall       DeeMA___seq_insertall__

#define DeeMA___seq_pushfront___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_pushfront___name[]; /* "__seq_pushfront__" */
DDATDEF char const DeeMA___seq_pushfront___doc[];  /* "(item)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_pushfront__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
#define DeeMA_Sequence_pushfront_flags DeeMA___seq_pushfront___flags
#define DeeMA_Sequence_pushfront_doc   DeeMA___seq_pushfront___doc
#define DeeMA_Sequence_pushfront       DeeMA___seq_pushfront__
DDATDEF char const DeeMA_Sequence_pushfront_name[]; /* "pushfront" */
#define DeeMA_seq_pushfront_flags DeeMA_Sequence_pushfront_flags
#define DeeMA_seq_pushfront_name  DeeMA_Sequence_pushfront_name
#define DeeMA_seq_pushfront_doc   DeeMA_Sequence_pushfront_doc
#define DeeMA_seq_pushfront       DeeMA_Sequence_pushfront
#define DeeMA_explicit_seq_pushfront_flags DeeMA___seq_pushfront___flags
#define DeeMA_explicit_seq_pushfront_name  DeeMA___seq_pushfront___name
#define DeeMA_explicit_seq_pushfront_doc   DeeMA___seq_pushfront___doc
#define DeeMA_explicit_seq_pushfront       DeeMA___seq_pushfront__

#define DeeMA___seq_append___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_append___name[]; /* "__seq_append__" */
DDATDEF char const DeeMA___seq_append___doc[];  /* "(item)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_append__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
#define DeeMA_Sequence_append_flags DeeMA___seq_append___flags
#define DeeMA_Sequence_append_doc   DeeMA___seq_append___doc
#define DeeMA_Sequence_append       DeeMA___seq_append__
DDATDEF char const DeeMA_Sequence_append_name[]; /* "append" */
#define DeeMA_seq_append_flags DeeMA_Sequence_append_flags
#define DeeMA_seq_append_name  DeeMA_Sequence_append_name
#define DeeMA_seq_append_doc   DeeMA_Sequence_append_doc
#define DeeMA_seq_append       DeeMA_Sequence_append
#define DeeMA_Sequence_pushback_flags DeeMA___seq_append___flags
#define DeeMA_Sequence_pushback_doc   DeeMA___seq_append___doc
#define DeeMA_Sequence_pushback       DeeMA___seq_append__
DDATDEF char const DeeMA_Sequence_pushback_name[]; /* "pushback" */
#define DeeMA_seq_pushback_flags DeeMA_Sequence_pushback_flags
#define DeeMA_seq_pushback_name  DeeMA_Sequence_pushback_name
#define DeeMA_seq_pushback_doc   DeeMA_Sequence_pushback_doc
#define DeeMA_seq_pushback       DeeMA_Sequence_pushback
#define DeeMA_explicit_seq_append_flags DeeMA___seq_append___flags
#define DeeMA_explicit_seq_append_name  DeeMA___seq_append___name
#define DeeMA_explicit_seq_append_doc   DeeMA___seq_append___doc
#define DeeMA_explicit_seq_append       DeeMA___seq_append__

#define DeeMA___seq_extend___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_extend___name[]; /* "__seq_extend__" */
DDATDEF char const DeeMA___seq_extend___doc[];  /* "(items:?S?O)" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_extend__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
#define DeeMA_Sequence_extend_flags DeeMA___seq_extend___flags
#define DeeMA_Sequence_extend_doc   DeeMA___seq_extend___doc
#define DeeMA_Sequence_extend       DeeMA___seq_extend__
DDATDEF char const DeeMA_Sequence_extend_name[]; /* "extend" */
#define DeeMA_seq_extend_flags DeeMA_Sequence_extend_flags
#define DeeMA_seq_extend_name  DeeMA_Sequence_extend_name
#define DeeMA_seq_extend_doc   DeeMA_Sequence_extend_doc
#define DeeMA_seq_extend       DeeMA_Sequence_extend
#define DeeMA_explicit_seq_extend_flags DeeMA___seq_extend___flags
#define DeeMA_explicit_seq_extend_name  DeeMA___seq_extend___name
#define DeeMA_explicit_seq_extend_doc   DeeMA___seq_extend___doc
#define DeeMA_explicit_seq_extend       DeeMA___seq_extend__

#define DeeMA___seq_xchitem___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_xchitem___name[]; /* "__seq_xchitem__" */
DDATDEF char const DeeMA___seq_xchitem___doc[];  /* "(index:?Dint,item)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_xchitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_xchitem_flags DeeMA___seq_xchitem___flags
#define DeeMA_Sequence_xchitem_doc   DeeMA___seq_xchitem___doc
#define DeeMA_Sequence_xchitem       DeeMA___seq_xchitem__
DDATDEF char const DeeMA_Sequence_xchitem_name[]; /* "xchitem" */
#define DeeMA_seq_xchitem_flags DeeMA_Sequence_xchitem_flags
#define DeeMA_seq_xchitem_name  DeeMA_Sequence_xchitem_name
#define DeeMA_seq_xchitem_doc   DeeMA_Sequence_xchitem_doc
#define DeeMA_seq_xchitem       DeeMA_Sequence_xchitem
#define DeeMA_explicit_seq_xchitem_flags DeeMA___seq_xchitem___flags
#define DeeMA_explicit_seq_xchitem_name  DeeMA___seq_xchitem___name
#define DeeMA_explicit_seq_xchitem_doc   DeeMA___seq_xchitem___doc
#define DeeMA_explicit_seq_xchitem       DeeMA___seq_xchitem__

#define DeeMA___seq_clear___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___seq_clear___name[]; /* "__seq_clear__" */
DDATDEF char const DeeMA___seq_clear___doc[];  /* "" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_clear__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
#define DeeMA_Sequence_clear_flags DeeMA___seq_clear___flags
#define DeeMA_Sequence_clear_doc   DeeMA___seq_clear___doc
#define DeeMA_Sequence_clear       DeeMA___seq_clear__
DDATDEF char const DeeMA_Sequence_clear_name[]; /* "clear" */
#define DeeMA_seq_clear_flags DeeMA_Sequence_clear_flags
#define DeeMA_seq_clear_name  DeeMA_Sequence_clear_name
#define DeeMA_seq_clear_doc   DeeMA_Sequence_clear_doc
#define DeeMA_seq_clear       DeeMA_Sequence_clear
#define DeeMA_Set_clear_flags DeeMA___seq_clear___flags
#define DeeMA_Set_clear_doc   DeeMA___seq_clear___doc
#define DeeMA_Set_clear       DeeMA___seq_clear__
DDATDEF char const DeeMA_Set_clear_name[]; /* "clear" */
#define DeeMA_Mapping_clear_flags DeeMA___seq_clear___flags
#define DeeMA_Mapping_clear_doc   DeeMA___seq_clear___doc
#define DeeMA_Mapping_clear       DeeMA___seq_clear__
DDATDEF char const DeeMA_Mapping_clear_name[]; /* "clear" */
#define DeeMA_explicit_seq_clear_flags DeeMA___seq_clear___flags
#define DeeMA_explicit_seq_clear_name  DeeMA___seq_clear___name
#define DeeMA_explicit_seq_clear_doc   DeeMA___seq_clear___doc
#define DeeMA_explicit_seq_clear       DeeMA___seq_clear__

#define DeeMA___seq_pop___flags Dee_TYPE_METHOD_FKWDS
DDATDEF char const DeeMA___seq_pop___name[]; /* "__seq_pop__" */
DDATDEF char const DeeMA___seq_pop___doc[];  /* "(index=!-1)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___seq_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeMA_Sequence_pop_flags DeeMA___seq_pop___flags
#define DeeMA_Sequence_pop_doc   DeeMA___seq_pop___doc
#define DeeMA_Sequence_pop       DeeMA___seq_pop__
DDATDEF char const DeeMA_Sequence_pop_name[]; /* "pop" */
#define DeeMA_seq_pop_flags DeeMA_Sequence_pop_flags
#define DeeMA_seq_pop_name  DeeMA_Sequence_pop_name
#define DeeMA_seq_pop_doc   DeeMA_Sequence_pop_doc
#define DeeMA_seq_pop       DeeMA_Sequence_pop
#define DeeMA_explicit_seq_pop_flags DeeMA___seq_pop___flags
#define DeeMA_explicit_seq_pop_name  DeeMA___seq_pop___name
#define DeeMA_explicit_seq_pop_doc   DeeMA___seq_pop___doc
#define DeeMA_explicit_seq_pop       DeeMA___seq_pop__

#define DeeMA___set_iter___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_iter___name[]; /* "__set_iter__" */
DDATDEF char const DeeMA___set_iter___doc[];  /* "->?DIterator" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___set_size___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_size___name[]; /* "__set_size__" */
DDATDEF char const DeeMA___set_size___doc[];  /* "->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___set_hash___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_hash___name[]; /* "__set_hash__" */
DDATDEF char const DeeMA___set_hash___doc[];  /* "->?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___set_compare_eq___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_compare_eq___name[]; /* "__set_compare_eq__" */
DDATDEF char const DeeMA___set_compare_eq___doc[];  /* "(rhs:?X2?DSet?S?O)->?X2?Dbool?Dint" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___set_eq___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_eq___name[]; /* "__set_eq__" */
DDATDEF char const DeeMA___set_eq___doc[];  /* "(rhs:?X2?DSet?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___set_ne___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___set_ne___name[]; /* "__set_ne__" */
DDATDEF char const DeeMA___set_ne___doc[];  /* "(rhs:?X2?DSet?S?O)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___set_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_getitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_getitem___name[]; /* "__map_getitem__" */
DDATDEF char const DeeMA___map_getitem___doc[];  /* "(key)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_delitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_delitem___name[]; /* "__map_delitem__" */
DDATDEF char const DeeMA___map_delitem___doc[];  /* "(key)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_setitem___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_setitem___name[]; /* "__map_setitem__" */
DDATDEF char const DeeMA___map_setitem___doc[];  /* "(key,value)->" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_contains___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_contains___name[]; /* "__map_contains__" */
DDATDEF char const DeeMA___map_contains___doc[];  /* "(key)->?Dbool" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_iterkeys___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_iterkeys___name[]; /* "__map_iterkeys__" */
DDATDEF char const DeeMA___map_iterkeys___doc[];  /* "->?DIterator" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_iterkeys__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);

#define DeeMA___map_enumerate___flags Dee_TYPE_METHOD_FNORMAL
DDATDEF char const DeeMA___map_enumerate___name[]; /* "__map_enumerate__" */
DDATDEF char const DeeMA___map_enumerate___doc[];  /* "(cb:?DCallable,startkey?,endkey?)->?X2?O?N" */
DFUNDEF NONNULL((1)) DREF DeeObject *DCALL DeeMA___map_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv);
/*[[[end]]]*/
/* clang-format on */

/* Same as `DeeType_GetExplicitMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions.
 *
 * Never returns NULL when `id' has an "%{unsupported}" implementation. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id);

/* Same as `DeeType_GetMethodHint', but don't make use of the method-hint cache.
 *
 * Never returns NULL when `id' has an "%{unsupported}" implementation. */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetUncachedMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id);

#ifdef CONFIG_BUILDING_DEEMON
/* Check if `self' specifically is able to supply the method hint `id'
 * in some form. If not, return `NULL' to indicate this lack of support.
 *
 * Note that this function doesn't return "%{unsupported}" implementations.
 *
 * WARNING: Only call this function for some given "self, orig_type" if
 *          you've already called it with all preceding types "self" that
 *          appear in "orig_type.__mro__". */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_funptr_t
(DCALL DeeType_GetPrivateMethodHint)(DeeTypeObject *self, DeeTypeObject *orig_type, enum Dee_tmh_id id);

/* Same as `DeeType_GetPrivateMethodHint', but only check for attributes
 * without doing any default substitutions.
 *
 * WARNING: Only call this function for some given "self, orig_type" if
 *          you've already called it with all preceding types "self" that
 *          appear in "orig_type.__mro__". */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_funptr_t
(DCALL DeeType_GetPrivateMethodHintNoDefault)(DeeTypeObject *self, DeeTypeObject *orig_type, enum Dee_tmh_id id);
#endif /* CONFIG_BUILDING_DEEMON */

/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetExplicitMethodHint)(DeeTypeObject *__restrict self, enum Dee_tmh_id id);

/* Returns the "%{unsupported}" implementation of `id'
 * (if it has one). If not, return `NULL' instead. */
DFUNDEF ATTR_CONST WUNUSED Dee_funptr_t
(DCALL DeeType_GetUnsupportedMethodHint)(enum Dee_tmh_id id);

struct Dee_type_mh_cache_array {
	Dee_funptr_t mh_funcs[Dee_TMH_COUNT];
};

#ifndef __OPTIMIZE_SIZE__
#undef DeeType_GetMethodHint
#define DeeType_GetMethodHint(self, id) DeeType_GetMethodHint_inline(self, id)
LOCAL ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetMethodHint_inline)(DeeTypeObject *__restrict self, enum Dee_tmh_id id) {
	/* NOTE: By inlining the likely case where the method "id" has already been
	 *       cached, the compiler can optimize calls to method hints to be pretty
	 *       must just-as-fast as regular operator invocations. */
	struct Dee_type_mh_cache_array *mhcache;
	mhcache = (struct Dee_type_mh_cache_array *)self->tp_mhcache;
	if likely(mhcache) {
		Dee_funptr_t result = mhcache->mh_funcs[id];
		if likely(result)
			return result;
	}
	return (DeeType_GetMethodHint)(self, id);
}
#endif /* !__OPTIMIZE_SIZE__ */

#define DeeType_RequireMethodHint(self, name) \
	((DeeMH_##name##_t)DeeType_GetMethodHint(self, Dee_TMH_##name))
#define DeeType_InvokeMethodHint0(self, name)     (*DeeType_RequireMethodHint(Dee_TYPE(self), name))(self)
#define DeeType_InvokeMethodHint(self, name, ...) (*DeeType_RequireMethodHint(Dee_TYPE(self), name))(self, __VA_ARGS__)


/* Everything below is deprecated! */
#define DeeType_TryRequireSeqForeachReverse(self)               ((DeeMH_seq_foreach_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_foreach_reverse))
#define DeeType_TryRequireSeqEnumerateIndexReverse(self)        ((DeeMH_seq_enumerate_index_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_enumerate_index_reverse))
#define DeeType_RequireSeqMakeEnumeration(self)                 ((DeeMH_seq_makeenumeration_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration))
#define DeeType_RequireSeqMakeEnumerationWithIntRange(self)     ((DeeMH_seq_makeenumeration_with_int_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_int_range))
#define DeeType_RequireSeqMakeEnumerationWithRange(self)        ((DeeMH_seq_makeenumeration_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_range))
#define DeeType_RequireSeqOperatorBool(self)                    ((DeeMH_seq_operator_bool_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bool))
#define DeeType_RequireSeqOperatorIter(self)                    ((DeeMH_seq_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_iter))
#define DeeType_RequireSeqOperatorSizeOb(self)                  ((DeeMH_seq_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_sizeob))
#define DeeType_RequireSeqOperatorContains(self)                ((DeeMH_seq_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_contains))
#define DeeType_RequireSeqOperatorGetItem(self)                 ((DeeMH_seq_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem))
#define DeeType_RequireSeqOperatorDelItem(self)                 ((DeeMH_seq_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem))
#define DeeType_RequireSeqOperatorSetItem(self)                 ((DeeMH_seq_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem))
#define DeeType_RequireSeqOperatorGetRange(self)                ((DeeMH_seq_operator_getrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange))
#define DeeType_RequireSeqOperatorDelRange(self)                ((DeeMH_seq_operator_delrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange))
#define DeeType_RequireSeqOperatorSetRange(self)                ((DeeMH_seq_operator_setrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange))
#define DeeType_RequireSeqOperatorForeach(self)                 ((DeeMH_seq_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach))
#define DeeType_RequireSeqOperatorForeachPair(self)             ((DeeMH_seq_operator_foreach_pair_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach_pair))
#define DeeType_RequireSeqEnumerate(self)                       ((DeeMH_seq_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_enumerate))
#define DeeType_RequireSeqEnumerateIndex(self)                  ((DeeMH_seq_enumerate_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_enumerate_index))
#define DeeType_RequireSeqOperatorIterKeys(self)                ((DeeMH_seq_iterkeys_t)DeeType_GetMethodHint(self, Dee_TMH_seq_iterkeys))
#define DeeType_RequireSeqOperatorBoundItem(self)               ((DeeMH_seq_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem))
#define DeeType_RequireSeqOperatorHasItem(self)                 ((DeeMH_seq_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem))
#define DeeType_RequireSeqOperatorSize(self)                    ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_size))
#define DeeType_RequireSeqOperatorGetItemIndex(self)            ((DeeMH_seq_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem_index))
#define DeeType_RequireSeqOperatorDelItemIndex(self)            ((DeeMH_seq_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem_index))
#define DeeType_RequireSeqOperatorSetItemIndex(self)            ((DeeMH_seq_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem_index))
#define DeeType_RequireSeqOperatorBoundItemIndex(self)          ((DeeMH_seq_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem_index))
#define DeeType_RequireSeqOperatorHasItemIndex(self)            ((DeeMH_seq_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem_index))
#define DeeType_RequireSeqOperatorGetRangeIndex(self)           ((DeeMH_seq_operator_getrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index))
#define DeeType_RequireSeqOperatorDelRangeIndex(self)           ((DeeMH_seq_operator_delrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index))
#define DeeType_RequireSeqOperatorSetRangeIndex(self)           ((DeeMH_seq_operator_setrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index))
#define DeeType_RequireSeqOperatorGetRangeIndexN(self)          ((DeeMH_seq_operator_getrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index_n))
#define DeeType_RequireSeqOperatorDelRangeIndexN(self)          ((DeeMH_seq_operator_delrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index_n))
#define DeeType_RequireSeqOperatorSetRangeIndexN(self)          ((DeeMH_seq_operator_setrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index_n))
#define DeeType_RequireSeqOperatorTryGetItem(self)              ((DeeMH_seq_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem))
#define DeeType_RequireSeqOperatorTryGetItemIndex(self)         ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem_index))
#define DeeType_RequireSeqOperatorHash(self)                    ((DeeMH_seq_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hash))
#define DeeType_RequireSeqOperatorCompareEq(self)               ((DeeMH_seq_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare_eq))
#define DeeType_RequireSeqOperatorCompare(self)                 ((DeeMH_seq_operator_compare_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare))
#define DeeType_RequireSeqOperatorTryCompareEq(self)            ((DeeMH_seq_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trycompare_eq))
#define DeeType_RequireSeqOperatorEq(self)                      ((DeeMH_seq_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_eq))
#define DeeType_RequireSeqOperatorNe(self)                      ((DeeMH_seq_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ne))
#define DeeType_RequireSeqOperatorLo(self)                      ((DeeMH_seq_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_lo))
#define DeeType_RequireSeqOperatorLe(self)                      ((DeeMH_seq_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_le))
#define DeeType_RequireSeqOperatorGr(self)                      ((DeeMH_seq_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_gr))
#define DeeType_RequireSeqOperatorGe(self)                      ((DeeMH_seq_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ge))
#define DeeType_RequireSeqOperatorInplaceAdd(self)              ((DeeMH_seq_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_add))
#define DeeType_RequireSeqOperatorInplaceMul(self)              ((DeeMH_seq_operator_inplace_mul_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_mul))
#define DeeType_RequireSetOperatorIter(self)                    ((DeeMH_set_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_iter))
#define DeeType_RequireSetOperatorForeach(self)                 ((DeeMH_set_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_foreach))
#define DeeType_RequireSetOperatorForeachPair(self)             ((DeeMH_set_operator_foreach_pair_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_foreach_pair))
#define DeeType_RequireSetOperatorSize(self)                    ((DeeMH_set_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_size))
#define DeeType_RequireSetOperatorSizeOb(self)                  ((DeeMH_set_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sizeob))
#define DeeType_RequireSetOperatorHash(self)                    ((DeeMH_set_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_hash))
#define DeeType_RequireSetOperatorCompareEq(self)               ((DeeMH_set_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_compare_eq))
#define DeeType_RequireSetOperatorTryCompareEq(self)            ((DeeMH_set_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_trycompare_eq))
#define DeeType_RequireSetOperatorEq(self)                      ((DeeMH_set_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_eq))
#define DeeType_RequireSetOperatorNe(self)                      ((DeeMH_set_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ne))
#define DeeType_RequireSetOperatorLo(self)                      ((DeeMH_set_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_lo))
#define DeeType_RequireSetOperatorLe(self)                      ((DeeMH_set_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_le))
#define DeeType_RequireSetOperatorGr(self)                      ((DeeMH_set_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_gr))
#define DeeType_RequireSetOperatorGe(self)                      ((DeeMH_set_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ge))
#define DeeType_RequireSetOperatorInv(self)                     ((DeeMH_set_operator_inv_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inv))
#define DeeType_RequireSetOperatorAdd(self)                     ((DeeMH_set_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_add))
#define DeeType_RequireSetOperatorSub(self)                     ((DeeMH_set_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sub))
#define DeeType_RequireSetOperatorAnd(self)                     ((DeeMH_set_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_and))
#define DeeType_RequireSetOperatorXor(self)                     ((DeeMH_set_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_xor))
#define DeeType_RequireSetOperatorInplaceAdd(self)              ((DeeMH_set_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_add))
#define DeeType_RequireSetOperatorInplaceSub(self)              ((DeeMH_set_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_sub))
#define DeeType_RequireSetOperatorInplaceAnd(self)              ((DeeMH_set_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_and))
#define DeeType_RequireSetOperatorInplaceXor(self)              ((DeeMH_set_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_xor))
#define DeeType_RequireMapOperatorContains(self)                ((DeeMH_map_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_contains))
#define DeeType_RequireMapOperatorGetItem(self)                 ((DeeMH_map_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem))
#define DeeType_RequireMapOperatorDelItem(self)                 ((DeeMH_map_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem))
#define DeeType_RequireMapOperatorSetItem(self)                 ((DeeMH_map_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem))
#define DeeType_RequireMapOperatorBoundItem(self)               ((DeeMH_map_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem))
#define DeeType_RequireMapOperatorHasItem(self)                 ((DeeMH_map_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem))
#define DeeType_RequireMapOperatorGetItemIndex(self)            ((DeeMH_map_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_index))
#define DeeType_RequireMapOperatorDelItemIndex(self)            ((DeeMH_map_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_index))
#define DeeType_RequireMapOperatorSetItemIndex(self)            ((DeeMH_map_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_index))
#define DeeType_RequireMapOperatorBoundItemIndex(self)          ((DeeMH_map_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_index))
#define DeeType_RequireMapOperatorHasItemIndex(self)            ((DeeMH_map_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_index))
#define DeeType_RequireMapOperatorTryGetItem(self)              ((DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem))
#define DeeType_RequireMapOperatorTryGetItemIndex(self)         ((DeeMH_map_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_index))
#define DeeType_RequireMapOperatorTryGetItemStringHash(self)    ((DeeMH_map_operator_trygetitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_hash))
#define DeeType_RequireMapOperatorGetItemStringHash(self)       ((DeeMH_map_operator_getitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_hash))
#define DeeType_RequireMapOperatorDelItemStringHash(self)       ((DeeMH_map_operator_delitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_hash))
#define DeeType_RequireMapOperatorSetItemStringHash(self)       ((DeeMH_map_operator_setitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_hash))
#define DeeType_RequireMapOperatorBoundItemStringHash(self)     ((DeeMH_map_operator_bounditem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_hash))
#define DeeType_RequireMapOperatorHasItemStringHash(self)       ((DeeMH_map_operator_hasitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_hash))
#define DeeType_RequireMapOperatorTryGetItemStringLenHash(self) ((DeeMH_map_operator_trygetitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_len_hash))
#define DeeType_RequireMapOperatorGetItemStringLenHash(self)    ((DeeMH_map_operator_getitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_len_hash))
#define DeeType_RequireMapOperatorDelItemStringLenHash(self)    ((DeeMH_map_operator_delitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_len_hash))
#define DeeType_RequireMapOperatorSetItemStringLenHash(self)    ((DeeMH_map_operator_setitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_len_hash))
#define DeeType_RequireMapOperatorBoundItemStringLenHash(self)  ((DeeMH_map_operator_bounditem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_len_hash))
#define DeeType_RequireMapOperatorHasItemStringLenHash(self)    ((DeeMH_map_operator_hasitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_len_hash))
#define DeeType_RequireMapOperatorCompareEq(self)               ((DeeMH_map_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_compare_eq))
#define DeeType_RequireMapOperatorTryCompareEq(self)            ((DeeMH_map_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trycompare_eq))
#define DeeType_RequireMapOperatorEq(self)                      ((DeeMH_map_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_eq))
#define DeeType_RequireMapOperatorNe(self)                      ((DeeMH_map_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ne))
#define DeeType_RequireMapOperatorLo(self)                      ((DeeMH_map_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_lo))
#define DeeType_RequireMapOperatorLe(self)                      ((DeeMH_map_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_le))
#define DeeType_RequireMapOperatorGr(self)                      ((DeeMH_map_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_gr))
#define DeeType_RequireMapOperatorGe(self)                      ((DeeMH_map_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ge))
#define DeeType_RequireMapOperatorAdd(self)                     ((DeeMH_map_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_add))
#define DeeType_RequireMapOperatorSub(self)                     ((DeeMH_map_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_sub))
#define DeeType_RequireMapOperatorAnd(self)                     ((DeeMH_map_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_and))
#define DeeType_RequireMapOperatorXor(self)                     ((DeeMH_map_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_xor))
#define DeeType_RequireMapOperatorInplaceAdd(self)              ((DeeMH_map_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_add))
#define DeeType_RequireMapOperatorInplaceSub(self)              ((DeeMH_map_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_sub))
#define DeeType_RequireMapOperatorInplaceAnd(self)              ((DeeMH_map_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_and))
#define DeeType_RequireMapOperatorInplaceXor(self)              ((DeeMH_map_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_xor))
#define DeeType_RequireSeqTryGetFirst(self)                     ((DeeMH_seq_trygetfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetfirst))
#define DeeType_RequireSeqBoundFirst(self)                      ((DeeMH_seq_boundfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_boundfirst))
#define DeeType_RequireSeqGetFirst(self)                        ((DeeMH_seq_getfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_getfirst))
#define DeeType_RequireSeqDelFirst(self)                        ((DeeMH_seq_delfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_delfirst))
#define DeeType_RequireSeqSetFirst(self)                        ((DeeMH_seq_setfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_setfirst))
#define DeeType_RequireSeqTryGetLast(self)                      ((DeeMH_seq_trygetlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetlast))
#define DeeType_RequireSeqBoundLast(self)                       ((DeeMH_seq_boundlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_boundlast))
#define DeeType_RequireSeqGetLast(self)                         ((DeeMH_seq_getlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_getlast))
#define DeeType_RequireSeqDelLast(self)                         ((DeeMH_seq_dellast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_dellast))
#define DeeType_RequireSeqSetLast(self)                         ((DeeMH_seq_setlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_setlast))
#define DeeType_RequireSeqCached(self)                          ((DeeMH_seq_cached_t)DeeType_GetMethodHint(self, Dee_TMH_seq_cached))
#define DeeType_RequireSeqFrozen(self)                          ((DeeMH_seq_frozen_t)DeeType_GetMethodHint(self, Dee_TMH_seq_frozen))
#define DeeType_RequireSeqAny(self)                             ((DeeMH_seq_any_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any))
#define DeeType_RequireSeqAnyWithKey(self)                      ((DeeMH_seq_any_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_key))
#define DeeType_RequireSeqAnyWithRange(self)                    ((DeeMH_seq_any_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range))
#define DeeType_RequireSeqAnyWithRangeAndKey(self)              ((DeeMH_seq_any_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range_and_key))
#define DeeType_RequireSeqAll(self)                             ((DeeMH_seq_all_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all))
#define DeeType_RequireSeqAllWithKey(self)                      ((DeeMH_seq_all_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_key))
#define DeeType_RequireSeqAllWithRange(self)                    ((DeeMH_seq_all_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range))
#define DeeType_RequireSeqAllWithRangeAndKey(self)              ((DeeMH_seq_all_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range_and_key))
#define DeeType_RequireSeqParity(self)                          ((DeeMH_seq_parity_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity))
#define DeeType_RequireSeqParityWithKey(self)                   ((DeeMH_seq_parity_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_key))
#define DeeType_RequireSeqParityWithRange(self)                 ((DeeMH_seq_parity_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range))
#define DeeType_RequireSeqParityWithRangeAndKey(self)           ((DeeMH_seq_parity_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range_and_key))
#define DeeType_RequireSeqReduce(self)                          ((DeeMH_seq_reduce_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce))
#define DeeType_RequireSeqReduceWithInit(self)                  ((DeeMH_seq_reduce_with_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_init))
#define DeeType_RequireSeqReduceWithRange(self)                 ((DeeMH_seq_reduce_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range))
#define DeeType_RequireSeqReduceWithRangeAndInit(self)          ((DeeMH_seq_reduce_with_range_and_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range_and_init))
#define DeeType_RequireSeqMin(self)                             ((DeeMH_seq_min_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min))
#define DeeType_RequireSeqMinWithKey(self)                      ((DeeMH_seq_min_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_key))
#define DeeType_RequireSeqMinWithRange(self)                    ((DeeMH_seq_min_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range))
#define DeeType_RequireSeqMinWithRangeAndKey(self)              ((DeeMH_seq_min_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range_and_key))
#define DeeType_RequireSeqMax(self)                             ((DeeMH_seq_max_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max))
#define DeeType_RequireSeqMaxWithKey(self)                      ((DeeMH_seq_max_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_key))
#define DeeType_RequireSeqMaxWithRange(self)                    ((DeeMH_seq_max_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range))
#define DeeType_RequireSeqMaxWithRangeAndKey(self)              ((DeeMH_seq_max_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range_and_key))
#define DeeType_RequireSeqSum(self)                             ((DeeMH_seq_sum_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum))
#define DeeType_RequireSeqSumWithRange(self)                    ((DeeMH_seq_sum_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum_with_range))
#define DeeType_RequireSeqCount(self)                           ((DeeMH_seq_count_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count))
#define DeeType_RequireSeqCountWithKey(self)                    ((DeeMH_seq_count_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_key))
#define DeeType_RequireSeqCountWithRange(self)                  ((DeeMH_seq_count_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range))
#define DeeType_RequireSeqCountWithRangeAndKey(self)            ((DeeMH_seq_count_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range_and_key))
#define DeeType_RequireSeqContains(self)                        ((DeeMH_seq_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains))
#define DeeType_RequireSeqContainsWithKey(self)                 ((DeeMH_seq_contains_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_key))
#define DeeType_RequireSeqContainsWithRange(self)               ((DeeMH_seq_contains_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range))
#define DeeType_RequireSeqContainsWithRangeAndKey(self)         ((DeeMH_seq_contains_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range_and_key))
#define DeeType_RequireSeqLocate(self)                          ((DeeMH_seq_locate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate))
#define DeeType_RequireSeqLocateWithRange(self)                 ((DeeMH_seq_locate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate_with_range))
#define DeeType_RequireSeqRLocate(self)                         ((DeeMH_seq_rlocate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rlocate))
#define DeeType_RequireSeqRLocateWithRange(self)                ((DeeMH_seq_rlocate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rlocate_with_range))
#define DeeType_RequireSeqStartsWith(self)                      ((DeeMH_seq_startswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith))
#define DeeType_RequireSeqStartsWithWithKey(self)               ((DeeMH_seq_startswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_key))
#define DeeType_RequireSeqStartsWithWithRange(self)             ((DeeMH_seq_startswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range))
#define DeeType_RequireSeqStartsWithWithRangeAndKey(self)       ((DeeMH_seq_startswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range_and_key))
#define DeeType_RequireSeqEndsWith(self)                        ((DeeMH_seq_endswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith))
#define DeeType_RequireSeqEndsWithWithKey(self)                 ((DeeMH_seq_endswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_key))
#define DeeType_RequireSeqEndsWithWithRange(self)               ((DeeMH_seq_endswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range))
#define DeeType_RequireSeqEndsWithWithRangeAndKey(self)         ((DeeMH_seq_endswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range_and_key))
#define DeeType_RequireSeqFind(self)                            ((DeeMH_seq_find_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find))
#define DeeType_RequireSeqFindWithKey(self)                     ((DeeMH_seq_find_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find_with_key))
#define DeeType_RequireSeqRFind(self)                           ((DeeMH_seq_rfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind))
#define DeeType_RequireSeqRFindWithKey(self)                    ((DeeMH_seq_rfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind_with_key))
#define DeeType_RequireSeqErase(self)                           ((DeeMH_seq_erase_t)DeeType_GetMethodHint(self, Dee_TMH_seq_erase))
#define DeeType_RequireSeqInsert(self)                          ((DeeMH_seq_insert_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insert))
#define DeeType_RequireSeqInsertAll(self)                       ((DeeMH_seq_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insertall))
#define DeeType_RequireSeqPushFront(self)                       ((DeeMH_seq_pushfront_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pushfront))
#define DeeType_RequireSeqAppend(self)                          ((DeeMH_seq_append_t)DeeType_GetMethodHint(self, Dee_TMH_seq_append))
#define DeeType_RequireSeqExtend(self)                          ((DeeMH_seq_extend_t)DeeType_GetMethodHint(self, Dee_TMH_seq_extend))
#define DeeType_RequireSeqXchItemIndex(self)                    ((DeeMH_seq_xchitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_xchitem_index))
#define DeeType_RequireSeqClear(self)                           ((DeeMH_seq_clear_t)DeeType_GetMethodHint(self, Dee_TMH_seq_clear))
#define DeeType_RequireSeqPop(self)                             ((DeeMH_seq_pop_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pop))
#define DeeType_RequireSeqRemove(self)                          ((DeeMH_seq_remove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove))
#define DeeType_RequireSeqRemoveWithKey(self)                   ((DeeMH_seq_remove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove_with_key))
#define DeeType_RequireSeqRRemove(self)                         ((DeeMH_seq_rremove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove))
#define DeeType_RequireSeqRRemoveWithKey(self)                  ((DeeMH_seq_rremove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove_with_key))
#define DeeType_RequireSeqRemoveAll(self)                       ((DeeMH_seq_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall))
#define DeeType_RequireSeqRemoveAllWithKey(self)                ((DeeMH_seq_removeall_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall_with_key))
#define DeeType_RequireSeqRemoveIf(self)                        ((DeeMH_seq_removeif_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeif))
#define DeeType_RequireSeqResize(self)                          ((DeeMH_seq_resize_t)DeeType_GetMethodHint(self, Dee_TMH_seq_resize))
#define DeeType_RequireSeqFill(self)                            ((DeeMH_seq_fill_t)DeeType_GetMethodHint(self, Dee_TMH_seq_fill))
#define DeeType_RequireSeqReverse(self)                         ((DeeMH_seq_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reverse))
#define DeeType_RequireSeqReversed(self)                        ((DeeMH_seq_reversed_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reversed))
#define DeeType_RequireSeqSort(self)                            ((DeeMH_seq_sort_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort))
#define DeeType_RequireSeqSortWithKey(self)                     ((DeeMH_seq_sort_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort_with_key))
#define DeeType_RequireSeqSorted(self)                          ((DeeMH_seq_sorted_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted))
#define DeeType_RequireSeqSortedWithKey(self)                   ((DeeMH_seq_sorted_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted_with_key))
#define DeeType_RequireSeqBFind(self)                           ((DeeMH_seq_bfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind))
#define DeeType_RequireSeqBFindWithKey(self)                    ((DeeMH_seq_bfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind_with_key))
#define DeeType_RequireSeqBPosition(self)                       ((DeeMH_seq_bposition_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition))
#define DeeType_RequireSeqBPositionWithKey(self)                ((DeeMH_seq_bposition_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition_with_key))
#define DeeType_RequireSeqBRange(self)                          ((DeeMH_seq_brange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange))
#define DeeType_RequireSeqBRangeWithKey(self)                   ((DeeMH_seq_brange_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange_with_key))
#define DeeType_RequireSetInsert(self)                          ((DeeMH_set_insert_t)DeeType_GetMethodHint(self, Dee_TMH_set_insert))
#define DeeType_RequireSetRemove(self)                          ((DeeMH_set_remove_t)DeeType_GetMethodHint(self, Dee_TMH_set_remove))
#define DeeType_RequireSetUnify(self)                           ((DeeMH_set_unify_t)DeeType_GetMethodHint(self, Dee_TMH_set_unify))
#define DeeType_RequireSetInsertAll(self)                       ((DeeMH_set_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_set_insertall))
#define DeeType_RequireSetRemoveAll(self)                       ((DeeMH_set_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_set_removeall))
#define DeeType_RequireSetPop(self)                             ((DeeMH_set_pop_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop))
#define DeeType_RequireSetPopWithDefault(self)                  ((DeeMH_set_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop_with_default))
#define DeeType_RequireMapEnumerate(self)                       ((DeeMH_map_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_map_enumerate))
#define DeeType_RequireMapSetOld(self)                          ((DeeMH_map_setold_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold))
#define DeeType_RequireMapSetOldEx(self)                        ((DeeMH_map_setold_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold_ex))
#define DeeType_RequireMapSetNew(self)                          ((DeeMH_map_setnew_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew))
#define DeeType_RequireMapSetNewEx(self)                        ((DeeMH_map_setnew_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew_ex))
#define DeeType_RequireMapSetDefault(self)                      ((DeeMH_map_setdefault_t)DeeType_GetMethodHint(self, Dee_TMH_map_setdefault))
#define DeeType_RequireMapUpdate(self)                          ((DeeMH_map_update_t)DeeType_GetMethodHint(self, Dee_TMH_map_update))
#define DeeType_RequireMapRemove(self)                          ((DeeMH_map_remove_t)DeeType_GetMethodHint(self, Dee_TMH_map_remove))
#define DeeType_RequireMapRemoveKeys(self)                      ((DeeMH_map_removekeys_t)DeeType_GetMethodHint(self, Dee_TMH_map_removekeys))
#define DeeType_RequireMapPop(self)                             ((DeeMH_map_pop_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop))
#define DeeType_RequireMapPopWithDefault(self)                  ((DeeMH_map_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop_with_default))
#define DeeType_RequireMapPopItem(self)                         ((DeeMH_map_popitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_popitem))


/* Generic sequence operators: treat "self" as a (possibly writable) indexable sequence.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Sequence", in which case it is treated
 * like an empty sequence).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeSeq_OperatorBool(self)                                  (*DeeType_RequireSeqOperatorBool(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorIter(self)                                  (*DeeType_RequireSeqOperatorIter(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorSizeOb(self)                                (*DeeType_RequireSeqOperatorSizeOb(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorContains(self, some_object)                 (*DeeType_RequireSeqOperatorContains(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGetItem(self, index)                        (*DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItem(self, index)                        (*DeeType_RequireSeqOperatorDelItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItem(self, index, value)                 (*DeeType_RequireSeqOperatorSetItem(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorGetRange(self, start, end)                  (*DeeType_RequireSeqOperatorGetRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRange(self, start, end)                  (*DeeType_RequireSeqOperatorDelRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRange(self, start, end, values)          (*DeeType_RequireSeqOperatorSetRange(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorForeach(self, proc, arg)                    (*DeeType_RequireSeqOperatorForeach(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorForeachPair(self, proc, arg)                (*DeeType_RequireSeqOperatorForeachPair(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorIterKeys(self)                              (*DeeType_RequireSeqOperatorIterKeys(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorBoundItem(self, index)                      (*DeeType_RequireSeqOperatorBoundItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItem(self, index)                        (*DeeType_RequireSeqOperatorHasItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSize(self)                                  (*DeeType_RequireSeqOperatorSize(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorGetItemIndex(self, index)                   (*DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItemIndex(self, index)                   (*DeeType_RequireSeqOperatorDelItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItemIndex(self, index, value)            (*DeeType_RequireSeqOperatorSetItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorBoundItemIndex(self, index)                 (*DeeType_RequireSeqOperatorBoundItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItemIndex(self, index)                   (*DeeType_RequireSeqOperatorHasItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorGetRangeIndex(self, start, end)             (*DeeType_RequireSeqOperatorGetRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRangeIndex(self, start, end)             (*DeeType_RequireSeqOperatorDelRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRangeIndex(self, start, end, values)     (*DeeType_RequireSeqOperatorSetRangeIndex(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorGetRangeIndexN(self, start)                 (*DeeType_RequireSeqOperatorGetRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorDelRangeIndexN(self, start)                 (*DeeType_RequireSeqOperatorDelRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorSetRangeIndexN(self, start, values)         (*DeeType_RequireSeqOperatorSetRangeIndexN(Dee_TYPE(self)))(self, start, values)
#define DeeSeq_OperatorTryGetItem(self, index)                     (*DeeType_RequireSeqOperatorTryGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorTryGetItemIndex(self, index)                (*DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHash(self)                                  (*DeeType_RequireSeqOperatorHash(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorCompareEq(self, some_object)                (*DeeType_RequireSeqOperatorCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorCompare(self, some_object)                  (*DeeType_RequireSeqOperatorCompare(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorTryCompareEq(self, some_object)             (*DeeType_RequireSeqOperatorTryCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorEq(self, some_object)                       (*DeeType_RequireSeqOperatorEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorNe(self, some_object)                       (*DeeType_RequireSeqOperatorNe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLo(self, some_object)                       (*DeeType_RequireSeqOperatorLo(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLe(self, some_object)                       (*DeeType_RequireSeqOperatorLe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGr(self, some_object)                       (*DeeType_RequireSeqOperatorGr(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGe(self, some_object)                       (*DeeType_RequireSeqOperatorGe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorInplaceAdd(p_self, some_object)             (*DeeType_RequireSeqOperatorInplaceAdd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSeq_OperatorInplaceMul(p_self, some_object)             (*DeeType_RequireSeqOperatorInplaceMul(Dee_TYPE(*(p_self))))(p_self, some_object)

/* Generic set operators: treat "self" as a (possibly writable) set that can
 * be enumerated (via iter/foreach) to yield keys.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Set", in which case it is treated
 * like an empty set).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeSet_OperatorBool                            DeeSeq_OperatorBool
#define DeeSet_OperatorContains                        DeeSeq_OperatorContains
#define DeeSet_OperatorIter(self)                      (*DeeType_RequireSetOperatorIter(Dee_TYPE(self)))(self)
#define DeeSet_OperatorForeach(self, cb, arg)          (*DeeType_RequireSetOperatorForeach(Dee_TYPE(self)))(self, cb, arg)
#define DeeSet_OperatorForeachPair(self, cb, arg)      (*DeeType_RequireSetOperatorForeachPair(Dee_TYPE(self)))(self, cb, arg)
#define DeeSet_OperatorSize(self)                      (*DeeType_RequireSetOperatorSize(Dee_TYPE(self)))(self)
#define DeeSet_OperatorSizeOb(self)                    (*DeeType_RequireSetOperatorSizeOb(Dee_TYPE(self)))(self)
#define DeeSet_OperatorHash(self)                      (*DeeType_RequireSetOperatorHash(Dee_TYPE(self)))(self)
#define DeeSet_OperatorCompareEq(self, some_object)    (*DeeType_RequireSetOperatorCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorTryCompareEq(self, some_object) (*DeeType_RequireSetOperatorTryCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorEq(self, some_object)           (*DeeType_RequireSetOperatorEq(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorNe(self, some_object)           (*DeeType_RequireSetOperatorNe(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorLo(self, some_object)           (*DeeType_RequireSetOperatorLo(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorLe(self, some_object)           (*DeeType_RequireSetOperatorLe(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorGr(self, some_object)           (*DeeType_RequireSetOperatorGr(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorGe(self, some_object)           (*DeeType_RequireSetOperatorGe(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorInv(self)                       (*DeeType_RequireSetOperatorInv(Dee_TYPE(self)))(self)
#define DeeSet_OperatorAdd(self, some_object)          (*DeeType_RequireSetOperatorAdd(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorSub(self, some_object)          (*DeeType_RequireSetOperatorSub(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorAnd(self, some_object)          (*DeeType_RequireSetOperatorAnd(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorXor(self, some_object)          (*DeeType_RequireSetOperatorXor(Dee_TYPE(self)))(self, some_object)
#define DeeSet_OperatorInplaceAdd(p_self, some_object) (*DeeType_RequireSetOperatorInplaceAdd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSet_OperatorInplaceSub(p_self, some_object) (*DeeType_RequireSetOperatorInplaceSub(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSet_OperatorInplaceAnd(p_self, some_object) (*DeeType_RequireSetOperatorInplaceAnd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSet_OperatorInplaceXor(p_self, some_object) (*DeeType_RequireSetOperatorInplaceXor(Dee_TYPE(*(p_self))))(p_self, some_object)

/* Generic map operators: treat "self" as a (possibly writable) map that
 * enumerates to yield (key, value) pairs.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Mapping", in which case it is treated
 * like an empty map).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeMap_OperatorBool                                                 DeeSet_OperatorBool
#define DeeMap_OperatorIter                                                 DeeSet_OperatorIter
#define DeeMap_OperatorSizeOb                                               DeeSet_OperatorSizeOb
#define DeeMap_OperatorSize                                                 DeeSet_OperatorSize
#define DeeMap_OperatorForeach                                              DeeSet_OperatorForeach
#define DeeMap_OperatorForeachPair                                          DeeSet_OperatorForeachPair
#define DeeMap_OperatorContains(self, some_object)                          (*DeeType_RequireMapOperatorContains(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorGetItem(self, index)                                 (*DeeType_RequireMapOperatorGetItem(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorDelItem(self, index)                                 (*DeeType_RequireMapOperatorDelItem(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorSetItem(self, index, value)                          (*DeeType_RequireMapOperatorSetItem(Dee_TYPE(self)))(self, index, value)
#define DeeMap_OperatorBoundItem(self, index)                               (*DeeType_RequireMapOperatorBoundItem(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorHasItem(self, index)                                 (*DeeType_RequireMapOperatorHasItem(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorGetItemIndex(self, index)                            (*DeeType_RequireMapOperatorGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorDelItemIndex(self, index)                            (*DeeType_RequireMapOperatorDelItemIndex(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorSetItemIndex(self, index, value)                     (*DeeType_RequireMapOperatorSetItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeMap_OperatorBoundItemIndex(self, index)                          (*DeeType_RequireMapOperatorBoundItemIndex(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorHasItemIndex(self, index)                            (*DeeType_RequireMapOperatorHasItemIndex(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorTryGetItem(self, index)                              (*DeeType_RequireMapOperatorTryGetItem(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorTryGetItemIndex(self, index)                         (*DeeType_RequireMapOperatorTryGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeMap_OperatorTryGetItemStringHash(self, key, hash)                (*DeeType_RequireMapOperatorTryGetItemStringHash(Dee_TYPE(self)))(self, key, hash)
#define DeeMap_OperatorGetItemStringHash(self, key, hash)                   (*DeeType_RequireMapOperatorGetItemStringHash(Dee_TYPE(self)))(self, key, hash)
#define DeeMap_OperatorDelItemStringHash(self, key, hash)                   (*DeeType_RequireMapOperatorDelItemStringHash(Dee_TYPE(self)))(self, key, hash)
#define DeeMap_OperatorSetItemStringHash(self, key, hash, value)            (*DeeType_RequireMapOperatorSetItemStringHash(Dee_TYPE(self)))(self, key, hash, value)
#define DeeMap_OperatorBoundItemStringHash(self, key, hash)                 (*DeeType_RequireMapOperatorBoundItemStringHash(Dee_TYPE(self)))(self, key, hash)
#define DeeMap_OperatorHasItemStringHash(self, key, hash)                   (*DeeType_RequireMapOperatorHasItemStringHash(Dee_TYPE(self)))(self, key, hash)
#define DeeMap_OperatorTryGetItemStringLenHash(self, key, keylen, hash)     (*DeeType_RequireMapOperatorTryGetItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash)
#define DeeMap_OperatorGetItemStringLenHash(self, key, keylen, hash)        (*DeeType_RequireMapOperatorGetItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash)
#define DeeMap_OperatorDelItemStringLenHash(self, key, keylen, hash)        (*DeeType_RequireMapOperatorDelItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash)
#define DeeMap_OperatorSetItemStringLenHash(self, key, keylen, hash, value) (*DeeType_RequireMapOperatorSetItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash, value)
#define DeeMap_OperatorBoundItemStringLenHash(self, key, keylen, hash)      (*DeeType_RequireMapOperatorBoundItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash)
#define DeeMap_OperatorHasItemStringLenHash(self, key, keylen, hash)        (*DeeType_RequireMapOperatorHasItemStringLenHash(Dee_TYPE(self)))(self, key, keylen, hash)
#define DeeMap_OperatorHash                                                 DeeSet_OperatorHash
#define DeeMap_OperatorCompareEq(self, some_object)                         (*DeeType_RequireMapOperatorCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorTryCompareEq(self, some_object)                      (*DeeType_RequireMapOperatorTryCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorEq(self, some_object)                                (*DeeType_RequireMapOperatorEq(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorNe(self, some_object)                                (*DeeType_RequireMapOperatorNe(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorLo(self, some_object)                                (*DeeType_RequireMapOperatorLo(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorLe(self, some_object)                                (*DeeType_RequireMapOperatorLe(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorGr(self, some_object)                                (*DeeType_RequireMapOperatorGr(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorGe(self, some_object)                                (*DeeType_RequireMapOperatorGe(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorAdd(self, some_object)                               (*DeeType_RequireMapOperatorAdd(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorSub(self, some_object)                               (*DeeType_RequireMapOperatorSub(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorAnd(self, some_object)                               (*DeeType_RequireMapOperatorAnd(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorXor(self, some_object)                               (*DeeType_RequireMapOperatorXor(Dee_TYPE(self)))(self, some_object)
#define DeeMap_OperatorInplaceAdd(p_self, some_object)                      (*DeeType_RequireMapOperatorInplaceAdd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeMap_OperatorInplaceSub(p_self, some_object)                      (*DeeType_RequireMapOperatorInplaceSub(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeMap_OperatorInplaceAnd(p_self, some_object)                      (*DeeType_RequireMapOperatorInplaceAnd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeMap_OperatorInplaceXor(p_self, some_object)                      (*DeeType_RequireMapOperatorInplaceXor(Dee_TYPE(*(p_self))))(p_self, some_object)

/* Helpers to quickly invoke default sequence functions. */
#define DeeSeq_InvokeTryGetFirst(self)                                        (*DeeType_RequireSeqTryGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeGetFirst(self)                                           (*DeeType_RequireSeqGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeBoundFirst(self)                                         (*DeeType_RequireSeqBoundFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeDelFirst(self)                                           (*DeeType_RequireSeqDelFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSetFirst(self, v)                                        (*DeeType_RequireSeqSetFirst(Dee_TYPE(self)))(self, v)
#define DeeSeq_InvokeTryGetLast(self)                                         (*DeeType_RequireSeqTryGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeGetLast(self)                                            (*DeeType_RequireSeqGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeBoundLast(self)                                          (*DeeType_RequireSeqBoundLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeDelLast(self)                                            (*DeeType_RequireSeqDelLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSetLast(self, v)                                         (*DeeType_RequireSeqSetLast(Dee_TYPE(self)))(self, v)
#define DeeSeq_InvokeCached(self)                                             (*DeeType_RequireSeqCached(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeFrozen(self)                                             (*DeeType_RequireSeqFrozen(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMakeEnumeration(self)                                    (*DeeType_RequireSeqMakeEnumeration(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end)            (*DeeType_RequireSeqMakeEnumerationWithIntRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMakeEnumerationWithRange(self, start, end)               (*DeeType_RequireSeqMakeEnumerationWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeEnumerate(self, proc, arg)                               (*DeeType_RequireSeqEnumerate(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_InvokeEnumerateIndex(self, proc, arg, start, end)              (*DeeType_RequireSeqEnumerateIndex(Dee_TYPE(self)))(self, proc, arg, start, end)
#define DeeSeq_InvokeAny(self)                                                (*DeeType_RequireSeqAny(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeAnyWithKey(self, key)                                    (*DeeType_RequireSeqAnyWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeAnyWithRange(self, start, end)                           (*DeeType_RequireSeqAnyWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeAnyWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqAnyWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeAll(self)                                                (*DeeType_RequireSeqAll(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeAllWithKey(self, key)                                    (*DeeType_RequireSeqAllWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeAllWithRange(self, start, end)                           (*DeeType_RequireSeqAllWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeAllWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqAllWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeParity(self)                                             (*DeeType_RequireSeqParity(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeParityWithKey(self, key)                                 (*DeeType_RequireSeqParityWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeParityWithRange(self, start, end)                        (*DeeType_RequireSeqParityWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeParityWithRangeAndKey(self, start, end, key)             (*DeeType_RequireSeqParityWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeReduce(self, combine)                                    (*DeeType_RequireSeqReduce(Dee_TYPE(self)))(self, combine)
#define DeeSeq_InvokeReduceWithInit(self, combine, init)                      (*DeeType_RequireSeqReduceWithInit(Dee_TYPE(self)))(self, combine, init)
#define DeeSeq_InvokeReduceWithRange(self, combine, start, end)               (*DeeType_RequireSeqReduceWithRange(Dee_TYPE(self)))(self, combine, start, end)
#define DeeSeq_InvokeReduceWithRangeAndInit(self, combine, start, end, init)  (*DeeType_RequireSeqReduceWithRangeAndInit(Dee_TYPE(self)))(self, combine, start, end, init)
#define DeeSeq_InvokeMin(self)                                                (*DeeType_RequireSeqMin(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMinWithKey(self, key)                                    (*DeeType_RequireSeqMinWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeMinWithRange(self, start, end)                           (*DeeType_RequireSeqMinWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMinWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqMinWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeMax(self)                                                (*DeeType_RequireSeqMax(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMaxWithKey(self, key)                                    (*DeeType_RequireSeqMaxWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeMaxWithRange(self, start, end)                           (*DeeType_RequireSeqMaxWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMaxWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqMaxWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeSum(self)                                                (*DeeType_RequireSeqSum(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSumWithRange(self, start, end)                           (*DeeType_RequireSeqSumWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeCount(self, item)                                        (*DeeType_RequireSeqCount(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeCountWithKey(self, item, key)                            (*DeeType_RequireSeqCountWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeCountWithRange(self, item, start, end)                   (*DeeType_RequireSeqCountWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeCountWithRangeAndKey(self, item, start, end, key)        (*DeeType_RequireSeqCountWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeContains(self, item)                                     (*DeeType_RequireSeqContains(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeContainsWithKey(self, item, key)                         (*DeeType_RequireSeqContainsWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeContainsWithRange(self, item, start, end)                (*DeeType_RequireSeqContainsWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeContainsWithRangeAndKey(self, item, start, end, key)     (*DeeType_RequireSeqContainsWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeLocate(self, match, def)                                 (*DeeType_RequireSeqLocate(Dee_TYPE(self)))(self, match, def)
#define DeeSeq_InvokeLocateWithRange(self, match, start, end, def)            (*DeeType_RequireSeqLocateWithRange(Dee_TYPE(self)))(self, match, start, end, def)
#define DeeSeq_InvokeRLocate(self, match, def)                                (*DeeType_RequireSeqRLocate(Dee_TYPE(self)))(self, match, def)
#define DeeSeq_InvokeRLocateWithRange(self, match, start, end, def)           (*DeeType_RequireSeqRLocateWithRange(Dee_TYPE(self)))(self, match, start, end, def)
#define DeeSeq_InvokeStartsWith(self, item)                                   (*DeeType_RequireSeqStartsWith(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeStartsWithWithKey(self, item, key)                       (*DeeType_RequireSeqStartsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeStartsWithWithRange(self, item, start, end)              (*DeeType_RequireSeqStartsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeStartsWithWithRangeAndKey(self, item, start, end, key)   (*DeeType_RequireSeqStartsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeEndsWith(self, item)                                     (*DeeType_RequireSeqEndsWith(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeEndsWithWithKey(self, item, key)                         (*DeeType_RequireSeqEndsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeEndsWithWithRange(self, item, start, end)                (*DeeType_RequireSeqEndsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeEndsWithWithRangeAndKey(self, item, start, end, key)     (*DeeType_RequireSeqEndsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeFind(self, item, start, end)                             (*DeeType_RequireSeqFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeFindWithKey(self, item, start, end, key)                 (*DeeType_RequireSeqFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRFind(self, item, start, end)                            (*DeeType_RequireSeqRFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRFindWithKey(self, item, start, end, key)                (*DeeType_RequireSeqRFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeErase(self, index, count)                                (*DeeType_RequireSeqErase(Dee_TYPE(self)))(self, index, count)
#define DeeSeq_InvokeInsert(self, index, item)                                (*DeeType_RequireSeqInsert(Dee_TYPE(self)))(self, index, item)
#define DeeSeq_InvokeInsertAll(self, index, items)                            (*DeeType_RequireSeqInsertAll(Dee_TYPE(self)))(self, index, items)
#define DeeSeq_InvokePushFront(self, item)                                    (*DeeType_RequireSeqPushFront(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeAppend(self, item)                                       (*DeeType_RequireSeqAppend(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeExtend(self, items)                                      (*DeeType_RequireSeqExtend(Dee_TYPE(self)))(self, items)
#define DeeSeq_InvokeXchItemIndex(self, index, value)                         (*DeeType_RequireSeqXchItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_InvokeClear(self)                                              (*DeeType_RequireSeqClear(Dee_TYPE(self)))(self)
#define DeeSeq_InvokePop(self, index)                                         (*DeeType_RequireSeqPop(Dee_TYPE(self)))(self, index)
#define DeeSeq_InvokeRemove(self, item, start, end)                           (*DeeType_RequireSeqRemove(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRemoveWithKey(self, item, start, end, key)               (*DeeType_RequireSeqRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRRemove(self, item, start, end)                          (*DeeType_RequireSeqRRemove(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRRemoveWithKey(self, item, start, end, key)              (*DeeType_RequireSeqRRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRemoveAll(self, item, start, end, max)                   (*DeeType_RequireSeqRemoveAll(Dee_TYPE(self)))(self, item, start, end, max)
#define DeeSeq_InvokeRemoveAllWithKey(self, item, start, end, max, key)       (*DeeType_RequireSeqRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, max, key)
#define DeeSeq_InvokeRemoveIf(self, should, start, end, max)                  (*DeeType_RequireSeqRemoveIf(Dee_TYPE(self)))(self, should, start, end, max)
#define DeeSeq_InvokeResize(self, newsize, filler)                            (*DeeType_RequireSeqResize(Dee_TYPE(self)))(self, newsize, filler)
#define DeeSeq_InvokeFill(self, start, end, filler)                           (*DeeType_RequireSeqFill(Dee_TYPE(self)))(self, start, end, filler)
#define DeeSeq_InvokeReverse(self, start, end)                                (*DeeType_RequireSeqReverse(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeReversed(self, start, end)                               (*DeeType_RequireSeqReversed(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSort(self, start, end)                                   (*DeeType_RequireSeqSort(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSortWithKey(self, start, end, key)                       (*DeeType_RequireSeqSortWithKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeSorted(self, start, end)                                 (*DeeType_RequireSeqSorted(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSortedWithKey(self, start, end, key)                     (*DeeType_RequireSeqSortedWithKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeBFind(self, item, start, end)                            (*DeeType_RequireSeqBFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeBFindWithKey(self, item, start, end, key)                (*DeeType_RequireSeqBFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeBPosition(self, item, start, end)                        (*DeeType_RequireSeqBPosition(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeBPositionWithKey(self, item, start, end, key)            (*DeeType_RequireSeqBPositionWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeBRange(self, item, start, end, result_range)             (*DeeType_RequireSeqBRange(Dee_TYPE(self)))(self, item, start, end, result_range)
#define DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range) (*DeeType_RequireSeqBRangeWithKey(Dee_TYPE(self)))(self, item, start, end, key, result_range)

/* Set functions */
#define DeeSet_InvokeInsert(self, key)              (*DeeType_RequireSetInsert(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeRemove(self, key)              (*DeeType_RequireSetRemove(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeUnify(self, key)               (*DeeType_RequireSetUnify(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeInsertAll(self, keys)          (*DeeType_RequireSetInsertAll(Dee_TYPE(self)))(self, keys)
#define DeeSet_InvokeRemoveAll(self, keys)          (*DeeType_RequireSetRemoveAll(Dee_TYPE(self)))(self, keys)
#define DeeSet_InvokePop(self)                      (*DeeType_RequireSetPop(Dee_TYPE(self)))(self)
#define DeeSet_InvokePopWithDefault(self, default_) (*DeeType_RequireSetPopWithDefault(Dee_TYPE(self)))(self, default_)

/* Map functions */
#define DeeMap_InvokeEnumerate(self, proc, arg)          (*DeeType_RequireMapEnumerate(Dee_TYPE(self)))(self, proc, arg)
#define DeeMap_InvokeSetOld(self, key, value)            (*DeeType_RequireMapSetOld(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetOldEx(self, key, value)          (*DeeType_RequireMapSetOldEx(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetNew(self, key, value)            (*DeeType_RequireMapSetNew(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetNewEx(self, key, value)          (*DeeType_RequireMapSetNewEx(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetDefault(self, key, value)        (*DeeType_RequireMapSetDefault(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeUpdate(self, items)                 (*DeeType_RequireMapUpdate(Dee_TYPE(self)))(self, items)
#define DeeMap_InvokeRemove(self, key)                   (*DeeType_RequireMapRemove(Dee_TYPE(self)))(self, key)
#define DeeMap_InvokeRemoveKeys(self, keys)              (*DeeType_RequireMapRemoveKeys(Dee_TYPE(self)))(self, keys)
#define DeeMap_InvokePop(self, key)                      (*DeeType_RequireMapPop(Dee_TYPE(self)))(self, key)
#define DeeMap_InvokePopWithDefault(self, key, default_) (*DeeType_RequireMapPopWithDefault(Dee_TYPE(self)))(self, key, default_)
#define DeeMap_InvokePopItem(self)                       (*DeeType_RequireMapPopItem(Dee_TYPE(self)))(self)
#define DeeMap_InvokeKeys(self)                          (*DeeType_RequireMapKeys(Dee_TYPE(self)))(self)
#define DeeMap_InvokeValues(self)                        (*DeeType_RequireMapValues(Dee_TYPE(self)))(self)
#define DeeMap_InvokeIterKeys(self)                      (*DeeType_RequireMapIterKeys(Dee_TYPE(self)))(self)
#define DeeMap_InvokeIterValues(self)                    (*DeeType_RequireMapIterValues(Dee_TYPE(self)))(self)

#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

typedef Dee_seq_enumerate_t Dee_seq_enumerate_t;
typedef Dee_seq_enumerate_index_t Dee_seq_enumerate_index_t;

enum Dee_tmh_id {
	/* !!! CAUTION !!! Method hint IDs are prone to arbitrarily change !!!
	 *
	 * Do not make use of these IDs if you're developing a DEX module and
	 * wish to remain compatible with the deemon core across many version. */
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	Dee_TMH_##func_name,
#include "method-hints.def"
	Dee_TMH_COUNT
};

enum {
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	_Dee_TMH_WRAPPER_FLAGS_##attr_name = wrapper_flags,
#include "method-hints.def"
};

#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	typedef Treturn (cc *Dee_mh_##func_name##_t) params;
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeMH_##attr_name) wrapper_params;           \
	DDATDEF char const DeeMH_##attr_name##_doc[];                                                    \
	DDATDEF char const DeeMH_##attr_name##_name[];
#define Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(new_attr_name, alias_method_name, old_attr_name) \
	DDATDEF char const DeeMH_##new_attr_name##_name[];
#include "method-hints.def"


/*[[[deemon
@@Method hint aliases: (nameAsUsedIn_TYPE_METHOD_HINTREF, attributeName, nameOfAliasedHint)
local MH_ALIASES = [];
#define Dee_DEFINE_TYPE_METHOD_HINT_ALIAS(new_attr_name, alias_method_name, old_attr_name) \
	MH_ALIASES.append((#new_attr_name, alias_method_name, #old_attr_name));
#include "method-hints.def"

local longestNewName = MH_ALIASES.each.first.length > ...;
for (local newName, none, oldName: MH_ALIASES) {
	print("#define DeeMH_", newName.ljust(longestNewName), "                  DeeMH_", oldName);
	print("#define DeeMH_", (newName + "_doc").ljust(longestNewName + 4), "              DeeMH_", oldName, "_doc");
	print("#define _Dee_TMH_WRAPPER_FLAGS_", newName.ljust(longestNewName), " _Dee_TMH_WRAPPER_FLAGS_", oldName);
}
]]]*/
#define DeeMH_explicit_seq_enumerate                   DeeMH_seq_enumerate
#define DeeMH_explicit_seq_enumerate_doc               DeeMH_seq_enumerate_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_enumerate  _Dee_TMH_WRAPPER_FLAGS_seq_enumerate
#define DeeMH_explicit_seq_any                         DeeMH_seq_any
#define DeeMH_explicit_seq_any_doc                     DeeMH_seq_any_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_any        _Dee_TMH_WRAPPER_FLAGS_seq_any
#define DeeMH_explicit_seq_all                         DeeMH_seq_all
#define DeeMH_explicit_seq_all_doc                     DeeMH_seq_all_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_all        _Dee_TMH_WRAPPER_FLAGS_seq_all
#define DeeMH_explicit_seq_parity                      DeeMH_seq_parity
#define DeeMH_explicit_seq_parity_doc                  DeeMH_seq_parity_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_parity     _Dee_TMH_WRAPPER_FLAGS_seq_parity
#define DeeMH_explicit_seq_reduce                      DeeMH_seq_reduce
#define DeeMH_explicit_seq_reduce_doc                  DeeMH_seq_reduce_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_reduce     _Dee_TMH_WRAPPER_FLAGS_seq_reduce
#define DeeMH_explicit_seq_min                         DeeMH_seq_min
#define DeeMH_explicit_seq_min_doc                     DeeMH_seq_min_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_min        _Dee_TMH_WRAPPER_FLAGS_seq_min
#define DeeMH_explicit_seq_max                         DeeMH_seq_max
#define DeeMH_explicit_seq_max_doc                     DeeMH_seq_max_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_max        _Dee_TMH_WRAPPER_FLAGS_seq_max
#define DeeMH_explicit_seq_sum                         DeeMH_seq_sum
#define DeeMH_explicit_seq_sum_doc                     DeeMH_seq_sum_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_sum        _Dee_TMH_WRAPPER_FLAGS_seq_sum
#define DeeMH_explicit_seq_count                       DeeMH_seq_count
#define DeeMH_explicit_seq_count_doc                   DeeMH_seq_count_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_count      _Dee_TMH_WRAPPER_FLAGS_seq_count
#define DeeMH_explicit_seq_contains                    DeeMH_seq_contains
#define DeeMH_explicit_seq_contains_doc                DeeMH_seq_contains_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_contains   _Dee_TMH_WRAPPER_FLAGS_seq_contains
#define DeeMH_explicit_seq_locate                      DeeMH_seq_locate
#define DeeMH_explicit_seq_locate_doc                  DeeMH_seq_locate_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_locate     _Dee_TMH_WRAPPER_FLAGS_seq_locate
#define DeeMH_explicit_seq_rlocate                     DeeMH_seq_rlocate
#define DeeMH_explicit_seq_rlocate_doc                 DeeMH_seq_rlocate_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_rlocate    _Dee_TMH_WRAPPER_FLAGS_seq_rlocate
#define DeeMH_explicit_seq_startswith                  DeeMH_seq_startswith
#define DeeMH_explicit_seq_startswith_doc              DeeMH_seq_startswith_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_startswith _Dee_TMH_WRAPPER_FLAGS_seq_startswith
#define DeeMH_explicit_seq_endswith                    DeeMH_seq_endswith
#define DeeMH_explicit_seq_endswith_doc                DeeMH_seq_endswith_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_endswith   _Dee_TMH_WRAPPER_FLAGS_seq_endswith
#define DeeMH_explicit_seq_find                        DeeMH_seq_find
#define DeeMH_explicit_seq_find_doc                    DeeMH_seq_find_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_find       _Dee_TMH_WRAPPER_FLAGS_seq_find
#define DeeMH_explicit_seq_rfind                       DeeMH_seq_rfind
#define DeeMH_explicit_seq_rfind_doc                   DeeMH_seq_rfind_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_rfind      _Dee_TMH_WRAPPER_FLAGS_seq_rfind
#define DeeMH_explicit_seq_erase                       DeeMH_seq_erase
#define DeeMH_explicit_seq_erase_doc                   DeeMH_seq_erase_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_erase      _Dee_TMH_WRAPPER_FLAGS_seq_erase
#define DeeMH_explicit_seq_insert                      DeeMH_seq_insert
#define DeeMH_explicit_seq_insert_doc                  DeeMH_seq_insert_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_insert     _Dee_TMH_WRAPPER_FLAGS_seq_insert
#define DeeMH_explicit_seq_insertall                   DeeMH_seq_insertall
#define DeeMH_explicit_seq_insertall_doc               DeeMH_seq_insertall_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_insertall  _Dee_TMH_WRAPPER_FLAGS_seq_insertall
#define DeeMH_explicit_seq_pushfront                   DeeMH_seq_pushfront
#define DeeMH_explicit_seq_pushfront_doc               DeeMH_seq_pushfront_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_pushfront  _Dee_TMH_WRAPPER_FLAGS_seq_pushfront
#define DeeMH_explicit_seq_append                      DeeMH_seq_append
#define DeeMH_explicit_seq_append_doc                  DeeMH_seq_append_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_append     _Dee_TMH_WRAPPER_FLAGS_seq_append
#define DeeMH_explicit_seq_extend                      DeeMH_seq_extend
#define DeeMH_explicit_seq_extend_doc                  DeeMH_seq_extend_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_extend     _Dee_TMH_WRAPPER_FLAGS_seq_extend
#define DeeMH_explicit_seq_xchitem                     DeeMH_seq_xchitem
#define DeeMH_explicit_seq_xchitem_doc                 DeeMH_seq_xchitem_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_xchitem    _Dee_TMH_WRAPPER_FLAGS_seq_xchitem
#define DeeMH_explicit_seq_clear                       DeeMH_seq_clear
#define DeeMH_explicit_seq_clear_doc                   DeeMH_seq_clear_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_clear      _Dee_TMH_WRAPPER_FLAGS_seq_clear
#define DeeMH_explicit_seq_pop                         DeeMH_seq_pop
#define DeeMH_explicit_seq_pop_doc                     DeeMH_seq_pop_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_pop        _Dee_TMH_WRAPPER_FLAGS_seq_pop
#define DeeMH_explicit_seq_remove                      DeeMH_seq_remove
#define DeeMH_explicit_seq_remove_doc                  DeeMH_seq_remove_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_remove     _Dee_TMH_WRAPPER_FLAGS_seq_remove
#define DeeMH_explicit_seq_rremove                     DeeMH_seq_rremove
#define DeeMH_explicit_seq_rremove_doc                 DeeMH_seq_rremove_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_rremove    _Dee_TMH_WRAPPER_FLAGS_seq_rremove
#define DeeMH_explicit_seq_removeall                   DeeMH_seq_removeall
#define DeeMH_explicit_seq_removeall_doc               DeeMH_seq_removeall_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_removeall  _Dee_TMH_WRAPPER_FLAGS_seq_removeall
#define DeeMH_explicit_seq_removeif                    DeeMH_seq_removeif
#define DeeMH_explicit_seq_removeif_doc                DeeMH_seq_removeif_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_removeif   _Dee_TMH_WRAPPER_FLAGS_seq_removeif
#define DeeMH_explicit_seq_resize                      DeeMH_seq_resize
#define DeeMH_explicit_seq_resize_doc                  DeeMH_seq_resize_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_resize     _Dee_TMH_WRAPPER_FLAGS_seq_resize
#define DeeMH_explicit_seq_fill                        DeeMH_seq_fill
#define DeeMH_explicit_seq_fill_doc                    DeeMH_seq_fill_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_fill       _Dee_TMH_WRAPPER_FLAGS_seq_fill
#define DeeMH_explicit_seq_reverse                     DeeMH_seq_reverse
#define DeeMH_explicit_seq_reverse_doc                 DeeMH_seq_reverse_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_reverse    _Dee_TMH_WRAPPER_FLAGS_seq_reverse
#define DeeMH_explicit_seq_reversed                    DeeMH_seq_reversed
#define DeeMH_explicit_seq_reversed_doc                DeeMH_seq_reversed_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_reversed   _Dee_TMH_WRAPPER_FLAGS_seq_reversed
#define DeeMH_explicit_seq_sort                        DeeMH_seq_sort
#define DeeMH_explicit_seq_sort_doc                    DeeMH_seq_sort_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_sort       _Dee_TMH_WRAPPER_FLAGS_seq_sort
#define DeeMH_explicit_seq_sorted                      DeeMH_seq_sorted
#define DeeMH_explicit_seq_sorted_doc                  DeeMH_seq_sorted_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_sorted     _Dee_TMH_WRAPPER_FLAGS_seq_sorted
#define DeeMH_explicit_seq_bfind                       DeeMH_seq_bfind
#define DeeMH_explicit_seq_bfind_doc                   DeeMH_seq_bfind_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_bfind      _Dee_TMH_WRAPPER_FLAGS_seq_bfind
#define DeeMH_explicit_seq_bposition                   DeeMH_seq_bposition
#define DeeMH_explicit_seq_bposition_doc               DeeMH_seq_bposition_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_bposition  _Dee_TMH_WRAPPER_FLAGS_seq_bposition
#define DeeMH_explicit_seq_brange                      DeeMH_seq_brange
#define DeeMH_explicit_seq_brange_doc                  DeeMH_seq_brange_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_seq_brange     _Dee_TMH_WRAPPER_FLAGS_seq_brange
#define DeeMH_explicit_set_insert                      DeeMH_set_insert
#define DeeMH_explicit_set_insert_doc                  DeeMH_set_insert_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_insert     _Dee_TMH_WRAPPER_FLAGS_set_insert
#define DeeMH_explicit_set_remove                      DeeMH_set_remove
#define DeeMH_explicit_set_remove_doc                  DeeMH_set_remove_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_remove     _Dee_TMH_WRAPPER_FLAGS_set_remove
#define DeeMH_explicit_set_unify                       DeeMH_set_unify
#define DeeMH_explicit_set_unify_doc                   DeeMH_set_unify_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_unify      _Dee_TMH_WRAPPER_FLAGS_set_unify
#define DeeMH_explicit_set_insertall                   DeeMH_set_insertall
#define DeeMH_explicit_set_insertall_doc               DeeMH_set_insertall_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_insertall  _Dee_TMH_WRAPPER_FLAGS_set_insertall
#define DeeMH_explicit_set_removeall                   DeeMH_set_removeall
#define DeeMH_explicit_set_removeall_doc               DeeMH_set_removeall_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_removeall  _Dee_TMH_WRAPPER_FLAGS_set_removeall
#define DeeMH_explicit_set_pop                         DeeMH_set_pop
#define DeeMH_explicit_set_pop_doc                     DeeMH_set_pop_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_set_pop        _Dee_TMH_WRAPPER_FLAGS_set_pop
#define DeeMH_explicit_map_setold                      DeeMH_map_setold
#define DeeMH_explicit_map_setold_doc                  DeeMH_map_setold_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_setold     _Dee_TMH_WRAPPER_FLAGS_map_setold
#define DeeMH_explicit_map_setold_ex                   DeeMH_map_setold_ex
#define DeeMH_explicit_map_setold_ex_doc               DeeMH_map_setold_ex_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_setold_ex  _Dee_TMH_WRAPPER_FLAGS_map_setold_ex
#define DeeMH_explicit_map_setnew                      DeeMH_map_setnew
#define DeeMH_explicit_map_setnew_doc                  DeeMH_map_setnew_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_setnew     _Dee_TMH_WRAPPER_FLAGS_map_setnew
#define DeeMH_explicit_map_setnew_ex                   DeeMH_map_setnew_ex
#define DeeMH_explicit_map_setnew_ex_doc               DeeMH_map_setnew_ex_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_setnew_ex  _Dee_TMH_WRAPPER_FLAGS_map_setnew_ex
#define DeeMH_explicit_map_setdefault                  DeeMH_map_setdefault
#define DeeMH_explicit_map_setdefault_doc              DeeMH_map_setdefault_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_setdefault _Dee_TMH_WRAPPER_FLAGS_map_setdefault
#define DeeMH_explicit_map_update                      DeeMH_map_update
#define DeeMH_explicit_map_update_doc                  DeeMH_map_update_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_update     _Dee_TMH_WRAPPER_FLAGS_map_update
#define DeeMH_explicit_map_remove                      DeeMH_map_remove
#define DeeMH_explicit_map_remove_doc                  DeeMH_map_remove_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_remove     _Dee_TMH_WRAPPER_FLAGS_map_remove
#define DeeMH_explicit_map_removekeys                  DeeMH_map_removekeys
#define DeeMH_explicit_map_removekeys_doc              DeeMH_map_removekeys_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_removekeys _Dee_TMH_WRAPPER_FLAGS_map_removekeys
#define DeeMH_explicit_map_pop                         DeeMH_map_pop
#define DeeMH_explicit_map_pop_doc                     DeeMH_map_pop_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_pop        _Dee_TMH_WRAPPER_FLAGS_map_pop
#define DeeMH_explicit_map_popitem                     DeeMH_map_popitem
#define DeeMH_explicit_map_popitem_doc                 DeeMH_map_popitem_doc
#define _Dee_TMH_WRAPPER_FLAGS_explicit_map_popitem    _Dee_TMH_WRAPPER_FLAGS_map_popitem
/*[[[end]]]*/

#ifdef CONFIG_BUILDING_DEEMON

/* Type sequence operator definition functions. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_foreach_reverse_t DCALL DeeType_TryRequireSeqForeachReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_enumerate_index_reverse_t DCALL DeeType_TryRequireSeqEnumerateIndexReverse(DeeTypeObject *__restrict self);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_t DCALL DeeType_RequireSeqMakeEnumeration(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_int_range_t DCALL DeeType_RequireSeqMakeEnumerationWithIntRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_range_t DCALL DeeType_RequireSeqMakeEnumerationWithRange(DeeTypeObject *__restrict self);

/*[[[begin:seq_operators]]]*/
/* Sequence operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bool_t DCALL DeeType_RequireSeqOperatorBool(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iter_t DCALL DeeType_RequireSeqOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_sizeob_t DCALL DeeType_RequireSeqOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_contains_t DCALL DeeType_RequireSeqOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_t DCALL DeeType_RequireSeqOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_t DCALL DeeType_RequireSeqOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_t DCALL DeeType_RequireSeqOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_t DCALL DeeType_RequireSeqOperatorGetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_t DCALL DeeType_RequireSeqOperatorDelRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_t DCALL DeeType_RequireSeqOperatorSetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_t DCALL DeeType_RequireSeqOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_pair_t DCALL DeeType_RequireSeqOperatorForeachPair(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_t DCALL DeeType_RequireSeqOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_index_t DCALL DeeType_RequireSeqOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iterkeys_t DCALL DeeType_RequireSeqOperatorIterKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_t DCALL DeeType_RequireSeqOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_t DCALL DeeType_RequireSeqOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_t DCALL DeeType_RequireSeqOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_fast_t DCALL DeeType_RequireSeqOperatorSizeFast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_index_t DCALL DeeType_RequireSeqOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_index_t DCALL DeeType_RequireSeqOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_index_t DCALL DeeType_RequireSeqOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_index_t DCALL DeeType_RequireSeqOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_index_t DCALL DeeType_RequireSeqOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_t DCALL DeeType_RequireSeqOperatorGetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_t DCALL DeeType_RequireSeqOperatorDelRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_t DCALL DeeType_RequireSeqOperatorSetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_n_t DCALL DeeType_RequireSeqOperatorGetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_n_t DCALL DeeType_RequireSeqOperatorDelRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_n_t DCALL DeeType_RequireSeqOperatorSetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_t DCALL DeeType_RequireSeqOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_index_t DCALL DeeType_RequireSeqOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hash_t DCALL DeeType_RequireSeqOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_eq_t DCALL DeeType_RequireSeqOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_t DCALL DeeType_RequireSeqOperatorCompare(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trycompare_eq_t DCALL DeeType_RequireSeqOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_eq_t DCALL DeeType_RequireSeqOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ne_t DCALL DeeType_RequireSeqOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_lo_t DCALL DeeType_RequireSeqOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_le_t DCALL DeeType_RequireSeqOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_gr_t DCALL DeeType_RequireSeqOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ge_t DCALL DeeType_RequireSeqOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_add_t DCALL DeeType_RequireSeqOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_mul_t DCALL DeeType_RequireSeqOperatorInplaceMul(DeeTypeObject *__restrict self);

/* Set operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_iter_t DCALL DeeType_RequireSetOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_foreach_t DCALL DeeType_RequireSetOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_size_t DCALL DeeType_RequireSetOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_sizeob_t DCALL DeeType_RequireSetOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_hash_t DCALL DeeType_RequireSetOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_compare_eq_t DCALL DeeType_RequireSetOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_trycompare_eq_t DCALL DeeType_RequireSetOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_eq_t DCALL DeeType_RequireSetOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ne_t DCALL DeeType_RequireSetOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_lo_t DCALL DeeType_RequireSetOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_le_t DCALL DeeType_RequireSetOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_gr_t DCALL DeeType_RequireSetOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ge_t DCALL DeeType_RequireSetOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inv_t DCALL DeeType_RequireSetOperatorInv(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_add_t DCALL DeeType_RequireSetOperatorAdd(DeeTypeObject *__restrict self); /* {"a"} + {"b"}         -> {"a","b"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_sub_t DCALL DeeType_RequireSetOperatorSub(DeeTypeObject *__restrict self); /* {"a","b"} - {"b"}     -> {"a"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_and_t DCALL DeeType_RequireSetOperatorAnd(DeeTypeObject *__restrict self); /* {"a","b"} & {"a"}     -> {"a"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_xor_t DCALL DeeType_RequireSetOperatorXor(DeeTypeObject *__restrict self); /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_add_t DCALL DeeType_RequireSetOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_sub_t DCALL DeeType_RequireSetOperatorInplaceSub(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_and_t DCALL DeeType_RequireSetOperatorInplaceAnd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_xor_t DCALL DeeType_RequireSetOperatorInplaceXor(DeeTypeObject *__restrict self);

/* Map operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_contains_t DCALL DeeType_RequireMapOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_t DCALL DeeType_RequireMapOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_t DCALL DeeType_RequireMapOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_t DCALL DeeType_RequireMapOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_t DCALL DeeType_RequireMapOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_index_t DCALL DeeType_RequireMapOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_t DCALL DeeType_RequireMapOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_t DCALL DeeType_RequireMapOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_index_t DCALL DeeType_RequireMapOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_index_t DCALL DeeType_RequireMapOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_index_t DCALL DeeType_RequireMapOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_index_t DCALL DeeType_RequireMapOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_index_t DCALL DeeType_RequireMapOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_t DCALL DeeType_RequireMapOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_index_t DCALL DeeType_RequireMapOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_hash_t DCALL DeeType_RequireMapOperatorGetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_hash_t DCALL DeeType_RequireMapOperatorDelItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_hash_t DCALL DeeType_RequireMapOperatorSetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_hash_t DCALL DeeType_RequireMapOperatorHasItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_len_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_len_hash_t DCALL DeeType_RequireMapOperatorGetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_len_hash_t DCALL DeeType_RequireMapOperatorDelItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_len_hash_t DCALL DeeType_RequireMapOperatorSetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_len_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_len_hash_t DCALL DeeType_RequireMapOperatorHasItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_compare_eq_t DCALL DeeType_RequireMapOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trycompare_eq_t DCALL DeeType_RequireMapOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_eq_t DCALL DeeType_RequireMapOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ne_t DCALL DeeType_RequireMapOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_lo_t DCALL DeeType_RequireMapOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_le_t DCALL DeeType_RequireMapOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_gr_t DCALL DeeType_RequireMapOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ge_t DCALL DeeType_RequireMapOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_add_t DCALL DeeType_RequireMapOperatorAdd(DeeTypeObject *__restrict self); /* {"a":1} + {"b":2}       -> {"a":1,"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_sub_t DCALL DeeType_RequireMapOperatorSub(DeeTypeObject *__restrict self); /* {"a":1,"b":2} - {"a"}   -> {"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_and_t DCALL DeeType_RequireMapOperatorAnd(DeeTypeObject *__restrict self); /* {"a":1,"b":2} & {"a"}   -> {"a":1} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_xor_t DCALL DeeType_RequireMapOperatorXor(DeeTypeObject *__restrict self); /* {"a":1,"b":2} ^ {"a":3} -> {"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_add_t DCALL DeeType_RequireMapOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_sub_t DCALL DeeType_RequireMapOperatorInplaceSub(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_and_t DCALL DeeType_RequireMapOperatorInplaceAnd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_xor_t DCALL DeeType_RequireMapOperatorInplaceXor(DeeTypeObject *__restrict self);
/*[[[end:seq_operators]]]*/

/* Sequence function... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetfirst_t DCALL DeeType_RequireSeqTryGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetlast_t DCALL DeeType_RequireSeqTryGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_t DCALL DeeType_RequireSeqAny(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_key_t DCALL DeeType_RequireSeqAnyWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_t DCALL DeeType_RequireSeqAnyWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_and_key_t DCALL DeeType_RequireSeqAnyWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_t DCALL DeeType_RequireSeqAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_key_t DCALL DeeType_RequireSeqAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_t DCALL DeeType_RequireSeqAllWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_and_key_t DCALL DeeType_RequireSeqAllWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_t DCALL DeeType_RequireSeqParity(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_key_t DCALL DeeType_RequireSeqParityWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_t DCALL DeeType_RequireSeqParityWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_and_key_t DCALL DeeType_RequireSeqParityWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_t DCALL DeeType_RequireSeqReduce(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_init_t DCALL DeeType_RequireSeqReduceWithInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_t DCALL DeeType_RequireSeqReduceWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_and_init_t DCALL DeeType_RequireSeqReduceWithRangeAndInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_t DCALL DeeType_RequireSeqMin(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_key_t DCALL DeeType_RequireSeqMinWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_t DCALL DeeType_RequireSeqMinWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_and_key_t DCALL DeeType_RequireSeqMinWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_t DCALL DeeType_RequireSeqMax(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_key_t DCALL DeeType_RequireSeqMaxWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_t DCALL DeeType_RequireSeqMaxWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_and_key_t DCALL DeeType_RequireSeqMaxWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sum_t DCALL DeeType_RequireSeqSum(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sum_with_range_t DCALL DeeType_RequireSeqSumWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_t DCALL DeeType_RequireSeqCount(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_key_t DCALL DeeType_RequireSeqCountWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_t DCALL DeeType_RequireSeqCountWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_and_key_t DCALL DeeType_RequireSeqCountWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_t DCALL DeeType_RequireSeqContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_key_t DCALL DeeType_RequireSeqContainsWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_t DCALL DeeType_RequireSeqContainsWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_and_key_t DCALL DeeType_RequireSeqContainsWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_t DCALL DeeType_RequireSeqLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_with_range_t DCALL DeeType_RequireSeqLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rlocate_with_range_t DCALL DeeType_RequireSeqRLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_t DCALL DeeType_RequireSeqStartsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_key_t DCALL DeeType_RequireSeqStartsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_t DCALL DeeType_RequireSeqStartsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_and_key_t DCALL DeeType_RequireSeqStartsWithWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_t DCALL DeeType_RequireSeqEndsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_key_t DCALL DeeType_RequireSeqEndsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_t DCALL DeeType_RequireSeqEndsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_and_key_t DCALL DeeType_RequireSeqEndsWithWithRangeAndKey(DeeTypeObject *__restrict self);

/* Mutable sequence functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_t DCALL DeeType_RequireSeqFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_with_key_t DCALL DeeType_RequireSeqFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_t DCALL DeeType_RequireSeqRFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_with_key_t DCALL DeeType_RequireSeqRFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_erase_t DCALL DeeType_RequireSeqErase(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insert_t DCALL DeeType_RequireSeqInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insertall_t DCALL DeeType_RequireSeqInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pushfront_t DCALL DeeType_RequireSeqPushFront(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_append_t DCALL DeeType_RequireSeqAppend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_extend_t DCALL DeeType_RequireSeqExtend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_xchitem_index_t DCALL DeeType_RequireSeqXchItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_clear_t DCALL DeeType_RequireSeqClear(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pop_t DCALL DeeType_RequireSeqPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_t DCALL DeeType_RequireSeqRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_with_key_t DCALL DeeType_RequireSeqRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_t DCALL DeeType_RequireSeqRRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_with_key_t DCALL DeeType_RequireSeqRRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_t DCALL DeeType_RequireSeqRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_with_key_t DCALL DeeType_RequireSeqRemoveAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeif_t DCALL DeeType_RequireSeqRemoveIf(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_resize_t DCALL DeeType_RequireSeqResize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_fill_t DCALL DeeType_RequireSeqFill(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reverse_t DCALL DeeType_RequireSeqReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reversed_t DCALL DeeType_RequireSeqReversed(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_t DCALL DeeType_RequireSeqSort(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_with_key_t DCALL DeeType_RequireSeqSortWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_t DCALL DeeType_RequireSeqSorted(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_with_key_t DCALL DeeType_RequireSeqSortedWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_t DCALL DeeType_RequireSeqBFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_with_key_t DCALL DeeType_RequireSeqBFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_t DCALL DeeType_RequireSeqBPosition(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_with_key_t DCALL DeeType_RequireSeqBPositionWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_t DCALL DeeType_RequireSeqBRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_with_key_t DCALL DeeType_RequireSeqBRangeWithKey(DeeTypeObject *__restrict self);

/* Set functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insert_t DCALL DeeType_RequireSetInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_remove_t DCALL DeeType_RequireSetRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_unify_t DCALL DeeType_RequireSetUnify(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insertall_t DCALL DeeType_RequireSetInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_removeall_t DCALL DeeType_RequireSetRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_t DCALL DeeType_RequireSetPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_with_default_t DCALL DeeType_RequireSetPopWithDefault(DeeTypeObject *__restrict self);

/* Map functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_t DCALL DeeType_RequireMapSetOld(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_ex_t DCALL DeeType_RequireMapSetOldEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_t DCALL DeeType_RequireMapSetNew(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_ex_t DCALL DeeType_RequireMapSetNewEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setdefault_t DCALL DeeType_RequireMapSetDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_update_t DCALL DeeType_RequireMapUpdate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_remove_t DCALL DeeType_RequireMapRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_removekeys_t DCALL DeeType_RequireMapRemoveKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_t DCALL DeeType_RequireMapPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_with_default_t DCALL DeeType_RequireMapPopWithDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_popitem_t DCALL DeeType_RequireMapPopItem(DeeTypeObject *__restrict self);

#else /* CONFIG_BUILDING_DEEMON */
#define DeeType_TryRequireSeqForeachReverse(self)               ((Dee_mh_seq_foreach_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_foreach_reverse))
#define DeeType_TryRequireSeqEnumerateIndexReverse(self)        ((Dee_mh_seq_enumerate_index_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_enumerate_index_reverse))
#define DeeType_RequireSeqMakeEnumeration(self)                 ((Dee_mh_seq_makeenumeration_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration))
#define DeeType_RequireSeqMakeEnumerationWithIntRange(self)     ((Dee_mh_seq_makeenumeration_with_int_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_int_range))
#define DeeType_RequireSeqMakeEnumerationWithRange(self)        ((Dee_mh_seq_makeenumeration_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_range))
#define DeeType_RequireSeqOperatorBool(self)                    ((Dee_mh_seq_operator_bool_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bool))
#define DeeType_RequireSeqOperatorIter(self)                    ((Dee_mh_seq_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_iter))
#define DeeType_RequireSeqOperatorSizeOb(self)                  ((Dee_mh_seq_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_sizeob))
#define DeeType_RequireSeqOperatorContains(self)                ((Dee_mh_seq_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_contains))
#define DeeType_RequireSeqOperatorGetItem(self)                 ((Dee_mh_seq_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem))
#define DeeType_RequireSeqOperatorDelItem(self)                 ((Dee_mh_seq_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem))
#define DeeType_RequireSeqOperatorSetItem(self)                 ((Dee_mh_seq_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem))
#define DeeType_RequireSeqOperatorGetRange(self)                ((Dee_mh_seq_operator_getrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange))
#define DeeType_RequireSeqOperatorDelRange(self)                ((Dee_mh_seq_operator_delrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange))
#define DeeType_RequireSeqOperatorSetRange(self)                ((Dee_mh_seq_operator_setrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange))
#define DeeType_RequireSeqOperatorForeach(self)                 ((Dee_mh_seq_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach))
#define DeeType_RequireSeqOperatorForeachPair(self)             ((Dee_mh_seq_operator_foreach_pair_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach_pair))
#define DeeType_RequireSeqOperatorEnumerate(self)               ((Dee_mh_seq_operator_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_enumerate))
#define DeeType_RequireSeqOperatorEnumerateIndex(self)          ((Dee_mh_seq_operator_enumerate_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_enumerate_index))
#define DeeType_RequireSeqOperatorIterKeys(self)                ((Dee_mh_seq_operator_iterkeys_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_iterkeys))
#define DeeType_RequireSeqOperatorBoundItem(self)               ((Dee_mh_seq_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem))
#define DeeType_RequireSeqOperatorHasItem(self)                 ((Dee_mh_seq_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem))
#define DeeType_RequireSeqOperatorSize(self)                    ((Dee_mh_seq_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_size))
#define DeeType_RequireSeqOperatorSizeFast(self)                ((Dee_mh_seq_operator_size_fast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_size_fast))
#define DeeType_RequireSeqOperatorGetItemIndex(self)            ((Dee_mh_seq_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem_index))
#define DeeType_RequireSeqOperatorDelItemIndex(self)            ((Dee_mh_seq_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem_index))
#define DeeType_RequireSeqOperatorSetItemIndex(self)            ((Dee_mh_seq_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem_index))
#define DeeType_RequireSeqOperatorBoundItemIndex(self)          ((Dee_mh_seq_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem_index))
#define DeeType_RequireSeqOperatorHasItemIndex(self)            ((Dee_mh_seq_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem_index))
#define DeeType_RequireSeqOperatorGetRangeIndex(self)           ((Dee_mh_seq_operator_getrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index))
#define DeeType_RequireSeqOperatorDelRangeIndex(self)           ((Dee_mh_seq_operator_delrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index))
#define DeeType_RequireSeqOperatorSetRangeIndex(self)           ((Dee_mh_seq_operator_setrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index))
#define DeeType_RequireSeqOperatorGetRangeIndexN(self)          ((Dee_mh_seq_operator_getrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index_n))
#define DeeType_RequireSeqOperatorDelRangeIndexN(self)          ((Dee_mh_seq_operator_delrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index_n))
#define DeeType_RequireSeqOperatorSetRangeIndexN(self)          ((Dee_mh_seq_operator_setrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index_n))
#define DeeType_RequireSeqOperatorTryGetItem(self)              ((Dee_mh_seq_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem))
#define DeeType_RequireSeqOperatorTryGetItemIndex(self)         ((Dee_mh_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem_index))
#define DeeType_RequireSeqOperatorHash(self)                    ((Dee_mh_seq_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hash))
#define DeeType_RequireSeqOperatorCompareEq(self)               ((Dee_mh_seq_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare_eq))
#define DeeType_RequireSeqOperatorCompare(self)                 ((Dee_mh_seq_operator_compare_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare))
#define DeeType_RequireSeqOperatorTryCompareEq(self)            ((Dee_mh_seq_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trycompare_eq))
#define DeeType_RequireSeqOperatorEq(self)                      ((Dee_mh_seq_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_eq))
#define DeeType_RequireSeqOperatorNe(self)                      ((Dee_mh_seq_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ne))
#define DeeType_RequireSeqOperatorLo(self)                      ((Dee_mh_seq_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_lo))
#define DeeType_RequireSeqOperatorLe(self)                      ((Dee_mh_seq_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_le))
#define DeeType_RequireSeqOperatorGr(self)                      ((Dee_mh_seq_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_gr))
#define DeeType_RequireSeqOperatorGe(self)                      ((Dee_mh_seq_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ge))
#define DeeType_RequireSeqOperatorInplaceAdd(self)              ((Dee_mh_seq_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_add))
#define DeeType_RequireSeqOperatorInplaceMul(self)              ((Dee_mh_seq_operator_inplace_mul_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_mul))
#define DeeType_RequireSetOperatorIter(self)                    ((Dee_mh_set_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_iter))
#define DeeType_RequireSetOperatorForeach(self)                 ((Dee_mh_set_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_foreach))
#define DeeType_RequireSetOperatorSize(self)                    ((Dee_mh_set_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_size))
#define DeeType_RequireSetOperatorSizeOb(self)                  ((Dee_mh_set_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sizeob))
#define DeeType_RequireSetOperatorHash(self)                    ((Dee_mh_set_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_hash))
#define DeeType_RequireSetOperatorCompareEq(self)               ((Dee_mh_set_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_compare_eq))
#define DeeType_RequireSetOperatorTryCompareEq(self)            ((Dee_mh_set_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_trycompare_eq))
#define DeeType_RequireSetOperatorEq(self)                      ((Dee_mh_set_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_eq))
#define DeeType_RequireSetOperatorNe(self)                      ((Dee_mh_set_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ne))
#define DeeType_RequireSetOperatorLo(self)                      ((Dee_mh_set_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_lo))
#define DeeType_RequireSetOperatorLe(self)                      ((Dee_mh_set_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_le))
#define DeeType_RequireSetOperatorGr(self)                      ((Dee_mh_set_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_gr))
#define DeeType_RequireSetOperatorGe(self)                      ((Dee_mh_set_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ge))
#define DeeType_RequireSetOperatorInv(self)                     ((Dee_mh_set_operator_inv_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inv))
#define DeeType_RequireSetOperatorAdd(self)                     ((Dee_mh_set_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_add))
#define DeeType_RequireSetOperatorSub(self)                     ((Dee_mh_set_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sub))
#define DeeType_RequireSetOperatorAnd(self)                     ((Dee_mh_set_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_and))
#define DeeType_RequireSetOperatorXor(self)                     ((Dee_mh_set_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_xor))
#define DeeType_RequireSetOperatorInplaceAdd(self)              ((Dee_mh_set_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_add))
#define DeeType_RequireSetOperatorInplaceSub(self)              ((Dee_mh_set_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_sub))
#define DeeType_RequireSetOperatorInplaceAnd(self)              ((Dee_mh_set_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_and))
#define DeeType_RequireSetOperatorInplaceXor(self)              ((Dee_mh_set_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_xor))
#define DeeType_RequireMapOperatorContains(self)                ((Dee_mh_map_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_contains))
#define DeeType_RequireMapOperatorGetItem(self)                 ((Dee_mh_map_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem))
#define DeeType_RequireMapOperatorDelItem(self)                 ((Dee_mh_map_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem))
#define DeeType_RequireMapOperatorSetItem(self)                 ((Dee_mh_map_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem))
#define DeeType_RequireMapOperatorEnumerate(self)               ((Dee_mh_map_operator_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_enumerate))
#define DeeType_RequireMapOperatorEnumerateIndex(self)          ((Dee_mh_map_operator_enumerate_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_enumerate_index))
#define DeeType_RequireMapOperatorBoundItem(self)               ((Dee_mh_map_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem))
#define DeeType_RequireMapOperatorHasItem(self)                 ((Dee_mh_map_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem))
#define DeeType_RequireMapOperatorGetItemIndex(self)            ((Dee_mh_map_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_index))
#define DeeType_RequireMapOperatorDelItemIndex(self)            ((Dee_mh_map_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_index))
#define DeeType_RequireMapOperatorSetItemIndex(self)            ((Dee_mh_map_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_index))
#define DeeType_RequireMapOperatorBoundItemIndex(self)          ((Dee_mh_map_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_index))
#define DeeType_RequireMapOperatorHasItemIndex(self)            ((Dee_mh_map_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_index))
#define DeeType_RequireMapOperatorTryGetItem(self)              ((Dee_mh_map_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem))
#define DeeType_RequireMapOperatorTryGetItemIndex(self)         ((Dee_mh_map_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_index))
#define DeeType_RequireMapOperatorTryGetItemStringHash(self)    ((Dee_mh_map_operator_trygetitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_hash))
#define DeeType_RequireMapOperatorGetItemStringHash(self)       ((Dee_mh_map_operator_getitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_hash))
#define DeeType_RequireMapOperatorDelItemStringHash(self)       ((Dee_mh_map_operator_delitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_hash))
#define DeeType_RequireMapOperatorSetItemStringHash(self)       ((Dee_mh_map_operator_setitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_hash))
#define DeeType_RequireMapOperatorBoundItemStringHash(self)     ((Dee_mh_map_operator_bounditem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_hash))
#define DeeType_RequireMapOperatorHasItemStringHash(self)       ((Dee_mh_map_operator_hasitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_hash))
#define DeeType_RequireMapOperatorTryGetItemStringLenHash(self) ((Dee_mh_map_operator_trygetitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_len_hash))
#define DeeType_RequireMapOperatorGetItemStringLenHash(self)    ((Dee_mh_map_operator_getitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_len_hash))
#define DeeType_RequireMapOperatorDelItemStringLenHash(self)    ((Dee_mh_map_operator_delitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_len_hash))
#define DeeType_RequireMapOperatorSetItemStringLenHash(self)    ((Dee_mh_map_operator_setitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_len_hash))
#define DeeType_RequireMapOperatorBoundItemStringLenHash(self)  ((Dee_mh_map_operator_bounditem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_len_hash))
#define DeeType_RequireMapOperatorHasItemStringLenHash(self)    ((Dee_mh_map_operator_hasitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_len_hash))
#define DeeType_RequireMapOperatorCompareEq(self)               ((Dee_mh_map_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_compare_eq))
#define DeeType_RequireMapOperatorTryCompareEq(self)            ((Dee_mh_map_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trycompare_eq))
#define DeeType_RequireMapOperatorEq(self)                      ((Dee_mh_map_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_eq))
#define DeeType_RequireMapOperatorNe(self)                      ((Dee_mh_map_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ne))
#define DeeType_RequireMapOperatorLo(self)                      ((Dee_mh_map_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_lo))
#define DeeType_RequireMapOperatorLe(self)                      ((Dee_mh_map_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_le))
#define DeeType_RequireMapOperatorGr(self)                      ((Dee_mh_map_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_gr))
#define DeeType_RequireMapOperatorGe(self)                      ((Dee_mh_map_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ge))
#define DeeType_RequireMapOperatorAdd(self)                     ((Dee_mh_map_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_add))
#define DeeType_RequireMapOperatorSub(self)                     ((Dee_mh_map_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_sub))
#define DeeType_RequireMapOperatorAnd(self)                     ((Dee_mh_map_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_and))
#define DeeType_RequireMapOperatorXor(self)                     ((Dee_mh_map_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_xor))
#define DeeType_RequireMapOperatorInplaceAdd(self)              ((Dee_mh_map_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_add))
#define DeeType_RequireMapOperatorInplaceSub(self)              ((Dee_mh_map_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_sub))
#define DeeType_RequireMapOperatorInplaceAnd(self)              ((Dee_mh_map_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_and))
#define DeeType_RequireMapOperatorInplaceXor(self)              ((Dee_mh_map_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_xor))
#define DeeType_RequireSeqTryGetFirst(self)                     ((Dee_mh_seq_trygetfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetfirst))
#define DeeType_RequireSeqTryGetLast(self)                      ((Dee_mh_seq_trygetlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetlast))
#define DeeType_RequireSeqAny(self)                             ((Dee_mh_seq_any_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any))
#define DeeType_RequireSeqAnyWithKey(self)                      ((Dee_mh_seq_any_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_key))
#define DeeType_RequireSeqAnyWithRange(self)                    ((Dee_mh_seq_any_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range))
#define DeeType_RequireSeqAnyWithRangeAndKey(self)              ((Dee_mh_seq_any_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range_and_key))
#define DeeType_RequireSeqAll(self)                             ((Dee_mh_seq_all_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all))
#define DeeType_RequireSeqAllWithKey(self)                      ((Dee_mh_seq_all_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_key))
#define DeeType_RequireSeqAllWithRange(self)                    ((Dee_mh_seq_all_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range))
#define DeeType_RequireSeqAllWithRangeAndKey(self)              ((Dee_mh_seq_all_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range_and_key))
#define DeeType_RequireSeqParity(self)                          ((Dee_mh_seq_parity_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity))
#define DeeType_RequireSeqParityWithKey(self)                   ((Dee_mh_seq_parity_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_key))
#define DeeType_RequireSeqParityWithRange(self)                 ((Dee_mh_seq_parity_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range))
#define DeeType_RequireSeqParityWithRangeAndKey(self)           ((Dee_mh_seq_parity_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range_and_key))
#define DeeType_RequireSeqReduce(self)                          ((Dee_mh_seq_reduce_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce))
#define DeeType_RequireSeqReduceWithInit(self)                  ((Dee_mh_seq_reduce_with_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_init))
#define DeeType_RequireSeqReduceWithRange(self)                 ((Dee_mh_seq_reduce_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range))
#define DeeType_RequireSeqReduceWithRangeAndInit(self)          ((Dee_mh_seq_reduce_with_range_and_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range_and_init))
#define DeeType_RequireSeqMin(self)                             ((Dee_mh_seq_min_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min))
#define DeeType_RequireSeqMinWithKey(self)                      ((Dee_mh_seq_min_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_key))
#define DeeType_RequireSeqMinWithRange(self)                    ((Dee_mh_seq_min_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range))
#define DeeType_RequireSeqMinWithRangeAndKey(self)              ((Dee_mh_seq_min_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range_and_key))
#define DeeType_RequireSeqMax(self)                             ((Dee_mh_seq_max_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max))
#define DeeType_RequireSeqMaxWithKey(self)                      ((Dee_mh_seq_max_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_key))
#define DeeType_RequireSeqMaxWithRange(self)                    ((Dee_mh_seq_max_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range))
#define DeeType_RequireSeqMaxWithRangeAndKey(self)              ((Dee_mh_seq_max_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range_and_key))
#define DeeType_RequireSeqSum(self)                             ((Dee_mh_seq_sum_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum))
#define DeeType_RequireSeqSumWithRange(self)                    ((Dee_mh_seq_sum_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum_with_range))
#define DeeType_RequireSeqCount(self)                           ((Dee_mh_seq_count_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count))
#define DeeType_RequireSeqCountWithKey(self)                    ((Dee_mh_seq_count_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_key))
#define DeeType_RequireSeqCountWithRange(self)                  ((Dee_mh_seq_count_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range))
#define DeeType_RequireSeqCountWithRangeAndKey(self)            ((Dee_mh_seq_count_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range_and_key))
#define DeeType_RequireSeqContains(self)                        ((Dee_mh_seq_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains))
#define DeeType_RequireSeqContainsWithKey(self)                 ((Dee_mh_seq_contains_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_key))
#define DeeType_RequireSeqContainsWithRange(self)               ((Dee_mh_seq_contains_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range))
#define DeeType_RequireSeqContainsWithRangeAndKey(self)         ((Dee_mh_seq_contains_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range_and_key))
#define DeeType_RequireSeqLocate(self)                          ((Dee_mh_seq_locate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate))
#define DeeType_RequireSeqLocateWithRange(self)                 ((Dee_mh_seq_locate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate_with_range))
#define DeeType_RequireSeqRLocateWithRange(self)                ((Dee_mh_seq_rlocate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rlocate_with_range))
#define DeeType_RequireSeqStartsWith(self)                      ((Dee_mh_seq_startswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith))
#define DeeType_RequireSeqStartsWithWithKey(self)               ((Dee_mh_seq_startswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_key))
#define DeeType_RequireSeqStartsWithWithRange(self)             ((Dee_mh_seq_startswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range))
#define DeeType_RequireSeqStartsWithWithRangeAndKey(self)       ((Dee_mh_seq_startswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range_and_key))
#define DeeType_RequireSeqEndsWith(self)                        ((Dee_mh_seq_endswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith))
#define DeeType_RequireSeqEndsWithWithKey(self)                 ((Dee_mh_seq_endswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_key))
#define DeeType_RequireSeqEndsWithWithRange(self)               ((Dee_mh_seq_endswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range))
#define DeeType_RequireSeqEndsWithWithRangeAndKey(self)         ((Dee_mh_seq_endswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range_and_key))
#define DeeType_RequireSeqFind(self)                            ((Dee_mh_seq_find_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find))
#define DeeType_RequireSeqFindWithKey(self)                     ((Dee_mh_seq_find_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find_with_key))
#define DeeType_RequireSeqRFind(self)                           ((Dee_mh_seq_rfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind))
#define DeeType_RequireSeqRFindWithKey(self)                    ((Dee_mh_seq_rfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind_with_key))
#define DeeType_RequireSeqErase(self)                           ((Dee_mh_seq_erase_t)DeeType_GetMethodHint(self, Dee_TMH_seq_erase))
#define DeeType_RequireSeqInsert(self)                          ((Dee_mh_seq_insert_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insert))
#define DeeType_RequireSeqInsertAll(self)                       ((Dee_mh_seq_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insertall))
#define DeeType_RequireSeqPushFront(self)                       ((Dee_mh_seq_pushfront_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pushfront))
#define DeeType_RequireSeqAppend(self)                          ((Dee_mh_seq_append_t)DeeType_GetMethodHint(self, Dee_TMH_seq_append))
#define DeeType_RequireSeqExtend(self)                          ((Dee_mh_seq_extend_t)DeeType_GetMethodHint(self, Dee_TMH_seq_extend))
#define DeeType_RequireSeqXchItemIndex(self)                    ((Dee_mh_seq_xchitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_xchitem_index))
#define DeeType_RequireSeqClear(self)                           ((Dee_mh_seq_clear_t)DeeType_GetMethodHint(self, Dee_TMH_seq_clear))
#define DeeType_RequireSeqPop(self)                             ((Dee_mh_seq_pop_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pop))
#define DeeType_RequireSeqRemove(self)                          ((Dee_mh_seq_remove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove))
#define DeeType_RequireSeqRemoveWithKey(self)                   ((Dee_mh_seq_remove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove_with_key))
#define DeeType_RequireSeqRRemove(self)                         ((Dee_mh_seq_rremove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove))
#define DeeType_RequireSeqRRemoveWithKey(self)                  ((Dee_mh_seq_rremove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove_with_key))
#define DeeType_RequireSeqRemoveAll(self)                       ((Dee_mh_seq_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall))
#define DeeType_RequireSeqRemoveAllWithKey(self)                ((Dee_mh_seq_removeall_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall_with_key))
#define DeeType_RequireSeqRemoveIf(self)                        ((Dee_mh_seq_removeif_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeif))
#define DeeType_RequireSeqResize(self)                          ((Dee_mh_seq_resize_t)DeeType_GetMethodHint(self, Dee_TMH_seq_resize))
#define DeeType_RequireSeqFill(self)                            ((Dee_mh_seq_fill_t)DeeType_GetMethodHint(self, Dee_TMH_seq_fill))
#define DeeType_RequireSeqReverse(self)                         ((Dee_mh_seq_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reverse))
#define DeeType_RequireSeqReversed(self)                        ((Dee_mh_seq_reversed_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reversed))
#define DeeType_RequireSeqSort(self)                            ((Dee_mh_seq_sort_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort))
#define DeeType_RequireSeqSortWithKey(self)                     ((Dee_mh_seq_sort_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort_with_key))
#define DeeType_RequireSeqSorted(self)                          ((Dee_mh_seq_sorted_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted))
#define DeeType_RequireSeqSortedWithKey(self)                   ((Dee_mh_seq_sorted_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted_with_key))
#define DeeType_RequireSeqBFind(self)                           ((Dee_mh_seq_bfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind))
#define DeeType_RequireSeqBFindWithKey(self)                    ((Dee_mh_seq_bfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind_with_key))
#define DeeType_RequireSeqBPosition(self)                       ((Dee_mh_seq_bposition_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition))
#define DeeType_RequireSeqBPositionWithKey(self)                ((Dee_mh_seq_bposition_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition_with_key))
#define DeeType_RequireSeqBRange(self)                          ((Dee_mh_seq_brange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange))
#define DeeType_RequireSeqBRangeWithKey(self)                   ((Dee_mh_seq_brange_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange_with_key))
#define DeeType_RequireSetInsert(self)                          ((Dee_mh_set_insert_t)DeeType_GetMethodHint(self, Dee_TMH_set_insert))
#define DeeType_RequireSetRemove(self)                          ((Dee_mh_set_remove_t)DeeType_GetMethodHint(self, Dee_TMH_set_remove))
#define DeeType_RequireSetUnify(self)                           ((Dee_mh_set_unify_t)DeeType_GetMethodHint(self, Dee_TMH_set_unify))
#define DeeType_RequireSetInsertAll(self)                       ((Dee_mh_set_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_set_insertall))
#define DeeType_RequireSetRemoveAll(self)                       ((Dee_mh_set_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_set_removeall))
#define DeeType_RequireSetPop(self)                             ((Dee_mh_set_pop_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop))
#define DeeType_RequireSetPopWithDefault(self)                  ((Dee_mh_set_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop_with_default))
#define DeeType_RequireMapSetOld(self)                          ((Dee_mh_map_setold_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold))
#define DeeType_RequireMapSetOldEx(self)                        ((Dee_mh_map_setold_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold_ex))
#define DeeType_RequireMapSetNew(self)                          ((Dee_mh_map_setnew_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew))
#define DeeType_RequireMapSetNewEx(self)                        ((Dee_mh_map_setnew_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew_ex))
#define DeeType_RequireMapSetDefault(self)                      ((Dee_mh_map_setdefault_t)DeeType_GetMethodHint(self, Dee_TMH_map_setdefault))
#define DeeType_RequireMapUpdate(self)                          ((Dee_mh_map_update_t)DeeType_GetMethodHint(self, Dee_TMH_map_update))
#define DeeType_RequireMapRemove(self)                          ((Dee_mh_map_remove_t)DeeType_GetMethodHint(self, Dee_TMH_map_remove))
#define DeeType_RequireMapRemoveKeys(self)                      ((Dee_mh_map_removekeys_t)DeeType_GetMethodHint(self, Dee_TMH_map_removekeys))
#define DeeType_RequireMapPop(self)                             ((Dee_mh_map_pop_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop))
#define DeeType_RequireMapPopWithDefault(self)                  ((Dee_mh_map_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop_with_default))
#define DeeType_RequireMapPopItem(self)                         ((Dee_mh_map_popitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_popitem))
#endif /* !CONFIG_BUILDING_DEEMON */

#define DeeType_RequireMapOperatorHash DeeType_RequireSetOperatorHash

/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id);

/* Same as `DeeType_GetPrivateMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions. */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id);
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */






/* Used to declare type method hints in C */
struct Dee_type_method_hint {
	enum Dee_tmh_id tmh_id;    /* Method hint ID (one of `Dee_TMH_*') */
	uint32_t        tmh_flags; /* Method flags (set of `Dee_METHOD_F*') */
	Dee_funptr_t    tmh_func;  /* [1..1] Method hint implementation (custom/type-specific) (NULL marks end-of-list) */
};

#ifdef __INTELLISENSE__
/* c++ magic to assert that the function pointers passed to `Dee_TYPE_METHOD_HINT_F'
 * and `Dee_TYPE_METHOD_HINT' are binary-compatible with whatever the resp. method
 * hint expects for its prototype (but note that pointer bases can be exchanged for
 * arbitrary types, meaning this doesn't fail if you use the real object types in
 * function parameters). */
extern "C++" {namespace __intern {
template<class T1, class T2> struct __PRIVATE_binary_compatible { enum{_value=false}; };
template<class T1, class T2> struct __PRIVATE_binary_compatible<T1 *, T2 *> { enum{_value=true}; };
template<class T> struct __PRIVATE_binary_compatible<T, T> { enum{_value=true}; };
template<> struct __PRIVATE_binary_compatible<void(), void()> { enum{_value=true}; };
template<class A1, class A2> struct __PRIVATE_binary_compatible<void(A1), void(A2)>: __PRIVATE_binary_compatible<A1, A2> { };
template<class A1, class... TARGS1, class A2, class... TARGS2>
struct __PRIVATE_binary_compatible<void(A1, TARGS1...), void(A2, TARGS2...)> {
	enum{_value = __PRIVATE_binary_compatible<A1, A2>::_value &&
	              __PRIVATE_binary_compatible<void(TARGS1...), void(TARGS2...)>::_value};
};
template<class T> struct __PRIVATE_match_method_hint { static Dee_funptr_t _match(T); };
template<class RT1, class... TARGS1> struct __PRIVATE_match_method_hint<RT1(DCALL *)(TARGS1...)> {
	template<class RT2, class... TARGS2>
	static ::__intern::____INTELLISENSE_enableif<
	::__intern::__PRIVATE_binary_compatible<RT1, RT2>::_value &&
	::__intern::__PRIVATE_binary_compatible<void(TARGS1...), void(TARGS2...)>::_value,
	Dee_funptr_t>::__type _match(RT2(DCALL *)(TARGS2...));
};
}}

#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#define Dee_TYPE_METHOD_HINT_F(func_name, func, flags) \
	{ Dee_TMH_##func_name, flags, ::__intern::__PRIVATE_match_method_hint<DeeMH_##func_name##_t>::_match(func) }
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
#define Dee_TYPE_METHOD_HINT_F(func_name, func, flags) \
	{ Dee_TMH_##func_name, flags, ::__intern::__PRIVATE_match_method_hint<Dee_mh_##func_name##_t>::_match(func) }
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
#else /* __INTELLISENSE__ */
#define Dee_TYPE_METHOD_HINT_F(func_name, func, flags) \
	{ Dee_TMH_##func_name, flags, (Dee_funptr_t)(func) }
#endif /* !__INTELLISENSE__ */
#define Dee_TYPE_METHOD_HINT(func_name, func) Dee_TYPE_METHOD_HINT_F(func_name, func, 0)
#define Dee_TYPE_METHOD_HINT_END { (enum Dee_tmh_id)0, 0, NULL }


/* Link a type method in as part of a type's `tp_method_hints' array.
 * Behavior is undefined/depends-on-the-method-in-question if a type
 * defines a method as a hint reference, but fails to implement all
 * method hints used by the hinted method attribute. */
#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#define Dee_TYPE_METHOD_HINTREF(attr_name) \
	{ DeeMA_##attr_name##_name,            \
	  (Dee_objmethod_t)&DeeMA_##attr_name, \
	  DeeMA_##attr_name##_doc,             \
	  DeeMA_##attr_name##_flags }
#define Dee_TYPE_METHOD_HINTREF_DOC(attr_name, doc) \
	{ DeeMA_##attr_name##_name,                     \
	  (Dee_objmethod_t)&DeeMA_##attr_name, doc,     \
	  DeeMA_##attr_name##_flags }
#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */
#define Dee_TYPE_METHOD_HINTREF(attr_name) \
	{ DeeMH_##attr_name##_name,            \
	  (Dee_objmethod_t)&DeeMH_##attr_name, \
	  DeeMH_##attr_name##_doc,             \
	  _Dee_TMH_WRAPPER_FLAGS_##attr_name }
#define Dee_TYPE_METHOD_HINTREF_DOC(attr_name, doc) \
	{ DeeMH_##attr_name##_name,                     \
	  (Dee_objmethod_t)&DeeMH_##attr_name, doc,     \
	  _Dee_TMH_WRAPPER_FLAGS_##attr_name }
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


#ifdef DEE_SOURCE
#define TYPE_METHOD_HINT        Dee_TYPE_METHOD_HINT
#define TYPE_METHOD_HINT_F      Dee_TYPE_METHOD_HINT_F
#define TYPE_METHOD_HINT_END    Dee_TYPE_METHOD_HINT_END
#define TYPE_METHOD_HINTREF     Dee_TYPE_METHOD_HINTREF
#define TYPE_METHOD_HINTREF_DOC Dee_TYPE_METHOD_HINTREF_DOC
#endif /* DEE_SOURCE */


DECL_END

#endif /* !GUARD_DEEMON_METHOD_HINTS_H */
