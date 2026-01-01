/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

operator {

/*[[[deemon
import * from deemon;

function unary(neg: string) {
	local NEG = neg.upper();
	local Neg = neg.capitalize();

	print('[[export("DeeObject_{|T}', Neg, '")]]');
	print("[[wunused]] DREF DeeObject *");
	print("tp_math->tp_", neg, "([[nonnull]] DeeObject *self)");
	print("%{class using OPERATOR_", NEG, ": {");
	print("	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_", NEG, ");");
	print("}} = OPERATOR_", NEG, ";");
	print;
}

function binaryMath(add: string) {
	local ADD = add.upper();
	local Add = add.capitalize();

	print('[[export("DeeObject_{|T}', Add, '")]]');
	print("[[wunused]] DREF DeeObject *");
	print("tp_math->tp_", add, "([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class using OPERATOR_", ADD, ": {");
	print("	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_", ADD, ", rhs);");
	print("}} = OPERATOR_", ADD, ";");
	print;
	print('[[export("DeeObject_{|T}Inplace', Add, '")]]');
	print("[[tp_self(Dee_TYPE(*p_lhs))]]");
	print("[[wunused]] int");
	print("tp_math->tp_inplace_", add, "([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class using OPERATOR_INPLACE_", ADD, ": {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_", ADD, ", rhs);");
	print("	Dee_Decref_unlikely(*p_lhs);");
	print("	*p_lhs = result; /" "* Inherit reference *" "/");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}}");
	print("%{using tp_math->tp_", add, ": {");
	print("	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_", add, ", *p_lhs, rhs);");
	print("	if unlikely(!result)");
	print("		goto err;");
	print("	Dee_Decref_unlikely(*p_lhs);");
	print("	*p_lhs = result; /" "* Inherit reference *" "/");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}} = OPERATOR_INPLACE_", ADD, ";");
	print;
}

function inplaceUnary(prefix: string, inc: string) {
	local INC = inc.upper();
	local Inc = inc.capitalize();
	local add = { "inc": "add", "dec": "sub" }[inc];
	local ADD = add.upper();

	print('[[export("DeeObject_{|T}', Inc, '")]]');
	print("[[tp_self(Dee_TYPE(*p_self))]]");
	print("[[wunused]] int");
	print("", prefix, "tp_", inc, "([[nonnull]] DREF DeeObject **__restrict p_self)");
	print("%{class using OPERATOR_", INC, ": {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, *p_self, OPERATOR_", INC, ");");
	print("	Dee_Decref_unlikely(*p_self);");
	print("	*p_self = result; /" "* Inherit reference *" "/");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}}");
	print("%{using ", prefix, "tp_inplace_", add, ": {");
	print("	return CALL_DEPENDENCY(", prefix, "tp_inplace_", add, ", p_self, DeeInt_One);");
	print("}}");
	print("%{using ", prefix, "tp_", add, ": {");
	print("	DREF DeeObject *result = CALL_DEPENDENCY(", prefix, "tp_", add, ", *p_self, DeeInt_One);");
	print("	if unlikely(!result)");
	print("		goto err;");
	print("	Dee_Decref_unlikely(*p_self);");
	print("	*p_self = result; /" "* Inherit reference *" "/");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}} = OPERATOR_", INC, ";");
	print;
}

function enterLeave(enter: string) {
	local ENTER = enter.upper();
	local Enter = enter.capitalize();
	local leave = { "enter": "leave", "leave": "enter" }[enter];
	local LEAVE = leave.upper();

	print('[[export("DeeObject_{|T}', Enter, '")]]');
	print("[[wunused]] int");
	print("tp_with->tp_", enter, "([[nonnull]] DeeObject *__restrict self)");
	print("%{class using OPERATOR_", ENTER, ": {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, self, OPERATOR_", ENTER, ");");
	print("	if unlikely(!result)");
	print("		goto err;");
	print("	Dee_Decref_unlikely(result);");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}}");
	print("%{using tp_with->tp_", leave, ": {");
	print("	return 0;");
	print("}} = OPERATOR_", ENTER, ";");
	print;
}


unary("inv");
unary("pos");
unary("neg");

binaryMath("add");
binaryMath("sub");
binaryMath("mul");
binaryMath("div");
binaryMath("mod");
binaryMath("shl");
binaryMath("shr");
binaryMath("and");
binaryMath("or");
binaryMath("xor");
binaryMath("pow");

inplaceUnary("tp_math->", "inc");
inplaceUnary("tp_math->", "dec");

enterLeave("enter");
enterLeave("leave");

]]]*/
[[export("DeeObject_{|T}Inv")]]
[[wunused]] DREF DeeObject *
tp_math->tp_inv([[nonnull]] DeeObject *self)
%{class using OPERATOR_INV: {
	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_INV);
}} = OPERATOR_INV;

[[export("DeeObject_{|T}Pos")]]
[[wunused]] DREF DeeObject *
tp_math->tp_pos([[nonnull]] DeeObject *self)
%{class using OPERATOR_POS: {
	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_POS);
}} = OPERATOR_POS;

[[export("DeeObject_{|T}Neg")]]
[[wunused]] DREF DeeObject *
tp_math->tp_neg([[nonnull]] DeeObject *self)
%{class using OPERATOR_NEG: {
	return_DeeClass_CallOperator_NoArgs(THIS_TYPE, self, OPERATOR_NEG);
}} = OPERATOR_NEG;

[[export("DeeObject_{|T}Add")]]
[[wunused]] DREF DeeObject *
tp_math->tp_add([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_ADD: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_ADD, rhs);
}} = OPERATOR_ADD;

[[export("DeeObject_{|T}InplaceAdd")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_add([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_ADD: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_ADD, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_add: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_ADD;

[[export("DeeObject_{|T}Sub")]]
[[wunused]] DREF DeeObject *
tp_math->tp_sub([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_SUB: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_SUB, rhs);
}} = OPERATOR_SUB;

[[export("DeeObject_{|T}InplaceSub")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_sub([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_SUB: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SUB, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_sub: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_SUB;

[[export("DeeObject_{|T}Mul")]]
[[wunused]] DREF DeeObject *
tp_math->tp_mul([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_MUL: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_MUL, rhs);
}} = OPERATOR_MUL;

[[export("DeeObject_{|T}InplaceMul")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mul([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_MUL: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MUL, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_mul: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mul, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_MUL;

[[export("DeeObject_{|T}Div")]]
[[wunused]] DREF DeeObject *
tp_math->tp_div([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_DIV: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_DIV, rhs);
}} = OPERATOR_DIV;

[[export("DeeObject_{|T}InplaceDiv")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_div([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_DIV: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_DIV, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_div: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_div, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_DIV;

[[export("DeeObject_{|T}Mod")]]
[[wunused]] DREF DeeObject *
tp_math->tp_mod([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_MOD: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_MOD, rhs);
}} = OPERATOR_MOD;

[[export("DeeObject_{|T}InplaceMod")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mod([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_MOD: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MOD, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_mod: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_mod, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_MOD;

[[export("DeeObject_{|T}Shl")]]
[[wunused]] DREF DeeObject *
tp_math->tp_shl([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_SHL: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_SHL, rhs);
}} = OPERATOR_SHL;

[[export("DeeObject_{|T}InplaceShl")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shl([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_SHL: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHL, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_shl: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shl, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_SHL;

[[export("DeeObject_{|T}Shr")]]
[[wunused]] DREF DeeObject *
tp_math->tp_shr([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_SHR: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_SHR, rhs);
}} = OPERATOR_SHR;

[[export("DeeObject_{|T}InplaceShr")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shr([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_SHR: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHR, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_shr: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_shr, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_SHR;

[[export("DeeObject_{|T}And")]]
[[wunused]] DREF DeeObject *
tp_math->tp_and([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_AND: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_AND, rhs);
}} = OPERATOR_AND;

[[export("DeeObject_{|T}InplaceAnd")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_and([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_AND: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_AND, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_and: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_and, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_AND;

[[export("DeeObject_{|T}Or")]]
[[wunused]] DREF DeeObject *
tp_math->tp_or([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_OR: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_OR, rhs);
}} = OPERATOR_OR;

[[export("DeeObject_{|T}InplaceOr")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_or([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_OR: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_OR, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_or: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_or, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_OR;

[[export("DeeObject_{|T}Xor")]]
[[wunused]] DREF DeeObject *
tp_math->tp_xor([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_XOR: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_XOR, rhs);
}} = OPERATOR_XOR;

[[export("DeeObject_{|T}InplaceXor")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_xor([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_XOR: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_XOR, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_xor: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_xor, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_XOR;

[[export("DeeObject_{|T}Pow")]]
[[wunused]] DREF DeeObject *
tp_math->tp_pow([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_POW: {
	return_DeeClass_CallOperator_1Arg(THIS_TYPE, lhs, OPERATOR_POW, rhs);
}} = OPERATOR_POW;

[[export("DeeObject_{|T}InplacePow")]]
[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_pow([[nonnull]] DREF DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class using OPERATOR_INPLACE_POW: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_POW, rhs);
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_pow: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_pow, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INPLACE_POW;

[[export("DeeObject_{|T}Inc")]]
[[tp_self(Dee_TYPE(*p_self))]]
[[wunused]] int
tp_math->tp_inc([[nonnull]] DREF DeeObject **__restrict p_self)
%{class using OPERATOR_INC: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, *p_self, OPERATOR_INC);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_inplace_add: {
	return CALL_DEPENDENCY(tp_math->tp_inplace_add, p_self, DeeInt_One);
}}
%{using tp_math->tp_add: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_add, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_INC;

[[export("DeeObject_{|T}Dec")]]
[[tp_self(Dee_TYPE(*p_self))]]
[[wunused]] int
tp_math->tp_dec([[nonnull]] DREF DeeObject **__restrict p_self)
%{class using OPERATOR_DEC: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, *p_self, OPERATOR_DEC);
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}}
%{using tp_math->tp_inplace_sub: {
	return CALL_DEPENDENCY(tp_math->tp_inplace_sub, p_self, DeeInt_One);
}}
%{using tp_math->tp_sub: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_math->tp_sub, *p_self, DeeInt_One);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} = OPERATOR_DEC;

[[export("DeeObject_{|T}Enter")]]
[[wunused]] int
tp_with->tp_enter([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_ENTER: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, self, OPERATOR_ENTER);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}}
%{using tp_with->tp_leave: {
	return 0;
}} = OPERATOR_ENTER;

[[export("DeeObject_{|T}Leave")]]
[[wunused]] int
tp_with->tp_leave([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_LEAVE: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_NoArgs(err, result, THIS_TYPE, self, OPERATOR_LEAVE);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}}
%{using tp_with->tp_enter: {
	return 0;
}} = OPERATOR_LEAVE;
/*[[[end]]]*/





} /* operator */
