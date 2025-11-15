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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>

#include <hybrid/host.h>

/**/
#include "operator-hint-errors.h"
#include "runtime_error.h"

/**/
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* intN_t */

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printNativeOperatorExportedApi from "..method-hints.method-hints")();]]]*/
PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_Assign)(DeeObject *self, DeeObject *value) {
	__register DeeNO_assign_t tp_assign;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely((tp_assign = tp_self->tp_init.tp_assign) == NULL)
		tp_assign = _DeeType_RequireNativeOperator(tp_self, assign);
	return (*tp_assign)(self, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_MoveAssign)(DeeObject *self, DeeObject *value) {
	__register DeeNO_move_assign_t tp_move_assign;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely((tp_move_assign = tp_self->tp_init.tp_move_assign) == NULL)
		tp_move_assign = _DeeType_RequireNativeOperator(tp_self, move_assign);
	return (*tp_move_assign)(self, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_Bool)(DeeObject *__restrict self) {
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
#ifndef __OPTIMIZE_SIZE__
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type)
		return self == Dee_True ? 1 : 0;
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	return (*tp_bool)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Call)(DeeObject *self, size_t argc, DeeObject *const *argv) {
	__register DeeNO_call_t tp_call;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely((tp_call = tp_self->tp_call) == NULL)
		tp_call = _DeeType_RequireNativeOperator(tp_self, call);
	return (*tp_call)(self, argc, argv);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_CallKw)(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	__register DeeNO_call_kw_t tp_call_kw;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_call_kw = tp_self->tp_callable->tp_call_kw) == NULL)
		tp_call_kw = _DeeType_RequireNativeOperator(tp_self, call_kw);
	return (*tp_call_kw)(self, argc, argv, kw);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_ThisCall)(DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv) {
	__register DeeNO_thiscall_t tp_thiscall;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_thiscall = tp_self->tp_callable->tp_thiscall) == NULL)
		tp_thiscall = _DeeType_RequireNativeOperator(tp_self, thiscall);
	return (*tp_thiscall)(self, thisarg, argc, argv);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_ThisCallKw)(DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	__register DeeNO_thiscall_kw_t tp_thiscall_kw;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_thiscall_kw = tp_self->tp_callable->tp_thiscall_kw) == NULL)
		tp_thiscall_kw = _DeeType_RequireNativeOperator(tp_self, thiscall_kw);
	return (*tp_thiscall_kw)(self, thisarg, argc, argv, kw);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTuple)(DeeObject *self, DeeObject *args) {
	__register DeeNO_call_tuple_t tp_call_tuple;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_call_tuple = tp_self->tp_callable->tp_call_tuple) == NULL)
		tp_call_tuple = _DeeType_RequireNativeOperator(tp_self, call_tuple);
	return (*tp_call_tuple)(self, args);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleKw)(DeeObject *self, DeeObject *args, DeeObject *kw) {
	__register DeeNO_call_tuple_kw_t tp_call_tuple_kw;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_call_tuple_kw = tp_self->tp_callable->tp_call_tuple_kw) == NULL)
		tp_call_tuple_kw = _DeeType_RequireNativeOperator(tp_self, call_tuple_kw);
	return (*tp_call_tuple_kw)(self, args, kw);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTuple)(DeeObject *self, DeeObject *thisarg, DeeObject *args) {
	__register DeeNO_thiscall_tuple_t tp_thiscall_tuple;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_thiscall_tuple = tp_self->tp_callable->tp_thiscall_tuple) == NULL)
		tp_thiscall_tuple = _DeeType_RequireNativeOperator(tp_self, thiscall_tuple);
	return (*tp_thiscall_tuple)(self, thisarg, args);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleKw)(DeeObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw) {
	__register DeeNO_thiscall_tuple_kw_t tp_thiscall_tuple_kw;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_callable || (tp_thiscall_tuple_kw = tp_self->tp_callable->tp_thiscall_tuple_kw) == NULL)
		tp_thiscall_tuple_kw = _DeeType_RequireNativeOperator(tp_self, thiscall_tuple_kw);
	return (*tp_thiscall_tuple_kw)(self, thisarg, args, kw);
}

#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_IterNext)(DeeObject *__restrict self) {
	__register DeeNO_iter_next_t tp_iter_next;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely((tp_iter_next = tp_self->tp_iter_next) == NULL)
		tp_iter_next = _DeeType_RequireNativeOperator(tp_self, iter_next);
	return (*tp_iter_next)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_IterNextPair)(DeeObject *__restrict self, DREF DeeObject *key_and_value[2]) {
	__register DeeNO_nextpair_t tp_nextpair;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_iterator || (tp_nextpair = tp_self->tp_iterator->tp_nextpair) == NULL)
		tp_nextpair = _DeeType_RequireNativeOperator(tp_self, nextpair);
	return (*tp_nextpair)(self, key_and_value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_IterNextKey)(DeeObject *__restrict self) {
	__register DeeNO_nextkey_t tp_nextkey;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_iterator || (tp_nextkey = tp_self->tp_iterator->tp_nextkey) == NULL)
		tp_nextkey = _DeeType_RequireNativeOperator(tp_self, nextkey);
	return (*tp_nextkey)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_IterNextValue)(DeeObject *__restrict self) {
	__register DeeNO_nextvalue_t tp_nextvalue;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_iterator || (tp_nextvalue = tp_self->tp_iterator->tp_nextvalue) == NULL)
		tp_nextvalue = _DeeType_RequireNativeOperator(tp_self, nextvalue);
	return (*tp_nextvalue)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) size_t
(DCALL DeeObject_IterAdvance)(DeeObject *__restrict self, size_t step) {
	__register DeeNO_advance_t tp_advance;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_iterator || (tp_advance = tp_self->tp_iterator->tp_advance) == NULL)
		tp_advance = _DeeType_RequireNativeOperator(tp_self, advance);
	return (*tp_advance)(self, step);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Int)(DeeObject *__restrict self) {
	__register DeeNO_int_t tp_int;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_int = tp_self->tp_math->tp_int) == NULL)
		tp_int = _DeeType_RequireNativeOperator(tp_self, int);
	return (*tp_int)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_Get32Bit)(DeeObject *__restrict self, int32_t *__restrict p_result) {
	__register DeeNO_int32_t tp_int32;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_int32 = tp_self->tp_math->tp_int32) == NULL)
		tp_int32 = _DeeType_RequireNativeOperator(tp_self, int32);
	return (*tp_int32)(self, p_result);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_Get64Bit)(DeeObject *__restrict self, int64_t *__restrict p_result) {
	__register DeeNO_int64_t tp_int64;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_int64 = tp_self->tp_math->tp_int64) == NULL)
		tp_int64 = _DeeType_RequireNativeOperator(tp_self, int64);
	return (*tp_int64)(self, p_result);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AsDouble)(DeeObject *__restrict self, double *__restrict p_result) {
	__register DeeNO_double_t tp_double;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_double = tp_self->tp_math->tp_double) == NULL)
		tp_double = _DeeType_RequireNativeOperator(tp_self, double);
	return (*tp_double)(self, p_result);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_CompareEq)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_compare_eq_t tp_compare_eq;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_compare_eq = tp_self->tp_cmp->tp_compare_eq) == NULL)
		tp_compare_eq = _DeeType_RequireNativeOperator(tp_self, compare_eq);
	return (*tp_compare_eq)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_Compare)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_compare_t tp_compare;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_compare = tp_self->tp_cmp->tp_compare) == NULL)
		tp_compare = _DeeType_RequireNativeOperator(tp_self, compare);
	return (*tp_compare)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TryCompareEq)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_trycompare_eq_t tp_trycompare_eq;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_trycompare_eq = tp_self->tp_cmp->tp_trycompare_eq) == NULL)
		tp_trycompare_eq = _DeeType_RequireNativeOperator(tp_self, trycompare_eq);
	return (*tp_trycompare_eq)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpEq)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_eq_t tp_eq;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_eq = tp_self->tp_cmp->tp_eq) == NULL)
		tp_eq = _DeeType_RequireNativeOperator(tp_self, eq);
	return (*tp_eq)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpNe)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_ne_t tp_ne;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_ne = tp_self->tp_cmp->tp_ne) == NULL)
		tp_ne = _DeeType_RequireNativeOperator(tp_self, ne);
	return (*tp_ne)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpLo)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_lo_t tp_lo;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_lo = tp_self->tp_cmp->tp_lo) == NULL)
		tp_lo = _DeeType_RequireNativeOperator(tp_self, lo);
	return (*tp_lo)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpLe)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_le_t tp_le;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_le = tp_self->tp_cmp->tp_le) == NULL)
		tp_le = _DeeType_RequireNativeOperator(tp_self, le);
	return (*tp_le)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpGr)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_gr_t tp_gr;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_gr = tp_self->tp_cmp->tp_gr) == NULL)
		tp_gr = _DeeType_RequireNativeOperator(tp_self, gr);
	return (*tp_gr)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CmpGe)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_ge_t tp_ge;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_cmp || (tp_ge = tp_self->tp_cmp->tp_ge) == NULL)
		tp_ge = _DeeType_RequireNativeOperator(tp_self, ge);
	return (*tp_ge)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Iter)(DeeObject *__restrict self) {
	__register DeeNO_iter_t tp_iter;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_iter = tp_self->tp_seq->tp_iter) == NULL)
		tp_iter = _DeeType_RequireNativeOperator(tp_self, iter);
	return (*tp_iter)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeObject_Foreach)(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	__register DeeNO_foreach_t tp_foreach;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_foreach = tp_self->tp_seq->tp_foreach) == NULL)
		tp_foreach = _DeeType_RequireNativeOperator(tp_self, foreach);
	return (*tp_foreach)(self, cb, arg);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) Dee_ssize_t
(DCALL DeeObject_ForeachPair)(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	__register DeeNO_foreach_pair_t tp_foreach_pair;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_foreach_pair = tp_self->tp_seq->tp_foreach_pair) == NULL)
		tp_foreach_pair = _DeeType_RequireNativeOperator(tp_self, foreach_pair);
	return (*tp_foreach_pair)(self, cb, arg);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_SizeOb)(DeeObject *__restrict self) {
	__register DeeNO_sizeob_t tp_sizeob;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_sizeob = tp_self->tp_seq->tp_sizeob) == NULL)
		tp_sizeob = _DeeType_RequireNativeOperator(tp_self, sizeob);
	return (*tp_sizeob)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) size_t
(DCALL DeeObject_Size)(DeeObject *__restrict self) {
	__register DeeNO_size_t tp_size;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_size = tp_self->tp_seq->tp_size) == NULL)
		tp_size = _DeeType_RequireNativeOperator(tp_self, size);
	return (*tp_size)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) size_t
(DCALL DeeObject_SizeFast)(DeeObject *__restrict self) {
	__register DeeNO_size_fast_t tp_size_fast;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_size_fast = tp_self->tp_seq->tp_size_fast) == NULL)
		tp_size_fast = _DeeType_RequireNativeOperator(tp_self, size_fast);
	return (*tp_size_fast)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Contains)(DeeObject *self, DeeObject *item) {
	__register DeeNO_contains_t tp_contains;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_contains = tp_self->tp_seq->tp_contains) == NULL)
		tp_contains = _DeeType_RequireNativeOperator(tp_self, contains);
	return (*tp_contains)(self, item);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetItem)(DeeObject *self, DeeObject *index) {
	__register DeeNO_getitem_t tp_getitem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getitem = tp_self->tp_seq->tp_getitem) == NULL)
		tp_getitem = _DeeType_RequireNativeOperator(tp_self, getitem);
	return (*tp_getitem)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_GetItemIndex)(DeeObject *self, size_t index) {
	__register DeeNO_getitem_index_t tp_getitem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getitem_index = tp_self->tp_seq->tp_getitem_index) == NULL)
		tp_getitem_index = _DeeType_RequireNativeOperator(tp_self, getitem_index);
	return (*tp_getitem_index)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_getitem_string_hash_t tp_getitem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getitem_string_hash = tp_self->tp_seq->tp_getitem_string_hash) == NULL)
		tp_getitem_string_hash = _DeeType_RequireNativeOperator(tp_self, getitem_string_hash);
	return (*tp_getitem_string_hash)(self, key, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_getitem_string_len_hash_t tp_getitem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getitem_string_len_hash = tp_self->tp_seq->tp_getitem_string_len_hash) == NULL)
		tp_getitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, getitem_string_len_hash);
	return (*tp_getitem_string_len_hash)(self, key, keylen, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TryGetItem)(DeeObject *self, DeeObject *index) {
	__register DeeNO_trygetitem_t tp_trygetitem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_trygetitem = tp_self->tp_seq->tp_trygetitem) == NULL)
		tp_trygetitem = _DeeType_RequireNativeOperator(tp_self, trygetitem);
	return (*tp_trygetitem)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_TryGetItemIndex)(DeeObject *self, size_t index) {
	__register DeeNO_trygetitem_index_t tp_trygetitem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_index = tp_self->tp_seq->tp_trygetitem_index) == NULL)
		tp_trygetitem_index = _DeeType_RequireNativeOperator(tp_self, trygetitem_index);
	return (*tp_trygetitem_index)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TryGetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_trygetitem_string_hash_t tp_trygetitem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_string_hash = tp_self->tp_seq->tp_trygetitem_string_hash) == NULL)
		tp_trygetitem_string_hash = _DeeType_RequireNativeOperator(tp_self, trygetitem_string_hash);
	return (*tp_trygetitem_string_hash)(self, key, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TryGetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_trygetitem_string_len_hash_t tp_trygetitem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_string_len_hash = tp_self->tp_seq->tp_trygetitem_string_len_hash) == NULL)
		tp_trygetitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, trygetitem_string_len_hash);
	return (*tp_trygetitem_string_len_hash)(self, key, keylen, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItem)(DeeObject *self, DeeObject *index) {
	__register DeeNO_bounditem_t tp_bounditem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_bounditem = tp_self->tp_seq->tp_bounditem) == NULL)
		tp_bounditem = _DeeType_RequireNativeOperator(tp_self, bounditem);
	return (*tp_bounditem)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_BoundItemIndex)(DeeObject *self, size_t index) {
	__register DeeNO_bounditem_index_t tp_bounditem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_bounditem_index = tp_self->tp_seq->tp_bounditem_index) == NULL)
		tp_bounditem_index = _DeeType_RequireNativeOperator(tp_self, bounditem_index);
	return (*tp_bounditem_index)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_bounditem_string_hash_t tp_bounditem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_bounditem_string_hash = tp_self->tp_seq->tp_bounditem_string_hash) == NULL)
		tp_bounditem_string_hash = _DeeType_RequireNativeOperator(tp_self, bounditem_string_hash);
	return (*tp_bounditem_string_hash)(self, key, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_bounditem_string_len_hash_t tp_bounditem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_bounditem_string_len_hash = tp_self->tp_seq->tp_bounditem_string_len_hash) == NULL)
		tp_bounditem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, bounditem_string_len_hash);
	return (*tp_bounditem_string_len_hash)(self, key, keylen, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItem)(DeeObject *self, DeeObject *index) {
	__register DeeNO_hasitem_t tp_hasitem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_hasitem = tp_self->tp_seq->tp_hasitem) == NULL)
		tp_hasitem = _DeeType_RequireNativeOperator(tp_self, hasitem);
	return (*tp_hasitem)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_HasItemIndex)(DeeObject *self, size_t index) {
	__register DeeNO_hasitem_index_t tp_hasitem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_hasitem_index = tp_self->tp_seq->tp_hasitem_index) == NULL)
		tp_hasitem_index = _DeeType_RequireNativeOperator(tp_self, hasitem_index);
	return (*tp_hasitem_index)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_hasitem_string_hash_t tp_hasitem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_hasitem_string_hash = tp_self->tp_seq->tp_hasitem_string_hash) == NULL)
		tp_hasitem_string_hash = _DeeType_RequireNativeOperator(tp_self, hasitem_string_hash);
	return (*tp_hasitem_string_hash)(self, key, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_hasitem_string_len_hash_t tp_hasitem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_hasitem_string_len_hash = tp_self->tp_seq->tp_hasitem_string_len_hash) == NULL)
		tp_hasitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, hasitem_string_len_hash);
	return (*tp_hasitem_string_len_hash)(self, key, keylen, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItem)(DeeObject *self, DeeObject *index) {
	__register DeeNO_delitem_t tp_delitem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delitem = tp_self->tp_seq->tp_delitem) == NULL)
		tp_delitem = _DeeType_RequireNativeOperator(tp_self, delitem);
	return (*tp_delitem)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelItemIndex)(DeeObject *self, size_t index) {
	__register DeeNO_delitem_index_t tp_delitem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delitem_index = tp_self->tp_seq->tp_delitem_index) == NULL)
		tp_delitem_index = _DeeType_RequireNativeOperator(tp_self, delitem_index);
	return (*tp_delitem_index)(self, index);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_delitem_string_hash_t tp_delitem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delitem_string_hash = tp_self->tp_seq->tp_delitem_string_hash) == NULL)
		tp_delitem_string_hash = _DeeType_RequireNativeOperator(tp_self, delitem_string_hash);
	return (*tp_delitem_string_hash)(self, key, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_delitem_string_len_hash_t tp_delitem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delitem_string_len_hash = tp_self->tp_seq->tp_delitem_string_len_hash) == NULL)
		tp_delitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, delitem_string_len_hash);
	return (*tp_delitem_string_len_hash)(self, key, keylen, hash);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_SetItem)(DeeObject *self, DeeObject *index, DeeObject *value) {
	__register DeeNO_setitem_t tp_setitem;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setitem = tp_self->tp_seq->tp_setitem) == NULL)
		tp_setitem = _DeeType_RequireNativeOperator(tp_self, setitem);
	return (*tp_setitem)(self, index, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_SetItemIndex)(DeeObject *self, size_t index, DeeObject *value) {
	__register DeeNO_setitem_index_t tp_setitem_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setitem_index = tp_self->tp_seq->tp_setitem_index) == NULL)
		tp_setitem_index = _DeeType_RequireNativeOperator(tp_self, setitem_index);
	return (*tp_setitem_index)(self, index, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetItemStringHash)(DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	__register DeeNO_setitem_string_hash_t tp_setitem_string_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setitem_string_hash = tp_self->tp_seq->tp_setitem_string_hash) == NULL)
		tp_setitem_string_hash = _DeeType_RequireNativeOperator(tp_self, setitem_string_hash);
	return (*tp_setitem_string_hash)(self, key, hash, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeObject_SetItemStringLenHash)(DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	__register DeeNO_setitem_string_len_hash_t tp_setitem_string_len_hash;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setitem_string_len_hash = tp_self->tp_seq->tp_setitem_string_len_hash) == NULL)
		tp_setitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, setitem_string_len_hash);
	return (*tp_setitem_string_len_hash)(self, key, keylen, hash, value);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_GetRange)(DeeObject *self, DeeObject *start, DeeObject *end) {
	__register DeeNO_getrange_t tp_getrange;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getrange = tp_self->tp_seq->tp_getrange) == NULL)
		tp_getrange = _DeeType_RequireNativeOperator(tp_self, getrange);
	return (*tp_getrange)(self, start, end);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_GetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	__register DeeNO_getrange_index_t tp_getrange_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getrange_index = tp_self->tp_seq->tp_getrange_index) == NULL)
		tp_getrange_index = _DeeType_RequireNativeOperator(tp_self, getrange_index);
	return (*tp_getrange_index)(self, start, end);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_GetRangeIndexN)(DeeObject *self, Dee_ssize_t start) {
	__register DeeNO_getrange_index_n_t tp_getrange_index_n;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_getrange_index_n = tp_self->tp_seq->tp_getrange_index_n) == NULL)
		tp_getrange_index_n = _DeeType_RequireNativeOperator(tp_self, getrange_index_n);
	return (*tp_getrange_index_n)(self, start);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_DelRange)(DeeObject *self, DeeObject *start, DeeObject *end) {
	__register DeeNO_delrange_t tp_delrange;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delrange = tp_self->tp_seq->tp_delrange) == NULL)
		tp_delrange = _DeeType_RequireNativeOperator(tp_self, delrange);
	return (*tp_delrange)(self, start, end);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	__register DeeNO_delrange_index_t tp_delrange_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delrange_index = tp_self->tp_seq->tp_delrange_index) == NULL)
		tp_delrange_index = _DeeType_RequireNativeOperator(tp_self, delrange_index);
	return (*tp_delrange_index)(self, start, end);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_DelRangeIndexN)(DeeObject *self, Dee_ssize_t start) {
	__register DeeNO_delrange_index_n_t tp_delrange_index_n;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_delrange_index_n = tp_self->tp_seq->tp_delrange_index_n) == NULL)
		tp_delrange_index_n = _DeeType_RequireNativeOperator(tp_self, delrange_index_n);
	return (*tp_delrange_index_n)(self, start);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeObject_SetRange)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	__register DeeNO_setrange_t tp_setrange;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setrange = tp_self->tp_seq->tp_setrange) == NULL)
		tp_setrange = _DeeType_RequireNativeOperator(tp_self, setrange);
	return (*tp_setrange)(self, start, end, values);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 4)) int
(DCALL DeeObject_SetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	__register DeeNO_setrange_index_t tp_setrange_index;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setrange_index = tp_self->tp_seq->tp_setrange_index) == NULL)
		tp_setrange_index = _DeeType_RequireNativeOperator(tp_self, setrange_index);
	return (*tp_setrange_index)(self, start, end, values);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_SetRangeIndexN)(DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	__register DeeNO_setrange_index_n_t tp_setrange_index_n;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_seq || (tp_setrange_index_n = tp_self->tp_seq->tp_setrange_index_n) == NULL)
		tp_setrange_index_n = _DeeType_RequireNativeOperator(tp_self, setrange_index_n);
	return (*tp_setrange_index_n)(self, start, values);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Inv)(DeeObject *self) {
	__register DeeNO_inv_t tp_inv;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_inv = tp_self->tp_math->tp_inv) == NULL)
		tp_inv = _DeeType_RequireNativeOperator(tp_self, inv);
	return (*tp_inv)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Pos)(DeeObject *self) {
	__register DeeNO_pos_t tp_pos;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_pos = tp_self->tp_math->tp_pos) == NULL)
		tp_pos = _DeeType_RequireNativeOperator(tp_self, pos);
	return (*tp_pos)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_Neg)(DeeObject *self) {
	__register DeeNO_neg_t tp_neg;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_math || (tp_neg = tp_self->tp_math->tp_neg) == NULL)
		tp_neg = _DeeType_RequireNativeOperator(tp_self, neg);
	return (*tp_neg)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Add)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_add_t tp_add;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_add = tp_self->tp_math->tp_add) == NULL)
		tp_add = _DeeType_RequireNativeOperator(tp_self, add);
	return (*tp_add)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceAdd)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_add_t tp_inplace_add;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_add = tp_self->tp_math->tp_inplace_add) == NULL)
		tp_inplace_add = _DeeType_RequireNativeOperator(tp_self, inplace_add);
	return (*tp_inplace_add)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Sub)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_sub_t tp_sub;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_sub = tp_self->tp_math->tp_sub) == NULL)
		tp_sub = _DeeType_RequireNativeOperator(tp_self, sub);
	return (*tp_sub)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceSub)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_sub_t tp_inplace_sub;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_sub = tp_self->tp_math->tp_inplace_sub) == NULL)
		tp_inplace_sub = _DeeType_RequireNativeOperator(tp_self, inplace_sub);
	return (*tp_inplace_sub)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Mul)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_mul_t tp_mul;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_mul = tp_self->tp_math->tp_mul) == NULL)
		tp_mul = _DeeType_RequireNativeOperator(tp_self, mul);
	return (*tp_mul)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceMul)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_mul_t tp_inplace_mul;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_mul = tp_self->tp_math->tp_inplace_mul) == NULL)
		tp_inplace_mul = _DeeType_RequireNativeOperator(tp_self, inplace_mul);
	return (*tp_inplace_mul)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Div)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_div_t tp_div;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_div = tp_self->tp_math->tp_div) == NULL)
		tp_div = _DeeType_RequireNativeOperator(tp_self, div);
	return (*tp_div)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceDiv)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_div_t tp_inplace_div;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_div = tp_self->tp_math->tp_inplace_div) == NULL)
		tp_inplace_div = _DeeType_RequireNativeOperator(tp_self, inplace_div);
	return (*tp_inplace_div)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Mod)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_mod_t tp_mod;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_mod = tp_self->tp_math->tp_mod) == NULL)
		tp_mod = _DeeType_RequireNativeOperator(tp_self, mod);
	return (*tp_mod)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceMod)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_mod_t tp_inplace_mod;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_mod = tp_self->tp_math->tp_inplace_mod) == NULL)
		tp_inplace_mod = _DeeType_RequireNativeOperator(tp_self, inplace_mod);
	return (*tp_inplace_mod)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Shl)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_shl_t tp_shl;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_shl = tp_self->tp_math->tp_shl) == NULL)
		tp_shl = _DeeType_RequireNativeOperator(tp_self, shl);
	return (*tp_shl)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceShl)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_shl_t tp_inplace_shl;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_shl = tp_self->tp_math->tp_inplace_shl) == NULL)
		tp_inplace_shl = _DeeType_RequireNativeOperator(tp_self, inplace_shl);
	return (*tp_inplace_shl)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Shr)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_shr_t tp_shr;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_shr = tp_self->tp_math->tp_shr) == NULL)
		tp_shr = _DeeType_RequireNativeOperator(tp_self, shr);
	return (*tp_shr)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceShr)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_shr_t tp_inplace_shr;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_shr = tp_self->tp_math->tp_inplace_shr) == NULL)
		tp_inplace_shr = _DeeType_RequireNativeOperator(tp_self, inplace_shr);
	return (*tp_inplace_shr)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_And)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_and_t tp_and;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_and = tp_self->tp_math->tp_and) == NULL)
		tp_and = _DeeType_RequireNativeOperator(tp_self, and);
	return (*tp_and)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceAnd)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_and_t tp_inplace_and;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_and = tp_self->tp_math->tp_inplace_and) == NULL)
		tp_inplace_and = _DeeType_RequireNativeOperator(tp_self, inplace_and);
	return (*tp_inplace_and)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Or)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_or_t tp_or;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_or = tp_self->tp_math->tp_or) == NULL)
		tp_or = _DeeType_RequireNativeOperator(tp_self, or);
	return (*tp_or)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceOr)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_or_t tp_inplace_or;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_or = tp_self->tp_math->tp_inplace_or) == NULL)
		tp_inplace_or = _DeeType_RequireNativeOperator(tp_self, inplace_or);
	return (*tp_inplace_or)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Xor)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_xor_t tp_xor;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_xor = tp_self->tp_math->tp_xor) == NULL)
		tp_xor = _DeeType_RequireNativeOperator(tp_self, xor);
	return (*tp_xor)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceXor)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_xor_t tp_inplace_xor;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_xor = tp_self->tp_math->tp_inplace_xor) == NULL)
		tp_inplace_xor = _DeeType_RequireNativeOperator(tp_self, inplace_xor);
	return (*tp_inplace_xor)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_Pow)(DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_pow_t tp_pow;
	__register DeeTypeObject *tp_self = Dee_TYPE(lhs);
	if unlikely(!tp_self->tp_math || (tp_pow = tp_self->tp_math->tp_pow) == NULL)
		tp_pow = _DeeType_RequireNativeOperator(tp_self, pow);
	return (*tp_pow)(lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplacePow)(DREF DeeObject **__restrict p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_pow_t tp_inplace_pow;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_lhs);
	if unlikely(!tp_self->tp_math || (tp_inplace_pow = tp_self->tp_math->tp_inplace_pow) == NULL)
		tp_inplace_pow = _DeeType_RequireNativeOperator(tp_self, inplace_pow);
	return (*tp_inplace_pow)(p_lhs, rhs);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_Inc)(DREF DeeObject **__restrict p_self) {
	__register DeeNO_inc_t tp_inc;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_self);
	if unlikely(!tp_self->tp_math || (tp_inc = tp_self->tp_math->tp_inc) == NULL)
		tp_inc = _DeeType_RequireNativeOperator(tp_self, inc);
	return (*tp_inc)(p_self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_Dec)(DREF DeeObject **__restrict p_self) {
	__register DeeNO_dec_t tp_dec;
	__register DeeTypeObject *tp_self = Dee_TYPE(*p_self);
	if unlikely(!tp_self->tp_math || (tp_dec = tp_self->tp_math->tp_dec) == NULL)
		tp_dec = _DeeType_RequireNativeOperator(tp_self, dec);
	return (*tp_dec)(p_self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_Enter)(DeeObject *__restrict self) {
	__register DeeNO_enter_t tp_enter;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_with || (tp_enter = tp_self->tp_with->tp_enter) == NULL)
		tp_enter = _DeeType_RequireNativeOperator(tp_self, enter);
	return (*tp_enter)(self);
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int
(DCALL DeeObject_Leave)(DeeObject *__restrict self) {
	__register DeeNO_leave_t tp_leave;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if unlikely(!tp_self->tp_with || (tp_leave = tp_self->tp_with->tp_leave) == NULL)
		tp_leave = _DeeType_RequireNativeOperator(tp_self, leave);
	return (*tp_leave)(self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TAssign)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	__register DeeNO_assign_t tp_assign;
	if unlikely((tp_assign = tp_self->tp_init.tp_assign) == NULL) {
		tp_assign = _DeeType_RequireNativeOperator(tp_self, assign);
		if unlikely(tp_assign == (DeeNO_assign_t)&default__assign__unsupported)
			return (*tp_assign)(self, value);
	}
	return (*maketyped__assign(tp_assign))(tp_self, self, value);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TMoveAssign)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *value) {
	__register DeeNO_move_assign_t tp_move_assign;
	if unlikely((tp_move_assign = tp_self->tp_init.tp_move_assign) == NULL) {
		tp_move_assign = _DeeType_RequireNativeOperator(tp_self, move_assign);
		if unlikely(tp_move_assign == (DeeNO_move_assign_t)&default__move_assign__unsupported)
			return (*tp_move_assign)(self, value);
	}
	return (*maketyped__move_assign(tp_move_assign))(tp_self, self, value);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TBool)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_bool_t tp_bool;
#ifndef __OPTIMIZE_SIZE__
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type)
		return self == Dee_True ? 1 : 0;
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL) {
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
		if unlikely(tp_bool == (DeeNO_bool_t)&default__bool__unsupported)
			return (*tp_bool)(self);
	}
	return (*maketyped__bool(tp_bool))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TCall)(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv) {
	__register DeeNO_call_t tp_call;
	if unlikely((tp_call = tp_self->tp_call) == NULL) {
		tp_call = _DeeType_RequireNativeOperator(tp_self, call);
		if unlikely(tp_call == (DeeNO_call_t)&default__call__unsupported)
			return (*tp_call)(self, argc, argv);
	}
	return (*maketyped__call(tp_call))(tp_self, self, argc, argv);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TCallKw)(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	__register DeeNO_call_kw_t tp_call_kw;
	if unlikely(!tp_self->tp_callable || (tp_call_kw = tp_self->tp_callable->tp_call_kw) == NULL) {
		tp_call_kw = _DeeType_RequireNativeOperator(tp_self, call_kw);
		if unlikely(tp_call_kw == (DeeNO_call_kw_t)&default__call_kw__badalloc ||
		            tp_call_kw == (DeeNO_call_kw_t)&default__call_kw__unsupported)
			return (*tp_call_kw)(self, argc, argv, kw);
	}
	return (*maketyped__call_kw(tp_call_kw))(tp_self, self, argc, argv, kw);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TThisCall)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv) {
	__register DeeNO_thiscall_t tp_thiscall;
	if unlikely(!tp_self->tp_callable || (tp_thiscall = tp_self->tp_callable->tp_thiscall) == NULL) {
		tp_thiscall = _DeeType_RequireNativeOperator(tp_self, thiscall);
		if unlikely(tp_thiscall == (DeeNO_thiscall_t)&default__thiscall__badalloc ||
		            tp_thiscall == (DeeNO_thiscall_t)&default__thiscall__unsupported)
			return (*tp_thiscall)(self, thisarg, argc, argv);
	}
	return (*maketyped__thiscall(tp_thiscall))(tp_self, self, thisarg, argc, argv);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TThisCallKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	__register DeeNO_thiscall_kw_t tp_thiscall_kw;
	if unlikely(!tp_self->tp_callable || (tp_thiscall_kw = tp_self->tp_callable->tp_thiscall_kw) == NULL) {
		tp_thiscall_kw = _DeeType_RequireNativeOperator(tp_self, thiscall_kw);
		if unlikely(tp_thiscall_kw == (DeeNO_thiscall_kw_t)&default__thiscall_kw__badalloc ||
		            tp_thiscall_kw == (DeeNO_thiscall_kw_t)&default__thiscall_kw__unsupported)
			return (*tp_thiscall_kw)(self, thisarg, argc, argv, kw);
	}
	return (*maketyped__thiscall_kw(tp_thiscall_kw))(tp_self, self, thisarg, argc, argv, kw);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args) {
	__register DeeNO_call_tuple_t tp_call_tuple;
	if unlikely(!tp_self->tp_callable || (tp_call_tuple = tp_self->tp_callable->tp_call_tuple) == NULL) {
		tp_call_tuple = _DeeType_RequireNativeOperator(tp_self, call_tuple);
		if unlikely(tp_call_tuple == (DeeNO_call_tuple_t)&default__call_tuple__badalloc ||
		            tp_call_tuple == (DeeNO_call_tuple_t)&default__call_tuple__unsupported)
			return (*tp_call_tuple)(self, args);
	}
	return (*maketyped__call_tuple(tp_call_tuple))(tp_self, self, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args, DeeObject *kw) {
	__register DeeNO_call_tuple_kw_t tp_call_tuple_kw;
	if unlikely(!tp_self->tp_callable || (tp_call_tuple_kw = tp_self->tp_callable->tp_call_tuple_kw) == NULL) {
		tp_call_tuple_kw = _DeeType_RequireNativeOperator(tp_self, call_tuple_kw);
		if unlikely(tp_call_tuple_kw == (DeeNO_call_tuple_kw_t)&default__call_tuple_kw__badalloc ||
		            tp_call_tuple_kw == (DeeNO_call_tuple_kw_t)&default__call_tuple_kw__unsupported)
			return (*tp_call_tuple_kw)(self, args, kw);
	}
	return (*maketyped__call_tuple_kw(tp_call_tuple_kw))(tp_self, self, args, kw);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TThisCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args) {
	__register DeeNO_thiscall_tuple_t tp_thiscall_tuple;
	if unlikely(!tp_self->tp_callable || (tp_thiscall_tuple = tp_self->tp_callable->tp_thiscall_tuple) == NULL) {
		tp_thiscall_tuple = _DeeType_RequireNativeOperator(tp_self, thiscall_tuple);
		if unlikely(tp_thiscall_tuple == (DeeNO_thiscall_tuple_t)&default__thiscall_tuple__badalloc ||
		            tp_thiscall_tuple == (DeeNO_thiscall_tuple_t)&default__thiscall_tuple__unsupported)
			return (*tp_thiscall_tuple)(self, thisarg, args);
	}
	return (*maketyped__thiscall_tuple(tp_thiscall_tuple))(tp_self, self, thisarg, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TThisCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw) {
	__register DeeNO_thiscall_tuple_kw_t tp_thiscall_tuple_kw;
	if unlikely(!tp_self->tp_callable || (tp_thiscall_tuple_kw = tp_self->tp_callable->tp_thiscall_tuple_kw) == NULL) {
		tp_thiscall_tuple_kw = _DeeType_RequireNativeOperator(tp_self, thiscall_tuple_kw);
		if unlikely(tp_thiscall_tuple_kw == (DeeNO_thiscall_tuple_kw_t)&default__thiscall_tuple_kw__badalloc ||
		            tp_thiscall_tuple_kw == (DeeNO_thiscall_tuple_kw_t)&default__thiscall_tuple_kw__unsupported)
			return (*tp_thiscall_tuple_kw)(self, thisarg, args, kw);
	}
	return (*maketyped__thiscall_tuple_kw(tp_thiscall_tuple_kw))(tp_self, self, thisarg, args, kw);
}

#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TIterNext)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_iter_next_t tp_iter_next;
	if unlikely((tp_iter_next = tp_self->tp_iter_next) == NULL) {
		tp_iter_next = _DeeType_RequireNativeOperator(tp_self, iter_next);
		if unlikely(tp_iter_next == (DeeNO_iter_next_t)&default__iter_next__unsupported)
			return (*tp_iter_next)(self);
	}
	return (*maketyped__iter_next(tp_iter_next))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TIterNextPair)(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]) {
	__register DeeNO_nextpair_t tp_nextpair;
	if unlikely(!tp_self->tp_iterator || (tp_nextpair = tp_self->tp_iterator->tp_nextpair) == NULL) {
		tp_nextpair = _DeeType_RequireNativeOperator(tp_self, nextpair);
		if unlikely(tp_nextpair == (DeeNO_nextpair_t)&default__nextpair__badalloc ||
		            tp_nextpair == (DeeNO_nextpair_t)&default__nextpair__unsupported)
			return (*tp_nextpair)(self, key_and_value);
	}
	return (*maketyped__nextpair(tp_nextpair))(tp_self, self, key_and_value);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TIterNextKey)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_nextkey_t tp_nextkey;
	if unlikely(!tp_self->tp_iterator || (tp_nextkey = tp_self->tp_iterator->tp_nextkey) == NULL) {
		tp_nextkey = _DeeType_RequireNativeOperator(tp_self, nextkey);
		if unlikely(tp_nextkey == (DeeNO_nextkey_t)&default__nextkey__badalloc ||
		            tp_nextkey == (DeeNO_nextkey_t)&default__nextkey__unsupported)
			return (*tp_nextkey)(self);
	}
	return (*maketyped__nextkey(tp_nextkey))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TIterNextValue)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_nextvalue_t tp_nextvalue;
	if unlikely(!tp_self->tp_iterator || (tp_nextvalue = tp_self->tp_iterator->tp_nextvalue) == NULL) {
		tp_nextvalue = _DeeType_RequireNativeOperator(tp_self, nextvalue);
		if unlikely(tp_nextvalue == (DeeNO_nextvalue_t)&default__nextvalue__badalloc ||
		            tp_nextvalue == (DeeNO_nextvalue_t)&default__nextvalue__unsupported)
			return (*tp_nextvalue)(self);
	}
	return (*maketyped__nextvalue(tp_nextvalue))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) size_t
(DCALL DeeObject_TIterAdvance)(DeeTypeObject *tp_self, DeeObject *self, size_t step) {
	__register DeeNO_advance_t tp_advance;
	if unlikely(!tp_self->tp_iterator || (tp_advance = tp_self->tp_iterator->tp_advance) == NULL) {
		tp_advance = _DeeType_RequireNativeOperator(tp_self, advance);
		if unlikely(tp_advance == (DeeNO_advance_t)&default__advance__badalloc ||
		            tp_advance == (DeeNO_advance_t)&default__advance__unsupported)
			return (*tp_advance)(self, step);
	}
	return (*maketyped__advance(tp_advance))(tp_self, self, step);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TInt)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_int_t tp_int;
	if unlikely(!tp_self->tp_math || (tp_int = tp_self->tp_math->tp_int) == NULL) {
		tp_int = _DeeType_RequireNativeOperator(tp_self, int);
		if unlikely(tp_int == (DeeNO_int_t)&default__int__badalloc ||
		            tp_int == (DeeNO_int_t)&default__int__unsupported)
			return (*tp_int)(self);
	}
	return (*maketyped__int(tp_int))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TGet32Bit)(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result) {
	__register DeeNO_int32_t tp_int32;
	if unlikely(!tp_self->tp_math || (tp_int32 = tp_self->tp_math->tp_int32) == NULL) {
		tp_int32 = _DeeType_RequireNativeOperator(tp_self, int32);
		if unlikely(tp_int32 == (DeeNO_int32_t)&default__int32__badalloc ||
		            tp_int32 == (DeeNO_int32_t)&default__int32__unsupported)
			return (*tp_int32)(self, p_result);
	}
	return (*maketyped__int32(tp_int32))(tp_self, self, p_result);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TGet64Bit)(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result) {
	__register DeeNO_int64_t tp_int64;
	if unlikely(!tp_self->tp_math || (tp_int64 = tp_self->tp_math->tp_int64) == NULL) {
		tp_int64 = _DeeType_RequireNativeOperator(tp_self, int64);
		if unlikely(tp_int64 == (DeeNO_int64_t)&default__int64__badalloc ||
		            tp_int64 == (DeeNO_int64_t)&default__int64__unsupported)
			return (*tp_int64)(self, p_result);
	}
	return (*maketyped__int64(tp_int64))(tp_self, self, p_result);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TAsDouble)(DeeTypeObject *tp_self, DeeObject *self, double *p_result) {
	__register DeeNO_double_t tp_double;
	if unlikely(!tp_self->tp_math || (tp_double = tp_self->tp_math->tp_double) == NULL) {
		tp_double = _DeeType_RequireNativeOperator(tp_self, double);
		if unlikely(tp_double == (DeeNO_double_t)&default__double__badalloc ||
		            tp_double == (DeeNO_double_t)&default__double__unsupported)
			return (*tp_double)(self, p_result);
	}
	return (*maketyped__double(tp_double))(tp_self, self, p_result);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TCompareEq)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_compare_eq_t tp_compare_eq;
	if unlikely(!tp_self->tp_cmp || (tp_compare_eq = tp_self->tp_cmp->tp_compare_eq) == NULL) {
		tp_compare_eq = _DeeType_RequireNativeOperator(tp_self, compare_eq);
		if unlikely(tp_compare_eq == (DeeNO_compare_eq_t)&default__compare_eq__badalloc ||
		            tp_compare_eq == (DeeNO_compare_eq_t)&default__compare_eq__unsupported)
			return (*tp_compare_eq)(lhs, rhs);
	}
	return (*maketyped__compare_eq(tp_compare_eq))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TCompare)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_compare_t tp_compare;
	if unlikely(!tp_self->tp_cmp || (tp_compare = tp_self->tp_cmp->tp_compare) == NULL) {
		tp_compare = _DeeType_RequireNativeOperator(tp_self, compare);
		if unlikely(tp_compare == (DeeNO_compare_t)&default__compare__badalloc ||
		            tp_compare == (DeeNO_compare_t)&default__compare__unsupported)
			return (*tp_compare)(lhs, rhs);
	}
	return (*maketyped__compare(tp_compare))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TTryCompareEq)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_trycompare_eq_t tp_trycompare_eq;
	if unlikely(!tp_self->tp_cmp || (tp_trycompare_eq = tp_self->tp_cmp->tp_trycompare_eq) == NULL) {
		tp_trycompare_eq = _DeeType_RequireNativeOperator(tp_self, trycompare_eq);
		if unlikely(tp_trycompare_eq == (DeeNO_trycompare_eq_t)&default__trycompare_eq__badalloc ||
		            tp_trycompare_eq == (DeeNO_trycompare_eq_t)&default__trycompare_eq__unsupported)
			return (*tp_trycompare_eq)(lhs, rhs);
	}
	return (*maketyped__trycompare_eq(tp_trycompare_eq))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpEq)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_eq_t tp_eq;
	if unlikely(!tp_self->tp_cmp || (tp_eq = tp_self->tp_cmp->tp_eq) == NULL) {
		tp_eq = _DeeType_RequireNativeOperator(tp_self, eq);
		if unlikely(tp_eq == (DeeNO_eq_t)&default__eq__badalloc ||
		            tp_eq == (DeeNO_eq_t)&default__eq__unsupported)
			return (*tp_eq)(lhs, rhs);
	}
	return (*maketyped__eq(tp_eq))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpNe)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_ne_t tp_ne;
	if unlikely(!tp_self->tp_cmp || (tp_ne = tp_self->tp_cmp->tp_ne) == NULL) {
		tp_ne = _DeeType_RequireNativeOperator(tp_self, ne);
		if unlikely(tp_ne == (DeeNO_ne_t)&default__ne__badalloc ||
		            tp_ne == (DeeNO_ne_t)&default__ne__unsupported)
			return (*tp_ne)(lhs, rhs);
	}
	return (*maketyped__ne(tp_ne))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpLo)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_lo_t tp_lo;
	if unlikely(!tp_self->tp_cmp || (tp_lo = tp_self->tp_cmp->tp_lo) == NULL) {
		tp_lo = _DeeType_RequireNativeOperator(tp_self, lo);
		if unlikely(tp_lo == (DeeNO_lo_t)&default__lo__badalloc ||
		            tp_lo == (DeeNO_lo_t)&default__lo__unsupported)
			return (*tp_lo)(lhs, rhs);
	}
	return (*maketyped__lo(tp_lo))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpLe)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_le_t tp_le;
	if unlikely(!tp_self->tp_cmp || (tp_le = tp_self->tp_cmp->tp_le) == NULL) {
		tp_le = _DeeType_RequireNativeOperator(tp_self, le);
		if unlikely(tp_le == (DeeNO_le_t)&default__le__badalloc ||
		            tp_le == (DeeNO_le_t)&default__le__unsupported)
			return (*tp_le)(lhs, rhs);
	}
	return (*maketyped__le(tp_le))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpGr)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_gr_t tp_gr;
	if unlikely(!tp_self->tp_cmp || (tp_gr = tp_self->tp_cmp->tp_gr) == NULL) {
		tp_gr = _DeeType_RequireNativeOperator(tp_self, gr);
		if unlikely(tp_gr == (DeeNO_gr_t)&default__gr__badalloc ||
		            tp_gr == (DeeNO_gr_t)&default__gr__unsupported)
			return (*tp_gr)(lhs, rhs);
	}
	return (*maketyped__gr(tp_gr))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCmpGe)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_ge_t tp_ge;
	if unlikely(!tp_self->tp_cmp || (tp_ge = tp_self->tp_cmp->tp_ge) == NULL) {
		tp_ge = _DeeType_RequireNativeOperator(tp_self, ge);
		if unlikely(tp_ge == (DeeNO_ge_t)&default__ge__badalloc ||
		            tp_ge == (DeeNO_ge_t)&default__ge__unsupported)
			return (*tp_ge)(lhs, rhs);
	}
	return (*maketyped__ge(tp_ge))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TIter)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_iter_t tp_iter;
	if unlikely(!tp_self->tp_seq || (tp_iter = tp_self->tp_seq->tp_iter) == NULL) {
		tp_iter = _DeeType_RequireNativeOperator(tp_self, iter);
		if unlikely(tp_iter == (DeeNO_iter_t)&default__iter__badalloc ||
		            tp_iter == (DeeNO_iter_t)&default__iter__unsupported)
			return (*tp_iter)(self);
	}
	return (*maketyped__iter(tp_iter))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t
(DCALL DeeObject_TForeach)(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg) {
	__register DeeNO_foreach_t tp_foreach;
	if unlikely(!tp_self->tp_seq || (tp_foreach = tp_self->tp_seq->tp_foreach) == NULL) {
		tp_foreach = _DeeType_RequireNativeOperator(tp_self, foreach);
		if unlikely(tp_foreach == (DeeNO_foreach_t)&default__foreach__badalloc ||
		            tp_foreach == (DeeNO_foreach_t)&default__foreach__unsupported)
			return (*tp_foreach)(self, cb, arg);
	}
	return (*maketyped__foreach(tp_foreach))(tp_self, self, cb, arg);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t
(DCALL DeeObject_TForeachPair)(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg) {
	__register DeeNO_foreach_pair_t tp_foreach_pair;
	if unlikely(!tp_self->tp_seq || (tp_foreach_pair = tp_self->tp_seq->tp_foreach_pair) == NULL) {
		tp_foreach_pair = _DeeType_RequireNativeOperator(tp_self, foreach_pair);
		if unlikely(tp_foreach_pair == (DeeNO_foreach_pair_t)&default__foreach_pair__badalloc ||
		            tp_foreach_pair == (DeeNO_foreach_pair_t)&default__foreach_pair__unsupported)
			return (*tp_foreach_pair)(self, cb, arg);
	}
	return (*maketyped__foreach_pair(tp_foreach_pair))(tp_self, self, cb, arg);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TSizeOb)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_sizeob_t tp_sizeob;
	if unlikely(!tp_self->tp_seq || (tp_sizeob = tp_self->tp_seq->tp_sizeob) == NULL) {
		tp_sizeob = _DeeType_RequireNativeOperator(tp_self, sizeob);
		if unlikely(tp_sizeob == (DeeNO_sizeob_t)&default__sizeob__badalloc ||
		            tp_sizeob == (DeeNO_sizeob_t)&default__sizeob__unsupported)
			return (*tp_sizeob)(self);
	}
	return (*maketyped__sizeob(tp_sizeob))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) size_t
(DCALL DeeObject_TSize)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_size_t tp_size;
	if unlikely(!tp_self->tp_seq || (tp_size = tp_self->tp_seq->tp_size) == NULL) {
		tp_size = _DeeType_RequireNativeOperator(tp_self, size);
		if unlikely(tp_size == (DeeNO_size_t)&default__size__badalloc ||
		            tp_size == (DeeNO_size_t)&default__size__unsupported)
			return (*tp_size)(self);
	}
	return (*maketyped__size(tp_size))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) size_t
(DCALL DeeObject_TSizeFast)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_size_fast_t tp_size_fast;
	if unlikely(!tp_self->tp_seq || (tp_size_fast = tp_self->tp_seq->tp_size_fast) == NULL)
		tp_size_fast = _DeeType_RequireNativeOperator(tp_self, size_fast);
	return (*tp_size_fast)(self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TContains)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *item) {
	__register DeeNO_contains_t tp_contains;
	if unlikely(!tp_self->tp_seq || (tp_contains = tp_self->tp_seq->tp_contains) == NULL) {
		tp_contains = _DeeType_RequireNativeOperator(tp_self, contains);
		if unlikely(tp_contains == (DeeNO_contains_t)&default__contains__badalloc ||
		            tp_contains == (DeeNO_contains_t)&default__contains__unsupported)
			return (*tp_contains)(self, item);
	}
	return (*maketyped__contains(tp_contains))(tp_self, self, item);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TGetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	__register DeeNO_getitem_t tp_getitem;
	if unlikely(!tp_self->tp_seq || (tp_getitem = tp_self->tp_seq->tp_getitem) == NULL) {
		tp_getitem = _DeeType_RequireNativeOperator(tp_self, getitem);
		if unlikely(tp_getitem == (DeeNO_getitem_t)&default__getitem__badalloc ||
		            tp_getitem == (DeeNO_getitem_t)&default__getitem__unsupported)
			return (*tp_getitem)(self, index);
	}
	return (*maketyped__getitem(tp_getitem))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TGetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	__register DeeNO_getitem_index_t tp_getitem_index;
	if unlikely(!tp_self->tp_seq || (tp_getitem_index = tp_self->tp_seq->tp_getitem_index) == NULL) {
		tp_getitem_index = _DeeType_RequireNativeOperator(tp_self, getitem_index);
		if unlikely(tp_getitem_index == (DeeNO_getitem_index_t)&default__getitem_index__badalloc ||
		            tp_getitem_index == (DeeNO_getitem_index_t)&default__getitem_index__unsupported)
			return (*tp_getitem_index)(self, index);
	}
	return (*maketyped__getitem_index(tp_getitem_index))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TGetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_getitem_string_hash_t tp_getitem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_getitem_string_hash = tp_self->tp_seq->tp_getitem_string_hash) == NULL) {
		tp_getitem_string_hash = _DeeType_RequireNativeOperator(tp_self, getitem_string_hash);
		if unlikely(tp_getitem_string_hash == (DeeNO_getitem_string_hash_t)&default__getitem_string_hash__badalloc ||
		            tp_getitem_string_hash == (DeeNO_getitem_string_hash_t)&default__getitem_string_hash__unsupported)
			return (*tp_getitem_string_hash)(self, key, hash);
	}
	return (*maketyped__getitem_string_hash(tp_getitem_string_hash))(tp_self, self, key, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TGetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_getitem_string_len_hash_t tp_getitem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_getitem_string_len_hash = tp_self->tp_seq->tp_getitem_string_len_hash) == NULL) {
		tp_getitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, getitem_string_len_hash);
		if unlikely(tp_getitem_string_len_hash == (DeeNO_getitem_string_len_hash_t)&default__getitem_string_len_hash__badalloc ||
		            tp_getitem_string_len_hash == (DeeNO_getitem_string_len_hash_t)&default__getitem_string_len_hash__unsupported)
			return (*tp_getitem_string_len_hash)(self, key, keylen, hash);
	}
	return (*maketyped__getitem_string_len_hash(tp_getitem_string_len_hash))(tp_self, self, key, keylen, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TTryGetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	__register DeeNO_trygetitem_t tp_trygetitem;
	if unlikely(!tp_self->tp_seq || (tp_trygetitem = tp_self->tp_seq->tp_trygetitem) == NULL) {
		tp_trygetitem = _DeeType_RequireNativeOperator(tp_self, trygetitem);
		if unlikely(tp_trygetitem == (DeeNO_trygetitem_t)&default__trygetitem__badalloc ||
		            tp_trygetitem == (DeeNO_trygetitem_t)&default__trygetitem__unsupported)
			return (*tp_trygetitem)(self, index);
	}
	return (*maketyped__trygetitem(tp_trygetitem))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TTryGetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	__register DeeNO_trygetitem_index_t tp_trygetitem_index;
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_index = tp_self->tp_seq->tp_trygetitem_index) == NULL) {
		tp_trygetitem_index = _DeeType_RequireNativeOperator(tp_self, trygetitem_index);
		if unlikely(tp_trygetitem_index == (DeeNO_trygetitem_index_t)&default__trygetitem_index__badalloc ||
		            tp_trygetitem_index == (DeeNO_trygetitem_index_t)&default__trygetitem_index__unsupported)
			return (*tp_trygetitem_index)(self, index);
	}
	return (*maketyped__trygetitem_index(tp_trygetitem_index))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TTryGetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_trygetitem_string_hash_t tp_trygetitem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_string_hash = tp_self->tp_seq->tp_trygetitem_string_hash) == NULL) {
		tp_trygetitem_string_hash = _DeeType_RequireNativeOperator(tp_self, trygetitem_string_hash);
		if unlikely(tp_trygetitem_string_hash == (DeeNO_trygetitem_string_hash_t)&default__trygetitem_string_hash__badalloc ||
		            tp_trygetitem_string_hash == (DeeNO_trygetitem_string_hash_t)&default__trygetitem_string_hash__unsupported)
			return (*tp_trygetitem_string_hash)(self, key, hash);
	}
	return (*maketyped__trygetitem_string_hash(tp_trygetitem_string_hash))(tp_self, self, key, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TTryGetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_trygetitem_string_len_hash_t tp_trygetitem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_trygetitem_string_len_hash = tp_self->tp_seq->tp_trygetitem_string_len_hash) == NULL) {
		tp_trygetitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, trygetitem_string_len_hash);
		if unlikely(tp_trygetitem_string_len_hash == (DeeNO_trygetitem_string_len_hash_t)&default__trygetitem_string_len_hash__badalloc ||
		            tp_trygetitem_string_len_hash == (DeeNO_trygetitem_string_len_hash_t)&default__trygetitem_string_len_hash__unsupported)
			return (*tp_trygetitem_string_len_hash)(self, key, keylen, hash);
	}
	return (*maketyped__trygetitem_string_len_hash(tp_trygetitem_string_len_hash))(tp_self, self, key, keylen, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TBoundItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	__register DeeNO_bounditem_t tp_bounditem;
	if unlikely(!tp_self->tp_seq || (tp_bounditem = tp_self->tp_seq->tp_bounditem) == NULL) {
		tp_bounditem = _DeeType_RequireNativeOperator(tp_self, bounditem);
		if unlikely(tp_bounditem == (DeeNO_bounditem_t)&default__bounditem__badalloc ||
		            tp_bounditem == (DeeNO_bounditem_t)&default__bounditem__unsupported)
			return (*tp_bounditem)(self, index);
	}
	return (*maketyped__bounditem(tp_bounditem))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TBoundItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	__register DeeNO_bounditem_index_t tp_bounditem_index;
	if unlikely(!tp_self->tp_seq || (tp_bounditem_index = tp_self->tp_seq->tp_bounditem_index) == NULL) {
		tp_bounditem_index = _DeeType_RequireNativeOperator(tp_self, bounditem_index);
		if unlikely(tp_bounditem_index == (DeeNO_bounditem_index_t)&default__bounditem_index__badalloc ||
		            tp_bounditem_index == (DeeNO_bounditem_index_t)&default__bounditem_index__unsupported)
			return (*tp_bounditem_index)(self, index);
	}
	return (*maketyped__bounditem_index(tp_bounditem_index))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TBoundItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_bounditem_string_hash_t tp_bounditem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_bounditem_string_hash = tp_self->tp_seq->tp_bounditem_string_hash) == NULL) {
		tp_bounditem_string_hash = _DeeType_RequireNativeOperator(tp_self, bounditem_string_hash);
		if unlikely(tp_bounditem_string_hash == (DeeNO_bounditem_string_hash_t)&default__bounditem_string_hash__badalloc ||
		            tp_bounditem_string_hash == (DeeNO_bounditem_string_hash_t)&default__bounditem_string_hash__unsupported)
			return (*tp_bounditem_string_hash)(self, key, hash);
	}
	return (*maketyped__bounditem_string_hash(tp_bounditem_string_hash))(tp_self, self, key, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TBoundItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_bounditem_string_len_hash_t tp_bounditem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_bounditem_string_len_hash = tp_self->tp_seq->tp_bounditem_string_len_hash) == NULL) {
		tp_bounditem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, bounditem_string_len_hash);
		if unlikely(tp_bounditem_string_len_hash == (DeeNO_bounditem_string_len_hash_t)&default__bounditem_string_len_hash__badalloc ||
		            tp_bounditem_string_len_hash == (DeeNO_bounditem_string_len_hash_t)&default__bounditem_string_len_hash__unsupported)
			return (*tp_bounditem_string_len_hash)(self, key, keylen, hash);
	}
	return (*maketyped__bounditem_string_len_hash(tp_bounditem_string_len_hash))(tp_self, self, key, keylen, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_THasItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	__register DeeNO_hasitem_t tp_hasitem;
	if unlikely(!tp_self->tp_seq || (tp_hasitem = tp_self->tp_seq->tp_hasitem) == NULL) {
		tp_hasitem = _DeeType_RequireNativeOperator(tp_self, hasitem);
		if unlikely(tp_hasitem == (DeeNO_hasitem_t)&default__hasitem__badalloc ||
		            tp_hasitem == (DeeNO_hasitem_t)&default__hasitem__unsupported)
			return (*tp_hasitem)(self, index);
	}
	return (*maketyped__hasitem(tp_hasitem))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_THasItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	__register DeeNO_hasitem_index_t tp_hasitem_index;
	if unlikely(!tp_self->tp_seq || (tp_hasitem_index = tp_self->tp_seq->tp_hasitem_index) == NULL) {
		tp_hasitem_index = _DeeType_RequireNativeOperator(tp_self, hasitem_index);
		if unlikely(tp_hasitem_index == (DeeNO_hasitem_index_t)&default__hasitem_index__badalloc ||
		            tp_hasitem_index == (DeeNO_hasitem_index_t)&default__hasitem_index__unsupported)
			return (*tp_hasitem_index)(self, index);
	}
	return (*maketyped__hasitem_index(tp_hasitem_index))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_THasItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_hasitem_string_hash_t tp_hasitem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_hasitem_string_hash = tp_self->tp_seq->tp_hasitem_string_hash) == NULL) {
		tp_hasitem_string_hash = _DeeType_RequireNativeOperator(tp_self, hasitem_string_hash);
		if unlikely(tp_hasitem_string_hash == (DeeNO_hasitem_string_hash_t)&default__hasitem_string_hash__badalloc ||
		            tp_hasitem_string_hash == (DeeNO_hasitem_string_hash_t)&default__hasitem_string_hash__unsupported)
			return (*tp_hasitem_string_hash)(self, key, hash);
	}
	return (*maketyped__hasitem_string_hash(tp_hasitem_string_hash))(tp_self, self, key, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_THasItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_hasitem_string_len_hash_t tp_hasitem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_hasitem_string_len_hash = tp_self->tp_seq->tp_hasitem_string_len_hash) == NULL) {
		tp_hasitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, hasitem_string_len_hash);
		if unlikely(tp_hasitem_string_len_hash == (DeeNO_hasitem_string_len_hash_t)&default__hasitem_string_len_hash__badalloc ||
		            tp_hasitem_string_len_hash == (DeeNO_hasitem_string_len_hash_t)&default__hasitem_string_len_hash__unsupported)
			return (*tp_hasitem_string_len_hash)(self, key, keylen, hash);
	}
	return (*maketyped__hasitem_string_len_hash(tp_hasitem_string_len_hash))(tp_self, self, key, keylen, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TDelItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	__register DeeNO_delitem_t tp_delitem;
	if unlikely(!tp_self->tp_seq || (tp_delitem = tp_self->tp_seq->tp_delitem) == NULL) {
		tp_delitem = _DeeType_RequireNativeOperator(tp_self, delitem);
		if unlikely(tp_delitem == (DeeNO_delitem_t)&default__delitem__badalloc ||
		            tp_delitem == (DeeNO_delitem_t)&default__delitem__unsupported)
			return (*tp_delitem)(self, index);
	}
	return (*maketyped__delitem(tp_delitem))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TDelItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index) {
	__register DeeNO_delitem_index_t tp_delitem_index;
	if unlikely(!tp_self->tp_seq || (tp_delitem_index = tp_self->tp_seq->tp_delitem_index) == NULL) {
		tp_delitem_index = _DeeType_RequireNativeOperator(tp_self, delitem_index);
		if unlikely(tp_delitem_index == (DeeNO_delitem_index_t)&default__delitem_index__badalloc ||
		            tp_delitem_index == (DeeNO_delitem_index_t)&default__delitem_index__unsupported)
			return (*tp_delitem_index)(self, index);
	}
	return (*maketyped__delitem_index(tp_delitem_index))(tp_self, self, index);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TDelItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash) {
	__register DeeNO_delitem_string_hash_t tp_delitem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_delitem_string_hash = tp_self->tp_seq->tp_delitem_string_hash) == NULL) {
		tp_delitem_string_hash = _DeeType_RequireNativeOperator(tp_self, delitem_string_hash);
		if unlikely(tp_delitem_string_hash == (DeeNO_delitem_string_hash_t)&default__delitem_string_hash__badalloc ||
		            tp_delitem_string_hash == (DeeNO_delitem_string_hash_t)&default__delitem_string_hash__unsupported)
			return (*tp_delitem_string_hash)(self, key, hash);
	}
	return (*maketyped__delitem_string_hash(tp_delitem_string_hash))(tp_self, self, key, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TDelItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash) {
	__register DeeNO_delitem_string_len_hash_t tp_delitem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_delitem_string_len_hash = tp_self->tp_seq->tp_delitem_string_len_hash) == NULL) {
		tp_delitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, delitem_string_len_hash);
		if unlikely(tp_delitem_string_len_hash == (DeeNO_delitem_string_len_hash_t)&default__delitem_string_len_hash__badalloc ||
		            tp_delitem_string_len_hash == (DeeNO_delitem_string_len_hash_t)&default__delitem_string_len_hash__unsupported)
			return (*tp_delitem_string_len_hash)(self, key, keylen, hash);
	}
	return (*maketyped__delitem_string_len_hash(tp_delitem_string_len_hash))(tp_self, self, key, keylen, hash);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeObject_TSetItem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index, DeeObject *value) {
	__register DeeNO_setitem_t tp_setitem;
	if unlikely(!tp_self->tp_seq || (tp_setitem = tp_self->tp_seq->tp_setitem) == NULL) {
		tp_setitem = _DeeType_RequireNativeOperator(tp_self, setitem);
		if unlikely(tp_setitem == (DeeNO_setitem_t)&default__setitem__badalloc ||
		            tp_setitem == (DeeNO_setitem_t)&default__setitem__unsupported)
			return (*tp_setitem)(self, index, value);
	}
	return (*maketyped__setitem(tp_setitem))(tp_self, self, index, value);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_TSetItemIndex)(DeeTypeObject *tp_self, DeeObject *self, size_t index, DeeObject *value) {
	__register DeeNO_setitem_index_t tp_setitem_index;
	if unlikely(!tp_self->tp_seq || (tp_setitem_index = tp_self->tp_seq->tp_setitem_index) == NULL) {
		tp_setitem_index = _DeeType_RequireNativeOperator(tp_self, setitem_index);
		if unlikely(tp_setitem_index == (DeeNO_setitem_index_t)&default__setitem_index__badalloc ||
		            tp_setitem_index == (DeeNO_setitem_index_t)&default__setitem_index__unsupported)
			return (*tp_setitem_index)(self, index, value);
	}
	return (*maketyped__setitem_index(tp_setitem_index))(tp_self, self, index, value);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 5)) int
(DCALL DeeObject_TSetItemStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	__register DeeNO_setitem_string_hash_t tp_setitem_string_hash;
	if unlikely(!tp_self->tp_seq || (tp_setitem_string_hash = tp_self->tp_seq->tp_setitem_string_hash) == NULL) {
		tp_setitem_string_hash = _DeeType_RequireNativeOperator(tp_self, setitem_string_hash);
		if unlikely(tp_setitem_string_hash == (DeeNO_setitem_string_hash_t)&default__setitem_string_hash__badalloc ||
		            tp_setitem_string_hash == (DeeNO_setitem_string_hash_t)&default__setitem_string_hash__unsupported)
			return (*tp_setitem_string_hash)(self, key, hash, value);
	}
	return (*maketyped__setitem_string_hash(tp_setitem_string_hash))(tp_self, self, key, hash, value);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 6)) int
(DCALL DeeObject_TSetItemStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	__register DeeNO_setitem_string_len_hash_t tp_setitem_string_len_hash;
	if unlikely(!tp_self->tp_seq || (tp_setitem_string_len_hash = tp_self->tp_seq->tp_setitem_string_len_hash) == NULL) {
		tp_setitem_string_len_hash = _DeeType_RequireNativeOperator(tp_self, setitem_string_len_hash);
		if unlikely(tp_setitem_string_len_hash == (DeeNO_setitem_string_len_hash_t)&default__setitem_string_len_hash__badalloc ||
		            tp_setitem_string_len_hash == (DeeNO_setitem_string_len_hash_t)&default__setitem_string_len_hash__unsupported)
			return (*tp_setitem_string_len_hash)(self, key, keylen, hash, value);
	}
	return (*maketyped__setitem_string_len_hash(tp_setitem_string_len_hash))(tp_self, self, key, keylen, hash, value);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TGetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	__register DeeNO_getrange_t tp_getrange;
	if unlikely(!tp_self->tp_seq || (tp_getrange = tp_self->tp_seq->tp_getrange) == NULL) {
		tp_getrange = _DeeType_RequireNativeOperator(tp_self, getrange);
		if unlikely(tp_getrange == (DeeNO_getrange_t)&default__getrange__badalloc ||
		            tp_getrange == (DeeNO_getrange_t)&default__getrange__unsupported)
			return (*tp_getrange)(self, start, end);
	}
	return (*maketyped__getrange(tp_getrange))(tp_self, self, start, end);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TGetRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	__register DeeNO_getrange_index_t tp_getrange_index;
	if unlikely(!tp_self->tp_seq || (tp_getrange_index = tp_self->tp_seq->tp_getrange_index) == NULL) {
		tp_getrange_index = _DeeType_RequireNativeOperator(tp_self, getrange_index);
		if unlikely(tp_getrange_index == (DeeNO_getrange_index_t)&default__getrange_index__badalloc ||
		            tp_getrange_index == (DeeNO_getrange_index_t)&default__getrange_index__unsupported)
			return (*tp_getrange_index)(self, start, end);
	}
	return (*maketyped__getrange_index(tp_getrange_index))(tp_self, self, start, end);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TGetRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	__register DeeNO_getrange_index_n_t tp_getrange_index_n;
	if unlikely(!tp_self->tp_seq || (tp_getrange_index_n = tp_self->tp_seq->tp_getrange_index_n) == NULL) {
		tp_getrange_index_n = _DeeType_RequireNativeOperator(tp_self, getrange_index_n);
		if unlikely(tp_getrange_index_n == (DeeNO_getrange_index_n_t)&default__getrange_index_n__badalloc ||
		            tp_getrange_index_n == (DeeNO_getrange_index_n_t)&default__getrange_index_n__unsupported)
			return (*tp_getrange_index_n)(self, start);
	}
	return (*maketyped__getrange_index_n(tp_getrange_index_n))(tp_self, self, start);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeObject_TDelRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end) {
	__register DeeNO_delrange_t tp_delrange;
	if unlikely(!tp_self->tp_seq || (tp_delrange = tp_self->tp_seq->tp_delrange) == NULL) {
		tp_delrange = _DeeType_RequireNativeOperator(tp_self, delrange);
		if unlikely(tp_delrange == (DeeNO_delrange_t)&default__delrange__badalloc ||
		            tp_delrange == (DeeNO_delrange_t)&default__delrange__unsupported)
			return (*tp_delrange)(self, start, end);
	}
	return (*maketyped__delrange(tp_delrange))(tp_self, self, start, end);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TDelRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	__register DeeNO_delrange_index_t tp_delrange_index;
	if unlikely(!tp_self->tp_seq || (tp_delrange_index = tp_self->tp_seq->tp_delrange_index) == NULL) {
		tp_delrange_index = _DeeType_RequireNativeOperator(tp_self, delrange_index);
		if unlikely(tp_delrange_index == (DeeNO_delrange_index_t)&default__delrange_index__badalloc ||
		            tp_delrange_index == (DeeNO_delrange_index_t)&default__delrange_index__unsupported)
			return (*tp_delrange_index)(self, start, end);
	}
	return (*maketyped__delrange_index(tp_delrange_index))(tp_self, self, start, end);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TDelRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start) {
	__register DeeNO_delrange_index_n_t tp_delrange_index_n;
	if unlikely(!tp_self->tp_seq || (tp_delrange_index_n = tp_self->tp_seq->tp_delrange_index_n) == NULL) {
		tp_delrange_index_n = _DeeType_RequireNativeOperator(tp_self, delrange_index_n);
		if unlikely(tp_delrange_index_n == (DeeNO_delrange_index_n_t)&default__delrange_index_n__badalloc ||
		            tp_delrange_index_n == (DeeNO_delrange_index_n_t)&default__delrange_index_n__unsupported)
			return (*tp_delrange_index_n)(self, start);
	}
	return (*maketyped__delrange_index_n(tp_delrange_index_n))(tp_self, self, start);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) int
(DCALL DeeObject_TSetRange)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	__register DeeNO_setrange_t tp_setrange;
	if unlikely(!tp_self->tp_seq || (tp_setrange = tp_self->tp_seq->tp_setrange) == NULL) {
		tp_setrange = _DeeType_RequireNativeOperator(tp_self, setrange);
		if unlikely(tp_setrange == (DeeNO_setrange_t)&default__setrange__badalloc ||
		            tp_setrange == (DeeNO_setrange_t)&default__setrange__unsupported)
			return (*tp_setrange)(self, start, end, values);
	}
	return (*maketyped__setrange(tp_setrange))(tp_self, self, start, end, values);
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeObject_TSetRangeIndex)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	__register DeeNO_setrange_index_t tp_setrange_index;
	if unlikely(!tp_self->tp_seq || (tp_setrange_index = tp_self->tp_seq->tp_setrange_index) == NULL) {
		tp_setrange_index = _DeeType_RequireNativeOperator(tp_self, setrange_index);
		if unlikely(tp_setrange_index == (DeeNO_setrange_index_t)&default__setrange_index__badalloc ||
		            tp_setrange_index == (DeeNO_setrange_index_t)&default__setrange_index__unsupported)
			return (*tp_setrange_index)(self, start, end, values);
	}
	return (*maketyped__setrange_index(tp_setrange_index))(tp_self, self, start, end, values);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_TSetRangeIndexN)(DeeTypeObject *tp_self, DeeObject *self, Dee_ssize_t start, DeeObject *values) {
	__register DeeNO_setrange_index_n_t tp_setrange_index_n;
	if unlikely(!tp_self->tp_seq || (tp_setrange_index_n = tp_self->tp_seq->tp_setrange_index_n) == NULL) {
		tp_setrange_index_n = _DeeType_RequireNativeOperator(tp_self, setrange_index_n);
		if unlikely(tp_setrange_index_n == (DeeNO_setrange_index_n_t)&default__setrange_index_n__badalloc ||
		            tp_setrange_index_n == (DeeNO_setrange_index_n_t)&default__setrange_index_n__unsupported)
			return (*tp_setrange_index_n)(self, start, values);
	}
	return (*maketyped__setrange_index_n(tp_setrange_index_n))(tp_self, self, start, values);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TInv)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_inv_t tp_inv;
	if unlikely(!tp_self->tp_math || (tp_inv = tp_self->tp_math->tp_inv) == NULL) {
		tp_inv = _DeeType_RequireNativeOperator(tp_self, inv);
		if unlikely(tp_inv == (DeeNO_inv_t)&default__inv__badalloc ||
		            tp_inv == (DeeNO_inv_t)&default__inv__unsupported)
			return (*tp_inv)(self);
	}
	return (*maketyped__inv(tp_inv))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TPos)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_pos_t tp_pos;
	if unlikely(!tp_self->tp_math || (tp_pos = tp_self->tp_math->tp_pos) == NULL) {
		tp_pos = _DeeType_RequireNativeOperator(tp_self, pos);
		if unlikely(tp_pos == (DeeNO_pos_t)&default__pos__badalloc ||
		            tp_pos == (DeeNO_pos_t)&default__pos__unsupported)
			return (*tp_pos)(self);
	}
	return (*maketyped__pos(tp_pos))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_TNeg)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_neg_t tp_neg;
	if unlikely(!tp_self->tp_math || (tp_neg = tp_self->tp_math->tp_neg) == NULL) {
		tp_neg = _DeeType_RequireNativeOperator(tp_self, neg);
		if unlikely(tp_neg == (DeeNO_neg_t)&default__neg__badalloc ||
		            tp_neg == (DeeNO_neg_t)&default__neg__unsupported)
			return (*tp_neg)(self);
	}
	return (*maketyped__neg(tp_neg))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TAdd)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_add_t tp_add;
	if unlikely(!tp_self->tp_math || (tp_add = tp_self->tp_math->tp_add) == NULL) {
		tp_add = _DeeType_RequireNativeOperator(tp_self, add);
		if unlikely(tp_add == (DeeNO_add_t)&default__add__badalloc ||
		            tp_add == (DeeNO_add_t)&default__add__unsupported)
			return (*tp_add)(lhs, rhs);
	}
	return (*maketyped__add(tp_add))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceAdd)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_add_t tp_inplace_add;
	if unlikely(!tp_self->tp_math || (tp_inplace_add = tp_self->tp_math->tp_inplace_add) == NULL) {
		tp_inplace_add = _DeeType_RequireNativeOperator(tp_self, inplace_add);
		if unlikely(tp_inplace_add == (DeeNO_inplace_add_t)&default__inplace_add__badalloc ||
		            tp_inplace_add == (DeeNO_inplace_add_t)&default__inplace_add__unsupported)
			return (*tp_inplace_add)(p_lhs, rhs);
	}
	return (*maketyped__inplace_add(tp_inplace_add))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TSub)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_sub_t tp_sub;
	if unlikely(!tp_self->tp_math || (tp_sub = tp_self->tp_math->tp_sub) == NULL) {
		tp_sub = _DeeType_RequireNativeOperator(tp_self, sub);
		if unlikely(tp_sub == (DeeNO_sub_t)&default__sub__badalloc ||
		            tp_sub == (DeeNO_sub_t)&default__sub__unsupported)
			return (*tp_sub)(lhs, rhs);
	}
	return (*maketyped__sub(tp_sub))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceSub)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_sub_t tp_inplace_sub;
	if unlikely(!tp_self->tp_math || (tp_inplace_sub = tp_self->tp_math->tp_inplace_sub) == NULL) {
		tp_inplace_sub = _DeeType_RequireNativeOperator(tp_self, inplace_sub);
		if unlikely(tp_inplace_sub == (DeeNO_inplace_sub_t)&default__inplace_sub__badalloc ||
		            tp_inplace_sub == (DeeNO_inplace_sub_t)&default__inplace_sub__unsupported)
			return (*tp_inplace_sub)(p_lhs, rhs);
	}
	return (*maketyped__inplace_sub(tp_inplace_sub))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TMul)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_mul_t tp_mul;
	if unlikely(!tp_self->tp_math || (tp_mul = tp_self->tp_math->tp_mul) == NULL) {
		tp_mul = _DeeType_RequireNativeOperator(tp_self, mul);
		if unlikely(tp_mul == (DeeNO_mul_t)&default__mul__badalloc ||
		            tp_mul == (DeeNO_mul_t)&default__mul__unsupported)
			return (*tp_mul)(lhs, rhs);
	}
	return (*maketyped__mul(tp_mul))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceMul)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_mul_t tp_inplace_mul;
	if unlikely(!tp_self->tp_math || (tp_inplace_mul = tp_self->tp_math->tp_inplace_mul) == NULL) {
		tp_inplace_mul = _DeeType_RequireNativeOperator(tp_self, inplace_mul);
		if unlikely(tp_inplace_mul == (DeeNO_inplace_mul_t)&default__inplace_mul__badalloc ||
		            tp_inplace_mul == (DeeNO_inplace_mul_t)&default__inplace_mul__unsupported)
			return (*tp_inplace_mul)(p_lhs, rhs);
	}
	return (*maketyped__inplace_mul(tp_inplace_mul))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TDiv)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_div_t tp_div;
	if unlikely(!tp_self->tp_math || (tp_div = tp_self->tp_math->tp_div) == NULL) {
		tp_div = _DeeType_RequireNativeOperator(tp_self, div);
		if unlikely(tp_div == (DeeNO_div_t)&default__div__badalloc ||
		            tp_div == (DeeNO_div_t)&default__div__unsupported)
			return (*tp_div)(lhs, rhs);
	}
	return (*maketyped__div(tp_div))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceDiv)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_div_t tp_inplace_div;
	if unlikely(!tp_self->tp_math || (tp_inplace_div = tp_self->tp_math->tp_inplace_div) == NULL) {
		tp_inplace_div = _DeeType_RequireNativeOperator(tp_self, inplace_div);
		if unlikely(tp_inplace_div == (DeeNO_inplace_div_t)&default__inplace_div__badalloc ||
		            tp_inplace_div == (DeeNO_inplace_div_t)&default__inplace_div__unsupported)
			return (*tp_inplace_div)(p_lhs, rhs);
	}
	return (*maketyped__inplace_div(tp_inplace_div))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TMod)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_mod_t tp_mod;
	if unlikely(!tp_self->tp_math || (tp_mod = tp_self->tp_math->tp_mod) == NULL) {
		tp_mod = _DeeType_RequireNativeOperator(tp_self, mod);
		if unlikely(tp_mod == (DeeNO_mod_t)&default__mod__badalloc ||
		            tp_mod == (DeeNO_mod_t)&default__mod__unsupported)
			return (*tp_mod)(lhs, rhs);
	}
	return (*maketyped__mod(tp_mod))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceMod)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_mod_t tp_inplace_mod;
	if unlikely(!tp_self->tp_math || (tp_inplace_mod = tp_self->tp_math->tp_inplace_mod) == NULL) {
		tp_inplace_mod = _DeeType_RequireNativeOperator(tp_self, inplace_mod);
		if unlikely(tp_inplace_mod == (DeeNO_inplace_mod_t)&default__inplace_mod__badalloc ||
		            tp_inplace_mod == (DeeNO_inplace_mod_t)&default__inplace_mod__unsupported)
			return (*tp_inplace_mod)(p_lhs, rhs);
	}
	return (*maketyped__inplace_mod(tp_inplace_mod))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TShl)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_shl_t tp_shl;
	if unlikely(!tp_self->tp_math || (tp_shl = tp_self->tp_math->tp_shl) == NULL) {
		tp_shl = _DeeType_RequireNativeOperator(tp_self, shl);
		if unlikely(tp_shl == (DeeNO_shl_t)&default__shl__badalloc ||
		            tp_shl == (DeeNO_shl_t)&default__shl__unsupported)
			return (*tp_shl)(lhs, rhs);
	}
	return (*maketyped__shl(tp_shl))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceShl)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_shl_t tp_inplace_shl;
	if unlikely(!tp_self->tp_math || (tp_inplace_shl = tp_self->tp_math->tp_inplace_shl) == NULL) {
		tp_inplace_shl = _DeeType_RequireNativeOperator(tp_self, inplace_shl);
		if unlikely(tp_inplace_shl == (DeeNO_inplace_shl_t)&default__inplace_shl__badalloc ||
		            tp_inplace_shl == (DeeNO_inplace_shl_t)&default__inplace_shl__unsupported)
			return (*tp_inplace_shl)(p_lhs, rhs);
	}
	return (*maketyped__inplace_shl(tp_inplace_shl))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TShr)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_shr_t tp_shr;
	if unlikely(!tp_self->tp_math || (tp_shr = tp_self->tp_math->tp_shr) == NULL) {
		tp_shr = _DeeType_RequireNativeOperator(tp_self, shr);
		if unlikely(tp_shr == (DeeNO_shr_t)&default__shr__badalloc ||
		            tp_shr == (DeeNO_shr_t)&default__shr__unsupported)
			return (*tp_shr)(lhs, rhs);
	}
	return (*maketyped__shr(tp_shr))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceShr)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_shr_t tp_inplace_shr;
	if unlikely(!tp_self->tp_math || (tp_inplace_shr = tp_self->tp_math->tp_inplace_shr) == NULL) {
		tp_inplace_shr = _DeeType_RequireNativeOperator(tp_self, inplace_shr);
		if unlikely(tp_inplace_shr == (DeeNO_inplace_shr_t)&default__inplace_shr__badalloc ||
		            tp_inplace_shr == (DeeNO_inplace_shr_t)&default__inplace_shr__unsupported)
			return (*tp_inplace_shr)(p_lhs, rhs);
	}
	return (*maketyped__inplace_shr(tp_inplace_shr))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TAnd)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_and_t tp_and;
	if unlikely(!tp_self->tp_math || (tp_and = tp_self->tp_math->tp_and) == NULL) {
		tp_and = _DeeType_RequireNativeOperator(tp_self, and);
		if unlikely(tp_and == (DeeNO_and_t)&default__and__badalloc ||
		            tp_and == (DeeNO_and_t)&default__and__unsupported)
			return (*tp_and)(lhs, rhs);
	}
	return (*maketyped__and(tp_and))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceAnd)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_and_t tp_inplace_and;
	if unlikely(!tp_self->tp_math || (tp_inplace_and = tp_self->tp_math->tp_inplace_and) == NULL) {
		tp_inplace_and = _DeeType_RequireNativeOperator(tp_self, inplace_and);
		if unlikely(tp_inplace_and == (DeeNO_inplace_and_t)&default__inplace_and__badalloc ||
		            tp_inplace_and == (DeeNO_inplace_and_t)&default__inplace_and__unsupported)
			return (*tp_inplace_and)(p_lhs, rhs);
	}
	return (*maketyped__inplace_and(tp_inplace_and))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TOr)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_or_t tp_or;
	if unlikely(!tp_self->tp_math || (tp_or = tp_self->tp_math->tp_or) == NULL) {
		tp_or = _DeeType_RequireNativeOperator(tp_self, or);
		if unlikely(tp_or == (DeeNO_or_t)&default__or__badalloc ||
		            tp_or == (DeeNO_or_t)&default__or__unsupported)
			return (*tp_or)(lhs, rhs);
	}
	return (*maketyped__or(tp_or))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceOr)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_or_t tp_inplace_or;
	if unlikely(!tp_self->tp_math || (tp_inplace_or = tp_self->tp_math->tp_inplace_or) == NULL) {
		tp_inplace_or = _DeeType_RequireNativeOperator(tp_self, inplace_or);
		if unlikely(tp_inplace_or == (DeeNO_inplace_or_t)&default__inplace_or__badalloc ||
		            tp_inplace_or == (DeeNO_inplace_or_t)&default__inplace_or__unsupported)
			return (*tp_inplace_or)(p_lhs, rhs);
	}
	return (*maketyped__inplace_or(tp_inplace_or))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TXor)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_xor_t tp_xor;
	if unlikely(!tp_self->tp_math || (tp_xor = tp_self->tp_math->tp_xor) == NULL) {
		tp_xor = _DeeType_RequireNativeOperator(tp_self, xor);
		if unlikely(tp_xor == (DeeNO_xor_t)&default__xor__badalloc ||
		            tp_xor == (DeeNO_xor_t)&default__xor__unsupported)
			return (*tp_xor)(lhs, rhs);
	}
	return (*maketyped__xor(tp_xor))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplaceXor)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_xor_t tp_inplace_xor;
	if unlikely(!tp_self->tp_math || (tp_inplace_xor = tp_self->tp_math->tp_inplace_xor) == NULL) {
		tp_inplace_xor = _DeeType_RequireNativeOperator(tp_self, inplace_xor);
		if unlikely(tp_inplace_xor == (DeeNO_inplace_xor_t)&default__inplace_xor__badalloc ||
		            tp_inplace_xor == (DeeNO_inplace_xor_t)&default__inplace_xor__unsupported)
			return (*tp_inplace_xor)(p_lhs, rhs);
	}
	return (*maketyped__inplace_xor(tp_inplace_xor))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TPow)(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs) {
	__register DeeNO_pow_t tp_pow;
	if unlikely(!tp_self->tp_math || (tp_pow = tp_self->tp_math->tp_pow) == NULL) {
		tp_pow = _DeeType_RequireNativeOperator(tp_self, pow);
		if unlikely(tp_pow == (DeeNO_pow_t)&default__pow__badalloc ||
		            tp_pow == (DeeNO_pow_t)&default__pow__unsupported)
			return (*tp_pow)(lhs, rhs);
	}
	return (*maketyped__pow(tp_pow))(tp_self, lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TInplacePow)(DeeTypeObject *tp_self, DREF DeeObject **p_lhs, DeeObject *rhs) {
	__register DeeNO_inplace_pow_t tp_inplace_pow;
	if unlikely(!tp_self->tp_math || (tp_inplace_pow = tp_self->tp_math->tp_inplace_pow) == NULL) {
		tp_inplace_pow = _DeeType_RequireNativeOperator(tp_self, inplace_pow);
		if unlikely(tp_inplace_pow == (DeeNO_inplace_pow_t)&default__inplace_pow__badalloc ||
		            tp_inplace_pow == (DeeNO_inplace_pow_t)&default__inplace_pow__unsupported)
			return (*tp_inplace_pow)(p_lhs, rhs);
	}
	return (*maketyped__inplace_pow(tp_inplace_pow))(tp_self, p_lhs, rhs);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TInc)(DeeTypeObject *tp_self, DREF DeeObject **p_self) {
	__register DeeNO_inc_t tp_inc;
	if unlikely(!tp_self->tp_math || (tp_inc = tp_self->tp_math->tp_inc) == NULL) {
		tp_inc = _DeeType_RequireNativeOperator(tp_self, inc);
		if unlikely(tp_inc == (DeeNO_inc_t)&default__inc__badalloc ||
		            tp_inc == (DeeNO_inc_t)&default__inc__unsupported)
			return (*tp_inc)(p_self);
	}
	return (*maketyped__inc(tp_inc))(tp_self, p_self);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TDec)(DeeTypeObject *tp_self, DREF DeeObject **p_self) {
	__register DeeNO_dec_t tp_dec;
	if unlikely(!tp_self->tp_math || (tp_dec = tp_self->tp_math->tp_dec) == NULL) {
		tp_dec = _DeeType_RequireNativeOperator(tp_self, dec);
		if unlikely(tp_dec == (DeeNO_dec_t)&default__dec__badalloc ||
		            tp_dec == (DeeNO_dec_t)&default__dec__unsupported)
			return (*tp_dec)(p_self);
	}
	return (*maketyped__dec(tp_dec))(tp_self, p_self);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TEnter)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_enter_t tp_enter;
	if unlikely(!tp_self->tp_with || (tp_enter = tp_self->tp_with->tp_enter) == NULL) {
		tp_enter = _DeeType_RequireNativeOperator(tp_self, enter);
		if unlikely(tp_enter == (DeeNO_enter_t)&default__enter__badalloc ||
		            tp_enter == (DeeNO_enter_t)&default__enter__unsupported)
			return (*tp_enter)(self);
	}
	return (*maketyped__enter(tp_enter))(tp_self, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_TLeave)(DeeTypeObject *tp_self, DeeObject *self) {
	__register DeeNO_leave_t tp_leave;
	if unlikely(!tp_self->tp_with || (tp_leave = tp_self->tp_with->tp_leave) == NULL) {
		tp_leave = _DeeType_RequireNativeOperator(tp_self, leave);
		if unlikely(tp_leave == (DeeNO_leave_t)&default__leave__badalloc ||
		            tp_leave == (DeeNO_leave_t)&default__leave__unsupported)
			return (*tp_leave)(self);
	}
	return (*maketyped__leave(tp_leave))(tp_self, self);
}
/*[[[end]]]*/
/* clang-format on */


/************************************************************************/
/* MISC. RUNTIME OBJECT WRAPPERS                                        */
/************************************************************************/

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int DCALL
DeeObject_BoolInherited(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	int result;
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
#ifndef __OPTIMIZE_SIZE__
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type) {
		Dee_DecrefNokill(self);
		return self == Dee_True ? 1 : 0;
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	result = (*tp_bool)(self);
	Dee_Decref(self);
	return result;
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) int DCALL
DeeObject_BoolInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	int result;
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
#ifndef __OPTIMIZE_SIZE__
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type) {
		Dee_DecrefNokill(self);
		return self == Dee_True ? 1 : 0;
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	result = (*tp_bool)(self);
	if likely(result >= 0)
		Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_BoolOb(DeeObject *__restrict self) {
	int result;
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type)
		return_reference_(self);
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	result = (*tp_bool)(self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_BoolObInherited(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	int result;
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type)
		return self;
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	result = (*tp_bool)(self);
	Dee_Decref(self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_BoolObInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	int result;
	__register DeeNO_bool_t tp_bool;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	__builtin_assume((tp_self == &DeeBool_Type) ==
	                 (self == Dee_True || self == Dee_False));
	if (tp_self == &DeeBool_Type)
		return self;
	if unlikely((tp_bool = tp_self->tp_cast.tp_bool) == NULL)
		tp_bool = _DeeType_RequireNativeOperator(tp_self, bool);
	result = (*tp_bool)(self);
	if unlikely(result < 0)
		goto err;
	Dee_Decref(self);
	return_bool(result);
err:
	return NULL;
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_IntInherited(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	__register DeeNO_int_t tp_int;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if likely(tp_self == &DeeInt_Type)
		return self;
	if unlikely(!tp_self->tp_math || (tp_int = tp_self->tp_math->tp_int) == NULL)
		tp_int = _DeeType_RequireNativeOperator(tp_self, int);
	result = (*tp_int)(self);
	Dee_Decref(self);
	return result;
}

PUBLIC ATTR_HOT WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_IntInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	__register DeeNO_int_t tp_int;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if likely(tp_self == &DeeInt_Type)
		return self;
	if unlikely(!tp_self->tp_math || (tp_int = tp_self->tp_math->tp_int) == NULL)
		tp_int = _DeeType_RequireNativeOperator(tp_self, int);
	result = (*tp_int)(self);
	if likely(result)
		Dee_Decref(self);
	return result;
}

#ifdef __ARCH_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VCallPack, 12),
                    DCALL_ASSEMBLY_NAME(DeeObject_Call, 12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args) {
	return DeeObject_Call(self, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* __ARCH_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeObject_VCallPack(DeeObject *self, size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_Call(self, argc, DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !__ARCH_VA_LIST_IS_STACK_POINTER */



/* Same as the other functions above, but *always* inherits references to "self" (iow: the function being called) */
PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_CallInherited(/*inherit(always)*/ DREF DeeObject *self,
                        size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result = DeeObject_Call(self, argc, argv);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeObject *DCALL
DeeObject_CallKwInherited(/*inherit(always)*/ DREF DeeObject *self,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = DeeObject_CallKw(self, argc, argv, kw);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_ThisCallInherited(/*inherit(always)*/ DREF DeeObject *self,
                            DeeObject *thisarg, size_t argc,
                            DeeObject *const *argv) {
	DREF DeeObject *result = DeeObject_ThisCall(self, thisarg, argc, argv);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_ThisCallKwInherited(/*inherit(always)*/ DREF DeeObject *self,
                              DeeObject *thisarg, size_t argc,
                              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result = DeeObject_ThisCallKw(self, thisarg, argc, argv, kw);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args) {
	DREF DeeObject *result = DeeObject_CallTuple(self, args);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	DREF DeeObject *result = DeeObject_CallTupleKw(self, args, kw);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args) {
	DREF DeeObject *result = DeeObject_ThisCallTuple(self, thisarg, args);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	DREF DeeObject *result = DeeObject_ThisCallTupleKw(self, thisarg, args, kw);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

PUBLIC WUNUSED /*ATTR_PURE*/ NONNULL((1)) Dee_hash_t
(DCALL DeeObject_HashInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	Dee_hash_t result = DeeObject_Hash(self);
	/* *_unlikely because you usually compute the hash for something that
	 * is/gets stored elsewhere, rather than some temporary object. */
	Dee_Decref_unlikely(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_CopyInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_Copy(self);
	Dee_Decref_unlikely(self); /* You usually copy something because someone will continue to use the source */
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_DeepCopyInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_DeepCopy(self);
	Dee_Decref_unlikely(self); /* You usually copy something because someone will continue to use the source */
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_SizeObInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_SizeOb(self);
	Dee_Decref_unlikely(self); /* You usually take the size of something because you indent to keep working with that something */
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_StrInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_Str(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_ReprInherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result = DeeObject_Repr(self);
	Dee_Decref(self);
	return result;
}




DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_C */
