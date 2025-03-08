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
/* deemon.Set.operator + ()                                             */
/************************************************************************/
[[alias(Set.union)]]
__set_add__(rhs:?X3?DSet?DSequence?S?O)->?DSet {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__set_add__", &rhs))
		goto err;
	return CALL_DEPENDENCY(set_operator_add, self, rhs);
err:
	return NULL;
}


/* {"a"} + {"b"}   ->   {"a", "b"} */
[[operator(Set: tp_math->tp_add)]]
[[operator(Set: tp_math->tp_or)]]
[[wunused]] DREF DeeObject *
__set_add__.set_operator_add([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	if (SetInversion_CheckExact(rhs)) {
		/* Special case: `a | ~b' --> `~(~a & b)'
		 * -> Keep the inversion on the outside, since it prevents enumeration. */
		SetInversion *xrhs = (SetInversion *)rhs;
		DREF SetInversion *inv_lhs;
		DREF SetIntersection *intersection;
		if (DeeSet_CheckEmpty(xrhs->si_set))
			return_reference_(lhs);
		inv_lhs = SetInversion_New(lhs);
		if unlikely(!inv_lhs)
			goto err;
		intersection = SetIntersection_New_inherit_b(inv_lhs, xrhs->si_set);
		if unlikely(!intersection)
			goto err;
		return (DREF DeeObject *)SetInversion_New_inherit(intersection);
	}
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs);
	return (DREF DeeObject *)SetUnion_New(lhs, rhs);
err:
	return NULL;
})}
%{$none = return_none}
%{$empty = "default__set_operator_add__unsupported"} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}
