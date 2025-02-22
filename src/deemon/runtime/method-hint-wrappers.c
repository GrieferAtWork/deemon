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
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/tuple.h>

/**/
#include "../objects/seq/enumerate-cb.h"
#include "runtime_error.h"

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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_iter___name[] = "__seq_iter__";
PUBLIC_CONST char const DeeMA___seq_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
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
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem))(self, index))
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
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem))(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_getrange___name[] = "__seq_getrange__";
PUBLIC_CONST char const DeeMA___seq_getrange___doc[] = "(start?:?X2?Dint?N,end?:?X2?Dint?N)->?S";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *start = Dee_None, *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|oo:__seq_getrange__", &start, &end))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange))(self, start, end);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_delrange___name[] = "__seq_delrange__";
PUBLIC_CONST char const DeeMA___seq_delrange___doc[] = "(start?:?X2?Dint?N,end?:?X2?Dint?N)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_delrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *start = Dee_None, *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|oo:__seq_delrange__", &start, &end))
		goto err;
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_setrange___name[] = "__seq_setrange__";
PUBLIC_CONST char const DeeMA___seq_setrange___doc[] = "(start:?X2?Dint?N,end:?X2?Dint?N,items:?X2?DSequence?S?O)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_setrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *start, *end, *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_values,
	                    "ooo:__seq_setrange__", &start, &end, &items))
		goto err;
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, start, end, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_assign___name[] = "__seq_assign__";
PUBLIC_CONST char const DeeMA___seq_assign___doc[] = "(items:?X2?DSequence?S?O)";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_assign__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__seq_assign__", &items))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_assign))(self, items))
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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hash))(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_compare___name[] = "__seq_compare__";
PUBLIC_CONST char const DeeMA___seq_compare___doc[] = "(rhs:?S?O)->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_compare__", &rhs))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_compare))(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_compare_eq___name[] = "__seq_compare_eq__";
PUBLIC_CONST char const DeeMA___seq_compare_eq___doc[] = "(rhs:?S?O)->?X2?Dbool?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_compare_eq__", &rhs))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_compare_eq))(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_eq))(self, rhs);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ne))(self, rhs);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_lo))(self, rhs);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_le))(self, rhs);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_gr))(self, rhs);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ge))(self, rhs);
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_inplace_add))((DeeObject **)&self, rhs))
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_inplace_mul))((DeeObject **)&self, repeat))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
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
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate))(self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_enumerate_index_cb, &data, start, end);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
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

PUBLIC_CONST char const DeeMA___seq_unpack___name[] = "__seq_unpack__";
PUBLIC_CONST char const DeeMA___seq_unpack___doc[] = "(min:?Dint,max?:?Dint)->?DTuple";
PUBLIC_CONST char const DeeMA_Sequence_unpack_name[] = "unpack";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_unpack__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DREF DeeTupleObject *result;
	size_t min_count, max_count;
	if (argc == 1) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
handle_single_count:
		result = DeeTuple_NewUninitialized(min_count);
		if unlikely(!result)
			goto err;
		if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_unpack))(self, min_count, result->t_elem))
			goto err_r;
	} else if (argc == 2) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = DeeObject_AsDirectSize(argv[1]);
		if unlikely(max_count == (size_t)-1)
			goto err;
		if unlikely(min_count >= max_count) {
			if (min_count == max_count)
				goto handle_single_count;
			DeeError_Throwf(&DeeError_ValueError,
			                "In __seq_unpack__: min(%" PRFuSIZ ") "
			                "is greater than max(%" PRFuSIZ ")",
			                min_count, max_count);
			goto err;
		}
		result = DeeTuple_NewUninitialized(max_count);
		if unlikely(!result)
			goto err;
		min_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_unpack_ex))(self, min_count, max_count, result->t_elem);
		if unlikely(min_count == (size_t)-1)
			goto err_r;
		ASSERT(min_count <= max_count);
		if (min_count < max_count)
			result = DeeTuple_TruncateUninitialized(result, min_count);
	} else {
		err_invalid_argc("__seq_unpack__", argc, 1, 2);
		goto err;
	}
	return (DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_unpackub___name[] = "__seq_unpackub__";
PUBLIC_CONST char const DeeMA___seq_unpackub___doc[] = "(min:?Dint,max?:?Dint)->?Ert:NullableTuple";
PUBLIC_CONST char const DeeMA_Sequence_unpackub_name[] = "unpackub";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_unpackub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DREF DeeTupleObject *result;
	size_t min_count, max_count;
	if (argc == 1) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = min_count;
	} else if (argc == 2) {
		min_count = DeeObject_AsDirectSize(argv[0]);
		if unlikely(min_count == (size_t)-1)
			goto err;
		max_count = DeeObject_AsDirectSize(argv[1]);
		if unlikely(max_count == (size_t)-1)
			goto err;
		if unlikely(min_count > max_count) {
			DeeError_Throwf(&DeeError_ValueError,
			                "In __seq_unpackub__: min(%" PRFuSIZ ") "
			                "is greater than max(%" PRFuSIZ ")",
			                min_count, max_count);
			goto err;
		}
	} else {
		err_invalid_argc("__seq_unpackub__", argc, 1, 2);
		goto err;
	}
	result = DeeTuple_NewUninitialized(max_count);
	if unlikely(!result)
		goto err;
	min_count = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_unpack_ub))(self, min_count, max_count, result->t_elem);
	if unlikely(min_count == (size_t)-1)
		goto err_r;
	ASSERT(min_count <= max_count);
	if (min_count < max_count)
		result = DeeTuple_TruncateUninitialized(result, min_count);
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&DeeNullableTuple_Type);
	ASSERT(result->ob_type == &DeeTuple_Type);
	result->ob_type = &DeeNullableTuple_Type;
	return (DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
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
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_key))(self, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any))(self);
	} else {
		result = !DeeNone_Check(key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range_and_key))(self, start, end, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range))(self, start, end);
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
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_key))(self, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all))(self);
	} else {
		result = !DeeNone_Check(key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range_and_key))(self, start, end, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range))(self, start, end);
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
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_key))(self, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity))(self);
	} else {
		result = !DeeNone_Check(key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range_and_key))(self, start, end, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range))(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_reduce___name[] = "__seq_reduce__";
PUBLIC_CONST char const DeeMA___seq_reduce___doc[] = "(combine:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,init?)->";
PUBLIC_CONST char const DeeMA_Sequence_reduce_name[] = "reduce";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reduce__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *combine, *init = NULL;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__combine_start_end_init,
	                    "o|" UNPuSIZ UNPuSIZ "o:reduce",
	                    &combine, &start, &end, &init))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (init)
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_init))(self, combine, init);
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce))(self, combine);
	}
	if (init)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_range_and_init))(self, combine, start, end, init);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_range))(self, combine, start, end);
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
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_key))(self, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min))(self);
	} else {
		result = !DeeNone_Check(key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_range_and_key))(self, start, end, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_range))(self, start, end);
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
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_key))(self, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max))(self);
	} else {
		result = !DeeNone_Check(key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_range_and_key))(self, start, end, key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_range))(self, start, end);
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
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sum))(self);
	} else {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sum_with_range))(self, start, end);
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
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_count__",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count))(self, item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_key))(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_range))(self, item, start, end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_range_and_key))(self, item, start, end, key);
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
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains))(self, item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_key))(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_range))(self, item, start, end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_range_and_key))(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_locate___name[] = "__seq_locate__";
PUBLIC_CONST char const DeeMA___seq_locate___doc[] = "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]";
PUBLIC_CONST char const DeeMA_Sequence_locate_name[] = "locate";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_locate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *match, *def = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__match_start_end_def,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_locate__",
	                    &match, &start, &end, &def))
		goto err;
	if (start == 0 && end == (size_t)-1)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_locate))(self, match, def);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_locate_with_range))(self, match, start, end, def);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_rlocate___name[] = "__seq_rlocate__";
PUBLIC_CONST char const DeeMA___seq_rlocate___doc[] = "(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef]";
PUBLIC_CONST char const DeeMA_Sequence_rlocate_name[] = "rlocate";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rlocate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *match, *def = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__match_start_end_def,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_rlocate__",
	                    &match, &start, &end, &def))
		goto err;
	if (start == 0 && end == (size_t)-1)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rlocate))(self, match, def);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rlocate_with_range))(self, match, start, end, def);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_startswith___name[] = "__seq_startswith__";
PUBLIC_CONST char const DeeMA___seq_startswith___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_startswith_name[] = "startswith";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_startswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_startswith__",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith))(self, item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_key))(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_range))(self, item, start, end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_range_and_key))(self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_endswith___name[] = "__seq_endswith__";
PUBLIC_CONST char const DeeMA___seq_endswith___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_endswith_name[] = "endswith";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_endswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_endswith__",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith))(self, item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_key))(self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_range))(self, item, start, end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_range_and_key))(self, item, start, end, key);
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
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_find__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find))(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_rfind___name[] = "__seq_rfind__";
PUBLIC_CONST char const DeeMA___seq_rfind___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_rfind_name[] = "rfind";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rfind__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_rfind__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind))(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, index, count))
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
	                    UNPuSIZ "o:__seq_insert__",
	                    &index, &item))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insert))(self, index, item))
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
	                    UNPuSIZ "o:__seq_insertall__",
	                    &index, &items))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insertall))(self, index, items))
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pushfront))(self, item))
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_append))(self, item))
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_extend))(self, items))
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
	                    UNPuSIZ "o:__seq_xchitem__",
	                    &index, &item))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_xchitem_index))(self, index, item);
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
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_clear))(self))
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pop))(self, index);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_remove___name[] = "__seq_remove__";
PUBLIC_CONST char const DeeMA___seq_remove___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_remove_name[] = "remove";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_remove__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove))(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_rremove___name[] = "__seq_rremove__";
PUBLIC_CONST char const DeeMA___seq_rremove___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool";
PUBLIC_CONST char const DeeMA_Sequence_rremove_name[] = "rremove";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rremove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_rremove__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rremove_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rremove))(self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_removeall___name[] = "__seq_removeall__";
PUBLIC_CONST char const DeeMA___seq_removeall___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_removeall_name[] = "removeall";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_removeall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_max_key,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ "o:removeall",
	                    &item, &start, &end, &max, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall_with_key))(self, item, start, end, max, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall))(self, item, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_removeif___name[] = "__seq_removeif__";
PUBLIC_CONST char const DeeMA___seq_removeif___doc[] = "(should:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_removeif_name[] = "removeif";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_removeif__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t result;
	DeeObject *should;
	size_t start = 0, end = (size_t)-1, max = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__should_start_end_max,
	                    "o|" UNPuSIZ UNPuSIZ UNPuSIZ ":removeif",
	                    &should, &start, &end, &max))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeif))(self, should, start, end, max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_resize___name[] = "__seq_resize__";
PUBLIC_CONST char const DeeMA___seq_resize___doc[] = "(size:?Dint,filler=!N)";
PUBLIC_CONST char const DeeMA_Sequence_resize_name[] = "resize";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_resize__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t size;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__size_filler,
	                    UNPuSIZ "|o:resize", &size, &filler))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_resize))(self, size, filler))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_fill___name[] = "__seq_fill__";
PUBLIC_CONST char const DeeMA___seq_fill___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,filler=!N)";
PUBLIC_CONST char const DeeMA_Sequence_fill_name[] = "fill";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_fill__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t start = 0, end = (size_t)-1;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_filler,
	                    "|" UNPuSIZ UNPuSIZ "o:fill",
	                    &start, &end, &filler))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_fill))(self, start, end, filler))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_reverse___name[] = "__seq_reverse__";
PUBLIC_CONST char const DeeMA___seq_reverse___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)";
PUBLIC_CONST char const DeeMA_Sequence_reverse_name[] = "reverse";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reverse__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reverse",
	                    &start, &end))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reverse))(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_reversed___name[] = "__seq_reversed__";
PUBLIC_CONST char const DeeMA___seq_reversed___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?DSequence";
PUBLIC_CONST char const DeeMA_Sequence_reversed_name[] = "reversed";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reversed__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":reversed",
	                    &start, &end))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reversed))(self, start, end);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_sort___name[] = "__seq_sort__";
PUBLIC_CONST char const DeeMA___seq_sort___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)";
PUBLIC_CONST char const DeeMA_Sequence_sort_name[] = "sort";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sort__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sort",
	                    &start, &end, &key))
		goto err;
	if unlikely(!DeeNone_Check(key)
	            ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort_with_key))(self, start, end, key)
	            : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort))(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_sorted___name[] = "__seq_sorted__";
PUBLIC_CONST char const DeeMA___seq_sorted___doc[] = "(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)";
PUBLIC_CONST char const DeeMA_Sequence_sorted_name[] = "sorted";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sorted__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	size_t start = 0, end = (size_t)-1;
	DeeObject *key = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:sorted",
	                    &start, &end, &key))
		goto err;
	return !DeeNone_Check(key)
	       ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, start, end, key)
	       : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted))(self, start, end);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_bfind___name[] = "__seq_bfind__";
PUBLIC_CONST char const DeeMA___seq_bfind___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?X2?Dint?N";
PUBLIC_CONST char const DeeMA_Sequence_bfind_name[] = "bfind";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bfind__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_bfind__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bfind_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bfind))(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_bposition___name[] = "__seq_bposition__";
PUBLIC_CONST char const DeeMA___seq_bposition___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint";
PUBLIC_CONST char const DeeMA_Sequence_bposition_name[] = "bposition";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bposition__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:bposition",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bposition_with_key))(self, item, start, end, key)
	         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bposition))(self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___seq_brange___name[] = "__seq_brange__";
PUBLIC_CONST char const DeeMA___seq_brange___doc[] = "(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?T2?Dint?Dint";
PUBLIC_CONST char const DeeMA_Sequence_brange_name[] = "brange";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_brange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw){
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1, result_range[2];
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:brange",
	                    &item, &start, &end, &key))
		goto err;
	if (!DeeNone_Check(key)
	    ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_brange_with_key))(self, item, start, end, key, result_range)
	    : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_brange))(self, item, start, end, result_range))
		goto err;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, result_range[0], result_range[1]);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_iter___name[] = "__set_iter__";
PUBLIC_CONST char const DeeMA___set_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_size___name[] = "__set_size__";
PUBLIC_CONST char const DeeMA___set_size___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_size__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
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
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_hash))(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_compare_eq___name[] = "__set_compare_eq__";
PUBLIC_CONST char const DeeMA___set_compare_eq___doc[] = "(rhs:?X2?DSet?S?O)->?X2?Dbool?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_compare_eq__", &rhs))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_compare_eq))(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_eq___name[] = "__set_eq__";
PUBLIC_CONST char const DeeMA___set_eq___doc[] = "(rhs:?X2?DSet?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_eq__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_eq))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_ne___name[] = "__set_ne__";
PUBLIC_CONST char const DeeMA___set_ne___doc[] = "(rhs:?X2?DSet?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ne__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ne))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_lo___name[] = "__set_lo__";
PUBLIC_CONST char const DeeMA___set_lo___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_lo__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_lo))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_le___name[] = "__set_le__";
PUBLIC_CONST char const DeeMA___set_le___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?Dbool";
PUBLIC_CONST char const DeeMA_Set_issubset_name[] = "issubset";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_le__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_le))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_gr___name[] = "__set_gr__";
PUBLIC_CONST char const DeeMA___set_gr___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_gr__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_gr))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_ge___name[] = "__set_ge__";
PUBLIC_CONST char const DeeMA___set_ge___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?Dbool";
PUBLIC_CONST char const DeeMA_Set_issuperset_name[] = "issuperset";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ge__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ge))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_inv___name[] = "__set_inv__";
PUBLIC_CONST char const DeeMA___set_inv___doc[] = "->?DSet";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inv__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_inv__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inv))(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_add___name[] = "__set_add__";
PUBLIC_CONST char const DeeMA___set_add___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?DSet";
PUBLIC_CONST char const DeeMA_Set_union_name[] = "union";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_add__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_add))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_sub___name[] = "__set_sub__";
PUBLIC_CONST char const DeeMA___set_sub___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?DSet";
PUBLIC_CONST char const DeeMA_Set_difference_name[] = "difference";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_sub__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sub))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_and___name[] = "__set_and__";
PUBLIC_CONST char const DeeMA___set_and___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?DSet";
PUBLIC_CONST char const DeeMA_Set_intersection_name[] = "intersection";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_and__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_and))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_xor___name[] = "__set_xor__";
PUBLIC_CONST char const DeeMA___set_xor___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?DSet";
PUBLIC_CONST char const DeeMA_Set_symmetric_difference_name[] = "symmetric_difference";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_xor__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_xor))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_inplace_add___name[] = "__set_inplace_add__";
PUBLIC_CONST char const DeeMA___set_inplace_add___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_inplace_add__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_add))((DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_inplace_sub___name[] = "__set_inplace_sub__";
PUBLIC_CONST char const DeeMA___set_inplace_sub___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_inplace_sub__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_sub))((DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_inplace_and___name[] = "__set_inplace_and__";
PUBLIC_CONST char const DeeMA___set_inplace_and___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_inplace_and__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_and))((DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_inplace_xor___name[] = "__set_inplace_xor__";
PUBLIC_CONST char const DeeMA___set_inplace_xor___doc[] = "(rhs:?X3?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_inplace_xor__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_xor))((DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_unify___name[] = "__set_unify__";
PUBLIC_CONST char const DeeMA___set_unify___doc[] = "(key)->";
PUBLIC_CONST char const DeeMA_Set_unify_name[] = "unify";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_unify__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_unify__", &key))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_unify))(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_insert___name[] = "__set_insert__";
PUBLIC_CONST char const DeeMA___set_insert___doc[] = "(key)->?Dbool";
PUBLIC_CONST char const DeeMA_Set_insert_name[] = "insert";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_insert__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_insert__", &key))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_insert))(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_insertall___name[] = "__set_insertall__";
PUBLIC_CONST char const DeeMA___set_insertall___doc[] = "(keys:?X3?DSet?DSequence?S?O)->?Dbool";
PUBLIC_CONST char const DeeMA_Set_insertall_name[] = "insertall";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_insertall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__set_insertall__", &keys))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_insertall))(self, keys);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_remove___name[] = "__set_remove__";
PUBLIC_CONST char const DeeMA___set_remove___doc[] = "(key)->?Dbool";
PUBLIC_CONST char const DeeMA_Set_remove_name[] = "remove";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_remove__", &key))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_remove))(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_removeall___name[] = "__set_removeall__";
PUBLIC_CONST char const DeeMA___set_removeall___doc[] = "(keys:?X3?DSet?DSequence?S?O)";
PUBLIC_CONST char const DeeMA_Set_removeall_name[] = "removeall";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_removeall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__set_removeall__", &keys))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_removeall))(self, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___set_pop___name[] = "__set_pop__";
PUBLIC_CONST char const DeeMA___set_pop___doc[] = "(def?)->";
PUBLIC_CONST char const DeeMA_Set_pop_name[] = "pop";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *def = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:__set_pop__", &def))
		goto err;
	return def ? (*DeeType_RequireMethodHint(Dee_TYPE(self), set_pop_with_default))(self, def)
	           : (*DeeType_RequireMethodHint(Dee_TYPE(self), set_pop))(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_iter___name[] = "__map_iter__";
PUBLIC_CONST char const DeeMA___map_iter___doc[] = "->?DIterator";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__map_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_size___name[] = "__map_size__";
PUBLIC_CONST char const DeeMA___map_size___doc[] = "->?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__map_size__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sizeob))(self);
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_delitem___name[] = "__map_delitem__";
PUBLIC_CONST char const DeeMA___map_delitem___doc[] = "(key)->";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_delitem__", &key))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, key))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setitem___name[] = "__map_setitem__";
PUBLIC_CONST char const DeeMA___map_setitem___doc[] = "(key,value)->";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setitem__", &key, &value))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, key, value))
		goto err;
	return_none;
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
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_contains))(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_enumerate___name[] = "__map_enumerate__";
PUBLIC_CONST char const DeeMA___map_enumerate___doc[] = "(cb:?DCallable,start?,end?)->?X2?O?N";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	DeeObject *start, *end = NULL;
	if (DeeArg_Unpack(argc, argv, "o|oo:__map_enumerate__", &data.sed_cb, &start, &end))
		goto err;
	if (end) {
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate_range))(self, &seq_enumerate_cb, &data, start, end);
	} else {
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &seq_enumerate_cb, &data);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_enumerate_items___name[] = "__map_enumerate_items__";
PUBLIC_CONST char const DeeMA___map_enumerate_items___doc[] = "(start?,end?)->?S?T2";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *start, *end = NULL;
	if (DeeArg_Unpack(argc, argv, "|oo:__map_enumerate_items__", &start, &end))
		goto err;
	if (end)
		return DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, start, end);
	return DeeObject_InvokeMethodHint(map_makeenumeration, self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_compare_eq___name[] = "__map_compare_eq__";
PUBLIC_CONST char const DeeMA___map_compare_eq___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?X2?Dbool?Dint";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_compare_eq__", &rhs))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_compare_eq))(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_eq___name[] = "__map_eq__";
PUBLIC_CONST char const DeeMA___map_eq___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_eq__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_eq))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_ne___name[] = "__map_ne__";
PUBLIC_CONST char const DeeMA___map_ne___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_ne__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ne))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_lo___name[] = "__map_lo__";
PUBLIC_CONST char const DeeMA___map_lo___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_lo__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_lo))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_le___name[] = "__map_le__";
PUBLIC_CONST char const DeeMA___map_le___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_le__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_le))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_gr___name[] = "__map_gr__";
PUBLIC_CONST char const DeeMA___map_gr___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_gr__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_gr))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_ge___name[] = "__map_ge__";
PUBLIC_CONST char const DeeMA___map_ge___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_ge__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ge))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_add___name[] = "__map_add__";
PUBLIC_CONST char const DeeMA___map_add___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?DMapping";
PUBLIC_CONST char const DeeMA_Mapping_union_name[] = "union";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_add__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_add))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_sub___name[] = "__map_sub__";
PUBLIC_CONST char const DeeMA___map_sub___doc[] = "(keys:?X2?DSet?S?O)->?DMapping";
PUBLIC_CONST char const DeeMA_Mapping_difference_name[] = "difference";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_sub__", &keys))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sub))(self, keys);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_and___name[] = "__map_and__";
PUBLIC_CONST char const DeeMA___map_and___doc[] = "(keys:?X2?DSet?S?O)->?DMapping";
PUBLIC_CONST char const DeeMA_Mapping_intersection_name[] = "intersection";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_and__", &keys))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_and))(self, keys);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_xor___name[] = "__map_xor__";
PUBLIC_CONST char const DeeMA___map_xor___doc[] = "(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping";
PUBLIC_CONST char const DeeMA_Mapping_symmetric_difference_name[] = "symmetric_difference";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_xor__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_xor))(self, rhs);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_inplace_add___name[] = "__map_inplace_add__";
PUBLIC_CONST char const DeeMA___map_inplace_add___doc[] = "(items:?X3?DMapping?M?O?O?S?T2?O?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__map_inplace_add__", &items))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_add))((DeeObject **)&self, items))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_inplace_sub___name[] = "__map_inplace_sub__";
PUBLIC_CONST char const DeeMA___map_inplace_sub___doc[] = "(keys:?X2?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_inplace_sub__", &keys))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_sub))((DeeObject **)&self, keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_inplace_and___name[] = "__map_inplace_and__";
PUBLIC_CONST char const DeeMA___map_inplace_and___doc[] = "(keys:?X2?DSet?DSequence?S?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_inplace_and__", &keys))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_and))((DeeObject **)&self, keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_inplace_xor___name[] = "__map_inplace_xor__";
PUBLIC_CONST char const DeeMA___map_inplace_xor___doc[] = "(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?.";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_inplace_xor__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_xor))((DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setold___name[] = "__map_setold__";
PUBLIC_CONST char const DeeMA___map_setold___doc[] = "(key,value)->?Dbool";
PUBLIC_CONST char const DeeMA_Mapping_setold_name[] = "setold";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setold__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setold__", &key, &value))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold))(self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setold_ex___name[] = "__map_setold_ex__";
PUBLIC_CONST char const DeeMA___map_setold_ex___doc[] = "(key,value)->?T2?Dbool?X2?O?N";
PUBLIC_CONST char const DeeMA_Mapping_setold_ex_name[] = "setold_ex";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setold_ex__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	PRIVATE DEFINE_TUPLE(setold_failed_result, 2, { Dee_False, Dee_None });
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setold_ex__", &key, &value))
		goto err;
	old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold_ex))(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setold_failed_result);
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_True);
	DeeTuple_SET(result, 0, Dee_True);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setnew___name[] = "__map_setnew__";
PUBLIC_CONST char const DeeMA___map_setnew___doc[] = "(key,value)->?Dbool";
PUBLIC_CONST char const DeeMA_Mapping_setnew_name[] = "setnew";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setnew__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setnew__", &key, &value))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew))(self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setnew_ex___name[] = "__map_setnew_ex__";
PUBLIC_CONST char const DeeMA___map_setnew_ex___doc[] = "(key,value)->?T2?Dbool?X2?O?N";
PUBLIC_CONST char const DeeMA_Mapping_setnew_ex_name[] = "setnew_ex";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setnew_ex__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	PRIVATE DEFINE_TUPLE(setnew_success_result, 2, { Dee_True, Dee_None });
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setnew_ex__", &key, &value))
		goto err;
	old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew_ex))(self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setnew_success_result);
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_False);
	DeeTuple_SET(result, 0, Dee_False);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_setdefault___name[] = "__map_setdefault__";
PUBLIC_CONST char const DeeMA___map_setdefault___doc[] = "(key,value)->";
PUBLIC_CONST char const DeeMA_Mapping_setdefault_name[] = "setdefault";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setdefault__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setdefault__", &key, &value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setdefault))(self, key, value);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_update___name[] = "__map_update__";
PUBLIC_CONST char const DeeMA___map_update___doc[] = "(items:?X3?DMapping?M?O?O?S?T2?O?O)";
PUBLIC_CONST char const DeeMA_Mapping_update_name[] = "update";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_update__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__map_update__", &items))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_update))(self, items))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_remove___name[] = "__map_remove__";
PUBLIC_CONST char const DeeMA___map_remove___doc[] = "(key)->?Dbool";
PUBLIC_CONST char const DeeMA_Mapping_remove_name[] = "remove";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_remove__", &key))
		goto err;
	result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_remove))(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_removekeys___name[] = "__map_removekeys__";
PUBLIC_CONST char const DeeMA___map_removekeys___doc[] = "(keys:?X3?DSet?DSequence?S?O)";
PUBLIC_CONST char const DeeMA_Mapping_removekeys_name[] = "removekeys";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_removekeys__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_removekeys__", &keys))
		goto err;
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_removekeys))(self, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_pop___name[] = "__map_pop__";
PUBLIC_CONST char const DeeMA___map_pop___doc[] = "(key,def?)->";
PUBLIC_CONST char const DeeMA_Mapping_pop_name[] = "pop";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:__map_pop__", &key, &def))
		goto err;
	return def ? (*DeeType_RequireMethodHint(Dee_TYPE(self), map_pop_with_default))(self, key, def)
	           : (*DeeType_RequireMethodHint(Dee_TYPE(self), map_pop))(self, key);
err:
	return NULL;
}

PUBLIC_CONST char const DeeMA___map_popitem___name[] = "__map_popitem__";
PUBLIC_CONST char const DeeMA___map_popitem___doc[] = "->?X2?T2?O?O?N";
PUBLIC_CONST char const DeeMA_Mapping_popitem_name[] = "popitem";
PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_popitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__map_popitem__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_popitem))(self);
err:
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C */
