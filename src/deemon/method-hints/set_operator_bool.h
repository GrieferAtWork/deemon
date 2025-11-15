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

/* NOTE: "Set.operator bool()" does *NOT* count potentially unbound items! */

/************************************************************************/
/* deemon.Set.operator bool()                                           */
/************************************************************************/
__set_bool__()->?Dbool {
	int result = CALL_DEPENDENCY(set_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

[[operator(Set: tp_cast.tp_bool)]]
[[wunused]] int
__set_bool__.set_operator_bool([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator size"))}
%{$empty = "default__seq_operator_bool__empty"}
%{$with__set_boundfirst = "default__seq_operator_bool__with__set_boundfirst"}
%{$with__set_boundlast = "default__seq_operator_bool__with__set_boundlast"}
%{$with__set_trygetfirst = "default__seq_operator_bool__with__set_trygetfirst"}
%{$with__set_trygetlast = "default__seq_operator_bool__with__set_trygetlast"}
%{$with__seq_operator_foreach = "default__seq_operator_bool__with__seq_operator_foreach"}
%{$with__seq_operator_foreach_pair = "default__seq_operator_bool__with__seq_operator_foreach_pair"}
%{$with__seq_operator_iter = "default__seq_operator_bool__with__seq_operator_iter"}
%{$with__seq_operator_size = "default__seq_operator_bool__with__seq_operator_size"}
%{$with__seq_operator_sizeob = "default__seq_operator_bool__with__seq_operator_sizeob"}
%{$with__seq_operator_compare_eq = "default__seq_operator_bool__with__seq_operator_compare_eq"}
%{$with__set_operator_compare_eq = "default__seq_operator_bool__with__set_operator_compare_eq"}
%{$with__map_operator_compare_eq = "default__seq_operator_bool__with__map_operator_compare_eq"}
%{$with__set_operator_foreach =  [[prefix(DEFINE_default_seq_bool_with_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(set_operator_foreach, self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}}
%{$with__set_operator_size = {
	size_t size = CALL_DEPENDENCY(set_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return size != 0;
err:
	return -1;
}}
%{$with__set_operator_sizeob = {
	int result;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(set_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	if (DeeObject_AssertTypeExact(sizeob, &DeeInt_Type))
		goto err;
	result = DeeInt_IsZero(sizeob) ? 0 : 1;
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err_r;
	Dee_DecrefNokill(result);
	return DeeBool_IsTrue(result);
err_r:
	Dee_Decref(result);
err:
	return -1;
}


set_operator_bool = {
	DeeMH_seq_operator_bool_t seq_operator_bool = REQUIRE(seq_operator_bool);
	DeeMH_set_operator_size_t set_operator_size;

	/* Try to inherit from "Sequence.operator bool()" */
	if (seq_operator_bool == &default__seq_operator_bool__with__seq_operator_foreach ||
	    seq_operator_bool == &default__seq_operator_bool__with__seq_operator_foreach_pair ||
	    seq_operator_bool == &default__seq_operator_bool__with__seq_operator_iter ||
		seq_operator_bool == &default__seq_operator_bool__with__set_trygetfirst ||
		seq_operator_bool == &default__seq_operator_bool__with__set_trygetlast ||
		seq_operator_bool == &default__seq_operator_bool__with__set_boundfirst ||
		seq_operator_bool == &default__seq_operator_bool__with__set_boundlast) {
		return seq_operator_bool;
	} else if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__)) {
		if (seq_operator_bool == &default__seq_operator_bool__with__seq_operator_size ||
		    seq_operator_bool == &default__seq_operator_bool__with__seq_operator_sizeob ||
		    seq_operator_bool == &default__seq_operator_bool__with__seq_operator_compare_eq ||
		    seq_operator_bool == &default__seq_operator_bool__with__set_operator_compare_eq ||
		    seq_operator_bool == &default__seq_operator_bool__with__map_operator_compare_eq)
		    return seq_operator_bool;

		/* A user-defined "Sequence.operator bool()" can also always be
		 * used when "__seq_getitem_always_bound__" is explicitly enabled. */
		seq_operator_bool = REQUIRE_NODEFAULT(seq_operator_bool);
		if (seq_operator_bool)
			return seq_operator_bool;
	}

	/* Try to implement using "Seq.first" / "Seq.last" */
	if (REQUIRE_NODEFAULT(set_boundfirst))
		return &$with__set_boundfirst;
	if (REQUIRE_NODEFAULT(set_boundlast))
		return &$with__set_boundlast;
	if (REQUIRE_NODEFAULT(set_trygetfirst))
		return &$with__set_trygetfirst;
	if (REQUIRE_NODEFAULT(set_trygetlast))
		return &$with__set_trygetlast;

	/* Fallback: wrap around "Set.operator size()" */
	set_operator_size = REQUIRE(set_operator_size);
	if (set_operator_size == &default__set_operator_size__empty)
		return &$empty;
	if (set_operator_size == &default__set_operator_size__with__set_operator_sizeob)
		return &$with__set_operator_sizeob;
	if (set_operator_size == &default__set_operator_size__with__set_operator_iter ||
	    set_operator_size == &default__set_operator_size__with__set_operator_foreach) {
		DeeMH_set_operator_foreach_t set_operator_foreach = REQUIRE(set_operator_foreach);
		if (set_operator_foreach == &default__set_operator_foreach__empty) {
			return &$empty;
		} else if (set_operator_foreach == &default__set_operator_foreach__with__seq_operator_foreach) {
			DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
			if (seq_operator_foreach == &default__seq_operator_foreach__empty)
				return &$empty;
			if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair ||
			    seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
				return &$with__seq_operator_foreach_pair;
			if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
				goto use__seq_operator_iter;
			return &$with__seq_operator_foreach;
		} else if (set_operator_foreach == &default__set_operator_foreach__with__set_operator_iter ||
		           set_operator_size == &default__set_operator_size__with__set_operator_iter) {
			DeeMH_set_operator_iter_t set_operator_iter = REQUIRE(set_operator_iter);
			if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter) {
				DeeMH_seq_operator_iter_t seq_operator_iter;
use__seq_operator_iter:
				seq_operator_iter = REQUIRE(seq_operator_iter);
				if (seq_operator_iter == &default__seq_operator_iter__empty)
					return &$empty;
				if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem ||
				    seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate ||
				    seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index ||
				    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
				    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
					return &$with__seq_operator_foreach;
				return &$with__seq_operator_iter;
			}
		}
		return &$with__set_operator_foreach;
	}
	if (set_operator_size)
		return &$with__set_operator_size;
};
