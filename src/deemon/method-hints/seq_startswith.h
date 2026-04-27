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
/* deemon.Sequence.startswith()                                         */
/************************************************************************/

[[kw, alias(Sequence.startswith)]]
__seq_startswith__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dbool {
	int result;
	if (start == 0 && end == (size_t)-1) {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_startswith, self, item);
		} else {
			result = CALL_DEPENDENCY(seq_startswith_with_key, self, item, key);
		}
	} else {
		if (DeeNone_Check(key)) {
			result = CALL_DEPENDENCY(seq_startswith_with_range, self, item, start, end);
		} else {
			result = CALL_DEPENDENCY(seq_startswith_with_range_and_key, self, item, start, end, key);
		}
	}
	if (Dee_HAS_ISERR(result))
		goto err;
	return_bool(result);
err:
	return NULL;
}





/* @return: Dee_HAS_NO:  Does not start with
 * @return: Dee_HAS_YES: Does start with
 * @return: Dee_HAS_ERR: Error */
[[wunused]] int
__seq_startswith__.seq_startswith([[nonnull]] DeeObject *self,
                                  [[nonnull]] DeeObject *item)
%{unsupported(auto)}
%{$empty = Dee_HAS_NO}
%{$with__seq_trygetfirst = {
	int cmp_result;
	DREF DeeObject *first = CALL_DEPENDENCY(seq_trygetfirst, self);
	if (!ITER_ISOK(first))
		return Dee_HAS_FROMITERNOK(first);
	cmp_result = DeeObject_TryCompareEq(item, first);
	Dee_Decref(first);
	return Dee_HAS_FROM_COMPARE_EQ(cmp_result);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &item);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return Dee_HAS_ERR;
}

seq_startswith = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst;
	seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst)
		return &$with__seq_trygetfirst;
};






/* @return: Dee_HAS_NO:  Does not start with
 * @return: Dee_HAS_YES: Does start with
 * @return: Dee_HAS_ERR: Error */
[[wunused]] int
__seq_startswith__.seq_startswith_with_key([[nonnull]] DeeObject *self,
                                           [[nonnull]] DeeObject *item,
                                           [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = Dee_HAS_NO}
%{$with__seq_trygetfirst = {
	int cmp_result;
	DREF DeeObject *first;
	first = CALL_DEPENDENCY(seq_trygetfirst, self);
	if (!ITER_ISOK(first))
		return Dee_HAS_FROMITERNOK(first);
	cmp_result = DeeObject_TryCompareKeyEq(item, first, key);
	Dee_Decref(first);
	return Dee_HAS_FROM_COMPARE_EQ(cmp_result);
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
	return Dee_HAS_ERR;
}

seq_startswith_with_key = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst;
	seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst)
		return &$with__seq_trygetfirst;
};







/* @return: Dee_HAS_NO:  Does not start with
 * @return: Dee_HAS_YES: Does start with
 * @return: Dee_HAS_ERR: Error */
[[wunused]] int
__seq_startswith__.seq_startswith_with_range([[nonnull]] DeeObject *self,
                                             [[nonnull]] DeeObject *item,
                                             size_t start, size_t end)
%{unsupported(auto)}
%{$empty = Dee_HAS_NO}
%{$with__seq_operator_trygetitem_index = {
	int cmp_result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return Dee_HAS_NO;
	selfitem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, start);
	if (!ITER_ISOK(selfitem))
		return Dee_HAS_FROMITERNOK(selfitem);
	cmp_result = DeeObject_TryCompareEq(item, selfitem);
	Dee_Decref(selfitem);
	return Dee_HAS_FROM_COMPARE_EQ(cmp_result);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return Dee_HAS_ERR;
}

seq_startswith_with_range = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index)
		return &$with__seq_operator_trygetitem_index;
};






/* @return: Dee_HAS_NO:  Does not start with
 * @return: Dee_HAS_YES: Does start with
 * @return: Dee_HAS_ERR: Error */
[[wunused]] int
__seq_startswith__.seq_startswith_with_range_and_key([[nonnull]] DeeObject *self,
                                                     [[nonnull]] DeeObject *item,
                                                     size_t start, size_t end,
                                                     [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = Dee_HAS_NO}
%{$with__seq_operator_trygetitem_index = {
	int cmp_result;
	DREF DeeObject *selfitem;
	if (start >= end)
		return Dee_HAS_NO;
	selfitem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, start);
	if (!ITER_ISOK(selfitem))
		return Dee_HAS_FROMITERNOK(selfitem);
	cmp_result = DeeObject_TryCompareKeyEq(item, selfitem, key);
	Dee_Decref(selfitem);
	return Dee_HAS_FROM_COMPARE_EQ(cmp_result);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return Dee_HAS_ERR;
}

seq_startswith_with_range_and_key = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index)
		return &$with__seq_operator_trygetitem_index;
};
