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
/* deemon.Sequence.locate()                                             */
/************************************************************************/

[[kw, alias(Sequence.locate -> "seq_locate"), declNameAlias("explicit_seq_locate")]]
__seq_locate__(match,start=!0,end:?Dint=!A!Dint!PSIZE_MAX,def=!N)->?X2?O?Q!Adef] {
	DeeObject *match, *def = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__match_start_end_def,
	                    "o|" UNPuSIZ UNPuSIZ "o:__seq_locate__",
	                    &match, &start, &end, &def))
		goto err;
	if (start == 0 && end == (size_t)-1)
		return CALL_DEPENDENCY(seq_locate, self, match, def);
	return CALL_DEPENDENCY(seq_locate_with_range, self, match, start, end, def);
err:
	return NULL;
}





%[define(DEFINE_seq_locate_foreach_cb =
#ifndef DEFINED_seq_locate_foreach_cb
#define DEFINED_seq_locate_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_locate_foreach_cb(void *arg, DeeObject *item) {
	int match_result;
	DREF DeeObject *match_result_ob;
	DeeObject *match = *(DeeObject **)arg;
	match_result_ob = DeeObject_Call(match, 1, &item);
	if unlikely(!match_result_ob)
		goto err;
	match_result = DeeObject_BoolInherited(match_result_ob);
	if unlikely(match_result < 0)
		goto err;
	if (match_result) {
		Dee_Incref(item);
		*(DeeObject **)arg = item;
		return -2;
	}
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_locate_foreach_cb */
)]




[[wunused]] DREF DeeObject *
__seq_locate__.seq_locate([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *match,
                          [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$empty = return_reference_(def)}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_locate_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_locate_foreach_cb, &match);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}} {
	DeeObject *args[2];
	args[0] = match;
	args[1] = def;
	return LOCAL_CALLATTR(self, 2, args);
}

seq_locate = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};






%[define(DEFINE_seq_locate_enumerate_index_cb =
DEFINE_seq_locate_foreach_cb
#ifndef DEFINED_seq_locate_enumerate_index_cb
#define DEFINED_seq_locate_enumerate_index_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_locate_enumerate_index_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_locate_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_locate_enumerate_index_cb */
)]
[[wunused]] DREF DeeObject *
__seq_locate__.seq_locate_with_range([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *match,
                                     size_t start, size_t end,
                                     [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$empty = return_reference_(def)}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_locate_enumerate_index_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_locate_enumerate_index_cb, &match, start, end);
	if likely(foreach_status == -2)
		return match;
	return_reference_(def);
}} {
	return LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", match, start, end, def);
}

seq_locate_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

