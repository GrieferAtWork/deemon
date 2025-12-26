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
/* deemon.Mapping.operator & ()                                         */
/************************************************************************/
[[alias(Mapping.intersection)]]
__map_and__(keys:?X2?DSet?S?O)->?DMapping {
	return CALL_DEPENDENCY(map_operator_and, self, keys);
}


/* {"a":1, "b":2} & {"a"}   ->   {"a":1} */
[[operator(Mapping: tp_math->tp_and)]]
[[wunused]] DREF DeeObject *
__map_and__.map_operator_and([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *keys)
%{unsupported({
	if (SetInversion_CheckExact(keys)) {
		/* Special case: `a & ~b' -> `a - b' */
		SetInversion *xkeys = (SetInversion *)keys;
		return DeeObject_InvokeMethodHint(map_operator_sub, lhs, xkeys->si_set);
	}
	if (DeeSet_CheckEmpty(keys))
		return_reference_(Dee_EmptyMapping); /* `a & {}' -> `{}' */
	return Dee_AsObject(MapIntersection_New(lhs, keys));
})}
%{$none = return_none}
%{$empty = "default__map_operator_and__unsupported"} {
	return LOCAL_CALLATTR(lhs, 1, &keys);
}
