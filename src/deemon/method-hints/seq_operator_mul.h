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
/* deemon.Sequence.operator * ()                                        */
/************************************************************************/
__seq_mul__(repeat:?Dint)->?. {
	DeeObject *repeat;
	if (DeeArg_Unpack(argc, argv, "o:__seq_mul__", &repeat))
		goto err;
	return CALL_DEPENDENCY(seq_operator_mul, self, repeat);
err:
	return NULL;
}

[[operator(Sequence: tp_math->tp_mul)]]
[[wunused]] DREF DeeObject *
__seq_mul__.seq_operator_mul([[nonnull]] DeeObject *self,
                             [[nonnull]] DeeObject *repeat)
%{unsupported_alias("default__seq_operator_mul__with__DeeSeq_Repeat")}
%{$none = return_none}
%{$empty = return_empty_seq}
%{$with__DeeSeq_Repeat = {
	size_t repeatval;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0)
		return_empty_seq;
	if (repeatval == 1)
		return_reference_(self);
	return DeeSeq_Repeat(self, repeatval);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &repeat);
}
