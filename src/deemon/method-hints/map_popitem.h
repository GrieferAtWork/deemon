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
/* deemon.Mapping.popitem()                                             */
/************************************************************************/
[[alias(Mapping.popitem)]]
__map_popitem__()->?X2?T2?O?O?N {
	if (DeeArg_Unpack(argc, argv, ":__map_popitem__"))
		goto err;
	return CALL_DEPENDENCY(map_popitem, self);
err:
	return NULL;
}


/* Remove a random key/value pair from `self' and store it in `key_and_value'
 * Returns "Dee_None" if nothing found */
[[wunused]] DREF DeeObject *
__map_popitem__.map_popitem([[nonnull]] DeeObject *self)
%{unsupported(auto)}
/*%{$none = return_none}*/
%{$empty = return_none}
%{$with__seq_trygetlast__and__map_operator_delitem = {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetlast, self);
	if (result == ITER_DONE)
		return_none;
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	temp = CALL_DEPENDENCY(map_operator_delitem, self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{$with__seq_trygetfirst__and__map_operator_delitem = {
	int temp;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *result = CALL_DEPENDENCY(seq_trygetfirst, self);
	if (result == ITER_DONE)
		return_none;
	if unlikely(!result)
		goto err;
	if unlikely(DeeSeq_Unpack(result, 2, key_and_value))
		goto err_r;
	Dee_Decref(key_and_value[1]);
	temp = CALL_DEPENDENCY(map_operator_delitem, self, key_and_value[0]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{$with__seq_pop = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_pop, self, -1);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError))
			return_none;
		goto err;
	}
	return result;
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

map_popitem = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem) {
		DeeMH_seq_trygetfirst_t seq_trygetfirst;
		if (map_operator_delitem == &default__map_operator_delitem__empty)
			return &$empty;
		if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
			DeeMH_map_remove_t map_remove = REQUIRE(map_remove);
			if (map_remove == &default__map_remove__empty)
				return &$empty;
			if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index ||
			    map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
				return &$with__seq_pop;
		}
		if (REQUIRE_NODEFAULT(seq_trygetlast))
			return &$with__seq_trygetlast__and__map_operator_delitem;
		seq_trygetfirst = REQUIRE(seq_trygetfirst);
		if (seq_trygetfirst) {
			if (seq_trygetfirst == &default__seq_trygetfirst__empty)
				return &$empty;
			if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast)
				return &$with__seq_trygetlast__and__map_operator_delitem;
			return &$with__seq_trygetfirst__and__map_operator_delitem;
		}
	}
	if (REQUIRE(seq_pop))
		return &$with__seq_pop;
};
