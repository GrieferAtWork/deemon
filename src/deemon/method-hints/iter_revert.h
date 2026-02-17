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
/* deemon.Iterator.revert()                                            */
/************************************************************************/

[[alias(Iterator.revert)]]
__iter_revert__(size_t step)->?Dint {
	size_t result = CALL_DEPENDENCY(iter_revert, self, step);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* Revert an iterator by at most "step" items.
 * @return: step:       Success.
 * @return: < step:     Success, but reverting was stopped prematurely because ITER_DONE
 *                      was encountered. Return value is the # of successfully reverted
 *                      entries before "ITER_DONE" was encountered.
 * @return: (size_t)-1: Error. */
[[wunused]] size_t
__iter_revert__.iter_revert([[nonnull]] DeeObject *__restrict self, size_t step)
%{unsupported(err_iter_unsupportedf(self, "revert"))}
%{$empty = 0}
%{$with__iter_prev = {
	size_t result;
	PRELOAD_DEPENDENCY(iter_prev);
	for (result = 0; result < step; ++result) {
		DREF DeeObject *elem = CALL_DEPENDENCY(iter_prev, self); /* *--self */
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}}
/*%{$with__iter_getindex__and__iter_setindex = {
	size_t new_index, result;
	size_t old_index = CALL_DEPENDENCY(iter_getindex, self);
	if unlikely(old_index == (size_t)-1)
		goto err;
	if (OVERFLOW_USUB(old_index, step, &new_index))
		new_index = 0;
	result = (size_t)(old_index - new_index);
	if unlikely(result == (size_t)-1) {
		DeeRT_ErrIntegerOverflowU(result, (size_t)-1);
		goto err;
	}
	if (CALL_DEPENDENCY(iter_setindex, self, new_index))
		goto err;
	return result;
err:
	return (size_t)-1;
}}*/ {
	size_t result_value;
	DREF DeeObject *result = LOCAL_CALLATTRF(self, PCKuSIZ, step);
	if unlikely(!result)
		goto err;
	if (DeeObject_AsSize(result, &result_value))
		goto err_r;
	Dee_Decref(result);
	if unlikely(result_value == (size_t)-1)
		DeeRT_ErrIntegerOverflowU(result_value, (size_t)-2);
	return result_value;
err_r:
	Dee_Decref(result);
err:
	return (size_t)-1;
}


iter_revert = {
	if (REQUIRE_NODEFAULT(iter_prev))
		return &$with__iter_prev;
#if 0
	if (REQUIRE_ANY(iter_getindex) != &default__iter_getindex__unsupported) {
		/* TODO: Only if "iter_seq" has "__seq_getitem_always_bound__",
		 *       can use "$with__iter_getindex__and__iter_setindex" */
		DeeMH_iter_setindex_t iter_setindex = REQUIRE(iter_setindex);
		if (iter_setindex == &default__iter_setindex__empty)
			return &$empty;
		if (iter_setindex)
			return &$with__iter_getindex__and__iter_setindex;
	}
#endif
};
