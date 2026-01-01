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

/************************************************************************/
/* deemon.Mapping.operator <|<=|>|>= ()                                 */
/************************************************************************/

/*[[[deemon
for (local lo, ge: {
	("lo", "ge"),
	("le", "gr"),
	("gr", "le"),
	("ge", "lo"),
}) {
	print('__map_', lo, '__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool {');
	print('	return CALL_DEPENDENCY(map_operator_', lo, ', self, rhs);');
	print('}');
	print;
	print('[[operator(Mapping: tp_cmp->tp_', lo, ')]]');
	print('[[wunused]] DREF DeeObject *');
	print('__map_', lo, '__.map_operator_', lo, '([[nonnull]] DeeObject *lhs,');
	print('                           [[nonnull]] DeeObject *rhs)');
	print('%{unsupported(auto)}');
	print('%{$empty = "default__set_operator_', lo, '__empty"}');
	print('%{$with__map_operator_', ge, ' = [[prefix(DEFINE_xinvoke_not)]] {');
	print('	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_', ge, ', lhs, rhs);');
	print('	return xinvoke_not(result);');
	print('}}');
	print('%{$with__map_operator_foreach_pair = {');
if (lo in ["lo", "ge"]) {
	print('	size_t rhs_size;');
	print('	Dee_ssize_t contains_status;');
	print('	struct map_compare__lhs_foreach__rhs__data data;');
	print('	data.mc_lfr_rhs         = rhs;');
	print('	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);');
	print('	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);');
	print('	if unlikely(contains_status == -1)');
	print('		goto err;');
	print('	if (contains_status == -2)');
	print('		goto missing_item; /' '* "rhs" is missing some element of "lhs", or has a different value for it *' '/');
	print('	rhs_size = DeeObject_InvokeMethodHint(map_operator_size, rhs);');
	print('	if unlikely(rhs_size == (size_t)-1)');
	print('		goto err;');
	print('	if ((size_t)contains_status >= rhs_size)');
	print('		goto missing_item; /' '* "rhs" contains element not found in "lhs" *' '/');
	print('	', lo == "lo" ? "return_true" : "return_false", ';');
	print('missing_item:');
	print('	', lo == "lo" ? "return_false" : "return_true", ';');
	print('err:');
	print('	return NULL;');
} else if (lo in ["le", "gr"]) {
	print('	Dee_ssize_t contains_status;');
	print('	struct map_compare__lhs_foreach__rhs__data data;');
	print('	data.mc_lfr_rhs         = rhs;');
	print('	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);');
	print('	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);');
	print('	if unlikely(contains_status == -1)');
	print('		goto err;');
	print('	if (contains_status == -2)');
	print('		goto missing_item; /' '* "rhs" is missing some element of "lhs", or has a different value for it *' '/');
	print('	', lo == "le" ? "return_true" : "return_false", ';');
	print('missing_item:');
	print('	', lo == "le" ? "return_false" : "return_true", ';');
	print('err:');
	print('	return NULL;');
}
	print('}} {');
	print('	return LOCAL_CALLATTR(lhs, 1, &rhs);');
	print('}');
	print;
	print('map_operator_', lo, ' = {');
	print('	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;');
	print('	if (REQUIRE_NODEFAULT(map_operator_', ge, '))');
	print('		return &$with__map_operator_', ge, ';');
	print('	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);');
	print('	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)');
	print('		return &$empty;');
	print('	if (map_operator_foreach_pair)');
	print('		return &$with__map_operator_foreach_pair;');
	print('};');
	print;
	print;
}
]]]*/
__map_lo__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool {
	return CALL_DEPENDENCY(map_operator_lo, self, rhs);
}

[[operator(Mapping: tp_cmp->tp_lo)]]
[[wunused]] DREF DeeObject *
__map_lo__.map_operator_lo([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__set_operator_lo__empty"}
%{$with__map_operator_ge = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_ge, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__map_operator_foreach_pair = {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	rhs_size = DeeObject_InvokeMethodHint(map_operator_size, rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains element not found in "lhs" */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

map_operator_lo = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if (REQUIRE_NODEFAULT(map_operator_ge))
		return &$with__map_operator_ge;
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};


__map_le__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool {
	return CALL_DEPENDENCY(map_operator_le, self, rhs);
}

[[operator(Mapping: tp_cmp->tp_le)]]
[[wunused]] DREF DeeObject *
__map_le__.map_operator_le([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__set_operator_le__empty"}
%{$with__map_operator_gr = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_gr, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__map_operator_foreach_pair = {
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	return_true;
missing_item:
	return_false;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

map_operator_le = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if (REQUIRE_NODEFAULT(map_operator_gr))
		return &$with__map_operator_gr;
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};


__map_gr__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool {
	return CALL_DEPENDENCY(map_operator_gr, self, rhs);
}

[[operator(Mapping: tp_cmp->tp_gr)]]
[[wunused]] DREF DeeObject *
__map_gr__.map_operator_gr([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__set_operator_gr__empty"}
%{$with__map_operator_le = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_le, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__map_operator_foreach_pair = {
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

map_operator_gr = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if (REQUIRE_NODEFAULT(map_operator_le))
		return &$with__map_operator_le;
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};


__map_ge__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?Dbool {
	return CALL_DEPENDENCY(map_operator_ge, self, rhs);
}

[[operator(Mapping: tp_cmp->tp_ge)]]
[[wunused]] DREF DeeObject *
__map_ge__.map_operator_ge([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__set_operator_ge__empty"}
%{$with__map_operator_lo = [[prefix(DEFINE_xinvoke_not)]] {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_lo, lhs, rhs);
	return xinvoke_not(result);
}}
%{$with__map_operator_foreach_pair = {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	struct map_compare__lhs_foreach__rhs__data data;
	data.mc_lfr_rhs         = rhs;
	data.mc_lfr_rtrygetitem = DeeType_RequireNativeOperator(Dee_TYPE(rhs), trygetitem);
	contains_status = CALL_DEPENDENCY(map_operator_foreach_pair, lhs, &map_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		goto missing_item; /* "rhs" is missing some element of "lhs", or has a different value for it */
	rhs_size = DeeObject_InvokeMethodHint(map_operator_size, rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status >= rhs_size)
		goto missing_item; /* "rhs" contains element not found in "lhs" */
	return_false;
missing_item:
	return_true;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

map_operator_ge = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if (REQUIRE_NODEFAULT(map_operator_lo))
		return &$with__map_operator_lo;
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &$empty;
	if (map_operator_foreach_pair)
		return &$with__map_operator_foreach_pair;
};
/*[[[end]]]*/

