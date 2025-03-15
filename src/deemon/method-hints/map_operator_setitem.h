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
/* deemon.Mapping.operator []=()                                        */
/************************************************************************/
__map_setitem__(key,value) {
	if unlikely(CALL_DEPENDENCY(map_operator_setitem, self, key, value))
		goto err;
	return_none;
err:
	return NULL;
}

[[operator(Mapping: tp_seq->tp_setitem)]]
[[wunused]] int
__map_setitem__.map_operator_setitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key,
                                     [[nonnull]] DeeObject *value)
%{unsupported(auto("operator []="))}
%{$none = 0}
%{$empty = {
	(void)value;
	return err_unknown_key(self, key);
}}
%{using [map_operator_setitem_index, map_operator_setitem_string_len_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_setitem_string_len_hash, self,
		                       DeeString_STR(key),
		                       DeeString_SIZE(key),
		                       DeeString_Hash(key),
		                       value);
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_setitem_index, self, key_value, value);
err:
	return -1;
}}
%{using [map_operator_setitem_index, map_operator_setitem_string_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_setitem_string_hash, self,
		                       DeeString_STR(key),
		                       DeeString_Hash(key), value);
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_setitem_index, self, key_value, value);
err:
	return -1;
}}
%{using map_operator_setitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_setitem_string_len_hash, self,
	                       DeeString_STR(key),
	                       DeeString_SIZE(key),
	                       DeeString_Hash(key),
	                       value);
err:
	return -1;
}}
%{using map_operator_setitem_string_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_setitem_string_hash, self,
	                       DeeString_STR(key),
	                       DeeString_Hash(key),
	                       value);
err:
	return -1;
}}
%{using map_operator_setitem_index: {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_setitem_index, self, key_value, value);
err:
	return -1;
}}
%{$with__seq_enumerate__and__seq_operator_setitem__and__seq_append = [[prefix(DEFINE_map_setold_ex_with_seq_enumerate_cb)]] {
	int result;
	Dee_ssize_t status;
	DREF DeeObject *new_pair;
	struct map_setold_ex_with_seq_enumerate_data data;
	data.msoxwse_seq = self;
	data.msoxwse_key_and_value[0] = key;
	data.msoxwse_key_and_value[1] = value;
	status = CALL_DEPENDENCY(seq_enumerate, self, &map_setold_ex_with_seq_enumerate_cb, &data);
	if (status == MAP_SETOLD_EX_WITH_SEQ_ENUMERATE__SUCCESS) {
		Dee_Decref(data.msoxwse_key_and_value[1]);
		return 0;
	}
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	new_pair = DeeTuple_NewVector(2, data.msoxwse_key_and_value);
	if unlikely(!new_pair)
		goto err;
	result = CALL_DEPENDENCY(seq_append, self, new_pair);
	Dee_Decref_unlikely(new_pair);
	return result;
err:
	return -1;
}}
%{$with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append = [[prefix(DEFINE_map_setold_ex_with_seq_enumerate_index_cb)]] {
	int result;
	Dee_ssize_t status;
	DREF DeeObject *new_pair;
	struct map_setold_ex_with_seq_enumerate_index_data data;
	data.msoxwsei_seq = self;
	data.msoxwsei_key_and_value[0] = key;
	data.msoxwsei_key_and_value[1] = value;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &map_setold_ex_with_seq_enumerate_index_cb, &data, 0, (size_t)-1);
	if (status == MAP_SETOLD_EX_WITH_SEQ_ENUMERATE_INDEX__SUCCESS) {
		Dee_Decref(data.msoxwsei_key_and_value[1]);
		return 0;
	}
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	new_pair = DeeTuple_NewVector(2, data.msoxwsei_key_and_value);
	if unlikely(!new_pair)
		goto err;
	result = CALL_DEPENDENCY(seq_append, self, new_pair);
	Dee_Decref_unlikely(new_pair);
	return result;
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
	Dee_Decref_unlikely(result); /* *_unlikely because return value is probably "none" */
	return 0;
err:
	return -1;
}


map_operator_setitem = {
	DeeMH_map_enumerate_t map_enumerate;
	DeeMH_seq_operator_setitem_t seq_operator_setitem;
	/*if (REQUIRE_NODEFAULT(map_operator_setitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_setitem_index)
		       ? &$with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash
		       : &$with__map_operator_setitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_setitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_setitem_index)
		       ? &$with__map_operator_setitem_index__and__map_operator_setitem_string_hash
		       : &$with__map_operator_setitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_setitem_index)) {
		return &$with__map_operator_setitem_index;
	}*/
	if (((seq_operator_setitem = REQUIRE(seq_operator_setitem)) != NULL &&
	     (REQUIRE_ANY(seq_append) != &default__seq_append__unsupported)) ||
	    ((REQUIRE(seq_append) != NULL) &&
	     (seq_operator_setitem = REQUIRE_ANY(seq_operator_setitem)) != &default__seq_operator_setitem__unsupported)) {
		if (seq_operator_setitem == &default__seq_operator_setitem__empty)
			return &$empty;
		if (seq_operator_setitem == &default__seq_operator_setitem__with__seq_operator_setitem_index &&
		    REQUIRE_ANY(seq_enumerate_index) != &default__seq_enumerate_index__unsupported)
			return &$with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append;
		if (REQUIRE_ANY(seq_enumerate) != &default__seq_enumerate__unsupported)
			return &$with__seq_enumerate__and__seq_operator_setitem__and__seq_append;
	}
	map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
};





[[operator(Mapping: tp_seq->tp_setitem_index)]]
[[wunused]] int
__map_setitem__.map_operator_setitem_index([[nonnull]] DeeObject *self, size_t key,
                                           [[nonnull]] DeeObject *value)
%{unsupported(auto("operator []="))}
%{$none = 0}
%{$empty = {
	(void)value;
	return err_unknown_key_int(self, key);
}}
%{using map_operator_setitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_setitem, self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_setitem;

map_operator_setitem_index = {
	DeeMH_map_operator_setitem_t map_operator_setitem = REQUIRE(map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &$empty;
	if (map_operator_setitem)
		return &$with__map_operator_setitem;
};




[[operator(Mapping: tp_seq->tp_setitem_string_hash)]]
[[wunused]] int
__map_setitem__.map_operator_setitem_string_hash([[nonnull]] DeeObject *self,
                                                 [[nonnull]] char const *key, Dee_hash_t hash,
                                                 [[nonnull]] DeeObject *value)
%{unsupported({
	return err_map_unsupportedf(self, "operator []=(%q)", key);
})}
%{$none = 0}
%{$empty = {
	(void)hash;
	(void)value;
	return err_unknown_key_str(self, key);
}}
%{using map_operator_setitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_setitem, self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_setitem;

map_operator_setitem_string_hash = {
	DeeMH_map_operator_setitem_t map_operator_setitem = REQUIRE(map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &$empty;
	if (map_operator_setitem)
		return &$with__map_operator_setitem;
};




[[operator(Mapping: tp_seq->tp_setitem_string_len_hash)]]
[[wunused]] int
__map_setitem__.map_operator_setitem_string_len_hash([[nonnull]] DeeObject *self,
                                                     char const *key, size_t keylen,
                                                     Dee_hash_t hash,
                                                     [[nonnull]] DeeObject *value)
%{unsupported({
	return err_map_unsupportedf(self, "operator []=(%$q)", keylen, key);
})}
%{$none = 0}
%{$empty = {
	(void)hash;
	(void)value;
	return err_unknown_key_str_len(self, key, keylen);
}}
%{using map_operator_setitem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_setitem, self, keyob, value);
	Dee_Decref(keyob);
	return result;
err:
	return -1;
}} = $with__map_operator_setitem;

map_operator_setitem_string_len_hash = {
	DeeMH_map_operator_setitem_t map_operator_setitem = REQUIRE(map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &$empty;
	if (map_operator_setitem)
		return &$with__map_operator_setitem;
};
