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

/*[[[deemon
for (local op, eq, ne, Eq, EQ, isEq: {
	("==", "eq", "ne", "Eq", "EQ", true),
	("!=", "ne", "eq", "Ne", "NE", true),
	("<",  "lo", "ge", "Lo", "LO", false),
	("<=", "le", "gr", "Le", "LE", false),
	(">",  "gr", "le", "Gr", "GR", false),
	(">=", "ge", "lo", "Ge", "GE", false),
}) {
	print('' '/************************************************************************' '/');
	print('' '/* deemon.Sequence.operator ', op, ' ()                                       *' '/');
	print('' '/************************************************************************' '/');
	print('__seq_', eq, '__(rhs:?X2?DSequence?S?O)->?Dbool {');
	print('	return CALL_DEPENDENCY(seq_operator_', eq, ', self, rhs);');
	print('}');
	print;
	print("[[operator(Sequence: tp_cmp->tp_", eq, ")]]");
	print('[[wunused]] DREF DeeObject *');
	print('__seq_', eq, '__.seq_operator_', eq, '([[nonnull]] DeeObject *lhs,');
	print('                           [[nonnull]] DeeObject *rhs)');
	print('%{unsupported(auto("operator ', op, '"))}');
	print('%{$empty = "$with__seq_operator_compare', isEq ? '_eq' : '', '"}');
	print('%{$with__seq_operator_', ne, ' = [[prefix(DEFINE_xinvoke_not)]] {');
	print('	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_', ne, ', lhs, rhs);');
	print('	return xinvoke_not(result);');
	print('}}');
	print('%{$with__seq_operator_compare', isEq ? '_eq' : '', ' = {');
	print('	int result = CALL_DEPENDENCY(seq_operator_compare', isEq ? '_eq' : '', ', lhs, rhs);');
	print('	if unlikely(result == Dee_COMPARE_ERR)');
	print('		goto err;');
	print('	return_bool(result ', op, ' 0);');
	print('err:');
	print('	return NULL;');
	print('}} {');
	print('	return LOCAL_CALLATTR(lhs, 1, &rhs);');
	print('}');
	print('');
	print('seq_operator_', eq, ' = {');
	print('	if (REQUIRE_NODEFAULT(seq_operator_', ne, '))');
	print('		return &$with__seq_operator_', ne, ';');
	print('	if (REQUIRE(seq_operator_compare', isEq ? '_eq' : '', '))');
	print('		return &$with__seq_operator_compare', isEq ? '_eq' : '', ';');
	print('};');
	print;
	print;
	print;
}
]]]*/
/************************************************************************/
/* deemon.Sequence.operator == ()                                       */
/************************************************************************/
__seq_eq__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_eq, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_eq)]]
[[wunused]] DREF DeeObject *
__seq_eq__.seq_operator_eq([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator =="))}
%{$empty = "$with__seq_operator_compare_eq"}
%{$with__seq_operator_ne = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_ne, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare_eq = {
	int result = CALL_DEPENDENCY(seq_operator_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_eq = {
	if (REQUIRE_NODEFAULT(seq_operator_ne))
		return &$with__seq_operator_ne;
	if (REQUIRE(seq_operator_compare_eq))
		return &$with__seq_operator_compare_eq;
};



/************************************************************************/
/* deemon.Sequence.operator != ()                                       */
/************************************************************************/
__seq_ne__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_ne, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_ne)]]
[[wunused]] DREF DeeObject *
__seq_ne__.seq_operator_ne([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator !="))}
%{$empty = "$with__seq_operator_compare_eq"}
%{$with__seq_operator_eq = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_eq, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare_eq = {
	int result = CALL_DEPENDENCY(seq_operator_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_ne = {
	if (REQUIRE_NODEFAULT(seq_operator_eq))
		return &$with__seq_operator_eq;
	if (REQUIRE(seq_operator_compare_eq))
		return &$with__seq_operator_compare_eq;
};



/************************************************************************/
/* deemon.Sequence.operator < ()                                       */
/************************************************************************/
__seq_lo__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_lo, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_lo)]]
[[wunused]] DREF DeeObject *
__seq_lo__.seq_operator_lo([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator <"))}
%{$empty = "$with__seq_operator_compare"}
%{$with__seq_operator_ge = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_ge, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare = {
	int result = CALL_DEPENDENCY(seq_operator_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_lo = {
	if (REQUIRE_NODEFAULT(seq_operator_ge))
		return &$with__seq_operator_ge;
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator <= ()                                       */
/************************************************************************/
__seq_le__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_le, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_le)]]
[[wunused]] DREF DeeObject *
__seq_le__.seq_operator_le([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator <="))}
%{$empty = "$with__seq_operator_compare"}
%{$with__seq_operator_gr = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_gr, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare = {
	int result = CALL_DEPENDENCY(seq_operator_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_le = {
	if (REQUIRE_NODEFAULT(seq_operator_gr))
		return &$with__seq_operator_gr;
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator > ()                                       */
/************************************************************************/
__seq_gr__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_gr, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_gr)]]
[[wunused]] DREF DeeObject *
__seq_gr__.seq_operator_gr([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator >"))}
%{$empty = "$with__seq_operator_compare"}
%{$with__seq_operator_le = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_le, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare = {
	int result = CALL_DEPENDENCY(seq_operator_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_gr = {
	if (REQUIRE_NODEFAULT(seq_operator_le))
		return &$with__seq_operator_le;
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator >= ()                                       */
/************************************************************************/
__seq_ge__(rhs:?X2?DSequence?S?O)->?Dbool {
	return CALL_DEPENDENCY(seq_operator_ge, self, rhs);
}

[[operator(Sequence: tp_cmp->tp_ge)]]
[[wunused]] DREF DeeObject *
__seq_ge__.seq_operator_ge([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator >="))}
%{$empty = "$with__seq_operator_compare"}
%{$with__seq_operator_lo = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_lo, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__seq_operator_compare = {
	int result = CALL_DEPENDENCY(seq_operator_compare, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_ge = {
	if (REQUIRE_NODEFAULT(seq_operator_lo))
		return &$with__seq_operator_lo;
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};
/*[[[end]]]*/

