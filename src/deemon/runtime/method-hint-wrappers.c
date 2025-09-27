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
DeeMA___seq_bool__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__seq_bool__");
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_bool))(self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__seq_size__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_sizeob))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__seq_iter__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_iter))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *index;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_getitem__", &args.index);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getitem))(self, args.index);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *index;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_delitem__", &args.index);
{
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delitem))(self, args.index))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *index;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__seq_setitem__", &args.index, &args.value);
{
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setitem))(self, args.index, args.value))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_getrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *start;
		DeeObject *end;
	} args;
	args.start = Dee_None;
	args.end = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|oo:__seq_getrange__", &args))
		goto err;
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_getrange))(self, args.start, args.end);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_delrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *start;
		DeeObject *end;
	} args;
	args.start = Dee_None;
	args.end = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|oo:__seq_delrange__", &args))
		goto err;
{
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_delrange))(self, args.start, args.end))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_setrange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *start;
		DeeObject *end;
		DeeObject *items;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_items, "ooo:__seq_setrange__", &args))
		goto err;
{
	if ((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_setrange))(self, args.start, args.end, args.items))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_assign__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *items;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_assign__", &args.items);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_assign))(self, args.items))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__seq_hash__");
{
	Dee_hash_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_hash))(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_compare__", &args.rhs);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_compare))(self, args.rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_compare_eq__", &args.rhs);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_compare_eq))(self, args.rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_eq__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_eq))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_ne__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ne))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_lo__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_lo))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_le__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_le))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_gr__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_gr))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_ge__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_ge))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_add__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_add))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_mul__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *repeat;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_mul__", &args.repeat);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_mul))(self, args.repeat);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_inplace_add__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_inplace_add))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_inplace_mul__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *repeat;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_inplace_mul__", &args.repeat);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_operator_inplace_mul))((DeeObject **)&self, args.repeat))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *cb;
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "o|" UNPuSIZ UNPxSIZ ":__seq_enumerate__", &args))
		goto err;
{
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	data.sed_cb = args.cb;
	if (args.start == 0 && args.end == (size_t)-1) {
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate))(self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_enumerate_index))(self, &seq_enumerate_index_cb, &data, args.start, args.end);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *start;
		DeeObject *end;
	} args;
	args.start = NULL;
	args.end = NULL;
	args.start = NULL;
	args.end = NULL;
	_DeeArg_Unpack0Or1Or2(err, argc, argv, "__seq_enumerate_items__", &args.start, &args.end);
{
	size_t start_index, end_index;
	if (args.end) {
		if ((DeeInt_Check(args.start) && DeeInt_Check(args.end)) &&
		    (DeeInt_TryAsSize(args.start, &start_index) && DeeInt_TryAsSize(args.end, &end_index)))
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_intrange))(self, start_index, end_index);
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_range))(self, args.start, args.end);
	} else if (args.start) {
		if (DeeObject_AsSize(args.start, &start_index))
			goto err;
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration_with_intrange))(self, start_index, (size_t)-1);
	}
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_makeenumeration))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_unpack__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
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
DeeMA___seq_unpackub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
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
DeeMA___seq_any__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1) {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_key))(self, argv[0]);
		goto check_result;
	}
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any))(self);
			goto check_result;
		case 1:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_key))(self, argv[0]);
			goto check_result;
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2) {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range))(self, start, end);
			} else {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range_and_key))(self, start, end, argv[2]);
			}
			goto check_result;
		default:
			err_invalid_argc("__seq_any__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_any__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCanyable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_any__", &args))
		goto err;
/*[[[end]]]*/
	if (args.start == 0 && args.end == (size_t)-1) {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_key))(self, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any))(self);
	} else {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range_and_key))(self, args.start, args.end, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_any_with_range))(self, args.start, args.end);
	}
check_result:
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_all__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1) {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_key))(self, argv[0]);
		goto check_result;
	}
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all))(self);
			goto check_result;
		case 1:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_key))(self, argv[0]);
			goto check_result;
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2) {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range))(self, start, end);
			} else {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range_and_key))(self, start, end, argv[2]);
			}
			goto check_result;
		default:
			err_invalid_argc("__seq_all__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_all__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_all__", &args))
		goto err;
/*[[[end]]]*/
	if (args.start == 0 && args.end == (size_t)-1) {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_key))(self, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all))(self);
	} else {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range_and_key))(self, args.start, args.end, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_all_with_range))(self, args.start, args.end);
	}
check_result:
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_parity__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1) {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_key))(self, argv[0]);
		goto check_result;
	}
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity))(self);
			goto check_result;
		case 1:
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_key))(self, argv[0]);
			goto check_result;
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2) {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range))(self, start, end);
			} else {
				result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range_and_key))(self, start, end, argv[2]);
			}
			goto check_result;
		default:
			err_invalid_argc("__seq_parity__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_parity__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCparityable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_parity__", &args))
		goto err;
/*[[[end]]]*/
	if (args.start == 0 && args.end == (size_t)-1) {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_key))(self, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity))(self);
	} else {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range_and_key))(self, args.start, args.end, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_parity_with_range))(self, args.start, args.end);
	}
check_result:
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reduce__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *combine;
		size_t start;
		size_t end;
		DeeObject *init;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.init = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__combine_start_end_init, "o|" UNPuSIZ UNPxSIZ "o:__seq_reduce__", &args))
		goto err;
{
	if (args.start == 0 && args.end == (size_t)-1) {
		if (args.init)
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_init))(self, args.combine, args.init);
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce))(self, args.combine);
	}
	if (args.init)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_range_and_init))(self, args.combine, args.start, args.end, args.init);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reduce_with_range))(self, args.combine, args.start, args.end);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_min__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_min__", &args))
		goto err;
{
	DREF DeeObject *result;
	if (args.start == 0 && args.end == (size_t)-1) {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_key))(self, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min))(self);
	} else {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_range_and_key))(self, args.start, args.end, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_min_with_range))(self, args.start, args.end);
	}
	return result;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_max__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_max__", &args))
		goto err;
{
	DREF DeeObject *result;
	if (args.start == 0 && args.end == (size_t)-1) {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_key))(self, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max))(self);
	} else {
		result = !DeeNone_Check(args.key)
		         ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_range_and_key))(self, args.start, args.end, args.key)
		         : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_max_with_range))(self, args.start, args.end);
	}
	return result;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sum__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":__seq_sum__", &args))
		goto err;
{
	DREF DeeObject *result;
	if (args.start == 0 && args.end == (size_t)-1) {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sum))(self);
	} else {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sum_with_range))(self, args.start, args.end);
	}
	return result;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_count__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_count__", &args))
		goto err;
{
	size_t result;
	if (args.start == 0 && args.end == (size_t)-1) {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count))(self, args.item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_key))(self, args.item, args.key);
		}
	} else {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_range))(self, args.item, args.start, args.end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_count_with_range_and_key))(self, args.item, args.start, args.end, args.key);
		}
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_contains__", &args))
		goto err;
{
	int result;
	if (args.start == 0 && args.end == (size_t)-1) {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains))(self, args.item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_key))(self, args.item, args.key);
		}
	} else {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_range))(self, args.item, args.start, args.end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_contains_with_range_and_key))(self, args.item, args.start, args.end, args.key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_locate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *match;
		size_t start;
		size_t end;
		DeeObject *def;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.def = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__match_start_end_def, "o|" UNPuSIZ UNPxSIZ "o:__seq_locate__", &args))
		goto err;
{
	if (args.start == 0 && args.end == (size_t)-1)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_locate))(self, args.match, args.def);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_locate_with_range))(self, args.match, args.start, args.end, args.def);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rlocate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *match;
		size_t start;
		size_t end;
		DeeObject *def;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.def = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__match_start_end_def, "o|" UNPuSIZ UNPxSIZ "o:__seq_rlocate__", &args))
		goto err;
{
	if (args.start == 0 && args.end == (size_t)-1)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rlocate))(self, args.match, args.def);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rlocate_with_range))(self, args.match, args.start, args.end, args.def);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_startswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_startswith__", &args))
		goto err;
{
	int result;
	if (args.start == 0 && args.end == (size_t)-1) {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith))(self, args.item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_key))(self, args.item, args.key);
		}
	} else {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_range))(self, args.item, args.start, args.end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_startswith_with_range_and_key))(self, args.item, args.start, args.end, args.key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_endswith__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_endswith__", &args))
		goto err;
{
	int result;
	if (args.start == 0 && args.end == (size_t)-1) {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith))(self, args.item);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_key))(self, args.item, args.key);
		}
	} else {
		if (DeeNone_Check(args.key)) {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_range))(self, args.item, args.start, args.end);
		} else {
			result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_endswith_with_range_and_key))(self, args.item, args.start, args.end, args.key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_find__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_find__", &args))
		goto err;
{
	size_t result = !DeeNone_Check(args.key)
	                ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find_with_key))(self, args.item, args.start, args.end, args.key)
	                : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_find))(self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return DeeInt_NewMinusOne();
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rfind__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_rfind__", &args))
		goto err;
{
	size_t result = !DeeNone_Check(args.key)
	                ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind_with_key))(self, args.item, args.start, args.end, args.key)
	                : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rfind))(self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return DeeInt_NewMinusOne();
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_erase__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t index;
		size_t count;
	} args;
	args.count = 1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__index_count, UNPuSIZ "|" UNPuSIZ ":__seq_erase__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_erase))(self, args.index, args.count))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_insert__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t index;
		DeeObject *item;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__index_item, UNPuSIZ "o:__seq_insert__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insert))(self, args.index, args.item))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_insertall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t index;
		DeeObject *items;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__index_items, UNPuSIZ "o:__seq_insertall__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_insertall))(self, args.index, args.items))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_pushfront__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *item;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_pushfront__", &args.item);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pushfront))(self, args.item))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_append__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *item;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_append__", &args.item);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_append))(self, args.item))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_extend__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *items;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__seq_extend__", &args.items);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_extend))(self, args.items))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_xchitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t index;
		DeeObject *item;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__index_item, UNPuSIZ "o:__seq_xchitem__", &args))
		goto err;
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_xchitem_index))(self, args.index, args.item);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_clear__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__seq_clear__");
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_clear))(self))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		Dee_ssize_t index;
	} args;
	args.index = -1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__index, "|" UNPdSIZ ":__seq_pop__", &args))
		goto err;
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_pop))(self, args.index);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_remove__", &args))
		goto err;
{
	int result = !DeeNone_Check(args.key)
	             ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove_with_key))(self, args.item, args.start, args.end, args.key)
	             : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_remove))(self, args.item, args.start, args.end);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_rremove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_rremove__", &args))
		goto err;
{
	int result = !DeeNone_Check(args.key)
	             ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rremove_with_key))(self, args.item, args.start, args.end, args.key)
	             : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_rremove))(self, args.item, args.start, args.end);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_removeall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		size_t _max;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args._max = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_max_key, "o|" UNPuSIZ UNPxSIZ UNPxSIZ "o:__seq_removeall__", &args))
		goto err;
{
	size_t result = !DeeNone_Check(args.key)
	                ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall_with_key))(self, args.item, args.start, args.end, args._max, args.key)
	                : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeall))(self, args.item, args.start, args.end, args._max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_removeif__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *should;
		size_t start;
		size_t end;
		size_t _max;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args._max = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__should_start_end_max, "o|" UNPuSIZ UNPxSIZ UNPxSIZ ":__seq_removeif__", &args))
		goto err;
{
	size_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_removeif))(self, args.should, args.start, args.end, args._max);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_resize__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t size;
		DeeObject *filler;
	} args;
	args.filler = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__size_filler, UNPuSIZ "|o:__seq_resize__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_resize))(self, args.size, args.filler))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_fill__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
		DeeObject *filler;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.filler = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_filler, "|" UNPuSIZ UNPxSIZ "o:__seq_fill__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_fill))(self, args.start, args.end, args.filler))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reverse__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":__seq_reverse__", &args))
		goto err;
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reverse))(self, args.start, args.end))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_reversed__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		size_t start;
		size_t end;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end, "|" UNPuSIZ UNPxSIZ ":__seq_reversed__", &args))
		goto err;
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_reversed))(self, args.start, args.end);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sort__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int error;
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1) {
		result = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort_with_key))(self, argv[0]);
		goto check_result;
	}
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort))(self, 0, (size_t)-1);
			goto check_error;
		case 1:
			error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort_with_key))(self, 0, (size_t)-1, argv[0]);
			goto check_error;
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2) {
				error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort))(self, start, end);
			} else {
				error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort_with_key))(self, start, end, argv[2]);
			}
			goto check_error;
		default:
			err_invalid_argc("__seq_sort__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_sort__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCsortable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_sort__", &args))
		goto err;
/*[[[end]]]*/
	if (DeeNone_Check(args.key)) {
		error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort))(self, args.start, args.end);
	} else {
		error = (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sort_with_key))(self, args.start, args.end, args.key);
	}
check_error:
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_sorted__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
#ifdef __OPTIMIZE_SIZE__
	if (!kw && argc == 1)
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, argv[0]);
#else /* __OPTIMIZE_SIZE__ */
	if (!kw) {
		size_t start, end;
		switch (argc) {
		case 0:
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted))(self, 0, (size_t)-1);
		case 1:
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, 0, (size_t)-1, argv[0]);
		case 2:
		case 3:
			if (DeeObject_AsSize(argv[0], &start))
				goto err;
			if (DeeObject_AsSizeM1(argv[1], &end))
				goto err;
			if (argc == 2)
				return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted))(self, start, end);
			return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, start, end, argv[2]);
		default:
			err_invalid_argc("__seq_sorted__", argc, 0, 3);
			goto err;
		}
		__builtin_unreachable();
	}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("__seq_sorted__", params: "
	size_t start = 0, size_t end = (size_t)-1, key:?DCsortedable=!N
");]]]*/
	struct {
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__start_end_key, "|" UNPuSIZ UNPxSIZ "o:__seq_sorted__", &args))
		goto err;
/*[[[end]]]*/
	if (DeeNone_Check(args.key))
		return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted))(self, args.start, args.end);
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_sorted_with_key))(self, args.start, args.end, args.key);
err:
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bfind__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_bfind__", &args))
		goto err;
{
	size_t result = !DeeNone_Check(args.key)
	                ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bfind_with_key))(self, args.item, args.start, args.end, args.key)
	                : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bfind))(self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_bposition__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_bposition__", &args))
		goto err;
{
	size_t result = !DeeNone_Check(args.key)
	                ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bposition_with_key))(self, args.item, args.start, args.end, args.key)
	                : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_bposition))(self, args.item, args.start, args.end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___seq_brange__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *item;
		size_t start;
		size_t end;
		DeeObject *key;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.key = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__item_start_end_key, "o|" UNPuSIZ UNPxSIZ "o:__seq_brange__", &args))
		goto err;
{
	size_t result_range[2];
	if (!DeeNone_Check(args.key)
	    ? (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_brange_with_key))(self, args.item, args.start, args.end, args.key, result_range)
	    : (*DeeType_RequireMethodHint(Dee_TYPE(self), seq_brange))(self, args.item, args.start, args.end, result_range))
		goto err;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, result_range[0], result_range[1]);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__set_iter__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_iter))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__set_size__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sizeob))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__set_hash__");
{
	Dee_hash_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_hash))(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_compare_eq__", &args.rhs);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_compare_eq))(self, args.rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_eq__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_eq))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_ne__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ne))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_lo__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_lo))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_le__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_le))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_gr__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_gr))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_ge__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_ge))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inv__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__set_inv__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inv))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_add__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_add))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_sub__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_sub))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_and__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_and))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_xor__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_xor))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_inplace_add__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_add))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_inplace_sub__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_sub))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_inplace_and__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_and))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_inplace_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_inplace_xor__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_operator_inplace_xor))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_unify__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_unify__", &args.key);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), set_unify))(self, args.key);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_insert__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_insert__", &args.key);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_insert))(self, args.key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_insertall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_insertall__", &args.keys);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_insertall))(self, args.keys))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_remove__", &args.key);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), set_remove))(self, args.key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_removeall__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__set_removeall__", &args.keys);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), set_removeall))(self, args.keys))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___set_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *def;
	} args;
	args.def = NULL;
	args.def = NULL;
	_DeeArg_Unpack0Or1(err, argc, argv, "__set_pop__", &args.def);
{
	return args.def ? (*DeeType_RequireMethodHint(Dee_TYPE(self), set_pop_with_default))(self, args.def)
	           : (*DeeType_RequireMethodHint(Dee_TYPE(self), set_pop))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_iter__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__map_iter__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_iter))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_size__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__map_size__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sizeob))(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_hash__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__map_hash__");
{
	Dee_hash_t result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_hash))(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_getitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_getitem__", &args.key);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_getitem))(self, args.key);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_delitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_delitem__", &args.key);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_delitem))(self, args.key))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setitem__", &args.key, &args.value);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_setitem))(self, args.key, args.value))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_contains__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_contains__", &args.key);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_contains))(self, args.key);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_enumerate__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	if (argc == 3) {
		data.sed_cb = argv[0];
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate_range))(self, &seq_enumerate_cb, &data, argv[1], argv[2]);
	} else if (argc == 1) {
		data.sed_cb = argv[0];
		foreach_status = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_enumerate))(self, &seq_enumerate_cb, &data);
	} else {
		DeeArg_BadArgcEx("__map_enumerate__", argc, 1, 3);
		goto err;
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
DeeMA___map_enumerate_items__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	if (argc == 2)
		return DeeObject_InvokeMethodHint(map_makeenumeration_with_range, self, argv[0], argv[1]);
	if (argc == 0)
		return DeeObject_InvokeMethodHint(map_makeenumeration, self);
	DeeArg_BadArgcEx("__map_enumerate_items__", argc, 0, 2);
	return NULL;
}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_compare_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_compare_eq__", &args.rhs);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_compare_eq))(self, args.rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_eq__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_eq__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_eq))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ne__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_ne__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ne))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_lo__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_lo__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_lo))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_le__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_le__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_le))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_gr__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_gr__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_gr))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_ge__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_ge__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_ge))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_add__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_add))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_sub__", &args.keys);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_sub))(self, args.keys);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_and__", &args.keys);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_and))(self, args.keys);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_xor__", &args.rhs);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_xor))(self, args.rhs);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_add__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *items;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_inplace_add__", &args.items);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_add))((DeeObject **)&self, args.items))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_sub__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_inplace_sub__", &args.keys);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_sub))((DeeObject **)&self, args.keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_and__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_inplace_and__", &args.keys);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_and))((DeeObject **)&self, args.keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_inplace_xor__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *rhs;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_inplace_xor__", &args.rhs);
{
	Dee_Incref(self);
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_operator_inplace_xor))((DeeObject **)&self, args.rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setold__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setold__", &args.key, &args.value);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold))(self, args.key, args.value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setold_ex__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setold_ex__", &args.key, &args.value);
{
	PRIVATE DEFINE_TUPLE(setold_failed_result, 2, { Dee_False, Dee_None });
	DREF DeeTupleObject *result;
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setold_ex))(self, args.key, args.value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE) {
		Dee_Incref(&setold_failed_result);
		return (DeeObject *)&setold_failed_result;
	}
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	result->t_elem[0] = DeeBool_NewTrue();
	result->t_elem[1] = old_value; /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setnew__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setnew__", &args.key, &args.value);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew))(self, args.key, args.value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setnew_ex__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setnew_ex__", &args.key, &args.value);
{
	PRIVATE DEFINE_TUPLE(setnew_success_result, 2, { Dee_True, Dee_None });
	DREF DeeTupleObject *result;
	DREF DeeObject *old_value = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setnew_ex))(self, args.key, args.value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE) {
		Dee_Incref(&setnew_success_result);
		return (DeeObject *)&setnew_success_result;
	}
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	result->t_elem[0] = DeeBool_NewFalse();
	result->t_elem[1] = old_value; /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_setdefault__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *value;
	} args;
	_DeeArg_Unpack2(err, argc, argv, "__map_setdefault__", &args.key, &args.value);
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_setdefault))(self, args.key, args.value);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_update__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *items;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_update__", &args.items);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_update))(self, args.items))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_remove__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_remove__", &args.key);
{
	int result = (*DeeType_RequireMethodHint(Dee_TYPE(self), map_remove))(self, args.key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_removekeys__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *keys;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "__map_removekeys__", &args.keys);
{
	if unlikely((*DeeType_RequireMethodHint(Dee_TYPE(self), map_removekeys))(self, args.keys))
		goto err;
	return_none;
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_pop__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *key;
		DeeObject *def;
	} args;
	args.def = NULL;
	args.def = NULL;
	_DeeArg_Unpack1Or2(err, argc, argv, "__map_pop__", &args.key, &args.def);
{
	return args.def ? (*DeeType_RequireMethodHint(Dee_TYPE(self), map_pop_with_default))(self, args.key, args.def)
	           : (*DeeType_RequireMethodHint(Dee_TYPE(self), map_pop))(self, args.key);
err:
	return NULL;
}}

PUBLIC NONNULL((1)) DREF DeeObject *DCALL
DeeMA___map_popitem__(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "__map_popitem__");
{
	return (*DeeType_RequireMethodHint(Dee_TYPE(self), map_popitem))(self);
err:
	return NULL;
}}
/*[[[end]]]*/
/* clang-format on */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_WRAPPERS_C */
