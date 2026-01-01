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
/* deemon.Sequence.resize()                                             */
/************************************************************************/
[[kw, alias(Sequence.resize)]]
__seq_resize__(size_t size, filler=!N) {
	if unlikely(CALL_DEPENDENCY(seq_resize, self, size, filler))
		goto err;
	return_none;
err:
	return NULL;
}



[[wunused]] int
__seq_resize__.seq_resize([[nonnull]] DeeObject *self, size_t newsize,
                          [[nonnull]] DeeObject *filler)
%{unsupported(auto)}
%{$none = 0}
%{$empty = {
	return likely(newsize == 0) ? 0 : default__seq_resize__unsupported(self, newsize, filler);
}}
%{$with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index = {
	size_t oldsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return CALL_DEPENDENCY(seq_operator_delrange_index, self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize);
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_setrange_index = {
	size_t oldsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)oldsize, (Dee_ssize_t)oldsize, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)newsize, (Dee_ssize_t)oldsize, Dee_EmptySeq);
	}
	return 0;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_erase__and__seq_extend = {
	size_t oldsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(oldsize == (size_t)-1)
		goto err;
	if (oldsize < newsize) {
		int result;
		DREF DeeObject *repeat;
		repeat = DeeSeq_RepeatItem(filler, newsize - oldsize);
		if unlikely(!repeat)
			goto err;
		result = CALL_DEPENDENCY(seq_extend, self, repeat);
		Dee_Decref(repeat);
		return result;
	} else if (oldsize > newsize) {
		return CALL_DEPENDENCY(seq_erase, self, newsize, oldsize - newsize);
	}
	return 0;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ "o", newsize, filler);
	if unlikely(!result)
		goto err;
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}


seq_resize = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (REQUIRE_NODEFAULT(seq_erase) && REQUIRE_NODEFAULT(seq_extend))
			return &$with__seq_operator_size__and__seq_erase__and__seq_extend;
		seq_operator_setrange_index = REQUIRE(seq_operator_setrange_index);
		if (seq_operator_setrange_index) {
			DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
			if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall)
				return &$with__seq_operator_size__and__seq_erase__and__seq_extend;
			seq_operator_delrange_index = REQUIRE(seq_operator_delrange_index);
			if (seq_operator_delrange_index != &default__seq_operator_delrange_index__with__seq_operator_setrange_index)
				return &$with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index;
			return &$with__seq_operator_size__and__seq_operator_setrange_index;
		}
	}
};
