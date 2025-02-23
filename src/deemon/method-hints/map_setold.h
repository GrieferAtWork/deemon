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
/* deemon.Mapping.setold()                                              */
/************************************************************************/
[[alias(Mapping.setold -> "map_setold")]]
__map_setold__(key,value)->?Dbool {
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__map_setold__", &key, &value))
		goto err;
	result = CALL_DEPENDENCY(map_setold, self, key, value);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}


/* Override the value of a pre-existing key
 * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
 * @return: 1 :   The value of `key' was set to `value'
 * @return: 0 :   The given `key' doesn't exist (nothing was updated)
 * @return: -1:   Error */
[[wunused]] int
__map_setold__.map_setold([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key,
                          [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$empty = "default__map_setold__unsupported"}
%{$with__map_setold_ex = {
	DREF DeeObject *old_value;
	old_value = CALL_DEPENDENCY(map_setold_ex, self, key, value);
	if unlikely(!old_value)
		goto err;
	if (old_value == ITER_DONE)
		return 0;
	Dee_Decref(old_value);
	return 1;
err:
	return -1;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (!ITER_ISOK(old_value)) {
		if unlikely(!old_value)
			goto err;
		return 0; /* Key doesn't exist */
	}
	Dee_Decref(old_value);
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

map_setold = {
	DeeMH_map_setold_ex_t map_setold_ex = REQUIRE(map_setold_ex);
	if (map_setold_ex == &default__map_setold_ex__empty)
		return &$empty;
	if (map_setold_ex == &default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &$with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setold_ex)
		return &$with__map_setold_ex;
};
