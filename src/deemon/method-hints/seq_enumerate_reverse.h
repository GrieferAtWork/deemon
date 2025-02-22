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

/* Common utility functions... (WARNING: These are optional and don't have an "unsupported" fallback) */


[[wunused]] Dee_ssize_t
seq_foreach_reverse([[nonnull]] DeeObject *__restrict self,
                    [[nonnull]] Dee_foreach_t cb, void *arg)
%{$empty = 0}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	while (size) {
		DREF DeeObject *item;
		--size;
		item = (*tp_getitem_index_fast)(self, size);
		if likely(item) {
			temp = (*cb)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*cb)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *item;
		--size;
		item = CALL_DEPENDENCY(seq_operator_getitem_index, self, size);
		if likely(item) {
			temp = (*cb)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	for (;;) {
		DREF DeeObject *item;
		int size_is_nonzero = DeeObject_Bool(sizeob);
		if unlikely(size_is_nonzero < 0)
			goto err_sizeob;
		if (!size_is_nonzero)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob;
		item = CALL_DEPENDENCY(seq_operator_getitem, self, sizeob);
		if likely(item) {
			temp = (*cb)(arg, item);
			Dee_Decref(item);
			if unlikely(temp < 0)
				goto err_temp_sizeob;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob;
		}
		if (DeeThread_CheckInterrupt())
			goto err_sizeob;
	}
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob:
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}};

[[wunused]] Dee_ssize_t
seq_enumerate_index_reverse([[nonnull]] DeeObject *__restrict self,
                            [[nonnull]] Dee_seq_enumerate_index_t cb, void *arg,
                            size_t start, size_t end)
%{$empty = 0}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DeeObject *(DCALL *tp_getitem_index_fast)(DeeObject *self, size_t index);
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	tp_getitem_index_fast = Dee_TYPE(self)->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = (*tp_getitem_index_fast)(self, size);
		temp = (*cb)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, size);
		if unlikely(!item)
			goto err;
		if likely(item != ITER_DONE) {
			temp = (*cb)(arg, size, item);
			Dee_Decref(item);
		} else {
			temp = (*cb)(arg, size, NULL);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *item;
		--size;
		item = CALL_DEPENDENCY(seq_operator_getitem_index, self, size);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*cb)(arg, size, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	if (end != (size_t)-1) {
		int error;
		DREF DeeObject *wanted_end;
		wanted_end = DeeInt_NewSize(end);
		if unlikely(!wanted_end)
			goto err_sizeob;
		/* if (sizeob > wanted_end)
		 *     sizeob = wanted_end; */
		error = DeeObject_CmpGrAsBool(sizeob, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
		} else {
			Dee_Decref(sizeob);
			sizeob = wanted_end;
			if unlikely(error < 0)
				goto err_sizeob;
		}
	}
	if (start != 0) {
		startob = DeeInt_NewSize(start);
		if unlikely(!startob)
			goto err_sizeob;
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *item;
		int size_is_greater_start;
		if (startob) {
			size_is_greater_start = DeeObject_CmpGrAsBool(sizeob, startob);
		} else {
			size_is_greater_start = DeeObject_Bool(sizeob);
		}
		if unlikely(size_is_greater_start < 0)
			goto err_sizeob_startob;
		if (!size_is_greater_start)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob_startob;
		item = CALL_DEPENDENCY(seq_operator_getitem, self, sizeob);
		if unlikely(!item) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_startob;
		}
		if unlikely(DeeObject_AsSize(sizeob, &index_value))
			goto err_sizeob_startob;
		temp = 0;
		if likely(index_value >= start && index_value < end)
			temp = (*cb)(arg, index_value, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp_sizeob_startob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_startob;
	}
	Dee_XDecref(startob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_startob:
	Dee_XDecref(startob);
/*err_temp_sizeob:*/
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob_startob:
	Dee_XDecref(startob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}};


seq_foreach_reverse = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index == NULL)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
			ASSERT(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto nope;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
				return &$with__seq_operator_sizeob__and__seq_operator_getitem;
			if (seq_operator_getitem_index == default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
nope:;
};

seq_enumerate_index_reverse = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &$empty;
		if (seq_operator_trygetitem_index == NULL)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
			ASSERT(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &$empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto nope;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
				return &$with__seq_operator_sizeob__and__seq_operator_getitem;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
nope:;
};
