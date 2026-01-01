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
/* deemon.Mapping.setnew()                                              */
/************************************************************************/
[[alias(Mapping.setnew)]]
__map_setnew__(key,value)->?Dbool {
	int result = CALL_DEPENDENCY(map_setnew, self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}


/* Insert a new key whilst making sure that the key doesn't already exist
 * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
 * @return: 1 :   The value of `key' was set to `value' (the key didn't exist or used to be unbound)
 * @return: 0 :   The given `key' already exists (nothing was inserted)
 * @return: -1:   Error */
[[wunused]] int
__map_setnew__.map_setnew([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key,
                          [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__map_setnew__unsupported"}
%{$with__map_setnew_ex = {
	DREF DeeObject *old_value;
	old_value = CALL_DEPENDENCY(map_setnew_ex, self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return 1;
	Dee_Decref(old_value);
	return 0;
err:
	return -1;
}}
%{$with__map_operator_trygetitem__and__map_setdefault = {
	DREF DeeObject *temp;
	DREF DeeObject *bound = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	temp = CALL_DEPENDENCY(map_setdefault, self, key, value);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 1;
err:
	return -1;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *bound = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (bound != ITER_DONE) {
		if unlikely(!bound)
			goto err;
		Dee_Decref(bound);
		return 0; /* Key already exists */
	}
	if unlikely(CALL_DEPENDENCY(map_operator_setitem, self, key, value))
		goto err;
	return 1;
err:
	return -1;
}} {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	result = LOCAL_CALLATTR(self, 2, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

map_setnew = {
	DeeMH_map_setnew_ex_t map_setnew_ex = REQUIRE(map_setnew_ex);
	if (map_setnew_ex == &default__map_setnew_ex__empty)
		return &$empty;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault)
		return &$with__map_operator_trygetitem__and__map_setdefault;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &$with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setnew_ex)
		return &$with__map_setnew_ex;
};
