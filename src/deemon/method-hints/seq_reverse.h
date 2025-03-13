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
/* deemon.Sequence.reverse()                                            */
/************************************************************************/
[[kw, alias(Sequence.reverse)]]
__seq_reverse__(size_t start = 0, size_t end = (size_t)-1) {
	if unlikely(CALL_DEPENDENCY(seq_reverse, self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}



[[wunused]] int
__seq_reverse__.seq_reverse([[nonnull]] DeeObject *self,
                            size_t start, size_t end)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_reversed__and__seq_operator_setrange_index = {
	DREF DeeObject *reversed = CALL_DEPENDENCY(seq_reversed, self, start, end);
	int result = CALL_DEPENDENCY(seq_operator_setrange_index, self, start, end, reversed);
	Dee_Decref(reversed);
	return result;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index = {
	DREF DeeObject *lo_elem, *hi_elem;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, start);
		if unlikely(!lo_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		hi_elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, end - 1);
		if unlikely(!hi_elem && !DeeError_Catch(&DeeError_UnboundItem))
			goto err_lo_elem;
		if (hi_elem) {
			if unlikely(CALL_DEPENDENCY(seq_operator_setitem_index, self, start, hi_elem))
				goto err_lo_elem_hi_elem;
			Dee_Decref(hi_elem);
		} else {
			if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, start))
				goto err_lo_elem;
		}
		if (lo_elem) {
			if unlikely(CALL_DEPENDENCY(seq_operator_setitem_index, self, end - 1, lo_elem))
				goto err_lo_elem;
			Dee_Decref(lo_elem);
		} else {
			if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, end - 1))
				goto err;
		}
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_XDecref(hi_elem);
err_lo_elem:
	Dee_XDecref(lo_elem);
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index = {
	DREF DeeObject *lo_elem, *hi_elem;
	size_t selfsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(selfsize == (size_t)-1)
		goto err;
	if (end > selfsize)
		end = selfsize;
	while ((start + 1) < end) {
		lo_elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, start);
		if unlikely(!lo_elem)
			goto err;
		hi_elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, end - 1);
		if unlikely(!hi_elem)
			goto err_lo_elem;
		if unlikely(CALL_DEPENDENCY(seq_operator_setitem_index, self, start, hi_elem))
			goto err_lo_elem_hi_elem;
		Dee_Decref(hi_elem);
		if unlikely(CALL_DEPENDENCY(seq_operator_setitem_index, self, end - 1, lo_elem))
			goto err_lo_elem;
		Dee_Decref(lo_elem);
		++start;
		--end;
	}
	return 0;
err_lo_elem_hi_elem:
	Dee_Decref(hi_elem);
err_lo_elem:
	Dee_Decref(lo_elem);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably "Dee_None" */
	return 0;
err:
	return -1;
}


seq_reverse = {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if (REQUIRE_NODEFAULT(seq_reversed) &&
	    REQUIRE_ANY(seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &$with__seq_reversed__and__seq_operator_setrange_index;
	seq_operator_setitem_index = REQUIRE(seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &$empty;
		if ((seq_operator_size = REQUIRE_ANY(seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = REQUIRE_ANY(seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			if (REQUIRE_NODEFAULT(seq_operator_delitem_index))
				return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index;
			return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
};
