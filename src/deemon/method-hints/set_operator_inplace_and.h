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
/* deemon.Set.operator &= ()                                            */
/************************************************************************/
__set_inplace_and__(rhs:?X3?DSet?DSequence?S?O)->?. {
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(set_operator_inplace_and, (DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Set: tp_math->tp_inplace_and)]]
[[wunused]] int
__set_inplace_and__.set_operator_inplace_and([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	DREF DeeObject *result;
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), set_operator_inplace_sub))(p_self, xrhs->si_set);
	}
	result = DeeObject_InvokeMethodHint(set_operator_and, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
})}
%{$none = 0}
%{$empty = "default__set_operator_inplace_and__unsupported"}
%{$with__set_operator_foreach__and__set_removeall = {
	int result;
	DREF DeeObject *keys_to_remove_proxy;
	DREF DeeObject *keys_to_remove;
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return CALL_DEPENDENCY(set_operator_inplace_sub, p_self, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(rhs))
		return CALL_DEPENDENCY(seq_clear, *p_self);

	/* `a &= b' -> `(a as Set).removeall((((a as Set) - b) as Set).frozen)' */
	keys_to_remove_proxy = DeeObject_InvokeMethodHint(set_operator_sub, *p_self, rhs);
	if unlikely(!keys_to_remove_proxy)
		goto err;
	keys_to_remove = DeeObject_InvokeMethodHint(set_frozen, (DeeObject *)keys_to_remove_proxy);
	Dee_Decref_likely(keys_to_remove_proxy);
	if unlikely(!keys_to_remove)
		goto err;
	result = CALL_DEPENDENCY(set_removeall, *p_self, keys_to_remove);
	Dee_Decref_likely(keys_to_remove);
	return result;
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

set_operator_inplace_and = {
	DeeMH_set_removeall_t set_removeall = REQUIRE(set_removeall);
	if (set_removeall && REQUIRE_ANY(set_operator_foreach) != &default__set_operator_foreach__unsupported)
		return &$with__set_operator_foreach__and__set_removeall;
};

