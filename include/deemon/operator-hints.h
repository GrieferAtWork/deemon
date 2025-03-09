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
/**/

#include "types.h"
/**/

#include "none-operator.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* int32_t, int64_t */

DECL_BEGIN

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
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getitem_index_fast_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_getitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getitem_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_trygetitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *DeeNO_trygetitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_trygetitem_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
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
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_add_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_sub_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_sub_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_mul_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_mul_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_div_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_div_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_mod_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_mod_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_shl_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_shl_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_shr_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_shr_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_and_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_and_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_or_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_or_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_xor_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_xor_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_pow_t)(DeeObject *lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_inplace_pow_t)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_inc_t)(DREF DeeObject **__restrict p_self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_dec_t)(DREF DeeObject **__restrict p_self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_enter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *DeeNO_leave_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getattr_t)(DeeObject *self, DeeObject *attr);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getattr_string_hash_t)(DeeObject *self, char const *attr, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *DeeNO_getattr_string_len_hash_t)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_boundattr_t)(DeeObject *self, DeeObject *attr);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_boundattr_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_boundattr_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasattr_t)(DeeObject *self, DeeObject *attr);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasattr_string_hash_t)(DeeObject *self, char const *key, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_hasattr_string_len_hash_t)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delattr_t)(DeeObject *self, DeeObject *attr);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delattr_string_hash_t)(DeeObject *self, char const *attr, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *DeeNO_delattr_string_len_hash_t)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *DeeNO_setattr_t)(DeeObject *self, DeeObject *attr, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 4)) int (DCALL *DeeNO_setattr_string_hash_t)(DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *DeeNO_setattr_string_len_hash_t)(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
/*[[[end]]]*/
/* clang-format on */


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
	Dee_TNO_getitem_index_fast,
	Dee_TNO_getitem,
	Dee_TNO_getitem_index,
	Dee_TNO_getitem_string_hash,
	Dee_TNO_getitem_string_len_hash,
	Dee_TNO_trygetitem,
	Dee_TNO_trygetitem_index,
	Dee_TNO_trygetitem_string_hash,
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
	Dee_TNO_getattr,
	Dee_TNO_getattr_string_hash,
	Dee_TNO_getattr_string_len_hash,
	Dee_TNO_boundattr,
	Dee_TNO_boundattr_string_hash,
	Dee_TNO_boundattr_string_len_hash,
	Dee_TNO_hasattr,
	Dee_TNO_hasattr_string_hash,
	Dee_TNO_hasattr_string_len_hash,
	Dee_TNO_delattr,
	Dee_TNO_delattr_string_hash,
	Dee_TNO_delattr_string_len_hash,
	Dee_TNO_setattr,
	Dee_TNO_setattr_string_hash,
	Dee_TNO_setattr_string_len_hash,
/*[[[end]]]*/
	/* clang-format on */
	Dee_TNO_COUNT
};



/* Same as `DeeType_GetNativeOperatorWithoutInherit', but actually also does the
 * operator inherit part (meaning that this is the low-level* master-function
 * that's called when you invoke one of the standard operators whose callback
 * is currently set to "NULL" within its relevant type)
 * [*] The actual master function is `DeeType_GetNativeOperator', but that
 *     one only adds coalesce to `DeeType_GetNativeOperatorUnsupported()' */
DFUNDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutUnsupported)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Same as `DeeType_GetNativeOperatorWithoutUnsupported()', but never returns NULL
 * (for any operator linked against a deemon user-code ID (e.g. "OPERATOR_ITER"))
 * and instead returns special implementations for each operator that simply call
 * `err_unimplemented_operator()' with the relevant arguments, before returning
 * whatever is indicative of an error in the context of the native operator. */
DFUNDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id);


/* Convenience wrapper for `DeeType_GetNativeOperator' that
 * already casts the function pointer into the proper type.
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
#define DeeType_HasNativeOperator(self, name) (DeeType_RequireSupportedNativeOperator(self, name) != NULL)


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
#define _Dee_TNO_PATH_getitem_index_fast(_)         PATH2(_, tp_seq, tp_getitem_index_fast)
#define _Dee_TNO_PATH_getitem(_)                    PATH2(_, tp_seq, tp_getitem)
#define _Dee_TNO_PATH_getitem_index(_)              PATH2(_, tp_seq, tp_getitem_index)
#define _Dee_TNO_PATH_getitem_string_hash(_)        PATH2(_, tp_seq, tp_getitem_string_hash)
#define _Dee_TNO_PATH_getitem_string_len_hash(_)    PATH2(_, tp_seq, tp_getitem_string_len_hash)
#define _Dee_TNO_PATH_trygetitem(_)                 PATH2(_, tp_seq, tp_trygetitem)
#define _Dee_TNO_PATH_trygetitem_index(_)           PATH2(_, tp_seq, tp_trygetitem_index)
#define _Dee_TNO_PATH_trygetitem_string_hash(_)     PATH2(_, tp_seq, tp_trygetitem_string_hash)
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
#define _Dee_TNO_PATH_getattr(_)                    PATH2(_, tp_attr, tp_getattr)
#define _Dee_TNO_PATH_getattr_string_hash(_)        PATH2(_, tp_attr, tp_getattr_string_hash)
#define _Dee_TNO_PATH_getattr_string_len_hash(_)    PATH2(_, tp_attr, tp_getattr_string_len_hash)
#define _Dee_TNO_PATH_boundattr(_)                  PATH2(_, tp_attr, tp_boundattr)
#define _Dee_TNO_PATH_boundattr_string_hash(_)      PATH2(_, tp_attr, tp_boundattr_string_hash)
#define _Dee_TNO_PATH_boundattr_string_len_hash(_)  PATH2(_, tp_attr, tp_boundattr_string_len_hash)
#define _Dee_TNO_PATH_hasattr(_)                    PATH2(_, tp_attr, tp_hasattr)
#define _Dee_TNO_PATH_hasattr_string_hash(_)        PATH2(_, tp_attr, tp_hasattr_string_hash)
#define _Dee_TNO_PATH_hasattr_string_len_hash(_)    PATH2(_, tp_attr, tp_hasattr_string_len_hash)
#define _Dee_TNO_PATH_delattr(_)                    PATH2(_, tp_attr, tp_delattr)
#define _Dee_TNO_PATH_delattr_string_hash(_)        PATH2(_, tp_attr, tp_delattr_string_hash)
#define _Dee_TNO_PATH_delattr_string_len_hash(_)    PATH2(_, tp_attr, tp_delattr_string_len_hash)
#define _Dee_TNO_PATH_setattr(_)                    PATH2(_, tp_attr, tp_setattr)
#define _Dee_TNO_PATH_setattr_string_hash(_)        PATH2(_, tp_attr, tp_setattr_string_hash)
#define _Dee_TNO_PATH_setattr_string_len_hash(_)    PATH2(_, tp_attr, tp_setattr_string_len_hash)
/*[[[end]]]*/
/* clang-format on */


#if defined(CONFIG_BUILDING_DEEMON) || defined(__DEEMON__)
/* Return an actual, user-defined operator "id"
 * (*NOT* allowing stuff like `default__size__with__sizeob'
 * or `default__seq_operator_size__with__seq_operator_sizeob')
 * Also never returns `DeeType_GetNativeOperatorOOM()' or
 * `DeeType_GetNativeOperatorUnsupported()' */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutDefaults)(DeeTypeObject const *__restrict self, enum Dee_tno_id id);

/* Wrapper around `DeeType_SelectMissingNativeOperator' that checks if the
 * operator is already defined, and if not: see if can be substituted via
 * some other set of native operators (in which case: do that substitution
 * and then return the operator's function pointer) */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutHints)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

/* Same as `DeeType_GetNativeOperatorWithoutHints', but also load operators
 * from method hints (though don't inherit them from base-types, yet). */
INTDEF WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutInherit)(DeeTypeObject *__restrict self, enum Dee_tno_id id);

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

/* Returns the impl for "id" that calls `err_unimplemented_operator()'.
 * Returns "NULL" if "id" doesn't define a user-code ID */
INTDEF Dee_funptr_t tpconst _DeeType_GetNativeOperatorUnsupported[Dee_TNO_COUNT];
#define DeeType_GetNativeOperatorUnsupported(id) _DeeType_GetNativeOperatorUnsupported[id]


/* Smallest unsigned integer type still able to represent
 * all `enum Dee_tno_id' values (including `Dee_TNO_COUNT') */
typedef uint8_t Dee_compact_tno_id_t;

/* Returns the ID of some native operator that should always be present
 * (as an impl `!= DeeType_GetNativeOperatorUnsupported(return)') in
 * types that are considered to be implementing `op'.
 * Returns `Dee_TNO_COUNT' when the operator does not have a native variant
 * The caller must ensure that `op < Dee_OPERATOR_USERCOUNT' */
INTDEF Dee_compact_tno_id_t const _DeeType_GetTnoOfOperator[Dee_OPERATOR_USERCOUNT];
#define DeeType_GetTnoOfOperator(op) ((enum Dee_tno_id)_DeeType_GetTnoOfOperator[op])

/* The inverse of `DeeType_GetTnoOfOperator' */
INTDEF Dee_operator_t const _DeeType_GetOperatorOfTno[Dee_TNO_COUNT];
#define DeeType_GetOperatorOfTno(id) _DeeType_GetOperatorOfTno[id]


/* Find the type where native operator "id" has been inherited from.
 * This function correctly handles:
 * - when "id" is implemented as an alias for another operator (in which case it
 *   returns the origin of the operator's first dependency, or "self" if there are
 *   no dependencies)
 * - when "id" is implemented using a method hint, return the origin of that hint
 * - when "id" is not implemented at all, return "NULL" */
INTDEF WUNUSED NONNULL((1)) DeeTypeObject *
(DCALL DeeType_GetNativeOperatorOrigin)(DeeTypeObject *__restrict self, enum Dee_tno_id id);





/************************************************************************/
/* OPERATOR DEFAULT IMPLS                                               */
/************************************************************************/

/* Equivalence callbacks for native operators.
 *
 * These equivalences are only tangentially related to method hints,
 * and are applicable to *any* type of object, but only within an
 * object itself (iow: not when it comes to inherited operators). */

/* clang-format off */
/*[[[deemon (printNativeOperatorHintDecls from "...src.deemon.method-hints.method-hints")();]]]*/
/* tp_init.tp_assign */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__assign__with__ASSIGN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__assign__with__ASSIGN(DeeObject *self, DeeObject *value);
#define isusrtype__assign(tp_assign) ((tp_assign) == &usrtype__assign__with__ASSIGN)
#define maketyped__assign(tp_assign) ((tp_assign) == &usrtype__assign__with__ASSIGN ? &tusrtype__assign__with__ASSIGN : &tdefault__assign)

/* tp_init.tp_move_assign */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__move_assign__with__MOVEASSIGN(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__move_assign__with__assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__move_assign(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__move_assign__with__MOVEASSIGN(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__move_assign__with__assign(DeeObject *self, DeeObject *value);
#define isusrtype__move_assign(tp_move_assign) ((tp_move_assign) == &usrtype__move_assign__with__MOVEASSIGN)
#define isdefault__move_assign(tp_move_assign) ((tp_move_assign) == &default__move_assign__with__assign)
#define maketyped__move_assign(tp_move_assign) ((tp_move_assign) == &usrtype__move_assign__with__MOVEASSIGN ? &tusrtype__move_assign__with__MOVEASSIGN : (tp_move_assign) == &default__move_assign__with__assign ? &tdefault__move_assign__with__assign : &tdefault__move_assign)

/* tp_cast.tp_str */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str__with__STR(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str__with__PRINT(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__str__with__print(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__str(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str__with__STR(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str__with__PRINT(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__str__with__print(DeeObject *__restrict self);
#define isusrtype__str(tp_str) ((tp_str) == &usrtype__str__with__STR || (tp_str) == &usrtype__str__with__PRINT)
#define isdefault__str(tp_str) ((tp_str) == &default__str__with__print)
#define maketyped__str(tp_str) ((tp_str) == &usrtype__str__with__STR ? &tusrtype__str__with__STR : (tp_str) == &usrtype__str__with__PRINT ? &tusrtype__str__with__PRINT : (tp_str) == &default__str__with__print ? &tdefault__str__with__print : &tdefault__str)

/* tp_cast.tp_print */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print__with__STR(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print__with__PRINT(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__print__with__str(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print__with__STR(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print__with__PRINT(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__print__with__str(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__print(tp_print) ((tp_print) == &usrtype__print__with__STR || (tp_print) == &usrtype__print__with__PRINT)
#define isdefault__print(tp_print) ((tp_print) == &default__print__with__str)
#define maketyped__print(tp_print) ((tp_print) == &usrtype__print__with__STR ? &tusrtype__print__with__STR : (tp_print) == &usrtype__print__with__PRINT ? &tusrtype__print__with__PRINT : (tp_print) == &default__print__with__str ? &tdefault__print__with__str : &tdefault__print)

/* tp_cast.tp_repr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr__with__REPR(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr__with__PRINTREPR(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__repr__with__printrepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__repr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr__with__REPR(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr__with__PRINTREPR(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__repr__with__printrepr(DeeObject *__restrict self);
#define isusrtype__repr(tp_repr) ((tp_repr) == &usrtype__repr__with__REPR || (tp_repr) == &usrtype__repr__with__PRINTREPR)
#define isdefault__repr(tp_repr) ((tp_repr) == &default__repr__with__printrepr)
#define maketyped__repr(tp_repr) ((tp_repr) == &usrtype__repr__with__REPR ? &tusrtype__repr__with__REPR : (tp_repr) == &usrtype__repr__with__PRINTREPR ? &tusrtype__repr__with__PRINTREPR : (tp_repr) == &default__repr__with__printrepr ? &tdefault__repr__with__printrepr : &tdefault__repr)

/* tp_cast.tp_printrepr */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr__with__REPR(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr__with__PRINTREPR(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__printrepr__with__repr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr__with__REPR(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr__with__PRINTREPR(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__printrepr__with__repr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr__with__REPR || (tp_printrepr) == &usrtype__printrepr__with__PRINTREPR)
#define isdefault__printrepr(tp_printrepr) ((tp_printrepr) == &default__printrepr__with__repr)
#define maketyped__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr__with__REPR ? &tusrtype__printrepr__with__REPR : (tp_printrepr) == &usrtype__printrepr__with__PRINTREPR ? &tusrtype__printrepr__with__PRINTREPR : (tp_printrepr) == &default__printrepr__with__repr ? &tdefault__printrepr__with__repr : &tdefault__printrepr)

/* tp_cast.tp_bool */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__bool__with__BOOL(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bool(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__bool__with__BOOL(DeeObject *__restrict self);
#define isusrtype__bool(tp_bool) ((tp_bool) == &usrtype__bool__with__BOOL)
#define maketyped__bool(tp_bool) ((tp_bool) == &usrtype__bool__with__BOOL ? &tusrtype__bool__with__BOOL : &tdefault__bool)

/* tp_call */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call__with__CALL(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call__with__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call__with__CALL(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call__with__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv);
#define isusrtype__call(tp_call) ((tp_call) == &usrtype__call__with__CALL)
#define isdefault__call(tp_call) ((tp_call) == &default__call__with__call_kw)
#define maketyped__call(tp_call) ((tp_call) == &usrtype__call__with__CALL ? &tusrtype__call__with__CALL : (tp_call) == &default__call__with__call_kw ? &tdefault__call__with__call_kw : &tdefault__call)

/* tp_call_kw */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call_kw__with__CALL(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call_kw__with__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call_kw__with__CALL(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call_kw__with__call(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define isusrtype__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw__with__CALL)
#define isdefault__call_kw(tp_call_kw) ((tp_call_kw) == &default__call_kw__with__call)
#define maketyped__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw__with__CALL ? &tusrtype__call_kw__with__CALL : (tp_call_kw) == &default__call_kw__with__call ? &tdefault__call_kw__with__call : &tdefault__call_kw)

/* tp_iter_next */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter_next__with__ITERNEXT(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_next__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter_next__with__ITERNEXT(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_next__with__nextpair(DeeObject *__restrict self);
#define isusrtype__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next__with__ITERNEXT)
#define isdefault__iter_next(tp_iter_next) ((tp_iter_next) == &default__iter_next__with__nextpair)
#define maketyped__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next__with__ITERNEXT ? &tusrtype__iter_next__with__ITERNEXT : (tp_iter_next) == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next)

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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__int__with__INT(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int64(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int32(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__double(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__int__with__INT(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int64(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int32(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__double(DeeObject *__restrict self);
#define isusrtype__int(tp_int) ((tp_int) == &usrtype__int__with__INT)
#define isdefault__int(tp_int) ((tp_int) == &default__int__with__int64 || (tp_int) == &default__int__with__int32 || (tp_int) == &default__int__with__double)
#define maketyped__int(tp_int) ((tp_int) == &usrtype__int__with__INT ? &tusrtype__int__with__INT : (tp_int) == &default__int__with__int64 ? &tdefault__int__with__int64 : (tp_int) == &default__int__with__int32 ? &tdefault__int__with__int32 : (tp_int) == &default__int__with__double ? &tdefault__int__with__double : &tdefault__int)

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
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__double__with__FLOAT(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int64(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int32(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__double__with__FLOAT(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int64(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int32(DeeObject *__restrict self, double *__restrict p_result);
#define isusrtype__double(tp_double) ((tp_double) == &usrtype__double__with__FLOAT)
#define isdefault__double(tp_double) ((tp_double) == &default__double__with__int || (tp_double) == &default__double__with__int64 || (tp_double) == &default__double__with__int32)
#define maketyped__double(tp_double) ((tp_double) == &usrtype__double__with__FLOAT ? &tusrtype__double__with__FLOAT : (tp_double) == &default__double__with__int ? &tdefault__double__with__int : (tp_double) == &default__double__with__int64 ? &tdefault__double__with__int64 : (tp_double) == &default__double__with__int32 ? &tdefault__double__with__int32 : &tdefault__double)

/* tp_cmp->tp_hash */
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tusrtype__hash__with__HASH(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tusrtype__hash__with__(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) Dee_hash_t DCALL tdefault__hash(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL usrtype__hash__with__HASH(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL usrtype__hash__with__(DeeObject *__restrict self);
#define isusrtype__hash(tp_hash) ((tp_hash) == &usrtype__hash__with__HASH || (tp_hash) == &usrtype__hash__with__)
#define maketyped__hash(tp_hash) ((tp_hash) == &usrtype__hash__with__HASH ? &tusrtype__hash__with__HASH : (tp_hash) == &usrtype__hash__with__ ? &tusrtype__hash__with__ : &tdefault__hash)

/* tp_cmp->tp_compare_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__compare_eq__with__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__lo__and__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq__with__le__and__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__compare_eq__with__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__lo__and__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__compare_eq__with__le__and__ge(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__compare_eq(tp_compare_eq) ((tp_compare_eq) == &usrtype__compare_eq__with__)
#define isdefault__compare_eq(tp_compare_eq) ((tp_compare_eq) == &default__compare_eq__with__compare || (tp_compare_eq) == &default__compare_eq__with__eq || (tp_compare_eq) == &default__compare_eq__with__ne || (tp_compare_eq) == &default__compare_eq__with__lo__and__gr || (tp_compare_eq) == &default__compare_eq__with__le__and__ge)
#define maketyped__compare_eq(tp_compare_eq) ((tp_compare_eq) == &usrtype__compare_eq__with__ ? &tusrtype__compare_eq__with__ : (tp_compare_eq) == &default__compare_eq__with__compare ? &tdefault__compare_eq__with__compare : (tp_compare_eq) == &default__compare_eq__with__eq ? &tdefault__compare_eq__with__eq : (tp_compare_eq) == &default__compare_eq__with__ne ? &tdefault__compare_eq__with__ne : (tp_compare_eq) == &default__compare_eq__with__lo__and__gr ? &tdefault__compare_eq__with__lo__and__gr : (tp_compare_eq) == &default__compare_eq__with__le__and__ge ? &tdefault__compare_eq__with__le__and__ge : &tdefault__compare_eq)

/* tp_cmp->tp_compare */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__compare__with__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
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
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__compare__with__(DeeObject *lhs, DeeObject *rhs);
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
#define isusrtype__compare(tp_compare) ((tp_compare) == &usrtype__compare__with__)
#define isdefault__compare(tp_compare) ((tp_compare) == &default__compare__with__eq__and__lo || (tp_compare) == &default__compare__with__eq__and__le || (tp_compare) == &default__compare__with__eq__and__gr || (tp_compare) == &default__compare__with__eq__and__ge || (tp_compare) == &default__compare__with__ne__and__lo || (tp_compare) == &default__compare__with__ne__and__le || (tp_compare) == &default__compare__with__ne__and__gr || (tp_compare) == &default__compare__with__ne__and__ge || (tp_compare) == &default__compare__with__lo__and__gr || (tp_compare) == &default__compare__with__le__and__ge)
#define maketyped__compare(tp_compare) ((tp_compare) == &usrtype__compare__with__ ? &tusrtype__compare__with__ : (tp_compare) == &default__compare__with__eq__and__lo ? &tdefault__compare__with__eq__and__lo : (tp_compare) == &default__compare__with__eq__and__le ? &tdefault__compare__with__eq__and__le : (tp_compare) == &default__compare__with__eq__and__gr ? &tdefault__compare__with__eq__and__gr : (tp_compare) == &default__compare__with__eq__and__ge ? &tdefault__compare__with__eq__and__ge : (tp_compare) == &default__compare__with__ne__and__lo ? &tdefault__compare__with__ne__and__lo : (tp_compare) == &default__compare__with__ne__and__le ? &tdefault__compare__with__ne__and__le : (tp_compare) == &default__compare__with__ne__and__gr ? &tdefault__compare__with__ne__and__gr : (tp_compare) == &default__compare__with__ne__and__ge ? &tdefault__compare__with__ne__and__ge : (tp_compare) == &default__compare__with__lo__and__gr ? &tdefault__compare__with__lo__and__gr : (tp_compare) == &default__compare__with__le__and__ge ? &tdefault__compare__with__le__and__ge : &tdefault__compare)

/* tp_cmp->tp_trycompare_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__trycompare_eq__with__(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__trycompare_eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__trycompare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__trycompare_eq__with__(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__trycompare_eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__trycompare_eq(tp_trycompare_eq) ((tp_trycompare_eq) == &usrtype__trycompare_eq__with__)
#define isdefault__trycompare_eq(tp_trycompare_eq) ((tp_trycompare_eq) == &default__trycompare_eq__with__compare_eq)
#define maketyped__trycompare_eq(tp_trycompare_eq) ((tp_trycompare_eq) == &usrtype__trycompare_eq__with__ ? &tusrtype__trycompare_eq__with__ : (tp_trycompare_eq) == &default__trycompare_eq__with__compare_eq ? &tdefault__trycompare_eq__with__compare_eq : &tdefault__trycompare_eq)

/* tp_cmp->tp_eq */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__eq__with__EQ(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq__with__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__eq__with__EQ(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__eq__with__ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__eq__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__eq(tp_eq) ((tp_eq) == &usrtype__eq__with__EQ)
#define isdefault__eq(tp_eq) ((tp_eq) == &default__eq__with__ne || (tp_eq) == &default__eq__with__compare_eq)
#define maketyped__eq(tp_eq) ((tp_eq) == &usrtype__eq__with__EQ ? &tusrtype__eq__with__EQ : (tp_eq) == &default__eq__with__ne ? &tdefault__eq__with__ne : (tp_eq) == &default__eq__with__compare_eq ? &tdefault__eq__with__compare_eq : &tdefault__eq)

/* tp_cmp->tp_ne */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__ne__with__NE(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne__with__eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne__with__compare_eq(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ne(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__ne__with__NE(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ne__with__eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ne__with__compare_eq(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__ne(tp_ne) ((tp_ne) == &usrtype__ne__with__NE)
#define isdefault__ne(tp_ne) ((tp_ne) == &default__ne__with__eq || (tp_ne) == &default__ne__with__compare_eq)
#define maketyped__ne(tp_ne) ((tp_ne) == &usrtype__ne__with__NE ? &tusrtype__ne__with__NE : (tp_ne) == &default__ne__with__eq ? &tdefault__ne__with__eq : (tp_ne) == &default__ne__with__compare_eq ? &tdefault__ne__with__compare_eq : &tdefault__ne)

/* tp_cmp->tp_lo */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__lo__with__LO(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo__with__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__lo__with__LO(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__lo__with__ge(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__lo__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__lo(tp_lo) ((tp_lo) == &usrtype__lo__with__LO)
#define isdefault__lo(tp_lo) ((tp_lo) == &default__lo__with__ge || (tp_lo) == &default__lo__with__compare)
#define maketyped__lo(tp_lo) ((tp_lo) == &usrtype__lo__with__LO ? &tusrtype__lo__with__LO : (tp_lo) == &default__lo__with__ge ? &tdefault__lo__with__ge : (tp_lo) == &default__lo__with__compare ? &tdefault__lo__with__compare : &tdefault__lo)

/* tp_cmp->tp_le */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__le__with__LE(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le__with__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__le__with__LE(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__le__with__gr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__le__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__le(tp_le) ((tp_le) == &usrtype__le__with__LE)
#define isdefault__le(tp_le) ((tp_le) == &default__le__with__gr || (tp_le) == &default__le__with__compare)
#define maketyped__le(tp_le) ((tp_le) == &usrtype__le__with__LE ? &tusrtype__le__with__LE : (tp_le) == &default__le__with__gr ? &tdefault__le__with__gr : (tp_le) == &default__le__with__compare ? &tdefault__le__with__compare : &tdefault__le)

/* tp_cmp->tp_gr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__gr__with__GR(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr__with__le(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__gr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__gr__with__GR(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__gr__with__le(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__gr__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__gr(tp_gr) ((tp_gr) == &usrtype__gr__with__GR)
#define isdefault__gr(tp_gr) ((tp_gr) == &default__gr__with__le || (tp_gr) == &default__gr__with__compare)
#define maketyped__gr(tp_gr) ((tp_gr) == &usrtype__gr__with__GR ? &tusrtype__gr__with__GR : (tp_gr) == &default__gr__with__le ? &tdefault__gr__with__le : (tp_gr) == &default__gr__with__compare ? &tdefault__gr__with__compare : &tdefault__gr)

/* tp_cmp->tp_ge */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__ge__with__GE(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge__with__lo(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge__with__compare(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__ge(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__ge__with__GE(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ge__with__lo(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__ge__with__compare(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__ge(tp_ge) ((tp_ge) == &usrtype__ge__with__GE)
#define isdefault__ge(tp_ge) ((tp_ge) == &default__ge__with__lo || (tp_ge) == &default__ge__with__compare)
#define maketyped__ge(tp_ge) ((tp_ge) == &usrtype__ge__with__GE ? &tusrtype__ge__with__GE : (tp_ge) == &default__ge__with__lo ? &tdefault__ge__with__lo : (tp_ge) == &default__ge__with__compare ? &tdefault__ge__with__compare : &tdefault__ge)

/* tp_seq->tp_iter */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter__with__ITER(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter__with__ITER(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach_pair(DeeObject *__restrict self);
#define isusrtype__iter(tp_iter) ((tp_iter) == &usrtype__iter__with__ITER)
#define isdefault__iter(tp_iter) ((tp_iter) == &default__iter__with__foreach || (tp_iter) == &default__iter__with__foreach_pair)
#define maketyped__iter(tp_iter) ((tp_iter) == &usrtype__iter__with__ITER ? &tusrtype__iter__with__ITER : &tdefault__iter)

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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__sizeob__with__SIZE(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__sizeob__with__size(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__sizeob(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__sizeob__with__SIZE(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__sizeob__with__size(DeeObject *__restrict self);
#define isusrtype__sizeob(tp_sizeob) ((tp_sizeob) == &usrtype__sizeob__with__SIZE)
#define isdefault__sizeob(tp_sizeob) ((tp_sizeob) == &default__sizeob__with__size)
#define maketyped__sizeob(tp_sizeob) ((tp_sizeob) == &usrtype__sizeob__with__SIZE ? &tusrtype__sizeob__with__SIZE : (tp_sizeob) == &default__sizeob__with__size ? &tdefault__sizeob__with__size : &tdefault__sizeob)

/* tp_seq->tp_size */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size__with__sizeob(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__size(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__size__with__sizeob(DeeObject *__restrict self);
#define isdefault__size(tp_size) ((tp_size) == &default__size__with__sizeob)
#define maketyped__size(tp_size) ((tp_size) == &default__size__with__sizeob ? &tdefault__size__with__sizeob : &tdefault__size)

/* tp_seq->tp_size_fast */
#define tdefault__size_fast__with__ (*(size_t (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_retsm1_2)
#define default__size_fast__with__ (*(size_t (DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1)
#define isdefault__size_fast(tp_size_fast) ((tp_size_fast) == &default__size_fast__with__)

/* tp_seq->tp_contains */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__contains__with__CONTAINS(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__contains(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__contains__with__CONTAINS(DeeObject *self, DeeObject *item);
#define isusrtype__contains(tp_contains) ((tp_contains) == &usrtype__contains__with__CONTAINS)
#define maketyped__contains(tp_contains) ((tp_contains) == &usrtype__contains__with__CONTAINS ? &tusrtype__contains__with__CONTAINS : &tdefault__contains)

/* tp_seq->tp_getitem_index_fast */

/* tp_seq->tp_getitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__getitem__with__GETITEM(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index__and__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index__and__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__trygetitem__and__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__getitem__with__GETITEM(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index__and__getitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index__and__getitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__getitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__trygetitem__and__hasitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem__with__trygetitem(DeeObject *self, DeeObject *index);
#define isusrtype__getitem(tp_getitem) ((tp_getitem) == &usrtype__getitem__with__GETITEM)
#define isdefault__getitem(tp_getitem) ((tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_len_hash || (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_hash || (tp_getitem) == &default__getitem__with__getitem_index || (tp_getitem) == &default__getitem__with__getitem_string_len_hash || (tp_getitem) == &default__getitem__with__getitem_string_hash || (tp_getitem) == &default__getitem__with__trygetitem__and__hasitem || (tp_getitem) == &default__getitem__with__trygetitem)
#define maketyped__getitem(tp_getitem) ((tp_getitem) == &usrtype__getitem__with__GETITEM ? &tusrtype__getitem__with__GETITEM : (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_len_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_len_hash : (tp_getitem) == &default__getitem__with__getitem_index__and__getitem_string_hash ? &tdefault__getitem__with__getitem_index__and__getitem_string_hash : (tp_getitem) == &default__getitem__with__getitem_index ? &tdefault__getitem__with__getitem_index : (tp_getitem) == &default__getitem__with__getitem_string_len_hash ? &tdefault__getitem__with__getitem_string_len_hash : (tp_getitem) == &default__getitem__with__getitem_string_hash ? &tdefault__getitem__with__getitem_string_hash : (tp_getitem) == &default__getitem__with__trygetitem__and__hasitem ? &tdefault__getitem__with__trygetitem__and__hasitem : (tp_getitem) == &default__getitem__with__trygetitem ? &tdefault__getitem__with__trygetitem : &tdefault__getitem)

/* tp_seq->tp_getitem_index */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__trygetitem_index__and__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__getitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__trygetitem_index__and__hasitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__getitem(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__getitem_index__with__trygetitem_index(DeeObject *self, size_t index);
#define isdefault__getitem_index(tp_getitem_index) ((tp_getitem_index) == &default__getitem_index__with__size__and__getitem_index_fast || (tp_getitem_index) == &default__getitem_index__with__trygetitem_index__and__hasitem_index || (tp_getitem_index) == &default__getitem_index__with__getitem || (tp_getitem_index) == &default__getitem_index__with__trygetitem_index)
#define maketyped__getitem_index(tp_getitem_index) ((tp_getitem_index) == &default__getitem_index__with__size__and__getitem_index_fast ? &tdefault__getitem_index__with__size__and__getitem_index_fast : (tp_getitem_index) == &default__getitem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__getitem_index__with__trygetitem_index__and__hasitem_index : (tp_getitem_index) == &default__getitem_index__with__getitem ? &tdefault__getitem_index__with__getitem : (tp_getitem_index) == &default__getitem_index__with__trygetitem_index ? &tdefault__getitem_index__with__trygetitem_index : &tdefault__getitem_index)

/* tp_seq->tp_getitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__getitem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__getitem_string_hash(tp_getitem_string_hash) ((tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem_string_len_hash || (tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash || (tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem || (tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash)
#define maketyped__getitem_string_hash(tp_getitem_string_hash) ((tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem_string_len_hash ? &tdefault__getitem_string_hash__with__getitem_string_len_hash : (tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : (tp_getitem_string_hash) == &default__getitem_string_hash__with__getitem ? &tdefault__getitem_string_hash__with__getitem : (tp_getitem_string_hash) == &default__getitem_string_hash__with__trygetitem_string_hash ? &tdefault__getitem_string_hash__with__trygetitem_string_hash : &tdefault__getitem_string_hash)

/* tp_seq->tp_getitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__getitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__getitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__getitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getitem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__getitem_string_len_hash(tp_getitem_string_len_hash) ((tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem_string_hash || (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash || (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem || (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash)
#define maketyped__getitem_string_len_hash(tp_getitem_string_len_hash) ((tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem_string_hash ? &tdefault__getitem_string_len_hash__with__getitem_string_hash : (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__getitem ? &tdefault__getitem_string_len_hash__with__getitem : (tp_getitem_string_len_hash) == &default__getitem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__getitem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__getitem_string_len_hash)

/* tp_seq->tp_trygetitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__trygetitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem__with__getitem(DeeObject *self, DeeObject *index);
#define isdefault__trygetitem(tp_trygetitem) ((tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_index || (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_len_hash || (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_hash || (tp_trygetitem) == &default__trygetitem__with__getitem)
#define maketyped__trygetitem(tp_trygetitem) ((tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_index__and__trygetitem_string_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_index ? &tdefault__trygetitem__with__trygetitem_index : (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_len_hash ? &tdefault__trygetitem__with__trygetitem_string_len_hash : (tp_trygetitem) == &default__trygetitem__with__trygetitem_string_hash ? &tdefault__trygetitem__with__trygetitem_string_hash : (tp_trygetitem) == &default__trygetitem__with__getitem ? &tdefault__trygetitem__with__getitem : &tdefault__trygetitem)

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

/* tp_seq->tp_trygetitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_hash__with__trygetitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__trygetitem_string_hash(tp_trygetitem_string_hash) ((tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem_string_len_hash || (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__getitem_string_hash || (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem)
#define maketyped__trygetitem_string_hash(tp_trygetitem_string_hash) ((tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem_string_len_hash ? &tdefault__trygetitem_string_hash__with__trygetitem_string_len_hash : (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__getitem_string_hash ? &tdefault__trygetitem_string_hash__with__getitem_string_hash : (tp_trygetitem_string_hash) == &default__trygetitem_string_hash__with__trygetitem ? &tdefault__trygetitem_string_hash__with__trygetitem : &tdefault__trygetitem_string_hash)

/* tp_seq->tp_trygetitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__trygetitem_string_len_hash__with__trygetitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__trygetitem_string_len_hash(tp_trygetitem_string_len_hash) ((tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem_string_hash || (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__getitem_string_len_hash || (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem)
#define maketyped__trygetitem_string_len_hash(tp_trygetitem_string_len_hash) ((tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem_string_hash ? &tdefault__trygetitem_string_len_hash__with__trygetitem_string_hash : (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__getitem_string_len_hash ? &tdefault__trygetitem_string_len_hash__with__getitem_string_len_hash : (tp_trygetitem_string_len_hash) == &default__trygetitem_string_len_hash__with__trygetitem ? &tdefault__trygetitem_string_len_hash__with__trygetitem : &tdefault__trygetitem_string_len_hash)

/* tp_seq->tp_bounditem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__getitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__trygetitem__and__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem__with__trygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index__and__bounditem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index__and__bounditem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__getitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__trygetitem__and__hasitem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__bounditem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem__with__trygetitem(DeeObject *self, DeeObject *index);
#define isdefault__bounditem(tp_bounditem) ((tp_bounditem) == &default__bounditem__with__size__and__getitem_index_fast || (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash || (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_hash || (tp_bounditem) == &default__bounditem__with__getitem || (tp_bounditem) == &default__bounditem__with__trygetitem__and__hasitem || (tp_bounditem) == &default__bounditem__with__bounditem_index || (tp_bounditem) == &default__bounditem__with__bounditem_string_len_hash || (tp_bounditem) == &default__bounditem__with__bounditem_string_hash || (tp_bounditem) == &default__bounditem__with__trygetitem)
#define maketyped__bounditem(tp_bounditem) ((tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_len_hash : (tp_bounditem) == &default__bounditem__with__bounditem_index__and__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_index__and__bounditem_string_hash : (tp_bounditem) == &default__bounditem__with__getitem ? &tdefault__bounditem__with__getitem : (tp_bounditem) == &default__bounditem__with__trygetitem__and__hasitem ? &tdefault__bounditem__with__trygetitem__and__hasitem : (tp_bounditem) == &default__bounditem__with__bounditem_index ? &tdefault__bounditem__with__bounditem_index : (tp_bounditem) == &default__bounditem__with__bounditem_string_len_hash ? &tdefault__bounditem__with__bounditem_string_len_hash : (tp_bounditem) == &default__bounditem__with__bounditem_string_hash ? &tdefault__bounditem__with__bounditem_string_hash : (tp_bounditem) == &default__bounditem__with__trygetitem ? &tdefault__bounditem__with__trygetitem : &tdefault__bounditem)

/* tp_seq->tp_bounditem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__getitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index__with__trygetitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__getitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__trygetitem_index__and__hasitem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__bounditem(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__bounditem_index__with__trygetitem_index(DeeObject *self, size_t index);
#define isdefault__bounditem_index(tp_bounditem_index) ((tp_bounditem_index) == &default__bounditem_index__with__size__and__getitem_index_fast || (tp_bounditem_index) == &default__bounditem_index__with__getitem_index || (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index__and__hasitem_index || (tp_bounditem_index) == &default__bounditem_index__with__bounditem || (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index)
#define maketyped__bounditem_index(tp_bounditem_index) ((tp_bounditem_index) == &default__bounditem_index__with__size__and__getitem_index_fast ? &tdefault__bounditem_index__with__size__and__getitem_index_fast : (tp_bounditem_index) == &default__bounditem_index__with__getitem_index ? &tdefault__bounditem_index__with__getitem_index : (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index__and__hasitem_index ? &tdefault__bounditem_index__with__trygetitem_index__and__hasitem_index : (tp_bounditem_index) == &default__bounditem_index__with__bounditem ? &tdefault__bounditem_index__with__bounditem : (tp_bounditem_index) == &default__bounditem_index__with__trygetitem_index ? &tdefault__bounditem_index__with__trygetitem_index : &tdefault__bounditem_index)

/* tp_seq->tp_bounditem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__getitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash__with__trygetitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__getitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__bounditem_string_len_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__bounditem(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_hash__with__trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__bounditem_string_hash(tp_bounditem_string_hash) ((tp_bounditem_string_hash) == &default__bounditem_string_hash__with__getitem_string_hash || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem_string_len_hash || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem || (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash)
#define maketyped__bounditem_string_hash(tp_bounditem_string_hash) ((tp_bounditem_string_hash) == &default__bounditem_string_hash__with__getitem_string_hash ? &tdefault__bounditem_string_hash__with__getitem_string_hash : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem_string_len_hash ? &tdefault__bounditem_string_hash__with__bounditem_string_len_hash : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__bounditem ? &tdefault__bounditem_string_hash__with__bounditem : (tp_bounditem_string_hash) == &default__bounditem_string_hash__with__trygetitem_string_hash ? &tdefault__bounditem_string_hash__with__trygetitem_string_hash : &tdefault__bounditem_string_hash)

/* tp_seq->tp_bounditem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__getitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__getitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__bounditem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__bounditem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__bounditem_string_len_hash__with__trygetitem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__bounditem_string_len_hash(tp_bounditem_string_len_hash) ((tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__getitem_string_len_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem_string_hash || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem || (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash)
#define maketyped__bounditem_string_len_hash(tp_bounditem_string_len_hash) ((tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__getitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__getitem_string_len_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem_string_hash ? &tdefault__bounditem_string_len_hash__with__bounditem_string_hash : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__bounditem ? &tdefault__bounditem_string_len_hash__with__bounditem : (tp_bounditem_string_len_hash) == &default__bounditem_string_len_hash__with__trygetitem_string_len_hash ? &tdefault__bounditem_string_len_hash__with__trygetitem_string_len_hash : &tdefault__bounditem_string_len_hash)

/* tp_seq->tp_hasitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__bounditem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__bounditem(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index__and__hasitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index__and__hasitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__hasitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem__with__size__and__getitem_index_fast(DeeObject *self, DeeObject *index);
#define isdefault__hasitem(tp_hasitem) ((tp_hasitem) == &default__hasitem__with__bounditem || (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash || (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_hash || (tp_hasitem) == &default__hasitem__with__hasitem_index || (tp_hasitem) == &default__hasitem__with__hasitem_string_len_hash || (tp_hasitem) == &default__hasitem__with__hasitem_string_hash || (tp_hasitem) == &default__hasitem__with__size__and__getitem_index_fast)
#define maketyped__hasitem(tp_hasitem) ((tp_hasitem) == &default__hasitem__with__bounditem ? &tdefault__hasitem__with__bounditem : (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_len_hash : (tp_hasitem) == &default__hasitem__with__hasitem_index__and__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_index__and__hasitem_string_hash : (tp_hasitem) == &default__hasitem__with__hasitem_index ? &tdefault__hasitem__with__hasitem_index : (tp_hasitem) == &default__hasitem__with__hasitem_string_len_hash ? &tdefault__hasitem__with__hasitem_string_len_hash : (tp_hasitem) == &default__hasitem__with__hasitem_string_hash ? &tdefault__hasitem__with__hasitem_string_hash : &tdefault__hasitem)

/* tp_seq->tp_hasitem_index */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__bounditem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__size__and__getitem_index_fast(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__hasitem_index(DeeTypeObject *tp_self, DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__bounditem_index(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__size__and__getitem_index_fast(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL default__hasitem_index__with__hasitem(DeeObject *self, size_t index);
#define isdefault__hasitem_index(tp_hasitem_index) ((tp_hasitem_index) == &default__hasitem_index__with__bounditem_index || (tp_hasitem_index) == &default__hasitem_index__with__size__and__getitem_index_fast || (tp_hasitem_index) == &default__hasitem_index__with__hasitem)
#define maketyped__hasitem_index(tp_hasitem_index) ((tp_hasitem_index) == &default__hasitem_index__with__bounditem_index ? &tdefault__hasitem_index__with__bounditem_index : (tp_hasitem_index) == &default__hasitem_index__with__size__and__getitem_index_fast ? &tdefault__hasitem_index__with__size__and__getitem_index_fast : (tp_hasitem_index) == &default__hasitem_index__with__hasitem ? &tdefault__hasitem_index__with__hasitem : &tdefault__hasitem_index)

/* tp_seq->tp_hasitem_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__bounditem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__bounditem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__hasitem_string_len_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_hash__with__hasitem(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__hasitem_string_hash(tp_hasitem_string_hash) ((tp_hasitem_string_hash) == &default__hasitem_string_hash__with__bounditem_string_hash || (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem_string_len_hash || (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem)
#define maketyped__hasitem_string_hash(tp_hasitem_string_hash) ((tp_hasitem_string_hash) == &default__hasitem_string_hash__with__bounditem_string_hash ? &tdefault__hasitem_string_hash__with__bounditem_string_hash : (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem_string_len_hash ? &tdefault__hasitem_string_hash__with__hasitem_string_len_hash : (tp_hasitem_string_hash) == &default__hasitem_string_hash__with__hasitem ? &tdefault__hasitem_string_hash__with__hasitem : &tdefault__hasitem_string_hash)

/* tp_seq->tp_hasitem_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__hasitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash__with__hasitem(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__bounditem_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__hasitem_string_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasitem_string_len_hash__with__hasitem(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__hasitem_string_len_hash(tp_hasitem_string_len_hash) ((tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__bounditem_string_len_hash || (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem_string_hash || (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem)
#define maketyped__hasitem_string_len_hash(tp_hasitem_string_len_hash) ((tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__bounditem_string_len_hash ? &tdefault__hasitem_string_len_hash__with__bounditem_string_len_hash : (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem_string_hash ? &tdefault__hasitem_string_len_hash__with__hasitem_string_hash : (tp_hasitem_string_len_hash) == &default__hasitem_string_len_hash__with__hasitem ? &tdefault__hasitem_string_len_hash__with__hasitem : &tdefault__hasitem_string_len_hash)

/* tp_seq->tp_delitem */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__delitem__with__DELITEM(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index__and__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index__and__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem__with__delitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__delitem__with__DELITEM(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index__and__delitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index__and__delitem_string_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_index(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_string_len_hash(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delitem__with__delitem_string_hash(DeeObject *self, DeeObject *index);
#define isusrtype__delitem(tp_delitem) ((tp_delitem) == &usrtype__delitem__with__DELITEM)
#define isdefault__delitem(tp_delitem) ((tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_len_hash || (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_hash || (tp_delitem) == &default__delitem__with__delitem_index || (tp_delitem) == &default__delitem__with__delitem_string_len_hash || (tp_delitem) == &default__delitem__with__delitem_string_hash)
#define maketyped__delitem(tp_delitem) ((tp_delitem) == &usrtype__delitem__with__DELITEM ? &tusrtype__delitem__with__DELITEM : (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_len_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_len_hash : (tp_delitem) == &default__delitem__with__delitem_index__and__delitem_string_hash ? &tdefault__delitem__with__delitem_index__and__delitem_string_hash : (tp_delitem) == &default__delitem__with__delitem_index ? &tdefault__delitem__with__delitem_index : (tp_delitem) == &default__delitem__with__delitem_string_len_hash ? &tdefault__delitem__with__delitem_string_len_hash : (tp_delitem) == &default__delitem__with__delitem_string_hash ? &tdefault__delitem__with__delitem_string_hash : &tdefault__delitem)

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
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tusrtype__setitem__with__SETITEM(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index__and__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index__and__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_index(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem__with__setitem_string_hash(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL usrtype__setitem__with__SETITEM(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index__and__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index__and__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_index(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_string_len_hash(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__setitem__with__setitem_string_hash(DeeObject *self, DeeObject *index, DeeObject *value);
#define isusrtype__setitem(tp_setitem) ((tp_setitem) == &usrtype__setitem__with__SETITEM)
#define isdefault__setitem(tp_setitem) ((tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_len_hash || (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_hash || (tp_setitem) == &default__setitem__with__setitem_index || (tp_setitem) == &default__setitem__with__setitem_string_len_hash || (tp_setitem) == &default__setitem__with__setitem_string_hash)
#define maketyped__setitem(tp_setitem) ((tp_setitem) == &usrtype__setitem__with__SETITEM ? &tusrtype__setitem__with__SETITEM : (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_len_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_len_hash : (tp_setitem) == &default__setitem__with__setitem_index__and__setitem_string_hash ? &tdefault__setitem__with__setitem_index__and__setitem_string_hash : (tp_setitem) == &default__setitem__with__setitem_index ? &tdefault__setitem__with__setitem_index : (tp_setitem) == &default__setitem__with__setitem_string_len_hash ? &tdefault__setitem__with__setitem_string_len_hash : (tp_setitem) == &default__setitem__with__setitem_string_hash ? &tdefault__setitem__with__setitem_string_hash : &tdefault__setitem)

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
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tusrtype__getrange__with__GETRANGE(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__getrange__with__getrange_index__and__getrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL tdefault__getrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL usrtype__getrange__with__GETRANGE(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL default__getrange__with__getrange_index__and__getrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
#define isusrtype__getrange(tp_getrange) ((tp_getrange) == &usrtype__getrange__with__GETRANGE)
#define isdefault__getrange(tp_getrange) ((tp_getrange) == &default__getrange__with__getrange_index__and__getrange_index_n)
#define maketyped__getrange(tp_getrange) ((tp_getrange) == &usrtype__getrange__with__GETRANGE ? &tusrtype__getrange__with__GETRANGE : (tp_getrange) == &default__getrange__with__getrange_index__and__getrange_index_n ? &tdefault__getrange__with__getrange_index__and__getrange_index_n : &tdefault__getrange)

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
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tusrtype__delrange__with__DELRANGE(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__delrange__with__delrange_index__and__delrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__delrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL usrtype__delrange__with__DELRANGE(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL default__delrange__with__delrange_index__and__delrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end);
#define isusrtype__delrange(tp_delrange) ((tp_delrange) == &usrtype__delrange__with__DELRANGE)
#define isdefault__delrange(tp_delrange) ((tp_delrange) == &default__delrange__with__delrange_index__and__delrange_index_n)
#define maketyped__delrange(tp_delrange) ((tp_delrange) == &usrtype__delrange__with__DELRANGE ? &tusrtype__delrange__with__DELRANGE : (tp_delrange) == &default__delrange__with__delrange_index__and__delrange_index_n ? &tdefault__delrange__with__delrange_index__and__delrange_index_n : &tdefault__delrange)

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
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tusrtype__setrange__with__SETRANGE(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tdefault__setrange__with__setrange_index__and__setrange_index_n(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL tdefault__setrange(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL usrtype__setrange__with__SETRANGE(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL default__setrange__with__setrange_index__and__setrange_index_n(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
#define isusrtype__setrange(tp_setrange) ((tp_setrange) == &usrtype__setrange__with__SETRANGE)
#define isdefault__setrange(tp_setrange) ((tp_setrange) == &default__setrange__with__setrange_index__and__setrange_index_n)
#define maketyped__setrange(tp_setrange) ((tp_setrange) == &usrtype__setrange__with__SETRANGE ? &tusrtype__setrange__with__SETRANGE : (tp_setrange) == &default__setrange__with__setrange_index__and__setrange_index_n ? &tdefault__setrange__with__setrange_index__and__setrange_index_n : &tdefault__setrange)

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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__inv__with__INV(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__inv(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__inv__with__INV(DeeObject *self);
#define isusrtype__inv(tp_inv) ((tp_inv) == &usrtype__inv__with__INV)
#define maketyped__inv(tp_inv) ((tp_inv) == &usrtype__inv__with__INV ? &tusrtype__inv__with__INV : &tdefault__inv)

/* tp_math->tp_pos */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__pos__with__POS(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__pos(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__pos__with__POS(DeeObject *self);
#define isusrtype__pos(tp_pos) ((tp_pos) == &usrtype__pos__with__POS)
#define maketyped__pos(tp_pos) ((tp_pos) == &usrtype__pos__with__POS ? &tusrtype__pos__with__POS : &tdefault__pos)

/* tp_math->tp_neg */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__neg__with__NEG(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__neg(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__neg__with__NEG(DeeObject *self);
#define isusrtype__neg(tp_neg) ((tp_neg) == &usrtype__neg__with__NEG)
#define maketyped__neg(tp_neg) ((tp_neg) == &usrtype__neg__with__NEG ? &tusrtype__neg__with__NEG : &tdefault__neg)

/* tp_math->tp_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__add__with__ADD(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__add__with__ADD(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__add(tp_add) ((tp_add) == &usrtype__add__with__ADD)
#define maketyped__add(tp_add) ((tp_add) == &usrtype__add__with__ADD ? &tusrtype__add__with__ADD : &tdefault__add)

/* tp_math->tp_inplace_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_add__with__INPLACE_ADD(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_add__with__add(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_add(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_add__with__INPLACE_ADD(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_add__with__add(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add__with__INPLACE_ADD)
#define isdefault__inplace_add(tp_inplace_add) ((tp_inplace_add) == &default__inplace_add__with__add)
#define maketyped__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add__with__INPLACE_ADD ? &tusrtype__inplace_add__with__INPLACE_ADD : (tp_inplace_add) == &default__inplace_add__with__add ? &tdefault__inplace_add__with__add : &tdefault__inplace_add)

/* tp_math->tp_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__sub__with__SUB(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__sub__with__SUB(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__sub(tp_sub) ((tp_sub) == &usrtype__sub__with__SUB)
#define maketyped__sub(tp_sub) ((tp_sub) == &usrtype__sub__with__SUB ? &tusrtype__sub__with__SUB : &tdefault__sub)

/* tp_math->tp_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_sub__with__INPLACE_SUB(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_sub__with__sub(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_sub(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_sub__with__INPLACE_SUB(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_sub__with__sub(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub__with__INPLACE_SUB)
#define isdefault__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &default__inplace_sub__with__sub)
#define maketyped__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub__with__INPLACE_SUB ? &tusrtype__inplace_sub__with__INPLACE_SUB : (tp_inplace_sub) == &default__inplace_sub__with__sub ? &tdefault__inplace_sub__with__sub : &tdefault__inplace_sub)

/* tp_math->tp_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mul__with__MUL(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mul__with__MUL(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mul(tp_mul) ((tp_mul) == &usrtype__mul__with__MUL)
#define maketyped__mul(tp_mul) ((tp_mul) == &usrtype__mul__with__MUL ? &tusrtype__mul__with__MUL : &tdefault__mul)

/* tp_math->tp_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mul__with__INPLACE_MUL(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mul__with__mul(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mul(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mul__with__INPLACE_MUL(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mul__with__mul(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul__with__INPLACE_MUL)
#define isdefault__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &default__inplace_mul__with__mul)
#define maketyped__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul__with__INPLACE_MUL ? &tusrtype__inplace_mul__with__INPLACE_MUL : (tp_inplace_mul) == &default__inplace_mul__with__mul ? &tdefault__inplace_mul__with__mul : &tdefault__inplace_mul)

/* tp_math->tp_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__div__with__DIV(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__div__with__DIV(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__div(tp_div) ((tp_div) == &usrtype__div__with__DIV)
#define maketyped__div(tp_div) ((tp_div) == &usrtype__div__with__DIV ? &tusrtype__div__with__DIV : &tdefault__div)

/* tp_math->tp_inplace_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_div__with__INPLACE_DIV(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_div__with__div(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_div(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_div__with__INPLACE_DIV(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_div__with__div(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div__with__INPLACE_DIV)
#define isdefault__inplace_div(tp_inplace_div) ((tp_inplace_div) == &default__inplace_div__with__div)
#define maketyped__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div__with__INPLACE_DIV ? &tusrtype__inplace_div__with__INPLACE_DIV : (tp_inplace_div) == &default__inplace_div__with__div ? &tdefault__inplace_div__with__div : &tdefault__inplace_div)

/* tp_math->tp_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mod__with__MOD(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mod__with__MOD(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mod(tp_mod) ((tp_mod) == &usrtype__mod__with__MOD)
#define maketyped__mod(tp_mod) ((tp_mod) == &usrtype__mod__with__MOD ? &tusrtype__mod__with__MOD : &tdefault__mod)

/* tp_math->tp_inplace_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mod__with__INPLACE_MOD(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mod__with__mod(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mod(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mod__with__INPLACE_MOD(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mod__with__mod(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod__with__INPLACE_MOD)
#define isdefault__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &default__inplace_mod__with__mod)
#define maketyped__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod__with__INPLACE_MOD ? &tusrtype__inplace_mod__with__INPLACE_MOD : (tp_inplace_mod) == &default__inplace_mod__with__mod ? &tdefault__inplace_mod__with__mod : &tdefault__inplace_mod)

/* tp_math->tp_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shl__with__SHL(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shl__with__SHL(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shl(tp_shl) ((tp_shl) == &usrtype__shl__with__SHL)
#define maketyped__shl(tp_shl) ((tp_shl) == &usrtype__shl__with__SHL ? &tusrtype__shl__with__SHL : &tdefault__shl)

/* tp_math->tp_inplace_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shl__with__INPLACE_SHL(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shl__with__shl(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shl(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shl__with__INPLACE_SHL(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shl__with__shl(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl__with__INPLACE_SHL)
#define isdefault__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &default__inplace_shl__with__shl)
#define maketyped__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl__with__INPLACE_SHL ? &tusrtype__inplace_shl__with__INPLACE_SHL : (tp_inplace_shl) == &default__inplace_shl__with__shl ? &tdefault__inplace_shl__with__shl : &tdefault__inplace_shl)

/* tp_math->tp_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shr__with__SHR(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shr__with__SHR(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shr(tp_shr) ((tp_shr) == &usrtype__shr__with__SHR)
#define maketyped__shr(tp_shr) ((tp_shr) == &usrtype__shr__with__SHR ? &tusrtype__shr__with__SHR : &tdefault__shr)

/* tp_math->tp_inplace_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shr__with__INPLACE_SHR(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shr__with__shr(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shr(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shr__with__INPLACE_SHR(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shr__with__shr(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr__with__INPLACE_SHR)
#define isdefault__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &default__inplace_shr__with__shr)
#define maketyped__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr__with__INPLACE_SHR ? &tusrtype__inplace_shr__with__INPLACE_SHR : (tp_inplace_shr) == &default__inplace_shr__with__shr ? &tdefault__inplace_shr__with__shr : &tdefault__inplace_shr)

/* tp_math->tp_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__and__with__AND(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__and__with__AND(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__and(tp_and) ((tp_and) == &usrtype__and__with__AND)
#define maketyped__and(tp_and) ((tp_and) == &usrtype__and__with__AND ? &tusrtype__and__with__AND : &tdefault__and)

/* tp_math->tp_inplace_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_and__with__INPLACE_AND(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_and__with__and(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_and(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_and__with__INPLACE_AND(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_and__with__and(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and__with__INPLACE_AND)
#define isdefault__inplace_and(tp_inplace_and) ((tp_inplace_and) == &default__inplace_and__with__and)
#define maketyped__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and__with__INPLACE_AND ? &tusrtype__inplace_and__with__INPLACE_AND : (tp_inplace_and) == &default__inplace_and__with__and ? &tdefault__inplace_and__with__and : &tdefault__inplace_and)

/* tp_math->tp_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__or__with__OR(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__or__with__OR(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__or(tp_or) ((tp_or) == &usrtype__or__with__OR)
#define maketyped__or(tp_or) ((tp_or) == &usrtype__or__with__OR ? &tusrtype__or__with__OR : &tdefault__or)

/* tp_math->tp_inplace_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_or__with__INPLACE_OR(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_or__with__or(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_or(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_or__with__INPLACE_OR(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_or__with__or(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or__with__INPLACE_OR)
#define isdefault__inplace_or(tp_inplace_or) ((tp_inplace_or) == &default__inplace_or__with__or)
#define maketyped__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or__with__INPLACE_OR ? &tusrtype__inplace_or__with__INPLACE_OR : (tp_inplace_or) == &default__inplace_or__with__or ? &tdefault__inplace_or__with__or : &tdefault__inplace_or)

/* tp_math->tp_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__xor__with__XOR(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__xor__with__XOR(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__xor(tp_xor) ((tp_xor) == &usrtype__xor__with__XOR)
#define maketyped__xor(tp_xor) ((tp_xor) == &usrtype__xor__with__XOR ? &tusrtype__xor__with__XOR : &tdefault__xor)

/* tp_math->tp_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_xor__with__INPLACE_XOR(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_xor__with__xor(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_xor(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_xor__with__INPLACE_XOR(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_xor__with__xor(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor__with__INPLACE_XOR)
#define isdefault__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &default__inplace_xor__with__xor)
#define maketyped__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor__with__INPLACE_XOR ? &tusrtype__inplace_xor__with__INPLACE_XOR : (tp_inplace_xor) == &default__inplace_xor__with__xor ? &tdefault__inplace_xor__with__xor : &tdefault__inplace_xor)

/* tp_math->tp_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__pow__with__POW(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__pow__with__POW(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__pow(tp_pow) ((tp_pow) == &usrtype__pow__with__POW)
#define maketyped__pow(tp_pow) ((tp_pow) == &usrtype__pow__with__POW ? &tusrtype__pow__with__POW : &tdefault__pow)

/* tp_math->tp_inplace_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_pow__with__INPLACE_POW(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_pow__with__pow(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_pow(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_pow__with__INPLACE_POW(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_pow__with__pow(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow__with__INPLACE_POW)
#define isdefault__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &default__inplace_pow__with__pow)
#define maketyped__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow__with__INPLACE_POW ? &tusrtype__inplace_pow__with__INPLACE_POW : (tp_inplace_pow) == &default__inplace_pow__with__pow ? &tdefault__inplace_pow__with__pow : &tdefault__inplace_pow)

/* tp_math->tp_inc */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__inc__with__INC(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__inplace_add(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__add(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__inc__with__INC(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__inplace_add(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__add(DREF DeeObject **__restrict p_self);
#define isusrtype__inc(tp_inc) ((tp_inc) == &usrtype__inc__with__INC)
#define isdefault__inc(tp_inc) ((tp_inc) == &default__inc__with__inplace_add || (tp_inc) == &default__inc__with__add)
#define maketyped__inc(tp_inc) ((tp_inc) == &usrtype__inc__with__INC ? &tusrtype__inc__with__INC : (tp_inc) == &default__inc__with__inplace_add ? &tdefault__inc__with__inplace_add : (tp_inc) == &default__inc__with__add ? &tdefault__inc__with__add : &tdefault__inc)

/* tp_math->tp_dec */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__dec__with__DEC(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__inplace_sub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__sub(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec(DeeTypeObject *tp_self, DREF DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__dec__with__DEC(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__inplace_sub(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__sub(DREF DeeObject **__restrict p_self);
#define isusrtype__dec(tp_dec) ((tp_dec) == &usrtype__dec__with__DEC)
#define isdefault__dec(tp_dec) ((tp_dec) == &default__dec__with__inplace_sub || (tp_dec) == &default__dec__with__sub)
#define maketyped__dec(tp_dec) ((tp_dec) == &usrtype__dec__with__DEC ? &tusrtype__dec__with__DEC : (tp_dec) == &default__dec__with__inplace_sub ? &tdefault__dec__with__inplace_sub : (tp_dec) == &default__dec__with__sub ? &tdefault__dec__with__sub : &tdefault__dec)

/* tp_with->tp_enter */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__enter__with__ENTER(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__enter__with__leave (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__enter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__enter__with__ENTER(DeeObject *__restrict self);
#define default__enter__with__leave (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__enter(tp_enter) ((tp_enter) == &usrtype__enter__with__ENTER)
#define isdefault__enter(tp_enter) ((tp_enter) == &default__enter__with__leave)
#define maketyped__enter(tp_enter) ((tp_enter) == &usrtype__enter__with__ENTER ? &tusrtype__enter__with__ENTER : &tdefault__enter)

/* tp_with->tp_leave */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__leave__with__LEAVE(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__leave__with__enter (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__leave(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__leave__with__LEAVE(DeeObject *__restrict self);
#define default__leave__with__enter (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__leave(tp_leave) ((tp_leave) == &usrtype__leave__with__LEAVE)
#define isdefault__leave(tp_leave) ((tp_leave) == &default__leave__with__enter)
#define maketyped__leave(tp_leave) ((tp_leave) == &usrtype__leave__with__LEAVE ? &tusrtype__leave__with__LEAVE : &tdefault__leave)

/* tp_attr->tp_getattr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__getattr__with__GETATTR(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__getattr__with__GETATTR(DeeObject *self, DeeObject *attr);
#define isusrtype__getattr(tp_getattr) ((tp_getattr) == &usrtype__getattr__with__GETATTR)
#define maketyped__getattr(tp_getattr) ((tp_getattr) == &usrtype__getattr__with__GETATTR ? &tusrtype__getattr__with__GETATTR : &tdefault__getattr)

/* tp_attr->tp_getattr_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getattr_string_hash__with__getattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getattr_string_hash__with__getattr(DeeObject *self, char const *attr, Dee_hash_t hash);
#define isdefault__getattr_string_hash(tp_getattr_string_hash) ((tp_getattr_string_hash) == &default__getattr_string_hash__with__getattr)
#define maketyped__getattr_string_hash(tp_getattr_string_hash) ((tp_getattr_string_hash) == &default__getattr_string_hash__with__getattr ? &tdefault__getattr_string_hash__with__getattr : &tdefault__getattr_string_hash)

/* tp_attr->tp_getattr_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getattr_string_len_hash__with__getattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__getattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL default__getattr_string_len_hash__with__getattr(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
#define isdefault__getattr_string_len_hash(tp_getattr_string_len_hash) ((tp_getattr_string_len_hash) == &default__getattr_string_len_hash__with__getattr)
#define maketyped__getattr_string_len_hash(tp_getattr_string_len_hash) ((tp_getattr_string_len_hash) == &default__getattr_string_len_hash__with__getattr ? &tdefault__getattr_string_len_hash__with__getattr : &tdefault__getattr_string_len_hash)

/* tp_attr->tp_boundattr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr__with__getattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__boundattr__with__getattr(DeeObject *self, DeeObject *attr);
#define isdefault__boundattr(tp_boundattr) ((tp_boundattr) == &default__boundattr__with__getattr)
#define maketyped__boundattr(tp_boundattr) ((tp_boundattr) == &default__boundattr__with__getattr ? &tdefault__boundattr__with__getattr : &tdefault__boundattr)

/* tp_attr->tp_boundattr_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_hash__with__getattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_hash__with__boundattr(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__boundattr_string_hash__with__getattr_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__boundattr_string_hash__with__boundattr(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__boundattr_string_hash(tp_boundattr_string_hash) ((tp_boundattr_string_hash) == &default__boundattr_string_hash__with__getattr_string_hash || (tp_boundattr_string_hash) == &default__boundattr_string_hash__with__boundattr)
#define maketyped__boundattr_string_hash(tp_boundattr_string_hash) ((tp_boundattr_string_hash) == &default__boundattr_string_hash__with__getattr_string_hash ? &tdefault__boundattr_string_hash__with__getattr_string_hash : (tp_boundattr_string_hash) == &default__boundattr_string_hash__with__boundattr ? &tdefault__boundattr_string_hash__with__boundattr : &tdefault__boundattr_string_hash)

/* tp_attr->tp_boundattr_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_len_hash__with__getattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_len_hash__with__boundattr(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__boundattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__boundattr_string_len_hash__with__getattr_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__boundattr_string_len_hash__with__boundattr(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__boundattr_string_len_hash(tp_boundattr_string_len_hash) ((tp_boundattr_string_len_hash) == &default__boundattr_string_len_hash__with__getattr_string_len_hash || (tp_boundattr_string_len_hash) == &default__boundattr_string_len_hash__with__boundattr)
#define maketyped__boundattr_string_len_hash(tp_boundattr_string_len_hash) ((tp_boundattr_string_len_hash) == &default__boundattr_string_len_hash__with__getattr_string_len_hash ? &tdefault__boundattr_string_len_hash__with__getattr_string_len_hash : (tp_boundattr_string_len_hash) == &default__boundattr_string_len_hash__with__boundattr ? &tdefault__boundattr_string_len_hash__with__boundattr : &tdefault__boundattr_string_len_hash)

/* tp_attr->tp_hasattr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr__with__boundattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasattr__with__boundattr(DeeObject *self, DeeObject *attr);
#define isdefault__hasattr(tp_hasattr) ((tp_hasattr) == &default__hasattr__with__boundattr)
#define maketyped__hasattr(tp_hasattr) ((tp_hasattr) == &default__hasattr__with__boundattr ? &tdefault__hasattr__with__boundattr : &tdefault__hasattr)

/* tp_attr->tp_hasattr_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr_string_hash__with__boundattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasattr_string_hash__with__boundattr_string_hash(DeeObject *self, char const *key, Dee_hash_t hash);
#define isdefault__hasattr_string_hash(tp_hasattr_string_hash) ((tp_hasattr_string_hash) == &default__hasattr_string_hash__with__boundattr_string_hash)
#define maketyped__hasattr_string_hash(tp_hasattr_string_hash) ((tp_hasattr_string_hash) == &default__hasattr_string_hash__with__boundattr_string_hash ? &tdefault__hasattr_string_hash__with__boundattr_string_hash : &tdefault__hasattr_string_hash)

/* tp_attr->tp_hasattr_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr_string_len_hash__with__boundattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__hasattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__hasattr_string_len_hash__with__boundattr_string_len_hash(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash);
#define isdefault__hasattr_string_len_hash(tp_hasattr_string_len_hash) ((tp_hasattr_string_len_hash) == &default__hasattr_string_len_hash__with__boundattr_string_len_hash)
#define maketyped__hasattr_string_len_hash(tp_hasattr_string_len_hash) ((tp_hasattr_string_len_hash) == &default__hasattr_string_len_hash__with__boundattr_string_len_hash ? &tdefault__hasattr_string_len_hash__with__boundattr_string_len_hash : &tdefault__hasattr_string_len_hash)

/* tp_attr->tp_delattr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__delattr__with__DELATTR(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__delattr__with__DELATTR(DeeObject *self, DeeObject *attr);
#define isusrtype__delattr(tp_delattr) ((tp_delattr) == &usrtype__delattr__with__DELATTR)
#define maketyped__delattr(tp_delattr) ((tp_delattr) == &usrtype__delattr__with__DELATTR ? &tusrtype__delattr__with__DELATTR : &tdefault__delattr)

/* tp_attr->tp_delattr_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delattr_string_hash__with__delattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delattr_string_hash__with__delattr(DeeObject *self, char const *attr, Dee_hash_t hash);
#define isdefault__delattr_string_hash(tp_delattr_string_hash) ((tp_delattr_string_hash) == &default__delattr_string_hash__with__delattr)
#define maketyped__delattr_string_hash(tp_delattr_string_hash) ((tp_delattr_string_hash) == &default__delattr_string_hash__with__delattr ? &tdefault__delattr_string_hash__with__delattr : &tdefault__delattr_string_hash)

/* tp_attr->tp_delattr_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delattr_string_len_hash__with__delattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__delattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__delattr_string_len_hash__with__delattr(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);
#define isdefault__delattr_string_len_hash(tp_delattr_string_len_hash) ((tp_delattr_string_len_hash) == &default__delattr_string_len_hash__with__delattr)
#define maketyped__delattr_string_len_hash(tp_delattr_string_len_hash) ((tp_delattr_string_len_hash) == &default__delattr_string_len_hash__with__delattr ? &tdefault__delattr_string_len_hash__with__delattr : &tdefault__delattr_string_len_hash)

/* tp_attr->tp_setattr */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tusrtype__setattr__with__SETATTR(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL tdefault__setattr(DeeTypeObject *tp_self, DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL usrtype__setattr__with__SETATTR(DeeObject *self, DeeObject *attr, DeeObject *value);
#define isusrtype__setattr(tp_setattr) ((tp_setattr) == &usrtype__setattr__with__SETATTR)
#define maketyped__setattr(tp_setattr) ((tp_setattr) == &usrtype__setattr__with__SETATTR ? &tusrtype__setattr__with__SETATTR : &tdefault__setattr)

/* tp_attr->tp_setattr_string_hash */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL tdefault__setattr_string_hash__with__setattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int DCALL tdefault__setattr_string_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL default__setattr_string_hash__with__setattr(DeeObject *self, char const *attr, Dee_hash_t hash, DeeObject *value);
#define isdefault__setattr_string_hash(tp_setattr_string_hash) ((tp_setattr_string_hash) == &default__setattr_string_hash__with__setattr)
#define maketyped__setattr_string_hash(tp_setattr_string_hash) ((tp_setattr_string_hash) == &default__setattr_string_hash__with__setattr ? &tdefault__setattr_string_hash__with__setattr : &tdefault__setattr_string_hash)

/* tp_attr->tp_setattr_string_len_hash */
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__setattr_string_len_hash__with__setattr(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int DCALL tdefault__setattr_string_len_hash(DeeTypeObject *tp_self, DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL default__setattr_string_len_hash__with__setattr(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
#define isdefault__setattr_string_len_hash(tp_setattr_string_len_hash) ((tp_setattr_string_len_hash) == &default__setattr_string_len_hash__with__setattr)
#define maketyped__setattr_string_len_hash(tp_setattr_string_len_hash) ((tp_setattr_string_len_hash) == &default__setattr_string_len_hash__with__setattr ? &tdefault__setattr_string_len_hash__with__setattr : &tdefault__setattr_string_len_hash)
/*[[[end]]]*/
/* clang-format on */


/* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS: Backward compat -- DEPRECATED! */
#define DeeType_InheritStr(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_str) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_print))
#define DeeType_InheritRepr(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_repr) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_printrepr))
#define DeeType_InheritBool(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_bool) && 1)
#define DeeType_InheritCall(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_call) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_call_kw))
#define DeeType_InheritInt(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_int) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_int32) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_int64) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_double))
#define DeeType_InheritInv(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inv) && 1)
#define DeeType_InheritPos(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_pos) && 1)
#define DeeType_InheritNeg(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_neg) && 1)
#define DeeType_InheritAdd(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_add) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_sub) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_add) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_sub) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inc) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_dec))
#define DeeType_InheritMul(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_mul) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_mul))
#define DeeType_InheritDiv(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_div) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_div))
#define DeeType_InheritMod(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_mod) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_mod))
#define DeeType_InheritShl(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_shl) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_shl))
#define DeeType_InheritShr(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_shr) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_shr))
#define DeeType_InheritAnd(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_and) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_and))
#define DeeType_InheritOr(self)       (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_or) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_or))
#define DeeType_InheritXor(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_xor) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_xor))
#define DeeType_InheritPow(self)      (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_pow) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_inplace_pow))
#define DeeType_InheritCompare(self)  (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_hash) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_eq) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_ne) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_lo) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_le) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_gr) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_ge) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_compare_eq) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_compare) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_trycompare_eq))
#define DeeType_InheritIterNext(self) (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_iter_next) && 1)
#define DeeType_InheritIter(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_iter) && 1)
#define DeeType_InheritSize(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_sizeob) && 1)
#define DeeType_InheritContains(self) (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_contains) && 1)
#define DeeType_InheritGetItem(self)  (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_getitem) && 1)
#define DeeType_InheritDelItem(self)  (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_delitem) && 1)
#define DeeType_InheritSetItem(self)  (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_setitem) && 1)
#define DeeType_InheritGetRange(self) (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_getrange) && 1)
#define DeeType_InheritDelRange(self) (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_delrange) && 1)
#define DeeType_InheritSetRange(self) (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_setrange) && 1)
#define DeeType_InheritWith(self)     (DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_enter) && DeeType_GetNativeOperatorWithoutUnsupported(self, Dee_TNO_leave))

/* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS: Backward compat -- DEPRECATED! */
#define DeeType_InvokeCastStr(tp_self, self)                             (*maketyped__str((tp_self)->tp_cast.tp_str))(tp_self, self)
#define DeeType_InvokeCastPrint(tp_self, self, printer, arg)             (*maketyped__print((tp_self)->tp_cast.tp_print))(tp_self, self, printer, arg)
#define DeeType_InvokeCastRepr(tp_self, self)                            (*maketyped__repr((tp_self)->tp_cast.tp_repr))(tp_self, self)
#define DeeType_InvokeCastPrintRepr(tp_self, self, printer, arg)         (*maketyped__printrepr((tp_self)->tp_cast.tp_printrepr))(tp_self, self, printer, arg)
#define DeeType_InvokeCastBool(tp_self, self)                            (*maketyped__bool((tp_self)->tp_cast.tp_bool))(tp_self, self)
#define DeeType_InvokeCall(tp_self, self, argc, argv)                    (*maketyped__call((tp_self)->tp_call))(tp_self, self, argc, argv)
#define DeeType_InvokeCallKw(tp_self, self, argc, argv, kw)              (*maketyped__call_kw((tp_self)->tp_call_kw))(tp_self, self, argc, argv, kw)
#define DeeType_InvokeMathInt32(tp_self, self, result)                   (*maketyped__int32((tp_self)->tp_math->tp_int32))(tp_self, self, result)
#define DeeType_InvokeMathInt64(tp_self, self, result)                   (*maketyped__int64((tp_self)->tp_math->tp_int64))(tp_self, self, result)
#define DeeType_InvokeMathDouble(tp_self, self, result)                  (*maketyped__double((tp_self)->tp_math->tp_double))(tp_self, self, result)
#define DeeType_InvokeMathInt(tp_self, self)                             (*maketyped__int((tp_self)->tp_math->tp_int))(tp_self, self)
#define DeeType_InvokeCmpHash(tp_self, self)                             (*maketyped__hash((tp_self)->tp_cmp->tp_hash))(tp_self, self)
#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_OPERATOR_HINTS_H */
