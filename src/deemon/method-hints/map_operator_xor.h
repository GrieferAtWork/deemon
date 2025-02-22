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
/* deemon.Mapping.operator ^ ()                                         */
/************************************************************************/
[[alias(Mapping.symmetric_difference -> "map_symmetric_difference")]]
__map_xor__(rhs:?X2?M?O?O?S?T2?O?O)->?DMapping {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__map_xor__", &rhs))
		goto err;
	return CALL_DEPENDENCY(map_operator_xor, self, rhs);
err:
	return NULL;
}


/* {"a":1,"b":2} ^ {"a":3, "c":4}   ->   {"b":2, "c":4} */
[[operator(Mapping: tp_math->tp_xor)]]
[[wunused]] DREF DeeObject *
__map_xor__.map_operator_xor([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *rhs)
%{unsupported({
	if (DeeMap_CheckEmpty(rhs))
		return_reference_(lhs);
	return (DREF DeeObject *)MapSymmetricDifference_New(lhs, rhs);
})}
%{$empty = "default__map_operator_xor__unsupported"} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}
