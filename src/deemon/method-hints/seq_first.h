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
/* deemon.Sequence.first                                                */
/************************************************************************/

[[getset, alias(Sequence.first)]]
__seq_first__->?O;


%[define(DEFINE_seq_default_getfirst_with_foreach_cb =
#ifndef DEFINED_seq_default_getfirst_with_foreach_cb
#define DEFINED_seq_default_getfirst_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	*(DREF DeeObject **)arg = item;
	return -2;
}
#endif /* !DEFINED_seq_default_getfirst_with_foreach_cb */
)]



/* Try to return the first element of the sequence (returns ITER_DONE if the sequence is empty) */
[[wunused]] DREF DeeObject *
__seq_first__.seq_trygetfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("first"))} %{$empty = ITER_DONE}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_trygetitem_index)]] {
	DREF DeeObject *result;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	result = (*THIS_TYPE->tp_seq->tp_getitem_index_fast)(self, 0);
	if (!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}}
%{$with__seq_operator_trygetitem_index = {
	return CALL_DEPENDENCY(seq_operator_trygetitem_index, self, 0);
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_default_getfirst_with_foreach_cb)]] {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}} {
	DREF DeeObject *result = LOCAL_GETATTR(self);
	if (!result && DeeError_Catch(&DeeError_UnboundAttribute))
		result = ITER_DONE;
	return result;
}


/* Return the first element of the sequence */
[[wunused, getset_member("get")]] DREF DeeObject *
__seq_first__.seq_getfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias(default__seq_trygetfirst__unsupported)}
%{$empty = {
	err_unbound_attribute_string(THIS_TYPE, "first");
	return NULL;
}}
%{$with__seq_operator_getitem_index = {
	return CALL_DEPENDENCY(seq_operator_getitem_index, self, 0);
}}
%{$with__seq_trygetfirst = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetfirst, self);
	if unlikely(result == ITER_DONE) {
		err_unbound_attribute_string(THIS_TYPE, "first");
		result = NULL;
	}
	return result;
}} {
	return LOCAL_GETATTR(self);
}


/* Check if the first element of the sequence is bound */
[[wunused, getset_member("bound")]] int
__seq_first__.seq_boundfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias($empty)} %{$empty = Dee_BOUND_MISSING}
%{$with__seq_operator_bounditem_index = {
	return CALL_DEPENDENCY(seq_operator_bounditem_index, self, 0);
}}
%{$with__seq_trygetfirst = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetfirst, self);
	if (result == ITER_DONE)
		return Dee_BOUND_NO;
	if unlikely(!result)
		return Dee_BOUND_ERR;
	Dee_Decref(result);
	return Dee_BOUND_YES;
}} {
	return LOCAL_BOUNDATTR(self);
}


/* Remove or unbound the first element of the sequence */
[[wunused, getset_member("del")]] int
__seq_first__.seq_delfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported({ return err_seq_unsupportedf(self, "del first"); })}
%{$empty = 0}
%{$with__seq_operator_delitem_index = {
	int result = CALL_DEPENDENCY(seq_operator_delitem_index, self, 0);
	if (result < 0 && DeeError_Catch(&DeeError_IndexError))
		result = 0;
	return result;
}} {
	return LOCAL_DELATTR(self);
}


/* Override the first element of the sequence with "value" */
[[wunused, getset_member("set")]] int
__seq_first__.seq_setfirst([[nonnull]] DeeObject *self,
                           [[nonnull]] DeeObject *value)
%{unsupported({ return err_seq_unsupportedf(self, "first = %r", value); })}
%{$empty = { return err_empty_sequence(self); }}
%{$with__seq_operator_setitem_index = {
	return CALL_DEPENDENCY(seq_operator_setitem_index, self, 0, value);
}} {
	return LOCAL_SETATTR(self, value);
}





seq_trygetfirst = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ) {
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast &&
		    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
	}
	seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_trygetitem_index)
		return &$with__seq_operator_trygetitem_index;
};

seq_getfirst = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_trygetfirst)
		return &$with__seq_trygetfirst;
};

seq_boundfirst = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index)
		return &$with__seq_operator_bounditem_index;
	if (seq_trygetfirst)
		return &$with__seq_trygetfirst;
};

seq_delfirst = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_delitem_index))
			return &$with__seq_operator_delitem_index;
	}
	if (seq_trygetfirst)
		return &$unsupported;
};

seq_setfirst = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = REQUIRE(seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &$empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_setitem_index))
			return &$with__seq_operator_setitem_index;
	}
	if (seq_trygetfirst)
		return &$unsupported;
};
