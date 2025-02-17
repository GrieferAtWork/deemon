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
/* deemon.Mapping.operator []()                                        */
/************************************************************************/
__map_getitem__(key)->?O {
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__map_getitem__", &key))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem, self, key);
err:
	return NULL;
}

%[define(DEFINE_default_map_getitem_with_enumerate_cb =
#ifndef DEFINED_default_map_getitem_with_enumerate_cb
#define DEFINED_default_map_getitem_with_enumerate_cb
struct default_map_getitem_with_enumerate_data {
	DeeObject      *mgied_key;    /* [1..1] The key we're looking for. */
	DREF DeeObject *mgied_result; /* [?..1][out] Result value. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct default_map_getitem_with_enumerate_data *data;
	data = (struct default_map_getitem_with_enumerate_data *)arg;
	temp = DeeObject_TryCompareEq(data->mgied_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgied_result = value;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_map_getitem_with_enumerate_cb */
)]


[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_getitem)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_unknown_key(self, key);
	return NULL;
}}
%{using [map_operator_getitem_index, map_operator_getitem_string_len_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_getitem_string_len_hash, self,
		                       DeeString_STR(key),
		                       DeeString_SIZE(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem_index, self, key_value);
err:
	return NULL;
}}
%{using [map_operator_getitem_index, map_operator_getitem_string_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_getitem_string_hash, self,
		                       DeeString_STR(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem_index, self, key_value);
err:
	return NULL;
}}
%{using map_operator_getitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem_string_len_hash, self,
	                       DeeString_STR(key),
	                       DeeString_SIZE(key),
	                       DeeString_Hash(key));
err:
	return NULL;
}}
%{using map_operator_getitem_string_hash: {
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem_string_hash, self,
	                       DeeString_STR(key),
	                       DeeString_Hash(key));
err:
	return NULL;
}}
%{using map_operator_getitem_index: {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_getitem_index, self, key_value);
err:
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_with_enumerate_cb)]] {
	struct default_map_getitem_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgied_key = key;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if unlikely(status == -3) {
		err_unbound_key(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &key);
}


map_operator_getitem = {
	DeeMH_map_enumerate_t map_enumerate;
	/*if (REQUIRE_NODEFAULT(map_operator_getitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_getitem_index)
		       ? &$with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash
		       : &$with__map_operator_getitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_getitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_getitem_index)
		       ? &$with__map_operator_getitem_index__and__map_operator_getitem_string_hash
		       : &$with__map_operator_getitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_getitem_index)) {
		return &$with__map_operator_getitem_index;
	}*/
	map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
	if (map_enumerate)
		return &$with__map_enumerate;
};


/* Returns `ITER_DONE' if `key' doesn't exist. */
[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_trygetitem)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem([[nonnull]] DeeObject *self,
                                        [[nonnull]] DeeObject *key)
%{unsupported_alias("default__map_operator_getitem__unsupported")}
%{$empty = ITER_DONE}
%{using [map_operator_trygetitem_index, map_operator_trygetitem_string_len_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_trygetitem_string_len_hash, self,
		                       DeeString_STR(key),
		                       DeeString_SIZE(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_trygetitem_index, self, key_value);
err:
	return NULL;
}}
%{using [map_operator_trygetitem_index, map_operator_trygetitem_string_hash]: {
	size_t key_value;
	if (DeeString_Check(key)) {
		return CALL_DEPENDENCY(map_operator_trygetitem_string_hash, self,
		                       DeeString_STR(key),
		                       DeeString_Hash(key));
	}
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_trygetitem_index, self, key_value);
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return ITER_DONE;
	return NULL;
}}
%{using map_operator_trygetitem_string_len_hash: {
	if (!DeeString_Check(key))
		return ITER_DONE;
	return CALL_DEPENDENCY(map_operator_trygetitem_string_len_hash, self,
	                       DeeString_STR(key),
	                       DeeString_SIZE(key),
	                       DeeString_Hash(key));
}}
%{using map_operator_trygetitem_string_hash: {
	if (!DeeString_Check(key))
		return ITER_DONE;
	return CALL_DEPENDENCY(map_operator_trygetitem_string_hash, self,
	                       DeeString_STR(key),
	                       DeeString_Hash(key));
}}
%{using map_operator_trygetitem_index: {
	size_t key_value;
	if (DeeObject_AsSize(key, &key_value))
		goto err;
	return CALL_DEPENDENCY(map_operator_trygetitem_index, self, key_value);
err:
	if (DeeError_Catch(&DeeError_NotImplemented))
		return ITER_DONE;
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_with_enumerate_cb)]] {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	data.mgied_key = key;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}}
%{using map_operator_getitem: {
	DREF DeeObject *result;
	result = CALL_DEPENDENCY(map_operator_getitem, self, key);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}} = $with__map_operator_getitem;

map_operator_trygetitem = {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	/*if (REQUIRE_NODEFAULT(map_operator_trygetitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_trygetitem_index)
		       ? &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash
		       : &$with__map_operator_trygetitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_trygetitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_trygetitem_index)
		       ? &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash
		       : &$with__map_operator_trygetitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_trygetitem_index)) {
		return &$with__map_operator_trygetitem_index;
	}*/
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash)
		return &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash)
		return &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_string_len_hash)
		return &$with__map_operator_trygetitem_string_len_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_string_hash)
		return &$with__map_operator_trygetitem_string_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index)
		return &$with__map_operator_trygetitem_index;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_getitem_index)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_unknown_key_int(self, key);
	return NULL;
}}
%{using map_operator_getitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_getitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_index = {
	DeeMH_map_operator_getitem_t map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_trygetitem_index)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported_alias("default__map_operator_getitem_index__unsupported")}
%{$empty = ITER_DONE}
%{using map_operator_trygetitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_trygetitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_index = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};



%[define(DEFINE_string_hash_equals_object =
#ifndef DEFINED_string_hash_equals_object
#define DEFINED_string_hash_equals_object
PRIVATE WUNUSED NONNULL((1, 3)) bool DCALL
string_hash_equals_object(char const *lhs, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && strcmp(lhs, DeeString_STR(rhs)) == 0);
	if (DeeBytes_Check(rhs))
		return (strlen(lhs) == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), DeeBytes_SIZE(rhs)) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}
#endif /* !DEFINED_string_hash_equals_object */
)]

%[define(DEFINE_default_map_getitem_string_hash_with_enumerate_cb =
#ifndef DEFINED_default_map_getitem_string_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_hash_with_enumerate_cb
struct default_map_getitem_string_hash_with_enumerate_data {
	char const     *mgished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t      mgished_hash;   /* Hash for `mgished_key'. */
	DREF DeeObject *mgished_result; /* [?..1][out] Result value. */
};

DEFINE_string_hash_equals_object

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_hash_with_enumerate_data *)arg;
	if (string_hash_equals_object(data->mgished_key, data->mgished_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgished_result = value;
		return -2;
	}
	return 0;
}
#endif /* !DEFINED_default_map_getitem_string_hash_with_enumerate_cb */
)]


[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_getitem_string_hash)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem_string_hash([[nonnull]] DeeObject *self,
                                                 [[nonnull]] char const *key, Dee_hash_t hash)
%{unsupported({
	err_map_unsupportedf(self, "operator [](%q)", key);
	return NULL;
})}
%{$empty = {
	err_unknown_key_str(self, key);
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_string_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}}
%{using map_operator_getitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_getitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_string_hash = {
	DeeMH_map_operator_getitem_t map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_trygetitem_string_hash)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_string_hash([[nonnull]] DeeObject *self,
                                                    [[nonnull]] char const *key, Dee_hash_t hash)
%{unsupported_alias("default__map_operator_getitem_string_hash__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_string_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{using map_operator_trygetitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_trygetitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_string_hash = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};




%[define(DEFINE_string_len_hash_equals_object =
#ifndef DEFINED_string_len_hash_equals_object
#define DEFINED_string_len_hash_equals_object
PRIVATE WUNUSED NONNULL((1, 4)) bool DCALL
string_len_hash_equals_object(char const *lhs, size_t lhs_len, Dee_hash_t lhs_hash, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return (DeeString_Hash(rhs) == lhs_hash && DeeString_EqualsBuf(rhs, lhs, lhs_len));
	if (DeeBytes_Check(rhs))
		return (lhs_len == DeeBytes_SIZE(rhs) && bcmp(lhs, DeeBytes_DATA(rhs), lhs_len) == 0);
	/* `string.operator ==' isn't implemented for any other types. */
	return false;
}
#endif /* !DEFINED_string_len_hash_equals_object */
)]

%[define(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb =
#ifndef DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
struct default_map_getitem_string_len_hash_with_enumerate_data {
	char const     *mgislhed_key;    /* [1..1] The key we're looking for. */
	size_t          mgislhed_keylen; /* Length of `mgislhed_key'. */
	Dee_hash_t      mgislhed_hash;   /* Hash for `mgislhed_key'. */
	DREF DeeObject *mgislhed_result; /* [?..1][out] Result value. */
};

DEFINE_string_len_hash_equals_object

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_map_getitem_string_len_hash_with_enumerate_data *data;
	data = (struct default_map_getitem_string_len_hash_with_enumerate_data *)arg;
	if (string_len_hash_equals_object(data->mgislhed_key, data->mgislhed_keylen, data->mgislhed_hash, key)) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgislhed_result = value;
		return -2;
	}
	return 0;
}
#endif /* !DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb */
)]


[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_getitem_string_len_hash)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem_string_len_hash([[nonnull]] DeeObject *self,
                                                     char const *key, size_t keylen,
                                                     Dee_hash_t hash)
%{unsupported({
	err_map_unsupportedf(self, "operator [](%$q)", keylen, key);
	return NULL;
})}
%{$empty = {
	err_unknown_key_str_len(self, key, keylen);
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	if unlikely(status == -3) {
		err_unbound_key_str(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_str(self, key);
err:
	return NULL;
}}
%{using map_operator_getitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_getitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_string_len_hash = {
	DeeMH_map_operator_getitem_t map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_trygetitem_string_len_hash)]]
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_string_len_hash([[nonnull]] DeeObject *self,
                                                        char const *key, size_t keylen,
                                                        Dee_hash_t hash)
%{unsupported_alias("default__map_operator_getitem_string_len_hash__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{using map_operator_trygetitem: {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_trygetitem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_string_len_hash = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};







%[define(DEFINE_default_map_bounditem_with_enumerate_cb =
#ifndef DEFINED_default_map_bounditem_with_enumerate_cb
#define DEFINED_default_map_bounditem_with_enumerate_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	(void)value;
	temp = DeeObject_TryCompareEq((DeeObject *)arg, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0)
		return value ? -2 : -3; /* Stop iteration */
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_map_bounditem_with_enumerate_cb */
)]



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_bounditem)]]
[[wunused]] int
__map_getitem__.map_operator_bounditem([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *key)
%{unsupported({
	default__map_operator_getitem__unsupported(self, key);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{$with__map_enumerate = [[prefix(DEFINE_default_map_bounditem_with_enumerate_cb)]] {
	Dee_ssize_t status;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_map_bounditem_with_enumerate_cb, key);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2) {
		status = Dee_BOUND_YES;
	} else if (status == -3) {
		status = Dee_BOUND_NO;
	} else if (status == 0) {
		status = Dee_BOUND_MISSING;
	} else {
#if Dee_BOUND_ERR != -1
		status = Dee_BOUND_ERR;
#endif /* Dee_BOUND_ERR != -1 */
	}
	return (int)status;
}}
%{$with__map_operator_contains = {
	int result_status;
	DREF DeeObject *result;
	result = CALL_DEPENDENCY(map_operator_contains, self, key);
	if unlikely(!result)
		goto err;
	result_status = DeeObject_BoolInherited(result);
	return Dee_BOUND_FROMHAS_BOUND(result_status);
err:
	return Dee_BOUND_ERR;
}}
%{using map_operator_getitem: {
	DREF DeeObject *value;
	value = CALL_DEPENDENCY(map_operator_getitem, self, key);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}} = $with__map_operator_getitem;

map_operator_bounditem = {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	DeeMH_map_operator_contains_t map_operator_contains = REQUIRE_NODEFAULT(map_operator_contains);
	if (map_operator_contains) {
		if (map_operator_contains == &default__map_operator_contains__empty)
			return &$empty;
		return &$with__map_operator_contains;
	}
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &$with__map_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_bounditem_index)]]
[[wunused]] int
__map_getitem__.map_operator_bounditem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported({
	default__map_operator_getitem_index__unsupported(self, key);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{using map_operator_bounditem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_bounditem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using map_operator_getitem_index: {
	DREF DeeObject *value;
	value = CALL_DEPENDENCY(map_operator_getitem_index, self, key);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}} = $with__map_operator_getitem_index;

map_operator_bounditem_index = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_index;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};




[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_bounditem_string_hash)]]
[[wunused]] int
__map_getitem__.map_operator_bounditem_string_hash([[nonnull]] DeeObject *self,
                                                   [[nonnull]] char const *key,
                                                   Dee_hash_t hash)
%{unsupported({
	default__map_operator_getitem_string_hash__unsupported(self, key, hash);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{using map_operator_bounditem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_bounditem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using map_operator_getitem_string_hash: {
	DREF DeeObject *value;
	value = CALL_DEPENDENCY(map_operator_getitem_string_hash, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}} = $with__map_operator_getitem_string_hash;

map_operator_bounditem_string_hash = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_string_hash;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};


[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_bounditem_string_len_hash)]]
[[wunused]] int
__map_getitem__.map_operator_bounditem_string_len_hash([[nonnull]] DeeObject *self,
                                                       char const *key, size_t keylen,
                                                       Dee_hash_t hash)
%{unsupported({
	default__map_operator_getitem_string_len_hash__unsupported(self, key, keylen, hash);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{using map_operator_bounditem: {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(map_operator_bounditem, self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using map_operator_getitem_string_len_hash: {
	DREF DeeObject *value;
	value = CALL_DEPENDENCY(map_operator_getitem_string_len_hash, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}} = $with__map_operator_getitem_string_len_hash;

map_operator_bounditem_string_len_hash = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_string_len_hash;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};





[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_hasitem)]]
[[wunused]] int
__map_getitem__.map_operator_hasitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key)
%{$empty = 0}
%{using map_operator_bounditem: {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = CALL_DEPENDENCY(map_operator_bounditem, self, key);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem;

map_operator_hasitem = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem;
#else /* Dee_BOUND_MAYALIAS_HAS */
		if (map_operator_bounditem == &default__map_operator_bounditem__empty)
			return &$empty;
		return &$with__map_operator_bounditem;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_hasitem_index)]]
[[wunused]] int
__map_getitem__.map_operator_hasitem_index([[nonnull]] DeeObject *self, size_t key)
%{$empty = 0}
%{using map_operator_bounditem_index: {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = CALL_DEPENDENCY(map_operator_bounditem_index, self, key);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_index;

map_operator_hasitem_index = {
	DeeMH_map_operator_bounditem_index_t map_operator_bounditem_index = REQUIRE(map_operator_bounditem_index);
	if (map_operator_bounditem_index) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_index;
#else /* Dee_BOUND_MAYALIAS_HAS */
		if (map_operator_bounditem_index == &default__map_operator_bounditem_index__empty)
			return &$empty;
		return &$with__map_operator_bounditem_index;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_hasitem_string_hash)]]
[[wunused]] int
__map_getitem__.map_operator_hasitem_string_hash([[nonnull]] DeeObject *self,
                                                 [[nonnull]] char const *key,
                                                 Dee_hash_t hash)
%{$empty = 0}
%{using map_operator_bounditem_string_hash: {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = CALL_DEPENDENCY(map_operator_bounditem_string_hash, self, key, hash);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_string_hash;

map_operator_hasitem_string_hash = {
	DeeMH_map_operator_bounditem_string_hash_t map_operator_bounditem_string_hash = REQUIRE(map_operator_bounditem_string_hash);
	if (map_operator_bounditem_string_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		if (map_operator_bounditem_string_hash == &default__map_operator_bounditem_string_hash__empty)
			return &$empty;
		return &$with__map_operator_bounditem_string_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[operator(Mapping.OPERATOR_GETITEM: tp_seq->tp_hasitem_string_len_hash)]]
[[wunused]] int
__map_getitem__.map_operator_hasitem_string_len_hash([[nonnull]] DeeObject *self,
                                                     char const *key, size_t keylen,
                                                     Dee_hash_t hash)
%{$empty = 0}
%{using map_operator_bounditem_string_len_hash: {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = CALL_DEPENDENCY(map_operator_bounditem_string_len_hash, self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_string_len_hash;

map_operator_hasitem_string_len_hash = {
	DeeMH_map_operator_bounditem_string_len_hash_t map_operator_bounditem_string_len_hash = REQUIRE(map_operator_bounditem_string_len_hash);
	if (map_operator_bounditem_string_len_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_len_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		if (map_operator_bounditem_string_len_hash == &default__map_operator_bounditem_string_len_hash__empty)
			return &$empty;
		return &$with__map_operator_bounditem_string_len_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};
