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
/* deemon.Sequence.rfind()                                              */
/************************************************************************/
[[kw, alias(Sequence.rfind)]]
__seq_rfind__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dint {
	size_t result = !DeeNone_Check(key)
	                ? CALL_DEPENDENCY(seq_rfind_with_key, self, item, start, end, key)
	                : CALL_DEPENDENCY(seq_rfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return DeeInt_NewMinusOne();
	return DeeInt_NewSize(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_rfind_foreach_cb =
#ifndef DEFINED_default_seq_rfind_foreach_cb
#define DEFINED_default_seq_rfind_foreach_cb
struct default_seq_rfind_foreach_data {
	DeeObject *dsrf_elem;   /* [1..1] The element to search for */
	size_t     dsrf_result; /* The last-matched index. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_rfind_foreach_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_rfind_foreach_data *data;
	data = (struct default_seq_rfind_foreach_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->dsrf_elem, value);
	if (Dee_COMPARE_ISEQ_NO_ERR(cmp)) {
		data->dsrf_result = index;
	} else if (Dee_COMPARE_ISERR(cmp)) {
		goto err;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_rfind_foreach_cb */
)]



/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
[[wunused]] size_t
__seq_rfind__.seq_rfind([[nonnull]] DeeObject *self,
                        [[nonnull]] DeeObject *item,
                        size_t start, size_t end)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ")", item, start, end);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = "default__seq_find__empty"}
%{$with__seq_enumerate_index_reverse = [[prefix(DEFINE_default_seq_find_foreach_cb)]] {
	Dee_ssize_t status;
	union default_seq_find_foreach_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.dsff_elem = item;
	renum = REQUIRE_DEPENDENCY(seq_enumerate_index_reverse);
	ASSERT(renum);
	status = (*renum)(self, &default_seq_find_foreach_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.dsff_index == (size_t)Dee_COMPARE_ERR)
			DeeRT_ErrIntegerOverflowU(data.dsff_index, (size_t)Dee_COMPARE_ERR - 1);
		return data.dsff_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_rfind_foreach_cb)]] {
	Dee_ssize_t status;
	struct default_seq_rfind_foreach_data data;
	data.dsrf_elem   = item;
	data.dsrf_result = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &default_seq_rfind_foreach_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.dsrf_result == (size_t)Dee_COMPARE_ERR)
		DeeRT_ErrIntegerOverflowU(data.dsrf_result, (size_t)Dee_COMPARE_ERR - 1);
	return data.dsrf_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSizeM1(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		DeeRT_ErrIntegerOverflowU(result_index, (size_t)Dee_COMPARE_ERR - 1);
	return result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

%[define(DEFINE_default_seq_rfind_with_key_foreach_cb =
#ifndef DEFINED_default_seq_rfind_with_key_foreach_cb
#define DEFINED_default_seq_rfind_with_key_foreach_cb
struct default_seq_rfind_with_key_foreach_data {
	DeeObject *dsrfwk_kelem;   /* [1..1] The element to search for */
	size_t     dsrfwk_result; /* The last-matched index. */
	DeeObject *dsrfwk_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_rfind_with_key_foreach_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct default_seq_rfind_with_key_foreach_data *data;
	data = (struct default_seq_rfind_with_key_foreach_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->dsrfwk_kelem, value);
	if (Dee_COMPARE_ISEQ_NO_ERR(cmp)) {
		data->dsrfwk_result = index;
	} else if (Dee_COMPARE_ISERR(cmp)) {
		goto err;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_rfind_with_key_foreach_cb */
)]


/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
[[wunused]] size_t
__seq_rfind__.seq_rfind_with_key([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *item,
                                 size_t start, size_t end,
                                 [[nonnull]] DeeObject *key)
%{unsupported({
	err_seq_unsupportedf(self, "__seq_rfind__(%r, %" PRFuSIZ ", %" PRFuSIZ ", %r)", item, start, end, key);
	return (size_t)Dee_COMPARE_ERR;
})}
%{$empty = "default__seq_find_with_key__empty"}
%{$with__seq_enumerate_index_reverse = [[prefix(DEFINE_default_seq_find_with_key_foreach_cb)]] {
	Dee_ssize_t status;
	struct default_seq_find_with_key_foreach_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.dsfwkf_base.dsff_elem = item;
	data.dsfwkf_key = key;
	renum = REQUIRE_DEPENDENCY(seq_enumerate_index_reverse);
	ASSERT(renum);
	status = (*renum)(self, &default_seq_find_with_key_foreach_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.dsfwkf_base.dsff_index == (size_t)Dee_COMPARE_ERR)
			DeeRT_ErrIntegerOverflowU(data.dsfwkf_base.dsff_index, (size_t)Dee_COMPARE_ERR - 1);
		return data.dsfwkf_base.dsff_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_rfind_with_key_foreach_cb)]] {
	Dee_ssize_t status;
	struct default_seq_rfind_with_key_foreach_data data;
	data.dsrfwk_kelem = item;
	data.dsrfwk_key   = key;
	data.dsrfwk_result = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &default_seq_rfind_with_key_foreach_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.dsrfwk_result == (size_t)Dee_COMPARE_ERR)
		DeeRT_ErrIntegerOverflowU(data.dsrfwk_result, (size_t)Dee_COMPARE_ERR - 1);
	return data.dsrfwk_result;
err:
	return (size_t)Dee_COMPARE_ERR;
}} {
	int temp;
	size_t result_index;
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	temp = DeeObject_AsSizeM1(result, &result_index);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	if unlikely(result_index == (size_t)Dee_COMPARE_ERR)
		DeeRT_ErrIntegerOverflowU(result_index, (size_t)Dee_COMPARE_ERR - 1);
	return result_index;
err:
	return (size_t)Dee_COMPARE_ERR;
}

seq_rfind = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (REQUIRE(seq_enumerate_index_reverse))
		return &$with__seq_enumerate_index_reverse;
	seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_rfind_with_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (REQUIRE(seq_enumerate_index_reverse))
		return &$with__seq_enumerate_index_reverse;
	seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};
