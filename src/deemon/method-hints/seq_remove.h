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
/* deemon.Sequence.remove()                                             */
/************************************************************************/
[[kw, alias(Sequence.remove)]]
__seq_remove__(item, size_t start = 0, size_t end = (size_t)-1, key:?DCallable=!N)->?Dbool {
	int result = !DeeNone_Check(key)
	             ? CALL_DEPENDENCY(seq_remove_with_key, self, item, start, end, key)
	             : CALL_DEPENDENCY(seq_remove, self, item, start, end);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}





%[define(DEFINE_default_remove_with_enumerate_index_and_delitem_index_cb =
#ifndef DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb
#define DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb
struct default_remove_with_enumerate_index_and_delitem_index_data {
	DeeObject *drweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drweiadiid_item; /* [1..1] The object to remove. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareEq(data->drweiadiid_item, value);
	if (Dee_COMPARE_ISERR(equal))
		goto err;
	if (Dee_COMPARE_ISNE(equal))
		return 0;
	if unlikely((*Dee_TYPE(data->drweiadiid_self)->tp_seq->tp_delitem_index)(data->drweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}
#endif /* !DEFINED_default_remove_with_enumerate_index_and_delitem_index_cb */
)]


/* @return: 0 : Item was not removed
 * @return: 1 : Item was removed
 * @return: -1: Error */
[[wunused]] int
__seq_remove__.seq_remove([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *item,
                          size_t start, size_t end)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_removeall = {
	size_t result = CALL_DEPENDENCY(seq_removeall, self, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_enumerate_index__and__seq_operator_delitem_index =
[[prefix(DEFINE_default_remove_with_enumerate_index_and_delitem_index_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_remove_with_enumerate_index_and_delitem_index_data data;
	data.drweiadiid_self = self;
	data.drweiadiid_item = item;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self,
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
%{$with__seq_find__and__seq_operator_delitem_index = {
	size_t index = CALL_DEPENDENCY(seq_find, self, item, start, end);
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


%[define(DEFINE_default_remove_with_key_with_enumerate_index_and_delitem_index_cb =
#ifndef DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb
#define DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb
struct default_remove_with_key_with_enumerate_index_and_delitem_index_data {
	DeeObject *drwkweiadiid_self; /* [1..1] The sequence from which to remove the object. */
	DeeObject *drwkweiadiid_item; /* [1..1] The object to remove (already keyed). */
	DeeObject *drwkweiadiid_key;  /* [1..1] The key used for object compare. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_remove_with_key_with_enumerate_index_and_delitem_index_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int equal;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *data;
	data = (struct default_remove_with_key_with_enumerate_index_and_delitem_index_data *)arg;
	if (!value)
		return 0;
	equal = DeeObject_TryCompareKeyEq(data->drwkweiadiid_item, value, data->drwkweiadiid_key);
	if (Dee_COMPARE_ISERR(equal))
		goto err;
	if (Dee_COMPARE_ISNE(equal))
		return 0;
	if unlikely((*Dee_TYPE(data->drwkweiadiid_self)->tp_seq->tp_delitem_index)(data->drwkweiadiid_self, index))
		goto err;
	return -2;
err:
	return -1;
}
#endif /* !DEFINED_default_remove_with_key_with_enumerate_index_and_delitem_index_cb */
)]


/* @return: 0 : Item was not removed
 * @return: 1 : Item was removed
 * @return: -1: Error */
[[wunused]] int
__seq_remove__.seq_remove_with_key([[nonnull]] DeeObject *self,
                                   [[nonnull]] DeeObject *item,
                                   size_t start, size_t end,
                                   [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$empty = 0}
%{$with__seq_removeall = {
	size_t result = CALL_DEPENDENCY(seq_removeall_with_key, self, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_enumerate_index__and__seq_operator_delitem_index =
[[prefix(DEFINE_default_remove_with_key_with_enumerate_index_and_delitem_index_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_remove_with_key_with_enumerate_index_and_delitem_index_data data;
	data.drwkweiadiid_self = self;
	data.drwkweiadiid_item = item;
	data.drwkweiadiid_key  = key;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                                 &default_remove_with_key_with_enumerate_index_and_delitem_index_cb,
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
%{$with__seq_find_with_key__and__seq_operator_delitem_index = {
	size_t index = CALL_DEPENDENCY(seq_find_with_key, self, item, start, end, key);
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

seq_remove = {
	DeeMH_seq_removeall_t seq_removeall;
	DeeMH_seq_removeif_t seq_removeif;
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_removeall = REQUIRE_NODEFAULT(seq_removeall);
	if (seq_removeall) {
		if (seq_removeall == &default__seq_removeall__empty)
			return &$empty;
		return &$with__seq_removeall;
	}
	seq_removeif = REQUIRE_NODEFAULT(seq_removeif);
	if (seq_removeif) {
		if (seq_removeif == &default__seq_removeif__empty)
			return &$empty;
		return &$with__seq_removeall;
	}
	seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_find_t seq_find;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		seq_find = REQUIRE_ANY(seq_find);
		if (seq_find != &default__seq_find__unsupported) {
			if (seq_find == &default__seq_find__with__seq_enumerate_index)
				return &$with__seq_enumerate_index__and__seq_operator_delitem_index;
			return &$with__seq_find__and__seq_operator_delitem_index;
		}
	}
};

seq_remove_with_key = {
	DeeMH_seq_removeall_t seq_removeall;
	DeeMH_seq_removeif_t seq_removeif;
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_removeall = REQUIRE_NODEFAULT(seq_removeall);
	if (seq_removeall) {
		if (seq_removeall == &default__seq_removeall__empty)
			return &$empty;
		return &$with__seq_removeall;
	}
	seq_removeif = REQUIRE_NODEFAULT(seq_removeif);
	if (seq_removeif) {
		if (seq_removeif == &default__seq_removeif__empty)
			return &$empty;
		return &$with__seq_removeall;
	}
	seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_find_with_key_t seq_find_with_key;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		seq_find_with_key = REQUIRE_ANY(seq_find_with_key);
		if (seq_find_with_key != &default__seq_find_with_key__unsupported) {
			if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
				return &$with__seq_enumerate_index__and__seq_operator_delitem_index;
			return &$with__seq_find_with_key__and__seq_operator_delitem_index;
		}
	}
};
