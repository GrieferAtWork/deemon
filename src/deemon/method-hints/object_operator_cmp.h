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
/* deemon.Object.operator <=>()                                         */
/************************************************************************/

operator {


/* Same as "tp_compare", but only needs to support equal/not-equal compare:
 * @return: Dee_COMPARE_ERR: An error occurred.
 * @return: -1: `lhs != rhs'
 * @return: 0:  `lhs == rhs'
 * @return: 1:  `lhs != rhs' */
[[wunused]] int
tp_cmp->tp_compare_eq([[nonnull]] DeeObject *lhs,
                      [[nonnull]] DeeObject *rhs)
%{using tp_cmp->tp_compare: { // TODO: Special handling in linker: directly alias
	return CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
}}
%{using tp_cmp->tp_eq: {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}}
%{using tp_cmp->tp_ne: {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_lo, tp_cmp->tp_gr]: {
	int temp;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_lo, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_gr, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_le, tp_cmp->tp_ge]: {
	int temp;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_le, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_gr, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return -1; /* Different */
	return 0;
err:
	return Dee_COMPARE_ERR;
}} = OPERATOR_EQ;


/* Rich-compare operator that can be defined instead
 * of `tp_eq', `tp_ne', `tp_lo', `tp_le', `tp_gr', `tp_ge'
 * @return: Dee_COMPARE_ERR: An error occurred.
 * @return: -1: `lhs < rhs'
 * @return: 0:  `lhs == rhs'
 * @return: 1:  `lhs > rhs' */
[[wunused]] int
tp_cmp->tp_compare([[nonnull]] DeeObject *lhs,
                   [[nonnull]] DeeObject *rhs)
%{using [tp_cmp->tp_eq, tp_cmp->tp_lo]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_lo, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_eq, tp_cmp->tp_le]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_le, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_eq, tp_cmp->tp_gr]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_gr, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_eq, tp_cmp->tp_ge]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ge, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_ne, tp_cmp->tp_lo]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_lo, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_ne, tp_cmp->tp_le]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_le, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Less */
	return 1;      /* Greater */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_ne, tp_cmp->tp_gr]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_gr, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_ne, tp_cmp->tp_ge]: {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(tp_cmp->tp_ge, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 1; /* Greater */
	return -1;    /* Less */
err:
	return Dee_COMPARE_ERR;
}}
%{using [tp_cmp->tp_lo, tp_cmp->tp_gr] = default__compare_eq__with__lo__and__gr}
%{using [tp_cmp->tp_le, tp_cmp->tp_ge] = default__compare_eq__with__le__and__ge}
= OPERATOR_LO;


/* Same as "tp_compare_eq", but shouldn't[1] throw `NotImplemented', `TypeError' or `ValueError'.
 * Instead of throwing these errors, this implementation should handle these errors by returning
 * either `-1' or `1' to indicate non-equality.
 *
 * [1] With "shouldn't" I mean *REALLY* shouldn't. As in: unless you *really* want it to throw
 *     one of those errors, you should either use API functions that never throw these errors,
 *     or add `DeeError_Catch()' calls to your function to catch those errors by returning either
 *     `-1' or `1' instead.
 *
 * !!! THIS OPERATOR CANNOT BE USED TO SUBSTITUTE "tp_compare_eq" !!!
 * -> Defining this operator but not defining "tp_compare_eq" is !NOT VALID!
 *    However, "tp_trycompare_eq" can ITSELF be substituted by "tp_compare_eq"
 *
 * @return: Dee_COMPARE_ERR: An error occurred.
 * @return: -1: `lhs != rhs'
 * @return: 0:  `lhs == rhs'
 * @return: 1:  `lhs != rhs' */
[[custom_unsupported_impl_name(default__trycompare_eq__unsupported)]]
[[wunused]] int
tp_cmp->tp_trycompare_eq([[nonnull]] DeeObject *lhs,
                         [[nonnull]] DeeObject *rhs)
%{using tp_cmp->tp_compare_eq: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare_eq, lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}} = OPERATOR_EQ;



/*[[[deemon
import * from deemon;
function gen(eq: string, cmp: string, iseq: bool) {
	local EQ = eq.upper();
	local _eq = iseq ? "_eq" : "";
	print("[[wunused]] DREF DeeObject *");
	print("tp_cmp->tp_", eq, "([[nonnull]] DeeObject *lhs,");
	print("              [[nonnull]] DeeObject *rhs)");
	print("%{class {");
	print("	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_", EQ, ", 1, &rhs);");
	print("}}");
	print("%{using tp_cmp->tp_compare", _eq, ": {");
	print("	int result = CALL_DEPENDENCY(tp_cmp->tp_compare", _eq, ", lhs, rhs);");
	print("	if unlikely(result == Dee_COMPARE_ERR)");
	print("		goto err;");
	print("	return_bool(result ", cmp, " 0);");
	print("err:");
	print("	return NULL;");
	print("}} = OPERATOR_", EQ, ";");
	print;
	print;
	print;
}
gen("eq", "==", true);
gen("ne", "!=", true);
gen("lo", "<", false);
gen("le", "<=", false);
gen("gr", ">", false);
gen("ge", ">=", false);
]]]*/
[[wunused]] DREF DeeObject *
tp_cmp->tp_eq([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_EQ, 1, &rhs);
}}
%{using tp_cmp->tp_compare_eq: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}} = OPERATOR_EQ;



[[wunused]] DREF DeeObject *
tp_cmp->tp_ne([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_NE, 1, &rhs);
}}
%{using tp_cmp->tp_compare_eq: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}} = OPERATOR_NE;



[[wunused]] DREF DeeObject *
tp_cmp->tp_lo([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_LO, 1, &rhs);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}} = OPERATOR_LO;



[[wunused]] DREF DeeObject *
tp_cmp->tp_le([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_LE, 1, &rhs);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}} = OPERATOR_LE;



[[wunused]] DREF DeeObject *
tp_cmp->tp_gr([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_GR, 1, &rhs);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}} = OPERATOR_GR;



[[wunused]] DREF DeeObject *
tp_cmp->tp_ge([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_GE, 1, &rhs);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}} = OPERATOR_GE;
/*[[[end]]]*/



} /* operator */
