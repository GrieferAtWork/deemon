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

/************************************************************************/
/* deemon.Sequence.find()                                               */
/************************************************************************/
[[kw, alias(Sequence.find)]]
__seq_find__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dint {
	size_t result = !DeeNone_Check(key)
	                ? CALL_DEPENDENCY(seq_find_with_key, self, item, start, end, key)
	                : CALL_DEPENDENCY(seq_find, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return DeeInt_NewMinusOne();
	return DeeInt_NewSize(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_find_foreach_cb =
#ifndef DEFINED_default_seq_find_foreach_cb
#define DEFINED_default_seq_find_foreach_cb
union default_seq_find_foreach_data {
	DeeObject *dsff_elem;  /* [in][1..1] Element to search for */
	size_t     dsff_index; /* [out] Located index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_find_foreach_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	union default_seq_find_foreach_data *data;
	data = (union default_seq_find_foreach_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->dsff_elem, value);
	if (cmp == 0) {
		/* Found the index! */
		data->dsff_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_find_foreach_cb */
)]



/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
[[wunused]] size_t
__seq_find__.seq_find([[nonnull]] DeeObject *self,
                      [[nonnull]] DeeObject *item,
                      size_t start, size_t end)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_find__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = (size_t)-1}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_find_foreach_cb)]] {
	Dee_ssize_t status;
	union default_seq_find_foreach_data data;
	data.dsff_elem = item;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &default_seq_find_foreach_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.dsff_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.dsff_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

%[define(DEFINE_default_seq_find_with_key_foreach_cb =
#ifndef DEFINED_default_seq_find_with_key_foreach_cb
#define DEFINED_default_seq_find_with_key_foreach_cb
struct default_seq_find_with_key_foreach_data {
	union default_seq_find_foreach_data dsfwkf_base; /* Base find data */
	DeeObject                          *dsfwkf_key;  /* Find element key */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_find_with_key_foreach_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_find_with_key_foreach_data *data;
	data = (struct default_seq_find_with_key_foreach_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareKeyEq(data->dsfwkf_base.dsff_elem, value, data->dsfwkf_key);
	if (cmp == 0) {
		/* Found the index! */
		data->dsfwkf_base.dsff_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_find_with_key_foreach_cb */
)]


/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
[[wunused]] size_t
__seq_find__.seq_find_with_key([[nonnull]] DeeObject *self,
                               [[nonnull]] DeeObject *item,
                               size_t start, size_t end,
                               [[nonnull]] DeeObject *key)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_find__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = (size_t)-1}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_find_with_key_foreach_cb)]] {
	Dee_ssize_t status;
	struct default_seq_find_with_key_foreach_data data;
	data.dsfwkf_base.dsff_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.dsfwkf_base.dsff_elem)
		goto err;
	data.dsfwkf_key = key;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &default_seq_find_with_key_foreach_cb, &data, start, end);
	Dee_Decref(data.dsfwkf_base.dsff_elem);
	if likely(status == -2) {
		if unlikely(data.dsfwkf_base.dsff_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.dsfwkf_base.dsff_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	int temp;
	Dee_ssize_t result_index;
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSSize(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return (size_t)result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

seq_find = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_find_with_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};
