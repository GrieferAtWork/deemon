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
/* deemon.Sequence.__seq_trycompare_eq__()                              */
/************************************************************************/
__seq_trycompare_eq__(rhs:?S?O)->?Dbool {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_trycompare_eq__", &rhs))
		goto err;
	result = DeeSeq_OperatorTryCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

[[wunused]] int
__seq_trycompare_eq__.seq_operator_trycompare_eq([[nonnull]] DeeObject *lhs,
                                                 [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = {
	DeeMH_seq_operator_bool_t seq_operator_bool;
	int rhs_nonempty;
	seq_operator_bool = (DeeMH_seq_operator_bool_t)DeeType_GetMethodHint(Dee_TYPE(rhs), Dee_TMH_seq_operator_bool);
	if (seq_operator_bool == &default__seq_operator_bool__unsupported)
		return 1;
	rhs_nonempty = (*seq_operator_bool)(rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? 0 : 1 /*or: -1*/;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_foreach = {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_foreach(lhs, rhs);
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index(lhs, rhs);
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index(lhs, rhs);
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if ((!tp_rhs->tp_seq || !tp_rhs->tp_seq->tp_foreach) && !DeeType_InheritIter(tp_rhs))
		return 1;
	return default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem(lhs, rhs);
}}
%{$with__seq_operator_compare_eq = {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
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

seq_operator_trycompare_eq = {
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireTryCompareEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_trycompare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (THIS_TYPE->tp_seq &&
	    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    THIS_TYPE->tp_seq->tp_size)
		return &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast;
	seq_operator_compare_eq = REQUIRE(seq_operator_compare_eq);
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
		return &$empty;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_compare_eq)
		return &$with__seq_operator_compare_eq;
};
