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

operator {

/*[[[deemon
import * from deemon;

function unary(neg: string) {
	local NEG = neg.upper();

	print("[[wunused]] DREF DeeObject *");
	print("tp_math->tp_", neg, "([[nonnull]] DeeObject *self)");
	print("%{class {");
	print("	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_", NEG, ", 0, NULL);");
	print("}};");
	print;
}

function binaryMath(add: string) {
	local ADD = add.upper();

	print("[[wunused]] DREF DeeObject *");
	print("tp_math->tp_", add, "([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class {");
	print("	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_", ADD, ", 1, &rhs);");
	print("}};");
	print;

	print("[[tp_self(Dee_TYPE(*p_lhs))]]");
	print("[[wunused]] int");
	print("tp_math->tp_inplace_", add, "([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_", ADD, ", 1, &rhs);");
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
	print("}};");
	print;
}

function inplaceUnary(prefix: string, inc: string) {
	local INC = inc.upper();
	local add = { "inc": "add", "dec": "sub" }[inc];
	local ADD = add.upper();

	print("[[tp_self(Dee_TYPE(*p_self))]]");
	print("[[wunused]] int");
	print("", prefix, "tp_", inc, "([[nonnull]] DeeObject **__restrict p_self)");
	print("%{class {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_self, OPERATOR_", INC, ", 0, NULL);");
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
	print("}};");
	print;
}

function enterLeave(enter: string) {
	local ENTER = enter.upper();
	local leave = { "enter": "leave", "leave": "enter" }[enter];
	local LEAVE = leave.upper();

	print("[[wunused]] int");
	print("tp_with->tp_", enter, "([[nonnull]] DeeObject *__restrict self)");
	print("%{class {");
	print("	DREF DeeObject *result;");
	print("	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_", ENTER, ", 0, NULL);");
	print("	if unlikely(!result)");
	print("		goto err;");
	print("	Dee_Decref_unlikely(result);");
	print("	return 0;");
	print("err:");
	print("	return -1;");
	print("}}");
	print("%{using tp_with->tp_", leave, ": {");
	print("	return 0;");
	print("}};");
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
[[wunused]] DREF DeeObject *
tp_math->tp_inv([[nonnull]] DeeObject *self)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_INV, 0, NULL);
}};

[[wunused]] DREF DeeObject *
tp_math->tp_pos([[nonnull]] DeeObject *self)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_POS, 0, NULL);
}};

[[wunused]] DREF DeeObject *
tp_math->tp_neg([[nonnull]] DeeObject *self)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_NEG, 0, NULL);
}};

[[wunused]] DREF DeeObject *
tp_math->tp_add([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_ADD, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_add([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_sub([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SUB, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_sub([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_mul([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MUL, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mul([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_div([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_DIV, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_div([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_mod([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MOD, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mod([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_shl([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHL, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shl([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_shr([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHR, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shr([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_and([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_AND, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_and([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_or([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_OR, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_or([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_xor([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_XOR, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_xor([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
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
}};

[[wunused]] DREF DeeObject *
tp_math->tp_pow([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_POW, 1, &rhs);
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_pow([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
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
}};

[[tp_self(Dee_TYPE(*p_self))]]
[[wunused]] int
tp_math->tp_inc([[nonnull]] DeeObject **__restrict p_self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_self, OPERATOR_INC, 0, NULL);
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
}};

[[tp_self(Dee_TYPE(*p_self))]]
[[wunused]] int
tp_math->tp_dec([[nonnull]] DeeObject **__restrict p_self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, *p_self, OPERATOR_DEC, 0, NULL);
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
}};

[[wunused]] int
tp_with->tp_enter([[nonnull]] DeeObject *__restrict self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_ENTER, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}}
%{using tp_with->tp_leave: {
	return 0;
}};

[[wunused]] int
tp_with->tp_leave([[nonnull]] DeeObject *__restrict self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_LEAVE, 0, NULL);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result);
	return 0;
err:
	return -1;
}}
%{using tp_with->tp_enter: {
	return 0;
}};
/*[[[end]]]*/





} /* operator */
