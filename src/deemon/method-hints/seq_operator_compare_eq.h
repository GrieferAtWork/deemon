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
/* deemon.Sequence.__seq_compare_eq__()                                 */
/************************************************************************/
__seq_compare_eq__(rhs:?S?O)->?Dbool {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_compare_eq__", &rhs))
		goto err;
	result = DeeType_InvokeMethodHint(lhs, seq_operator_compare_eq, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

[[operator(Sequence.OPERATOR_EQ: tp_cmp->tp_compare_eq)]]
[[wunused]] int
__seq_compare_eq__.seq_operator_compare_eq([[nonnull]] DeeObject *lhs,
                                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__seq_operator_compare__empty"}
%{$with__seq_operator_foreach = {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
	Dee_ssize_t result;
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = DeeType_InvokeMethodHint(lhs, seq_operator_foreach, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_trygetitem_index;
		result = DeeType_InvokeMethodHint(lhs, seq_operator_foreach, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index;
		result = DeeType_InvokeMethodHint(lhs, seq_operator_foreach, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = DeeType_InvokeMethodHint(lhs, seq_operator_foreach, &seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				Dee_Decref(data.scf_sg_oindex);
				if unlikely(temp < 0) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
				}
			} else {
				Dee_Decref(data.scf_sg_oindex);
			}
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = (*tp_rhs->tp_seq->tp_iter)(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = DeeType_InvokeMethodHint(lhs, seq_operator_foreach, &seq_compareeq__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
	if unlikely(result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL)
		return 0;
	return 1;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *lhs, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_trygetitem_index = Dee_TYPE(lhs)->tp_seq->tp_trygetitem_index;
	lhs_size = (*Dee_TYPE(lhs)->tp_seq->tp_size)(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (tp_rhs->tp_seq->tp_size_fast != NULL) {
		size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
		if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
			return 1;
	}
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                         rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                       rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                    rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_trygetitem_index,
		                                                                                rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_trygetitem_index;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	int result;
	size_t lhs_size;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *lhs, size_t index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem_index = Dee_TYPE(lhs)->tp_seq->tp_getitem_index;
	lhs_size = (*Dee_TYPE(lhs)->tp_seq->tp_size)(lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (tp_rhs->tp_seq->tp_size_fast != NULL) {
		size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
		if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
			return 1;
	}
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index,
		                                                                                      rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                                    rhs, rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index,
		                                                                                 rhs, rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index,
		                                                                             rhs, rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&
		    data.scf_sgi_oindex >= data.scf_sgi_osize) {
			result = 0;
		} else {
			result = 1;
		}
	}
	return result;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	int result;
	DREF DeeObject *lhs_sizeob;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *lhs, DeeObject *index);
	Dee_ssize_t (DCALL *other_tp_foreach)(DeeObject *__restrict lhs, Dee_foreach_t proc, void *arg);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		goto err_other_no_iter;
	other_tp_foreach = tp_rhs->tp_seq->tp_foreach;
	ASSERT(other_tp_foreach);
	lhs_getitem = Dee_TYPE(lhs)->tp_seq->tp_getitem;
	lhs_sizeob = (*Dee_TYPE(lhs)->tp_seq->tp_sizeob)(lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndexFast) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                                  rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndTryGetItemIndex) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeAndGetItemIndex ||
	           other_tp_foreach == &DeeSeq_DefaultForeachWithSizeDefaultAndGetItemIndexDefault) {
		size_t rhs_size = (*tp_rhs->tp_seq->tp_size)(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                             rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (other_tp_foreach == &DeeSeq_DefaultForeachWithSizeObAndGetItem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, lhs_getitem, rhs,
		                                                                         rhs_sizeob, tp_rhs->tp_seq->tp_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		Dee_ssize_t foreach_result;
		DREF DeeObject *lhs_indexob;
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));
		if unlikely(!lhs_indexob)
			goto err_lhs_sizeob;
		data.scf_sg_other    = lhs;
		data.scf_sg_osize    = lhs_sizeob;
		data.scf_sg_oindex   = lhs_indexob;
		data.scf_sg_ogetitem = lhs_getitem;
		foreach_result = (*other_tp_foreach)(rhs, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		lhs_indexob = data.scf_sg_oindex;
		Dee_Decref(lhs_indexob);
		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);
		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
	return Dee_COMPARE_ERR;
err_other_no_iter:
	err_unimplemented_operator(tp_rhs, OPERATOR_ITER);
err:
	return Dee_COMPARE_ERR;
}} {
	int result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	result = DeeObject_BoolInherited(resultob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}

seq_operator_compare_eq = {
	DeeMH_seq_operator_compare_t seq_operator_compare;
	if (THIS_TYPE->tp_seq &&
	    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    THIS_TYPE->tp_seq->tp_size)
		return &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast;

	seq_operator_compare = REQUIRE(seq_operator_compare);
	if (seq_operator_compare == &default__seq_operator_compare__empty)
		return &$empty;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_compare)
		return seq_operator_compare; /* Binary-compatible! */
};
