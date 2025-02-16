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
/* deemon.Set.operator <|<=|>|>= ()                                     */
/************************************************************************/

/*[[[deemon
for (local lo, ge, alias: {
	("lo", "ge", none),
	("le", "gr", 'issubset'),
	("gr", "le", none),
	("ge", "lo", 'issuperset'),
}) {
	if (alias !is none)
		print('[[alias(Set.', alias, ' -> "set_', alias, '")]]');
	print('__set_', lo, '__(rhs:?X3?DSet?DSequence?S?O)->?Dbool {');
	print('	DeeObject *rhs;');
	print('	if (DeeArg_Unpack(argc, argv, "o:__set_', lo, '__", &rhs))');
	print('		goto err;');
	print('	return CALL_DEPENDENCY(set_operator_', lo, ', self, rhs);');
	print('err:');
	print('	return NULL;');
	print('}');
	print;
	print('[[operator(Set.OPERATOR_', lo.upper(), ': tp_cmp->tp_', lo, ')]]');
	print('[[wunused]] DREF DeeObject *');
	print('__set_', lo, '__.set_operator_', lo, '([[nonnull]] DeeObject *lhs,');
	print('                           [[nonnull]] DeeObject *rhs)');
	print('%{unsupported(auto)}');
if (lo in ["lo", "ge"]) {
	print('%{$empty = {');
	print('	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);');
	print('	if unlikely(rhs_nonempty < 0)');
	print('		goto err;');
	print('	return_bool(rhs_nonempty ', lo == "lo" ? '!=' : '==', ' 0);');
	print('err:');
	print('	return NULL;');
	print('}}');
} else if (lo == "le") {
	print('%{$empty = return_true}');
} else if (lo == "gr") {
	print('%{$empty = return_false}');
}
	print('%{$with__set_operator_', ge, ' = [[prefix(DEFINE_xinvoke_not)]] {');
	print('	DREF DeeObject *result = CALL_DEPENDENCY(set_operator_', ge, ', lhs, rhs);');
	print('	return xinvoke_not(result);');
	print('}}');
	print('%{$with__set_operator_foreach = {');
if (lo in ["lo", "ge"]) {
	print('	size_t rhs_size;');
	print('	Dee_ssize_t contains_status;');
	print('	struct set_compare__lhs_foreach__rhs__data data;');
	print('	data.sc_lfr_rhs       = rhs;');
	print('	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);');
	print('	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);');
	print('	if unlikely(contains_status == -1)');
	print('		goto err;');
	print('	if (contains_status == -2)');
	print('		goto missing_item; /' '* "rhs" is missing some element of "lhs" *' '/');
	print('	rhs_size = DeeObject_InvokeMethodHint(seq_operator_size, rhs);');
	print('	if unlikely(rhs_size == (size_t)-1)');
	print('		goto err;');
	print('	if ((size_t)contains_status >= rhs_size)');
	print('		goto missing_item; /' '* "rhs" contains elements not found in "lhs" *' '/');
	print('	', lo == "lo" ? 'return_true' : 'return_false', ';');
	print('missing_item:');
	print('	', lo == "lo" ? 'return_false' : 'return_true', ';');
	print('err:');
	print('	return NULL;');
} else if (lo in ["le", "gr"]) {
	print('	Dee_ssize_t contains_status;');
	print('	struct set_compare__lhs_foreach__rhs__data data;');
	print('	data.sc_lfr_rhs       = rhs;');
	print('	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);');
	print('	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);');
	print('	if unlikely(contains_status == -1)');
	print('		goto err;');
	print('	if (contains_status == -2)');
	print('		goto missing_item; /' '* "rhs" is missing some element of "lhs" *' '/');
	print('	', lo == "le" ? 'return_true' : 'return_false', ';');
	print('missing_item:');
	print('	', lo == "le" ? 'return_false' : 'return_true', ';');
	print('err:');
	print('	return NULL;');
}
	print('}} {');
	print('	return LOCAL_CALLATTR(lhs, 1, &rhs);');
	print('}');
	print;
	print('set_operator_', lo, ' = {');
	print('	DeeMH_set_operator_foreach_t set_operator_foreach;');
	print('	if (REQUIRE_NODEFAULT(set_operator_', ge, '))');
	print('		return &$with__set_operator_', ge, ';');
	print('	set_operator_foreach = REQUIRE(set_operator_foreach);');
	print('	if (set_operator_foreach == &default__set_operator_foreach__empty)');
	print('		return &$empty;');
	print('	if (set_operator_foreach)');
	print('		return &$with__set_operator_foreach;');
	print('};');
	print;
	print;
}
]]]*/
__set_lo__(rhs:?X3?DSet?DSequence?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_lo__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_lo, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_LO: tp_cmp->tp_lo)]]
[[wunused]] DREF DeeObject *
__set_lo__.set_operator_lo([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return_bool(rhs_nonempty != 0);
err:
	return NULL;
}}
%{$with__set_operator_ge = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(set_operator_ge, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__set_operator_foreach = {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);
	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	rhs_size = DeeObject_InvokeMethodHint(seq_operator_size, rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains elements not found in "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_lo = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if (REQUIRE_NODEFAULT(set_operator_ge))
		return &$with__set_operator_ge;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};


[[alias(Set.issubset -> "set_issubset")]]
__set_le__(rhs:?X3?DSet?DSequence?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_le__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_le, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_LE: tp_cmp->tp_le)]]
[[wunused]] DREF DeeObject *
__set_le__.set_operator_le([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = return_true}
%{$with__set_operator_gr = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(set_operator_gr, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__set_operator_foreach = {
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);
	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_le = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if (REQUIRE_NODEFAULT(set_operator_gr))
		return &$with__set_operator_gr;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};


__set_gr__(rhs:?X3?DSet?DSequence?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_gr__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_gr, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_GR: tp_cmp->tp_gr)]]
[[wunused]] DREF DeeObject *
__set_gr__.set_operator_gr([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = return_false}
%{$with__set_operator_le = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(set_operator_le, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__set_operator_foreach = {
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);
	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_gr = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if (REQUIRE_NODEFAULT(set_operator_le))
		return &$with__set_operator_le;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};


[[alias(Set.issuperset -> "set_issuperset")]]
__set_ge__(rhs:?X3?DSet?DSequence?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ge__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_ge, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_GE: tp_cmp->tp_ge)]]
[[wunused]] DREF DeeObject *
__set_ge__.set_operator_ge([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = {
	int rhs_nonempty = DeeObject_InvokeMethodHint(seq_operator_bool, rhs);
	if unlikely(rhs_nonempty < 0)
		goto err;
	return_bool(rhs_nonempty == 0);
err:
	return NULL;
}}
%{$with__set_operator_lo = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(set_operator_lo, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__set_operator_foreach = {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireMethodHint(Dee_TYPE(rhs), seq_operator_contains);
	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs" */
	rhs_size = DeeObject_InvokeMethodHint(seq_operator_size, rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains elements not found in "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_ge = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if (REQUIRE_NODEFAULT(set_operator_lo))
		return &$with__set_operator_lo;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};
/*[[[end]]]*/

