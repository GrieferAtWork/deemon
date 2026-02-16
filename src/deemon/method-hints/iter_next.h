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
/* deemon.Iterator.nextkey()                                            */
/* deemon.Iterator.nextvalue()                                          */
/************************************************************************/

[[alias(Iterator.nextkey)]]
__iter_nextkey__(def?)->?O {
	DREF DeeObject *result = CALL_DEPENDENCY(iter_nextkey, self);
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

[[alias(Iterator.nextvalue)]]
__iter_nextvalue__(def?)->?O {
	DREF DeeObject *result = CALL_DEPENDENCY(iter_nextvalue, self);
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


/* Return the iterator's next key. Same as:
 * >> local key, value = this.operator next()...;
 * >> return key; */
[[operator(*: tp_iterator->tp_nextkey)]] /* TODO: Remove this (only here because "tp_nextpair" still exists) */
[[wunused]] DREF DeeObject *
__iter_nextkey__.iter_nextkey([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias("default__iter_nextkey__with__operator_next")}
%{$empty = ITER_DONE}
%{$with__operator_next = {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = DeeObject_IterNext(self);
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
%{$with__iter_nextpair = {
	DREF DeeObject *key_and_value[2];
	int status = CALL_DEPENDENCY(iter_nextpair, self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[1]);
	return key_and_value[0];
err:
	return NULL;
}}
[[prefix(DEFINE_iterator_dummy)]] {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, iterator_dummy_vec);
	if (result == &iterator_dummy) {
		Dee_DecrefNokill(&iterator_dummy);
		result = ITER_DONE;
	}
	return result;
}

iter_nextkey = {
	if (REQUIRE_NODEFAULT(iter_nextpair))
		return &$with__iter_nextpair;
};


/* Return the iterator's next value. Same as:
 * >> local key, value = this.operator next()...;
 * >> return value; */
[[operator(*: tp_iterator->tp_nextvalue)]] /* TODO: Remove this (only here because "tp_nextpair" still exists) */
[[wunused]] DREF DeeObject *
__iter_nextvalue__.iter_nextvalue([[nonnull]] DeeObject *__restrict self)
%{unsupported_alias("default__iter_nextvalue__with__operator_next")}
%{$empty = ITER_DONE}
%{$with__operator_next = {
	int unpack_status;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *item = DeeObject_IterNext(self);
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
%{$with__iter_nextpair = {
	DREF DeeObject *key_and_value[2];
	int status = CALL_DEPENDENCY(iter_nextpair, self, key_and_value);
	if unlikely(status != 0) {
		if unlikely(status < 0)
			goto err;
		return ITER_DONE;
	}
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err:
	return NULL;
}}
[[prefix(DEFINE_iterator_dummy)]] {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, iterator_dummy_vec);
	if (result == &iterator_dummy) {
		Dee_DecrefNokill(&iterator_dummy);
		result = ITER_DONE;
	}
	return result;
}


iter_nextvalue = {
	if (REQUIRE_NODEFAULT(iter_nextpair))
		return &$with__iter_nextpair;
};



/* Fast-pass for `DeeSeq_Unpack(DeeObject_IterNext(self), 2)'
 * @return: 0 : Success
 * @return: 1 : Iterator has been exhausted
 * @return: -1: Error */
[[operator(*: tp_iterator->tp_nextpair)]] /* TODO: Remove this (only here because "tp_nextpair" still exists) */
[[wunused]] int
iter_nextpair([[nonnull]] DeeObject *__restrict self,
              [[nonnull]] DREF DeeObject *key_and_value[2])
%{unsupported_alias("default__iter_nextvalue__with__operator_next")}
%{$empty = 1}
%{$with__operator_next = {
	int result;
	DREF DeeObject *item = DeeObject_IterNext(self);
	if (item == ITER_DONE)
		return 1;
	if unlikely(item == NULL)
		goto err;
	result = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref_likely(item);
	return result;
err:
	return -1;
}};


