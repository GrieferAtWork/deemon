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
/* deemon.Sequence.operator [:]=()                                      */
/************************************************************************/
[[kw]]
__seq_setrange__(start:?X2?Dint?N,end:?X2?Dint?N,items:?X2?DSequence?S?O) {
	if (CALL_DEPENDENCY(seq_operator_setrange, self, start, end, items))
		goto err;
	return_none;
err:
	return NULL;
}

[[operator(Sequence: tp_seq->tp_setrange)]]
[[wunused]] int
__seq_setrange__.seq_operator_setrange([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *start,
                                       [[nonnull]] DeeObject *end,
                                       [[nonnull]] DeeObject *items)
%{unsupported(auto("operator [:]="))}
%{$none = 0}
%{$empty = {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange__unsupported(self, start, end, items);
err:
	return -1;
}}
%{using [seq_operator_setrange_index, seq_operator_setrange_index_n]: {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return CALL_DEPENDENCY(seq_operator_setrange_index_n, self, start_index, items);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return CALL_DEPENDENCY(seq_operator_setrange_index, self, start_index, end_index, items);
err:
	return -1;
}} {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = items;
	result = LOCAL_CALLATTR(self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}


seq_operator_setrange = {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return &$empty;
	if (seq_operator_setrange_index && REQUIRE(seq_operator_setrange_index_n))
		return &$with__seq_operator_setrange_index__and__seq_operator_setrange_index_n;
};







[[operator(Sequence: tp_seq->tp_setrange_index)]]
[[wunused]] int
__seq_setrange__.seq_operator_setrange_index([[nonnull]] DeeObject *self,
                                             Dee_ssize_t start, Dee_ssize_t end,
                                             [[nonnull]] DeeObject *items)
%{unsupported(auto("operator [:]="))}
%{$none = 0}
%{$empty = {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index__unsupported(self, start, end, items);
err:
	return -1;
}}
%{using seq_operator_setrange: {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = CALL_DEPENDENCY(seq_operator_setrange, self, startob, endob, items);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_erase__and__seq_insertall = {
	struct Dee_seq_range range;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_end > range.sr_start) {
		/* Erase what was there before... */
		if unlikely(CALL_DEPENDENCY(seq_erase, self, range.sr_start, range.sr_end - range.sr_start))
			goto err;
	}
	/* Insert new items. */
	return CALL_DEPENDENCY(seq_insertall, self, range.sr_start, items);
err:
	return -1;
}} = $with__seq_operator_setrange;

seq_operator_setrange_index = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE_NODEFAULT(seq_erase) &&
		    REQUIRE_NODEFAULT(seq_insertall))
			return &$with__seq_operator_size__and__seq_erase__and__seq_insertall;
	}
};




[[operator(Sequence: tp_seq->tp_setrange_index_n)]]
[[wunused]] int
__seq_setrange__.seq_operator_setrange_index_n([[nonnull]] DeeObject *self,
                                               Dee_ssize_t start,
                                               [[nonnull]] DeeObject *items)
%{unsupported({
	return err_seq_unsupportedf(self, "operator [:]=(%" PCKdSIZ ", none, %r)", start, items);
})}
%{$none = 0}
%{$empty = {
	int items_empty = DeeObject_InvokeMethodHint(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_operator_setrange_index_n__unsupported(self, start, items);
err:
	return -1;
}}
%{using [seq_operator_size, seq_operator_setrange_index]: {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0) {
					start = 0;
				} else {
					start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
				}
			}
		}
	}
	return CALL_DEPENDENCY(seq_operator_setrange_index, self, start, (Dee_ssize_t)size, items);
err:
	return -1;
}}
%{using seq_operator_setrange: {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = CALL_DEPENDENCY(seq_operator_setrange, self, startob, Dee_None, items);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}} = $with__seq_operator_setrange;



seq_operator_setrange_index_n = {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return &$empty;
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_setrange)
		return &$with__seq_operator_setrange;
	if (seq_operator_setrange_index &&
	    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
		return &$with__seq_operator_size__and__seq_operator_setrange_index;
};


