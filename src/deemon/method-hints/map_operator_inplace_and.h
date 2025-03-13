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
/* deemon.Mapping.operator &= ()                                        */
/************************************************************************/
__map_inplace_and__(keys:?X2?DSet?DSequence?S?O)->?. {
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(map_operator_inplace_and, (DeeObject **)&self, keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Mapping: tp_math->tp_inplace_and)]]
[[wunused]] int
__map_inplace_and__.map_operator_inplace_and([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *keys)
%{unsupported({
	DREF DeeObject *result;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_sub))(p_self, xkeys->si_set);
	}
	result = DeeObject_InvokeMethodHint(map_operator_and, *p_self, keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
})}
%{$none = 0}
%{$empty = "default__map_operator_inplace_and__unsupported"}
%{$with__map_operator_foreach_pair__and__map_removekeys = {
	int result;
	DREF DeeObject *a_keys;
	DREF DeeObject *a_keys_without_b_proxy;
	DREF DeeObject *a_keys_without_b;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a &= ~b' -> `a -= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return CALL_DEPENDENCY(map_operator_inplace_sub, p_self, xkeys->si_set);
	}

	/* `a &= {}' -> `(a as Sequence).clear()' */
	if (DeeSet_CheckEmpty(keys))
		return CALL_DEPENDENCY(seq_clear, *p_self);

	/* `a &= b' -> `(a as Mapping).removekeys(((((a as Mapping).keys as Set) - b) as Set).frozen)' */
	a_keys = CALL_DEPENDENCY(map_keys, *p_self);
	if unlikely(!a_keys)
		goto err;
	a_keys_without_b_proxy = DeeObject_InvokeMethodHint(set_operator_sub, a_keys, keys);
	Dee_Decref(a_keys);
	if unlikely(!a_keys_without_b_proxy)
		goto err;
	a_keys_without_b = DeeObject_InvokeMethodHint(set_frozen, a_keys_without_b_proxy);
	Dee_Decref(a_keys_without_b_proxy);
	if unlikely(!a_keys_without_b)
		goto err;
	result = CALL_DEPENDENCY(map_removekeys, *p_self, a_keys_without_b);
	Dee_Decref(a_keys_without_b);
	return result;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(*p_self, 1, &keys);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

map_operator_inplace_and = {
	DeeMH_map_removekeys_t map_removekeys = REQUIRE(map_removekeys);
	if (map_removekeys && REQUIRE_ANY(map_operator_foreach_pair) != &default__map_operator_foreach_pair__unsupported)
		return &$with__map_operator_foreach_pair__and__map_removekeys;
};

