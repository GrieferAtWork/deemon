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


%[define(DEFINE_map_pop_with_seq_enumerate_cb =
#ifndef DEFINED_map_pop_with_seq_enumerate_cb
#define DEFINED_map_pop_with_seq_enumerate_cb
#define MAP_POP_WITH_SEQ_ENUMERATE__SUCCESS (-2)
struct map_pop_with_seq_enumerate_data {
	DeeObject *mpwse_seq; /* [1..1] Sequence to pop "mpwse_key" from */
	DeeObject *mpwse_key; /* [1..1] Key to pop from "<mpwse_seq> as Mapping" */
};
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_pop_with_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	int status;
	DREF DeeObject *this_key_and_value[2];
	struct map_pop_with_seq_enumerate_data *data;
	data = (struct map_pop_with_seq_enumerate_data *)arg;
	if (value) {
		if (DeeObject_Unpack(value, 2, this_key_and_value))
			goto err;
		status = DeeObject_TryCompareEq(data->mpwse_key, this_key_and_value[0]);
		Dee_Decref(this_key_and_value[0]);
		if unlikely(status == Dee_COMPARE_ERR)
			goto err_this_value;
		if (status == 0) {
			/* Found it! */
			status = DeeObject_InvokeMethodHint(seq_operator_delitem, data->mpwse_seq, index);
			if unlikely(status < 0)
				goto err_this_value;
			data->mpwse_key = this_key_and_value[1]; /* Inherit reference */
			return MAP_POP_WITH_SEQ_ENUMERATE__SUCCESS;
		}
		Dee_Decref(this_key_and_value[1]);
	}
	return 0;
err_this_value:
	Dee_Decref(this_key_and_value[1]);
err:
	return -1;
}
#endif /* !DEFINED_map_pop_with_seq_enumerate_cb */
)]


%[define(DEFINE_map_pop_with_seq_enumerate_index_cb =
#ifndef DEFINED_map_pop_with_seq_enumerate_index_cb
#define DEFINED_map_pop_with_seq_enumerate_index_cb
#define MAP_POP_WITH_SEQ_ENUMERATE_INDEX__SUCCESS (-2)
struct map_pop_with_seq_enumerate_index_data {
	DeeObject *mpwsei_seq; /* [1..1] Sequence to pop "mpwsei_key" from */
	DeeObject *mpwsei_key; /* [1..1] Key to pop from "<mpwsei_seq> as Mapping" */
};
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
map_pop_with_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	int status;
	DREF DeeObject *this_key_and_value[2];
	struct map_pop_with_seq_enumerate_index_data *data;
	data = (struct map_pop_with_seq_enumerate_index_data *)arg;
	if (value) {
		if (DeeObject_Unpack(value, 2, this_key_and_value))
			goto err;
		status = DeeObject_TryCompareEq(data->mpwsei_key, this_key_and_value[0]);
		Dee_Decref(this_key_and_value[0]);
		if unlikely(status == Dee_COMPARE_ERR)
			goto err_this_value;
		if (status == 0) {
			/* Found it! */
			status = DeeObject_InvokeMethodHint(seq_operator_delitem_index, data->mpwsei_seq, index);
			if unlikely(status < 0)
				goto err_this_value;
			data->mpwsei_key = this_key_and_value[1]; /* Inherit reference */
			return MAP_POP_WITH_SEQ_ENUMERATE_INDEX__SUCCESS;
		}
		Dee_Decref(this_key_and_value[1]);
	}
	return 0;
err_this_value:
	Dee_Decref(this_key_and_value[1]);
err:
	return -1;
}
#endif /* !DEFINED_map_pop_with_seq_enumerate_index_cb */
)]



/* Remove/unbind `key' and return whatever used to be assigned to it.
 * When the key was already absent/unbound, return `default_' or throw a `KeyError'
 * @return: * :   The old value of `key'
 * @return: NULL: Error */
[[wunused]] DREF DeeObject *
__map_pop__.map_pop([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = return_none}
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
}}
%{$with__seq_enumerate__and__seq_operator_delitem = [[prefix(DEFINE_map_pop_with_seq_enumerate_cb)]] {
	Dee_ssize_t status;
	struct map_pop_with_seq_enumerate_data data;
	data.mpwse_seq = self;
	data.mpwse_key = key;
	status = CALL_DEPENDENCY(seq_enumerate, self, &map_pop_with_seq_enumerate_cb, &data);
	if (status == MAP_POP_WITH_SEQ_ENUMERATE__SUCCESS)
		return data.mpwse_key;
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	err_unknown_key(self, key);
err:
	return NULL;
}}
%{$with__seq_enumerate_index__and__seq_operator_delitem_index = [[prefix(DEFINE_map_pop_with_seq_enumerate_index_cb)]] {
	Dee_ssize_t status;
	struct map_pop_with_seq_enumerate_index_data data;
	data.mpwsei_seq = self;
	data.mpwsei_key = key;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &map_pop_with_seq_enumerate_index_cb, &data, 0, (size_t)-1);
	if (status == MAP_POP_WITH_SEQ_ENUMERATE_INDEX__SUCCESS)
		return data.mpwsei_key;
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	err_unknown_key(self, key);
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
%{$none = return_none}
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
}}
%{$with__seq_enumerate__and__seq_operator_delitem = [[prefix(DEFINE_map_pop_with_seq_enumerate_cb)]] {
	Dee_ssize_t status;
	struct map_pop_with_seq_enumerate_data data;
	data.mpwse_seq = self;
	data.mpwse_key = key;
	status = CALL_DEPENDENCY(seq_enumerate, self, &map_pop_with_seq_enumerate_cb, &data);
	if (status == MAP_POP_WITH_SEQ_ENUMERATE__SUCCESS)
		return data.mpwse_key;
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	return_reference_(default_);
err:
	return NULL;
}}
%{$with__seq_enumerate_index__and__seq_operator_delitem_index = [[prefix(DEFINE_map_pop_with_seq_enumerate_index_cb)]] {
	Dee_ssize_t status;
	struct map_pop_with_seq_enumerate_index_data data;
	data.mpwsei_seq = self;
	data.mpwsei_key = key;
	status = CALL_DEPENDENCY(seq_enumerate_index, self, &map_pop_with_seq_enumerate_index_cb, &data, 0, (size_t)-1);
	if (status == MAP_POP_WITH_SEQ_ENUMERATE_INDEX__SUCCESS)
		return data.mpwsei_key;
	ASSERT(status == 0 || status == -1);
	if unlikely(status < 0)
		goto err;
	return_reference_(default_);
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
	if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
		DeeMH_map_remove_t map_remove = REQUIRE(map_remove);
		if (map_remove == &default__map_remove__empty)
			return &$empty;
		if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index)
			return &$with__seq_enumerate_index__and__seq_operator_delitem_index;
		if (map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
			return &$with__seq_enumerate__and__seq_operator_delitem;
	}
	if (map_operator_delitem &&
	    REQUIRE_ANY(map_operator_getitem) != &default__map_operator_getitem__unsupported)
		return &$with__map_operator_getitem__and__map_operator_delitem;;
};

map_pop_with_default = {
	DeeMH_map_operator_delitem_t map_operator_delitem = REQUIRE(map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &$empty;
	if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
		DeeMH_map_remove_t map_remove = REQUIRE(map_remove);
		if (map_remove == &default__map_remove__empty)
			return &$empty;
		if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index)
			return &$with__seq_enumerate_index__and__seq_operator_delitem_index;
		if (map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
			return &$with__seq_enumerate__and__seq_operator_delitem;
	}
	if (map_operator_delitem &&
	    REQUIRE_ANY(map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &$with__map_operator_trygetitem__and__map_operator_delitem;;
};
