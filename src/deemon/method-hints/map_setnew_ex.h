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
/* deemon.Mapping.setnew_ex()                                           */
/************************************************************************/
[[alias(Mapping.setnew_ex -> "map_setnew_ex"), declNameAlias("explicit_map_setnew_ex")]]
__map_setnew_ex__(key,value)->?T2?Dbool?X2?O?N {
	PRIVATE DEFINE_TUPLE(setnew_success_result, 2, { Dee_True, Dee_None });
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setnew_ex__", &key, &value))
		goto err;
	old_value = CALL_DEPENDENCY(map_setnew_ex, self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setnew_success_result);
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_False);
	DeeTuple_SET(result, 0, Dee_False);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}


/* @return: ITER_DONE: The value of `key' was set to `value' (the key didn't exist or used to be unbound)
 * @return: * :        The given `key' already exists (nothing was inserted; returned object is the already-present value)
 * @return: -1:        Error */
[[wunused]] DREF DeeObject *
__map_setnew_ex__.map_setnew_ex([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *key,
                                [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$empty = "default__map_setnew_ex__unsupported"}
%{$with__map_operator_trygetitem__and__map_setnew = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	if unlikely(CALL_DEPENDENCY(map_setnew, self, key, value) < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_operator_trygetitem__and__map_setdefault = {
	DREF DeeObject *temp;
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	temp = CALL_DEPENDENCY(map_setdefault, self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	if unlikely(CALL_DEPENDENCY(map_operator_setitem, self, key, value))
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_operator_trygetitem__and__seq_append = {
	int append_status;
	DREF DeeObject *new_pair;
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if unlikely(!old_value)
		goto err;
	if (old_value != ITER_DONE)
		return old_value;
	new_pair = DeeTuple_PackPair(key, value);
	if unlikely(!new_pair)
		goto err;
	append_status = CALL_DEPENDENCY(seq_append, self, new_pair);
	Dee_Decref_unlikely(new_pair);
	if unlikely(append_status)
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
	temp = DeeObject_Unpack(result, 2, status);
	Dee_Decref(result);
	if unlikely(temp)
		goto err;
	temp = DeeObject_BoolInherited(status[0]);
	if unlikely(temp < 0)
		goto err_status1;
	if (temp) {
		Dee_Decref_unlikely(status[1]); /* Should always be `Dee_None' */
		return ITER_DONE;
	}
	return status[1];
err_status1:
	Dee_Decref(status[1]);
err:
	return NULL;
}

map_setnew_ex = {
	if (REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported) {
		DeeMH_map_operator_setitem_t map_operator_setitem;
		if (REQUIRE_NODEFAULT(map_setnew))
			return &$with__map_operator_trygetitem__and__map_setnew;
		if (REQUIRE_NODEFAULT(map_setdefault))
			return &$with__map_operator_trygetitem__and__map_setdefault;
		map_operator_setitem = REQUIRE(map_operator_setitem);
		if (map_operator_setitem) {
			if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append ||
			    map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append)
				return &$with__map_operator_trygetitem__and__seq_append;
			return &$with__map_operator_trygetitem__and__map_operator_setitem;
		}
		if (REQUIRE(seq_append))
			return &$with__map_operator_trygetitem__and__seq_append;
	}
};
