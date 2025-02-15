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
#ifndef GUARD_DEEMON_OPERATOR_HINTS_H
#define GUARD_DEEMON_OPERATOR_HINTS_H 1

#include "api.h"
#include "object.h"

#ifdef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#include "none-operator.h"
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

DECL_BEGIN

/* Equivalence callbacks for native operators.
 *
 * These equivalences are not related to method hints,
 * and are applicable to *any* type of object, but only
 * within an object itself. */

/* clang-format off */
/*[[[deemon (printNativeOperatorTypes from "...src.deemon.method-hints.method-hints")();]]]*/
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_assign_t)(DeeObject *self, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_move_assign_t)(DeeObject *self, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_str_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeNO_print_t)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_repr_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeNO_printrepr_t)(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_bool_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_call_t)(DeeObject *self, size_t argc, DeeObject *const *argv);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_call_kw_t)(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_iter_next_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_nextpair_t)(DeeObject *__restrict self, DREF DeeObject *key_and_value[2]);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_nextkey_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_nextvalue_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeNO_advance_t)(DeeObject *__restrict self, size_t step);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_int_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_int32_t)(DeeObject *__restrict self, int32_t *__restrict p_result);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_int64_t)(DeeObject *__restrict self, int64_t *__restrict p_result);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_double_t)(DeeObject *__restrict self, double *__restrict p_result);
typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *DeeNO_hash_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_compare_eq_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_compare_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_trycompare_eq_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_eq_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_ne_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_lo_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_le_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_gr_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_ge_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_iter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeNO_foreach_t)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *DeeNO_foreach_pair_t)(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_sizeob_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeNO_size_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *DeeNO_size_fast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_contains_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_trygetitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getitem_index_fast_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_trygetitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_trygetitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_trygetitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_bounditem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_bounditem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_bounditem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_bounditem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_hasitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_delitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeNO_setitem_t)(DeeObject *self, DeeObject *index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeNO_setitem_index_t)(DeeObject *self, size_t index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 4)) int (DCALL *DeeNO_setitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeNO_setitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *DeeNO_getrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeNO_delrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_delrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_delrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1, 2, 3, 4)) int (DCALL *DeeNO_setrange_t)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *DeeNO_setrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *DeeNO_setrange_index_n_t)(DeeObject *self, Dee_ssize_t start, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_inv_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_pos_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_neg_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_add_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_add_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_sub_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_sub_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_mul_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_mul_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_div_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_div_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_mod_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_mod_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_shl_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_shl_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_shr_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_shr_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_and_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_and_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_or_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_or_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_xor_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_xor_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_pow_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_pow_t)(DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_inc_t)(DeeObject **__restrict p_self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_dec_t)(DeeObject **__restrict p_self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_enter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_leave_t)(DeeObject *__restrict self);
/*[[[end]]]*/
/* clang-format on */

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#if defined(CONFIG_BUILDING_DEEMON) || defined(__DEEMON__)

/* Dee_TypeNativeOperator_ID */
enum Dee_tno_id {
	/* clang-format off */
/*[[[deemon (printNativeOperatorIds from "...src.deemon.method-hints.method-hints")();]]]*/
	Dee_TNO_assign,
	Dee_TNO_move_assign,
	Dee_TNO_str,
	Dee_TNO_print,
	Dee_TNO_repr,
	Dee_TNO_printrepr,
	Dee_TNO_bool,
	Dee_TNO_call,
	Dee_TNO_call_kw,
	Dee_TNO_iter_next,
	Dee_TNO_nextpair,
	Dee_TNO_nextkey,
	Dee_TNO_nextvalue,
	Dee_TNO_advance,
	Dee_TNO_int,
	Dee_TNO_int32,
	Dee_TNO_int64,
	Dee_TNO_double,
	Dee_TNO_hash,
	Dee_TNO_compare_eq,
	Dee_TNO_compare,
	Dee_TNO_trycompare_eq,
	Dee_TNO_eq,
	Dee_TNO_ne,
	Dee_TNO_lo,
	Dee_TNO_le,
	Dee_TNO_gr,
	Dee_TNO_ge,
	Dee_TNO_iter,
	Dee_TNO_foreach,
	Dee_TNO_foreach_pair,
	Dee_TNO_sizeob,
	Dee_TNO_size,
	Dee_TNO_size_fast,
	Dee_TNO_contains,
	Dee_TNO_getitem,
	Dee_TNO_trygetitem,
	Dee_TNO_getitem_index_fast,
	Dee_TNO_getitem_index,
	Dee_TNO_trygetitem_index,
	Dee_TNO_getitem_string_hash,
	Dee_TNO_trygetitem_string_hash,
	Dee_TNO_getitem_string_len_hash,
	Dee_TNO_trygetitem_string_len_hash,
	Dee_TNO_bounditem,
	Dee_TNO_bounditem_index,
	Dee_TNO_bounditem_string_hash,
	Dee_TNO_bounditem_string_len_hash,
	Dee_TNO_hasitem,
	Dee_TNO_hasitem_index,
	Dee_TNO_hasitem_string_hash,
	Dee_TNO_hasitem_string_len_hash,
	Dee_TNO_delitem,
	Dee_TNO_delitem_index,
	Dee_TNO_delitem_string_hash,
	Dee_TNO_delitem_string_len_hash,
	Dee_TNO_setitem,
	Dee_TNO_setitem_index,
	Dee_TNO_setitem_string_hash,
	Dee_TNO_setitem_string_len_hash,
	Dee_TNO_getrange,
	Dee_TNO_getrange_index,
	Dee_TNO_getrange_index_n,
	Dee_TNO_delrange,
	Dee_TNO_delrange_index,
	Dee_TNO_delrange_index_n,
	Dee_TNO_setrange,
	Dee_TNO_setrange_index,
	Dee_TNO_setrange_index_n,
	Dee_TNO_inv,
	Dee_TNO_pos,
	Dee_TNO_neg,
	Dee_TNO_add,
	Dee_TNO_inplace_add,
	Dee_TNO_sub,
	Dee_TNO_inplace_sub,
	Dee_TNO_mul,
	Dee_TNO_inplace_mul,
	Dee_TNO_div,
	Dee_TNO_inplace_div,
	Dee_TNO_mod,
	Dee_TNO_inplace_mod,
	Dee_TNO_shl,
	Dee_TNO_inplace_shl,
	Dee_TNO_shr,
	Dee_TNO_inplace_shr,
	Dee_TNO_and,
	Dee_TNO_inplace_and,
	Dee_TNO_or,
	Dee_TNO_inplace_or,
	Dee_TNO_xor,
	Dee_TNO_inplace_xor,
	Dee_TNO_pow,
	Dee_TNO_inplace_pow,
	Dee_TNO_inc,
	Dee_TNO_dec,
	Dee_TNO_enter,
	Dee_TNO_leave,
/*[[[end]]]*/
	/* clang-format on */
	Dee_TNO_COUNT
};

struct Dee_tno_assign {
	enum Dee_tno_id tnoa_id; /* Operator slot ID to write to */
	Dee_funptr_t    tnoa_cb; /* [1..1] Function pointer to write to `tnoa_id' */
};

/* The max # of operator slots that might ever need to be assigned at once.
 * iow: this is the length of the longest non-looping dependency chain that
 *      can be formed using `default__*__with__*' callbacks below.
 * Example:
 *  - default__hasitem__with__bounditem
 *  - default__bounditem__with__getitem
 *  - default__getitem__with__getitem_index
 * Note that the following doesn't count because is can be shortened:
 *  - default__hasitem_index__with__hasitem
 *  - default__hasitem__with__bounditem
 *  - default__bounditem__with__getitem
 *  - default__getitem__with__getitem_index
 * Shorter version is:
 *  - default__hasitem_index__with__bounditem_index
 *  - default__bounditem_index__with__getitem_index */
#define Dee_TNO_ASSIGN_MAXLEN 3 /* TODO: Dynamically calculate */


/* Looking at related operators that actually *are* present,
 * and assuming that `id' isn't implemented, return the most
 * applicable default implementation for the operator.
 *
 * e.g. Given a type that defines `tp_iter' and `id=Dee_TNO_foreach',
 *      this function would return `&default__foreach__with__iter'
 *
 * When no related operators are present (or `id' doesn't
 * have *any* related operators), return `NULL' instead.
 *
 * NOTE: This function does *!!NOT!!* return method hint pointers
 * @param actions: Actions that need to be performed (multiple assignments
 *                 might be necessary in case of a transitive dependency)
 *                 Stored actions must be performed in *REVERSE* order
 *                 The first (last-written) action is always for `id'
 * @return: The number of actions written to `actions' */
INTDEF WUNUSED NONNULL((1)) size_t
(DCALL DeeType_SelectMissingNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id,
                                            struct Dee_tno_assign actions[Dee_TNO_ASSIGN_MAXLEN]);

/* Wrapper around `DeeType_SelectMissingNativeOperator' that checks if the
 * operator is already defined, and if not: see if can be substituted via
 * some other set of native operators (in which case: do that substitution
 * and then return the operator's function pointer) */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutHints)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Ignoring method hints that might have been able to implement "id" along
 * the way, and assuming that `DeeType_GetNativeOperatorWithoutHints(from, id)'
 * returned `impl', make sure that `impl' can be (and is) inherited by `into'.
 *
 * This function is allowed to assume that "impl" really is what should be
 * inherited for the specified "id" (if it is not correct, everything breaks)
 *
 * @return: Indicative of a successful inherit (inherit may fail when "impl"
 *          is `default__*__with__*', and dependencies could not be written
 *          due to OOM, though in this case, no error is thrown) */
INTDEF WUNUSED NONNULL((1, 2, 4)) bool
(DCALL DeeType_InheritOperatorWithoutHints)(DeeTypeObject *__restrict from,
                                            DeeTypeObject *__restrict into,
                                            enum Dee_tno_id id, Dee_funptr_t impl);

/* Same as `DeeType_GetNativeOperatorWithoutHints', but also load operators
 * from method hints (though don't inherit them from base-types, yet). */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutInherit)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Same as `DeeType_GetNativeOperatorWithoutInherit', but actually also does
 * the operator inherit part (meaning that this is the actual master-function
 * that's called when you invoke one of the standard operators whose callback
 * is currently set to "NULL" within its relevant type) */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutUnsupported)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Returns a special impl for "id" returned by:
 * - DeeType_GetNativeOperatorWithoutHints()
 * - DeeType_GetNativeOperatorWithoutInherit()
 * - DeeType_GetNativeOperatorWithoutUnsupported()
 * when it failed to allocate a necessary operator table. These impls behave
 * similar to `DeeType_GetNativeOperatorUnsupported()', except that rather
 * than calling `err_unimplemented_operator()', these call:
 * >> Dee_BadAlloc(type_tno_sizeof_table(oh_init_specs[id].ohis_table));
 * where "id" is the same as the given "id"
 *
 * Returns "NULL" if "id" isn't part of a sub-table. */
INTDEF Dee_funptr_t tpconst _DeeType_GetNativeOperatorOOM[Dee_TNO_COUNT];
#define DeeType_GetNativeOperatorOOM(id) _DeeType_GetNativeOperatorOOM[id]

/* Same as `DeeType_GetNativeOperatorWithoutUnsupported()', but never returns NULL
 * (for any operator linked against a deemon user-code ID (e.g. "OPERATOR_ITER")
 * and instead returns special implementations for each operator that simply call
 * `err_unimplemented_operator()' with the relevant arguments, before returning
 * whatever is indicative of an error in the context of the native operator. */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Returns the impl for "id" that calls `err_unimplemented_operator()'.
 * Returns "NULL" if "id" doesn't define a user-code ID */
INTDEF Dee_funptr_t tpconst _DeeType_GetNativeOperatorUnsupported[Dee_TNO_COUNT];
#define DeeType_GetNativeOperatorUnsupported(id) _DeeType_GetNativeOperatorUnsupported[id]

/* Convenience wrapper for `DeeType_GetNativeOperator' that
 * already the function pointer into the proper type.
 *
 * Using this, something like (e.g.) `DeeObject_Iter()' is implemented as:
 * >> PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
 * >> DeeObject_Iter(DeeObject *self) {
 * >>     DeeNO_iter_t iter;
 * >>     if unlikely(!Dee_TYPE(self)->tp_seq || (iter = Dee_TYPE(self)->tp_seq->tp_iter) == NULL)
 * >>         iter = _DeeType_RequireNativeOperator(Dee_TYPE(self), iter);
 * >>     return (*iter)(self);
 * >> }
 *
 * And `DeeObject_TIter()' is implemented as:
 * >> PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
 * >> DeeObject_TIter(DeeTypeObject *tp_self, DeeObject *self) {
 * >>     DeeNO_iter_t iter;
 * >>     if unlikely(!tp_self->tp_seq || (iter = tp_self->tp_seq->tp_iter) == NULL) {
 * >>         iter = _DeeType_RequireNativeOperator(tp_self, iter);
 * >>         // vvv these checks are needed because `maketyped__iter' can't handle signaling impls
 * >>         if unlikely(iter == DeeType_GetNativeOperatorOOM(Dee_TNO_iter) ||
 * >>                     iter == DeeType_GetNativeOperatorUnsupported(Dee_TNO_iter))
 * >>             return (*iter)(self);
 * >>     }
 * >>     return (*maketyped__iter(iter))(tp_self, self);
 * >> } */
#define _DeeType_RequireNativeOperator(self, name) \
	((DeeNO_##name##_t)DeeType_GetNativeOperator(self, Dee_TNO_##name))
#define _DeeType_RequireSupportedNativeOperator(self, name) \
	((DeeNO_##name##_t)DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_##name))

/* Inline helper for optimized lookup of a specific native operator,
 * whilst also including fallback handling for inheriting missing
 * operators, as well as returning everything with proper typing. */
#ifdef __OPTIMIZE_SIZE__
#define DeeType_RequireNativeOperator(self, name) \
	_DeeType_RequireNativeOperator(self, name)
#define DeeType_RequireSupportedNativeOperator(self, name) \
	_DeeType_RequireSupportedNativeOperator(self, name)
#else /* __OPTIMIZE_SIZE__ */
#define DeeType_RequireNativeOperator(self, name) \
	(likely(_Dee_TNO_HAS(self, name)) ? _Dee_TNO_GET(self, name) : _DeeType_RequireNativeOperator(self, name))
#define DeeType_RequireSupportedNativeOperator(self, name) \
	(likely(_Dee_TNO_HAS(self, name)) ? _Dee_TNO_GET(self, name) : _DeeType_RequireSupportedNativeOperator(self, name))
#endif /* !__OPTIMIZE_SIZE__ */


#define _Dee_TNO_GET4_PATH1(_, a)    (_)->a
#define _Dee_TNO_GET4_PATH2(_, a, b) (_)->a->b
#define _Dee_TNO_GET3(x)             _Dee_TNO_GET4_##x
#define _Dee_TNO_GET2(x)             _Dee_TNO_GET3(x)
#define _Dee_TNO_GET(tp_self, x)     _Dee_TNO_GET2(_Dee_TNO_PATH_##x(tp_self))
#define _Dee_TNO_HAS4_PATH1(_, a)    (_)->a
#define _Dee_TNO_HAS4_PATH2(_, a, b) ((_)->a && (_)->a->b)
#define _Dee_TNO_HAS3(x)             _Dee_TNO_HAS4_##x
#define _Dee_TNO_HAS2(x)             _Dee_TNO_HAS3(x)
#define _Dee_TNO_HAS(tp_self, x)     _Dee_TNO_HAS2(_Dee_TNO_PATH_##x(tp_self))

/* clang-format off */
/*[[[deemon (printNativeOperatorPathMacros from "...src.deemon.method-hints.method-hints")();]]]*/
#define _Dee_TNO_PATH_assign(_)                     PATH1(_, tp_init.tp_assign)
#define _Dee_TNO_PATH_move_assign(_)                PATH1(_, tp_init.tp_move_assign)
#define _Dee_TNO_PATH_str(_)                        PATH1(_, tp_cast.tp_str)
#define _Dee_TNO_PATH_print(_)                      PATH1(_, tp_cast.tp_print)
#define _Dee_TNO_PATH_repr(_)                       PATH1(_, tp_cast.tp_repr)
#define _Dee_TNO_PATH_printrepr(_)                  PATH1(_, tp_cast.tp_printrepr)
#define _Dee_TNO_PATH_bool(_)                       PATH1(_, tp_cast.tp_bool)
#define _Dee_TNO_PATH_call(_)                       PATH1(_, tp_call)
#define _Dee_TNO_PATH_call_kw(_)                    PATH1(_, tp_call_kw)
#define _Dee_TNO_PATH_iter_next(_)                  PATH1(_, tp_iter_next)
#define _Dee_TNO_PATH_nextpair(_)                   PATH2(_, tp_iterator, tp_nextpair)
#define _Dee_TNO_PATH_nextkey(_)                    PATH2(_, tp_iterator, tp_nextkey)
#define _Dee_TNO_PATH_nextvalue(_)                  PATH2(_, tp_iterator, tp_nextvalue)
#define _Dee_TNO_PATH_advance(_)                    PATH2(_, tp_iterator, tp_advance)
#define _Dee_TNO_PATH_int(_)                        PATH2(_, tp_math, tp_int)
#define _Dee_TNO_PATH_int32(_)                      PATH2(_, tp_math, tp_int32)
#define _Dee_TNO_PATH_int64(_)                      PATH2(_, tp_math, tp_int64)
#define _Dee_TNO_PATH_double(_)                     PATH2(_, tp_math, tp_double)
#define _Dee_TNO_PATH_hash(_)                       PATH2(_, tp_cmp, tp_hash)
#define _Dee_TNO_PATH_compare_eq(_)                 PATH2(_, tp_cmp, tp_compare_eq)
#define _Dee_TNO_PATH_compare(_)                    PATH2(_, tp_cmp, tp_compare)
#define _Dee_TNO_PATH_trycompare_eq(_)              PATH2(_, tp_cmp, tp_trycompare_eq)
#define _Dee_TNO_PATH_eq(_)                         PATH2(_, tp_cmp, tp_eq)
#define _Dee_TNO_PATH_ne(_)                         PATH2(_, tp_cmp, tp_ne)
#define _Dee_TNO_PATH_lo(_)                         PATH2(_, tp_cmp, tp_lo)
#define _Dee_TNO_PATH_le(_)                         PATH2(_, tp_cmp, tp_le)
#define _Dee_TNO_PATH_gr(_)                         PATH2(_, tp_cmp, tp_gr)
#define _Dee_TNO_PATH_ge(_)                         PATH2(_, tp_cmp, tp_ge)
#define _Dee_TNO_PATH_iter(_)                       PATH2(_, tp_seq, tp_iter)
#define _Dee_TNO_PATH_foreach(_)                    PATH2(_, tp_seq, tp_foreach)
#define _Dee_TNO_PATH_foreach_pair(_)               PATH2(_, tp_seq, tp_foreach_pair)
#define _Dee_TNO_PATH_sizeob(_)                     PATH2(_, tp_seq, tp_sizeob)
#define _Dee_TNO_PATH_size(_)                       PATH2(_, tp_seq, tp_size)
#define _Dee_TNO_PATH_size_fast(_)                  PATH2(_, tp_seq, tp_size_fast)
#define _Dee_TNO_PATH_contains(_)                   PATH2(_, tp_seq, tp_contains)
#define _Dee_TNO_PATH_getitem(_)                    PATH2(_, tp_seq, tp_getitem)
#define _Dee_TNO_PATH_trygetitem(_)                 PATH2(_, tp_seq, tp_trygetitem)
#define _Dee_TNO_PATH_getitem_index_fast(_)         PATH2(_, tp_seq, tp_getitem_index_fast)
#define _Dee_TNO_PATH_getitem_index(_)              PATH2(_, tp_seq, tp_getitem_index)
#define _Dee_TNO_PATH_trygetitem_index(_)           PATH2(_, tp_seq, tp_trygetitem_index)
#define _Dee_TNO_PATH_getitem_string_hash(_)        PATH2(_, tp_seq, tp_getitem_string_hash)
#define _Dee_TNO_PATH_trygetitem_string_hash(_)     PATH2(_, tp_seq, tp_trygetitem_string_hash)
#define _Dee_TNO_PATH_getitem_string_len_hash(_)    PATH2(_, tp_seq, tp_getitem_string_len_hash)
#define _Dee_TNO_PATH_trygetitem_string_len_hash(_) PATH2(_, tp_seq, tp_trygetitem_string_len_hash)
#define _Dee_TNO_PATH_bounditem(_)                  PATH2(_, tp_seq, tp_bounditem)
#define _Dee_TNO_PATH_bounditem_index(_)            PATH2(_, tp_seq, tp_bounditem_index)
#define _Dee_TNO_PATH_bounditem_string_hash(_)      PATH2(_, tp_seq, tp_bounditem_string_hash)
#define _Dee_TNO_PATH_bounditem_string_len_hash(_)  PATH2(_, tp_seq, tp_bounditem_string_len_hash)
#define _Dee_TNO_PATH_hasitem(_)                    PATH2(_, tp_seq, tp_hasitem)
#define _Dee_TNO_PATH_hasitem_index(_)              PATH2(_, tp_seq, tp_hasitem_index)
#define _Dee_TNO_PATH_hasitem_string_hash(_)        PATH2(_, tp_seq, tp_hasitem_string_hash)
#define _Dee_TNO_PATH_hasitem_string_len_hash(_)    PATH2(_, tp_seq, tp_hasitem_string_len_hash)
#define _Dee_TNO_PATH_delitem(_)                    PATH2(_, tp_seq, tp_delitem)
#define _Dee_TNO_PATH_delitem_index(_)              PATH2(_, tp_seq, tp_delitem_index)
#define _Dee_TNO_PATH_delitem_string_hash(_)        PATH2(_, tp_seq, tp_delitem_string_hash)
#define _Dee_TNO_PATH_delitem_string_len_hash(_)    PATH2(_, tp_seq, tp_delitem_string_len_hash)
#define _Dee_TNO_PATH_setitem(_)                    PATH2(_, tp_seq, tp_setitem)
#define _Dee_TNO_PATH_setitem_index(_)              PATH2(_, tp_seq, tp_setitem_index)
#define _Dee_TNO_PATH_setitem_string_hash(_)        PATH2(_, tp_seq, tp_setitem_string_hash)
#define _Dee_TNO_PATH_setitem_string_len_hash(_)    PATH2(_, tp_seq, tp_setitem_string_len_hash)
#define _Dee_TNO_PATH_getrange(_)                   PATH2(_, tp_seq, tp_getrange)
#define _Dee_TNO_PATH_getrange_index(_)             PATH2(_, tp_seq, tp_getrange_index)
#define _Dee_TNO_PATH_getrange_index_n(_)           PATH2(_, tp_seq, tp_getrange_index_n)
#define _Dee_TNO_PATH_delrange(_)                   PATH2(_, tp_seq, tp_delrange)
#define _Dee_TNO_PATH_delrange_index(_)             PATH2(_, tp_seq, tp_delrange_index)
#define _Dee_TNO_PATH_delrange_index_n(_)           PATH2(_, tp_seq, tp_delrange_index_n)
#define _Dee_TNO_PATH_setrange(_)                   PATH2(_, tp_seq, tp_setrange)
#define _Dee_TNO_PATH_setrange_index(_)             PATH2(_, tp_seq, tp_setrange_index)
#define _Dee_TNO_PATH_setrange_index_n(_)           PATH2(_, tp_seq, tp_setrange_index_n)
#define _Dee_TNO_PATH_inv(_)                        PATH2(_, tp_math, tp_inv)
#define _Dee_TNO_PATH_pos(_)                        PATH2(_, tp_math, tp_pos)
#define _Dee_TNO_PATH_neg(_)                        PATH2(_, tp_math, tp_neg)
#define _Dee_TNO_PATH_add(_)                        PATH2(_, tp_math, tp_add)
#define _Dee_TNO_PATH_inplace_add(_)                PATH2(_, tp_math, tp_inplace_add)
#define _Dee_TNO_PATH_sub(_)                        PATH2(_, tp_math, tp_sub)
#define _Dee_TNO_PATH_inplace_sub(_)                PATH2(_, tp_math, tp_inplace_sub)
#define _Dee_TNO_PATH_mul(_)                        PATH2(_, tp_math, tp_mul)
#define _Dee_TNO_PATH_inplace_mul(_)                PATH2(_, tp_math, tp_inplace_mul)
#define _Dee_TNO_PATH_div(_)                        PATH2(_, tp_math, tp_div)
#define _Dee_TNO_PATH_inplace_div(_)                PATH2(_, tp_math, tp_inplace_div)
#define _Dee_TNO_PATH_mod(_)                        PATH2(_, tp_math, tp_mod)
#define _Dee_TNO_PATH_inplace_mod(_)                PATH2(_, tp_math, tp_inplace_mod)
#define _Dee_TNO_PATH_shl(_)                        PATH2(_, tp_math, tp_shl)
#define _Dee_TNO_PATH_inplace_shl(_)                PATH2(_, tp_math, tp_inplace_shl)
#define _Dee_TNO_PATH_shr(_)                        PATH2(_, tp_math, tp_shr)
#define _Dee_TNO_PATH_inplace_shr(_)                PATH2(_, tp_math, tp_inplace_shr)
#define _Dee_TNO_PATH_and(_)                        PATH2(_, tp_math, tp_and)
#define _Dee_TNO_PATH_inplace_and(_)                PATH2(_, tp_math, tp_inplace_and)
#define _Dee_TNO_PATH_or(_)                         PATH2(_, tp_math, tp_or)
#define _Dee_TNO_PATH_inplace_or(_)                 PATH2(_, tp_math, tp_inplace_or)
#define _Dee_TNO_PATH_xor(_)                        PATH2(_, tp_math, tp_xor)
#define _Dee_TNO_PATH_inplace_xor(_)                PATH2(_, tp_math, tp_inplace_xor)
#define _Dee_TNO_PATH_pow(_)                        PATH2(_, tp_math, tp_pow)
#define _Dee_TNO_PATH_inplace_pow(_)                PATH2(_, tp_math, tp_inplace_pow)
#define _Dee_TNO_PATH_inc(_)                        PATH2(_, tp_math, tp_inc)
#define _Dee_TNO_PATH_dec(_)                        PATH2(_, tp_math, tp_dec)
#define _Dee_TNO_PATH_enter(_)                      PATH2(_, tp_with, tp_enter)
#define _Dee_TNO_PATH_leave(_)                      PATH2(_, tp_with, tp_leave)
/*[[[end]]]*/
/* clang-format on */


/* clang-format off */
/*[[[deemon (printNativeOperatorHintDecls from "...src.deemon.method-hints.method-hints")();]]]*/
/* tp_init.tp_assign */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__assign(DeeObject *self, DeeObject *value);
#define isusrtype__assign(tp_assign) ((tp_assign) == &usrtype__assign)
#define maketyped__assign(tp_assign) ((tp_assign) == &usrtype__assign ? &tusrtype__assign : &tdefault__assign)

/* tp_init.tp_move_assign */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__move_assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__move_assign__with__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__move_assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__move_assign(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__move_assign__with__assign(DeeObject *self, DeeObject *value);
#define isusrtype__move_assign(tp_move_assign) ((tp_move_assign) == &usrtype__move_assign)
#define isdefault__move_assign(tp_move_assign) ((tp_move_assign) == &default__move_assign__with__assign)
#define maketyped__move_assign(tp_move_assign) ((tp_move_assign) == &usrtype__move_assign ? &tusrtype__move_assign : (tp_move_assign) == &default__move_assign__with__assign ? &tdefault__move_assign__with__assign : &tdefault__move_assign)

/* tp_cast.tp_str */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str__by_print(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__str__with__print(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__str(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str__by_print(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__str__with__print(DeeObject *__restrict self);
#define isusrtype__str(tp_str) ((tp_str) == &usrtype__str || (tp_str) == &usrtype__str__by_print)
#define isdefault__str(tp_str) ((tp_str) == &default__str__with__print)
#define maketyped__str(tp_str) ((tp_str) == &usrtype__str ? &tusrtype__str : (tp_str) == &usrtype__str__by_print ? &tusrtype__str__by_print : (tp_str) == &default__str__with__print ? &tdefault__str__with__print : &tdefault__str)

/* tp_cast.tp_print */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__print__with__str(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__print__with__str(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__print(tp_print) ((tp_print) == &usrtype__print || (tp_print) == &usrtype__print__by_print)
#define isdefault__print(tp_print) ((tp_print) == &default__print__with__str)
#define maketyped__print(tp_print) ((tp_print) == &usrtype__print ? &tusrtype__print : (tp_print) == &usrtype__print__by_print ? &tusrtype__print__by_print : (tp_print) == &default__print__with__str ? &tdefault__print__with__str : &tdefault__print)

/* tp_cast.tp_repr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr__by_printrepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__repr__with__printrepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__repr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr__by_printrepr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__repr__with__printrepr(DeeObject *__restrict self);
#define isusrtype__repr(tp_repr) ((tp_repr) == &usrtype__repr || (tp_repr) == &usrtype__repr__by_printrepr)
#define isdefault__repr(tp_repr) ((tp_repr) == &default__repr__with__printrepr)
#define maketyped__repr(tp_repr) ((tp_repr) == &usrtype__repr ? &tusrtype__repr : (tp_repr) == &usrtype__repr__by_printrepr ? &tusrtype__repr__by_printrepr : (tp_repr) == &default__repr__with__printrepr ? &tdefault__repr__with__printrepr : &tdefault__repr)

/* tp_cast.tp_printrepr */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__printrepr__with__repr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__printrepr__with__repr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr || (tp_printrepr) == &usrtype__printrepr__by_print)
#define isdefault__printrepr(tp_printrepr) ((tp_printrepr) == &default__printrepr__with__repr)
#define maketyped__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr ? &tusrtype__printrepr : (tp_printrepr) == &usrtype__printrepr__by_print ? &tusrtype__printrepr__by_print : (tp_printrepr) == &default__printrepr__with__repr ? &tdefault__printrepr__with__repr : &tdefault__printrepr)

/* tp_cast.tp_bool */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__bool(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bool(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__bool(DeeObject *__restrict self);
#define isusrtype__bool(tp_bool) ((tp_bool) == &usrtype__bool)
#define maketyped__bool(tp_bool) ((tp_bool) == &usrtype__bool ? &tusrtype__bool : &tdefault__bool)

/* tp_call */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call__with__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call__with__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv);
#define isusrtype__call(tp_call) ((tp_call) == &usrtype__call)
#define isdefault__call(tp_call) ((tp_call) == &default__call__with__call_kw)
#define maketyped__call(tp_call) ((tp_call) == &usrtype__call ? &tusrtype__call : (tp_call) == &default__call__with__call_kw ? &tdefault__call__with__call_kw : &tdefault__call)

/* tp_call_kw */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call_kw__with__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call_kw__with__call(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define isusrtype__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw)
#define isdefault__call_kw(tp_call_kw) ((tp_call_kw) == &default__call_kw__with__call)
#define maketyped__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw ? &tusrtype__call_kw : (tp_call_kw) == &default__call_kw__with__call ? &tdefault__call_kw__with__call : &tdefault__call_kw)

/* tp_iter_next */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_next__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_next__with__nextpair(DeeObject *__restrict self);
#define isusrtype__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next)
#define isdefault__iter_next(tp_iter_next) ((tp_iter_next) == &default__iter_next__with__nextpair)
#define maketyped__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next ? &tusrtype__iter_next : (tp_iter_next) == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next)

/* tp_iterator->tp_nextpair */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__nextpair__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__nextpair(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__nextpair__with__iter_next(DeeObject *__restrict self, DREF DeeObject *key_and_value[2]);
#define isdefault__nextpair(tp_nextpair) ((tp_nextpair) == &default__nextpair__with__iter_next)
#define maketyped__nextpair(tp_nextpair) ((tp_nextpair) == &default__nextpair__with__iter_next ? &tdefault__nextpair__with__iter_next : &tdefault__nextpair)

/* tp_iterator->tp_nextkey */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextkey__with__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextkey__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextkey(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextkey__with__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextkey__with__nextpair(DeeObject *__restrict self);
#define isdefault__nextkey(tp_nextkey) ((tp_nextkey) == &default__nextkey__with__iter_next || (tp_nextkey) == &default__nextkey__with__nextpair)
#define maketyped__nextkey(tp_nextkey) ((tp_nextkey) == &default__nextkey__with__iter_next ? &tdefault__nextkey__with__iter_next : (tp_nextkey) == &default__nextkey__with__nextpair ? &tdefault__nextkey__with__nextpair : &tdefault__nextkey)

/* tp_iterator->tp_nextvalue */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextvalue__with__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextvalue__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextvalue(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextvalue__with__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextvalue__with__nextpair(DeeObject *__restrict self);
#define isdefault__nextvalue(tp_nextvalue) ((tp_nextvalue) == &default__nextvalue__with__iter_next || (tp_nextvalue) == &default__nextvalue__with__nextpair)
#define maketyped__nextvalue(tp_nextvalue) ((tp_nextvalue) == &default__nextvalue__with__iter_next ? &tdefault__nextvalue__with__iter_next : (tp_nextvalue) == &default__nextvalue__with__nextpair ? &tdefault__nextvalue__with__nextpair : &tdefault__nextvalue)

/* tp_iterator->tp_advance */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextkey(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextvalue(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextpair(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextkey(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextvalue(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextpair(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__iter_next(DeeObject *__restrict self, size_t step);
#define isdefault__advance(tp_advance) ((tp_advance) == &default__advance__with__nextkey || (tp_advance) == &default__advance__with__nextvalue || (tp_advance) == &default__advance__with__nextpair || (tp_advance) == &default__advance__with__iter_next)
#define maketyped__advance(tp_advance) ((tp_advance) == &default__advance__with__nextkey ? &tdefault__advance__with__nextkey : (tp_advance) == &default__advance__with__nextvalue ? &tdefault__advance__with__nextvalue : (tp_advance) == &default__advance__with__nextpair ? &tdefault__advance__with__nextpair : (tp_advance) == &default__advance__with__iter_next ? &tdefault__advance__with__iter_next : &tdefault__advance)

/* tp_math->tp_int */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__int(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int64(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int32(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__double(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__int(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int64(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int32(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__double(DeeObject *__restrict self);
#define isusrtype__int(tp_int) ((tp_int) == &usrtype__int)
#define isdefault__int(tp_int) ((tp_int) == &default__int__with__int64 || (tp_int) == &default__int__with__int32 || (tp_int) == &default__int__with__double)
#define maketyped__int(tp_int) ((tp_int) == &usrtype__int ? &tusrtype__int : (tp_int) == &default__int__with__int64 ? &tdefault__int__with__int64 : (tp_int) == &default__int__with__int32 ? &tdefault__int__with__int32 : (tp_int) == &default__int__with__double ? &tdefault__int__with__double : &tdefault__int)

/* tp_math->tp_int32 */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__int64(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__int(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__double(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__int64(DeeObject *__restrict self, int32_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__int(DeeObject *__restrict self, int32_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__double(DeeObject *__restrict self, int32_t *__restrict p_result);
#define isdefault__int32(tp_int32) ((tp_int32) == &default__int32__with__int64 || (tp_int32) == &default__int32__with__int || (tp_int32) == &default__int32__with__double)
#define maketyped__int32(tp_int32) ((tp_int32) == &default__int32__with__int64 ? &tdefault__int32__with__int64 : (tp_int32) == &default__int32__with__int ? &tdefault__int32__with__int : (tp_int32) == &default__int32__with__double ? &tdefault__int32__with__double : &tdefault__int32)

/* tp_math->tp_int64 */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__int32(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__int(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__double(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__int32(DeeObject *__restrict self, int64_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__int(DeeObject *__restrict self, int64_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__double(DeeObject *__restrict self, int64_t *__restrict p_result);
#define isdefault__int64(tp_int64) ((tp_int64) == &default__int64__with__int32 || (tp_int64) == &default__int64__with__int || (tp_int64) == &default__int64__with__double)
#define maketyped__int64(tp_int64) ((tp_int64) == &default__int64__with__int32 ? &tdefault__int64__with__int32 : (tp_int64) == &default__int64__with__int ? &tdefault__int64__with__int : (tp_int64) == &default__int64__with__double ? &tdefault__int64__with__double : &tdefault__int64)

/* tp_math->tp_double */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int64(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int32(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__double(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int64(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int32(DeeObject *__restrict self, double *__restrict p_result);
#define isusrtype__double(tp_double) ((tp_double) == &usrtype__double)
#define isdefault__double(tp_double) ((tp_double) == &default__double__with__int || (tp_double) == &default__double__with__int64 || (tp_double) == &default__double__with__int32)
#define maketyped__double(tp_double) ((tp_double) == &usrtype__double ? &tusrtype__double : (tp_double) == &default__double__with__int ? &tdefault__double__with__int : (tp_double) == &default__double__with__int64 ? &tdefault__double__with__int64 : (tp_double) == &default__double__with__int32 ? &tdefault__double__with__int32 : &tdefault__double)

/* tp_cmp->tp_hash */
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tusrtype__hash(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tdefault__hash(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL usrtype__hash(DeeObject *__restrict self);
#define isusrtype__hash(tp_hash) ((tp_hash) == &usrtype__hash)
#define maketyped__hash(tp_hash) ((tp_hash) == &usrtype__hash ? &tusrtype__hash : &tdefault__hash)

/* tp_cmp->tp_compare_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__lo__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__le__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__lo__and__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__le__and__ge(DeeObject *lhs, DeeObject *rhs);
#define isdefault__compare_eq(tp_compare_eq) ((tp_compare_eq) == &default__compare_eq__with__compare || (tp_compare_eq) == &default__compare_eq__with__eq || (tp_compare_eq) == &default__compare_eq__with__ne || (tp_compare_eq) == &default__compare_eq__with__lo__and__gr || (tp_compare_eq) == &default__compare_eq__with__le__and__ge)
#define maketyped__compare_eq(tp_compare_eq) ((tp_compare_eq) == &default__compare_eq__with__compare ? &tdefault__compare_eq__with__compare : (tp_compare_eq) == &default__compare_eq__with__eq ? &tdefault__compare_eq__with__eq : (tp_compare_eq) == &default__compare_eq__with__ne ? &tdefault__compare_eq__with__ne : (tp_compare_eq) == &default__compare_eq__with__lo__and__gr ? &tdefault__compare_eq__with__lo__and__gr : (tp_compare_eq) == &default__compare_eq__with__le__and__ge ? &tdefault__compare_eq__with__le__and__ge : &tdefault__compare_eq)

/* tp_cmp->tp_compare */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__eq__and__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__eq__and__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__eq__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__eq__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__ne__and__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__ne__and__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__ne__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare__with__ne__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define tdefault__compare__with__lo__and__gr (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))&tdefault__compare_eq__with__lo__and__gr)
#define tdefault__compare__with__le__and__ge (*(int (DCALL *)(DeeTypeObject *, DeeObject *, DeeObject *))&tdefault__compare_eq__with__le__and__ge)
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__eq__and__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__eq__and__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__eq__and__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__eq__and__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__ne__and__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__ne__and__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__ne__and__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare__with__ne__and__ge(DeeObject *lhs, DeeObject *rhs);
#define default__compare__with__lo__and__gr (*(int (DCALL *)(DeeObject *, DeeObject *))&default__compare_eq__with__lo__and__gr)
#define default__compare__with__le__and__ge (*(int (DCALL *)(DeeObject *, DeeObject *))&default__compare_eq__with__le__and__ge)
#define isdefault__compare(tp_compare) ((tp_compare) == &default__compare__with__eq__and__lo || (tp_compare) == &default__compare__with__eq__and__le || (tp_compare) == &default__compare__with__eq__and__gr || (tp_compare) == &default__compare__with__eq__and__ge || (tp_compare) == &default__compare__with__ne__and__lo || (tp_compare) == &default__compare__with__ne__and__le || (tp_compare) == &default__compare__with__ne__and__gr || (tp_compare) == &default__compare__with__ne__and__ge || (tp_compare) == &default__compare__with__lo__and__gr || (tp_compare) == &default__compare__with__le__and__ge)
#define maketyped__compare(tp_compare) ((tp_compare) == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : (tp_compare) == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : (tp_compare) == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : (tp_compare) == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : (tp_compare) == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : (tp_compare) == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : (tp_compare) == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : (tp_compare) == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : (tp_compare) == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : (tp_compare) == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare)

/* tp_cmp->tp_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__trycompare_eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__trycompare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__trycompare_eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isdefault__trycompare_eq(tp_trycompare_eq) ((tp_trycompare_eq) == &default__trycompare_eq__with__compare_eq)
#define maketyped__trycompare_eq(tp_trycompare_eq) ((tp_trycompare_eq) == &default__trycompare_eq__with__compare_eq ? &tdefault__trycompare_eq__with__compare_eq : &tdefault__trycompare_eq)

/* tp_cmp->tp_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq__with__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__eq__with__ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__eq(tp_eq) ((tp_eq) == &usrtype__eq)
#define isdefault__eq(tp_eq) ((tp_eq) == &default__eq__with__ne || (tp_eq) == &default__eq__with__compare_eq)
#define maketyped__eq(tp_eq) ((tp_eq) == &usrtype__eq ? &tusrtype__eq : (tp_eq) == &default__eq__with__ne ? &tdefault__eq__with__ne : (tp_eq) == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq)

/* tp_cmp->tp_ne */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne__with__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ne__with__eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ne__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__ne(tp_ne) ((tp_ne) == &usrtype__ne)
#define isdefault__ne(tp_ne) ((tp_ne) == &default__ne__with__eq || (tp_ne) == &default__ne__with__compare_eq)
#define maketyped__ne(tp_ne) ((tp_ne) == &usrtype__ne ? &tusrtype__ne : (tp_ne) == &default__ne__with__eq ? &tdefault__ne__with__eq : (tp_ne) == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne)

/* tp_cmp->tp_lo */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo__with__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__lo__with__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__lo__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__lo(tp_lo) ((tp_lo) == &usrtype__lo)
#define isdefault__lo(tp_lo) ((tp_lo) == &default__lo__with__ge || (tp_lo) == &default__lo__with__compare)
#define maketyped__lo(tp_lo) ((tp_lo) == &usrtype__lo ? &tusrtype__lo : (tp_lo) == &default__lo__with__ge ? &tdefault__lo__with__ge : (tp_lo) == &default__lo__with__compare ? &tdefault__lo__with__compare : &tdefault__lo)

/* tp_cmp->tp_le */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le__with__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__le__with__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__le__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__le(tp_le) ((tp_le) == &usrtype__le)
#define isdefault__le(tp_le) ((tp_le) == &default__le__with__gr || (tp_le) == &default__le__with__compare)
#define maketyped__le(tp_le) ((tp_le) == &usrtype__le ? &tusrtype__le : (tp_le) == &default__le__with__gr ? &tdefault__le__with__gr : (tp_le) == &default__le__with__compare ? &tdefault__le__with__compare : &tdefault__le)

/* tp_cmp->tp_gr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr__with__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__gr__with__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__gr__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__gr(tp_gr) ((tp_gr) == &usrtype__gr)
#define isdefault__gr(tp_gr) ((tp_gr) == &default__gr__with__le || (tp_gr) == &default__gr__with__compare)
#define maketyped__gr(tp_gr) ((tp_gr) == &usrtype__gr ? &tusrtype__gr : (tp_gr) == &default__gr__with__le ? &tdefault__gr__with__le : (tp_gr) == &default__gr__with__compare ? &tdefault__gr__with__compare : &tdefault__gr)

/* tp_cmp->tp_ge */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge__with__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ge__with__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ge__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__ge(tp_ge) ((tp_ge) == &usrtype__ge)
#define isdefault__ge(tp_ge) ((tp_ge) == &default__ge__with__lo || (tp_ge) == &default__ge__with__compare)
#define maketyped__ge(tp_ge) ((tp_ge) == &usrtype__ge ? &tusrtype__ge : (tp_ge) == &default__ge__with__lo ? &tdefault__ge__with__lo : (tp_ge) == &default__ge__with__compare ? &tdefault__ge__with__compare : &tdefault__ge)

/* tp_seq->tp_iter */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach_pair(DeeObject *__restrict self);
#define isusrtype__iter(tp_iter) ((tp_iter) == &usrtype__iter)
#define isdefault__iter(tp_iter) ((tp_iter) == &default__iter__with__foreach || (tp_iter) == &default__iter__with__foreach_pair)
#define maketyped__iter(tp_iter) ((tp_iter) == &usrtype__iter ? &tusrtype__iter : &tdefault__iter)

/* tp_seq->tp_foreach */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach__with__iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach__with__foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define isdefault__foreach(tp_foreach) ((tp_foreach) == &default__foreach__with__iter || (tp_foreach) == &default__foreach__with__foreach_pair)
#define maketyped__foreach(tp_foreach) ((tp_foreach) == &default__foreach__with__iter ? &tdefault__foreach__with__iter : (tp_foreach) == &default__foreach__with__foreach_pair ? &tdefault__foreach__with__foreach_pair : &tdefault__foreach)

/* tp_seq->tp_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach_pair__with__foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach_pair__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach_pair__with__foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach_pair__with__iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define isdefault__foreach_pair(tp_foreach_pair) ((tp_foreach_pair) == &default__foreach_pair__with__foreach || (tp_foreach_pair) == &default__foreach_pair__with__iter)
#define maketyped__foreach_pair(tp_foreach_pair) ((tp_foreach_pair) == &default__foreach_pair__with__foreach ? &tdefault__foreach_pair__with__foreach : (tp_foreach_pair) == &default__foreach_pair__with__iter ? &tdefault__foreach_pair__with__iter : &tdefault__foreach_pair)

/* tp_seq->tp_sizeob */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__sizeob(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__sizeob__with__size(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__sizeob(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__sizeob(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__sizeob__with__size(DeeObject *__restrict self);
#define isusrtype__sizeob(tp_sizeob) ((tp_sizeob) == &usrtype__sizeob)
#define isdefault__sizeob(tp_sizeob) ((tp_sizeob) == &default__sizeob__with__size)
#define maketyped__sizeob(tp_sizeob) ((tp_sizeob) == &usrtype__sizeob ? &tusrtype__sizeob : (tp_sizeob) == &default__sizeob__with__size ? &tdefault__sizeob__with__size : &tdefault__sizeob)

/* tp_seq->tp_size */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size__with__sizeob(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__size__with__sizeob(DeeObject *__restrict self);
#define isdefault__size(tp_size) ((tp_size) == &default__size__with__sizeob)
#define maketyped__size(tp_size) ((tp_size) == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size)

/* tp_seq->tp_size_fast */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size_fast__with__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size_fast(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__size_fast__with__(DeeObject *__restrict self);
#define isdefault__size_fast(tp_size_fast) ((tp_size_fast) == &default__size_fast__with__)

/* tp_seq->tp_contains */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__contains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__contains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__contains(DeeObject *self, DeeObject *item);
#define isusrtype__contains(tp_contains) ((tp_contains) == &usrtype__contains)
#define maketyped__contains(tp_contains) ((tp_contains) == &usrtype__contains ? &tusrtype__contains : &tdefault__contains)

/* tp_seq->tp_getitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index__and__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index__and__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__trygetitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index__and__getitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index__and__getitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_string_hash(DeeObject *self, DeeObject *index);
#define isusrtype__getitem(tp_getitem) ((tp_getitem) == &usrtype__getitem)
#define isdefault__getitem(tp_getitem) ((tp_getitem) == &default__getitem__with__trygetitem || (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_len_hash || (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_hash || (tp_getitem) == &default__getitem__with__getitem_index || (tp_getitem) == &default__getitem__with__getitem_string_len_hash || (tp_getitem) == &default__getitem__with__getitem_string_hash)
#define maketyped__getitem(tp_getitem) ((tp_getitem) == &usrtype__getitem ? &tusrtype__getitem : (tp_getitem) == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : (tp_getitem) == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : (tp_getitem) == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : (tp_getitem) == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : &tdefault__getitem)

/* tp_seq->tp_trygetitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_string_hash(DeeObject *self, DeeObject *index);
#define isdefault__trygetitem(tp_trygetitem) ((tp_trygetitem) == &default__trygetitem__with__getitem || (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_index || (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_len_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_hash)
#define maketyped__trygetitem(tp_trygetitem) ((tp_trygetitem) == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : &tdefault__trygetitem)

/* tp_seq->tp_getitem_index_fast */

/* tp_seq->tp_getitem_index */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__getitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__trygetitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__getitem(DeeObject *self, size_t index);
#define isdefault__getitem_index(tp_getitem_index) ((tp_getitem_index) == &default__getitem_index__with__size__and__getitem_index_fast || (tp_getitem_index) == &default__getitem_index__with__trygetitem_index || (tp_getitem_index) == &default__getitem_index__with__getitem)
#define maketyped__getitem_index(tp_getitem_index) ((tp_getitem_index) == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : (tp_getitem_index) == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : (tp_getitem_index) == &default__getitem_index__with__getitem ? &tdefault__getitem_index__with__getitem : &tdefault__getitem_index)

/* tp_seq->tp_trygetitem_index */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__trygetitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__trygetitem_index__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__trygetitem_index__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__trygetitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__trygetitem_index__with__getitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__trygetitem_index__with__trygetitem(DeeObject *self, size_t index);
#define isdefault__trygetitem_index(tp_trygetitem_index) ((tp_trygetitem_index) == &default__trygetitem_index__with__size__and__getitem_index_fast || (tp_trygetitem_index) == &default__trygetitem_index__with__getitem_index || (tp_trygetitem_index) == &default__trygetitem_index__with__trygetitem)
#define maketyped__trygetitem_index(tp_trygetitem_index) ((tp_trygetitem_index) == &default__trygetitem_index__with__size__and__getitem_index_fast ? &tdefault__trygetitem_index__with__size__and__getitem_index_fast : (tp_trygetitem_index) == &default__trygetitem_index__with__getitem_index ? &tdefault__trygetitem_index__with__getitem_index : (tp_trygetitem_index) == &default__trygetitem_index__with__trygetitem ? &tdefault__trygetitem_index__with__trygetitem : &tdefault__trygetitem_index)

/* tp_seq->tp_getitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__getitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__getitem_string_hash(tp_getitem_string_hash) ((tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash || (tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem)
#define maketyped__getitem_string_hash(tp_getitem_string_hash) ((tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash : (tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem ? &tdefault__getitem_string_hash__with__getitem : &tdefault__getitem_string_hash)

/* tp_seq->tp_trygetitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_hash__with__trygetitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__trygetitem_string_hash(tp_trygetitem_string_hash) ((tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__getitem_string_hash || (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem)
#define maketyped__trygetitem_string_hash(tp_trygetitem_string_hash) ((tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash)

/* tp_seq->tp_getitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__getitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__getitem_string_len_hash(tp_getitem_string_len_hash) ((tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash || (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem || (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem_string_hash)
#define maketyped__getitem_string_len_hash(tp_getitem_string_len_hash) ((tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash : (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem ? &tdefault__getitem_string_len_hash__with__getitem : (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem_string_hash ? &tdefault__getitem_string_len_hash__with__getitem_string_hash : &tdefault__getitem_string_len_hash)

/* tp_seq->tp_trygetitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__trygetitem_string_len_hash(tp_trygetitem_string_len_hash) ((tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__getitem_string_len_hash || (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem || (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem_string_hash)
#define maketyped__trygetitem_string_len_hash(tp_trygetitem_string_len_hash) ((tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem_string_hash ? &tdefault__trygetitem_string_len_hash__with__trygetitem_string_hash : &tdefault__trygetitem_string_len_hash)

/* tp_seq->tp_bounditem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__trygetitem__and__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__trygetitem__and__hasitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__trygetitem(DeeObject *self, DeeObject *index);
#define isdefault__bounditem(tp_bounditem) ((tp_bounditem) == &default__bounditem__with__size__and__getitem_index_fast || (tp_bounditem) == &default__bounditem__with__getitem || (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash || (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_hash || (tp_bounditem) == &default__bounditem__with__trygetitem__and__hasitem || (tp_bounditem) == &default__bounditem__with__bounditem_index || (tp_bounditem) == &default__bounditem__with__bounditem_string_len_hash || (tp_bounditem) == &default__bounditem__with__bounditem_string_hash || (tp_bounditem) == &default__bounditem__with__trygetitem)
#define maketyped__bounditem(tp_bounditem) ((tp_bounditem) == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : (tp_bounditem) == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : (tp_bounditem) == &default__bounditem__with__bounditem_index ? &tdefault__bounditem__with__bounditem_index : (tp_bounditem) == &default__bounditem__with__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_string_len_hash : (tp_bounditem) == &default__bounditem__with__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_string_hash : (tp_bounditem) == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem)

/* tp_seq->tp_bounditem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__getitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__bounditem(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__trygetitem_index(DeeObject *self, size_t index);
#define isdefault__bounditem_index(tp_bounditem_index) ((tp_bounditem_index) == &default__bounditem_index__with__size__and__getitem_index_fast || (tp_bounditem_index) == &default__bounditem_index__with__getitem_index || (tp_bounditem_index) == &default__bounditem_index__with__bounditem || (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index__and__hasitem_index || (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index)
#define maketyped__bounditem_index(tp_bounditem_index) ((tp_bounditem_index) == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : (tp_bounditem_index) == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : (tp_bounditem_index) == &default__bounditem_index__with__bounditem ? &tdefault__bounditem_index__with__bounditem : (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index)

/* tp_seq->tp_bounditem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__bounditem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__bounditem_string_hash(tp_bounditem_string_hash) ((tp_bounditem_string_hash) == &default__bounditem_string_hash__with__getitem_string_hash || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash)
#define maketyped__bounditem_string_hash(tp_bounditem_string_hash) ((tp_bounditem_string_hash) == &default__bounditem_string_hash__with__getitem_string_hash ? &tdefault__bounditem_string_hash__with__getitem_string_hash : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem ? &tdefault__bounditem_string_hash__with__bounditem : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash : &tdefault__bounditem_string_hash)

/* tp_seq->tp_bounditem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__bounditem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__bounditem_string_len_hash(tp_bounditem_string_len_hash) ((tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__getitem_string_len_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem_string_hash)
#define maketyped__bounditem_string_len_hash(tp_bounditem_string_len_hash) ((tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__getitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem ? &tdefault__bounditem_string_len_hash__with__bounditem : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem_string_hash ? &tdefault__bounditem_string_len_hash__with__bounditem_string_hash : &tdefault__bounditem_string_len_hash)

/* tp_seq->tp_hasitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__trygetitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__bounditem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_string_hash(DeeObject *self, DeeObject *index);
#define isdefault__hasitem(tp_hasitem) ((tp_hasitem) == &default__hasitem__with__size__and__getitem_index_fast || (tp_hasitem) == &default__hasitem__with__trygetitem || (tp_hasitem) == &default__hasitem__with__bounditem || (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash || (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_hash || (tp_hasitem) == &default__hasitem__with__hasitem_index || (tp_hasitem) == &default__hasitem__with__hasitem_string_len_hash || (tp_hasitem) == &default__hasitem__with__hasitem_string_hash)
#define maketyped__hasitem(tp_hasitem) ((tp_hasitem) == &default__hasitem__with__trygetitem ? &tdefault__hasitem__with__trygetitem : (tp_hasitem) == &default__hasitem__with__bounditem ? &tdefault__hasitem__with__bounditem : (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : (tp_hasitem) == &default__hasitem__with__hasitem_index ? &tdefault__hasitem__with__hasitem_index : (tp_hasitem) == &default__hasitem__with__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_string_len_hash : (tp_hasitem) == &default__hasitem__with__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_string_hash : &tdefault__hasitem)

/* tp_seq->tp_hasitem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__trygetitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__bounditem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__hasitem(DeeObject *self, size_t index);
#define isdefault__hasitem_index(tp_hasitem_index) ((tp_hasitem_index) == &default__hasitem_index__with__size__and__getitem_index_fast || (tp_hasitem_index) == &default__hasitem_index__with__trygetitem_index || (tp_hasitem_index) == &default__hasitem_index__with__bounditem_index || (tp_hasitem_index) == &default__hasitem_index__with__hasitem)
#define maketyped__hasitem_index(tp_hasitem_index) ((tp_hasitem_index) == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : (tp_hasitem_index) == &default__hasitem_index__with__trygetitem_index ? &tdefault__hasitem_index__with__trygetitem_index : (tp_hasitem_index) == &default__hasitem_index__with__bounditem_index ? &tdefault__hasitem_index__with__bounditem_index : (tp_hasitem_index) == &default__hasitem_index__with__hasitem ? &tdefault__hasitem_index__with__hasitem : &tdefault__hasitem_index)

/* tp_seq->tp_hasitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__hasitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__hasitem_string_hash(tp_hasitem_string_hash) ((tp_hasitem_string_hash) == &default__hasitem_string_hash__with__trygetitem_string_hash || (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__bounditem_string_hash || (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem)
#define maketyped__hasitem_string_hash(tp_hasitem_string_hash) ((tp_hasitem_string_hash) == &default__hasitem_string_hash__with__trygetitem_string_hash ? &tdefault__hasitem_string_hash__with__trygetitem_string_hash : (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__bounditem_string_hash ? &tdefault__hasitem_string_hash__with__bounditem_string_hash : (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem ? &tdefault__hasitem_string_hash__with__hasitem : &tdefault__hasitem_string_hash)

/* tp_seq->tp_hasitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__hasitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__hasitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__hasitem_string_len_hash(tp_hasitem_string_len_hash) ((tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__trygetitem_string_len_hash || (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__bounditem_string_len_hash || (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem || (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem_string_hash)
#define maketyped__hasitem_string_len_hash(tp_hasitem_string_len_hash) ((tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__trygetitem_string_len_hash : (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__bounditem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash : (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem ? &tdefault__hasitem_string_len_hash__with__hasitem : (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem_string_hash ? &tdefault__hasitem_string_len_hash__with__hasitem_string_hash : &tdefault__hasitem_string_len_hash)

/* tp_seq->tp_delitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__delitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index__and__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index__and__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__delitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index__and__delitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index__and__delitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_string_hash(DeeObject *self, DeeObject *index);
#define isusrtype__delitem(tp_delitem) ((tp_delitem) == &usrtype__delitem)
#define isdefault__delitem(tp_delitem) ((tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_len_hash || (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_hash || (tp_delitem) == &default__delitem__with__delitem_index || (tp_delitem) == &default__delitem__with__delitem_string_len_hash || (tp_delitem) == &default__delitem__with__delitem_string_hash)
#define maketyped__delitem(tp_delitem) ((tp_delitem) == &usrtype__delitem ? &tusrtype__delitem : (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_len_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash : (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_hash : (tp_delitem) == &default__delitem__with__delitem_index ? &tdefault__delitem__with__delitem_index : (tp_delitem) == &default__delitem__with__delitem_string_len_hash ? &tdefault__delitem__with__delitem_string_len_hash : (tp_delitem) == &default__delitem__with__delitem_string_hash ? &tdefault__delitem__with__delitem_string_hash : &tdefault__delitem)

/* tp_seq->tp_delitem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delitem_index__with__delitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__delitem_index__with__delitem(DeeObject *self, size_t index);
#define isdefault__delitem_index(tp_delitem_index) ((tp_delitem_index) == &default__delitem_index__with__delitem)
#define maketyped__delitem_index(tp_delitem_index) ((tp_delitem_index) == &default__delitem_index__with__delitem ? &tdefault__delitem_index__with__delitem : &tdefault__delitem_index)

/* tp_seq->tp_delitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem_string_hash__with__delitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem_string_hash__with__delitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__delitem_string_hash(tp_delitem_string_hash) ((tp_delitem_string_hash) == &default__delitem_string_hash__with__delitem)
#define maketyped__delitem_string_hash(tp_delitem_string_hash) ((tp_delitem_string_hash) == &default__delitem_string_hash__with__delitem ? &tdefault__delitem_string_hash__with__delitem : &tdefault__delitem_string_hash)

/* tp_seq->tp_delitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem_string_len_hash__with__delitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem_string_len_hash__with__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem_string_len_hash__with__delitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem_string_len_hash__with__delitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__delitem_string_len_hash(tp_delitem_string_len_hash) ((tp_delitem_string_len_hash) == &default__delitem_string_len_hash__with__delitem || (tp_delitem_string_len_hash) == &default__delitem_string_len_hash__with__delitem_string_hash)
#define maketyped__delitem_string_len_hash(tp_delitem_string_len_hash) ((tp_delitem_string_len_hash) == &default__delitem_string_len_hash__with__delitem ? &tdefault__delitem_string_len_hash__with__delitem : (tp_delitem_string_len_hash) == &default__delitem_string_len_hash__with__delitem_string_hash ? &tdefault__delitem_string_len_hash__with__delitem_string_hash : &tdefault__delitem_string_len_hash)

/* tp_seq->tp_setitem */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tusrtype__setitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index__and__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index__and__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL usrtype__setitem(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index__and__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index__and__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value);
#define isusrtype__setitem(tp_setitem) ((tp_setitem) == &usrtype__setitem)
#define isdefault__setitem(tp_setitem) ((tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_len_hash || (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_hash || (tp_setitem) == &default__setitem__with__setitem_index || (tp_setitem) == &default__setitem__with__setitem_string_len_hash || (tp_setitem) == &default__setitem__with__setitem_string_hash)
#define maketyped__setitem(tp_setitem) ((tp_setitem) == &usrtype__setitem ? &tusrtype__setitem : (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_len_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash : (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_hash : (tp_setitem) == &default__setitem__with__setitem_index ? &tdefault__setitem__with__setitem_index : (tp_setitem) == &default__setitem__with__setitem_string_len_hash ? &tdefault__setitem__with__setitem_string_len_hash : (tp_setitem) == &default__setitem__with__setitem_string_hash ? &tdefault__setitem__with__setitem_string_hash : &tdefault__setitem)

/* tp_seq->tp_setitem_index */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__setitem_index__with__setitem(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__setitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__setitem_index__with__setitem(DeeObject *self, size_t index, DeeObject *value);
#define isdefault__setitem_index(tp_setitem_index) ((tp_setitem_index) == &default__setitem_index__with__setitem)
#define maketyped__setitem_index(tp_setitem_index) ((tp_setitem_index) == &default__setitem_index__with__setitem ? &tdefault__setitem_index__with__setitem : &tdefault__setitem_index)

/* tp_seq->tp_setitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL tdefault__setitem_string_hash__with__setitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL tdefault__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__setitem_string_hash__with__setitem(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value);
#define isdefault__setitem_string_hash(tp_setitem_string_hash) ((tp_setitem_string_hash) == &default__setitem_string_hash__with__setitem)
#define maketyped__setitem_string_hash(tp_setitem_string_hash) ((tp_setitem_string_hash) == &default__setitem_string_hash__with__setitem ? &tdefault__setitem_string_hash__with__setitem : &tdefault__setitem_string_hash)

/* tp_seq->tp_setitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__setitem_string_len_hash__with__setitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__setitem_string_len_hash__with__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__setitem_string_len_hash__with__setitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__setitem_string_len_hash__with__setitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
#define isdefault__setitem_string_len_hash(tp_setitem_string_len_hash) ((tp_setitem_string_len_hash) == &default__setitem_string_len_hash__with__setitem || (tp_setitem_string_len_hash) == &default__setitem_string_len_hash__with__setitem_string_hash)
#define maketyped__setitem_string_len_hash(tp_setitem_string_len_hash) ((tp_setitem_string_len_hash) == &default__setitem_string_len_hash__with__setitem ? &tdefault__setitem_string_len_hash__with__setitem : (tp_setitem_string_len_hash) == &default__setitem_string_len_hash__with__setitem_string_hash ? &tdefault__setitem_string_len_hash__with__setitem_string_hash : &tdefault__setitem_string_len_hash)

/* tp_seq->tp_getrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tusrtype__getrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__getrange__with__getrange_index__and__getrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__getrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL usrtype__getrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__getrange__with__getrange_index__and__getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
#define isusrtype__getrange(tp_getrange) ((tp_getrange) == &usrtype__getrange)
#define isdefault__getrange(tp_getrange) ((tp_getrange) == &default__getrange__with__getrange_index__and__getrange_index_n)
#define maketyped__getrange(tp_getrange) ((tp_getrange) == &usrtype__getrange ? &tusrtype__getrange : (tp_getrange) == &default__getrange__with__getrange_index__and__getrange_index_n ? &tdefault__getrange__with__getrange_index__and__getrange_index_n : &tdefault__getrange)

/* tp_seq->tp_getrange_index */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getrange_index__with__getrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getrange_index__with__getrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define isdefault__getrange_index(tp_getrange_index) ((tp_getrange_index) == &default__getrange_index__with__getrange)
#define maketyped__getrange_index(tp_getrange_index) ((tp_getrange_index) == &default__getrange_index__with__getrange ? &tdefault__getrange_index__with__getrange : &tdefault__getrange_index)

/* tp_seq->tp_getrange_index_n */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getrange_index_n__with__getrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getrange_index_n__with__getrange(DeeObject *self, Dee_ssize_t start);
#define isdefault__getrange_index_n(tp_getrange_index_n) ((tp_getrange_index_n) == &default__getrange_index_n__with__getrange)
#define maketyped__getrange_index_n(tp_getrange_index_n) ((tp_getrange_index_n) == &default__getrange_index_n__with__getrange ? &tdefault__getrange_index_n__with__getrange : &tdefault__getrange_index_n)

/* tp_seq->tp_delrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tusrtype__delrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__delrange__with__delrange_index__and__delrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__delrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL usrtype__delrange(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__delrange__with__delrange_index__and__delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
#define isusrtype__delrange(tp_delrange) ((tp_delrange) == &usrtype__delrange)
#define isdefault__delrange(tp_delrange) ((tp_delrange) == &default__delrange__with__delrange_index__and__delrange_index_n)
#define maketyped__delrange(tp_delrange) ((tp_delrange) == &usrtype__delrange ? &tusrtype__delrange : (tp_delrange) == &default__delrange__with__delrange_index__and__delrange_index_n ? &tdefault__delrange__with__delrange_index__and__delrange_index_n : &tdefault__delrange)

/* tp_seq->tp_delrange_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delrange_index__with__delrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL default__delrange_index__with__delrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
#define isdefault__delrange_index(tp_delrange_index) ((tp_delrange_index) == &default__delrange_index__with__delrange)
#define maketyped__delrange_index(tp_delrange_index) ((tp_delrange_index) == &default__delrange_index__with__delrange ? &tdefault__delrange_index__with__delrange : &tdefault__delrange_index)

/* tp_seq->tp_delrange_index_n */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delrange_index_n__with__delrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__delrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int DCALL default__delrange_index_n__with__delrange(DeeObject *self, Dee_ssize_t start);
#define isdefault__delrange_index_n(tp_delrange_index_n) ((tp_delrange_index_n) == &default__delrange_index_n__with__delrange)
#define maketyped__delrange_index_n(tp_delrange_index_n) ((tp_delrange_index_n) == &default__delrange_index_n__with__delrange ? &tdefault__delrange_index_n__with__delrange : &tdefault__delrange_index_n)

/* tp_seq->tp_setrange */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tusrtype__setrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tdefault__setrange__with__setrange_index__and__setrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tdefault__setrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL usrtype__setrange(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__setrange__with__setrange_index__and__setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
#define isusrtype__setrange(tp_setrange) ((tp_setrange) == &usrtype__setrange)
#define isdefault__setrange(tp_setrange) ((tp_setrange) == &default__setrange__with__setrange_index__and__setrange_index_n)
#define maketyped__setrange(tp_setrange) ((tp_setrange) == &usrtype__setrange ? &tusrtype__setrange : (tp_setrange) == &default__setrange__with__setrange_index__and__setrange_index_n ? &tdefault__setrange__with__setrange_index__and__setrange_index_n : &tdefault__setrange)

/* tp_seq->tp_setrange_index */
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__setrange_index__with__setrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL tdefault__setrange_index(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL default__setrange_index__with__setrange(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
#define isdefault__setrange_index(tp_setrange_index) ((tp_setrange_index) == &default__setrange_index__with__setrange)
#define maketyped__setrange_index(tp_setrange_index) ((tp_setrange_index) == &default__setrange_index__with__setrange ? &tdefault__setrange_index__with__setrange : &tdefault__setrange_index)

/* tp_seq->tp_setrange_index_n */
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__setrange_index_n__with__setrange(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL tdefault__setrange_index_n(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL default__setrange_index_n__with__setrange(DeeObject *self, Dee_ssize_t start, DeeObject *values);
#define isdefault__setrange_index_n(tp_setrange_index_n) ((tp_setrange_index_n) == &default__setrange_index_n__with__setrange)
#define maketyped__setrange_index_n(tp_setrange_index_n) ((tp_setrange_index_n) == &default__setrange_index_n__with__setrange ? &tdefault__setrange_index_n__with__setrange : &tdefault__setrange_index_n)

/* tp_math->tp_inv */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__inv(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__inv(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__inv(DeeObject *self);
#define isusrtype__inv(tp_inv) ((tp_inv) == &usrtype__inv)
#define maketyped__inv(tp_inv) ((tp_inv) == &usrtype__inv ? &tusrtype__inv : &tdefault__inv)

/* tp_math->tp_pos */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__pos(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__pos(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__pos(DeeObject *self);
#define isusrtype__pos(tp_pos) ((tp_pos) == &usrtype__pos)
#define maketyped__pos(tp_pos) ((tp_pos) == &usrtype__pos ? &tusrtype__pos : &tdefault__pos)

/* tp_math->tp_neg */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__neg(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__neg(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__neg(DeeObject *self);
#define isusrtype__neg(tp_neg) ((tp_neg) == &usrtype__neg)
#define maketyped__neg(tp_neg) ((tp_neg) == &usrtype__neg ? &tusrtype__neg : &tdefault__neg)

/* tp_math->tp_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__add(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__add(tp_add) ((tp_add) == &usrtype__add)
#define maketyped__add(tp_add) ((tp_add) == &usrtype__add ? &tusrtype__add : &tdefault__add)

/* tp_math->tp_inplace_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_add__with__add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_add__with__add(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add)
#define isdefault__inplace_add(tp_inplace_add) ((tp_inplace_add) == &default__inplace_add__with__add)
#define maketyped__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add ? &tusrtype__inplace_add : (tp_inplace_add) == &default__inplace_add__with__add ? &tdefault__inplace_add__with__add : &tdefault__inplace_add)

/* tp_math->tp_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__sub(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__sub(tp_sub) ((tp_sub) == &usrtype__sub)
#define maketyped__sub(tp_sub) ((tp_sub) == &usrtype__sub ? &tusrtype__sub : &tdefault__sub)

/* tp_math->tp_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_sub__with__sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_sub__with__sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub)
#define isdefault__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &default__inplace_sub__with__sub)
#define maketyped__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub ? &tusrtype__inplace_sub : (tp_inplace_sub) == &default__inplace_sub__with__sub ? &tdefault__inplace_sub__with__sub : &tdefault__inplace_sub)

/* tp_math->tp_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mul(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mul(tp_mul) ((tp_mul) == &usrtype__mul)
#define maketyped__mul(tp_mul) ((tp_mul) == &usrtype__mul ? &tusrtype__mul : &tdefault__mul)

/* tp_math->tp_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mul__with__mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mul__with__mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul)
#define isdefault__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &default__inplace_mul__with__mul)
#define maketyped__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul ? &tusrtype__inplace_mul : (tp_inplace_mul) == &default__inplace_mul__with__mul ? &tdefault__inplace_mul__with__mul : &tdefault__inplace_mul)

/* tp_math->tp_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__div(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__div(tp_div) ((tp_div) == &usrtype__div)
#define maketyped__div(tp_div) ((tp_div) == &usrtype__div ? &tusrtype__div : &tdefault__div)

/* tp_math->tp_inplace_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_div__with__div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_div__with__div(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div)
#define isdefault__inplace_div(tp_inplace_div) ((tp_inplace_div) == &default__inplace_div__with__div)
#define maketyped__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div ? &tusrtype__inplace_div : (tp_inplace_div) == &default__inplace_div__with__div ? &tdefault__inplace_div__with__div : &tdefault__inplace_div)

/* tp_math->tp_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mod(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mod(tp_mod) ((tp_mod) == &usrtype__mod)
#define maketyped__mod(tp_mod) ((tp_mod) == &usrtype__mod ? &tusrtype__mod : &tdefault__mod)

/* tp_math->tp_inplace_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mod__with__mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mod__with__mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod)
#define isdefault__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &default__inplace_mod__with__mod)
#define maketyped__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod ? &tusrtype__inplace_mod : (tp_inplace_mod) == &default__inplace_mod__with__mod ? &tdefault__inplace_mod__with__mod : &tdefault__inplace_mod)

/* tp_math->tp_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shl(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shl(tp_shl) ((tp_shl) == &usrtype__shl)
#define maketyped__shl(tp_shl) ((tp_shl) == &usrtype__shl ? &tusrtype__shl : &tdefault__shl)

/* tp_math->tp_inplace_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shl__with__shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shl__with__shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl)
#define isdefault__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &default__inplace_shl__with__shl)
#define maketyped__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl ? &tusrtype__inplace_shl : (tp_inplace_shl) == &default__inplace_shl__with__shl ? &tdefault__inplace_shl__with__shl : &tdefault__inplace_shl)

/* tp_math->tp_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shr(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shr(tp_shr) ((tp_shr) == &usrtype__shr)
#define maketyped__shr(tp_shr) ((tp_shr) == &usrtype__shr ? &tusrtype__shr : &tdefault__shr)

/* tp_math->tp_inplace_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shr__with__shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shr__with__shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr)
#define isdefault__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &default__inplace_shr__with__shr)
#define maketyped__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr ? &tusrtype__inplace_shr : (tp_inplace_shr) == &default__inplace_shr__with__shr ? &tdefault__inplace_shr__with__shr : &tdefault__inplace_shr)

/* tp_math->tp_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__and(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__and(tp_and) ((tp_and) == &usrtype__and)
#define maketyped__and(tp_and) ((tp_and) == &usrtype__and ? &tusrtype__and : &tdefault__and)

/* tp_math->tp_inplace_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_and__with__and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_and__with__and(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and)
#define isdefault__inplace_and(tp_inplace_and) ((tp_inplace_and) == &default__inplace_and__with__and)
#define maketyped__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and ? &tusrtype__inplace_and : (tp_inplace_and) == &default__inplace_and__with__and ? &tdefault__inplace_and__with__and : &tdefault__inplace_and)

/* tp_math->tp_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__or(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__or(tp_or) ((tp_or) == &usrtype__or)
#define maketyped__or(tp_or) ((tp_or) == &usrtype__or ? &tusrtype__or : &tdefault__or)

/* tp_math->tp_inplace_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_or__with__or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_or__with__or(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or)
#define isdefault__inplace_or(tp_inplace_or) ((tp_inplace_or) == &default__inplace_or__with__or)
#define maketyped__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or ? &tusrtype__inplace_or : (tp_inplace_or) == &default__inplace_or__with__or ? &tdefault__inplace_or__with__or : &tdefault__inplace_or)

/* tp_math->tp_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__xor(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__xor(tp_xor) ((tp_xor) == &usrtype__xor)
#define maketyped__xor(tp_xor) ((tp_xor) == &usrtype__xor ? &tusrtype__xor : &tdefault__xor)

/* tp_math->tp_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_xor__with__xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_xor__with__xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor)
#define isdefault__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &default__inplace_xor__with__xor)
#define maketyped__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor ? &tusrtype__inplace_xor : (tp_inplace_xor) == &default__inplace_xor__with__xor ? &tdefault__inplace_xor__with__xor : &tdefault__inplace_xor)

/* tp_math->tp_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__pow(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__pow(tp_pow) ((tp_pow) == &usrtype__pow)
#define maketyped__pow(tp_pow) ((tp_pow) == &usrtype__pow ? &tusrtype__pow : &tdefault__pow)

/* tp_math->tp_inplace_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_pow__with__pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_pow__with__pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow)
#define isdefault__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &default__inplace_pow__with__pow)
#define maketyped__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow ? &tusrtype__inplace_pow : (tp_inplace_pow) == &default__inplace_pow__with__pow ? &tdefault__inplace_pow__with__pow : &tdefault__inplace_pow)

/* tp_math->tp_inc */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__inc(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__inplace_add(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__add(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__inc(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__inplace_add(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__add(DeeObject **__restrict p_self);
#define isusrtype__inc(tp_inc) ((tp_inc) == &usrtype__inc)
#define isdefault__inc(tp_inc) ((tp_inc) == &default__inc__with__inplace_add || (tp_inc) == &default__inc__with__add)
#define maketyped__inc(tp_inc) ((tp_inc) == &usrtype__inc ? &tusrtype__inc : (tp_inc) == &default__inc__with__inplace_add ? &tdefault__inc__with__inplace_add : (tp_inc) == &default__inc__with__add ? &tdefault__inc__with__add : &tdefault__inc)

/* tp_math->tp_dec */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__dec(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__sub(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__dec(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__inplace_sub(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__sub(DeeObject **__restrict p_self);
#define isusrtype__dec(tp_dec) ((tp_dec) == &usrtype__dec)
#define isdefault__dec(tp_dec) ((tp_dec) == &default__dec__with__inplace_sub || (tp_dec) == &default__dec__with__sub)
#define maketyped__dec(tp_dec) ((tp_dec) == &usrtype__dec ? &tusrtype__dec : (tp_dec) == &default__dec__with__inplace_sub ? &tdefault__dec__with__inplace_sub : (tp_dec) == &default__dec__with__sub ? &tdefault__dec__with__sub : &tdefault__dec)

/* tp_with->tp_enter */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__enter(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__enter__with__leave (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__enter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__enter(DeeObject *__restrict self);
#define default__enter__with__leave (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__enter(tp_enter) ((tp_enter) == &usrtype__enter)
#define isdefault__enter(tp_enter) ((tp_enter) == &default__enter__with__leave)
#define maketyped__enter(tp_enter) ((tp_enter) == &usrtype__enter ? &tusrtype__enter : &tdefault__enter)

/* tp_with->tp_leave */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__leave(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__leave__with__enter (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__leave(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__leave(DeeObject *__restrict self);
#define default__leave__with__enter (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__leave(tp_leave) ((tp_leave) == &usrtype__leave)
#define isdefault__leave(tp_leave) ((tp_leave) == &default__leave__with__enter)
#define maketyped__leave(tp_leave) ((tp_leave) == &usrtype__leave ? &tusrtype__leave : &tdefault__leave)
/*[[[end]]]*/
/* clang-format on */


/* Backward compat */
#define instance_tassign             tusrtype__assign
#define instance_assign              usrtype__assign
#define instance_tmoveassign         tusrtype__move_assign
#define instance_moveassign          usrtype__move_assign
#define instance_tstr                tusrtype__str
#define instance_str                 usrtype__str
#define instance_trepr               tusrtype__repr
#define instance_repr                usrtype__repr
#define instance_tprint              tusrtype__print
#define instance_print               usrtype__print
#define instance_tprintrepr          tusrtype__printrepr
#define instance_printrepr           usrtype__printrepr
#define instance_tstr_by_print       tusrtype__str__by_print
#define instance_str_by_print        usrtype__str__by_print
#define instance_trepr_by_print      tusrtype__repr__by_printrepr
#define instance_repr_by_print       usrtype__repr__by_printrepr
#define instance_tprint_by_print     tusrtype__print__by_print
#define instance_print_by_print      usrtype__print__by_print
#define instance_tprintrepr_by_print tusrtype__printrepr__by_print
#define instance_printrepr_by_print  usrtype__printrepr__by_print
#define instance_tbool               tusrtype__bool
#define instance_bool                usrtype__bool
#define instance_tcall               tusrtype__call
#define instance_call                usrtype__call
#define instance_tcallkw             tusrtype__call_kw
#define instance_callkw              usrtype__call_kw
#define instance_tnext               tusrtype__iter_next
#define instance_next                usrtype__iter_next
#define instance_tint                tusrtype__int
#define instance_int                 usrtype__int
#define instance_tdouble             tusrtype__double
#define instance_double              usrtype__double
#define instance_tinv                tusrtype__inv
#define instance_inv                 usrtype__inv
#define instance_tpos                tusrtype__pos
#define instance_pos                 usrtype__pos
#define instance_tneg                tusrtype__neg
#define instance_neg                 usrtype__neg
#define instance_tadd                tusrtype__add
#define instance_add                 usrtype__add
#define instance_tsub                tusrtype__sub
#define instance_sub                 usrtype__sub
#define instance_tmul                tusrtype__mul
#define instance_mul                 usrtype__mul
#define instance_tdiv                tusrtype__div
#define instance_div                 usrtype__div
#define instance_tmod                tusrtype__mod
#define instance_mod                 usrtype__mod
#define instance_tshl                tusrtype__shl
#define instance_shl                 usrtype__shl
#define instance_tshr                tusrtype__shr
#define instance_shr                 usrtype__shr
#define instance_tand                tusrtype__and
#define instance_and                 usrtype__and
#define instance_tor                 tusrtype__or
#define instance_or                  usrtype__or
#define instance_txor                tusrtype__xor
#define instance_xor                 usrtype__xor
#define instance_tpow                tusrtype__pow
#define instance_pow                 usrtype__pow
#define instance_tinc                tusrtype__inc
#define instance_inc                 usrtype__inc
#define instance_tdec                tusrtype__dec
#define instance_dec                 usrtype__dec
#define instance_tiadd               tusrtype__inplace_add
#define instance_iadd                usrtype__inplace_add
#define instance_tisub               tusrtype__inplace_sub
#define instance_isub                usrtype__inplace_sub
#define instance_timul               tusrtype__inplace_mul
#define instance_imul                usrtype__inplace_mul
#define instance_tidiv               tusrtype__inplace_div
#define instance_idiv                usrtype__inplace_div
#define instance_timod               tusrtype__inplace_mod
#define instance_imod                usrtype__inplace_mod
#define instance_tishl               tusrtype__inplace_shl
#define instance_ishl                usrtype__inplace_shl
#define instance_tishr               tusrtype__inplace_shr
#define instance_ishr                usrtype__inplace_shr
#define instance_tiand               tusrtype__inplace_and
#define instance_iand                usrtype__inplace_and
#define instance_tior                tusrtype__inplace_or
#define instance_ior                 usrtype__inplace_or
#define instance_tixor               tusrtype__inplace_xor
#define instance_ixor                usrtype__inplace_xor
#define instance_tipow               tusrtype__inplace_pow
#define instance_ipow                usrtype__inplace_pow
#define instance_thash               tusrtype__hash
#define instance_hash                usrtype__hash
#define instance_teq                 tusrtype__eq
#define instance_eq                  usrtype__eq
#define instance_tne                 tusrtype__ne
#define instance_ne                  usrtype__ne
#define instance_tlo                 tusrtype__lo
#define instance_lo                  usrtype__lo
#define instance_tle                 tusrtype__le
#define instance_le                  usrtype__le
#define instance_tgr                 tusrtype__gr
#define instance_gr                  usrtype__gr
#define instance_tge                 tusrtype__ge
#define instance_ge                  usrtype__ge
#define instance_titer               tusrtype__iter
#define instance_iter                usrtype__iter
#define instance_tsize               tusrtype__sizeob
#define instance_size                usrtype__sizeob
#define instance_tcontains           tusrtype__contains
#define instance_contains            usrtype__contains
#define instance_tgetitem            tusrtype__getitem
#define instance_getitem             usrtype__getitem
#define instance_tdelitem            tusrtype__delitem
#define instance_delitem             usrtype__delitem
#define instance_tsetitem            tusrtype__setitem
#define instance_setitem             usrtype__setitem
#define instance_tgetrange           tusrtype__getrange
#define instance_getrange            usrtype__getrange
#define instance_tdelrange           tusrtype__delrange
#define instance_delrange            usrtype__delrange
#define instance_tsetrange           tusrtype__setrange
#define instance_setrange            usrtype__setrange
#define instance_tenter              tusrtype__enter
#define instance_enter               usrtype__enter
#define instance_tleave              tusrtype__leave
#define instance_leave               usrtype__leave

#endif /* CONFIG_BUILDING_DEEMON */

#else /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

/* Forward compat */
#define DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_table, tp_field, require) \
	(((self)->tp_table && (self)->tp_table->tp_field) ? (self)->tp_table->tp_field : require(self) ? (self)->tp_table->tp_field : NULL)
#define DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_1(self, tp_field, require) \
	((self)->tp_field ? (self)->tp_field : require(self) ? (self)->tp_field : NULL)

#define _DeeType_RequireSupportedNativeOperator__iter(self)             DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_iter, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__foreach(self)          DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_foreach, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__foreach_pair(self)     DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_foreach_pair, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__getitem(self)          DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_getitem, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__getitem_index(self)    DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_getitem_index, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__trygetitem(self)       DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_trygetitem, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__trygetitem_index(self) DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_seq, tp_trygetitem_index, DeeType_InheritGetItem)
#define _DeeType_RequireSupportedNativeOperator__iter_next(self)        DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_1(self, tp_iter_next, DeeType_InheritIterNext)
#define _DeeType_RequireSupportedNativeOperator__nextkey(self)          DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_iterator, tp_nextkey, DeeType_InheritIterNext)
#define _DeeType_RequireSupportedNativeOperator__nextvalue(self)        DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_iterator, tp_nextvalue, DeeType_InheritIterNext)
#define _DeeType_RequireSupportedNativeOperator__nextpair(self)         DEE_PRIVATE_DeeType_RequireSupportedNativeOperator_2(self, tp_iterator, tp_nextpair, DeeType_InheritIterNext)

#define _DeeType_RequireSupportedNativeOperator(self, name) _DeeType_RequireSupportedNativeOperator__##name(self)
#define DeeType_RequireSupportedNativeOperator(self, name) _DeeType_RequireSupportedNativeOperator(self, name)

#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

DECL_END

#endif /* !GUARD_DEEMON_OPERATOR_HINTS_H */
