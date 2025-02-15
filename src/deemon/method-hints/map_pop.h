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
/* deemon.Mapping.pop()                                                 */
/************************************************************************/
[[alias(Mapping.pop -> "map_pop")]]
__map_pop__(key,def?)->?O {
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:__map_pop__", &key, &def))
		goto err;
	return def ? CALL_DEPENDENCY(map_pop_with_default, self, key, def)
	           : CALL_DEPENDENCY(map_pop, self, key);
err:
	return NULL;
}



/* Remove/unbind `key' and return whatever used to be assigned to it.
 * When the key was already absent/unbound, return `default_' or throw a `KeyError'
 * @return: * :   The old value of `key'
 * @return: NULL: Error */
[[wunused]] DREF DeeObject *
__map_pop__.map_pop([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = {
	err_unknown_key(self, key);
	return NULL;
}}
%{$with__map_operator_getitem__and__map_operator_delitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_getitem, self, key);
	if unlikely(!result)
		goto err;
	if unlikely(CALL_DEPENDENCY(map_operator_delitem, self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &key);
}




[[wunused]] DREF DeeObject *
__map_pop__.map_pop_with_default([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *key,
                                 [[nonnull]] DeeObject *default_)
%{unsupported(auto)}
%{$empty = return_reference_(default_)}
%{$with__map_operator_trygetitem__and__map_operator_delitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
	if unlikely(!result)
		goto err;
	if (result == ITER_DONE)
		return_reference_(default_);
	if unlikely(CALL_DEPENDENCY(map_operator_delitem, self, key))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}} {
	DeeObject *args[2];
	args[0] = key;
	args[1] = default_;
	return LOCAL_CALLATTR(self, 2, args);
}







map_pop = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem &&
	    REQUIRE_ANY(map_operator_getitem) != &default__map_operator_getitem__unsupported)
		return &$with__map_operator_getitem__and__map_operator_delitem;;
};

map_pop_with_default = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem &&
	    REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &$with__map_operator_trygetitem__and__map_operator_delitem;;
};
