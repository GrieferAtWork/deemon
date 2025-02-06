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

function binaryMath(add: string) {
	local ADD = add.upper();

	print("[[wunused]] DREF DeeObject *");
	print("tp_math->tp_", add, "([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class {");
	print("#ifdef __OPTIMIZE_SIZE__");
	print("	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_", ADD, ", 1, &rhs);");
	print("#else /" "* __OPTIMIZE_SIZE__ *" "/");
	print("	DREF DeeObject *cb, *result;");
	print("	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_", ADD, ");");
	print("	if unlikely(!cb)");
	print("		goto err;");
	print("	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);");
	print("	Dee_Decref_unlikely(cb);");
	print("	return result;");
	print("err:");
	print("	return NULL;");
	print("#endif /" "* !__OPTIMIZE_SIZE__ *" "/");
	print("}};");
	print;

	print("[[tp_self(Dee_TYPE(*p_lhs))]]");
	print("[[wunused]] int");
	print("tp_math->tp_inplace_", add, "([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)");
	print("%{class {");
	print("	DREF DeeObject *result;");
	print("#ifdef __OPTIMIZE_SIZE__");
	print("	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_", ADD, ", 1, &rhs);");
	print("#else /" "* __OPTIMIZE_SIZE__ *" "/");
	print("	DREF DeeObject *cb;");
	print("	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_", ADD, ");");
	print("	if unlikely(!cb)");
	print("		goto err;");
	print("	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);");
	print("	Dee_Decref_unlikely(cb);");
	print("#endif /" "* !__OPTIMIZE_SIZE__ *" "/");
	print("	if unlikely(!result)");
	print("		goto err;");
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
	print("#ifdef __OPTIMIZE_SIZE__");
	print("	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_", INC, ", 0, NULL);");
	print("#else /" "* __OPTIMIZE_SIZE__ *" "/");
	print("	DREF DeeObject *cb;");
	print("	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_", INC, ");");
	print("	if unlikely(!cb)");
	print("		goto err;");
	print("	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);");
	print("	Dee_Decref_unlikely(cb);");
	print("#endif /" "* !__OPTIMIZE_SIZE__ *" "/");
	print("	if unlikely(!result)");
	print("		goto err;");
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
	print("#ifdef __OPTIMIZE_SIZE__");
	print("	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_", ENTER, ", 0, NULL);");
	print("#else /" "* __OPTIMIZE_SIZE__ *" "/");
	print("	DREF DeeObject *cb;");
	print("	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_", ENTER, ");");
	print("	if unlikely(!cb)");
	print("		goto err;");
	print("	result = DeeObject_ThisCall(cb, self, 0, NULL);");
	print("	Dee_Decref_unlikely(cb);");
	print("#endif /" "* !__OPTIMIZE_SIZE__ *" "/");
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
tp_math->tp_add([[nonnull]] DeeObject *lhs, [[nonnull]] DeeObject *rhs)
%{class {
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_add([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_ADD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_ADD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_sub([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SUB, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SUB);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mul([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MUL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MUL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_div([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_DIV, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_DIV);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_mod([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_MOD, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_MOD);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shl([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHL, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHL);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_shr([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_SHR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_SHR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_and([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_AND, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_AND);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_or([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_OR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_OR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_xor([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_XOR, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_XOR);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	return DeeClass_CallOperator(THIS_TYPE, lhs, OPERATOR_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb, *result;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
	return result;
err:
	return NULL;
#endif /* !__OPTIMIZE_SIZE__ */
}};

[[tp_self(Dee_TYPE(*p_lhs))]]
[[wunused]] int
tp_math->tp_inplace_pow([[nonnull]] DeeObject **__restrict p_lhs, [[nonnull]] DeeObject *rhs)
%{class {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_lhs, OPERATOR_INPLACE_POW, 1, &rhs);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INPLACE_POW);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_lhs, 1, &rhs);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_INC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_INC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, *p_self, OPERATOR_DEC, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_DEC);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, *p_self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result)
		goto err;
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
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ENTER, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ENTER);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
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
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_LEAVE, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *cb;
	cb = DeeClass_GetOperator(THIS_TYPE, OPERATOR_LEAVE);
	if unlikely(!cb)
		goto err;
	result = DeeObject_ThisCall(cb, self, 0, NULL);
	Dee_Decref_unlikely(cb);
#endif /* !__OPTIMIZE_SIZE__ */
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
