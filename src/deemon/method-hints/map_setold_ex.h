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
/* deemon.Mapping.setold_ex()                                           */
/************************************************************************/
[[alias(Mapping.setold_ex -> "map_setold_ex"), declNameAlias("explicit_map_setold_ex")]]
__map_setold_ex__(key,value)->?T2?Dbool?X2?O?N {
	PRIVATE DEFINE_TUPLE(setold_failed_result, 2, { Dee_False, Dee_None });
	DeeObject *key, *value;
	DREF DeeObject *old_value;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setold_ex__", &key, &value))
		goto err;
	old_value = CALL_DEPENDENCY(map_setold_ex, self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return_reference_((DeeObject *)&setold_failed_result);
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_old_value;
	Dee_Incref(Dee_True);
	DeeTuple_SET(result, 0, Dee_True);
	DeeTuple_SET(result, 1, old_value); /* Inherit reference */
	return (DREF DeeObject *)result;
err_old_value:
	Dee_Decref(old_value);
err:
	return NULL;
}


/* @return: * :        The value of `key' was set to `value' (returned object is the old value)
 * @return: ITER_DONE: The given `key' doesn't exist (nothing was updated)
 * @return: NULL:      Error */
[[wunused]] DREF DeeObject *
__map_setold_ex__.map_setold_ex([[nonnull]] DeeObject *self,
                                [[nonnull]] DeeObject *key,
                                [[nonnull]] DeeObject *value)
%{unsupported(auto)}
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

map_setold_ex = {
	if (REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported) {
		if (REQUIRE_NODEFAULT(map_setold))
			return &$with__map_operator_trygetitem__and__map_setold;
		if (REQUIRE(map_operator_setitem))
			return &$with__map_operator_trygetitem__and__map_operator_setitem;
	}
};
