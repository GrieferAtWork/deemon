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
/* deemon.Sequence.__seq_compare__()                                    */
/************************************************************************/
[[alias(Sequence.compare)]]
__seq_compare__(rhs:?X2?DSequence?S?O)->?Dint {
	int result = CALL_DEPENDENCY(seq_operator_compare, self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}


/************************************************************************/
/* deemon.Sequence.__seq_compare_eq__()                                 */
/************************************************************************/
[[alias(Sequence.equals)]]
__seq_compare_eq__(rhs:?X2?DSequence?S?O)->?X2?Dbool?Dint {
	int result = CALL_DEPENDENCY(seq_operator_compare_eq, self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}


/* NOTE: In compare (and possibly some other) method hints, we must not
 *       explicitly require stuff like seq_operator_foreach of the rhs-
 *       object. -- THAT WOULD BE WRONG!
 * Instead, we need to keep on using `DeeObject_Foreach(rhs)', and that
 * is good and correct here:
 * >> assert {10} == ({10, 10} as Set);
 * ^ Calling "Sequence.__foreach__({10, 10} as Set)" will by-pass the
 *   Set-case (as intended)
 *
 * This also needs to be done to prevent false positives in code like:
 * >> class MyClass { __seq_iter__() -> {10, 20}.operator iter(); }
 * >> assert {10,20} == (MyClass() as Sequence); // OK!
 * >> assert {10,20} == MyClass();               // This should throw "NotImplemented: operator iter" (but currently wouldn't)
 */



/*[[[deemon
import * from deemon;
function gen(isEq: bool) {
	local __ = isEq ? "  " : "";
	local ___ = isEq ? "   " : "";
	local _eq = isEq ? "_eq" : "";
	local eq = isEq ? "eq" : "";
	local EQ = isEq ? "EQ" : "";
	local SEQ_COMPARE_FOREACH_RESULT_EQUAL = isEq ? 'SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL'    : 'SEQ_COMPARE_FOREACH_RESULT_EQUAL';
	local SEQ_COMPARE_FOREACH_RESULT_LESS  = isEq ? 'SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL' : 'SEQ_COMPARE_FOREACH_RESULT_LESS';
	local SEQ_COMPARE_FOREACH_RESULT_ERROR = isEq ? 'SEQ_COMPAREEQ_FOREACH_RESULT_ERROR'    : 'SEQ_COMPARE_FOREACH_RESULT_ERROR';

	print('[[operator(Sequence: tp_cmp->tp_compare', _eq, ')]]');
	print('[[wunused]] int');
	print('__seq_compare', _eq, '__.seq_operator_compare', _eq, '([[nonnull]] DeeObject *lhs,');
	print('             ', ___, '                       ', ___, ' [[nonnull]] DeeObject *rhs)');
	print('%{unsupported(auto)}');
if (isEq) {
	print('%{$empty = "default__seq_operator_compare__empty"}');
} else {
	print('%{$empty = {');
	print('	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);');
	print('	if unlikely(rhs_nonempty < 0)');
	print('		goto err;');
	print('	return rhs_nonempty ? -1 : 0;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
}

if (isEq) {
	print('%{$with__seq_operator_eq = {');
	print('	int result;');
	print('	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	result = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(result < 0)');
	print('		goto err;');
	print('	return result ? 0 : 1;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_ne = {');
	print('	int result;');
	print('	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	result = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(result < 0)');
	print('		goto err;');
	print('	return result;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_lo__and__seq_operator__gr = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return -1; /' '* Different *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 1; /' '* Different *' '/');
	print('	return 0;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_le__and__seq_operator__ge = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return 1; /' '* Different *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return -1; /' '* Different *' '/');
	print('	return 0;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
} else {
	print('%{$with__seq_operator_eq__and__seq_operator__lo = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return -1; /' '* Less *' '/');
	print('	return 1;      /' '* Greater *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_eq__and__seq_operator__le = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return -1; /' '* Less *' '/');
	print('	return 1;      /' '* Greater *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_eq__and__seq_operator__gr = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 1; /' '* Greater *' '/');
	print('	return -1;    /' '* Less *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_eq__and__seq_operator__ge = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ge, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 1; /' '* Greater *' '/');
	print('	return -1;    /' '* Less *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_ne__and__seq_operator__lo = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return -1; /' '* Less *' '/');
	print('	return 1;      /' '* Greater *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_ne__and__seq_operator__le = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return -1; /' '* Less *' '/');
	print('	return 1;      /' '* Greater *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_ne__and__seq_operator__gr = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 1; /' '* Greater *' '/');
	print('	return -1;    /' '* Less *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_ne__and__seq_operator__ge = {');
	print('	int temp;');
	print('	DREF DeeObject *cmp_ob;');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (!temp)');
	print('		return 0; /' '* Equal *' '/');
	print('	cmp_ob = CALL_DEPENDENCY(seq_operator_ge, lhs, rhs);');
	print('	if unlikely(!cmp_ob)');
	print('		goto err;');
	print('	temp = DeeObject_BoolInherited(cmp_ob);');
	print('	if unlikely(temp < 0)');
	print('		goto err;');
	print('	if (temp)');
	print('		return 1; /' '* Greater *' '/');
	print('	return -1;    /' '* Less *' '/');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');
	print('%{$with__seq_operator_lo__and__seq_operator__gr = "default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr"}');
	print('%{$with__seq_operator_le__and__seq_operator__ge = "default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge"}');
}


	// With "seq_operator_foreach"
	print('%{$with__seq_operator_foreach = {');
	print('	Dee_ssize_t result;');
	print('	PRELOAD_DEPENDENCY(seq_operator_foreach)');
	print('	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);');
	print('	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);');
	print('	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		data.scf_sgi_osize = DeeObject_Size(rhs);');
	print('		if unlikely(data.scf_sgi_osize == (size_t)-1)');
	print('			goto err;');
	print('		data.scf_sgi_other          = rhs;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;');
	print('		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare', eq, '__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);');
	print('		if (result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ' && data.scf_sgi_oindex < data.scf_sgi_osize)');
	print('			result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		data.scf_sgi_osize = DeeObject_Size(rhs);');
	print('		if unlikely(data.scf_sgi_osize == (size_t)-1)');
	print('			goto err;');
	print('		data.scf_sgi_other          = rhs;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);');
	print('		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare', eq, '__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);');
	print('		if (result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ' && data.scf_sgi_oindex < data.scf_sgi_osize)');
	print('			result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		data.scf_sgi_osize = DeeObject_Size(rhs);');
	print('		if unlikely(data.scf_sgi_osize == (size_t)-1)');
	print('			goto err;');
	print('		data.scf_sgi_other          = rhs;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);');
	print('		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare', eq, '__lhs_foreach__rhs_size_and_getitem_index__cb, &data);');
	print('		if (result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ' && data.scf_sgi_oindex < data.scf_sgi_osize)');
	print('			result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {');
	print('		struct seq_compare_foreach__sizeob_and_getitem__data data;');
	print('		data.scf_sg_osize = DeeObject_SizeOb(rhs);');
	print('		if unlikely(!data.scf_sg_osize)');
	print('			goto err;');
	print('		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));');
	print('		if unlikely(!data.scf_sg_oindex) {');
	print('			result = ', SEQ_COMPARE_FOREACH_RESULT_ERROR, ';');
	print('		} else {');
	print('			data.scf_sg_other    = rhs;');
	print('			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;');
	print('			result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare', eq, '__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);');
	print('			if (result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ') {');
	print('				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);');
	print('				if unlikely(temp < 0) {');
	print('					result = ', SEQ_COMPARE_FOREACH_RESULT_ERROR, ';');
	print('				} else if (temp) {');
	print('					result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('				}');
	print('			}');
	print('			Dee_Decref(data.scf_sg_oindex);');
	print('		}');
	print('		Dee_Decref(data.scf_sg_osize);');
	print('	} else {');
	print('		DREF DeeObject *rhs_iter = DeeObject_Iter(rhs);');
	print('		if unlikely(!rhs_iter)');
	print('			goto err;');
	print('		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare', eq, '__lhs_foreach__rhs_iter__cb, rhs_iter);');
	print('		if (result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ') {');
	print('			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);');
	print('			Dee_Decref(rhs_iter);');
	print('			if unlikely(!next)');
	print('				goto err;');
	print('			if (next != ITER_DONE) {');
	print('				Dee_Decref(next);');
	print('				result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('			}');
	print('		} else {');
	print('			Dee_Decref(rhs_iter);');
	print('		}');
	print('	}');
if (isEq) {
	print('	ASSERT(result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||');
	print('	       result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||');
	print('	       result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);');
	print('	if unlikely(result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)');
	print('		goto err;');
	print('	if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL)');
	print('		return 0;');
	print('	return 1;');
} else {
	print('	ASSERT(result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||');
	print('	       result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||');
	print('	       result == SEQ_COMPARE_FOREACH_RESULT_LESS ||');
	print('	       result == SEQ_COMPARE_FOREACH_RESULT_GREATER);');
	print('	if unlikely(result == SEQ_COMPARE_FOREACH_RESULT_ERROR)');
	print('		goto err;');
	print('	if (result == SEQ_COMPARE_FOREACH_RESULT_LESS)');
	print('		return -1;');
	print('	if (result == SEQ_COMPARE_FOREACH_RESULT_GREATER)');
	print('		return 1;');
	print('	return 0; /' '* Equal *' '/');
}
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');




	// With "seq_operator_size" and "tp_getitem_index_fast"
	print('%{$with__seq_operator_size__and__operator_getitem_index_fast =');
	print('[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {');
	print('	int result;');
	print('	DeeNO_getitem_index_fast_t lhs_getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;');
	print('	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);');
	print('	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);');
	print('	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);');
	print('	if unlikely(lhs_size == (size_t)-1)');
	print('		goto err;');
if (isEq) {
	print('if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {');
	print('	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);');
	print('	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)');
	print('		return 1;');
	print('}');
}
	print('	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {');
	print('	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,');
	print('		                      ', __, '                                                                   rhs, rhs_size, rhs_getitem_index_fast);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {');
	print('	    DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,');
	print('		                      ', __, '                                                                 rhs, rhs_size, rhs_trygetitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {');
	print('	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,');
	print('		                      ', __, '                                                              rhs, rhs_size, rhs_getitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {');
	print('	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);');
	print('		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);');
	print('		if unlikely(!rhs_sizeob)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,');
	print('		                      ', __, '                                                          rhs, rhs_sizeob, rhs_getitem);');
	print('		Dee_Decref(rhs_sizeob);');
	print('	} else {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		Dee_ssize_t foreach_result;');
	print('		data.scf_sgi_other          = lhs;');
	print('		data.scf_sgi_osize          = lhs_size;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;');
	print('		foreach_result = (*rhs_foreach)(rhs, &seq_compare', eq, '__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);');
if (isEq) {
	print('		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);');
	print('		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&');
	print('		    data.scf_sgi_oindex >= data.scf_sgi_osize) {');
	print('			result = 0;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
} else {
	print('		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);');
	print('		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {');
	print('			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;');
	print('		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {');
	print('			result = -1;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
}
	print('	}');
	print('	return result;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');




	// With "seq_operator_size" and "seq_operator_trygetitem_index"
	print('%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {');
	print('	int result;');
	print('	DeeMH_seq_operator_trygetitem_index_t lhs_seq_operator_trygetitem_index = REQUIRE_DEPENDENCY(seq_operator_trygetitem_index);');
	print('	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);');
	print('	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);');
	print('	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);');
	print('	if unlikely(lhs_size == (size_t)-1)');
	print('		goto err;');
if (isEq) {
	print('if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {');
	print('	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);');
	print('	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)');
	print('		return 1;');
	print('}');
}
	print('	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {');
	print('		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,');
	print('		                      ', __, '                                                                 rhs, rhs_size, rhs_getitem_index_fast);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {');
	print('		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,');
	print('		                      ', __, '                                                               rhs, rhs_size, rhs_trygetitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {');
	print('		DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,');
	print('		                      ', __, '                                                            rhs, rhs_size, rhs_getitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {');
	print('	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);');
	print('		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);');
	print('		if unlikely(!rhs_sizeob)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,');
	print('		                      ', __, '                                                        rhs, rhs_sizeob, rhs_getitem);');
	print('		Dee_Decref(rhs_sizeob);');
	print('	} else {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		Dee_ssize_t foreach_result;');
	print('		data.scf_sgi_other          = lhs;');
	print('		data.scf_sgi_osize          = lhs_size;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;');
	print('		foreach_result = (*rhs_foreach)(rhs, &seq_compare', eq, '__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);');
if (isEq) {
	print('		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);');
	print('		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&');
	print('		    data.scf_sgi_oindex >= data.scf_sgi_osize) {');
	print('			result = 0;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
} else {
	print('		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);');
	print('		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {');
	print('			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;');
	print('		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {');
	print('			result = -1;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
}
	print('	}');
	print('	return result;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');




	// With "seq_operator_size" and "seq_operator_getitem_index"
	print('%{$with__seq_operator_size__and__seq_operator_getitem_index = {');
	print('	int result;');
	print('	DeeMH_seq_operator_getitem_index_t lhs_seq_operator_getitem_index = REQUIRE_DEPENDENCY(seq_operator_getitem_index);');
	print('	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);');
	print('	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);');
	print('	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);');
	print('	if unlikely(lhs_size == (size_t)-1)');
	print('		goto err;');
if (isEq) {
	print('if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {');
	print('	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);');
	print('	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)');
	print('		return 1;');
	print('}');
}
	print('	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {');
	print('	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,');
	print('		                      ', __, '                                                                 rhs, rhs_size, rhs_getitem_index_fast);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {');
	print('		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,');
	print('		                      ', __, '                                                            rhs, rhs_size, rhs_trygetitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {');
	print('	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,');
	print('		                      ', __, '                                                            rhs, rhs_size, rhs_getitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {');
	print('	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);');
	print('		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);');
	print('		if unlikely(!rhs_sizeob)');
	print('			goto err;');
	print('		result = seq_docompare', eq, '__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,');
	print('		                      ', __, '                                                        rhs, rhs_sizeob, rhs_getitem);');
	print('		Dee_Decref(rhs_sizeob);');
	print('	} else {');
	print('		struct seq_compareforeach__size_and_getitem_index__data data;');
	print('		Dee_ssize_t foreach_result;');
	print('		data.scf_sgi_other          = lhs;');
	print('		data.scf_sgi_osize          = lhs_size;');
	print('		data.scf_sgi_oindex         = 0;');
	print('		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;');
	print('		foreach_result = (*rhs_foreach)(rhs, &seq_compare', eq, '__lhs_size_and_getitem_index__rhs_foreach__cb, &data);');
if (isEq) {
	print('		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);');
	print('		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL &&');
	print('		    data.scf_sgi_oindex >= data.scf_sgi_osize) {');
	print('			result = 0;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
} else {
	print('		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);');
	print('		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)');
	print('			goto err;');
	print('		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {');
	print('			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;');
	print('		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {');
	print('			result = -1;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
}
	print('	}');
	print('	return result;');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');




	// With "seq_operator_sizeob" and "seq_operator_getitem"
	print('%{$with__seq_operator_sizeob__and__seq_operator_getitem = {');
	print('	int result;');
	print('	PRELOAD_DEPENDENCY(seq_operator_getitem)');
	print('	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);');
	print('	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);');
	print('	DREF DeeObject *lhs_sizeob = CALL_DEPENDENCY(seq_operator_sizeob, lhs);');
	print('	if unlikely(!lhs_sizeob)');
	print('		goto err;');
	print('	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err_lhs_sizeob;');
	print('		result = seq_docompare', eq, '__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,');
	print('		                      ', __, '                                                          rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err_lhs_sizeob;');
	print('		result = seq_docompare', eq, '__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,');
	print('		                      ', __, '                                                        rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {');
	print('		size_t rhs_size = DeeObject_Size(rhs);');
	print('		if unlikely(rhs_size == (size_t)-1)');
	print('			goto err_lhs_sizeob;');
	print('		result = seq_docompare', eq, '__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,');
	print('		                      ', __, '                                                     rhs_size, tp_rhs->tp_seq->tp_getitem_index);');
	print('	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {');
	print('		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);');
	print('		if unlikely(!rhs_sizeob)');
	print('			goto err_lhs_sizeob;');
	print('		result = seq_docompare', eq, '__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,');
	print('		                      ', __, '                                                 rhs_sizeob, tp_rhs->tp_seq->tp_getitem);');
	print('		Dee_Decref(rhs_sizeob);');
	print('	} else {');
	print('		Dee_ssize_t foreach_result;');
	print('		DREF DeeObject *lhs_indexob;');
	print('		struct seq_compare_foreach__sizeob_and_getitem__data data;');
	print('		lhs_indexob = DeeObject_NewDefault(Dee_TYPE(lhs_sizeob));');
	print('		if unlikely(!lhs_indexob)');
	print('			goto err_lhs_sizeob;');
	print('		data.scf_sg_other    = lhs;');
	print('		data.scf_sg_osize    = lhs_sizeob;');
	print('		data.scf_sg_oindex   = lhs_indexob;');
	print('		data.scf_sg_ogetitem = REQUIRE_DEPENDENCY(seq_operator_getitem);');
	print('		foreach_result = (*rhs_foreach)(rhs, &seq_compare', eq, '__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);');
	print('		ASSERT(data.scf_sg_osize == lhs_sizeob);');
	print('		if (foreach_result == ', SEQ_COMPARE_FOREACH_RESULT_EQUAL, ') {');
	print('			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);');
	print('			if unlikely(temp < 0) {');
	print('				foreach_result = ', SEQ_COMPARE_FOREACH_RESULT_ERROR, ';');
	print('			} else if (temp) {');
	print('				foreach_result = ', SEQ_COMPARE_FOREACH_RESULT_LESS, ';');
	print('			}');
	print('		}');
	print('		Dee_Decref(data.scf_sg_oindex);');
if (isEq) {
	print('		ASSERT(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL);');
	print('		if unlikely(foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_ERROR)');
	print('			goto err_lhs_sizeob;');
	print('		result = foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL ? 0 : 1;');
} else {
	print('		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||');
	print('		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);');
	print('		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)');
	print('			goto err_lhs_sizeob;');
	print('		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {');
	print('			result = 0;');
	print('		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {');
	print('			result = -1;');
	print('		} else {');
	print('			result = 1;');
	print('		}');
}
	print('	}');
	print('	Dee_Decref(lhs_sizeob);');
	print('	return result;');
	print('err_lhs_sizeob:');
	print('	Dee_Decref(lhs_sizeob);');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}}');



	// Callattr impls
	print('{');
	print('	int result;');
	print('	DREF DeeObject *resultob;');
	print('	resultob = LOCAL_CALLATTR(lhs, 1, &rhs);');
	print('	if unlikely(!resultob)');
	print('		goto err;');
if (isEq) {
	print('	if (DeeBool_Check(resultob)) {');
	print('		Dee_DecrefNokill(resultob);');
	print('		return DeeBool_IsTrue(resultob) ? 0 : 1;');
	print('	}');
}
	print('	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))');
	print('		goto err_resultob;');
	print('	if (DeeInt_IsZero(resultob)) {');
	print('		result = 0;');
if (!isEq) {
	print('	} else if (DeeInt_IsNeg(resultob)) {');
	print('		result = -1;');
}
	print('	} else {');
	print('		result = 1;');
	print('	}');
	print('	Dee_Decref(resultob);');
	print('	return result;');
	print('err_resultob:');
	print('	Dee_Decref(resultob);');
	print('err:');
	print('	return Dee_COMPARE_ERR;');
	print('}');

	print;
	print;
	print;
}

gen(false);
gen(true);
]]]*/
[[operator(Sequence: tp_cmp->tp_compare)]]
[[wunused]] int
__seq_compare__.seq_operator_compare([[nonnull]] DeeObject *lhs,
                                     [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? -1 : 0;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_eq__and__seq_operator__lo = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);
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
%{$with__seq_operator_eq__and__seq_operator__le = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);
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
%{$with__seq_operator_eq__and__seq_operator__gr = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);
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
%{$with__seq_operator_eq__and__seq_operator__ge = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_ge, lhs, rhs);
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
%{$with__seq_operator_ne__and__seq_operator__lo = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);
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
%{$with__seq_operator_ne__and__seq_operator__le = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);
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
%{$with__seq_operator_ne__and__seq_operator__gr = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);
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
%{$with__seq_operator_ne__and__seq_operator__ge = {
	int temp;
	DREF DeeObject *cmp_ob;
	cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 0; /* Equal */
	cmp_ob = CALL_DEPENDENCY(seq_operator_ge, lhs, rhs);
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
%{$with__seq_operator_lo__and__seq_operator__gr = "default__seq_operator_compare_eq__with__seq_operator_lo__and__seq_operator__gr"}
%{$with__seq_operator_le__and__seq_operator__ge = "default__seq_operator_compare_eq__with__seq_operator_le__and__seq_operator__ge"}
%{$with__seq_operator_foreach = {
	Dee_ssize_t result;
	PRELOAD_DEPENDENCY(seq_operator_foreach)
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPARE_FOREACH_RESULT_LESS;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = DeeObject_SizeOb(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				if unlikely(temp < 0) {
					result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPARE_FOREACH_RESULT_LESS;
				}
			}
			Dee_Decref(data.scf_sg_oindex);
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compare__lhs_foreach__rhs_iter__cb, rhs_iter);
		if (result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			DREF DeeObject *next = DeeObject_IterNext(rhs_iter);
			Dee_Decref(rhs_iter);
			if unlikely(!next)
				goto err;
			if (next != ITER_DONE) {
				Dee_Decref(next);
				result = SEQ_COMPARE_FOREACH_RESULT_LESS;
			}
		} else {
			Dee_Decref(rhs_iter);
		}
	}
	ASSERT(result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
	       result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
	       result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
	       result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
	if unlikely(result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
		goto err;
	if (result == SEQ_COMPARE_FOREACH_RESULT_LESS)
		return -1;
	if (result == SEQ_COMPARE_FOREACH_RESULT_GREATER)
		return 1;
	return 0; /* Equal */
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	int result;
	DeeNO_getitem_index_fast_t lhs_getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                       rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	int result;
	DeeMH_seq_operator_trygetitem_index_t lhs_seq_operator_trygetitem_index = REQUIRE_DEPENDENCY(seq_operator_trygetitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                     rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                  rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                              rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	int result;
	DeeMH_seq_operator_getitem_index_t lhs_seq_operator_getitem_index = REQUIRE_DEPENDENCY(seq_operator_getitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                       rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                  rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                              rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = data.scf_sgi_oindex >= data.scf_sgi_osize ? 0 : 1;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	int result;
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	DREF DeeObject *lhs_sizeob = CALL_DEPENDENCY(seq_operator_sizeob, lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                              rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                           rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompare__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
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
		data.scf_sg_ogetitem = REQUIRE_DEPENDENCY(seq_operator_getitem);
		foreach_result = (*rhs_foreach)(rhs, &seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				foreach_result = SEQ_COMPARE_FOREACH_RESULT_ERROR;
			} else if (temp) {
				foreach_result = SEQ_COMPARE_FOREACH_RESULT_LESS;
			}
		}
		Dee_Decref(data.scf_sg_oindex);
		ASSERT(foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS ||
		       foreach_result == SEQ_COMPARE_FOREACH_RESULT_GREATER);
		if unlikely(foreach_result == SEQ_COMPARE_FOREACH_RESULT_ERROR)
			goto err_lhs_sizeob;
		if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_EQUAL) {
			result = 0;
		} else if (foreach_result == SEQ_COMPARE_FOREACH_RESULT_LESS) {
			result = -1;
		} else {
			result = 1;
		}
	}
	Dee_Decref(lhs_sizeob);
	return result;
err_lhs_sizeob:
	Dee_Decref(lhs_sizeob);
err:
	return Dee_COMPARE_ERR;
}}
{
	int result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else if (DeeInt_IsNeg(resultob)) {
		result = -1;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
err:
	return Dee_COMPARE_ERR;
}



[[operator(Sequence: tp_cmp->tp_compare_eq)]]
[[wunused]] int
__seq_compare_eq__.seq_operator_compare_eq([[nonnull]] DeeObject *lhs,
                                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__seq_operator_compare__empty"}
%{$with__seq_operator_eq = {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_ne = {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_lo__and__seq_operator__gr = {
	int temp;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return -1; /* Different */
	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);
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
%{$with__seq_operator_le__and__seq_operator__ge = {
	int temp;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	temp = DeeObject_BoolInherited(cmp_ob);
	if unlikely(temp < 0)
		goto err;
	if (!temp)
		return 1; /* Different */
	cmp_ob = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);
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
}}
%{$with__seq_operator_foreach = {
	Dee_ssize_t result;
	PRELOAD_DEPENDENCY(seq_operator_foreach)
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = tp_rhs->tp_seq->tp_getitem_index_fast;
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		struct seq_compareforeach__size_and_getitem_index__data data;
		data.scf_sgi_osize = DeeObject_Size(rhs);
		if unlikely(data.scf_sgi_osize == (size_t)-1)
			goto err;
		data.scf_sgi_other          = rhs;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb, &data);
		if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL && data.scf_sgi_oindex < data.scf_sgi_osize)
			result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		struct seq_compare_foreach__sizeob_and_getitem__data data;
		data.scf_sg_osize = DeeObject_SizeOb(rhs);
		if unlikely(!data.scf_sg_osize)
			goto err;
		data.scf_sg_oindex = DeeObject_NewDefault(Dee_TYPE(data.scf_sg_osize));
		if unlikely(!data.scf_sg_oindex) {
			result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
		} else {
			data.scf_sg_other    = rhs;
			data.scf_sg_ogetitem = tp_rhs->tp_seq->tp_getitem;
			result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb, &data);
			if (result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
				int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, data.scf_sg_osize);
				if unlikely(temp < 0) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
				} else if (temp) {
					result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
				}
			}
			Dee_Decref(data.scf_sg_oindex);
		}
		Dee_Decref(data.scf_sg_osize);
	} else {
		DREF DeeObject *rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			goto err;
		result = CALL_DEPENDENCY(seq_operator_foreach, lhs, &seq_compareeq__lhs_foreach__rhs_iter__cb, rhs_iter);
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
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	int result;
	DeeNO_getitem_index_fast_t lhs_getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                           rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
	    DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                         rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                      rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_getitem_index_fast,
		                                                                                  rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_getitem_index_fast;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index_fast__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	int result;
	DeeMH_seq_operator_trygetitem_index_t lhs_seq_operator_trygetitem_index = REQUIRE_DEPENDENCY(seq_operator_trygetitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                       rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_trygetitem_index,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_trygetitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	int result;
	DeeMH_seq_operator_getitem_index_t lhs_seq_operator_getitem_index = REQUIRE_DEPENDENCY(seq_operator_getitem_index);
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	size_t lhs_size = CALL_DEPENDENCY(seq_operator_size, lhs);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
if (tp_rhs->tp_seq && tp_rhs->tp_seq->tp_size_fast != NULL) {
	size_t rhs_sizefast = (*tp_rhs->tp_seq->tp_size_fast)(rhs);
	if (lhs_size != rhs_sizefast && rhs_sizefast != (size_t)-1)
		return 1;
}
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
	    DeeNO_getitem_index_fast_t rhs_getitem_index_fast = tp_rhs->tp_seq->tp_getitem_index_fast;
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                         rhs, rhs_size, rhs_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		DeeNO_trygetitem_index_t rhs_trygetitem_index = DeeType_RequireNativeOperator(tp_rhs, trygetitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
	    DeeNO_getitem_index_t rhs_getitem_index = DeeType_RequireNativeOperator(tp_rhs, getitem_index);
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                    rhs, rhs_size, rhs_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
	    DeeNO_getitem_t rhs_getitem = DeeType_RequireNativeOperator(tp_rhs, getitem);
		DREF DeeObject *rhs_sizeob = DeeObject_SizeOb(rhs);
		if unlikely(!rhs_sizeob)
			goto err;
		result = seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(lhs, lhs_size, lhs_seq_operator_getitem_index,
		                                                                                rhs, rhs_sizeob, rhs_getitem);
		Dee_Decref(rhs_sizeob);
	} else {
		struct seq_compareforeach__size_and_getitem_index__data data;
		Dee_ssize_t foreach_result;
		data.scf_sgi_other          = lhs;
		data.scf_sgi_osize          = lhs_size;
		data.scf_sgi_oindex         = 0;
		data.scf_sgi_ogetitem_index = lhs_seq_operator_getitem_index;
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb, &data);
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
err:
	return Dee_COMPARE_ERR;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	int result;
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	DeeNO_foreach_t rhs_foreach = DeeType_RequireNativeOperator(tp_rhs, foreach);
	DREF DeeObject *lhs_sizeob = CALL_DEPENDENCY(seq_operator_sizeob, lhs);
	if unlikely(!lhs_sizeob)
		goto err;
	if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                                  rhs_size, tp_rhs->tp_seq->tp_getitem_index_fast);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                                rhs_size, tp_rhs->tp_seq->tp_trygetitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index) {
		size_t rhs_size = DeeObject_Size(rhs);
		if unlikely(rhs_size == (size_t)-1)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
		                                                                             rhs_size, tp_rhs->tp_seq->tp_getitem_index);
	} else if (rhs_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem) {
		DREF DeeObject *rhs_sizeob = (*tp_rhs->tp_seq->tp_sizeob)(rhs);
		if unlikely(!rhs_sizeob)
			goto err_lhs_sizeob;
		result = seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(lhs, lhs_sizeob, REQUIRE_DEPENDENCY(seq_operator_getitem), rhs,
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
		data.scf_sg_ogetitem = REQUIRE_DEPENDENCY(seq_operator_getitem);
		foreach_result = (*rhs_foreach)(rhs, &seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb, &data);
		ASSERT(data.scf_sg_osize == lhs_sizeob);
		if (foreach_result == SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL) {
			int temp = DeeObject_CmpLoAsBool(data.scf_sg_oindex, lhs_sizeob);
			if unlikely(temp < 0) {
				foreach_result = SEQ_COMPAREEQ_FOREACH_RESULT_ERROR;
			} else if (temp) {
				foreach_result = SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL;
			}
		}
		Dee_Decref(data.scf_sg_oindex);
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
err:
	return Dee_COMPARE_ERR;
}}
{
	int result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(lhs, 1, &rhs);
	if unlikely(!resultob)
		goto err;
	if (DeeBool_Check(resultob)) {
		Dee_DecrefNokill(resultob);
		return DeeBool_IsTrue(resultob) ? 0 : 1;
	}
	if (DeeObject_AssertTypeExact(resultob, &DeeInt_Type))
		goto err_resultob;
	if (DeeInt_IsZero(resultob)) {
		result = 0;
	} else {
		result = 1;
	}
	Dee_Decref(resultob);
	return result;
err_resultob:
	Dee_Decref(resultob);
err:
	return Dee_COMPARE_ERR;
}
/*[[[end]]]*/





seq_operator_compare = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size;
	if (REQUIRE_NODEFAULT(seq_operator_eq)) {
		if (REQUIRE_NODEFAULT(seq_operator_lo))
			return &$with__seq_operator_eq__and__seq_operator__lo;
		if (REQUIRE_NODEFAULT(seq_operator_le))
			return &$with__seq_operator_eq__and__seq_operator__le;
		if (REQUIRE_NODEFAULT(seq_operator_gr))
			return &$with__seq_operator_eq__and__seq_operator__gr;
		if (REQUIRE_NODEFAULT(seq_operator_ge))
			return &$with__seq_operator_eq__and__seq_operator__ge;
	} else if (REQUIRE_NODEFAULT(seq_operator_ne)) {
		if (REQUIRE_NODEFAULT(seq_operator_lo))
			return &$with__seq_operator_ne__and__seq_operator__lo;
		if (REQUIRE_NODEFAULT(seq_operator_le))
			return &$with__seq_operator_ne__and__seq_operator__le;
		if (REQUIRE_NODEFAULT(seq_operator_gr))
			return &$with__seq_operator_ne__and__seq_operator__gr;
		if (REQUIRE_NODEFAULT(seq_operator_ge))
			return &$with__seq_operator_ne__and__seq_operator__ge;
	} else {
		if (REQUIRE_NODEFAULT(seq_operator_lo) && REQUIRE_NODEFAULT(seq_operator_gr))
			return &$with__seq_operator_lo__and__seq_operator__gr;
		if (REQUIRE_NODEFAULT(seq_operator_le) && REQUIRE_NODEFAULT(seq_operator_ge))
			return &$with__seq_operator_le__and__seq_operator__ge;
	}

	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index) {
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
				return &$empty;
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
				return &$with__seq_operator_foreach;
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
				if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob ||
					REQUIRE(seq_operator_getitem_index) == &default__seq_operator_getitem_index__with__seq_operator_getitem)
					return &$with__seq_operator_sizeob__and__seq_operator_getitem;
				return &$with__seq_operator_size__and__seq_operator_getitem_index;
			}
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
	}

	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};



seq_operator_compare_eq = {
	DeeMH_seq_operator_compare_t seq_operator_compare;
	if (REQUIRE_NODEFAULT(seq_operator_eq))
		return &$with__seq_operator_eq;
	if (REQUIRE_NODEFAULT(seq_operator_ne))
		return &$with__seq_operator_ne;
	seq_operator_compare = REQUIRE(seq_operator_compare);
	/*if (seq_operator_compare == &default__seq_operator_compare__empty)
		return &$empty;*/ /* Already optimally handled by fallback below */
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_compare)
		return seq_operator_compare; /* Binary-compatible! */
};










[[operator(Sequence: tp_cmp->tp_trycompare_eq)]]
[[wunused]] int
__seq_compare_eq__.seq_operator_trycompare_eq([[nonnull]] DeeObject *lhs,
                                              [[nonnull]] DeeObject *rhs)
%{unsupported({
	return 1;
})}
%{$empty = {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return rhs_nonempty ? 0 : 1 /*or: -1*/;
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return -1;
	return Dee_COMPARE_ERR;
}}
%{using seq_operator_compare_eq: {
	int result = CALL_DEPENDENCY(seq_operator_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}} = $with__seq_operator_compare_eq;

seq_operator_trycompare_eq = {
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq = REQUIRE(seq_operator_compare_eq);
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
		return &$empty;
	if (seq_operator_compare_eq)
		return &$with__seq_operator_compare_eq;
};
