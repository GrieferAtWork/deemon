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
/* deemon.Sequence.any()                                                */
/************************************************************************/
[[kw, alias(Sequence.any -> "seq_any")]]
__seq_any__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_any__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_any_with_key, self, key)
		         : CALL_DEPENDENCY(seq_any, self);
	} else {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_any_with_range_and_key, self, start, end, key)
		         : CALL_DEPENDENCY(seq_any_with_range, self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

%[define(DEFINE_seq_any_foreach_cb =
#ifndef DEFINED_seq_any_foreach_cb
#define DEFINED_seq_any_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp > 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_any_foreach_cb */
)]

[[wunused]]
int __seq_any__.seq_any([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_any_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_any_foreach_cb, NULL);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

%[define(DEFINE_seq_any_with_key_foreach_cb =
#ifndef DEFINED_seq_any_with_key_foreach_cb
#define DEFINED_seq_any_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_any_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp > 0)
		temp = -2;
	return temp;
err:
	return -1;
}
#endif /* !DEFINED_seq_any_with_key_foreach_cb */
)]

[[wunused]]
int __seq_any__.seq_any_with_key([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_any_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_any_with_key_foreach_cb, key);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}} {
	DREF DeeObject *result;
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	result = LOCAL_CALLATTR(self, 3, args);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

%[define(DEFINE_seq_any_enumerate_cb =
DEFINE_seq_any_foreach_cb
#ifndef DEFINED_seq_any_enumerate_cb
#define DEFINED_seq_any_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_any_enumerate_cb */
)]

[[wunused]]
int __seq_any__.seq_any_with_range([[nonnull]] DeeObject *__restrict self,
                                   size_t start, size_t end)
%{unsupported(auto)} %{$empty = 0}
%{$with__seqclass_map__and__seq_operator_bool__and__map_operator_size = {
	size_t map_size;
	if (start <= end)
		return 0;
	if (start == 0)
		return CALL_DEPENDENCY(seq_operator_bool, self);
	map_size = CALL_DEPENDENCY(map_operator_size, self);
	if unlikely(map_size == (size_t)-1)
		goto err;
	return start < map_size;
err:
	return -1;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_any_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_any_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}} {
	DREF DeeObject *result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

%[define(DEFINE_seq_any_with_key_enumerate_cb =
DEFINE_seq_any_with_key_foreach_cb
#ifndef DEFINED_seq_any_with_key_enumerate_cb
#define DEFINED_seq_any_with_key_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_any_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_any_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_any_with_key_enumerate_cb */
)]

[[wunused]]
int __seq_any__.seq_any_with_range_and_key([[nonnull]] DeeObject *self, size_t start, size_t end,
                                           [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_any_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_any_with_key_enumerate_cb, key, start, end);
	ASSERT(foreach_status == 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}} {
	DREF DeeObject *result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}


seq_any = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (SEQ_CLASS == Dee_SEQCLASS_MAP) {
		/* All sequence-like map items are "true" (because they
		 * are non-empty (2-element) tuples). As such, so-long as
		 * a mapping itself is non-empty, there will always exist
		 * an **item** (the 2-element tuple) that evaluations to
		 * true. */
		DeeMH_seq_operator_bool_t seq_operator_bool = REQUIRE(seq_operator_bool);
		if (seq_operator_bool)
			return seq_operator_bool;
	}
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_any_with_key = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_any_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (SEQ_CLASS == Dee_SEQCLASS_MAP) {
		/* All sequence-like map items are "true" (because they
		 * are non-empty (2-element) tuples). As such, so-long as
		 * a mapping itself is non-empty, there will always exist
		 * an **item** (the 2-element tuple) that evaluations to
		 * true. */
		if (REQUIRE(seq_operator_bool) || REQUIRE(map_operator_size))
			return &$with__seqclass_map__and__seq_operator_bool__and__map_operator_size;
	}
	seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_any_with_range_and_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

