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
/* deemon.Object.operator next()                                        */
/************************************************************************/

operator {

[[export("DeeObject_{|T}IterNext")]]
[[wunused]] DREF DeeObject *
tp_iter_next([[nonnull]] DeeObject *__restrict self)
%{class using OPERATOR_ITERNEXT: {
	DREF DeeObject *result;
#ifdef __OPTIMIZE_SIZE__
	result = DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_ITERNEXT, 0, NULL);
#else /* __OPTIMIZE_SIZE__ */
	DREF DeeObject *func;
	func = DeeClass_GetOperator(THIS_TYPE, OPERATOR_ITERNEXT);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCallInherited(func, self, 0, NULL);
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_StopIteration))
			goto err;
		result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}}
%{using tp_iterator->tp_nextpair: {
	int error;
	DREF DeeTupleObject *result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	error = CALL_DEPENDENCY(tp_iterator->tp_nextpair, self, DeeTuple_ELEM(result));
	if (error == 0)
		return Dee_AsObject(result);
	DeeTuple_FreeUninitializedPair(result);
	if likely(error > 0)
		return ITER_DONE;
err:
	return NULL;
}} = OPERATOR_ITERNEXT;


/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2)'
 * @return: 0 : Success
 * @return: 1 : Iterator has been exhausted
 * @return: -1: Error */
[[export("DeeObject_{|T}IterNextPair")]]
[[wunused]] int
tp_iterator->tp_nextpair([[nonnull]] DeeObject *__restrict self,
                         [[nonnull]] DREF DeeObject *key_and_value[2])
%{using tp_iter_next: {
	int result;
	DREF DeeObject *item = CALL_DEPENDENCY(tp_iter_next, self);
	if (item == ITER_DONE)
		return 1;
	if unlikely(item == NULL)
		goto err;
	result = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	return result;
err:
	return -1;
}} = OPERATOR_ITERNEXT;


/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2).first'
 * In the case of mapping iterators, these can be used to iterate only the
 * key/value part of the map, without needing to construct a temporary tuple
 * holding both values (as needs to be done by `tp_iter_next'). */
[[export("DeeObject_{|T}IterNextKey")]]
[[wunused]] DREF DeeObject *
tp_iterator->tp_nextkey([[nonnull]] DeeObject *__restrict self)
%{using tp_iter_next: {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = CALL_DEPENDENCY(tp_iter_next, self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}}
%{using tp_iterator->tp_nextpair: {
	DREF DeeObject *key_and_value[2];
	int status = CALL_DEPENDENCY(tp_iterator->tp_nextpair, self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}} = OPERATOR_ITERNEXT;


/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2).last'
 * In the case of mapping iterators, these can be used to iterate only the
 * key/value part of the map, without needing to construct a temporary tuple
 * holding both values (as needs to be done by `tp_iter_next'). */
[[export("DeeObject_{|T}IterNextValue")]]
[[wunused]] DREF DeeObject *
tp_iterator->tp_nextvalue([[nonnull]] DeeObject *__restrict self)
%{using tp_iter_next: {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = CALL_DEPENDENCY(tp_iter_next, self);
	if unlikely(!ITER_ISOK(item))
		return item;
	unpack_status = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	if unlikely(unpack_status)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}}
%{using tp_iterator->tp_nextpair: {
	DREF DeeObject *key_and_value[2];
	int status = CALL_DEPENDENCY(tp_iterator->tp_nextpair, self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}} = OPERATOR_ITERNEXT;


/* Advance an iterator by "step" items.
 * @return: step:       Success.
 * @return: < step:     Success, but advancing was stopped prematurely because ITER_DONE
 *                      was encountered. Return value is the # of successfully skipped
 *                      entries before "ITER_DONE" was encountered.
 * @return: (size_t)-1: Error. */
[[export("DeeObject_{|T}IterAdvance")]]
[[wunused]] size_t
tp_iterator->tp_advance([[nonnull]] DeeObject *__restrict self, size_t step)
%{using tp_iterator->tp_nextkey: {
	size_t result = 0;
	PRELOAD_DEPENDENCY(tp_iterator->tp_nextkey)
	while (result < step) {
		DREF DeeObject *elem = CALL_DEPENDENCY(tp_iterator->tp_nextkey, self);
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
%{using tp_iterator->tp_nextvalue: {
	size_t result = 0;
	PRELOAD_DEPENDENCY(tp_iterator->tp_nextvalue)
	while (result < step) {
		DREF DeeObject *elem = CALL_DEPENDENCY(tp_iterator->tp_nextvalue, self);
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
%{using tp_iterator->tp_nextpair: {
	size_t result = 0;
	PRELOAD_DEPENDENCY(tp_iterator->tp_nextpair)
	while (result < step) {
		DREF DeeObject *key_and_value[2];
		int error = CALL_DEPENDENCY(tp_iterator->tp_nextpair, self, key_and_value);
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
%{using tp_iter_next: {
	size_t result = 0;
	PRELOAD_DEPENDENCY(tp_iter_next)
	while (result < step) {
		DREF DeeObject *elem = CALL_DEPENDENCY(tp_iter_next, self);
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
}} = OPERATOR_ITERNEXT;


} /* operator */
