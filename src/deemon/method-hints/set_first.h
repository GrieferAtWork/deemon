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
/* deemon.Set.first                                                     */
/************************************************************************/

[[getset]]
[[alias(Set.first)]]
[[alias(Mapping.first)]]
__set_first__->?O;


/* Try to return the first element of the sequence (returns ITER_DONE if the sequence is empty) */
[[no_self_invocation_wrapper]]
[[wunused, getset_member("tryget")]] DREF DeeObject *
__set_first__.set_trygetfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias(default__set_getfirst__unsupported)}
%{$none = return_none}
%{$empty = ITER_DONE}
%{$with__set_getfirst = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_getfirst, self);
	if (!result && (DeeError_Catch(&DeeError_UnboundAttribute) ||
	                DeeError_Catch(&DeeError_IndexError)))
		result = ITER_DONE;
	return result;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast_ab = "default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast"}
%{$with__seq_operator_trygetitem_index_ab = "default__seq_trygetfirst__with__seq_operator_trygetitem_index"}
%{$with__seq_operator_foreach = "default__seq_trygetfirst__with__seq_operator_foreach"}
= $with__set_getfirst;




/* Return the first element of the sequence */
[[wunused, getset_member("get")]] DREF DeeObject *
__set_first__.set_getfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("first"))}
%{$none = return_none}
%{$empty = DeeRT_ErrTUnboundAttrCStr(&DeeSet_Type, self, "first")}
%{$with__seq_operator_getitem_index_ab = "default__seq_getfirst__with__seq_operator_getitem_index"}
%{$with__set_trygetfirst = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_trygetfirst, self);
	if unlikely(result == ITER_DONE)
		return default__set_getfirst__empty(self);
	return result;
}} {
	return LOCAL_GETATTR(self);
}


/* Check if the first element of the sequence is bound */
[[wunused, getset_member("bound")]] int
__set_first__.set_boundfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias($empty)}
%{$none = Dee_BOUND_YES}
%{$empty = Dee_BOUND_MISSING}
%{$with__set_operator_bool = "default__seq_boundfirst__with__set_operator_bool"}
{
	return LOCAL_BOUNDATTR(self);
}


/* Remove or unbound the first element of the sequence */
[[wunused, getset_member("del")]] int
__set_first__.set_delfirst([[nonnull]] DeeObject *__restrict self)
%{unsupported({ return err_seq_unsupportedf(self, "del first"); })}
%{$none = 0}
%{$empty = 0}
%{$with__seq_operator_delitem_index = "default__seq_delfirst__with__seq_operator_delitem_index"}
%{$with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_delitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size) {
		size_t i;
		PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
		for (i = 0; i < size; ++i) {
			int status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
			switch (status) {
			case Dee_BOUND_YES:
				return CALL_DEPENDENCY(seq_operator_delitem_index, self, i);
			case Dee_BOUND_NO:
				break;
			case Dee_BOUND_MISSING:
				goto done;
			case Dee_BOUND_ERR:
				goto err;
			default: __builtin_unreachable();
			}
			if (DeeThread_CheckInterrupt())
				goto err;
		}
	}
done:
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_bounditem_index__seq_operator_delitem_index = {
	size_t i;
	PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
	for (i = 0;; ++i) {
		int status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
		switch (status) {
		case Dee_BOUND_YES:
			return CALL_DEPENDENCY(seq_operator_delitem_index, self, i);
		case Dee_BOUND_NO:
			break;
		case Dee_BOUND_MISSING:
			goto done;
		case Dee_BOUND_ERR:
			goto err;
		default: __builtin_unreachable();
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return 0;
err:
	return -1;
}}
%{$with__set_trygetfirst__set_remove = {
	int remove_status;
	DREF DeeObject *first = CALL_DEPENDENCY(set_trygetfirst, self);
	if (!ITER_ISOK(first))
		return first == ITER_DONE ? 0 : -1;
	remove_status = CALL_DEPENDENCY(set_remove, self, first);
	Dee_Decref(first);
	if unlikely(remove_status < 0)
		goto err;
	return 0;
err:
	return -1;
}} {
	return LOCAL_DELATTR(self);
}


/* Override the first element of the sequence with "value" */
[[wunused, getset_member("set")]] int
__set_first__.set_setfirst([[nonnull]] DeeObject *self,
                           [[nonnull]] DeeObject *value)
%{unsupported({
	return err_seq_unsupportedf(self, "first = %r", value);
})}
%{$none = 0}
%{$empty = DeeRT_ErrEmptySequence(self)}
%{$with__seq_operator_setitem_index = "default__seq_setfirst__with__seq_operator_setitem_index"}
%{$with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_setitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size) {
		size_t i;
		PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
		for (i = 0; i < size; ++i) {
			int status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
			switch (status) {
			case Dee_BOUND_YES:
				return CALL_DEPENDENCY(seq_operator_setitem_index, self, i, value);
			case Dee_BOUND_NO:
				break;
			case Dee_BOUND_MISSING:
				goto done;
			case Dee_BOUND_ERR:
				goto err;
			default: __builtin_unreachable();
			}
			if (DeeThread_CheckInterrupt())
				goto err;
		}
	}
done:
	return DeeRT_ErrEmptySequence(self);
err:
	return -1;
}}
%{$with__seq_operator_bounditem_index__seq_operator_setitem_index = {
	size_t i;
	PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
	for (i = 0;; ++i) {
		int status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
		switch (status) {
		case Dee_BOUND_YES:
			return CALL_DEPENDENCY(seq_operator_setitem_index, self, i, value);
		case Dee_BOUND_NO:
			break;
		case Dee_BOUND_MISSING:
			goto done;
		case Dee_BOUND_ERR:
			goto err;
		default: __builtin_unreachable();
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return DeeRT_ErrEmptySequence(self);
err:
	return -1;
}} {
	return LOCAL_SETATTR(self, value);
}





set_trygetfirst = {
	DeeMH_seq_trygetfirst_t seq_trygetfirst;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (REQUIRE_NODEFAULT(set_getfirst))
		return &$with__set_getfirst;
	if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__) &&
	    (seq_trygetfirst = REQUIRE_NODEFAULT(seq_trygetfirst)) != NULL)
		return seq_trygetfirst;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach) {
		if (seq_operator_foreach == &default__seq_operator_foreach__empty)
			return &$empty;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast &&
		    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
			return &$with__seq_operator_size__and__operator_getitem_index_fast_ab;
		if ((seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index ||
		     seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index ||
		     seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem ||
		     seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem_index ||
		     seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem) &&
		    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
			return &$with__seq_operator_trygetitem_index_ab;
		return &$with__seq_operator_foreach;
	}
};

set_getfirst = {
	DeeMH_set_trygetfirst_t set_trygetfirst = REQUIRE(set_trygetfirst);
	if (set_trygetfirst == &default__set_trygetfirst__empty)
		return &$empty;
	if (set_trygetfirst == &default__set_trygetfirst__with__seq_operator_trygetitem_index_ab)
		return &$with__seq_operator_getitem_index_ab;
	if (set_trygetfirst == &default__set_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast_ab)
		return &$with__set_trygetfirst;
	if (set_trygetfirst) {
		if (set_trygetfirst == REQUIRE_NODEFAULT(seq_trygetfirst)) {
			DeeMH_seq_getfirst_t seq_getfirst = REQUIRE_NODEFAULT(seq_getfirst);
			if (seq_getfirst)
				return seq_getfirst;
		}
		return &$with__set_trygetfirst;
	}
};

set_boundfirst = {
	DeeMH_set_operator_bool_t set_operator_bool;
	DeeMH_set_boundlast_t set_boundlast;
	if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__)) {
		DeeMH_seq_boundfirst_t seq_boundfirst;
		DeeMH_seq_boundlast_t seq_boundlast;
		if ((seq_boundfirst = REQUIRE_NODEFAULT(seq_boundfirst)) != NULL)
			return seq_boundfirst;
		if ((seq_boundlast = REQUIRE_NODEFAULT(seq_boundlast)) != NULL)
			return seq_boundlast;
	}
	if ((set_boundlast = REQUIRE_NODEFAULT(set_boundlast)) != NULL)
		return set_boundlast;
	set_operator_bool = REQUIRE(set_operator_bool);
	if (set_operator_bool) {
		if (set_operator_bool == &default__set_operator_bool__empty)
			return &$empty;
		return &$with__set_operator_bool;
	}
};

set_delfirst = {
	DeeMH_set_trygetfirst_t set_trygetfirst = REQUIRE(set_trygetfirst);
	if (set_trygetfirst == &default__set_trygetfirst__empty)
		return &$empty;
	if (set_trygetfirst == &default__set_trygetfirst__with__seq_operator_trygetitem_index_ab ||
	    set_trygetfirst == &default__set_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast_ab) {
		if (REQUIRE(seq_operator_delitem_index))
			return &$with__seq_operator_delitem_index;
	} else if (set_trygetfirst) {
		if (set_trygetfirst == REQUIRE_NODEFAULT(seq_trygetfirst)) {
			DeeMH_seq_delfirst_t seq_delfirst = REQUIRE_NODEFAULT(seq_delfirst);
			if (seq_delfirst)
				return seq_delfirst;
		}
		if (REQUIRE_NODEFAULT(set_remove))
			return &$with__set_trygetfirst__set_remove;
		if (REQUIRE(seq_operator_bounditem_index) &&
		    REQUIRE(seq_operator_delitem_index)) {
			DeeMH_seq_operator_size_t seq_operator_size = REQUIRE(seq_operator_size);
			if (seq_operator_size) {
				if (seq_operator_size == &default__seq_operator_size__empty)
					return &$empty;
				if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
				    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach_pair &&
				    seq_operator_size != &default__seq_operator_size__with__map_enumerate &&
				    seq_operator_size != &default__seq_operator_size__with__seq_operator_iter &&
				    seq_operator_size != &default__seq_operator_size__with__seq_enumerate_index)
					return &$with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_delitem_index;
			}
			return &$with__seq_operator_bounditem_index__seq_operator_delitem_index;
		}
	}
	if (set_trygetfirst)
		return &$unsupported;
};

set_setfirst = {
	DeeMH_set_trygetfirst_t set_trygetfirst = REQUIRE(set_trygetfirst);
	if (set_trygetfirst == &default__set_trygetfirst__empty)
		return &$empty;
	if (set_trygetfirst == &default__set_trygetfirst__with__seq_operator_trygetitem_index_ab ||
	    set_trygetfirst == &default__set_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast_ab) {
		if (REQUIRE(seq_operator_setitem_index))
			return &$with__seq_operator_setitem_index;
	} else if (set_trygetfirst) {
		if (set_trygetfirst == REQUIRE_NODEFAULT(seq_trygetfirst)) {
			DeeMH_seq_setfirst_t seq_setfirst = REQUIRE_NODEFAULT(seq_setfirst);
			if (seq_setfirst)
				return seq_setfirst;
		}
		if (REQUIRE(seq_operator_bounditem_index) &&
		    REQUIRE(seq_operator_setitem_index)) {
			DeeMH_seq_operator_size_t seq_operator_size = REQUIRE(seq_operator_size);
			if (seq_operator_size) {
				if (seq_operator_size == &default__seq_operator_size__empty)
					return &$empty;
				if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
				    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach_pair &&
				    seq_operator_size != &default__seq_operator_size__with__map_enumerate &&
				    seq_operator_size != &default__seq_operator_size__with__seq_operator_iter &&
				    seq_operator_size != &default__seq_operator_size__with__seq_enumerate_index)
					return &$with__seq_operator_size__and__seq_operator_bounditem_index__seq_operator_setitem_index;
			}
			return &$with__seq_operator_bounditem_index__seq_operator_setitem_index;
		}
	}
	if (set_trygetfirst)
		return &$unsupported;
};
