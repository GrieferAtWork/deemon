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
__seq_setrange__(start?:?X2?Dint?N,end?:?X2?Dint?N,values:?S?O) {
	DeeObject *start, *end, *values;
	if (DeeArg_Unpack(argc, argv, "ooo:__seq_setrange__", &start, &end, &values))
		goto err;
	if (DeeSeq_OperatorSetRange(self, start, end, values))
		goto err;
	return_none;
err:
	return NULL;
}

[[wunused]] int
__seq_setrange__.seq_operator_setrange([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *start,
                                       [[nonnull]] DeeObject *end,
                                       [[nonnull]] DeeObject *values)
%{unsupported(auto("operator [:]="))}
%{$empty = {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange__unsupported(self, start, end, values);
err:
	return -1;
}}
%{$with__seq_operator_setrange_index__and__seq_operator_setrange_index_n = {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorSetRangeIndexN(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorSetRangeIndex(self, start_index, end_index, values);
err:
	return -1;
}} {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = start;
	args[1] = end;
	args[2] = values;
	result = LOCAL_CALLATTR(self, 3, args);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}


seq_operator_setrange = {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireSetRange(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_setrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return &$empty;
	if (seq_operator_setrange_index && REQUIRE(seq_operator_setrange_index_n))
		return &$with__seq_operator_setrange_index__and__seq_operator_setrange_index_n;
};







[[wunused]] int
__seq_setrange__.seq_operator_setrange_index([[nonnull]] DeeObject *self,
                                             Dee_ssize_t start, Dee_ssize_t end,
                                             [[nonnull]] DeeObject *values)
%{unsupported(auto("operator [:]="))}
%{$empty = {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange_index__unsupported(self, start, end, values);
err:
	return -1;
}}
%{$with__seq_operator_setrange = {
	int result;
	DREF DeeObject *startob, *endob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeSeq_OperatorSetRange(self, startob, endob, values);
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
	size_t size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_end > range.sr_start) {
		/* Erase what was there before... */
		if unlikely(DeeSeq_InvokeErase(self, range.sr_start, range.sr_end - range.sr_start))
			goto err;
	}
	/* Insert new values. */
	return DeeSeq_InvokeInsertAll(self, range.sr_start, values);
err:
	return -1;
}} = $with__seq_operator_setrange;

seq_operator_setrange_index = {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_setrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE_NODEFAULT(seq_erase) &&
		    REQUIRE_NODEFAULT(seq_insertall))
			return &$with__seq_operator_size__and__seq_erase__and__seq_insertall;
	}
};




[[wunused]] int
__seq_setrange__.seq_operator_setrange_index_n([[nonnull]] DeeObject *self,
                                               Dee_ssize_t start,
                                               [[nonnull]] DeeObject *values)
%{unsupported({
	return err_seq_unsupportedf(self, "operator [:]=(%" PCKdSIZ ", none, %r)", start, values);
})}
%{$empty = {
	int values_empty = DeeSeq_OperatorBool(values);
	if unlikely(values_empty < 0)
		goto err;
	if (values_empty)
		return 0;
	return default__seq_operator_setrange_index_n__unsupported(self, start, values);
err:
	return -1;
}}
%{$with__seq_operator_setrange_index = {
	size_t size = DeeSeq_OperatorSize(self);
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
	return DeeSeq_OperatorSetRangeIndex(self, start, (Dee_ssize_t)size, values);
err:
	return -1;
}}
%{$with__seq_operator_setrange = {
	int result;
	DREF DeeObject *startob;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeSeq_OperatorSetRange(self, startob, Dee_None, values);
	Dee_Decref(startob);
	return result;
err:
	return -1;
}} = $with__seq_operator_setrange;



seq_operator_setrange_index_n = {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndexN(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_setrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return $empty;
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_setrange)
		return $with__seq_operator_setrange;
	if (seq_operator_setrange_index)
		return $with__seq_operator_setrange_index;
};


