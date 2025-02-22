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
/* deemon.Sequence.operator del[:]()                                    */
/************************************************************************/
[[kw]]
__seq_delrange__(start?:?X2?Dint?N,end?:?X2?Dint?N) {
	DeeObject *start = Dee_None, *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|oo:__seq_delrange__", &start, &end))
		goto err;
	if (CALL_DEPENDENCY(seq_operator_delrange, self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}




[[operator(Sequence: tp_seq->tp_delrange)]]
[[wunused]] int
__seq_delrange__.seq_operator_delrange([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *start,
                                       [[nonnull]] DeeObject *end)
%{unsupported(auto("operator del[:]"))}
%{$empty = 0}
%{using [seq_operator_delrange_index, seq_operator_delrange_index_n]: {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return CALL_DEPENDENCY(seq_operator_delrange_index_n, self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return CALL_DEPENDENCY(seq_operator_delrange_index, self, start_index, end_index);
err:
	return -1;
}}
%{$with__seq_operator_setrange = {
	return CALL_DEPENDENCY(seq_operator_setrange, self, start, end, Dee_None);
}} {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	result = LOCAL_CALLATTR(self, 2, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}


seq_operator_delrange = {
	DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
	if (REQUIRE_NODEFAULT(seq_operator_setrange))
		return &$with__seq_operator_setrange;
	seq_operator_delrange_index = REQUIRE(seq_operator_delrange_index);
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__empty)
		return &$empty;
	if (seq_operator_delrange_index && REQUIRE(seq_operator_delrange_index_n))
		return &$with__seq_operator_delrange_index__and__seq_operator_delrange_index_n;
};







[[operator(Sequence: tp_seq->tp_delrange_index)]]
[[wunused]] int
__seq_delrange__.seq_operator_delrange_index([[nonnull]] DeeObject *self,
                                             Dee_ssize_t start, Dee_ssize_t end)
%{unsupported(auto("operator del[:]"))}
%{$empty = 0}
%{using seq_operator_delrange: {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = CALL_DEPENDENCY(seq_operator_delrange, self, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_erase = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return CALL_DEPENDENCY(seq_erase, self, (size_t)start, size - (size_t)start);
empty_range:
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_setrange_index = {
	return CALL_DEPENDENCY(seq_operator_setrange_index, self, start, end, Dee_None);
}} = $with__seq_operator_delrange;

seq_operator_delrange_index = {
	DeeMH_seq_operator_size_t seq_operator_size;
	if (REQUIRE_NODEFAULT(seq_operator_setrange_index))
		return &$with__seq_operator_setrange_index;
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE_NODEFAULT(seq_erase))
			return &$with__seq_operator_size__and__seq_erase;
	}
};




[[operator(Sequence: tp_seq->tp_delrange_index_n)]]
[[wunused]] int
__seq_delrange__.seq_operator_delrange_index_n([[nonnull]] DeeObject *self,
                                               Dee_ssize_t start)
%{unsupported({
	return err_seq_unsupportedf(self, "operator del[:](%" PCKdSIZ ", none)", start);
})}
%{$empty = 0}
%{using [seq_operator_size, seq_operator_delrange_index]: {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (start < 0) {
		if (start < 0) {
			start += size;
			if unlikely(start < 0) {
				if unlikely(size == 0)
					goto empty_range;
				start = (Dee_ssize_t)do_fix_negative_range_index(start, size);
			}
		}
	}
	return CALL_DEPENDENCY(seq_operator_delrange_index, self, start, (Dee_ssize_t)size);
empty_range:
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_delrange = {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = CALL_DEPENDENCY(seq_operator_delrange, self, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_setrange_index_n = {
	return CALL_DEPENDENCY(seq_operator_setrange_index_n, self, start, Dee_None);
}} = $with__seq_operator_delrange;



seq_operator_delrange_index_n = {
	DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
	if (REQUIRE_NODEFAULT(seq_operator_setrange_index_n))
		return &$with__seq_operator_setrange_index_n;
	seq_operator_delrange_index = REQUIRE(seq_operator_delrange_index);
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__empty)
		return $empty;
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__with__seq_operator_delrange)
		return $with__seq_operator_delrange;
	if (seq_operator_delrange_index &&
	    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
		return $with__seq_operator_size__and__seq_operator_delrange_index;
};


