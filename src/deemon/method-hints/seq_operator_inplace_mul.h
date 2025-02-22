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
/* deemon.Sequence.operator *=()                                        */
/************************************************************************/
__seq_inplace_mul__(repeat:?Dint)->?. {
	DeeObject *repeat;
	if (DeeArg_Unpack(argc, argv, "o:__seq_inplace_mul__", &repeat))
		goto err;
	Dee_Incref(self);
	if unlikely(CALL_DEPENDENCY(seq_operator_inplace_mul, (DeeObject **)&self, repeat))
		goto err_self;
	return self;
err_self:
	Dee_Decref_unlikely(self);
err:
	return NULL;
}

[[operator(Sequence.OPERATOR_INPLACE_MUL: tp_math->tp_inplace_mul)]]
[[wunused]] int
__seq_inplace_mul__.seq_operator_inplace_mul([[nonnull]] DREF DeeObject **__restrict p_self,
                                             [[nonnull]] DeeObject *repeat)
%{unsupported_alias("default__seq_operator_inplace_mul__with__DeeSeq_Repeat")}
%{$empty = 0}
%{$with__seq_clear__and__seq_extend = {
	int result;
	DREF DeeObject *extend_with_this;
	size_t repeatval;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0)
		return CALL_DEPENDENCY(seq_clear, *p_self);
	if (repeatval == 1)
		return 0;
	extend_with_this = DeeSeq_Repeat(*p_self, repeatval - 1);
	if unlikely(!extend_with_this)
		goto err;
	result = CALL_DEPENDENCY(seq_extend, *p_self, extend_with_this);
	Dee_Decref_likely(extend_with_this);
	return result;
err:
	return -1;
}}
%{$with__DeeSeq_Repeat = {
	size_t repeatval;
	DREF DeeObject *result;
	if (DeeObject_AsSize(repeat, &repeatval))
		goto err;
	if (repeatval == 0) {
		result = Dee_EmptySeq;
		Dee_Incref(Dee_EmptySeq);
	} else if (repeatval == 1) {
		return 0;
	} else {
		result = DeeSeq_Repeat(*p_self, repeatval);
		if unlikely(!result)
			goto err;
	}
	Dee_Decref_unlikely(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(*p_self, 1, &repeat);
	if unlikely(!result)
		goto err;
	Dee_Decref(*p_self);
	*p_self = result; /* Inherit reference */
	return 0;
err:
	return -1;
}

seq_operator_inplace_mul = {
	DeeMH_seq_extend_t seq_extend = REQUIRE(seq_extend);
	if (seq_extend) {
		DeeMH_seq_clear_t seq_clear = REQUIRE_ANY(seq_clear);
		if (seq_clear == &default__seq_clear__empty)
			return &$empty;
		if (seq_clear != &default__seq_clear__unsupported)
			return &$with__seq_clear__and__seq_extend;
	}
};

