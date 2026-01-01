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
/* deemon.Mapping.operator ^= ()                                        */
/************************************************************************/
__map_inplace_xor__(rhs:?X3?DMapping?M?O?O?S?T2?O?O)->?. {
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(map_operator_inplace_xor, (DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Mapping: tp_math->tp_inplace_xor)]]
[[wunused]] int
__map_inplace_xor__.map_operator_inplace_xor([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(map_operator_xor, *p_self, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
})}
%{$none = 0}
%{$empty = "default__map_operator_inplace_xor__unsupported"}
%{$with__map_operator_foreach_pair__and__map_update__and__map_removekeys = {
	/* >> a ^= b
	 * <=>
	 * >> local a_keys = (a as Mapping).keys;
	 * >> local b_keys = (b as Mapping).keys;
	 * >> local a_and_b_keys = (((a_keys as Set) & b_keys) as Set).frozen;
	 * >> (a as Mapping).removekeys(b_keys);
	 * >> (a as Mapping).update((b as Mapping) - a_and_b_keys); */
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *b_keys;
	DREF DeeObject *a_and_b_keys_proxy;
	DREF DeeObject *a_and_b_keys;
	DREF DeeObject *b_without_a_keys;
	a_keys = CALL_DEPENDENCY(map_keys, *p_self);
	if unlikely(!a_keys)
		goto err;
	b_keys = DeeObject_InvokeMethodHint(map_keys, rhs);
	if unlikely(!b_keys)
		goto err_a_keys;
	a_and_b_keys_proxy = DeeObject_InvokeMethodHint(set_operator_and, a_keys, b_keys);
	Dee_Decref(a_keys);
	if unlikely(!a_and_b_keys_proxy)
		goto err_b_keys;
	a_and_b_keys = DeeObject_InvokeMethodHint(set_frozen, a_and_b_keys_proxy);
	Dee_Decref(a_and_b_keys_proxy);
	if unlikely(!a_and_b_keys)
		goto err_b_keys;
	result = CALL_DEPENDENCY(map_removekeys, *p_self, b_keys);
	Dee_Decref(b_keys);
	if unlikely(result)
		goto err_a_and_b_keys;
	b_without_a_keys = DeeObject_InvokeMethodHint(map_operator_sub, rhs, a_and_b_keys);
	Dee_Decref(a_and_b_keys);
	if unlikely(!b_without_a_keys)
		goto err;
	result = CALL_DEPENDENCY(map_update, *p_self, b_without_a_keys);
	Dee_Decref(b_without_a_keys);
	return result;
err_a_and_b_keys:
	Dee_Decref(a_and_b_keys);
err:
	return -1;
err_a_keys:
	Dee_Decref(a_keys);
	goto err;
err_b_keys:
	Dee_Decref(b_keys);
	goto err;
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

map_operator_inplace_xor = {
	if ((REQUIRE_ANY(map_operator_foreach_pair) != &default__map_operator_foreach_pair__unsupported) &&
	    ((REQUIRE(map_removekeys) && REQUIRE_ANY(map_update)) ||
	     (REQUIRE(map_update) && REQUIRE_ANY(map_removekeys))))
		return &$with__map_operator_foreach_pair__and__map_update__and__map_removekeys;
};

