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
/* deemon.Set.operator ^= ()                                            */
/************************************************************************/
__set_inplace_xor__(rhs:?X3?DSet?DSequence?S?O)->?. {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_inplace_xor__", &rhs))
		goto err;
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(set_operator_inplace_xor, (DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Set: tp_math->tp_inplace_xor)]]
[[wunused]] int
__set_inplace_xor__.set_operator_inplace_xor([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(set_operator_xor, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
})}
%{$none = 0}
%{$empty = "default__set_operator_inplace_xor__unsupported"}
%{$with__set_operator_foreach__and__set_insertall__and__set_removeall = {
	DREF DeeObject *only_in_rhs_proxy;
	DREF DeeObject *only_in_rhs;
	if (DeeSet_CheckEmpty(rhs))
		return 0;

	/* >> a ^= b
	 * <=>
	 * >> local only_in_rhs = (((b as Set) - a) as Set).frozen;
	 * >> a.removeall(b);
	 * >> a.insertall(only_in_rhs); */
	only_in_rhs_proxy = DeeObject_InvokeMethodHint(set_operator_sub, rhs, *p_self);
	if unlikely(!only_in_rhs_proxy)
		goto err;
	only_in_rhs = DeeObject_InvokeMethodHint(set_frozen, (DeeObject *)only_in_rhs_proxy);
	Dee_Decref(only_in_rhs_proxy);
	if unlikely(!only_in_rhs)
		goto err;
	if unlikely(CALL_DEPENDENCY(set_removeall, *p_self, rhs))
		goto err_only_in_rhs;
	if unlikely(CALL_DEPENDENCY(set_insertall, *p_self, only_in_rhs))
		goto err_only_in_rhs;
	Dee_Decref(only_in_rhs);
	return 0;
err_only_in_rhs:
	Dee_Decref(only_in_rhs);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(*p_self, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

set_operator_inplace_xor = {
	if ((REQUIRE_ANY(set_operator_foreach) != &default__set_operator_foreach__unsupported) &&
	    ((REQUIRE(set_removeall) && REQUIRE_ANY(set_insertall)) ||
	     (REQUIRE(set_insertall) && REQUIRE_ANY(set_removeall))))
		return &$with__set_operator_foreach__and__set_insertall__and__set_removeall;
};

