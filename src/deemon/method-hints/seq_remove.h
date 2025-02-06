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
/* deemon.Sequence.remove()                                             */
/************************************************************************/
[[kw, alias(Sequence.remove -> "seq_remove"), declNameAlias("explicit_seq_remove")]]
__seq_remove__(item,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool {
	int result;
	DeeObject *item, *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__item_start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_remove__",
	                    &item, &start, &end, &key))
		goto err;
	result = !DeeNone_Check(key)
	         ? DeeType_InvokeMethodHint(self, seq_remove_with_key, item, start, end, key)
	         : DeeType_InvokeMethodHint(self, seq_remove, item, start, end);
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
__seq_remove__.seq_remove([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *item,
                          size_t start, size_t end)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_removeall = {
	size_t result;
	result = DeeType_InvokeMethodHint(self, seq_removeall, item, start, end, 1);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_removeif = {
	// TODO
}}
%{$with__seq_find__and__seq_operator_delitem_index = {
	size_t index = DeeType_InvokeMethodHint(self, seq_find, item, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely(DeeType_InvokeMethodHint(self, seq_operator_delitem_index, index))
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
[[wunused]] size_t
__seq_remove__.seq_remove_with_key([[nonnull]] DeeObject *self,
                                   [[nonnull]] DeeObject *item,
                                   size_t start, size_t end,
                                   [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_removeall = {
	size_t result;
	result = DeeType_InvokeMethodHint(self, seq_removeall_with_key, item, start, end, 1, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return result ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_removeif = {
	// TODO
}}
%{$with__seq_find_with_key__and__seq_operator_delitem_index = {
	size_t index = DeeType_InvokeMethodHint(self, seq_find_with_key, item, start, end, key);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	if (index == (size_t)-1)
		return 0;
	if unlikely(DeeType_InvokeMethodHint(self, seq_operator_delitem_index, index))
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
		return &$with__seq_removeif;
	}
	seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		if (REQUIRE_ANY(seq_find) != &default__seq_find__unsupported)
			return &$with__seq_find__and__seq_operator_delitem_index;
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
		return &$with__seq_removeif;
	}
	seq_operator_delitem_index = REQUIRE_NODEFAULT(seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &$empty;
		if (REQUIRE_ANY(seq_find_with_key) != &default__seq_find_with_key__unsupported)
			return &$with__seq_find_with_key__and__seq_operator_delitem_index;
	}
};
