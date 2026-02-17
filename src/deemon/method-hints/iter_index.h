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
/* deemon.Iterator.index                                                */
/************************************************************************/

[[getset]]
[[alias(Iterator.index)]]
__iter_index__->?X2?DNumeric?Dint = {
	get(self) {
		size_t index = CALL_DEPENDENCY(iter_getindex, self);
		if unlikely(index == (size_t)-1)
			goto err;
		return DeeInt_NewSize(index);
	err:
		return NULL;
	}

	del(self) {
		return CALL_DEPENDENCY(iter_rewind, self);
	}

	set(self, index) {
		size_t index_value;
		if (DeeObject_AsSize(index, &index_value))
			goto err;
		return CALL_DEPENDENCY(iter_setindex, self, index_value);
	err:
		return -1;
	}
}

/* Return the iterator's index */
[[no_self_invocation_wrapper]]
[[wunused, getset_member("get")]] size_t
__iter_index__.iter_getindex([[nonnull]] DeeObject *__restrict self)
%{unsupported(err_iter_unsupportedf(self, "index"))}
%{$empty = 0}
//TODO:%{$with__iter_getseq__and__iter_remaining = {}}
%{$with__iter_getseq__and__iter_operator_compare_eq = {
	size_t result;
	DREF DeeObject *scan;
	DREF DeeObject *seq = CALL_DEPENDENCY(iter_getseq, self);
	DeeMH_iter_advance_t iter_advance;
	if unlikely(!seq)
		goto err;
	scan = DeeObject_Iter(seq);
	Dee_Decref_unlikely(seq);
	if unlikely(!scan)
		goto err;
	iter_advance = DeeObject_RequireMethodHint(scan, iter_advance);
	for (result = 0;;) {
		size_t step;
		int temp = DeeObject_CompareEq(self, scan); /* TODO: CALL_DEPENDENCY(iter_operator_compare_eq) */
		if unlikely(Dee_COMPARE_ISERR(temp))
			goto err_scan;
		if (Dee_COMPARE_ISEQ(temp))
			break;
		step = (*iter_advance)(scan, 1);
		if unlikely(step == (size_t)-1)
			goto err_scan;
		if unlikely(!step)
			break;
		result += step;
	}
	Dee_Decref_likely(scan);
	return result;
err_scan:
	Dee_Decref_likely(scan);
err:
	return (size_t)-1;
}} {
	size_t index;
	DREF DeeObject *result = LOCAL_GETATTR(self);
	if unlikely(!result)
		goto err;
	if (DeeObject_AsSize(result, &index))
		goto err_r;
	Dee_Decref(result);
	if unlikely(index == (size_t)-1)
		DeeRT_ErrIntegerOverflowU(index, (size_t)-2);
	return index;
err_r:
	Dee_Decref(result);
err:
	return (size_t)-1;
}


iter_getindex = {
	DeeMH_iter_getseq_t iter_getseq = REQUIRE(iter_getseq);
	if (iter_getseq == &default__iter_getseq__empty)
		return &$empty;
	if (iter_getseq) {
		if (DeeType_RequireSupportedNativeOperator(THIS_TYPE, compare_eq)) /* TODO: REQUIRE_NODEFAULT(iter_operator_compare_eq) */
			return &$with__iter_getseq__and__iter_operator_compare_eq;
	}
};



/* Set the next item to-be enumerated to "index" (or exhaust the iterator when "index" is too great) */
[[no_self_invocation_wrapper]]
[[wunused, getset_member("set")]] int
__iter_index__.iter_setindex([[nonnull]] DeeObject *self, size_t index)
%{unsupported(err_seq_unsupportedf(self, "index = %" PRFuSIZ, index))}
%{$empty = 0}
%{$with__iter_getseq__and__iter_operator_assign = {
	int result;
	DREF DeeObject *scan;
	DREF DeeObject *seq = CALL_DEPENDENCY(iter_getseq, self);
	if unlikely(!seq)
		goto err;
	scan = DeeObject_Iter(seq);
	Dee_Decref_unlikely(seq);
	if unlikely(!scan)
		goto err;
	if (DeeObject_InvokeMethodHint(iter_advance, scan, index) == (size_t)-1)
		goto err_scan;
	result = DeeObject_Assign(self, scan); /* TODO: CALL_DEPENDENCY(iter_operator_assign) */
	Dee_Decref_likely(scan);
	return result;
err_scan:
	Dee_Decref_likely(scan);
err:
	return -1;
}} {
	int result;
	DREF DeeObject *index_ob = DeeInt_NewSize(index);
	if unlikely(!index_ob)
		goto err;
	result = LOCAL_SETATTR(self, index_ob);
	Dee_Decref(index_ob);
	return result;
err:
	return -1;
}



iter_setindex = {
	DeeMH_iter_getseq_t iter_getseq = REQUIRE(iter_getseq);
	if (iter_getseq == &default__iter_getseq__empty)
		return &$empty;
	if (iter_getseq) {
		if (DeeType_RequireSupportedNativeOperator(THIS_TYPE, assign)) /* TODO: REQUIRE_NODEFAULT(iter_operator_assign) */
			return &$with__iter_getseq__and__iter_operator_assign;
	}
};



[[alias(Iterator.rewind)]]
__iter_rewind__() {
	if unlikely(CALL_DEPENDENCY(iter_rewind, self))
		goto err;
	return_none;
err:
	return NULL;
}

/* Rewind the iterator to its beginning */
[[extra_method_hint_context("__iter_rewind__")]]
[[wunused, getset_member("del")]] int
__iter_index__.iter_rewind([[nonnull]] DeeObject *__restrict self)
%{unsupported(err_seq_unsupportedf(self, "del index"))}
%{$empty = 0}
%{$with_callattr_rewind = {
	/* custom hack (s.a. `gpmhnd_extra__iter_rewind()') */
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, Dee_AsObject(&str_rewind), 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}}
%{$with_callattr___iter_rewind__ = {
	/* custom hack (s.a. `gpmhnd_extra__iter_rewind()') */
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, Dee_AsObject(&str___iter_rewind__), 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}}
%{$with__iter_getseq__and__iter_operator_assign = {
	int result;
	DREF DeeObject *scan;
	DREF DeeObject *seq = CALL_DEPENDENCY(iter_getseq, self);
	if unlikely(!seq)
		goto err;
	scan = DeeObject_Iter(seq);
	Dee_Decref_unlikely(seq);
	if unlikely(!scan)
		goto err;
	result = DeeObject_Assign(self, scan); /* TODO: CALL_DEPENDENCY(iter_operator_assign) */
	Dee_Decref_likely(scan);
	return result;
err:
	return -1;
}}
%{$with__iter_setindex = {
	return CALL_DEPENDENCY(iter_setindex, self, 0);
}} {
	return LOCAL_DELATTR(self);
}


iter_rewind = {
	DeeMH_iter_setindex_t iter_setindex = REQUIRE(iter_setindex);
	if (iter_setindex == &default__iter_setindex__empty)
		return &$empty;
	if (iter_setindex == &default__iter_setindex__with__iter_getseq__and__iter_operator_assign)
		return &$with__iter_getseq__and__iter_operator_assign;
	if (iter_setindex)
		return &$with__iter_setindex;
};
