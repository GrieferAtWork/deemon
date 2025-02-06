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
/* deemon.Sequence.endswith()                                           */
/************************************************************************/

[[kw, alias(Sequence.endswith -> "seq_endswith"), declNameAlias("explicit_seq_endswith")]]
__seq_endswith__(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_endswith__",
	                    &item, &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = DeeType_InvokeMethodHint(self, seq_endswith, item);
		} else {
			result = DeeType_InvokeMethodHint(self, seq_endswith_with_key, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = DeeType_InvokeMethodHint(self, seq_endswith_with_range, item, start, end);
		} else {
			result = DeeType_InvokeMethodHint(self, seq_endswith_with_range_and_key, item, start, end, key);
		}
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
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
	DREF DeeObject *last = DeeType_InvokeMethodHint0(self, seq_trygetlast);
	if (last == ITER_DONE)
		return 0;
	if unlikely(!last)
		goto err;
	result = DeeObject_TryCompareEq(item, last);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
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
	last = DeeType_InvokeMethodHint0(self, seq_trygetlast);
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
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_last:
	Dee_Decref(last);
err:
	return -1;
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
	size_t selfsize = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeType_InvokeMethodHint(self, seq_operator_trygetitem_index, end - 1);
	if unlikely(!selfitem)
		goto err;
	if (selfitem == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
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
	size_t selfsize = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if (start >= end)
		return 0;
	selfitem = DeeType_InvokeMethodHint(self, seq_operator_trygetitem_index, end - 1);
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
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
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
