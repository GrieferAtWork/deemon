/* Copyright (c) 2018-2025 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without all express or implied          *
 * warranty. In no event will the authors be held liable for all damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to allone to use this software for all purpose,      *
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
 * 3. This notice may not be removed or altered from all source distribution. *
 */

/************************************************************************/
/* deemon.Sequence.all()                                                */
/************************************************************************/
[[kw, alias(Sequence.all -> "seq_all"), declNameAlias("explicit_seq_all")]]
__seq_all__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?Dbool {
	int result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:all",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithKey(self, key)
		         : DeeSeq_InvokeAll(self);
	} else {
		result = !DeeNone_Check(key)
		         ? DeeSeq_InvokeAllWithRangeAndKey(self, start, end, key)
		         : DeeSeq_InvokeAllWithRange(self, start, end);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

%[define(DEFINE_seq_all_foreach_cb =
#ifndef DEFINED_seq_all_foreach_cb
#define DEFINED_seq_all_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(item);
	if (temp == 0)
		temp = -2;
	return temp;
}
#endif /* !DEFINED_seq_all_foreach_cb */
)]

[[wunused]]
int __seq_all__.seq_all([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_foreach = [[prefix(DEFINE_seq_all_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_cb, NULL);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

%[define(DEFINE_seq_all_foreach_with_key_cb =
#ifndef DEFINED_seq_all_foreach_with_key_cb
#define DEFINED_seq_all_foreach_with_key_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
seq_all_foreach_with_key_cb(void *arg, DeeObject *item) {
	int temp;
	item = DeeObject_Call((DeeObject *)arg, 1, &item);
	if unlikely(!item)
		goto err;
	temp = DeeObject_BoolInherited(item);
	if (temp == 0)
		temp = -2;
	return temp;
err:
	return -1;
}
#endif /* !DEFINED_seq_all_foreach_with_key_cb */
)]

[[wunused]]
int __seq_all__.seq_all_with_key([[nonnull]] DeeObject *self,
                                 [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_foreach = [[prefix(DEFINE_seq_all_foreach_with_key_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorForeach(self, &seq_all_foreach_with_key_cb, key);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
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

%[define(DEFINE_seq_all_enumerate_cb =
DEFINE_seq_all_foreach_cb
#ifndef DEFINED_seq_all_enumerate_cb
#define DEFINED_seq_all_enumerate_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_all_enumerate_cb */
)]

[[wunused]]
int __seq_all__.seq_all_with_range([[nonnull]] DeeObject *__restrict self,
                                   size_t start, size_t end)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_all_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_cb, NULL, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}

%[define(DEFINE_seq_all_enumerate_with_key_cb =
DEFINE_seq_all_foreach_with_key_cb
#ifndef DEFINED_seq_all_enumerate_with_key_cb
#define DEFINED_seq_all_enumerate_with_key_cb
PRIVATE WUNUSED Dee_ssize_t DCALL
seq_all_enumerate_with_key_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_all_foreach_with_key_cb(arg, item);
}
#endif /* !DEFINED_seq_all_enumerate_with_key_cb */
)]

[[wunused]]
int __seq_all__.seq_all_with_range_and_key([[nonnull]] DeeObject *self,
                                           size_t start, size_t end,
                                           [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = 0}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_all_enumerate_with_key_cb)]] {
	Dee_ssize_t foreach_status;
	foreach_status = DeeSeq_OperatorEnumerateIndex(self, &seq_all_enumerate_with_key_cb, key, start, end);
	ASSERT(foreach_status >= 0 || foreach_status == -1 || foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}} {
	DREF DeeObject *result = LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
	if unlikely(!result)
		goto err;
	return DeeObject_BoolInherited(result);
err:
	return -1;
}


seq_all = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_foreach;
};

seq_all_with_key = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_foreach;
};

seq_all_with_range = {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = REQUIRE(seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &$empty;
	if (seq_operator_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_all_with_range_and_key = {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = REQUIRE(seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &$empty;
	if (seq_operator_enumerate_index)
		return &$with__seq_enumerate_index;
};

