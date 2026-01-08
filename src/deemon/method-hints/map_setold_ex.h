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
/* deemon.Mapping.setold_ex()                                           */
/************************************************************************/
[[alias(Mapping.setold_ex)]]
__map_setold_ex__(key,value)->?T2?Dbool?X2?O?N {
	PRIVATE DEFINE_TUPLE(setold_failed_result, 2, { Dee_False, Dee_None });
	DREF DeeTupleObject *result;
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_setold_ex, self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE) {
		Dee_Incref(&setold_failed_result);
		return Dee_AsObject(&setold_failed_result);
	}
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	result->t_elem[0] = DeeBool_NewTrue();
	result->t_elem[1] = old_value; /* Inherit reference */
	return Dee_AsObject(result);
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}



%[define(DEFINE_map_setold_ex_with_seq_enumerate_cb =
#ifndef DEFINED_map_setold_ex_with_seq_enumerate_cb
#define DEFINED_map_setold_ex_with_seq_enumerate_cb
#define MAP_SETOLD_EX_WITH_SEQ_ENUMERATE__SUCCESS (-2)
struct map_setold_ex_with_seq_enumerate_data {
	DeeObject *msoxwse_seq;              /* [1..1] Sequence to override "msoxwse_key_and_value[0]" in */
	DeeObject *msoxwse_key_and_value[2]; /* [1..1] Key to override in "<msoxwse_seq> as Mapping" */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_setold_ex_with_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	int status;
	DREF DeeObject *this_key_and_value[2];
	struct map_setold_ex_with_seq_enumerate_data *data;
	data = (struct map_setold_ex_with_seq_enumerate_data *)arg;
	if (value) {
		if (DeeSeq_Unpack(value, 2, this_key_and_value))
			goto err;
		status = DeeObject_TryCompareEq(data->msoxwse_key_and_value[0], this_key_and_value[0]);
		Dee_Decref(this_key_and_value[0]);
		if unlikely(status == Dee_COMPARE_ERR)
			goto err_this_value;
		if (status == 0) {
			/* Found it! (now to override it) */
			DREF DeeObject *new_pair;
			new_pair = DeeTuple_NewVector(2, data->msoxwse_key_and_value);
			if unlikely(!new_pair)
				goto err_this_value;
			status = DeeObject_InvokeMethodHint(seq_operator_setitem, data->msoxwse_seq, index, new_pair);
			Dee_Decref_unlikely(new_pair);
			if unlikely(status < 0)
				goto err_this_value;
			data->msoxwse_key_and_value[1] = this_key_and_value[1]; /* Inherit reference */
			return MAP_SETOLD_EX_WITH_SEQ_ENUMERATE__SUCCESS;
		}
		Dee_Decref(this_key_and_value[1]);
	}
	return 0;
err_this_value:
	Dee_Decref(this_key_and_value[1]);
err:
	return -1;
}
#endif /* !DEFINED_map_setold_ex_with_seq_enumerate_cb */
)]


%[define(DEFINE_map_setold_ex_with_seq_enumerate_index_cb =
#ifndef DEFINED_map_setold_ex_with_seq_enumerate_index_cb
#define DEFINED_map_setold_ex_with_seq_enumerate_index_cb
#define MAP_SETOLD_EX_WITH_SEQ_ENUMERATE_INDEX__SUCCESS (-2)
struct map_setold_ex_with_seq_enumerate_index_data {
	DeeObject *msoxwsei_seq;              /* [1..1] Sequence to override "msoxwsei_key_and_value[0]" in */
	DeeObject *msoxwsei_key_and_value[2]; /* [1..1] Key to override in "<msoxwsei_seq> as Mapping" */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
map_setold_ex_with_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	int status;
	DREF DeeObject *this_key_and_value[2];
	struct map_setold_ex_with_seq_enumerate_index_data *data;
	data = (struct map_setold_ex_with_seq_enumerate_index_data *)arg;
	if (value) {
		if (DeeSeq_Unpack(value, 2, this_key_and_value))
			goto err;
		status = DeeObject_TryCompareEq(data->msoxwsei_key_and_value[0], this_key_and_value[0]);
		Dee_Decref(this_key_and_value[0]);
		if unlikely(status == Dee_COMPARE_ERR)
			goto err_this_value;
		if (status == 0) {
			/* Found it! (now to override it) */
			DREF DeeObject *new_pair;
			new_pair = DeeTuple_NewVector(2, data->msoxwsei_key_and_value);
			if unlikely(!new_pair)
				goto err_this_value;
			status = DeeObject_InvokeMethodHint(seq_operator_setitem_index, data->msoxwsei_seq, index, new_pair);
			Dee_Decref_unlikely(new_pair);
			if unlikely(status < 0)
				goto err_this_value;
			data->msoxwsei_key_and_value[1] = this_key_and_value[1]; /* Inherit reference */
			return MAP_SETOLD_EX_WITH_SEQ_ENUMERATE_INDEX__SUCCESS;
		}
		Dee_Decref(this_key_and_value[1]);
	}
	return 0;
err_this_value:
	Dee_Decref(this_key_and_value[1]);
err:
	return -1;
}
#endif /* !DEFINED_map_setold_ex_with_seq_enumerate_index_cb */
)]





/* @return: * :        The value of `key' was set to `value' (returned object is the old value)
 * @return: ITER_DONE: The given `key' doesn't exist (nothing was updated)
 * @return: NULL:      Error */
[[wunused]] DREF DeeObject *
__map_setold_ex__.map_setold_ex([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *key,
                                [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = "default__map_setold_ex__unsupported"}
%{$with__map_operator_trygetitem__and__map_setold = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (ITER_ISOK(old_value)) {
		int status = CALL_DEPENDENCY(map_setold, self, key, value);
		if unlikely(status <= 0) {
			if unlikely(status < 0)
				goto err_old_value;
			Dee_Decref(old_value);
			return ITER_DONE;
		}
	}
	return old_value;
err_old_value:
	Dee_Decref(old_value);
/*err:*/
	return NULL;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (ITER_ISOK(old_value)) {
		if unlikely((*Dee_TYPE(self)->tp_seq->tp_setitem)(self, key, value))
			goto err_old_value;
	}
	return old_value;
err_old_value:
	Dee_Decref(old_value);
/*err:*/
	return NULL;
}}
%{$with__seq_enumerate__and__seq_operator_setitem = [[prefix(DEFINE_map_setold_ex_with_seq_enumerate_cb)]] {
	Dee_ssize_t status;
	struct map_setold_ex_with_seq_enumerate_data data;
	data.msoxwse_seq = self;
	data.msoxwse_key_and_value[0] = key;
	data.msoxwse_key_and_value[1] = value;
	status = CALL_DEPENDENCY(seq_enumerate, self, &map_setold_ex_with_seq_enumerate_cb, &data);
	if (status == MAP_SETOLD_EX_WITH_SEQ_ENUMERATE__SUCCESS)
		return data.msoxwse_key_and_value[1];
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__seq_enumerate_index__and__seq_operator_setitem_index = [[prefix(DEFINE_map_setold_ex_with_seq_enumerate_index_cb)]] {
	Dee_ssize_t status;
	struct map_setold_ex_with_seq_enumerate_index_data data;
	data.msoxwsei_seq = self;
	data.msoxwsei_key_and_value[0] = key;
	data.msoxwsei_key_and_value[1] = value;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &map_setold_ex_with_seq_enumerate_index_cb, &data, 0, (size_t)-1);
	if (status == MAP_SETOLD_EX_WITH_SEQ_ENUMERATE_INDEX__SUCCESS)
		return data.msoxwsei_key_and_value[1];
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}} {
	int temp;
	DeeObject *args[2];
	DREF DeeObject *result, *status[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_CALLATTR(self, 2, args);
	if unlikely(!result)
		goto err;
	temp = DeeSeq_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_probably_none(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

map_setold_ex = {
	DeeMH_map_operator_setitem_t map_operator_setitem;
	DeeMH_seq_operator_setitem_t seq_operator_setitem;
	if (REQUIRE_NODEFAULT(map_setold) &&
	    REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &$with__map_operator_trygetitem__and__map_setold;
	map_operator_setitem = REQUIRE(map_operator_setitem);
	if (map_operator_setitem) {
		if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append)
			return &$with__seq_enumerate_index__and__seq_operator_setitem_index;
		if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append)
			return &$with__seq_enumerate__and__seq_operator_setitem;
		if (REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
			return &$with__map_operator_trygetitem__and__map_operator_setitem;
	}
	seq_operator_setitem = REQUIRE(seq_operator_setitem);
	if (seq_operator_setitem) {
		if (seq_operator_setitem == &default__seq_operator_setitem__empty)
			return &$empty;
		if (seq_operator_setitem == &default__seq_operator_setitem__with__seq_operator_setitem_index &&
		    REQUIRE_ANY(seq_enumerate_index) != &default__seq_enumerate_index__unsupported)
			return &$with__seq_enumerate_index__and__seq_operator_setitem_index;
		if (REQUIRE_ANY(seq_enumerate) != &default__seq_enumerate__unsupported)
			return &$with__seq_enumerate__and__seq_operator_setitem;
	}
};
