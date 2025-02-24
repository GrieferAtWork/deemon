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
/* deemon.Set.operator - ()                                             */
/************************************************************************/
[[alias(Set.difference -> "set_difference")]]
__set_sub__(rhs:?X3?DSet?DSequence?S?O)->?DSet {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_sub__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_sub, self, rhs);
err:
	return NULL;
}


/* {"a", "b"} - {"b"}   ->   {"a"} */
[[operator(Set: tp_math->tp_sub)]]
[[wunused]] DREF DeeObject *
__set_sub__.set_operator_sub([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a - ~b' -> `a & b' */
		SetInversion *xrhs = (SetInversion *)rhs;
		return DeeObject_InvokeMethodHint(set_operator_and, lhs, xrhs->si_set);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs); /* `a - {}' -> `a' */
	return (DREF DeeObject *)SetDifference_New(lhs, rhs);
})}
%{$none = return_none}
%{$empty = "default__set_operator_sub__unsupported"} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}
