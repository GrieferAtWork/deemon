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
/* deemon.Set.__set_compare_eq__()                                      */
/************************************************************************/
__set_compare_eq__(rhs:?X2?DSet?S?O)->?X2?Dbool?Dint {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_compare_eq__", &rhs))
		goto err;
	result = CALL_DEPENDENCY(set_operator_compare_eq, self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	/* We always return "bool" here, but user-code is also allowed to return "int" */
	return_bool(result == 0);
err:
	return NULL;
}



[[operator(Set.OPERATOR_EQ: tp_cmp->tp_compare_eq)]]
[[wunused]] int
__set_compare_eq__.set_operator_compare_eq([[nonnull]] DeeObject *lhs,
                                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "default__seq_operator_compare_eq__empty"}
%{$with__set_operator_eq = {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(set_operator_eq, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__set_operator_ne = {
	int result;
	DREF DeeObject *cmp_ob = CALL_DEPENDENCY(set_operator_ne, lhs, rhs);
	if unlikely(!cmp_ob)
		goto err;
	result = DeeObject_BoolInherited(cmp_ob);
	if unlikely(result < 0)
		goto err;
	return result;
err:
	return Dee_COMPARE_ERR;
}}
%{$with__set_operator_foreach = {
	size_t rhs_size;
	Dee_ssize_t contains_status;
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	struct set_compare__lhs_foreach__rhs__data data;
	data.sc_lfr_rhs       = rhs;
	data.sc_lfr_rcontains = DeeType_RequireMethodHint(tp_rhs, seq_operator_contains);
	contains_status = CALL_DEPENDENCY(set_operator_foreach, lhs, &set_compare__lhs_foreach__rhs__cb, &data);
	if unlikely(contains_status == -1)
		goto err;
	if (contains_status == -2)
		return 1; /* "rhs" is missing some element of "lhs" */
	ASSERT(tp_rhs == Dee_TYPE(rhs));
	rhs_size = (*DeeType_RequireMethodHint(tp_rhs, set_operator_size))(rhs);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
	if ((size_t)contains_status != rhs_size)
		return 1; /* Sets have different sizes */
	return 0;
err:
	return Dee_COMPARE_ERR;
}} {
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



set_operator_compare_eq = {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if (REQUIRE_NODEFAULT(set_operator_eq))
		return $with__set_operator_eq;
	if (REQUIRE_NODEFAULT(set_operator_ne))
		return $with__set_operator_ne;
	set_operator_foreach = REQUIRE(set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &$empty;
	if (set_operator_foreach)
		return &$with__set_operator_foreach;
};



[[operator(Set.OPERATOR_EQ: tp_cmp->tp_trycompare_eq)]]
[[wunused]] int
__set_compare_eq__.set_operator_trycompare_eq([[nonnull]] DeeObject *lhs,
                                              [[nonnull]] DeeObject *rhs)
%{unsupported({
	return 1;
})}
%{$empty = "default__seq_operator_trycompare_eq__empty"}
%{using set_operator_compare_eq: {
	int result = CALL_DEPENDENCY(set_operator_compare_eq, lhs, rhs);
	if (result == Dee_COMPARE_ERR) {
		if (DeeError_Catch(&DeeError_NotImplemented) ||
		    DeeError_Catch(&DeeError_TypeError) ||
		    DeeError_Catch(&DeeError_ValueError))
			result = -1;
	}
	return result;
}} = $with__set_operator_compare_eq;


set_operator_trycompare_eq = {
	DeeMH_set_operator_compare_eq_t set_operator_compare_eq = REQUIRE(set_operator_compare_eq);
	if (set_operator_compare_eq == &default__set_operator_compare_eq__empty)
		return &$empty;
	if (set_operator_compare_eq)
		return &$with__set_operator_compare_eq;
};





/************************************************************************/
/* deemon.Set.operator == ()                                            */
/************************************************************************/
__set_eq__(rhs:?X2?DSet?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_eq__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_eq, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_EQ: tp_cmp->tp_eq)]]
[[wunused]] DREF DeeObject *
__set_eq__.set_operator_eq([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "$with__set_operator_compare_eq"}
%{$with__set_operator_compare_eq = {
	int result = CALL_DEPENDENCY(set_operator_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_eq = {
	if (REQUIRE(set_operator_compare_eq))
		return &$with__set_operator_compare_eq;
};



/************************************************************************/
/* deemon.Set.operator != ()                                            */
/************************************************************************/
__set_ne__(rhs:?X2?DSet?S?O)->?Dbool {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_ne__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_ne, self, rhs);
err:
	return NULL;
}

[[operator(Set.OPERATOR_NE: tp_cmp->tp_ne)]]
[[wunused]] DREF DeeObject *
__set_ne__.set_operator_ne([[nonnull]] DeeObject *lhs,
                           [[nonnull]] DeeObject *rhs)
%{unsupported(auto)}
%{$empty = "$with__set_operator_compare_eq"}
%{$with__set_operator_compare_eq = {
	int result = CALL_DEPENDENCY(set_operator_compare_eq, lhs, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result != 0);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

set_operator_ne = {
	if (REQUIRE(set_operator_compare_eq))
		return &$with__set_operator_compare_eq;
};

