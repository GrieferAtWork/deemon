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
/* deemon.Sequence.reduce()                                             */
/************************************************************************/
[[kw, alias(Sequence.reduce)]]
__seq_reduce__(combine:?DCallable, size_t start = 0, size_t end = (size_t)-1, init?)->?O {
	if (start == 0 && end == (size_t)-1) {
		if (init)
			return CALL_DEPENDENCY(seq_reduce_with_init, self, combine, init);
		return CALL_DEPENDENCY(seq_reduce, self, combine);
	}
	if (init)
		return CALL_DEPENDENCY(seq_reduce_with_range_and_init, self, combine, start, end, init);
	return CALL_DEPENDENCY(seq_reduce_with_range, self, combine, start, end);
err:
	return NULL;
}



%[define(DEFINE_seq_reduce_data =
#ifndef DEFINED_seq_reduce_data
#define DEFINED_seq_reduce_data
struct seq_reduce_data {
	DeeObject      *gsr_combine; /* [1..1] Combinatory predicate (invoke as `gsr_combine(gsr_init, item)') */
	DREF DeeObject *gsr_result;  /* [0..1] Current reduction result, or NULL if no init given and at first item. */
};
#endif /* !DEFINED_seq_reduce_data */
)]

%[define(DEFINE_seq_reduce_foreach_with_init_cb =
#ifndef DEFINED_seq_reduce_foreach_with_init_cb
#define DEFINED_seq_reduce_foreach_with_init_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_reduce_foreach_with_init_cb(void *arg, DeeObject *item) {
	DeeObject *args[2];
	DREF DeeObject *reduced;
	struct seq_reduce_data *data;
	data    = (struct seq_reduce_data *)arg;
	args[0] = data->gsr_result;
	args[1] = item;
	reduced = DeeObject_Call(data->gsr_combine, 2, args);
	if unlikely(!reduced)
		goto err;
	Dee_Decref(data->gsr_result);
	data->gsr_result = reduced;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_seq_reduce_foreach_with_init_cb */
)]

%[define(DEFINE_seq_reduce_foreach_cb =
#ifndef DEFINED_seq_reduce_foreach_cb
#define DEFINED_seq_reduce_foreach_cb
DEFINE_seq_reduce_foreach_with_init_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_reduce_foreach_cb(void *arg, DeeObject *item) {
	struct seq_reduce_data *data;
	data = (struct seq_reduce_data *)arg;
	if (data->gsr_result)
		return seq_reduce_foreach_with_init_cb(arg, item);
	data->gsr_result = item;
	Dee_Incref(item);
	return 0;
}
#endif /* !DEFINED_seq_reduce_foreach_cb */
)]

%[define(DEFINE_seq_reduce_enumerate_with_init_cb =
#ifndef DEFINED_seq_reduce_enumerate_with_init_cb
#define DEFINED_seq_reduce_enumerate_with_init_cb
DEFINE_seq_reduce_foreach_with_init_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_with_init_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_with_init_cb(arg, item);
}
#endif /* !DEFINED_seq_reduce_enumerate_with_init_cb */
)]

%[define(DEFINE_seq_reduce_enumerate_cb =
#ifndef DEFINED_seq_reduce_enumerate_cb
#define DEFINED_seq_reduce_enumerate_cb
DEFINE_seq_reduce_foreach_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_reduce_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	if unlikely(!item)
		return 0;
	(void)index;
	return seq_reduce_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_reduce_enumerate_cb */
)]



[[wunused]] DREF DeeObject *
__seq_reduce__.seq_reduce([[nonnull]] DeeObject *self,
                          [[nonnull]] DeeObject *combine)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = {
	(void)combine;
	DeeRT_ErrEmptySequence(self);
	return NULL;
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_seq_reduce_data), prefix(DEFINE_seq_reduce_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_reduce_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		DeeRT_ErrEmptySequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &combine);
}

[[wunused]] DREF DeeObject *
__seq_reduce__.seq_reduce_with_init([[nonnull]] DeeObject *self,
                                    [[nonnull]] DeeObject *combine,
                                    [[nonnull]] DeeObject *init)
%{unsupported(auto)}
%{$empty = {
	(void)self;
	(void)combine;
	return_reference_(init);
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_seq_reduce_data), prefix(DEFINE_seq_reduce_foreach_with_init_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_reduce_foreach_with_init_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_Decref(data.gsr_result);
	return NULL;
}} {
	DeeObject *args[4];
	args[0] = combine;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[3] = init;
	return LOCAL_CALLATTR(self, 4, args);
}

[[wunused]] DREF DeeObject *
__seq_reduce__.seq_reduce_with_range([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *combine,
                                     size_t start, size_t end)
%{unsupported(auto)}
%{$none = return_none}
%{$empty = {
	(void)combine;
	(void)start;
	(void)end;
	DeeRT_ErrEmptySequence(self);
	return NULL;
}}
%{$with__seq_enumerate_index =
[[prefix(DEFINE_seq_reduce_data), prefix(DEFINE_seq_reduce_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = NULL;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_reduce_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		DeeRT_ErrEmptySequence(self);
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, combine, start, end);
}

[[wunused]] DREF DeeObject *
__seq_reduce__.seq_reduce_with_range_and_init([[nonnull]] DeeObject *self,
                                              [[nonnull]] DeeObject *combine,
                                              size_t start, size_t end,
                                              [[nonnull]] DeeObject *init)
%{unsupported(auto)}
%{$empty = {
	(void)self;
	(void)combine;
	(void)start;
	(void)end;
	return_reference_(init);
}}
%{$with__seq_enumerate_index =
[[prefix(DEFINE_seq_reduce_data), prefix(DEFINE_seq_reduce_enumerate_with_init_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_reduce_enumerate_with_init_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	return data.gsr_result;
err_data_result:
	Dee_Decref(data.gsr_result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ "o", combine, start, end, init);
}

seq_reduce = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_reduce_with_init = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_reduce_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_reduce_with_range_and_init = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

