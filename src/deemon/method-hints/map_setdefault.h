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
/* deemon.Mapping.setdefault()                                          */
/************************************************************************/
[[alias(Mapping.setdefault -> "map_setdefault"), declNameAlias("explicit_map_setdefault")]]
__map_setdefault__(key,value)->?O {
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setdefault__", &key, &value))
		goto err;
	return CALL_DEPENDENCY(map_setdefault, self, key, value);
err:
	return NULL;
}


/* Same semantic functionality as `map_setnew_ex': insert if not already present
 * @return: * : The value associated with key after the call:
 *              - if already present and nothing was inserted, its old value
 *              - if used-to-be absent/unbound and was assigned/inserted, `value'
 * @return: NULL: Error */
[[wunused]] DREF DeeObject *
__map_setdefault__.map_setdefault([[nonnull]] DeeObject *self,
                                  [[nonnull]] DeeObject *key,
                                  [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$empty = "default__map_setdefault__unsupported"}
%{$with__map_setnew_ex = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_setnew_ex, self, key, value);
	if (old_value == ITER_DONE) {
		/* Value was just inserted */
		old_value = value;
		Dee_Incref(value);
	}
	return old_value;
}}
%{$with__map_setnew__and__map_operator_getitem = {
	int temp = CALL_DEPENDENCY(map_setnew, self, key, value);
	if unlikely(temp < 0)
		goto err;
	if (temp > 0)
		return_reference_(value);
	return CALL_DEPENDENCY(map_operator_getitem, self, key);
err:
	return NULL;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (result == ITER_DONE) {
		if unlikely(CALL_DEPENDENCY(map_operator_setitem, self, key, value))
			goto err;
		result = value;
		Dee_Incref(value);
	}
	return result;
err:
	return NULL;
}} {
	DeeObject *args[2];
	args[0] = key;
	args[1] = value;
	return LOCAL_CALLATTR(self, 2, args);
}

map_setdefault = {
	DeeMH_map_setnew_ex_t map_setnew_ex = REQUIRE(map_setnew_ex);
	if (map_setnew_ex == &default__map_setnew_ex__empty)
		return &$empty;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew)
		return &$with__map_setnew__and__map_operator_getitem;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &$with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &$with__map_setnew_ex;
};
