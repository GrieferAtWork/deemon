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

/*[[[deemon
for (local op, eq, Eq, isEq: {
	("==", "eq", "Eq", true),
	("!=", "ne", "Ne", true),
	("<", "lo", "Lo", false),
	("<=", "le", "Le", false),
	(">", "gr", "Gr", false),
	(">=", "ge", "Ge", false),
}) {
	print('' '/************************************************************************' '/');
	print('' '/* deemon.Sequence.operator ', op, ' ()                                       *' '/');
	print('' '/************************************************************************' '/');
	print('__seq_', eq, '__(rhs:?S?O)->?Dbool {');
	print('	DeeObject *rhs;');
	print('	if (DeeArg_Unpack(argc, argv, "o:__seq_', eq, '__", &rhs))');
	print('		goto err;');
	print('	return DeeSeq_OperatorEq(self, rhs);');
	print('err:');
	print('	return NULL;');
	print('}');
	print;
	print('[[wunused]] DREF DeeObject *');
	print('__seq_', eq, '__.seq_operator_', eq, '([[nonnull]] DeeObject *lhs,');
	print('                           [[nonnull]] DeeObject *rhs)');
	print('%{unsupported(auto("operator ', op, '"))}');
	print('%{$with__seq_operator_compare', isEq ? '_eq' : '', ' = {');
	print('	int result = DeeSeq_OperatorCompare', isEq ? 'Eq' : '', '(lhs, rhs);');
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
	print('#ifndef LOCAL_FOR_OPTIMIZE');
	print('	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))');
	print('		return THIS_TYPE->tp_cmp->tp_', eq, ';');
	print('#endif /' '* !LOCAL_FOR_OPTIMIZE *' '/');
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
__seq_eq__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_eq__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_eq__.seq_operator_eq([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator =="))}
%{$with__seq_operator_compare_eq = {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_eq = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare_eq))
		return &$with__seq_operator_compare_eq;
};



/************************************************************************/
/* deemon.Sequence.operator != ()                                       */
/************************************************************************/
__seq_ne__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ne__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_ne__.seq_operator_ne([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator !="))}
%{$with__seq_operator_compare_eq = {
	int result = DeeSeq_OperatorCompareEq(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_ne = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_ne;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare_eq))
		return &$with__seq_operator_compare_eq;
};



/************************************************************************/
/* deemon.Sequence.operator < ()                                       */
/************************************************************************/
__seq_lo__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_lo__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_lo__.seq_operator_lo([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator <"))}
%{$with__seq_operator_compare = {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result < 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_lo = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator <= ()                                       */
/************************************************************************/
__seq_le__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_le__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_le__.seq_operator_le([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator <="))}
%{$with__seq_operator_compare = {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result <= 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_le = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator > ()                                       */
/************************************************************************/
__seq_gr__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_gr__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_gr__.seq_operator_gr([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator >"))}
%{$with__seq_operator_compare = {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result > 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_gr = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};



/************************************************************************/
/* deemon.Sequence.operator >= ()                                       */
/************************************************************************/
__seq_ge__(rhs:?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_ge__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_ge__.seq_operator_ge([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto("operator >="))}
%{$with__seq_operator_compare = {
	int result = DeeSeq_OperatorCompare(lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result >= 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

seq_operator_ge = {
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEq(THIS_TYPE))
		return THIS_TYPE->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (REQUIRE(seq_operator_compare))
		return &$with__seq_operator_compare;
};
/*[[[end]]]*/

