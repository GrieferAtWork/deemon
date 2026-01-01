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
/* deemon.Sequence.cached                                               */
/************************************************************************/
[[getset, alias(Sequence.cached)]]
__seq_cached__->?DSequence;


/* Return the first element of the sequence */
[[wunused, getset_member("get")]] DREF DeeObject *
__seq_cached__.seq_cached([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias(DeeObject_NewRef)}
%{$none = return_none}
%{$empty = "DeeObject_NewRef"}
%{$with__seq_operator_iter = {
	DREF DeeObject *iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	return Dee_AsObject(CachedSeq_WithIter_New(iter));
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}}
%{$with__seq_operator_getitem = {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}}
%{$with__seq_enumerate = {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}}
%{$with__seq_enumerate_index = {
	// TODO
	return default__seq_cached__with__seq_operator_iter(self);
}} {
	return LOCAL_GETATTR(self);
}


seq_cached = {
	/* TODO:
	 * if (REQUIRE_NODEFAULT(map_cached))
	 *     return <deemon>Mapping.cached(this) as Sequence</deemon>;
	 * if (REQUIRE_NODEFAULT(set_cached))
	 *     return <deemon>Set.cached(this) as Sequence</deemon>;
	 */
	DeeMH_seq_enumerate_t seq_enumerate = REQUIRE(seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &$empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_enumerate_index) {
		DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
		if (seq_enumerate_index == &default__seq_enumerate_index__empty)
			return &$empty;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_getitem_index)
			return &$with__seq_operator_getitem;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter)
			goto use_seq_operator_iter;
	} else if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter) {
		DeeMH_seq_operator_iter_t seq_operator_iter;
use_seq_operator_iter:
		seq_operator_iter = REQUIRE(seq_operator_iter);
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index)
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem)
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem)
			return &$with__seq_operator_getitem;
		if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index)
			return &$with__seq_enumerate_index;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate)
			return &$with__seq_enumerate;
		return &$with__seq_operator_iter;
	}
	if (seq_enumerate) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (HAS_TRAIT_NODEFAULT(__seq_getitem_always_bound__))
			goto use_seq_operator_iter;
		seq_operator_size = REQUIRE(seq_operator_size);
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach ||
		    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair ||
		    seq_operator_size == &default__seq_operator_size__with__seq_operator_iter)
			goto use_seq_operator_iter;
		seq_operator_getitem = REQUIRE(seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__empty)
			return &$empty;
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto use_seq_operator_iter;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &$with__seq_operator_size__and__seq_operator_getitem_index;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate ||
			    seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
				return &$with__seq_enumerate_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob) {
			DeeMH_seq_operator_sizeob_t seq_operator_sizeob = REQUIRE(seq_operator_sizeob);
			if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
				return &$empty;
			if (seq_operator_sizeob == &default__seq_operator_sizeob__with__seq_enumerate)
				return &$with__seq_enumerate;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index)
			return &$with__seq_enumerate_index;
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_enumerate)
			return &$with__seq_enumerate;
		if (seq_operator_size && seq_operator_getitem)
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		return &$with__seq_enumerate;
	}
};
