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
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/tuple.h>

#include "../objects/seq/enumerate-cb.h"
#include "runtime_error.h"

/**/
#include "kwlist.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMethodAttributeImpls from "..method-hints.method-hints")();]]]*/
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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_size__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__seq_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__seq_getitem__", &index))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, index);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_eq__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_eq))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ne__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ne))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_lo__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_lo))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_le__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_le))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_gr__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_gr))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ge__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ge))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_add__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_add))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_mul__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *repeat;
	if (DeeArg_Unpack(argc, argv, "o:__seq_mul__", &repeat))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_mul))(self, repeat);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	size_t start, end;
	DeeObject *startob = NULL, *endob = NULL;
	if (DeeArg_Unpack(argc, argv, "|oo:__seq_enumerate_items__", &startob, &endob))
		goto err;
	if (endob) {
		if ((DeeInt_Check(startob) && DeeInt_Check(endob)) &&
		    (DeeInt_TryAsSize(startob, &start) && DeeInt_TryAsSize(endob, &end)))
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_intrange))(self, start, end);
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_range))(self, startob, endob);
	} else if (startob) {
		if (DeeObject_AsSize(startob, &start))
			goto err;
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_intrange))(self, start, (size_t)-1);
	}
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration))(self);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_size__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_eq__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_eq))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ne__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ne))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_lo__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_lo))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_le__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_le))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_gr__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_gr))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ge__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ge))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inv__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__set_inv__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inv))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_add__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_add))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_sub__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sub))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_and__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_and))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_xor__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_xor))(self, rhs);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_unify__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_unify__", &key))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_unify))(self, key);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__map_iter__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	if (DeeArg_Unpack(argc, argv, ":__map_size__"))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sizeob))(self);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_getitem__", &key))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, key);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_contains__", &key))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_contains))(self, key);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_eq__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_eq))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_ne__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ne))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_lo__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_lo))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_le__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_le))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_gr__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_gr))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_ge__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ge))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_add__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_add))(self, rhs);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_sub__", &keys))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sub))(self, keys);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_and__", &keys))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_and))(self, keys);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_xor__", &rhs))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_xor))(self, rhs);
err:
	return NULL;
}

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

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setdefault__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv){
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setdefault__", &key, &value))
		goto err;
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setdefault))(self, key, value);
err:
	return NULL;
}

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

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C */
