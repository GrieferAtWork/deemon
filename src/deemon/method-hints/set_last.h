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
/* deemon.Set.last                                                     */
/************************************************************************/

[[getset]]
[[alias(Set.last)]]
[[alias(Mapping.last)]]
__set_last__->?O;


/* Try to return the last element of the sequence (returns ITER_DONE if the sequence is empty) */
[[no_self_invocation_wrapper]]
[[wunused, getset_member("tryget")]] DREF DeeObject *
__set_last__.set_trygetlast([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias(default__set_getlast__unsupported)}
%{$none = return_none}
%{$empty = ITER_DONE}
%{$with__set_getlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_getlast, self);
	if (!result && (DeeError_Catch(&DeeError_UnboundAttribute) ||
	                DeeError_Catch(&DeeError_IndexError)))
		result = ITER_DONE;
	return result;
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast_ab = "default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast"}
%{$with__seq_operator_size__and__operator_getitem_index_fast = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *result;
		--size;
		result = (*THIS_TYPE->tp_seq->tp_getitem_index_fast)(self, size - 1);
		if (result)
			return result;
	}
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index_ab = "default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index"}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size) {
		size_t i = size;
		PRELOAD_DEPENDENCY(seq_operator_trygetitem_index);
		do {
			DREF DeeObject *result;
			--i;
			result = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
			if (result != ITER_DONE)
				return result;
			if (DeeThread_CheckInterrupt())
				goto err;
		} while (i);
	}
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__seq_operator_sizeob__and__seq_operator_trygetitem_ab = "default__seq_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem"}
%{$with__seq_operator_sizeob__and__seq_operator_trygetitem = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem);
	DREF DeeObject *result = ITER_DONE;
	DREF DeeObject *size = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!size)
		goto err;
	for (;;) {
		int temp = DeeObject_Bool(size);
		if unlikely(temp < 0)
			goto err_size;
		if (!temp)
			break;
		if (DeeObject_Dec(&size))
			goto err_size;
		result = CALL_DEPENDENCY(seq_operator_trygetitem, self, size);
		if (result != ITER_DONE)
			break;
		if (DeeThread_CheckInterrupt())
			goto err_size;
	}
	Dee_Decref(size);
	return result;
err_size:
	Dee_Decref(size);
err:
	return NULL;
}}
%{$with__seq_operator_foreach = "default__seq_trygetlast__with__seq_operator_foreach"}
= $with__set_getlast;




/* Return the last element of the sequence */
[[wunused, getset_member("get")]] DREF DeeObject *
__set_last__.set_getlast([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("last"))}
%{$none = return_none}
%{$empty = DeeRT_ErrTUnboundAttrCStr(&DeeSet_Type, self, "last")}
%{$with__seq_operator_size__and__seq_operator_getitem_index_ab = "default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index"}
%{$with__seq_operator_sizeob__and__seq_operator_getitem_ab = "default__seq_getlast__with__seq_operator_sizeob__and__seq_operator_getitem"}
%{$with__set_trygetlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_trygetlast, self);
	if unlikely(result == ITER_DONE)
		return default__set_getlast__empty(self);
	return result;
}} {
	return LOCAL_GETATTR(self);
}


/* Check if the last element of the sequence is bound */
[[wunused, getset_member("bound")]] int
__set_last__.set_boundlast([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias($empty)}
%{$none = Dee_BOUND_YES}
%{$empty = Dee_BOUND_MISSING}
%{$with__set_operator_bool = "default__seq_boundlast__with__set_operator_bool"}
{
	return LOCAL_BOUNDATTR(self);
}


/* Remove or unbound the last element of the sequence */
[[wunused, getset_member("del")]] int
__set_last__.set_dellast([[nonnull]] DeeObject *__restrict self)
%{unsupported({ return err_seq_unsupportedf(self, "del last"); })}
%{$none = 0}
%{$empty = 0}
%{$with__seq_operator_size__and__seq_operator_delitem_index_ab = "default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index"}
%{$with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_delitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size) {
		size_t i = size;
		PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
		do {
			int status;
			--i;
			status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
			switch (status) {
			case Dee_BOUND_YES:
				return CALL_DEPENDENCY(seq_operator_delitem_index, self, i);
			case Dee_BOUND_NO:
			case Dee_BOUND_MISSING:
				break;
			case Dee_BOUND_ERR:
				goto err;
			default: __builtin_unreachable();
			}
			if (DeeThread_CheckInterrupt())
				goto err;
		} while (i);
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_delitem_ab = "default__seq_dellast__with__seq_operator_sizeob__and__seq_operator_delitem"}
%{$with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_delitem = {
	int result = 0;
	PRELOAD_DEPENDENCY(seq_operator_bounditem);
	DREF DeeObject *size = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!size)
		goto err;
	for (;;) {
		int temp = DeeObject_Bool(size);
		if unlikely(temp < 0)
			goto err_size;
		if (!temp)
			break;
		if (DeeObject_Dec(&size))
			goto err_size;
		temp = CALL_DEPENDENCY(seq_operator_bounditem, self, size);
		switch (temp) {
		case Dee_BOUND_YES:
			result = CALL_DEPENDENCY(seq_operator_delitem, self, size);
			goto done;
		case Dee_BOUND_NO:
		case Dee_BOUND_MISSING:
			break;
		case Dee_BOUND_ERR:
			goto err;
		default: __builtin_unreachable();
		}
		if (DeeThread_CheckInterrupt())
			goto err_size;
	}
done:
	Dee_Decref(size);
	return result;
err_size:
	Dee_Decref(size);
err:
	return -1;
}}
%{$with__set_trygetlast__set_remove = {
	int remove_status;
	DREF DeeObject *last = CALL_DEPENDENCY(set_trygetlast, self);
	if (!ITER_ISOK(last))
		return last == ITER_DONE ? 0 : -1;
	remove_status = CALL_DEPENDENCY(set_remove, self, last);
	Dee_Decref(last);
	if unlikely(remove_status < 0)
		goto err;
	return 0;
err:
	return -1;
}} {
	return LOCAL_DELATTR(self);
}


/* Override the last element of the sequence with "value" */
[[wunused, getset_member("set")]] int
__set_last__.set_setlast([[nonnull]] DeeObject *self,
                         [[nonnull]] DeeObject *value)
%{unsupported({
	return err_seq_unsupportedf(self, "last = %r", value);
})}
%{$none = 0}
%{$empty = DeeRT_ErrEmptySequence(self)}
%{$with__seq_operator_size__and__seq_operator_setitem_index_ab = "default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index"}
%{$with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_setitem_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size) {
		size_t i = size;
		PRELOAD_DEPENDENCY(seq_operator_bounditem_index);
		do {
			int status;
			--i;
			status = CALL_DEPENDENCY(seq_operator_bounditem_index, self, i);
			switch (status) {
			case Dee_BOUND_YES:
				return CALL_DEPENDENCY(seq_operator_setitem_index, self, i, value);
			case Dee_BOUND_NO:
			case Dee_BOUND_MISSING:
				break;
			case Dee_BOUND_ERR:
				goto err;
			default: __builtin_unreachable();
			}
			if (DeeThread_CheckInterrupt())
				goto err;
		} while (i);
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_setitem_ab = "default__seq_setlast__with__seq_operator_sizeob__and__seq_operator_setitem"}
%{$with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_setitem = {
	int result = 0;
	PRELOAD_DEPENDENCY(seq_operator_bounditem);
	DREF DeeObject *size = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!size)
		goto err;
	for (;;) {
		int temp = DeeObject_Bool(size);
		if unlikely(temp < 0)
			goto err_size;
		if (!temp)
			break;
		if (DeeObject_Dec(&size))
			goto err_size;
		temp = CALL_DEPENDENCY(seq_operator_bounditem, self, size);
		switch (temp) {
		case Dee_BOUND_YES:
			result = CALL_DEPENDENCY(seq_operator_setitem, self, size, value);
			goto done;
		case Dee_BOUND_NO:
		case Dee_BOUND_MISSING:
			break;
		case Dee_BOUND_ERR:
			goto err;
		default: __builtin_unreachable();
		}
		if (DeeThread_CheckInterrupt())
			goto err_size;
	}
done:
	Dee_Decref(size);
	return result;
err_size:
	Dee_Decref(size);
err:
	return -1;
}} {
	return LOCAL_SETATTR(self, value);
}





set_trygetlast = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	DeeMH_seq_trygetlast_t seq_trygetlast;
	if (REQUIRE_NODEFAULT(set_getlast))
		return &$with__set_getlast;
	if ((seq_trygetlast = REQUIRE_NODEFAULT(seq_trygetlast)) != NULL &&
	    HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
		return seq_trygetlast;
	if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast &&
	    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported) {
		if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
			return &$with__seq_operator_size__and__operator_getitem_index_fast_ab;
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	}
	seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_trygetitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob) {
				if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
					return $with__seq_operator_sizeob__and__seq_operator_trygetitem_ab;
				return $with__seq_operator_sizeob__and__seq_operator_trygetitem;
			}
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
				DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem) {
					if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
						return $with__seq_operator_sizeob__and__seq_operator_trygetitem_ab;
					return $with__seq_operator_sizeob__and__seq_operator_trygetitem;
				}
			}
			if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
				return &$with__seq_operator_size__and__seq_operator_trygetitem_index_ab;
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
	}
	if (REQUIRE(seq_operator_foreach))
		return &$with__seq_operator_foreach;
};

set_getlast = {
	DeeMH_set_trygetlast_t set_trygetlast = REQUIRE(set_trygetlast);
	if (set_trygetlast == &default__set_trygetlast__empty)
		return &$empty;
	if (set_trygetlast == &default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index_ab)
		return &$with__seq_operator_size__and__seq_operator_getitem_index_ab;
	if (set_trygetlast == &default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem_ab)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem_ab;
	if (set_trygetlast) {
		if (set_trygetlast == REQUIRE_NODEFAULT(seq_trygetlast)) {
			DeeMH_seq_getlast_t seq_getlast = REQUIRE_NODEFAULT(seq_getlast);
			if (seq_getlast)
				return seq_getlast;
		}
		return &$with__set_trygetlast;
	}
};

set_boundlast = {
	DeeMH_set_boundfirst_t set_boundfirst = REQUIRE(set_boundfirst);
	if (set_boundfirst)
		return set_boundfirst;
};

set_dellast = {
	DeeMH_set_trygetlast_t set_trygetlast = REQUIRE(set_trygetlast);
	if (set_trygetlast == &default__set_trygetlast__empty)
		return &$empty;
	if (set_trygetlast == &default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index_ab) {
		if (REQUIRE(seq_operator_delitem_index))
			return &$with__seq_operator_size__and__seq_operator_delitem_index_ab;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_bounditem_index) && REQUIRE(seq_operator_delitem_index))
			return &$with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_delitem_index;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem_ab) {
		if (REQUIRE(seq_operator_delitem))
			return &$with__seq_operator_sizeob__and__seq_operator_delitem_ab;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem) {
		if (REQUIRE(seq_operator_bounditem) && REQUIRE(seq_operator_delitem))
			return &$with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_delitem;
	}
	if (set_trygetlast) {
		if (set_trygetlast == REQUIRE_NODEFAULT(seq_trygetlast)) {
			DeeMH_seq_dellast_t seq_dellast = REQUIRE_NODEFAULT(seq_dellast);
			if (seq_dellast)
				return seq_dellast;
		}
		if (REQUIRE_NODEFAULT(set_remove))
			return &$with__set_trygetlast__set_remove;
		return &$unsupported;
	}
};

set_setlast = {
	DeeMH_set_trygetlast_t set_trygetlast = REQUIRE(set_trygetlast);
	if (set_trygetlast == &default__set_trygetlast__empty)
		return &$empty;
	if (set_trygetlast == &default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index_ab) {
		if (REQUIRE(seq_operator_setitem_index))
			return &$with__seq_operator_size__and__seq_operator_setitem_index_ab;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if (REQUIRE(seq_operator_bounditem_index) && REQUIRE(seq_operator_setitem_index))
			return &$with__seq_operator_size__and__seq_operator_bounditem_index__and__seq_operator_setitem_index;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem_ab) {
		if (REQUIRE(seq_operator_setitem))
			return &$with__seq_operator_sizeob__and__seq_operator_setitem_ab;
	} else if (set_trygetlast == &default__set_trygetlast__with__seq_operator_sizeob__and__seq_operator_trygetitem) {
		if (REQUIRE(seq_operator_bounditem) && REQUIRE(seq_operator_setitem))
			return &$with__seq_operator_sizeob__and__seq_operator_bounditem__and__seq_operator_setitem;
	}
	if (set_trygetlast) {
		if (set_trygetlast == REQUIRE_NODEFAULT(seq_trygetlast)) {
			DeeMH_seq_setlast_t seq_setlast = REQUIRE_NODEFAULT(seq_setlast);
			if (seq_setlast)
				return seq_setlast;
		}
		return &$unsupported;
	}
};
