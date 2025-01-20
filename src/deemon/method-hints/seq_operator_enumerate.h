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
/* deemon.Sequence.__seq_enumerate__()                                  */
/************************************************************************/
__seq_enumerate__(cb:?DCallable,start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?X2?O?N {
	Dee_ssize_t foreach_status;
	DeeObject *cb;
	size_t start = 0;
	size_t end = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":__seq_enumerate__", &cb, &start, &end))
		goto err;
	/* TODO */
	if (start == 0 && end == (size_t)-1) {
		foreach_status = DeeSeq_OperatorEnumerate();
	} else {
		foreach_status = DeeSeq_OperatorEnumerateIndex();
	}
	if unlikely(foreach_status == -1)
		goto err;
	/* TODO */
	return 0;
err:
	return NULL;
}

[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_operator_enumerate([[nonnull]] DeeObject *__restrict self,
                                         [[nonnull]] Dee_enumerate_t proc,
                                         void *arg)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); })}
%{$empty = 0}
%{$with__seq_operator_size_and_seq_operator_try_getitem_index = {
	/* TODO */
}}
%{$with__seq_operator_sizeob_and_seq_operator_try_getitem = {
	/* TODO */
}}
%{$with__counter_and_seq_operator_foreach = {
	/* TODO */
}}
%{$with__counter_and_seq_operator_iter = {
	/* TODO */
}}
{
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = LOCAL_CALLATTR(self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}

[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_operator_enumerate_index([[nonnull]] DeeObject *__restrict self,
                                               [[nonnull]] Dee_enumerate_index_t proc,
                                               void *arg, size_t start, size_t end)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); })}
%{$empty = 0}
%{$with__seq_operator_size_and_seq_operator_try_getitem_index = {
	/* TODO */
}}
%{$with__counter_and_seq_operator_foreach = {
	/* TODO */
}}
%{$with__counter_and_seq_operator_iter = {
	/* TODO */
}} {
	DREF DeeObject *result;
	DREF DeeObject *cb_wrapper;
	/* TODO: Invoke user-defined __seq_enumerate__ callback */
	result = LOCAL_CALLATTR(self, 1, &cb_wrapper);
	Dee_Decref_unlikely(cb_wrapper);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	/* TODO */
err:
	return -1;
}


seq_operator_enumerate = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ) {
		if (THIS_TYPE->tp_seq &&
		    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
		    THIS_TYPE->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast
	}
	//TODO: $with__seq_operator_size_and_seq_operator_try_getitem_index
	//TODO: $with__seq_operator_sizeob_and_seq_operator_try_getitem
	//TODO: $with__counter_and_seq_operator_foreach
	//TODO: $with__counter_and_seq_operator_iter
};

seq_operator_enumerate_index = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ) {
		if (THIS_TYPE->tp_seq &&
		    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
		    THIS_TYPE->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast
	}
	//TODO: $with__seq_operator_size_and_seq_operator_try_getitem_index
	//TODO: $with__counter_and_seq_operator_foreach
	//TODO: $with__counter_and_seq_operator_iter
};
