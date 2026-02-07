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
/* deemon.Set.remove()                                                  */
/************************************************************************/
[[alias(Set.remove)]]
__set_remove__(key)->?Dbool {
	int result = CALL_DEPENDENCY(set_remove, self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

/* Remove a key from a set
 * @return: 1 : Given `key' was removed
 * @return: 0 : Given `key' was wasn't present
 * @return: -1: Error */
[[wunused]] int
__set_remove__.set_remove([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__set_remove__unsupported"}
%{$with__map_operator_trygetitem__and__map_operator_delitem = {
	int temp;
	DREF DeeObject *current_value;
	DREF DeeObject *map_key_and_value[2];
	if unlikely(DeeSeq_Unpack(key, 2, map_key_and_value))
		goto err;
	current_value = CALL_DEPENDENCY(map_operator_trygetitem, self, map_key_and_value[0]);
	if unlikely(!current_value)
		goto err_map_key_and_value;
	if (current_value == ITER_DONE) {
		/* map-key doesn't exist -> can't remove */
		Dee_Decref(map_key_and_value[1]);
neq_map_key:
		Dee_Decref(map_key_and_value[0]);
		return 0;
	}
	temp = DeeObject_TryCompareEq(map_key_and_value[1], current_value);
	Dee_Decref(map_key_and_value[1]);
	Dee_Decref(current_value);
	if (Dee_COMPARE_ISERR(temp))
		goto err_map_key;
	if (Dee_COMPARE_ISNE(temp))
		goto neq_map_key;
	temp = CALL_DEPENDENCY(map_operator_delitem, self, map_key_and_value[0]);
	Dee_Decref(map_key_and_value[0]);
	if unlikely(temp)
		goto err;
	return 1;
err_map_key_and_value:
	Dee_Decref(map_key_and_value[1]);
err_map_key:
	Dee_Decref(map_key_and_value[0]);
err:
	return -1;
}}
%{$with__seq_operator_size__and__set_removeall = {
	int temp;
	size_t old_size, new_size;
	DREF DeeObject *keys;
	old_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(old_size == (size_t)-1)
		goto err;
	keys = DeeSeq_OfOneSymbolic(key);
	if unlikely(!keys)
		goto err;
	temp = CALL_DEPENDENCY(set_removeall, self, keys);
	DeeSeqOne_DecrefSymbolic(keys);
	if unlikely(temp)
		goto err;
	new_size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(new_size == (size_t)-1)
		goto err;
	return old_size == new_size ? 0 : 1;
err:
	return -1;
}}
%{$with__seq_removeall = {
	size_t count = CALL_DEPENDENCY(seq_removeall, self, key, 0, (size_t)-1, (size_t)-1);
	if unlikely(count == (size_t)-1)
		goto err;
	return count ? 1 : 0;
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

set_remove = {
	if (REQUIRE_NODEFAULT(set_removeall)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size__and__set_removeall;
	}
	if (REQUIRE(seq_removeall))
		return &$with__seq_removeall;
	if (REQUIRE(map_operator_delitem) &&
	    REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &$with__map_operator_trygetitem__and__map_operator_delitem;
};
