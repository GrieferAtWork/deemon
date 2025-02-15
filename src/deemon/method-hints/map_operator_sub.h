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
/* deemon.Mapping.operator - ()                                         */
/************************************************************************/
[[alias(Mapping.difference -> "map_difference")]]
__map_sub__(keys:?X2?DSet?S?O)->?DMapping {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:__map_sub__", &keys))
		goto err;
	return CALL_DEPENDENCY(map_operator_sub, self, keys);
err:
	return NULL;
}


/* {"a":1, "b":2} - {"a"}   ->   {"b":2} */
[[operator(Mapping.OPERATOR_SUB: tp_math->tp_sub)]]
[[wunused]] DREF DeeObject *
__map_sub__.map_operator_sub([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *keys)
%{unsupported({
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a - ~b' -> `a & b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return DeeObject_InvokeMethodHint(map_operator_and, lhs, xkeys->si_set);
	}
	if (DeeSet_CheckEmpty(keys))
		return_reference_(lhs); /* `a - {}' -> `a' */
	return (DREF DeeObject *)MapDifference_New(lhs, keys);
})}
%{$empty = "default__map_operator_sub__unsupported"} {
	return LOCAL_CALLATTR(lhs, 1, &keys);
}
