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
/* deemon.Sequence.pop()                                          */
/************************************************************************/
[[kw, alias(Sequence.pop)]]
__seq_pop__(Dee_ssize_t index = -1)->?O {
	return CALL_DEPENDENCY(seq_pop, self, index);
}


/* When `index' is negative, count from end of sequence */
[[wunused]] DREF DeeObject *
__seq_pop__.seq_pop([[nonnull]] DeeObject *self, Dee_ssize_t index)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = "default__seq_pop__unsupported"}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase = {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = CALL_DEPENDENCY(seq_operator_size, self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = CALL_DEPENDENCY(seq_operator_getitem_index, self, used_index);
	if likely(result) {
		if unlikely(CALL_DEPENDENCY(seq_erase, self, index, 1))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index = {
	size_t used_index = (size_t)index;
	DREF DeeObject *result;
	if (index < 0) {
		size_t size = CALL_DEPENDENCY(seq_operator_size, self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_index = DeeSeqRange_Clamp_n(index, size);
	}
	result = CALL_DEPENDENCY(seq_operator_getitem_index, self, used_index);
	if likely(result) {
		if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKdSIZ, index);
}

seq_pop = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	/* TODO:
	 * >> local item;
	 * >> do {
	 * >>     local used_index = index;
	 * >>     if (used_index < 0)
	 * >>         used_index = DeeSeqRange_Clamp_n(used_index, Sequence.__size__(this));
	 * >>     item = Sequence.__getitem__(this, used_index);
	 * >> } while (!Set.remove(this, item));
	 * >> return item;
	 *
	 * >> $with__seq_operator_size__and__seq_operator_getitem_index__and__set_remove
	 */
	seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index &&
	    REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = REQUIRE(seq_operator_delitem_index);
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase)
			return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase)
			return &$with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index;
	}
};
