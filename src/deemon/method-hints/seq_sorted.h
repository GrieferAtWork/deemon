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
/* deemon.Sequence.sorted()                                             */
/************************************************************************/
[[kw, alias(Sequence.sorted)]]
__seq_sorted__(size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N) {
	return !DeeNone_Check(key)
	       ? CALL_DEPENDENCY(seq_sorted_with_key, self, start, end, key)
	       : CALL_DEPENDENCY(seq_sorted, self, start, end);
}





[[wunused]] DREF DeeObject *
__seq_sorted__.seq_sorted([[nonnull]] DeeObject *self,
                          size_t start, size_t end)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_empty_seq}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DeeTupleObject *result;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFast(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self,
	                                        start, THIS_TYPE->tp_seq->tp_getitem_index_fast))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DREF DeeTupleObject *result;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndex(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self, start,
	                                       REQUIRE_DEPENDENCY(seq_operator_trygetitem_index)))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_DeeSeq_GetForeachSubRangeAsTuple)]] {
	DREF DeeTupleObject *base, *result;
	base = (DREF DeeTupleObject *)DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVector(DeeTuple_SIZE(result),
	                              DeeTuple_ELEM(result),
	                              DeeTuple_ELEM(base)))
		goto err_base_r;
	DeeTuple_FreeUninitialized(base);
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}





[[wunused]] DREF DeeObject *
__seq_sorted__.seq_sorted_with_key([[nonnull]] DeeObject *self,
                                   size_t start, size_t end,
                                   [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_empty_seq}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DeeTupleObject *result;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortGetItemIndexFastWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self,
	                                               start, THIS_TYPE->tp_seq->tp_getitem_index_fast, key))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DREF DeeTupleObject *result;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	if unlikely(start > end)
		start = end;
	end -= start;
	if (!end)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(end);
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_SortTryGetItemIndexWithKey(DeeTuple_SIZE(result), DeeTuple_ELEM(result), self, start,
	                                              REQUIRE_DEPENDENCY(seq_operator_trygetitem_index), key))
		goto err_r;
	if unlikely(DeeTuple_GET(result, 0) == NULL) {
		/* Must trim unbound items (which were sorted to the start of the tuple) */
		size_t n_unbound = 1;
		while (n_unbound < end && DeeTuple_GET(result, n_unbound) == NULL)
			++n_unbound;
		end -= n_unbound;
		memmovedownc(DeeTuple_ELEM(result),
		             DeeTuple_ELEM(result) + n_unbound,
		             end, sizeof(DREF DeeObject *));
		result = DeeTuple_TruncateUninitialized(result, end);
	}
	return (DREF DeeObject *)result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_DeeSeq_GetForeachSubRangeAsTuple)]] {
	DREF DeeTupleObject *base, *result;
	base = (DREF DeeTupleObject *)DeeSeq_GetForeachSubRangeAsTuple(self, start, end);
	if unlikely(!base)
		goto err;
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(base));
	if unlikely(!result)
		goto err_base;
	if unlikely(DeeSeq_SortVectorWithKey(DeeTuple_SIZE(result),
	                                     DeeTuple_ELEM(result),
	                                     DeeTuple_ELEM(base), key))
		goto err_base_r;
	DeeTuple_FreeUninitialized(base);
	return (DREF DeeObject *)result;
err_base_r:
	DeeTuple_FreeUninitialized(result);
err_base:
	Dee_Decref(base);
err:
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
}





seq_sorted = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	if (REQUIRE(seq_operator_foreach))
		return &$with__seq_operator_foreach;
};

seq_sorted_with_key = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	if (REQUIRE(seq_operator_foreach))
		return &$with__seq_operator_foreach;
};
