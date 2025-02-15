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
/* deemon.Mapping.remove()                                              */
/************************************************************************/
[[alias(Mapping.remove -> "map_remove"), declNameAlias("explicit_map_remove")]]
__map_remove__(key)->?Dbool {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_remove__", &key))
		goto err;
	result = CALL_DEPENDENCY(map_remove, self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}


/* Remove a single key, returning true/false indicative of that key having been removed.
 * @return: 1 : Key was removed
 * @return: 0 : Key didn't exist (nothing was removed)
 * @return: -1: Error */
[[wunused]] int
__map_remove__.map_remove([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = "default__map_remove__unsupported"}
%{$with__map_operator_bounditem__and__map_operator_delitem = {
	int bound = CALL_DEPENDENCY(map_operator_bounditem, self, key);
	if unlikely(Dee_BOUND_ISERR(bound))
		goto err;
	if (!Dee_BOUND_ISBOUND(bound))
		return 0;
	if unlikely(CALL_DEPENDENCY(map_operator_delitem, self, key))
		goto err;
	return 1;
err:
	return -1;
}}
%{$with__seq_operator_size__and__map_operator_delitem = {
	size_t new_size;
	size_t old_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	if unlikely(old_size == 0)
		return 0;
	if unlikely(CALL_DEPENDENCY(map_operator_delitem, self, key))
		goto err;
	new_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

map_remove = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem) {
		if (REQUIRE_ANY(map_operator_bounditem) != &default__map_operator_bounditem__unsupported)
			return &$with__map_operator_bounditem__and__map_operator_delitem;
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__map_operator_delitem;
	}
};
