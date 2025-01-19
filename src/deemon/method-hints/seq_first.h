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
/* deemon.Sequence.operator bool()                                      */
/************************************************************************/

[[getset, alias(Sequence.first)]]
__seq_first__->?O;

[[wunused]] DREF DeeObject *
__seq_first__.seq_trygetfirst([[nonnull]] DeeObject *self)
%{unsupported(auto("first"))} %{$empty = ITER_DONE}
%{$with__seq_operator_foreach = {
	// TODO
}} {
	DREF DeeObject *result = LOCAL_GETATTR(self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}

[[wunused, getset_member("get")]] DREF DeeObject *
__seq_first__.seq_getfirst([[nonnull]] DeeObject *self)
%{unsupported_alias(default__seq_trygetfirst__unsupported)}
%{$empty = {
	err_unbound_attribute_string(Dee_TYPE(self), "first");
	return NULL;
}}
%{$with__seq_operator_foreach = {
	// TODO
}} {
	return LOCAL_GETATTR(self);
}

[[wunused, getset_member("bound")]] int
__seq_first__.seq_boundfirst([[nonnull]] DeeObject *self)
%{unsupported_alias($empty)} %{$empty = Dee_BOUND_MISSING} {
	return LOCAL_BOUNDATTR(self);
}

[[wunused, getset_member("del")]] int
__seq_first__.seq_delfirst([[nonnull]] DeeObject *self)
%{unsupported({ return err_seq_unsupportedf(self, "del first"); })}
%{$empty = 0} {
	return LOCAL_DELATTR(self);
}

[[wunused, getset_member("set")]] int
__seq_first__.seq_setfirst([[nonnull]] DeeObject *self, [[nonnull]] DeeObject *value)
%{unsupported({ return err_seq_unsupportedf(self, "first = %r", value); })}
%{$empty = { return err_empty_sequence(self); }} {
	return LOCAL_SETATTR(self, value);
}


seq_getfirst = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_trygetfirst = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_delfirst = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$unsupported;
};

seq_setfirst = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$unsupported;
};
