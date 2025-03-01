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
/* deemon.Sequence.operator + ()                                        */
/************************************************************************/
__seq_add__(rhs:?S?O)->?. {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__seq_add__", &rhs))
		goto err;
	return CALL_DEPENDENCY(seq_operator_add, self, rhs);
err:
	return NULL;
}


[[operator(Sequence: tp_math->tp_add)]]
[[wunused]] DREF DeeObject *
__seq_add__.seq_operator_add([[nonnull]] DeeObject *lhs,
                             [[nonnull]] DeeObject *rhs)
%{unsupported_alias("default__seq_operator_add__with__DeeSeq_Concat")}
%{$none = return_none}
%{$empty = return_reference_(rhs)}
%{$with__DeeSeq_Concat = "DeeSeq_Concat"} {
	return LOCAL_CALLATTR(lhs, 1, &rhs);
}

