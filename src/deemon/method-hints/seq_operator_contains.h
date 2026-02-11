/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/************************************************************************/
/* deemon.Sequence.contains()                                           */
/* deemon.Sequence.operator contains()                                  */
/************************************************************************/

[[kw, alias(Sequence.contains)]]
__seq_contains__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dbool {
	int result;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_contains, self, item);
		} else {
			result = CALL_DEPENDENCY(seq_contains_with_key, self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_contains_with_range, self, item, start, end);
		} else {
			result = CALL_DEPENDENCY(seq_contains_with_range_and_key, self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}





%[define(DEFINE_default_contains_with_foreach_cb =
#ifndef DEFINED_default_contains_with_foreach_cb
#define DEFINED_default_contains_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_contains_with_foreach_cb(void *arg, DeeObject *elem) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, elem);
	if (Dee_COMPARE_ISERR(temp))
		return -1;
	if (Dee_COMPARE_ISEQ(temp))
		return -2;
	return 0;
}
#endif /* !DEFINED_default_contains_with_foreach_cb */
)]



/* @return: 0 : Not contained
 * @return: 1 : Is contained
 * @return: -1: Error */
[[wunused]] int
__seq_contains__.seq_contains([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$empty = 0}
/* Can't use "using" here since the impl would only be used if the user
 * defines "__seq_contains__", and we want to use it whenever possible! */
%{$with__seq_operator_contains = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_contains, self, item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}}
%{$with__map_operator_trygetitem = {
	int result;
	DREF DeeObject *real_value;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(item, 2, key_and_value))
		goto err_trycatch;
	real_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(!ITER_ISOK(real_value)) {
		Dee_Decref(key_and_value[1]);
		if (real_value == ITER_DONE)
			return 0;
		goto err;
	}
	result = DeeObject_TryCompareEq(key_and_value[1], real_value);
	Dee_Decref(real_value);
	Dee_Decref(key_and_value[1]);
	if (Dee_COMPARE_ISERR(result))
		goto err;
	return Dee_COMPARE_ISEQ(result) ? 1 : 0;
err_trycatch:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return 0;
err:
	return -1;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_contains_with_foreach_cb)]] {
	Dee_ssize_t status;
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_contains_with_foreach_cb, item);
	if unlikely(status == -1)
		goto err;
	return status /*== -2*/ ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_find = {
	return default__seq_contains_with_range__with__seq_find(self, item, 0, (size_t)-1);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_contains = {
	DeeMH_seq_find_t seq_find;
	/* Check if the underlying operator is defined. */
	if (REQUIRE_NODEFAULT(seq_operator_contains))
		return &$with__seq_operator_contains;
	if (SEQ_CLASS == Dee_SEQCLASS_MAP) {
		DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
		map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
		if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
			return &$empty;
		if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
			return &$with__seq_operator_foreach;
		if (map_operator_trygetitem)
			return &$with__map_operator_trygetitem;
	}
	seq_find = REQUIRE(seq_find);
	if (seq_find == &default__seq_find__empty)
		return &$empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &$with__seq_operator_foreach;
	if (seq_find)
		return &$with__seq_find;
};






%[define(DEFINE_seq_contains_with_key_foreach_cb =
DEFINE_seq_count_with_key_foreach_cb
#ifndef DEFINED_seq_contains_with_key_foreach_cb
#define DEFINED_seq_contains_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_contains_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct seq_count_with_key_data *data;
	data = (struct seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if (Dee_COMPARE_ISERR(temp))
		return -1;
	return Dee_COMPARE_ISEQ(temp) ? -2 : 0;
}
#endif /* !DEFINED_seq_contains_with_key_foreach_cb */
)]




/* @return: 0 : Not contained
 * @return: 1 : Is contained
 * @return: -1: Error */
[[wunused]] int
__seq_contains__.seq_contains_with_key([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *item,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_contains_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
#ifdef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	data.gscwk_kelem = item;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_contains_with_key_foreach_cb, &data);
#else /* CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_contains_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
}}
%{$with__seq_find_with_key = {
	return default__seq_contains_with_range_and_key__with__seq_find_with_key(self, item, 0, (size_t)-1, key);
}} {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = Dee_AsObject(&Dee_int_SIZE_MAX);
	args[3] = key;
	result = LOCAL_CALLATTR(self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_contains_with_key = {
	DeeMH_seq_find_with_key_t seq_find_with_key = REQUIRE(seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &$empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &$with__seq_operator_foreach;
	if (seq_find_with_key)
		return &$with__seq_find_with_key;
};







%[define(DEFINE_seq_contains_enumerate_cb =
DEFINE_default_contains_with_foreach_cb
#ifndef DEFINED_seq_contains_enumerate_cb
#define DEFINED_seq_contains_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_contains_with_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_contains_enumerate_cb */
)]



/* @return: 0 : Not contained
 * @return: 1 : Is contained
 * @return: -1: Error */
[[wunused]] int
__seq_contains__.seq_contains_with_range([[nonnull]] DeeObject *self,
                                         [[nonnull]] DeeObject *item,
                                         size_t start, size_t end)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_contains_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_contains_enumerate_cb, item, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}}
%{$with__seq_find = {
	size_t match = CALL_DEPENDENCY(seq_find, self, item, start, end);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_contains_with_range = {
	DeeMH_seq_find_t seq_find = REQUIRE(seq_find);
	if (seq_find == &default__seq_find__empty)
		return &$empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_find)
		return &$with__seq_find;
};






%[define(DEFINE_seq_contains_with_key_enumerate_cb =
DEFINE_seq_contains_with_key_foreach_cb
#ifndef DEFINED_seq_contains_with_key_enumerate_cb
#define DEFINED_seq_contains_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_contains_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_contains_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_contains_with_key_enumerate_cb */
)]



/* @return: 0 : Not contained
 * @return: 1 : Is contained
 * @return: -1: Error */
[[wunused]] int
__seq_contains__.seq_contains_with_range_and_key([[nonnull]] DeeObject *self,
                                                 [[nonnull]] DeeObject *item,
                                                 size_t start, size_t end,
                                                 [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_contains_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
#ifdef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	data.gscwk_kelem = item;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_contains_with_key_enumerate_cb, &data, start, end);
#else /* CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_contains_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
#ifndef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
err:
	return -1;
#endif /* !CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
}}
%{$with__seq_find_with_key = {
	size_t match = CALL_DEPENDENCY(seq_find_with_key, self, item, start, end, key);
	if unlikely(match == (size_t)Dee_COMPARE_ERR)
		goto err;
	return match != (size_t)-1 ? 1 : 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_contains_with_range_and_key = {
	DeeMH_seq_find_with_key_t seq_find_with_key = REQUIRE(seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &$empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_find_with_key)
		return &$with__seq_find_with_key;
};









/* "operator contains(item)" -- implement bi-directionally with "Sequence.contains(item)" */
[[operator(Sequence: tp_seq->tp_contains)]]
[[wunused]] DREF DeeObject *
__seq_contains__.seq_operator_contains([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *item)
%{unsupported(auto("operator contains"))}
%{$none = return_none}
%{$empty = return_false}
%{$with__seq_contains = {
	int result = CALL_DEPENDENCY(seq_contains, self, item);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}} = $with__seq_contains;

seq_operator_contains = {
	DeeMH_seq_contains_t seq_contains = REQUIRE(seq_contains);
	if (seq_contains == &default__seq_contains__empty)
		return &$empty;
	if (seq_contains)
		return &$with__seq_contains;
};
