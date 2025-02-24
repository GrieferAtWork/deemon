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
/* deemon.Sequence.extend()                                             */
/************************************************************************/
[[alias(Sequence.extend -> "seq_extend")]]
__seq_extend__(items:?S?O) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__seq_extend__", &items))
		goto err;
	if unlikely(CALL_DEPENDENCY(seq_extend, self, items))
		goto err;
	return_none;
err:
	return NULL;
}


%[define(DEFINE_seq_extend_with_foreach_append_cb =
#ifndef DEFINED_seq_extend_with_foreach_append_cb
#define DEFINED_seq_extend_with_foreach_append_cb
struct seq_extend_with_foreach_append_data {
	DeeMH_seq_append_t dsewfad_append; /* [1..1] Append callback */
	DeeObject         *dsewfad_self;   /* [1..1] The sequence to append to */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_extend_with_foreach_append_cb(void *arg, DeeObject *item) {
	struct seq_extend_with_foreach_append_data *data;
	data = (struct seq_extend_with_foreach_append_data *)arg;
	return (Dee_ssize_t)(*data->dsewfad_append)(data->dsewfad_self, item);
}
#endif /* !DEFINED_seq_extend_with_foreach_append_cb */
)]


[[wunused]]
int __seq_extend__.seq_extend([[nonnull]] DeeObject *self,
                              [[nonnull]] DeeObject *items)
%{unsupported(auto)}
%{$none = 0}
%{$empty = "default__seq_extend__unsupported"}
%{$with__seq_operator_size__and__seq_operator_setrange_index = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return CALL_DEPENDENCY(seq_operator_setrange_index, self, size, size, items);
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_insertall = {
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return CALL_DEPENDENCY(seq_insertall, self, size, items);
err:
	return -1;
}}
%{$with__seq_append = [[prefix(DEFINE_seq_extend_with_foreach_append_cb)]] {
	struct seq_extend_with_foreach_append_data data;
	data.dsewfad_append = REQUIRE_DEPENDENCY(seq_append);
	data.dsewfad_self   = self;
	return (int)CALL_DEPENDENCY(seq_operator_foreach, items, &seq_extend_with_foreach_append_cb, &data);
}} {
	DREF DeeObject *result;
	result = LOCAL_CALLATTR(self, 1, &items);
	if unlikely(!result)
		goto err;
	Dee_Decref(result);
	return 0;
err:
	return -1;
}

seq_extend = {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_insertall_t seq_insertall;
	if (REQUIRE_NODEFAULT(seq_append))
		return &$with__seq_append;
	seq_insertall = REQUIRE(seq_insertall);
	if (seq_insertall == &default__seq_insertall__empty)
		return &$empty;
	seq_operator_size = REQUIRE_ANY(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_insertall == &default__seq_insertall__with__seq_operator_setrange_index)
			return &$with__seq_operator_size__and__seq_operator_setrange_index;
		if (seq_insertall)
			return &$with__seq_operator_size__and__seq_insertall;
	}
};
