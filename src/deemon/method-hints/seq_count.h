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
/* deemon.Sequence.count()                                                */
/************************************************************************/
[[kw, alias(Sequence.count)]]
__seq_count__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dint {
	size_t result;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_count, self, item);
		} else {
			result = CALL_DEPENDENCY(seq_count_with_key, self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_count_with_range, self, item, start, end);
		} else {
			result = CALL_DEPENDENCY(seq_count_with_range_and_key, self, item, start, end, key);
		}
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


%[define(DEFINE_seq_count_foreach_cb =
#ifndef DEFINED_seq_count_foreach_cb
#define DEFINED_seq_count_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_foreach_cb(void *arg, DeeObject *item) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, item);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}
#endif /* !DEFINED_seq_count_foreach_cb */
)]


/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_count__.seq_count([[nonnull]] DeeObject *self,
                        [[nonnull]] DeeObject *item)
%{$empty = 0}
%{unsupported(auto)}
/* Actually uses seq_operator_contains, but only linked in set-like
 * types. This also optimizes the case when "self" is a Mapping, where
 * seq_operator_contains is implemented to use map_operator_trygetitem */
%{$with__set_operator_contains = {
	DREF DeeObject *contains = CALL_DEPENDENCY(seq_operator_contains, self, item);
	if unlikely(!contains)
		goto err;
	return (size_t)DeeObject_BoolInherited(contains);
err:
	return (size_t)-1;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_count_foreach_cb)]] {
	return (size_t)CALL_DEPENDENCY(seq_operator_foreach, self, &seq_count_foreach_cb, item);
}}
%{$with__seq_find = {
	return default__seq_count_with_range__with__seq_find(self, item, 0, (size_t)-1);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}




%[define(DEFINE_seq_count_with_key_foreach_cb =
#ifndef DEFINED_seq_count_with_key_foreach_cb
#define DEFINED_seq_count_with_key_foreach_cb
struct seq_count_with_key_data {
	DeeObject *gscwk_key;   /* [1..1] Key predicate */
	DeeObject *gscwk_kelem; /* [1..1] Keyed search element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_count_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	struct seq_count_with_key_data *data;
	data = (struct seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, item, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}
#endif /* !DEFINED_seq_count_with_key_foreach_cb */
)]



/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_count__.seq_count_with_key([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *item,
                                 [[nonnull]] DeeObject *key)
%{$empty = 0}
%{unsupported({
	return (size_t)err_seq_unsupportedf(self, "count(%r, key: %r)", item, key);
})}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_count_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_count_with_key_foreach_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}}
%{$with__seq_find_with_key = {
	return default__seq_count_with_range_and_key__with__seq_find_with_key(self, item, 0, (size_t)-1, key);
}} {
	DREF DeeObject *result;
	DeeObject *args[4];
	args[0] = item;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = key;
	result = LOCAL_CALLATTR(self, 4, args);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}




%[define(DEFINE_seq_count_enumerate_cb =
DEFINE_seq_count_foreach_cb
#ifndef DEFINED_seq_count_enumerate_cb
#define DEFINED_seq_count_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_count_enumerate_cb */
)]



/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_count__.seq_count_with_range([[nonnull]] DeeObject *self,
                                   [[nonnull]] DeeObject *item,
                                   size_t start, size_t end)
%{$empty = 0}
%{unsupported(auto)}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_count_enumerate_cb)]] {
	return (size_t)CALL_DEPENDENCY(seq_enumerate_index, self, &seq_count_enumerate_cb, item, start, end);
}}
%{$with__seq_find = {
	PRELOAD_DEPENDENCY(seq_find)
	size_t result = 0;
	while (start < end) {
		size_t match = CALL_DEPENDENCY(seq_find, self, item, start, end);
		if unlikely(match == (size_t)Dee_COMPARE_ERR)
			goto err;
		if (match == (size_t)-1)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		start = match + 1;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}




%[define(DEFINE_seq_count_with_key_enumerate_cb =
DEFINE_seq_count_with_key_foreach_cb
#ifndef DEFINED_seq_count_with_key_enumerate_cb
#define DEFINED_seq_count_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_count_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_count_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_count_with_key_enumerate_cb */
)]



/* @return: (size_t)-1: Error */
[[wunused]] size_t
__seq_count__.seq_count_with_range_and_key([[nonnull]] DeeObject *self,
                                           [[nonnull]] DeeObject *item,
                                           size_t start, size_t end,
                                           [[nonnull]] DeeObject *key)
%{$empty = 0}
%{unsupported(auto)}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_count_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &item);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_count_with_key_enumerate_cb, &data, start, end);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}}
%{$with__seq_find_with_key = {
	PRELOAD_DEPENDENCY(seq_find_with_key)
	size_t result = 0;
	while (start < end) {
		size_t match = CALL_DEPENDENCY(seq_find_with_key, self, item, start, end, key);
		if unlikely(match == (size_t)Dee_COMPARE_ERR)
			goto err;
		if (match == (size_t)-1)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		start = match + 1;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_AsDirectSizeInherited(result);
err:
	return (size_t)-1;
}





seq_count = {
	DeeMH_seq_find_t seq_find;
	if (Dee_SEQCLASS_ISSETORMAP(SEQ_CLASS)) {
		DeeMH_seq_operator_contains_t seq_operator_contains;
		seq_operator_contains = REQUIRE(seq_operator_contains);
		if (seq_operator_contains)
			return &$with__set_operator_contains;
	}
	seq_find = REQUIRE(seq_find);
	if (seq_find == &default__seq_find__empty)
		return &$empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &$with__seq_operator_foreach;
	if (seq_find)
		return &$with__seq_find;
};

seq_count_with_key = {
	DeeMH_seq_find_with_key_t seq_find_with_key;
	seq_find_with_key = REQUIRE(seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &$empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &$with__seq_operator_foreach;
	if (seq_find_with_key)
		return &$with__seq_find_with_key;
};

seq_count_with_range = {
	DeeMH_seq_find_t seq_find;
	seq_find = REQUIRE(seq_find);
	if (seq_find == &default__seq_find__empty)
		return &$empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_find)
		return &$with__seq_find;
};

seq_count_with_range_and_key = {
	DeeMH_seq_find_with_key_t seq_find_with_key;
	seq_find_with_key = REQUIRE(seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &$empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_find_with_key)
		return &$with__seq_find_with_key;
};

