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

%[define(DEFINE_default_enumerate_with_counter_and_foreach_cb =
#ifndef DEFINED_default_enumerate_with_counter_and_foreach_cb
#define DEFINED_default_enumerate_with_counter_and_foreach_cb
struct default_enumerate_with_counter_and_foreach_data {
	Dee_enumerate_t dewcaf_proc;    /* [1..1] Wrapped callback */
	void           *dewcaf_arg;     /* [?..?] Cookie for `dewcaf_proc' */
	size_t          dewcaf_counter; /* Index of the next element that will be enumerated */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_with_counter_and_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_enumerate_with_counter_and_foreach_cb */
)]



[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_operator_enumerate([[nonnull]] DeeObject *__restrict self,
                                         [[nonnull]] Dee_enumerate_t proc,
                                         void *arg)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(...)"); })}
%{$empty = 0}
%{$with__seq_operator_size_and_seq_operator_getitem_index = {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	DREF DeeObject *indexob, *index_value;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		indexob = DeeInt_NewSize(i);
		if unlikely(!indexob)
			goto err;
		index_value = DeeSeq_OperatorTryGetItemIndex(self, i);
		if unlikely(!index_value)
			goto err_indexob;
		if (index_value == ITER_DONE) {
			temp = (*proc)(arg, indexob, NULL);
		} else {
			temp = (*proc)(arg, indexob, index_value);
			Dee_Decref(index_value);
		}
		Dee_Decref(indexob);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err_indexob:
	Dee_Decref(indexob);
err:
	return -1;
}}
%{$with__seq_operator_sizeob_and_seq_operator_getitem = {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *indexob, *index_value, *sizeob;
	sizeob = DeeSeq_OperatorSizeOb(self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	for (;;) {
		int index_is_less_than_size = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (index_is_less_than_size <= 0) {
			if unlikely(index_is_less_than_size < 0)
				goto err_sizeob_indexob;
			break;
		}
		index_value = DeeSeq_OperatorGetItem(self, indexob);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
		}
		temp = (*proc)(arg, indexob, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp_sizeob_indexob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_indexob:
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return temp;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}}
%{$with__counter_and_seq_operator_foreach =
[[prefix(DEFINE_default_enumerate_with_counter_and_foreach_cb)]] {
	struct default_enumerate_with_counter_and_foreach_data data;
	data.dewcaf_proc    = proc;
	data.dewcaf_arg     = arg;
	data.dewcaf_counter = 0;
	return DeeSeq_OperatorForeach(self, &default_enumerate_with_counter_and_foreach_cb, &data);
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

%[define(DEFINE_default_enumerate_index_with_counter_and_foreach_cb =
#ifndef DEFINED_default_enumerate_index_with_counter_and_foreach_cb
#define DEFINED_default_enumerate_index_with_counter_and_foreach_cb
#define default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */

struct default_enumerate_index_with_counter_and_foreach_data {
	Dee_enumerate_index_t deiwcaf_proc;  /* [1..1] Wrapped callback */
	void                 *deiwcaf_arg;   /* [?..?] Cookie for `deiwcaf_proc' */
	size_t                deiwcaf_index; /* Index of the next element that will be enumerate_indexd */
	size_t                deiwcaf_start; /* Enumeration start index */
	size_t                deiwcaf_end;   /* Enumeration end index */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_counter_and_foreach_cb(void *arg, DeeObject *elem);
#endif /* !DEFINED_default_enumerate_index_with_counter_and_foreach_cb */
)]

[[wunused]] Dee_ssize_t
__seq_enumerate__.seq_operator_enumerate_index([[nonnull]] DeeObject *__restrict self,
                                               [[nonnull]] Dee_enumerate_index_t proc,
                                               void *arg, size_t start, size_t end)
%{unsupported({ return err_seq_unsupportedf(self, "__seq_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); })}
%{$empty = 0}
%{$with__seq_operator_size_and_seq_operator_getitem_index = {
	Dee_ssize_t temp, result = 0;
	size_t i, size;
	size = DeeSeq_OperatorSize(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (end > size)
		end = size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *index_value;
		index_value = DeeSeq_OperatorGetItemIndex(self, i);
		if unlikely(!index_value) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, i, index_value);
		Dee_XDecref(index_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__counter_and_seq_operator_foreach =
[[prefix(DEFINE_default_enumerate_index_with_counter_and_foreach_cb)]] {
	struct default_enumerate_index_with_counter_and_foreach_data data;
	Dee_ssize_t result;
	data.deiwcaf_proc  = proc;
	data.deiwcaf_arg   = arg;
	data.deiwcaf_index = 0;
	data.deiwcaf_start = start;
	data.deiwcaf_end   = end;
	result = DeeSeq_OperatorForeach(self, &default_enumerate_index_with_counter_and_foreach_cb, &data);
	if unlikely(result == default_enumerate_index_with_counter_and_foreach_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
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
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ) {
		if (THIS_TYPE->tp_seq &&
		    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
		    THIS_TYPE->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast;
	}
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		seq_operator_getitem = REQUIRE(seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
			return &$with__seq_operator_size_and_seq_operator_getitem_index;
		if (seq_operator_getitem != NULL)
			return &$with__seq_operator_sizeob_and_seq_operator_getitem;
	}
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__counter_and_seq_operator_foreach;
};

seq_operator_enumerate_index = {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ) {
		if (THIS_TYPE->tp_seq &&
		    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
		    THIS_TYPE->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast;
	}
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE(seq_operator_getitem_index))
			return &$with__seq_operator_size_and_seq_operator_getitem_index;
	}
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__counter_and_seq_operator_foreach;
};
