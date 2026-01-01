/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/************************************************************************/
/* deemon.Sequence.operator *=()                                        */
/************************************************************************/
__seq_inplace_mul__(repeat:?Dint)->?. {
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(seq_operator_inplace_mul, (DeeObject **)&self, repeat))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

[[operator(Sequence: tp_math->tp_inplace_mul)]]
[[wunused]] int
__seq_inplace_mul__.seq_operator_inplace_mul([[nonnull]] DREF DeeObject **__restrict p_lhs,
                                             [[nonnull]] DeeObject *repeat)
%{unsupported_alias("default__seq_operator_inplace_mul__with__seq_operator_mul")}
%{$none = 0}
%{$empty = 0}
%{$with__seq_clear__and__seq_extend = {
	int result;
	DREF DeeObject *extend_with_this;
	size_t repeatval;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0)
		return CALL_DEPENDENCY(seq_clear, *p_lhs);
	if (repeatval == 1)
		return 0;
	extend_with_this = DeeSeq_Repeat(*p_lhs, repeatval - 1);
	if unlikely(!extend_with_this)
		goto err;
	result = CALL_DEPENDENCY(seq_extend, *p_lhs, extend_with_this);
	Dee_Decref_likely(extend_with_this);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_mul = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_mul, *p_lhs, repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(*p_lhs, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_lhs);
	*p_lhs = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

seq_operator_inplace_mul = {
	if (REQUIRE(seq_extend)) {
		DeeMH_seq_clear_t seq_clear = REQUIRE_ANY(seq_clear);
		if (seq_clear == &default__seq_clear__empty)
			return &$empty;
		if (seq_clear != &default__seq_clear__unsupported)
			return &$with__seq_clear__and__seq_extend;
	}
	if (REQUIRE(seq_operator_mul))
		return &$with__seq_operator_mul;
};

