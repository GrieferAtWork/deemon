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
/* deemon.Iterator.prev()                                               */
/************************************************************************/
[[alias(Iterator.prev)]]
__iter_prev__(def?)->?O {
	DREF DeeObject *result = CALL_DEPENDENCY(iter_prev, self);
	if (result == ITER_DONE) {
		result = def;
		if (result) {
			Dee_Incref(result);
		} else {
			DeeError_Throw(&DeeError_StopIteration_instance);
		}
	}
	return result;
}


%[define(DEFINE_iterator_dummy =
#ifndef DEFINED_iterator_dummy
#define DEFINED_iterator_dummy
PRIVATE DeeObject iterator_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };
PRIVATE DeeObject *tpconst iterator_dummy_vec[1] = { &iterator_dummy };
#endif /* !DEFINED_iterator_dummy */
)]

/* @return: ITER_DONE: Iterator is exhausted */
[[wunused]] DREF DeeObject *
__iter_prev__.iter_prev([[nonnull]] DeeObject *self)
%{unsupported(err_iter_unsupportedf(self, "prev"))}
%{$none = return_none}
%{$empty = ITER_DONE}
%{$with__iter_revert__and__iter_peek = {
	size_t count = CALL_DEPENDENCY(iter_revert, self, 1);
	if unlikely(count == (size_t)-1)
		goto err;
	if (count == 0)
		return ITER_DONE;
	return CALL_DEPENDENCY(iter_peek, self);
err:
	return NULL;
}}
%{$with__iter_getindex__and_operator_next__and__iter_setindex = {
	DREF DeeObject *result;
	size_t new_index, old_index = CALL_DEPENDENCY(iter_getindex, self);
	if unlikely(old_index == (size_t)-1)
		goto err;
	if (old_index == 0)
		return ITER_DONE;
	new_index = old_index - 1;
	if (CALL_DEPENDENCY(iter_setindex, self, new_index))
		goto err;
	result = DeeObject_IterNext(self);
	if (ITER_ISOK(result)) {
		if (CALL_DEPENDENCY(iter_setindex, self, new_index))
			goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}} [[prefix(DEFINE_iterator_dummy)]] {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, iterator_dummy_vec);
	if (result == &iterator_dummy) {
		Dee_DecrefNokill(&iterator_dummy);
		result = ITER_DONE;
	}
	return result;
}


iter_prev = {
	DeeMH_iter_peek_t iter_peek = REQUIRE(iter_peek);
	if (iter_peek == &default__iter_peek__empty)
		return &$empty;
	if (iter_peek == &default__iter_peek__with__operator_next__and__iter_revert)
		return &$with__iter_revert__and__iter_peek;
	if (iter_peek == &default__iter_peek__with__iter_getindex__and_operator_next__and__iter_setindex)
		return &$with__iter_getindex__and_operator_next__and__iter_setindex;
	if (iter_peek) {
		DeeMH_iter_revert_t iter_revert = REQUIRE(iter_revert);
		if (iter_revert == &default__iter_revert__empty)
			return &$empty;
		if (iter_revert == &default__iter_revert__with__iter_getindex__and__iter_setindex)
			return &$with__iter_getindex__and_operator_next__and__iter_setindex;
		return &$with__iter_revert__and__iter_peek;
	}
};
