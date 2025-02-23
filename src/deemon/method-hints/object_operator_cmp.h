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
/* deemon.Object.operator <=> ()                                        */
/************************************************************************/

operator {

%[define(DEFINE_DeeType_HasBaseForCompare =
#ifndef DeeType_HasBaseForCompare
#define DeeType_HasBaseForCompare(self) \
	(DeeType_Base(self) && DeeType_Base(self) != &DeeObject_Type)
#endif /* !DeeType_HasBaseForCompare */
)]

%[define(DEFINE_impl_instance_builtin_compare =
#ifndef DEFINED_impl_instance_builtin_compare
#define DEFINED_impl_instance_builtin_compare
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
impl_instance_builtin_compare(DeeTypeObject *tp_self,
                              DeeObject *self,
                              DeeObject *other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	Dee_instance_desc_lock_read(instance);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			if (!lhs_val || !rhs_val) {
				Dee_instance_desc_lock_endread(instance);
				return lhs_val ? 1 : -1; /* Different NULL values. */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);
			Dee_instance_desc_lock_endread(instance);

			/* Compare the two members. */
			temp = DeeObject_Compare(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp != 0)
				return temp; /* Error, or non-equal */
			Dee_instance_desc_lock_read(instance);
		}
	}
	Dee_instance_desc_lock_endread(instance);
	return 0; /* All elements are equal */
}
#endif /* !DEFINED_impl_instance_builtin_compare */
)]

%[define(DEFINE_impl_instance_builtin_compare_eq =
#ifndef DEFINED_impl_instance_builtin_compare_eq
#define DEFINED_impl_instance_builtin_compare_eq
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
impl_instance_builtin_compare_eq(DeeTypeObject *tp_self,
                                 DeeObject *self,
                                 DeeObject *other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	Dee_instance_desc_lock_read(instance);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			if (!lhs_val || !rhs_val) {
				Dee_instance_desc_lock_endread(instance);
				return 1; /* Different NULL values. */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);
			Dee_instance_desc_lock_endread(instance);

			/* Compare the two members. */
			temp = DeeObject_TryCompareEq(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp != 0)
				return temp; /* Error, or non-equal */
			Dee_instance_desc_lock_read(instance);
		}
	}
	Dee_instance_desc_lock_endread(instance);
	return 0; /* All elements are equal */
}
#endif /* !DEFINED_impl_instance_builtin_compare_eq */
)]

%[define(DEFINE_impl_instance_builtin_le =
#ifndef DEFINED_impl_instance_builtin_le
#define DEFINED_impl_instance_builtin_le
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
impl_instance_builtin_le(DeeTypeObject *tp_self,
                         DeeObject *self,
                         DeeObject *other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	Dee_instance_desc_lock_read(instance);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			size_t j;
			if (!lhs_val || !rhs_val) {
				Dee_instance_desc_lock_endread(instance);
				/* Different NULL values. */
				return !lhs_val ? 1 : /* NULL <= *        --> true */
				       0;             /* NON_NULL <= NULL --> false */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);

			/* Check if this is the last member. */
			for (j = i; j < size; ++j) {
				if (instance->id_vtab[j] ||
				    other_instance->id_vtab[j])
					goto non_last_member;
			}

			/* Last member! */
			Dee_instance_desc_lock_endread(instance);
			temp = DeeObject_CmpLeAsBool(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			return temp;
non_last_member:
			Dee_instance_desc_lock_endread(instance);

			/* Compare the two members. */
			temp = DeeObject_CmpLoAsBool(lhs_val, rhs_val);
			if (temp != 0) {
				Dee_Decref(rhs_val);
				Dee_Decref(lhs_val);
				return temp; /* Error, or lower */
			}
			temp = DeeObject_TryCmpEqAsBool(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp <= 0)
				return temp; /* Error, or non-equal */
			Dee_instance_desc_lock_read(instance);
		}
	}
	Dee_instance_desc_lock_endread(instance);
	return 1; /* All elements are equal */
}
#endif /* !DEFINED_impl_instance_builtin_le */
)]



/* Same as "tp_compare", but only needs to support equal/not-equal compare:
 * @return: Dee_COMPARE_ERR: An error occurred.
 * @return: -1: `lhs != rhs'
 * @return: 0:  `lhs == rhs'
 * @return: 1:  `lhs != rhs' */
[[export("DeeObject_{|T}CompareEq")]]
[[wunused]] int
tp_cmp->tp_compare_eq([[nonnull]] DeeObject *lhs,
                      [[nonnull]] DeeObject *rhs)
%{class using []:
	[[prefix(DEFINE_DeeType_HasBaseForCompare)]]
	[[prefix(DEFINE_impl_instance_builtin_compare_eq)]]
{
	if (DeeObject_AssertImplements(rhs, THIS_TYPE))
		goto err;

	/* Compare the underlying objects. */
	if (DeeType_HasBaseForCompare(THIS_TYPE)) {
		int result = DeeObject_TCompareEq(DeeType_Base(THIS_TYPE), lhs, rhs);
		if (result != 0)
			return result;
	}
	return impl_instance_builtin_compare_eq(THIS_TYPE, lhs, rhs);
err:
	return Dee_COMPARE_ERR;
}}
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
[[export("DeeObject_{|T}Compare")]]
[[wunused]] int
tp_cmp->tp_compare([[nonnull]] DeeObject *lhs,
                   [[nonnull]] DeeObject *rhs)
%{class using []:
	[[prefix(DEFINE_DeeType_HasBaseForCompare)]]
	[[prefix(DEFINE_impl_instance_builtin_compare)]]
{
	if (DeeObject_AssertImplements(rhs, THIS_TYPE))
		goto err;

	/* Compare the underlying objects. */
	if (DeeType_HasBaseForCompare(THIS_TYPE)) {
		int result = DeeObject_TCompare(DeeType_Base(THIS_TYPE), lhs, rhs);
		if (result != 0)
			return result;
	}
	return impl_instance_builtin_compare(THIS_TYPE, lhs, rhs);
err:
	return Dee_COMPARE_ERR;
}}
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
[[export("DeeObject_{|T}TryCompareEq")]]
[[custom_unsupported_impl_name(default__trycompare_eq__unsupported)]]
[[wunused]] int
tp_cmp->tp_trycompare_eq([[nonnull]] DeeObject *lhs,
                         [[nonnull]] DeeObject *rhs)
%{class using []:
	[[prefix(DEFINE_DeeType_HasBaseForCompare)]]
	[[prefix(DEFINE_impl_instance_builtin_compare_eq)]]
{
	if (!DeeObject_Implements(rhs, THIS_TYPE))
		return 1;

	/* Compare the underlying objects. */
	if (DeeType_HasBaseForCompare(THIS_TYPE)) {
		int result = DeeObject_TTryCompareEq(DeeType_Base(THIS_TYPE), lhs, rhs);
		if (result != 0)
			return result;
	}
	return impl_instance_builtin_compare_eq(THIS_TYPE, lhs, rhs);
}}
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


%[define(DEFINE_xinvoke_not =
#ifndef DEFINED_xinvoke_not
#define DEFINED_xinvoke_not
PRIVATE WUNUSED DREF DeeObject *DCALL
xinvoke_not(/*[0..1],inherit(always)*/ DREF DeeObject *ob) {
	if (ob) {
		int temp = DeeObject_BoolInherited(ob);
		if likely(temp >= 0) {
			ob = DeeBool_For(!temp);
			Dee_Incref(ob);
		} else {
			ob = NULL;
		}
	}
	return ob;
}
#endif /* !DEFINED_xinvoke_not */
)]

/*[[[deemon
import * from deemon;
function gen(eq: string, ne: string, cmp: string, iseq: bool) {
	local Eq = eq.capitalize();
	local EQ = eq.upper();
	local _eq = iseq ? "_eq" : "";
	print('[[export("DeeObject_{|T}Cmp', Eq, '")]]');
	print("[[wunused]] DREF DeeObject *");
	print("tp_cmp->tp_", eq, "([[nonnull]] DeeObject *lhs,");
	print("              [[nonnull]] DeeObject *rhs)");
	print("%{class using OPERATOR_", EQ, ": {");
	print("	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_", EQ, ", 1, &rhs);");
	print("}}");
	print("%{using tp_cmp->tp_", ne, ": [[prefix(DEFINE_xinvoke_not)]] {");
	print("	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_", ne, ", lhs, rhs);");
	print("	return xinvoke_not(result);");
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
gen("eq", "ne", "==", true);
gen("ne", "eq", "!=", true);
gen("lo", "ge", "<", false);
gen("le", "gr", "<=", false);
gen("gr", "le", ">", false);
gen("ge", "lo", ">=", false);
]]]*/
[[export("DeeObject_{|T}CmpEq")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_eq([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_EQ: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_EQ, 1, &rhs);
}}
%{using tp_cmp->tp_ne: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_ne, lhs, rhs);
	return xinvoke_not(result);
}}
%{using tp_cmp->tp_compare_eq: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}} = OPERATOR_EQ;



[[export("DeeObject_{|T}CmpNe")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_ne([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_NE: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_NE, 1, &rhs);
}}
%{using tp_cmp->tp_eq: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_eq, lhs, rhs);
	return xinvoke_not(result);
}}
%{using tp_cmp->tp_compare_eq: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}} = OPERATOR_NE;



[[export("DeeObject_{|T}CmpLo")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_lo([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_LO: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_LO, 1, &rhs);
}}
%{using tp_cmp->tp_ge: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_ge, lhs, rhs);
	return xinvoke_not(result);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}} = OPERATOR_LO;



[[export("DeeObject_{|T}CmpLe")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_le([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_LE: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_LE, 1, &rhs);
}}
%{using tp_cmp->tp_gr: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_gr, lhs, rhs);
	return xinvoke_not(result);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}} = OPERATOR_LE;



[[export("DeeObject_{|T}CmpGr")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_gr([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_GR: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_GR, 1, &rhs);
}}
%{using tp_cmp->tp_le: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_le, lhs, rhs);
	return xinvoke_not(result);
}}
%{using tp_cmp->tp_compare: {
	int result = CALL_DEPENDENCY(tp_cmp->tp_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}} = OPERATOR_GR;



[[export("DeeObject_{|T}CmpGe")]]
[[wunused]] DREF DeeObject *
tp_cmp->tp_ge([[nonnull]] DeeObject *lhs,
              [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_GE: {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_GE, 1, &rhs);
}}
%{using tp_cmp->tp_lo: [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_cmp->tp_lo, lhs, rhs);
	return xinvoke_not(result);
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
