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
/* deemon.Sequence.endswith()                                           */
/************************************************************************/

[[kw, alias(Sequence.endswith)]]
__seq_endswith__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dbool {
	int result;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_endswith, self, item);
		} else {
			result = CALL_DEPENDENCY(seq_endswith_with_key, self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_endswith_with_range, self, item, start, end);
		} else {
			result = CALL_DEPENDENCY(seq_endswith_with_range_and_key, self, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}





/* @return: 0 : Does not start with
 * @return: 1 : Does start with
 * @return: -1: Error */
[[wunused]] int
__seq_endswith__.seq_endswith([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_trygetlast = {
	int result;
	DREF DeeObject *last = CALL_DEPENDENCY(seq_trygetlast, self);
	if (last == ITER_DONE)
		return 0;
	if unlikely(!last)
		goto err;
	result = DeeObject_TryCompareEq(item, last);
	Dee_Decref(last);
	if (Dee_COMPARE_ISERR(result))
		goto err;
	return Dee_COMPARE_ISEQ(result) ? 1 : 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_endswith = {
	DeeMH_seq_trygetlast_t seq_trygetlast;
	seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast)
		return &$with__seq_trygetlast;
};






/* @return: 0 : Does not start with
 * @return: 1 : Does start with
 * @return: -1: Error */
[[wunused]] int
__seq_endswith__.seq_endswith_with_key([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *item,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_trygetlast = {
	int result;
	DREF DeeObject *last;
	last = CALL_DEPENDENCY(seq_trygetlast, self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_last;
	result = DeeObject_TryCompareKeyEq(item, last, key);
	Dee_Decref(item);
	Dee_Decref(last);
	if (Dee_COMPARE_ISERR(result))
		goto err;
	return Dee_COMPARE_ISEQ(result) ? 1 : 0;
err_last:
	Dee_Decref(last);
err:
	return -1;
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

seq_endswith_with_key = {
	DeeMH_seq_trygetlast_t seq_trygetlast;
	seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast)
		return &$with__seq_trygetlast;
};







/* @return: 0 : Does not start with
 * @return: 1 : Does start with
 * @return: -1: Error */
[[wunused]] int
__seq_endswith__.seq_endswith_with_range([[nonnull]] DeeObject *self,
                                         [[nonnull]] DeeObject *item,
                                         size_t start, size_t end)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_operator_size__and__operator_trygetitem_index = {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize;
	if (start >= end)
		return 0;
	selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize) {
		end = selfsize;
		if (start >= end)
			return 0;
	}
	selfitem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if (Dee_COMPARE_ISERR(result))
		goto err;
	return Dee_COMPARE_ISEQ(result) ? 1 : 0;
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

seq_endswith_with_range = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__operator_trygetitem_index;
	}
};






/* @return: 0 : Does not start with
 * @return: 1 : Does start with
 * @return: -1: Error */
[[wunused]] int
__seq_endswith__.seq_endswith_with_range_and_key([[nonnull]] DeeObject *self,
                                                 [[nonnull]] DeeObject *item,
                                                 size_t start, size_t end,
                                                 [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_operator_size__and__operator_trygetitem_index = {
	int result;
	DREF DeeObject *selfitem;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	item = DeeObject_Call(key, 1, &item);
	if unlikely(!item)
		goto err_selfitem;
	result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(item);
	Dee_Decref(selfitem);
	if (Dee_COMPARE_ISERR(result))
		goto err;
	return Dee_COMPARE_ISEQ(result) ? 1 : 0;
err_selfitem:
	Dee_Decref(selfitem);
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

seq_endswith_with_range_and_key = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__operator_trygetitem_index;
	}
};
