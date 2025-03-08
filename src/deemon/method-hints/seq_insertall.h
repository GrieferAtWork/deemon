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
/* deemon.Sequence.insertall()                                          */
/************************************************************************/
[[kw, alias(Sequence.insertall)]]
__seq_insertall__(index:?Dint,items:?S?O) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__index_items,
	                    UNPuSIZ "o:__seq_insertall__",
	                    &index, &items))
		goto err;
	if unlikely(CALL_DEPENDENCY(seq_insertall, self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}


%[define(DEFINE_seq_insertall_with_foreach_insert_cb =
#ifndef DEFINED_seq_insertall_with_foreach_insert_cb
#define DEFINED_seq_insertall_with_foreach_insert_cb
struct seq_insertall_with_foreach_insert_data {
	DeeMH_seq_insert_t dsiawfid_insert; /* [1..1] Insert callback */
	DeeObject         *dsiawfid_self;   /* [1..1] The sequence to insert into */
	size_t             dsiawfid_index;  /* Next index for insertion */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_insertall_with_foreach_insert_cb(void *arg, DeeObject *item) {
	struct seq_insertall_with_foreach_insert_data *data;
	data = (struct seq_insertall_with_foreach_insert_data *)arg;
	return (Dee_ssize_t)(*data->dsiawfid_insert)(data->dsiawfid_self, data->dsiawfid_index++, item);
}
#endif /* !DEFINED_seq_insertall_with_foreach_insert_cb */
)]


[[wunused]]
int __seq_insertall__.seq_insertall([[nonnull]] DeeObject *self, size_t index,
                                    [[nonnull]] DeeObject *items)
%{unsupported(auto)}
%{$none = 0}
%{$empty = {
	int items_empty = CALL_DEPENDENCY(seq_operator_bool, items);
	if unlikely(items_empty < 0)
		goto err;
	if (items_empty)
		return 0;
	return default__seq_insertall__unsupported(self, index, items);
err:
	return -1;
}}
%{$with__seq_operator_setrange_index = {
	return CALL_DEPENDENCY(seq_operator_setrange_index, self, (Dee_ssize_t)index, (Dee_ssize_t)index, items);
}}
%{$with__seq_insert = [[prefix(DEFINE_seq_insertall_with_foreach_insert_cb)]] {
	struct seq_insertall_with_foreach_insert_data data;
	data.dsiawfid_insert = REQUIRE_DEPENDENCY(seq_insert);
	data.dsiawfid_self   = self;
	data.dsiawfid_index  = index;
	return (int)CALL_DEPENDENCY(seq_operator_foreach, items, &seq_insertall_with_foreach_insert_cb, &data);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTRF(self, PCKuSIZ "o", index, items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_insertall = {
	if (REQUIRE_NODEFAULT(seq_operator_setrange_index))
		return &$with__seq_operator_setrange_index;
	if (REQUIRE_NODEFAULT(seq_insert))
		return &$with__seq_insert;
	if (REQUIRE(seq_operator_foreach) == &default__seq_operator_foreach__empty)
		return &$empty;
};
