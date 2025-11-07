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
/* deemon.Sequence.last                                                 */
/************************************************************************/

[[getset]]
[[alias(Sequence.last)]]
[[alias(Set.last)]]
[[alias(Mapping.last)]]
__seq_last__->?O;


%[define(DEFINE_seq_default_getlast_with_foreach_cb =
#ifndef DEFINED_seq_default_getlast_with_foreach_cb
#define DEFINED_seq_default_getlast_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getlast_with_foreach_cb(void *arg, DeeObject *item) {
	Dee_Incref(item);
	Dee_Decref_unlikely(*(DREF DeeObject **)arg);
	*(DREF DeeObject **)arg = item;
	return 1;
}
#endif /* !DEFINED_seq_default_getlast_with_foreach_cb */
)]



/* Try to return the last element of the sequence (returns ITER_DONE if the sequence is empty) */
[[no_self_invocation_wrapper]]
[[wunused, getset_member("tryget")]] DREF DeeObject *
__seq_last__.seq_trygetlast([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias(default__seq_getlast__unsupported)}
%{$none = return_none}
%{$empty = "default__seq_trygetfirst__empty"}
%{$with__seq_getlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_getlast, self);
	if (!result && (DeeError_Catch(&DeeError_UnboundAttribute) ||
	                DeeError_Catch(&DeeError_IndexError)))
		result = ITER_DONE;
	return result;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DeeObject *result;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	result = (*THIS_TYPE->tp_seq->tp_getitem_index_fast)(self, size - 1);
	if (!result)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return ITER_DONE;
	return CALL_DEPENDENCY(seq_operator_trygetitem_index, self, size - 1);
err:
	return NULL;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_default_getlast_with_foreach_cb)]] {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	result = DeeNone_NewRef();
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_default_getlast_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		return ITER_DONE;
	return NULL;
}} = $with__seq_getlast;


/* Return the last element of the sequence */
[[wunused, getset_member("get")]] DREF DeeObject *
__seq_last__.seq_getlast([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("last"))}
%{$none = return_none}
%{$empty = DeeRT_ErrTUnboundAttrCStr(&DeeSeq_Type, self, "last")}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(!size)
		return default__seq_getlast__empty(self);
	return CALL_DEPENDENCY(seq_operator_getitem_index, self, size - 1);
err:
	return NULL;
}}
%{$with__seq_trygetlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetlast, self);
	if unlikely(result == ITER_DONE)
		return default__seq_getlast__empty(self);
	return result;
}} {
	return LOCAL_GETATTR(self);
}


/* Check if the last element of the sequence is bound */
[[wunused, getset_member("bound")]] int
__seq_last__.seq_boundlast([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias($empty)}
%{$none = Dee_BOUND_YES}
%{$empty = Dee_BOUND_MISSING}
%{$with__seq_operator_size__and__seq_operator_bounditem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (!size)
		return Dee_BOUND_MISSING;
	return CALL_DEPENDENCY(seq_operator_bounditem_index, self, size - 1);
err:
	return -1;
}}
%{$with__seq_trygetlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetlast, self);
	if (result == ITER_DONE)
		return Dee_BOUND_NO;
	if unlikely(!result)
		return Dee_BOUND_ERR;
	Dee_Decref(result);
	return Dee_BOUND_YES;
}} {
	return LOCAL_BOUNDATTR(self);
}


/* Remove or unbound the last element of the sequence */
[[wunused, getset_member("del")]] int
__seq_last__.seq_dellast([[nonnull]] DeeObject *__restrict self)
%{unsupported({ return err_seq_unsupportedf(self, "del last"); })}
%{$none = 0}
%{$empty = 0}
%{$with__seq_operator_size__and__seq_operator_delitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(!size)
		return 0;
	return CALL_DEPENDENCY(seq_operator_delitem_index, self, size - 1);
err:
	return -1;
}} {
	return LOCAL_DELATTR(self);
}


/* Override the last element of the sequence with "value" */
[[wunused, getset_member("set")]] int
__seq_last__.seq_setlast([[nonnull]] DeeObject *self,
                         [[nonnull]] DeeObject *value)
%{unsupported({
	return err_seq_unsupportedf(self, "last = %r", value);
})}
%{$none = 0}
%{$empty = DeeRT_ErrEmptySequence(self)}
%{$with__seq_operator_size__and__seq_operator_setitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_empty;
	return CALL_DEPENDENCY(seq_operator_setitem_index, self, size - 1, value);
err_empty:
	return DeeRT_ErrEmptySequence(self);
err:
	return -1;
}} {
	return LOCAL_SETATTR(self, value);
}





seq_trygetlast = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (REQUIRE_NODEFAULT(seq_getlast))
		return &$with__seq_getlast;
	if (THIS_TYPE->tp_seq &&
	    THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    THIS_TYPE->tp_seq->tp_size)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_trygetitem_index &&
	    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
};

seq_getlast = {
	DeeMH_seq_trygetlast_t seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_trygetlast)
		return &$with__seq_trygetlast;
};

seq_boundlast = {
	DeeMH_seq_trygetlast_t seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_bounditem_index;
	if (seq_trygetlast)
		return &$with__seq_trygetlast;
};

seq_dellast = {
	DeeMH_seq_trygetlast_t seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_delitem_index))
			return &$with__seq_operator_size__and__seq_operator_delitem_index;
	}
	if (seq_trygetlast)
		return &$unsupported;
};

seq_setlast = {
	DeeMH_seq_trygetlast_t seq_trygetlast = REQUIRE(seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &$empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_setitem_index))
			return &$with__seq_operator_size__and__seq_operator_setitem_index;
	}
	if (seq_trygetlast)
		return &$unsupported;
};
