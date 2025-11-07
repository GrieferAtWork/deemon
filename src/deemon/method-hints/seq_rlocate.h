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
/* deemon.Sequence.rlocate()                                            */
/************************************************************************/

[[kw, alias(Sequence.rlocate)]]
__seq_rlocate__(match:?DCallable, size_t start = 0, size_t end = (size_t)-1, def=!N)->?X2?O?Q!Adef] {
	if (start == 0 && end == (size_t)-1)
		return CALL_DEPENDENCY(seq_rlocate, self, match, def);
	return CALL_DEPENDENCY(seq_rlocate_with_range, self, match, start, end, def);
}





%[define(DEFINE_default_seq_rlocate_foreach_cb =
#ifndef DEFINED_default_seq_rlocate_foreach_cb
#define DEFINED_default_seq_rlocate_foreach_cb
struct default_seq_rlocate_foreach_data {
	DeeObject      *dsrlwf_match;  /* [1..1] Matching function. */
	DREF DeeObject *dsrlwf_result; /* [1..1] Match result. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq_rlocate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	struct default_seq_rlocate_foreach_data *data;
	data = (struct default_seq_rlocate_foreach_data *)arg;
	match_result_ob = DeeObject_Call(data->dsrlwf_match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		Dee_Decref(data->dsrlwf_result);
		data->dsrlwf_result = item;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_rlocate_foreach_cb */
)]




[[wunused]] DREF DeeObject *
__seq_rlocate__.seq_rlocate([[nonnull]] DeeObject *self,
                            [[nonnull]] DeeObject *match,
                            [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference_(def)}
%{$with__seq_foreach_reverse = [[prefix(DEFINE_seq_locate_foreach_cb)]] {
	Dee_ssize_t foreach_status = CALL_DEPENDENCY(seq_foreach_reverse, self, &seq_locate_foreach_cb, &match);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_rlocate_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct default_seq_rlocate_foreach_data data;
	data.dsrlwf_match  = match;
	data.dsrlwf_result = def;
	Dee_Incref(def);
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_rlocate_foreach_cb, &data);
	if likely(foreach_status == 0)
		return data.dsrlwf_result;
	Dee_Decref_unlikely(data.dsrlwf_result);
	return NULL;
}} {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return LOCAL_CALLATTR(self, 2, args);
}

seq_rlocate = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (REQUIRE(seq_foreach_reverse))
		return &$with__seq_foreach_reverse;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};






%[define(DEFINE_seq_rlocate_enumerate_index_cb =
DEFINE_default_seq_rlocate_foreach_cb
#ifndef DEFINED_seq_rlocate_enumerate_index_cb
#define DEFINED_seq_rlocate_enumerate_index_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_rlocate_enumerate_index_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return default_seq_rlocate_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_rlocate_enumerate_index_cb */
)]
[[wunused]] DREF DeeObject *
__seq_rlocate__.seq_rlocate_with_range([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *match,
                                       size_t start, size_t end,
                                       [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference_(def)}
%{$with__seq_enumerate_index_reverse = [[prefix(DEFINE_seq_locate_enumerate_index_cb)]] {
	Dee_ssize_t foreach_status = CALL_DEPENDENCY(seq_enumerate_index_reverse, self, &seq_locate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_rlocate_enumerate_index_cb)]] {
	Dee_ssize_t foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_rlocate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}} {
	return LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

seq_rlocate_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (REQUIRE(seq_enumerate_index_reverse))
		return &$with__seq_enumerate_index_reverse;
	seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

