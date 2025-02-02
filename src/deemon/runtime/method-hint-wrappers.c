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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>

/**/
#include "../objects/seq/enumerate-cb.h"

/**/
#include "kwlist.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMethodAttributeImpls from "..method-hints.method-hints")();]]]*/
PUBLIC_CONST char const DeeMA___seq_bool___name[] = "__seq_bool__";
PUBLIC_CONST char const DeeMA___seq_bool___doc[] = "->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bool__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	if (DeeArg_Unpack(argc, argv, ":__seq_bool__"))
		goto err;
	result = DeeSeq_OperatorBool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_size___name[] = "__seq_size__";
PUBLIC_CONST char const DeeMA___seq_size___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_size__"))
		goto err;
	return DeeSeq_OperatorSizeOb(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_iter___name[] = "__seq_iter__";
PUBLIC_CONST char const DeeMA___seq_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iter__"))
		goto err;
	return DeeSeq_OperatorIter(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_iterkeys___name[] = "__seq_iterkeys__";
PUBLIC_CONST char const DeeMA___seq_iterkeys___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iterkeys__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iterkeys__"))
		goto err;
	return DeeSeq_OperatorIterKeys(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_enumerate___name[] = "__seq_enumerate__";
PUBLIC_CONST char const DeeMA___seq_enumerate___doc[] = "(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	size_t start = 0;
	size_t end = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":__seq_enumerate__", &data.sed_cb, &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		foreach_status = DeeSeq_OperatorEnumerate(self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_enumerate_index_cb, &data, start, end);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_getitem___name[] = "__seq_getitem__";
PUBLIC_CONST char const DeeMA___seq_getitem___doc[] = "(index:?Dint)->";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__seq_getitem__", &index))
		goto err;
	return DeeSeq_OperatorGetItem(self, index);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_delitem___name[] = "__seq_delitem__";
PUBLIC_CONST char const DeeMA___seq_delitem___doc[] = "(index:?Dint)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__seq_delitem__", &index))
		goto err;
	if (DeeSeq_OperatorDelItem(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_setitem___name[] = "__seq_setitem__";
PUBLIC_CONST char const DeeMA___seq_setitem___doc[] = "(index:?Dint,value)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *index, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__seq_setitem__", &index, &value))
		goto err;
	if (DeeSeq_OperatorSetItem(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_getrange___name[] = "__seq_getrange__";
PUBLIC_CONST char const DeeMA___seq_getrange___doc[] = "(start?:?X2?Dint?N,end?:?X2?Dint?N)->?S";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *start, *end;
	if (DeeArg_Unpack(argc, argv, "oo:__seq_getrange__", &start, &end))
		goto err;
	return DeeSeq_OperatorGetRange(self, start, end);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_delrange___name[] = "__seq_delrange__";
PUBLIC_CONST char const DeeMA___seq_delrange___doc[] = "(start?:?X2?Dint?N,end?:?X2?Dint?N)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_delrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *start, *end;
	if (DeeArg_Unpack(argc, argv, "oo:__seq_delrange__", &start, &end))
		goto err;
	if (DeeSeq_OperatorDelRange(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_setrange___name[] = "__seq_setrange__";
PUBLIC_CONST char const DeeMA___seq_setrange___doc[] = "(start?:?X2?Dint?N,end?:?X2?Dint?N,values:?S?O)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_setrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *start, *end, *values;
	if (DeeArg_Unpack(argc, argv, "ooo:__seq_setrange__", &start, &end, &values))
		goto err;
	if (DeeSeq_OperatorSetRange(self, start, end, values))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_hash___name[] = "__seq_hash__";
PUBLIC_CONST char const DeeMA___seq_hash___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__seq_hash__"))
		goto err;
	result = DeeSeq_OperatorHash(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_compare___name[] = "__seq_compare__";
PUBLIC_CONST char const DeeMA___seq_compare___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_compare__", &rhs))
		goto err;
	result = DeeSeq_OperatorCompare(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_compare_eq___name[] = "__seq_compare_eq__";
PUBLIC_CONST char const DeeMA___seq_compare_eq___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_compare_eq__", &rhs))
		goto err;
	result = DeeSeq_OperatorCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_trycompare_eq___name[] = "__seq_trycompare_eq__";
PUBLIC_CONST char const DeeMA___seq_trycompare_eq___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_trycompare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_trycompare_eq__", &rhs))
		goto err;
	result = DeeSeq_OperatorTryCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_eq___name[] = "__seq_eq__";
PUBLIC_CONST char const DeeMA___seq_eq___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_eq__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_ne___name[] = "__seq_ne__";
PUBLIC_CONST char const DeeMA___seq_ne___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ne__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_lo___name[] = "__seq_lo__";
PUBLIC_CONST char const DeeMA___seq_lo___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_lo__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_le___name[] = "__seq_le__";
PUBLIC_CONST char const DeeMA___seq_le___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_le__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_gr___name[] = "__seq_gr__";
PUBLIC_CONST char const DeeMA___seq_gr___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_gr__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_ge___name[] = "__seq_ge__";
PUBLIC_CONST char const DeeMA___seq_ge___doc[] = "(rhs:?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ge__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_inplace_add___name[] = "__seq_inplace_add__";
PUBLIC_CONST char const DeeMA___seq_inplace_add___doc[] = "(rhs:?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_inplace_add__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely(DeeSeq_OperatorInplaceAdd(&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_inplace_mul___name[] = "__seq_inplace_mul__";
PUBLIC_CONST char const DeeMA___seq_inplace_mul___doc[] = "(repeat:?Dint)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_inplace_mul__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *repeat;
	if (DeeArg_Unpack(argc, argv, "o:__seq_inplace_mul__", &repeat))
		goto err;
	Dee_Incref(self);
	if unlikely(DeeSeq_OperatorInplaceMul(&self, repeat))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_enumerate_items___name[] = "__seq_enumerate_items__";
PUBLIC_CONST char const DeeMA___seq_enumerate_items___doc[] = "(start?:?X2?Dint?O,end?:?X2?Dint?O)->?S?T2?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	size_t start, end;
	DeeObject *startob = NULL, *endob = NULL;
	if (DeeArg_Unpack(argc, argv, "|oo:__seq_enumerate_items__", &startob, &endob))
		goto err;
	if (endob) {
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end)))
			return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end);
		return DeeSeq_InvokeMakeEnumerationWithRange(self, startob, endob);
	} else if (startob) {
		if (DeeObject_AsSize(startob, &start))
			goto err;
		return DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, (size_t)-1);
	}
	return DeeSeq_InvokeMakeEnumeration(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_any___name[] = "__seq_any__";
PUBLIC_CONST char const DeeMA___seq_any___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_any_name[] = "any";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_any__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_any__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithKey(self, key)
		         : DeeSeq_InvokeAny(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAnyWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAnyWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_all___name[] = "__seq_all__";
PUBLIC_CONST char const DeeMA___seq_all___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_all_name[] = "all";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_all__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_all__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithKey(self, key)
		         : DeeSeq_InvokeAll(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAllWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_parity___name[] = "__seq_parity__";
PUBLIC_CONST char const DeeMA___seq_parity___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_parity_name[] = "parity";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_parity__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_parity__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeParityWithKey(self, key)
		         : DeeSeq_InvokeParity(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeParityWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeParityWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_min___name[] = "__seq_min__";
PUBLIC_CONST char const DeeMA___seq_min___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->";
PUBLIC_CONST char const DeeMA_Sequence_min_name[] = "min";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_min__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DREF DeeObject *result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_min__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeMinWithKey(self, key)
		         : DeeSeq_InvokeMin(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeMinWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeMinWithRange(self, start, end);
	}
	return result;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_max___name[] = "__seq_max__";
PUBLIC_CONST char const DeeMA___seq_max___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->";
PUBLIC_CONST char const DeeMA_Sequence_max_name[] = "max";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_max__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DREF DeeObject *result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_max__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeMaxWithKey(self, key)
		         : DeeSeq_InvokeMax(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeMaxWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeMaxWithRange(self, start, end);
	}
	return result;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_sum___name[] = "__seq_sum__";
PUBLIC_CONST char const DeeMA___seq_sum___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->";
PUBLIC_CONST char const DeeMA_Sequence_sum_name[] = "sum";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sum__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DREF DeeObject *result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":__seq_sum__",
	                    &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = DeeSeq_InvokeSum(self);
	} else {
		result = DeeSeq_InvokeSumWithRange(self, start, end);
	}
	return result;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_count___name[] = "__seq_count__";
PUBLIC_CONST char const DeeMA___seq_count___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_count_name[] = "count";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_count__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:count",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeCount(self, item);
		} else {
			result = DeeSeq_InvokeCountWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeCountWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeCountWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_contains___name[] = "__seq_contains__";
PUBLIC_CONST char const DeeMA___seq_contains___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:contains",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeContains(self, item);
		} else {
			result = DeeSeq_InvokeContainsWithKey(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeSeq_InvokeContainsWithRange(self, item, start, end);
		} else {
			result = DeeSeq_InvokeContainsWithRangeAndKey(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_find___name[] = "__seq_find__";
PUBLIC_CONST char const DeeMA___seq_find___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_find_name[] = "find";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_find__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_find__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeSeq_InvokeFindWithKey(self, item, start, end, key)
	         : DeeSeq_InvokeFind(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_erase___name[] = "__seq_erase__";
PUBLIC_CONST char const DeeMA___seq_erase___doc[] = "(index:?Dint,count=!1)";
PUBLIC_CONST char const DeeMA_Sequence_erase_name[] = "erase";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_erase__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t index, count = 1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_count,
	                    UNPuSIZ "|" UNPuSIZ ":__seq_erase__",
	                    &index, &count))
		goto err;
	if unlikely(DeeSeq_InvokeErase(self, index, count))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_insert___name[] = "__seq_insert__";
PUBLIC_CONST char const DeeMA___seq_insert___doc[] = "(index:?Dint,item)";
PUBLIC_CONST char const DeeMA_Sequence_insert_name[] = "insert";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_insert__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "|" UNPuSIZ ":__seq_insert__",
	                    &index, &item))
		goto err;
	if unlikely(DeeSeq_InvokeInsert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_insertall___name[] = "__seq_insertall__";
PUBLIC_CONST char const DeeMA___seq_insertall___doc[] = "(index:?Dint,items:?S?O)";
PUBLIC_CONST char const DeeMA_Sequence_insertall_name[] = "insertall";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_insertall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_items,
	                    UNPuSIZ "|" UNPuSIZ ":__seq_insertall__",
	                    &index, &items))
		goto err;
	if unlikely(DeeSeq_InvokeInsertAll(self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_pushfront___name[] = "__seq_pushfront__";
PUBLIC_CONST char const DeeMA___seq_pushfront___doc[] = "(item)";
PUBLIC_CONST char const DeeMA_Sequence_pushfront_name[] = "pushfront";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_pushfront__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:__seq_pushfront__", &item))
		goto err;
	if unlikely(DeeSeq_InvokePushFront(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_append___name[] = "__seq_append__";
PUBLIC_CONST char const DeeMA___seq_append___doc[] = "(item)";
PUBLIC_CONST char const DeeMA_Sequence_append_name[] = "append";
PUBLIC_CONST char const DeeMA_Sequence_pushback_name[] = "pushback";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_append__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:__seq_append__", &item))
		goto err;
	if unlikely(DeeSeq_InvokeAppend(self, item))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_extend___name[] = "__seq_extend__";
PUBLIC_CONST char const DeeMA___seq_extend___doc[] = "(items:?S?O)";
PUBLIC_CONST char const DeeMA_Sequence_extend_name[] = "extend";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_extend__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__seq_extend__", &items))
		goto err;
	if unlikely(DeeSeq_InvokeExtend(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_xchitem___name[] = "__seq_xchitem__";
PUBLIC_CONST char const DeeMA___seq_xchitem___doc[] = "(index:?Dint,item)->";
PUBLIC_CONST char const DeeMA_Sequence_xchitem_name[] = "xchitem";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_xchitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_item,
	                    UNPuSIZ "|" UNPuSIZ ":__seq_xchitem__",
	                    &index, &item))
		goto err;
	return DeeSeq_InvokeXchItemIndex(self, index, item);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_clear___name[] = "__seq_clear__";
PUBLIC_CONST char const DeeMA___seq_clear___doc[] = "";
PUBLIC_CONST char const DeeMA_Sequence_clear_name[] = "clear";
PUBLIC_CONST char const DeeMA_Set_clear_name[] = "clear";
PUBLIC_CONST char const DeeMA_Mapping_clear_name[] = "clear";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_clear__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_clear__"))
		goto err;
	if (DeeSeq_InvokeClear(self))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_pop___name[] = "__seq_pop__";
PUBLIC_CONST char const DeeMA___seq_pop___doc[] = "(index=!-1)->";
PUBLIC_CONST char const DeeMA_Sequence_pop_name[] = "pop";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	Dee_ssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index,
	                    "|" UNPdSIZ ":__seq_pop__", &index))
		goto err;
	return DeeSeq_InvokePop(self, index);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_iter___name[] = "__set_iter__";
PUBLIC_CONST char const DeeMA___set_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_iter__"))
		goto err;
	return DeeSet_OperatorIter(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_size___name[] = "__set_size__";
PUBLIC_CONST char const DeeMA___set_size___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_size__"))
		goto err;
	return DeeSet_OperatorSizeOb(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_hash___name[] = "__set_hash__";
PUBLIC_CONST char const DeeMA___set_hash___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__set_hash__"))
		goto err;
	result = DeeSet_OperatorHash(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_getitem___name[] = "__map_getitem__";
PUBLIC_CONST char const DeeMA___map_getitem___doc[] = "(key)->";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_getitem__", &key))
		goto err;
	return DeeMap_OperatorGetItem(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_contains___name[] = "__map_contains__";
PUBLIC_CONST char const DeeMA___map_contains___doc[] = "(key)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_contains__", &key))
		goto err;
	return DeeMap_OperatorContains(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_enumerate___name[] = "__map_enumerate__";
PUBLIC_CONST char const DeeMA___map_enumerate___doc[] = "(cb:?DCallable,startkey?:?Dint,endkey?:?Dint)->?X2?O?N";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	size_t startkey = 0;
	size_t endkey = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":__map_enumerate__", &data.sed_cb, &startkey, &endkey))
		goto err;
	if (startkey == 0 && endkey == (size_t)-1) {
		foreach_status = DeeMap_OperatorEnumerate(self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = DeeMap_OperatorEnumerateIndex(self, &seq_enumerate_index_cb, &data, startkey, endkey);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C */
