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
/* deemon.Sequence.operator bool()                                      */
/************************************************************************/
__seq_bool__()->?Dbool {
	int result = CALL_DEPENDENCY(seq_operator_bool, self);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_bool_with_foreach_cb =
#ifndef DEFINED_default_seq_bool_with_foreach_cb
#define DEFINED_default_seq_bool_with_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return -2;
}
#endif /* !DEFINED_default_seq_bool_with_foreach_cb */
)]

%[define(DEFINE_default_seq_bool_with_foreach_pair_cb =
#ifndef DEFINED_default_seq_bool_with_foreach_pair_cb
#define DEFINED_default_seq_bool_with_foreach_pair_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	(void)arg;
	(void)key;
	(void)value;
	return -2;
}
#endif /* !DEFINED_default_seq_bool_with_foreach_pair_cb */
)]

[[wunused]]
[[operator([Sequence, Set, Mapping]: tp_cast.tp_bool)]]
int __seq_bool__.seq_operator_bool([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator bool"))}
%{$none = 0}
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_bool_with_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}}
%{$with__seq_operator_foreach_pair = [[prefix(DEFINE_default_seq_bool_with_foreach_pair_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach_pair, self, &default_seq_bool_with_foreach_pair_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}}
%{$with__seq_operator_iter = {
	size_t skip;
	DREF DeeObject *iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	skip = DeeObject_IterAdvance(iter, 1);
	Dee_Decref_likely(iter);
	ASSERT(skip == 0 || skip == 1 || skip == (size_t)-1);
	return (int)(Dee_ssize_t)skip;
err:
	return -1;
}}
%{$with__seq_operator_size = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return size != 0;
err:
	return -1;
}}
%{$with__seq_operator_sizeob = {
	int result;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	if (DeeObject_AssertTypeExact(sizeob, &DeeInt_Type))
		goto err;
	result = DeeInt_IsZero(sizeob) ? 0 : 1;
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_compare_eq = {
	int result = CALL_DEPENDENCY(seq_operator_compare_eq, self, Dee_EmptySeq);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}}
%{$with__set_operator_compare_eq = {
	int result = CALL_DEPENDENCY(set_operator_compare_eq, self, Dee_EmptySet);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}}
%{$with__map_operator_compare_eq = {
	int result = CALL_DEPENDENCY(map_operator_compare_eq, self, Dee_EmptyMapping);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}}
%{$with__set_trygetfirst = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_trygetfirst, self);
	if (result == ITER_DONE)
		return 0;
	if unlikely(!result)
		return -1;
	Dee_Decref(result);
	return 1;
}}
%{$with__set_trygetlast = {
	DREF DeeObject *result = CALL_DEPENDENCY(set_trygetlast, self);
	if (result == ITER_DONE)
		return 0;
	if unlikely(!result)
		return -1;
	Dee_Decref(result);
	return 1;
}}
%{$with__set_boundfirst = {
	int status = CALL_DEPENDENCY(set_boundfirst, self);
	return Dee_BOUND_ISERR(status) ? -1 : Dee_BOUND_ISBOUND(status);
}}
%{$with__set_boundlast = {
	int status = CALL_DEPENDENCY(set_boundlast, self);
	return Dee_BOUND_ISERR(status) ? -1 : Dee_BOUND_ISBOUND(status);
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

seq_operator_bool = {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq;
	DeeMH_set_operator_compare_eq_t set_operator_compare_eq;
	DeeMH_map_operator_compare_eq_t map_operator_compare_eq;
	unsigned int seq_class = SEQ_CLASS;
	if (seq_class == Dee_SEQCLASS_SET) {
		DeeMH_set_operator_bool_t set_operator_bool;
		if ((set_operator_bool = REQUIRE_NODEFAULT(set_operator_bool)) != NULL)
			return set_operator_bool;
	}

	seq_operator_compare_eq = REQUIRE(seq_operator_compare_eq);
	if (seq_operator_compare_eq) {
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
			return &$empty;
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_foreach)
			goto use_size; /* return &$with__seq_operator_foreach; */
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index ||
		    seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index)
			goto use_size; /* return &$with__seq_operator_size; */
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem)
			goto use_size; /* return &$with__seq_operator_sizeob; */
		return &$with__seq_operator_compare_eq;
	}

	set_operator_compare_eq = REQUIRE(set_operator_compare_eq);
	if (set_operator_compare_eq) {
		if (set_operator_compare_eq == &default__set_operator_compare_eq__empty)
			return &$empty;
		if (set_operator_compare_eq == &default__set_operator_compare_eq__with__set_operator_foreach)
			goto use_size; /* return &$with__seq_operator_foreach; */
		return &$with__set_operator_compare_eq;
	}

	map_operator_compare_eq = REQUIRE(map_operator_compare_eq);
	if (map_operator_compare_eq) {
		if (map_operator_compare_eq == &default__map_operator_compare_eq__empty)
			return &$empty;
		if (map_operator_compare_eq == &default__map_operator_compare_eq__with__map_operator_foreach_pair)
			goto use_size; /* return &$with__seq_operator_foreach; */
		return &$with__map_operator_compare_eq;
	}

use_size:
	seq_operator_size = REQUIRE(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
		return &$with__seq_operator_sizeob;
	if ((seq_operator_size == &default__seq_operator_size__with__seq_operator_iter ||
	     seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach ||
	     seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair) &&
	    (seq_class == Dee_SEQCLASS_SET || HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))) {
		if (REQUIRE_NODEFAULT(set_boundfirst))
			return &$with__set_boundfirst;
		if (REQUIRE_NODEFAULT(set_boundlast))
			return &$with__set_boundlast;
		if (REQUIRE_NODEFAULT(set_trygetfirst))
			return &$with__set_trygetfirst;
		if (REQUIRE_NODEFAULT(set_trygetlast))
			return &$with__set_trygetlast;
	}
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair)
		return &$with__seq_operator_foreach_pair;
	if (seq_operator_size)
		return &$with__seq_operator_size;
};
