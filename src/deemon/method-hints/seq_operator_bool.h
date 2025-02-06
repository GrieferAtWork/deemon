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
/* deemon.Sequence.operator bool()                                      */
/************************************************************************/
__seq_bool__()->?Dbool {
	int result;
	if (DeeArg_Unpack(argc, argv, ":__seq_bool__"))
		goto err;
	result = DeeType_InvokeMethodHint0(self, seq_operator_bool);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_bool_with_foreach_cb =
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_bool_with_foreach_cb(void *arg, DeeObject *elem);
)]

[[wunused]]
[[operator([Sequence, Set, Mapping].OPERATOR_BOOL: tp_cast.tp_bool)]]
int __seq_bool__.seq_operator_bool([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator bool"))} %{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_bool_with_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = DeeType_InvokeMethodHint(self, seq_operator_foreach, &default_seq_bool_with_foreach_cb, NULL);
	ASSERT(foreach_status == -2 || foreach_status == -1 || foreach_status == 0);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
}}
%{$with__seq_operator_size = {
	size_t size = DeeType_InvokeMethodHint0(self, seq_operator_size);
	if unlikely(size == (size_t)-1)
		goto err;
	return size != 0;
err:
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

seq_operator_bool = {
	DeeMH_seq_operator_size_t seq_operator_size;
	/* TODO: $with__seq_operator_compare_eq */
	/* TODO: $with__seq_operator_eq */
	/* TODO: $with__seq_operator_ne */
	seq_operator_size = REQUIRE(seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_size)
		return &$with__seq_operator_size;
};
