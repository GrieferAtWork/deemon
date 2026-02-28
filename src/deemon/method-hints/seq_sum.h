/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

/************************************************************************/
/* deemon.Sequence.sum()                                                */
/************************************************************************/
[[kw, alias(Sequence.sum)]]
__seq_sum__(size_t start = 0, size_t end = (size_t)-1, def=!N, key:?DCallable=!N)->?O {
	DREF DeeObject *result;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_sum_with_key, self, def, key)
		         : CALL_DEPENDENCY(seq_sum, self, def);
	} else {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_sum_with_range_and_key, self, start, end, def, key)
		         : CALL_DEPENDENCY(seq_sum_with_range, self, start, end, def);
	}
	return result;
err:
	return NULL;
}

[[wunused]] DREF DeeObject *
__seq_sum__.seq_sum([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference(def)}
%{$with__seq_operator_foreach = {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &Dee_accu_add, &accu);
	if unlikely(foreach_status < 0)
		goto err;
	result = Dee_accu_pack(&accu);
	if unlikely(result == ITER_DONE)
		return_reference(def);
	return result;
err:
	Dee_accu_fini(&accu);
	return NULL;
}} {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = Dee_AsObject(&Dee_int_SIZE_MAX);
	args[2] = def;
	return LOCAL_CALLATTR(self, 3, args);
}



%[define(DEFINE_seq_sum_with_key_foreach_cb =
#ifndef DEFINED_seq_sum_with_key_foreach_cb
#define DEFINED_seq_sum_with_key_foreach_cb
struct seq_sum_with_key_foreach_data {
	struct Dee_accu sswkfd_accu; /* Accumulator */
	DeeObject      *sswkfd_key;  /* [1..1] Key function */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_sum_with_key_foreach_cb(void *arg, DeeObject *item) {
	struct seq_sum_with_key_foreach_data *data;
	data = (struct seq_sum_with_key_foreach_data *)arg;
	item = DeeObject_Call(data->sswkfd_key, 1, &item);
	if unlikely(!item)
		goto err;
	if (!item)
		return 0;
	return Dee_accu_add_inherited(&data->sswkfd_accu, item);
err:
	return -1;
}
#endif /* !DEFINED_seq_sum_with_key_foreach_cb */
)]


[[wunused]] DREF DeeObject *
__seq_sum__.seq_sum_with_key([[nonnull]] DeeObject *self,
                             [[nonnull]] DeeObject *def,
                             [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference(def)}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_sum_with_key_foreach_cb)]] {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	struct seq_sum_with_key_foreach_data data;
	Dee_accu_init(&data.sswkfd_accu);
	data.sswkfd_key = key;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_sum_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	result = Dee_accu_pack(&data.sswkfd_accu);
	if unlikely(result == ITER_DONE)
		return_reference(def);
	return result;
err:
	Dee_accu_fini(&data.sswkfd_accu);
	return NULL;
}} {
	DeeObject *args[4];
	args[0] = DeeInt_Zero;
	args[1] = Dee_AsObject(&Dee_int_SIZE_MAX);
	args[2] = def;
	args[3] = key;
	return LOCAL_CALLATTR(self, 4, args);
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
__seq_sum__.seq_sum_with_range([[nonnull]] DeeObject *self,
                               size_t start, size_t end,
                               [[nonnull]] DeeObject *def)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference(def)}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_sum_enumerate_cb)]] {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	struct Dee_accu accu;
	Dee_accu_init(&accu);
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_sum_enumerate_cb, &accu, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	result = Dee_accu_pack(&accu);
	if unlikely(result == ITER_DONE)
		return_reference(def);
	return result;
err:
	Dee_accu_fini(&accu);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, def);
}



%[define(DEFINE_seq_sum_with_key_enumerate_cb =
DEFINE_seq_sum_with_key_foreach_cb
#ifndef DEFINED_seq_sum_with_key_enumerate_cb
#define DEFINED_seq_sum_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_sum_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	return item ? seq_sum_with_key_foreach_cb(arg, item) : 0;
}
#endif /* !DEFINED_seq_sum_with_key_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_sum__.seq_sum_with_range_and_key([[nonnull]] DeeObject *self,
                                       size_t start, size_t end,
                                       [[nonnull]] DeeObject *def,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = return_reference(def)}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_sum_with_key_enumerate_cb)]] {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	struct seq_sum_with_key_foreach_data data;
	Dee_accu_init(&data.sswkfd_accu);
	data.sswkfd_key = key;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_sum_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err;
	result = Dee_accu_pack(&data.sswkfd_accu);
	if unlikely(result == ITER_DONE)
		return_reference(def);
	return result;
err:
	Dee_accu_fini(&data.sswkfd_accu);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "oo", start, end, def, key);
}




seq_sum = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_sum_with_key = {
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

seq_sum_with_range_and_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

