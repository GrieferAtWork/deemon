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
/* deemon.Set.insert()                                                  */
/************************************************************************/
[[alias(Set.insert)]]
__set_insert__(key)->?Dbool {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_insert__", &key))
		goto err;
	result = CALL_DEPENDENCY(set_insert, self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

/* Insert a key into a set
 * @return: 1 : Given `key' was inserted and wasn't already present
 * @return: 0 : Given `key' was already present
 * @return: -1: Error */
[[wunused]] int
__set_insert__.set_insert([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__set_insert__unsupported"}
%{$with__map_setnew = {
	int result;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeSeq_Unpack(key, 2, map_key_and_value))
		goto err;
	result = CALL_DEPENDENCY(map_setnew, self, map_key_and_value[0], map_key_and_value[1]);
	Dee_Decref(map_key_and_value[1]);
	Dee_Decref(map_key_and_value[0]);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_size__and__set_insertall = {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *items;
	old_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	items = DeeTuple_NewVectorSymbolic(1, &key);
	if unlikely(!items)
		goto err;
	temp = CALL_DEPENDENCY(set_insertall, self, items);
	DeeTuple_DecrefSymbolic(items);
	if unlikely(temp)
		goto err;
	new_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}}
%{$with__seq_contains__and__seq_append = {
	int contains = CALL_DEPENDENCY(seq_contains, self, key);
	if unlikely(contains < 0)
		goto err;
	if (contains)
		return 0;
	if unlikely(CALL_DEPENDENCY(seq_append, self, key))
		goto err;
	return 1;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

set_insert = {
	if (REQUIRE_NODEFAULT(set_insertall)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__set_insertall;
	}
	if (REQUIRE(seq_append)) {
		if (REQUIRE_ANY(seq_contains) != &default__seq_contains__unsupported)
			return &$with__seq_contains__and__seq_append;
	}
	if (REQUIRE(map_setnew))
		return &$with__map_setnew;
};
