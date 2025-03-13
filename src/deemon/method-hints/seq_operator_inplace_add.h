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
/* deemon.Sequence.operator +=()                                        */
/************************************************************************/
__seq_inplace_add__(rhs:?S?O)->?. {
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(seq_operator_inplace_add, (DeeObject **)&self, rhs))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}



[[operator(Sequence: tp_math->tp_inplace_add)]]
[[wunused]] int
__seq_inplace_add__.seq_operator_inplace_add([[nonnull]] DREF DeeObject **__restrict p_lhs,
                                             [[nonnull]] DeeObject *rhs)
%{unsupported_alias("default__seq_operator_inplace_add__with__seq_operator_add")}
%{$with__seq_extend = {
	return CALL_DEPENDENCY(seq_extend, *p_lhs, rhs);
}}
%{$none = 0}
%{$empty = "$with__seq_operator_add"}
%{$with__seq_operator_add = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_add, *p_lhs, rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(*p_lhs, 1, &rhs);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

seq_operator_inplace_add = {
	if (REQUIRE(seq_extend))
		return &$with__seq_extend;
	if (REQUIRE(seq_operator_add))
		return &$with__seq_operator_add;
};

