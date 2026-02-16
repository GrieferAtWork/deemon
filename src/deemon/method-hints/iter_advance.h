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
/* deemon.Iterator.advance()                                            */
/************************************************************************/

[[alias(Iterator.advance)]]
__iter_advance__(size_t step)->?Dint {
	size_t result = CALL_DEPENDENCY(iter_advance, self, step);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* Advance an iterator by "step" items.
 * @return: step:       Success.
 * @return: < step:     Success, but advancing was stopped prematurely because ITER_DONE
 *                      was encountered. Return value is the # of successfully skipped
 *                      entries before "ITER_DONE" was encountered.
 * @return: (size_t)-1: Error. */
[[operator(*: tp_iterator->tp_advance)]] /* TODO: Remove this (only here because "tp_nextpair" still exists) */
[[wunused]] size_t
__iter_advance__.iter_advance([[nonnull]] DeeObject *__restrict self, size_t step)
%{unsupported_alias("default__iter_advance__with__operator_next")}
%{$empty = 0}
%{$with__iter_nextkey = {
	size_t result = 0;
	PRELOAD_DEPENDENCY(iter_nextkey);
	while (result < step) {
		DREF DeeObject *elem = CALL_DEPENDENCY(iter_nextkey, self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}}
%{$with__iter_nextvalue = {
	size_t result = 0;
	PRELOAD_DEPENDENCY(iter_nextvalue);
	while (result < step) {
		DREF DeeObject *elem = CALL_DEPENDENCY(iter_nextvalue, self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}}
%{$with__iter_nextpair = {
	size_t result = 0;
	PRELOAD_DEPENDENCY(iter_nextpair)
	while (result < step) {
		DREF DeeObject *key_and_value[2];
		int error = CALL_DEPENDENCY(iter_nextpair, self, key_and_value);
		if unlikely(error != 0) {
			if unlikely(error < 0)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}}
%{$with__operator_next = {
	size_t result = 0;
	DeeNO_iter_next_t iter_next = DeeType_RequireNativeOperator(Dee_TYPE(self), iter_next);
	while (result < step) {
		DREF DeeObject *elem = (*iter_next)(self);
		if unlikely(!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			break; /* Premature stop. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
		++result;
	}
	return result;
err:
	return (size_t)-1;
}} {
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


iter_advance = {
	if (REQUIRE_NODEFAULT(iter_nextkey))
		return &$with__iter_nextkey;
	if (REQUIRE_NODEFAULT(iter_nextvalue))
		return &$with__iter_nextvalue;
	if (REQUIRE_NODEFAULT(iter_nextpair))
		return &$with__iter_nextpair;
};
