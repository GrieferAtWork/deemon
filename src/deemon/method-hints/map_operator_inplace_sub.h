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
/* deemon.Mapping.operator -= ()                                        */
/************************************************************************/
__map_inplace_sub__(keys:?X2?DSet?DSequence?S?O)->?. {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_inplace_sub__", &keys))
		goto err;
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(map_operator_inplace_sub, (DeeObject **)&self, keys))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Mapping.OPERATOR_INPLACE_SUB: tp_math->tp_inplace_sub)]]
[[wunused]] int
__map_inplace_sub__.map_operator_inplace_sub([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *keys)
%{unsupported({
	DREF DeeObject *result;
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return (*DeeType_RequireMethodHint(Dee_TYPE(*p_self), map_operator_inplace_and))(p_self, xkeys->si_set);
	}
	result = DeeObject_InvokeMethodHint(map_operator_sub, *p_self, keys);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
})}
%{$empty = "default__map_operator_inplace_sub__unsupported"}
%{$with__map_removekeys = {
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a -= ~b' -> `a &= b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return CALL_DEPENDENCY(map_operator_inplace_and, p_self, xkeys->si_set);
	}
	return CALL_DEPENDENCY(map_removekeys, *p_self, keys);
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

map_operator_inplace_sub = {
	if (REQUIRE(map_removekeys))
		return &$with__map_removekeys;
};

