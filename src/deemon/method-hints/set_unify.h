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
/* deemon.Set.unify()                                                   */
/************************************************************************/
[[alias(Set.unify)]]
__set_unify__(key)->?O {
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__set_unify__", &key))
		goto err;
	return CALL_DEPENDENCY(set_unify, self, key);
err:
	return NULL;
}

%[define(DEFINE_set_unify_foreach_cb =
#ifndef DEFINED_set_unify_foreach_cb
#define DEFINED_set_unify_foreach_cb
struct set_unify_foreach_data {
	DeeObject      *sufd_key;    /* [1..1] Key to find */
	DREF DeeObject *sufd_result; /* [?..1] Matching duplicate */
};

#define SET_UNIFY_FOREACH_FOUND ((Dee_ssize_t)-2)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
set_unify_foreach_cb(void *arg, DeeObject *key) {
	int temp;
	struct set_unify_foreach_data *data;
	data = (struct set_unify_foreach_data *)arg;
	temp = DeeObject_TryCompareEq(data->sufd_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(key);
		data->sufd_result = key;
		return SET_UNIFY_FOREACH_FOUND;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_set_unify_foreach_cb */
)]



%[define(DEFINE_seq_locate_item_foreach_cb =
#ifndef DEFINED_seq_locate_item_foreach_cb
#define DEFINED_seq_locate_item_foreach_cb
#define SET_LOCATE_ITEM_FOREACH_FOUND ((Dee_ssize_t)-2)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_item_foreach_cb(void *arg, DeeObject *key) {
	int temp;
	DeeObject *search_key = *(DeeObject **)arg;
	temp = DeeObject_TryCompareEq(search_key, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(key);
		*(DeeObject **)arg = key;
		return SET_LOCATE_ITEM_FOREACH_FOUND;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_item_foreach_cb */
)]



/* Insert `key' if not already present and re-return `key'.
 * If already present, return the pre-existing (and equal) instance instead.
 * @return: NULL: Error */
[[wunused]] DREF DeeObject *
__set_unify__.set_unify([[nonnull]] DeeObject *self,
                        [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = {
	return_reference_(key);
}}
%{$empty = "default__set_unify__unsupported"}
%{$with__seq_operator_foreach__and__set_insert = [[prefix(DEFINE_set_unify_foreach_cb)]] {
	int status = CALL_DEPENDENCY(set_insert, self, key);
	if unlikely(status < 0)
		goto err;
	if (!status) {
		struct set_unify_foreach_data data;
		Dee_ssize_t fe_status;
		data.sufd_key = key;
		DBG_memset(&data.sufd_result, 0xcc, sizeof(data.sufd_result));
		fe_status = CALL_DEPENDENCY(seq_operator_foreach, self, &set_unify_foreach_cb, &data);
		if likely(fe_status == SET_UNIFY_FOREACH_FOUND) {
			ASSERT_OBJECT(data.sufd_result);
			return data.sufd_result; /* Inherit reference */
		}
		if unlikely(fe_status < 0)
			goto err;
	}
	return_reference_(key);
err:
	return NULL;
}}
%{$with__seq_operator_foreach__and__seq_append = [[prefix(DEFINE_seq_locate_item_foreach_cb)]] {
	Dee_ssize_t status;
	Dee_Incref(key);
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_locate_item_foreach_cb, &key);
	if (status == SET_LOCATE_ITEM_FOREACH_FOUND)
		return key;
	if (status < 0)
		goto err_key_nokill;
	if (CALL_DEPENDENCY(seq_append, self, key))
		goto err_key_nokill;
	return key;
err_key_nokill:
	Dee_DecrefNokill(key); /* *Nokill because caller still has a reference */
/*err:*/
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &key);
}

set_unify = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach) {
		DeeMH_set_insert_t set_insert = REQUIRE(set_insert);
		if (set_insert == &default__set_insert__empty)
			return &$empty;
		if (set_insert == &default__set_insert__with__seq_contains__and__seq_append)
			return &$with__seq_operator_foreach__and__seq_append;
		if (set_insert)
			return &$with__seq_operator_foreach__and__set_insert;
		if (REQUIRE(seq_append))
			return &$with__seq_operator_foreach__and__seq_append;
	}
};
