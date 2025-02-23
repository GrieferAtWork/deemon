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
/* deemon.Sequence.sum()                                                */
/************************************************************************/
[[kw, alias(Sequence.sum -> "seq_sum")]]
__seq_sum__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX)->?O {
	DREF DeeObject *result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    "|" UNPuSIZ UNPuSIZ ":__seq_sum__",
	                    &start, &end))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = CALL_DEPENDENCY(seq_sum, self);
	} else {
		result = CALL_DEPENDENCY(seq_sum_with_range, self, start, end);
	}
	return result;
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_sum__.seq_sum([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = {
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &Dee_accu_add, &accu);
	if unlikely(foreach_status < 0)
		goto err;
	return Dee_accu_pack(&accu);
err:
	Dee_accu_fini(&accu);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}



%[define(DEFINE_seq_sum_enumerate_cb =
#ifndef DEFINED_seq_sum_enumerate_cb
#define DEFINED_seq_sum_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_sum_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return Dee_accu_add(arg, item);
}
#endif /* !DEFINED_seq_sum_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_sum__.seq_sum_with_range([[nonnull]] DeeObject *__restrict self,
                               size_t start, size_t end)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_sum_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_sum_enumerate_cb, &accu, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	return Dee_accu_pack(&accu);
err:
	Dee_accu_fini(&accu);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

seq_sum = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_sum_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

