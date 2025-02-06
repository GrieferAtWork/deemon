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
/* deemon.Sequence.operator [:]()                                       */
/************************************************************************/
__seq_getrange__(start?:?X2?Dint?N,end?:?X2?Dint?N)->?S?O {
	DeeObject *start, *end;
	if (DeeArg_Unpack(argc, argv, "oo:__seq_getrange__", &start, &end))
		goto err;
	return DeeType_InvokeMethodHint(self, seq_operator_getrange, start, end);
err:
	return NULL;
}




[[operator(Sequence.OPERATOR_GETRANGE: tp_seq->tp_getrange)]]
[[wunused]] DREF DeeObject *
__seq_getrange__.seq_operator_getrange([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *start,
                                       [[nonnull]] DeeObject *end)
%{unsupported(auto("operator [:]"))}
%{$empty = return_empty_seq}
%{$with__seq_operator_getrange_index__and__seq_operator_getrange_index_n = {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return DeeType_InvokeMethodHint(self, seq_operator_getrange_index_n, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeType_InvokeMethodHint(self, seq_operator_getrange_index, start_index, end_index);
err:
	return NULL;
}} {
	DeeObject *args[2];
	args[0] = start;
	args[1] = end;
	return LOCAL_CALLATTR(self, 2, args);
}


seq_operator_getrange = {
	DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index = REQUIRE(seq_operator_getrange_index);
	if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
		return &$empty;
	if (seq_operator_getrange_index && REQUIRE(seq_operator_getrange_index_n))
		return &$with__seq_operator_getrange_index__and__seq_operator_getrange_index_n;
};







[[operator(Sequence.OPERATOR_GETRANGE: tp_seq->tp_getrange_index)]]
[[wunused]] DREF DeeObject *
__seq_getrange__.seq_operator_getrange_index([[nonnull]] DeeObject *self,
                                             Dee_ssize_t start, Dee_ssize_t end)
%{unsupported(auto("operator [:]"))}
%{$empty = return_empty_seq}
%{$with__seq_operator_getrange = {
	DREF DeeObject *startob, *endob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	endob = DeeInt_NewSSize(end);
	if unlikely(!endob)
		goto err_startob;
	result = DeeType_InvokeMethodHint(self, seq_operator_getrange, startob, endob);
	Dee_Decref(endob);
	Dee_Decref(startob);
	return result;
err_startob:
	Dee_Decref(startob);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	struct Dee_seq_range range;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = range.sr_start;
	result->dssgi_end              = range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem = {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	struct Dee_seq_range range;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, size);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize(range.sr_start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(range.sr_end);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_iter = {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		goto empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dsial_seq     = self;
	result->dsial_start   = range.sr_start;
	result->dsial_limit   = range.sr_end - range.sr_start;
	result->dsial_tp_iter = DeeType_RequireSeqOperatorIter(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return (DREF DeeObject *)result;
empty_seq:
	return_empty_seq;
err:
	return NULL;
}} = $with__seq_operator_getrange;

seq_operator_getrange_index = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (THIS_TYPE->tp_seq &&
	    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    THIS_TYPE->tp_seq->tp_size &&
	    !DeeType_IsDefaultSize(THIS_TYPE->tp_seq->tp_size))
		return &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast;
	seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return $empty;
	if (seq_operator_trygetitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return $empty;
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach) {
				if (REQUIRE(seq_operator_iter))
					return $with__seq_operator_size__and__seq_operator_iter;
			}
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
				DeeMH_seq_operator_getitem_t seq_operator_getitem = REQUIRE(seq_operator_getitem);
				if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
					return $with__seq_operator_size__and__seq_operator_getitem_index;
				if (seq_operator_getitem == &default__seq_operator_getitem__empty)
					return $empty;
				return $with__seq_operator_size__and__seq_operator_getitem;
			}
			return $with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
	}
};




[[operator(Sequence.OPERATOR_GETRANGE: tp_seq->tp_getrange_index_n)]]
[[wunused]] DREF DeeObject *
__seq_getrange__.seq_operator_getrange_index_n([[nonnull]] DeeObject *self,
                                               Dee_ssize_t start)
%{unsupported({
	err_seq_unsupportedf(self, "operator [:](%" PCKdSIZ ", none)", start);
	return NULL;
})}
%{$empty = return_empty_seq}
%{$with__seq_operator_getrange = {
	DREF DeeObject *startob, *result;
	startob = DeeInt_NewSSize(start);
	if unlikely(!startob)
		goto err;
	result = DeeType_InvokeMethodHint(self, seq_operator_getrange, startob, Dee_None);
	Dee_Decref(startob);
	return result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getrange_index = {
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
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
	return DeeType_InvokeMethodHint(self, seq_operator_getrange_index, start, (Dee_ssize_t)size);
empty_range:
	return_empty_seq;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
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
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
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
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->dssgi_seq              = self;
	result->dssgi_tp_getitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self));
	result->dssgi_start            = (size_t)start;
	result->dssgi_end              = size;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem = {
	DREF DefaultSequence_WithSizeAndGetItem *result;
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
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
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	result->dssg_start = DeeInt_NewSize((size_t)start);
	if unlikely(!result->dssg_start)
		goto err_r;
	result->dssg_end = DeeInt_NewSize(size);
	if unlikely(!result->dssg_end)
		goto err_r_start;
	Dee_Incref(self);
	result->dssg_seq        = self;
	result->dssg_tp_getitem = DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self));
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return (DREF DeeObject *)result;
empty_range:
	return_empty_seq;
err_r_start:
	Dee_Decref(result->dssg_start);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_iter = {
	return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter(self, start, (size_t)-1);
}} = $with__seq_operator_getrange;



seq_operator_getrange_index_n = {
	DeeMH_seq_operator_size_t seq_operator_size;
	if (THIS_TYPE->tp_seq &&
	    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    THIS_TYPE->tp_seq->tp_size &&
	    !DeeType_IsDefaultSize(THIS_TYPE->tp_seq->tp_size))
		return &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return $empty;
		seq_operator_getrange_index = REQUIRE(seq_operator_getrange_index);
		if (seq_operator_getrange_index == &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast)
			return &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
			return &$empty;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index)
			return $with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return $with__seq_operator_size__and__seq_operator_trygetitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem)
			return $with__seq_operator_size__and__seq_operator_getitem;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter)
			return $with__seq_operator_size__and__seq_operator_iter;
		if (seq_operator_getrange_index)
			return $with__seq_operator_size__and__seq_operator_getrange_index;
	}
};
