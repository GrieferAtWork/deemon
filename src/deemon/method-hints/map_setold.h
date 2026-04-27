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
/* deemon.Mapping.setold()                                              */
/************************************************************************/
[[alias(Mapping.setold)]]
__map_setold__(key,value)->?Dbool {
	int result = CALL_DEPENDENCY(map_setold, self, key, value);
	if (Dee_HAS_ISERR(result))
		goto err;
	return_bool(Dee_HAS_ISYES_NO_ERR(result));
err:
	return NULL;
}


/* Override the value of a pre-existing key
 * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
 * @return: Dee_HAS_YES: The value of `key' was set to `value'
 * @return: Dee_HAS_NO:  The given `key' doesn't exist (nothing was updated)
 * @return: Dee_HAS_ERR: Error */
[[wunused]] int
__map_setold__.map_setold([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key,
                          [[nonnull]] DeeObject *value)
%{unsupported(auto)}
%{$none = Dee_HAS_NO}
%{$empty = "default__map_setold__unsupported"}
%{$with__map_setold_ex = {
	DREF DeeObject *old_value;
	old_value = CALL_DEPENDENCY(map_setold_ex, self, key, value);
	if (!ITER_ISOK(old_value))
		return Dee_HAS_FROMITERNOK(old_value);
	Dee_Decref(old_value);
	return Dee_HAS_YES;
}}
%{$with__map_operator_trygetitem__and__map_operator_setitem = {
	DREF DeeObject *old_value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if (!ITER_ISOK(old_value))
		return Dee_HAS_FROMITERNOK(old_value); /* Error, or key doesn't exist */
	Dee_Decref(old_value);
	if unlikely(CALL_DEPENDENCY(map_operator_setitem, self, key, value))
		goto err;
	return Dee_HAS_YES;
err:
	return Dee_HAS_ERR;
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
	return Dee_HAS_ERR;
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
