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
/* deemon.Sequence.rfind()                                              */
/************************************************************************/
[[kw, alias(Sequence.rfind -> "seq_rfind")]]
__seq_rfind__(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dint {
	DeeObject *item, *key = Dee_None;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_rfind__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? CALL_DEPENDENCY(seq_rfind_with_key, self, item, start, end, key)
	         : CALL_DEPENDENCY(seq_rfind, self, item, start, end);
	if unlikely(result == (size_t)Dee_COMPARE_ERR)
		goto err;
	if unlikely(result == (size_t)-1)
		return_reference_(DeeInt_MinusOne);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

%[define(DEFINE_seq_rfind_cb =
#ifndef DEFINED_seq_rfind_cb
#define DEFINED_seq_rfind_cb
struct seq_rfind_data {
	DeeObject *gsrfd_elem;   /* [1..1] The element to search for */
	size_t     gsrfd_result; /* The last-matched index. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_rfind_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct seq_rfind_data *data;
	data = (struct seq_rfind_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfd_elem, value);
	if (cmp == 0)
		data->gsrfd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_rfind_cb */
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
%{$with__seq_enumerate_index_reverse = [[prefix(DEFINE_seq_find_cb)]] {
	Dee_ssize_t status;
	union seq_find_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.gsfd_elem = item;
	renum = REQUIRE_DEPENDENCY(seq_enumerate_index_reverse);
	ASSERT(renum);
	status = (*renum)(self, &seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_rfind_cb)]] {
	Dee_ssize_t status;
	struct seq_rfind_data data;
	data.gsrfd_elem   = item;
	data.gsrfd_result = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_rfind_cb, &data, start, end);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfd_result;
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

%[define(DEFINE_seq_rfind_with_key_cb =
#ifndef DEFINED_seq_rfind_with_key_cb
#define DEFINED_seq_rfind_with_key_cb
struct seq_rfind_with_key_data {
	DeeObject *gsrfwkd_kelem;   /* [1..1] The element to search for */
	size_t     gsrfwkd_result; /* The last-matched index. */
	DeeObject *gsrfwkd_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_rfind_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct seq_rfind_with_key_data *data;
	data = (struct seq_rfind_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfwkd_kelem, value);
	if (cmp == 0)
		data->gsrfwkd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_rfind_with_key_cb */
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
%{$with__seq_enumerate_index_reverse = [[prefix(DEFINE_seq_find_with_key_cb)]] {
	Dee_ssize_t status;
	struct seq_find_with_key_data data;
	DeeMH_seq_enumerate_index_reverse_t renum;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	renum = REQUIRE_DEPENDENCY(seq_enumerate_index_reverse);
	ASSERT(renum);
	status = (*renum)(self, &seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_rfind_with_key_cb)]] {
	Dee_ssize_t status;
	struct seq_rfind_with_key_data data;
	data.gsrfwkd_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gsrfwkd_kelem)
		goto err;
	data.gsrfwkd_result = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_rfind_with_key_cb, &data, start, end);
	Dee_Decref(data.gsrfwkd_kelem);
	ASSERT(status == 0 || status == -1);
	if unlikely(status == -1)
		goto err;
	if unlikely(data.gsrfwkd_result == (size_t)Dee_COMPARE_ERR)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return data.gsrfwkd_result;
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
