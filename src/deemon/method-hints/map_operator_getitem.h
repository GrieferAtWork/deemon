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
	return DeeMap_OperatorGetItem(self, key);
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

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_with_enumerate_cb */
)]


[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_unknown_key(self, key);
	return NULL;
}}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_with_enumerate_cb)]] {
	struct default_map_getitem_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgied_key = key;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_with_enumerate_cb, &data);
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
	DeeMH_map_operator_enumerate_t map_operator_enumerate;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireGetItem(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_getitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_enumerate = REQUIRE(map_operator_enumerate);
	if (map_operator_enumerate == &default__map_operator_enumerate__empty)
		return &$empty;
	if (map_operator_enumerate)
		return &$with__map_operator_enumerate;
};


/* Returns `ITER_DONE' if `key' doesn't exist. */
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem([[nonnull]] DeeObject *self,
                                        [[nonnull]] DeeObject *key)
%{unsupported_alias("default__map_operator_getitem__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_with_enumerate_cb)]] {
	Dee_ssize_t status;
	struct default_map_getitem_with_enumerate_data data;
	data.mgied_key = key;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgied_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}}
%{$with__map_operator_getitem = {
	DREF DeeObject *result;
	result = DeeMap_OperatorGetItem(self, key);
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
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItem(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_trygetitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};



%[define(DEFINE_default_map_getitem_index_with_enumerate_index_cb =
#ifndef DEFINED_default_map_getitem_index_with_enumerate_index_cb
#define DEFINED_default_map_getitem_index_with_enumerate_index_cb
struct default_map_getitem_index_with_enumerate_index_data {
	size_t          mgiieid_key;    /* The key we're looking for. */
	DREF DeeObject *mgiieid_result; /* [?..1][out] Result value. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_map_getitem_index_with_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	struct default_map_getitem_index_with_enumerate_index_data *data;
	data = (struct default_map_getitem_index_with_enumerate_index_data *)arg;
	if (data->mgiieid_key == key) {
		if unlikely(!value)
			return -3;
		Dee_Incref(value);
		data->mgiieid_result = value;
		return -2;
	}
	return 0;
}
#endif /* !DEFINED_default_map_getitem_index_with_enumerate_index_cb */
)]




[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_getitem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_unknown_key_int(self, key);
	return NULL;
}}
%{$with__map_operator_enumerate_index = [[prefix(DEFINE_default_map_getitem_index_with_enumerate_index_cb)]] {
	struct default_map_getitem_index_with_enumerate_index_data data;
	Dee_ssize_t status;
	data.mgiieid_key = key;
	status = DeeMap_OperatorEnumerateIndex(self, &default_map_getitem_index_with_enumerate_index_cb, &data, key, key + 1);
	if likely(status == -2)
		return data.mgiieid_result;
	if unlikely(status == -3) {
		err_unbound_key_int(self, key);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_unknown_key_int(self, key);
err:
	return NULL;
}}
%{$with__map_operator_getitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_index = {
	DeeMH_map_operator_getitem_t map_operator_getitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireGetItemIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_getitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate_index;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported_alias("default__map_operator_getitem_index__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_operator_enumerate_index = [[prefix(DEFINE_default_map_getitem_index_with_enumerate_index_cb)]] {
	struct default_map_getitem_index_with_enumerate_index_data data;
	Dee_ssize_t status;
	data.mgiieid_key = key;
	status = DeeMap_OperatorEnumerateIndex(self, &default_map_getitem_index_with_enumerate_index_cb, &data, key, key + 1);
	if likely(status == -2)
		return data.mgiieid_result;
	if (status == -3 || status == 0)
		return ITER_DONE;
	ASSERT(status == -1);
	return NULL;
}}
%{$with__map_operator_trygetitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorTryGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_index = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_trygetitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate_index;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};




%[define(DEFINE_default_map_getitem_string_hash_with_enumerate_cb =
#ifndef DEFINED_default_map_getitem_string_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_hash_with_enumerate_cb
struct default_map_getitem_string_hash_with_enumerate_data {
	char const     *mgished_key;    /* [1..1] The key we're looking for. */
	Dee_hash_t      mgished_hash;   /* Hash for `mgished_key'. */
	DREF DeeObject *mgished_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_hash_with_enumerate_cb */
)]


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
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_string_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
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
%{$with__map_operator_getitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_string_hash = {
	DeeMH_map_operator_getitem_t map_operator_getitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireGetItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_getitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_string_hash([[nonnull]] DeeObject *self,
                                                    [[nonnull]] char const *key, Dee_hash_t hash)
%{unsupported_alias("default__map_operator_getitem_string_hash__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_string_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgished_key  = key;
	data.mgished_hash = hash;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_string_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgished_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_operator_trygetitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorTryGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_string_hash = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_trygetitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};




%[define(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb =
#ifndef DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
#define DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb
struct default_map_getitem_string_len_hash_with_enumerate_data {
	char const     *mgislhed_key;    /* [1..1] The key we're looking for. */
	size_t          mgislhed_keylen; /* Length of `mgislhed_key'. */
	Dee_hash_t      mgislhed_hash;   /* Hash for `mgislhed_key'. */
	DREF DeeObject *mgislhed_result; /* [?..1][out] Result value. */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_getitem_string_len_hash_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_getitem_string_len_hash_with_enumerate_cb */
)]


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
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
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
%{$with__map_operator_getitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_getitem;

map_operator_getitem_string_len_hash = {
	DeeMH_map_operator_getitem_t map_operator_getitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireGetItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_getitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};




/* Returns `ITER_DONE' if `key' doesn't exist. */
[[wunused]] DREF DeeObject *
__map_getitem__.map_operator_trygetitem_string_len_hash([[nonnull]] DeeObject *self,
                                                        char const *key, size_t keylen,
                                                        Dee_hash_t hash)
%{unsupported_alias("default__map_operator_getitem_string_len_hash__unsupported")}
%{$empty = ITER_DONE}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_getitem_string_len_hash_with_enumerate_cb)]] {
	struct default_map_getitem_string_len_hash_with_enumerate_data data;
	Dee_ssize_t status;
	data.mgislhed_key    = key;
	data.mgislhed_keylen = keylen;
	data.mgislhed_hash   = hash;
	status = DeeMap_OperatorEnumerate(self, &default_map_getitem_string_len_hash_with_enumerate_cb, &data);
	if likely(status == -2)
		return data.mgislhed_result;
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_operator_trygetitem = {
	DREF DeeObject *result, *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorTryGetItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return NULL;
}} = $with__map_operator_trygetitem;

map_operator_trygetitem_string_len_hash = {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_trygetitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_trygetitem = REQUIRE(map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &$empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_trygetitem)
		return &$with__map_operator_trygetitem;
};







%[define(DEFINE_default_map_bounditem_with_enumerate_cb =
#ifndef DEFINED_default_map_bounditem_with_enumerate_cb
#define DEFINED_default_map_bounditem_with_enumerate_cb
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_map_bounditem_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_map_bounditem_with_enumerate_cb */
)]



[[wunused]] int
__map_getitem__.map_operator_bounditem([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *key)
%{unsupported({
	default__map_operator_getitem__unsupported(self, key);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_map_bounditem_with_enumerate_cb)]] {
	Dee_ssize_t status;
	status = DeeMap_OperatorEnumerate(self, &default_map_bounditem_with_enumerate_cb, key);
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
	result = DeeMap_OperatorContains(self, key);
	if unlikely(!result)
		goto err;
	result_status = DeeObject_BoolInherited(result);
	return Dee_BOUND_FROMHAS_BOUND(result_status);
err:
	return Dee_BOUND_ERR;
}}
%{$with__map_operator_getitem = {
	DREF DeeObject *value;
	value = DeeMap_OperatorGetItem(self, key);
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
	DeeMH_map_operator_contains_t map_operator_contains;
	DeeMH_map_operator_getitem_t map_operator_getitem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireBoundItem(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_bounditem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_contains = REQUIRE_NODEFAULT(map_operator_contains);
	if (map_operator_contains) {
		if (map_operator_contains == &default__map_operator_contains__empty)
			return &$empty;
		return &$with__map_operator_contains;
	}
	map_operator_getitem = REQUIRE(map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &$empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_enumerate)
		return &$with__map_operator_enumerate;
	if (map_operator_getitem)
		return &$with__map_operator_getitem;
};



[[wunused]] int
__map_getitem__.map_operator_bounditem_index([[nonnull]] DeeObject *self, size_t key)
%{unsupported({
	default__map_operator_getitem_index__unsupported(self, key);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{$with__map_operator_bounditem = {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeInt_NewSize(key);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorBoundItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{$with__map_operator_getitem_index = {
	DREF DeeObject *value;
	value = DeeMap_OperatorGetItemIndex(self, key);
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
}}
= $with__map_operator_getitem_index;

map_operator_bounditem_index = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_bounditem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_index;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};




[[wunused]] int
__map_getitem__.map_operator_bounditem_string_hash([[nonnull]] DeeObject *self,
                                                   [[nonnull]] char const *key,
                                                   Dee_hash_t hash)
%{unsupported({
	default__map_operator_getitem_string_hash__unsupported(self, key, hash);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{$with__map_operator_bounditem = {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorBoundItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{$with__map_operator_getitem_string_hash = {
	DREF DeeObject *value;
	value = DeeMap_OperatorGetItemStringHash(self, key, hash);
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
}}
= $with__map_operator_getitem_string_hash;

map_operator_bounditem_string_hash = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_bounditem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_string_hash;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};


[[wunused]] int
__map_getitem__.map_operator_bounditem_string_len_hash([[nonnull]] DeeObject *self,
                                                       char const *key, size_t keylen,
                                                       Dee_hash_t hash)
%{unsupported({
	default__map_operator_getitem_string_len_hash__unsupported(self, key, keylen, hash);
	return Dee_BOUND_ERR;
})}
%{$empty = Dee_BOUND_MISSING}
%{$with__map_operator_bounditem = {
	int result;
	DREF DeeObject *keyob;
	keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = DeeMap_OperatorBoundItem(self, keyob);
	Dee_Decref(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{$with__map_operator_getitem_string_len_hash = {
	DREF DeeObject *value;
	value = DeeMap_OperatorGetItemStringLenHash(self, key, keylen, hash);
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
}}
= $with__map_operator_getitem_string_len_hash;

map_operator_bounditem_string_len_hash = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemStringLenHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_bounditem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &$empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &$with__map_operator_getitem_string_len_hash;
	if (map_operator_bounditem)
		return &$with__map_operator_bounditem;
};





[[wunused]] int
__map_getitem__.map_operator_hasitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *key)
%{$with__map_operator_bounditem = {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = DeeMap_OperatorBoundItem(self, key);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem;

map_operator_hasitem = {
	DeeMH_map_operator_bounditem_t map_operator_bounditem;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireHasItem(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_hasitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem = REQUIRE(map_operator_bounditem);
	if (map_operator_bounditem) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &$with__map_operator_bounditem;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[wunused]] int
__map_getitem__.map_operator_hasitem_index([[nonnull]] DeeObject *self, size_t key)
%{$with__map_operator_bounditem_index = {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = DeeMap_OperatorBoundItemIndex(self, key);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_index;

map_operator_hasitem_index = {
	DeeMH_map_operator_bounditem_index_t map_operator_bounditem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireHasItemIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_hasitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem_index = REQUIRE(map_operator_bounditem_index);
	if (map_operator_bounditem_index) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_index;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &$with__map_operator_bounditem_index;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[wunused]] int
__map_getitem__.map_operator_hasitem_string_hash([[nonnull]] DeeObject *self,
                                                 [[nonnull]] char const *key,
                                                 Dee_hash_t hash)
%{$with__map_operator_bounditem_string_hash = {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = DeeMap_OperatorBoundItemStringHash(self, key, hash);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_string_hash;

map_operator_hasitem_string_hash = {
	DeeMH_map_operator_bounditem_string_hash_t map_operator_bounditem_string_hash;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireHasItemStringHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_hasitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem_string_hash = REQUIRE(map_operator_bounditem_string_hash);
	if (map_operator_bounditem_string_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &$with__map_operator_bounditem_string_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};



[[wunused]] int
__map_getitem__.map_operator_hasitem_string_len_hash([[nonnull]] DeeObject *self,
                                                     char const *key, size_t keylen,
                                                     Dee_hash_t hash)
%{$with__map_operator_bounditem_string_len_hash = {
	/* TODO: Only define when `#ifndef Dee_BOUND_MAYALIAS_HAS' */
	int result = DeeMap_OperatorBoundItemStringLenHash(self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}} = $with__map_operator_bounditem_string_len_hash;

map_operator_hasitem_string_len_hash = {
	DeeMH_map_operator_bounditem_string_len_hash_t map_operator_bounditem_string_len_hash;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireHasItemStringLenHash(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_hasitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_bounditem_string_len_hash = REQUIRE(map_operator_bounditem_string_len_hash);
	if (map_operator_bounditem_string_len_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_len_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &$with__map_operator_bounditem_string_len_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
};
