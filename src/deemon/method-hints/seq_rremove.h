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
/* deemon.Sequence.rremove()                                            */
/************************************************************************/
[[kw, alias(Sequence.rremove -> "seq_rremove")]]
__seq_rremove__(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_rremove__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? CALL_DEPENDENCY(seq_rremove_with_key, self, item, start, end, key)
	         : CALL_DEPENDENCY(seq_rremove, self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}





/* @return: 0 : Item was not removed
 * @return: 1 : Item was removed
 * @return: -1: Error */
[[wunused]] int
__seq_rremove__.seq_rremove([[nonnull]] DeeObject *self,
                            [[nonnull]] DeeObject *item,
                            size_t start, size_t end)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_enumerate_index_reverse__and__seq_operator_delitem_index =
[[prefix(DEFINE_default_remove_with_enumerate_index_and_delitem_index_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index_reverse, self,
	                                 &default_remove_with_enumerate_index_and_delitem_index_cb,
	                                 &data, start, end);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}}
%{$with__seq_rfind__and__seq_operator_delitem_index = {
	size_t index = CALL_DEPENDENCY(seq_rfind, self, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, index))
		goto err;
	return 1;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, item, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}


/* @return: 0 : Item was not removed
 * @return: 1 : Item was removed
 * @return: -1: Error */
[[wunused]] int
__seq_rremove__.seq_rremove_with_key([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *item,
                                     size_t start, size_t end,
                                     [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = 0}
%{$empty = 0}
%{$with__seq_enumerate_index_reverse__and__seq_operator_delitem_index =
[[prefix(DEFINE_default_remove_with_key_with_enumerate_index_and_delitem_index_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = DeeObject_Call(key, 1, &item);
	if unlikely(!data.drwkweiadiid_item)
		goto err;
	data.drwkweiadiid_key = key;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index_reverse, self,
	                                 &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
	                                 &data, start, end);
	Dee_Decref(data.drwkweiadiid_item);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == 0)
		return 0; /* Not found */
	return 1;     /* Found and removed */
err:
	return -1;
}}
%{$with__seq_rfind_with_key__and__seq_operator_delitem_index = {
	size_t index = CALL_DEPENDENCY(seq_rfind_with_key, self, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely(CALL_DEPENDENCY(seq_operator_delitem_index, self, index))
		goto err;
	return 1;
err:
	return -1;
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", item, start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_rremove = {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_rfind_t seq_rfind;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		seq_rfind = REQUIRE_ANY(seq_rfind);
		if (seq_rfind != &default__seq_rfind__unsupported) {
			if (seq_rfind == &default__seq_rfind__with__seq_enumerate_index_reverse)
				return &$with__seq_enumerate_index_reverse__and__seq_operator_delitem_index;
			return &$with__seq_rfind__and__seq_operator_delitem_index;
		}
	}
};

seq_rremove_with_key = {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_rfind_with_key_t seq_rfind_with_key;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		seq_rfind_with_key = REQUIRE_ANY(seq_rfind_with_key);
		if (seq_rfind_with_key != &default__seq_rfind_with_key__unsupported) {
			if (seq_rfind_with_key == &default__seq_rfind_with_key__with__seq_enumerate_index_reverse)
				return &$with__seq_enumerate_index_reverse__and__seq_operator_delitem_index;
			return &$with__seq_rfind_with_key__and__seq_operator_delitem_index;
		}
	}
};
