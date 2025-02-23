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
/* deemon.Mapping.operator del[]()                                      */
/************************************************************************/
__map_delitem__(key)->?O {
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_delitem__", &key))
		goto err;
	if unlikely(CALL_DEPENDENCY(map_operator_delitem, self, key))
		goto err;
	return_none;
err:
	return NULL;
}

[[operator(Mapping: tp_seq->tp_delitem)]]
[[wunused]] int
__map_delitem__.map_operator_delitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key)
%{unsupported(auto("operator del[]"))}
%{$empty = 0}
%{$with__map_remove = {
	int result = CALL_DEPENDENCY(map_remove, self, key);
	if (result > 0)
		result = 0;
	return result;
}}
%{$with__map_pop_with_default = {
	DREF DeeObject *oldvalue = CALL_DEPENDENCY(map_pop_with_default, self, key, Dee_None);
	if unlikely(!oldvalue)
		goto err;
	Dee_Decref(oldvalue);
	return 0;
err:
	return -1;
}}
%{$with__map_removekeys = {
	int result;
	DREF DeeTupleObject *keys = DeeTuple_NewUninitialized(1);
	if unlikely(!keys)
		goto err;
	keys->t_elem[0] = key;
	result = CALL_DEPENDENCY(map_removekeys, self, (DeeObject *)keys);
	DeeTuple_DecrefSymbolic((DeeObject *)keys);
	return result;
err:
	return -1;
}}
%{using [map_operator_delitem_index, map_operator_delitem_string_len_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_delitem_string_len_hash, self,
		                       DeeString_STR(key),
		                       DeeString_SIZE(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_delitem_index, self, key_value);
err:
	return -1;
}}
%{using [map_operator_delitem_index, map_operator_delitem_string_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_delitem_string_hash, self,
		                       DeeString_STR(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_delitem_index, self, key_value);
err:
	return -1;
}}
%{using map_operator_delitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_delitem_string_len_hash, self,
	                       DeeString_STR(key),
	                       DeeString_SIZE(key),
	                       DeeString_Hash(key));
err:
	return -1;
}}
%{using map_operator_delitem_string_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_delitem_string_hash, self,
	                       DeeString_STR(key),
	                       DeeString_Hash(key));
err:
	return -1;
}}
%{using map_operator_delitem_index: {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_delitem_index, self, key_value);
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &key);
	if unlikely(!result)
		goto err;
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}


map_operator_delitem = {
	DeeMH_seq_operator_delitem_t seq_operator_delitem;
	DeeMH_map_enumerate_t map_enumerate;
	/*if (REQUIRE_NODEFAULT(map_operator_delitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_delitem_index)
		       ? &$with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash
		       : &$with__map_operator_delitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_delitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_delitem_index)
		       ? &$with__map_operator_delitem_index__and__map_operator_delitem_string_hash
		       : &$with__map_operator_delitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_delitem_index)) {
		return &$with__map_operator_delitem_index;
	}*/
	if (REQUIRE_NODEFAULT(map_remove))
		return &$with__map_remove;
	if (REQUIRE_NODEFAULT(map_pop_with_default))
		return &$with__map_pop_with_default;
	if (REQUIRE_NODEFAULT(map_removekeys))
		return &$with__map_removekeys;
	seq_operator_delitem = REQUIRE(seq_operator_delitem);
	if (seq_operator_delitem) {
		if (seq_operator_delitem == &default__seq_operator_delitem__empty)
			return &$empty;
		if (REQUIRE_ANY(seq_enumerate) != &default__seq_enumerate__unsupported)
			return &$with__map_remove; /* See selector in `map_remove' */
	}
	map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
};





[[operator(Mapping: tp_seq->tp_delitem_index)]]
[[wunused]] int
__map_delitem__.map_operator_delitem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported(auto("operator del[]"))}
%{$empty = 0}
%{using map_operator_delitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_delitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_delitem;

map_operator_delitem_index = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem)
		return &$with__map_operator_delitem;
};




[[operator(Mapping: tp_seq->tp_delitem_string_hash)]]
[[wunused]] int
__map_delitem__.map_operator_delitem_string_hash([[nonnull]] DeeObject *self,
                                                 [[nonnull]] char const *key, Dee_hash_t hash)
%{unsupported({
	return err_map_unsupportedf(self, "operator del[](%q)", key);
})}
%{$empty = 0}
%{using map_operator_delitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_delitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_delitem;

map_operator_delitem_string_hash = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem)
		return &$with__map_operator_delitem;
};




[[operator(Mapping: tp_seq->tp_delitem_string_len_hash)]]
[[wunused]] int
__map_delitem__.map_operator_delitem_string_len_hash([[nonnull]] DeeObject *self,
                                                     char const *key, size_t keylen,
                                                     Dee_hash_t hash)
%{unsupported({
	return err_map_unsupportedf(self, "operator del[](%$q)", keylen, key);
})}
%{$empty = 0}
%{using map_operator_delitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_delitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_delitem;

map_operator_delitem_string_len_hash = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem)
		return &$with__map_operator_delitem;
};
