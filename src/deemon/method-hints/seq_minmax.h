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

%[define(DEFINE_seq_minmax_with_key_data =
#ifndef DEFINED_seq_minmax_with_key_data
#define DEFINED_seq_minmax_with_key_data
struct seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};
#endif /* !DEFINED_seq_minmax_with_key_data */
)]

/*[[[deemon
function g(p) {
	print p.replace("{m}", "min").replace("{M}", "Min").replace("{==|>}", "==").replace("{>|==}", ">");
	print;
	print;
	print;
	print;
	print;
	print p.replace("{m}", "max").replace("{M}", "Max").replace("{==|>}", ">").replace("{>|==}", "==");
}

g('/' '************************************************************************' '/
/' '* deemon.Sequence.{m}()                                                *' '/
/' '************************************************************************' '/
[[kw, alias(Sequence.{m} -> "seq_{m}")]]
__seq_{m}__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?O {
	DREF DeeObject *result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_{m}__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_{m}_with_key, self, key)
		         : CALL_DEPENDENCY(seq_{m}, self);
	} else {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_{m}_with_range_and_key, self, start, end, key)
		         : CALL_DEPENDENCY(seq_{m}_with_range, self, start, end);
	}
	return result;
err:
	return NULL;
}

%[define(DEFINE_seq_{m}_foreach_cb =
#ifndef DEFINED_seq_{m}_foreach_cb
#define DEFINED_seq_{m}_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_{m}_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp {==|>} 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}
#endif /' '* !DEFINED_seq_{m}_foreach_cb *' '/
)]

[[wunused]] DREF DeeObject *
__seq_{m}__.seq_{m}([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_{m}_foreach_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_{m}_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

%[define(DEFINE_seq_{m}_with_key_foreach_cb =
DEFINE_seq_minmax_with_key_data
#ifndef DEFINED_seq_{m}_with_key_foreach_cb
#define DEFINED_seq_{m}_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_{m}_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct seq_minmax_with_key_data *data;
	data = (struct seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /' '* Inherit reference *' '/
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /' '* Inherit reference *' '/
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp {>|==} 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /' '* Inherit reference *' '/
	data->gsmmwk_kresult = kelem; /' '* Inherit reference *' '/
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}
#endif /' '* !DEFINED_seq_{m}_with_key_foreach_cb *' '/
)]

[[wunused]] DREF DeeObject *
__seq_{m}__.seq_{m}_with_key([[nonnull]] DeeObject *self,
                             [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_{m}_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_{m}_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/' '*err:*' '/
	return NULL;
}} {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return LOCAL_CALLATTR(self, 3, args);
}

%[define(DEFINE_seq_{m}_enumerate_cb =
DEFINE_seq_{m}_foreach_cb
#ifndef DEFINED_seq_{m}_enumerate_cb
#define DEFINED_seq_{m}_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_{m}_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_{m}_foreach_cb(arg, item);
}
#endif /' '* !DEFINED_seq_{m}_enumerate_cb *' '/
)]

[[wunused]] DREF DeeObject *
__seq_{m}__.seq_{m}_with_range([[nonnull]] DeeObject *__restrict self,
                               size_t start, size_t end)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_{m}_enumerate_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_{m}_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

%[define(DEFINE_seq_{m}_with_key_enumerate_cb =
DEFINE_seq_{m}_with_key_foreach_cb
#ifndef DEFINED_seq_{m}_with_key_enumerate_cb
#define DEFINED_seq_{m}_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_{m}_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_{m}_with_key_foreach_cb(arg, item);
}
#endif /' '* !DEFINED_seq_{m}_with_key_enumerate_cb *' '/
)]

[[wunused]] DREF DeeObject *
__seq_{m}__.seq_{m}_with_range_and_key([[nonnull]] DeeObject *self,
                                       size_t start, size_t end,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_{m}_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_{m}_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/' '*err:*' '/
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
}


seq_{m} = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_{m}_with_key = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_{m}_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_{m}_with_range_and_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};');
]]]*/
/************************************************************************/
/* deemon.Sequence.min()                                                */
/************************************************************************/
[[kw, alias(Sequence.min -> "seq_min")]]
__seq_min__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?O {
	DREF DeeObject *result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_min__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_min_with_key, self, key)
		         : CALL_DEPENDENCY(seq_min, self);
	} else {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_min_with_range_and_key, self, start, end, key)
		         : CALL_DEPENDENCY(seq_min_with_range, self, start, end);
	}
	return result;
err:
	return NULL;
}

%[define(DEFINE_seq_min_foreach_cb =
#ifndef DEFINED_seq_min_foreach_cb
#define DEFINED_seq_min_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_min_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp == 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}
#endif /* !DEFINED_seq_min_foreach_cb */
)]

[[wunused]] DREF DeeObject *
__seq_min__.seq_min([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_min_foreach_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_min_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

%[define(DEFINE_seq_min_with_key_foreach_cb =
DEFINE_seq_minmax_with_key_data
#ifndef DEFINED_seq_min_with_key_foreach_cb
#define DEFINED_seq_min_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_min_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct seq_minmax_with_key_data *data;
	data = (struct seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp > 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}
#endif /* !DEFINED_seq_min_with_key_foreach_cb */
)]

[[wunused]] DREF DeeObject *
__seq_min__.seq_min_with_key([[nonnull]] DeeObject *self,
                             [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_min_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_min_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}} {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return LOCAL_CALLATTR(self, 3, args);
}

%[define(DEFINE_seq_min_enumerate_cb =
DEFINE_seq_min_foreach_cb
#ifndef DEFINED_seq_min_enumerate_cb
#define DEFINED_seq_min_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_min_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_min__.seq_min_with_range([[nonnull]] DeeObject *__restrict self,
                               size_t start, size_t end)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_min_enumerate_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_min_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

%[define(DEFINE_seq_min_with_key_enumerate_cb =
DEFINE_seq_min_with_key_foreach_cb
#ifndef DEFINED_seq_min_with_key_enumerate_cb
#define DEFINED_seq_min_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_min_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_min_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_min_with_key_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_min__.seq_min_with_range_and_key([[nonnull]] DeeObject *self,
                                       size_t start, size_t end,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_min_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_min_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
}


seq_min = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_min_with_key = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_min_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_min_with_range_and_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};





/************************************************************************/
/* deemon.Sequence.max()                                                */
/************************************************************************/
[[kw, alias(Sequence.max -> "seq_max")]]
__seq_max__(start=!0,end:?Dint=!A!Dint!PSIZE_MAX,key:?DCallable=!N)->?O {
	DREF DeeObject *result;
	DeeObject *key = Dee_None;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_key,
	                    "|" UNPuSIZ UNPuSIZ "o:__seq_max__",
	                    &start, &end, &key))
		goto err;
	if (start == 0 && end == (size_t)-1) {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_max_with_key, self, key)
		         : CALL_DEPENDENCY(seq_max, self);
	} else {
		result = !DeeNone_Check(key)
		         ? CALL_DEPENDENCY(seq_max_with_range_and_key, self, start, end, key)
		         : CALL_DEPENDENCY(seq_max_with_range, self, start, end);
	}
	return result;
err:
	return NULL;
}

%[define(DEFINE_seq_max_foreach_cb =
#ifndef DEFINED_seq_max_foreach_cb
#define DEFINED_seq_max_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_max_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = item;
		Dee_Incref(item);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, item);
	if (temp > 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(item);
		*(DeeObject **)arg = item;
	}
	return temp;
}
#endif /* !DEFINED_seq_max_foreach_cb */
)]

[[wunused]] DREF DeeObject *
__seq_max__.seq_max([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_max_foreach_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_max_foreach_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}

%[define(DEFINE_seq_max_with_key_foreach_cb =
DEFINE_seq_minmax_with_key_data
#ifndef DEFINED_seq_max_with_key_foreach_cb
#define DEFINED_seq_max_with_key_foreach_cb
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_max_with_key_foreach_cb(void *arg, DeeObject *item) {
	int temp;
	DREF DeeObject *kelem;
	struct seq_minmax_with_key_data *data;
	data = (struct seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(item);
		data->gsmmwk_result = item; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &item);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp == 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(item);
	data->gsmmwk_result  = item;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}
#endif /* !DEFINED_seq_max_with_key_foreach_cb */
)]

[[wunused]] DREF DeeObject *
__seq_max__.seq_max_with_key([[nonnull]] DeeObject *self,
                             [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_operator_foreach = [[prefix(DEFINE_seq_max_with_key_foreach_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_operator_foreach, self, &seq_max_with_key_foreach_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}} {
	DeeObject *args[3];
	args[0] = DeeInt_Zero;
	args[1] = (DeeObject *)&Dee_int_SIZE_MAX;
	args[2] = key;
	return LOCAL_CALLATTR(self, 3, args);
}

%[define(DEFINE_seq_max_enumerate_cb =
DEFINE_seq_max_foreach_cb
#ifndef DEFINED_seq_max_enumerate_cb
#define DEFINED_seq_max_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_max_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_max__.seq_max_with_range([[nonnull]] DeeObject *__restrict self,
                               size_t start, size_t end)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_max_enumerate_cb)]] {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_max_enumerate_cb, &result, start, end);
	if unlikely(foreach_status < 0)
		goto err_r;
	if unlikely(!result)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ, start, end);
}

%[define(DEFINE_seq_max_with_key_enumerate_cb =
DEFINE_seq_max_with_key_foreach_cb
#ifndef DEFINED_seq_max_with_key_enumerate_cb
#define DEFINED_seq_max_with_key_enumerate_cb
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
seq_max_with_key_enumerate_cb(void *arg, size_t index, DeeObject *item) {
	(void)index;
	if (!item)
		return 0;
	return seq_max_with_key_foreach_cb(arg, item);
}
#endif /* !DEFINED_seq_max_with_key_enumerate_cb */
)]

[[wunused]] DREF DeeObject *
__seq_max__.seq_max_with_range_and_key([[nonnull]] DeeObject *self,
                                       size_t start, size_t end,
                                       [[nonnull]] DeeObject *key)
%{unsupported(auto)} %{$empty = return_none}
%{$with__seq_enumerate_index = [[prefix(DEFINE_seq_max_with_key_enumerate_cb)]] {
	Dee_ssize_t foreach_status;
	struct seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = CALL_DEPENDENCY(seq_enumerate_index, self, &seq_max_with_key_enumerate_cb, &data, start, end);
	if unlikely(foreach_status < 0)
		goto err_data;
	if unlikely(!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
/*err:*/
	return NULL;
}} {
	return LOCAL_CALLATTRF(self, PCKuSIZ PCKuSIZ "o", start, end, key);
}


seq_max = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_max_with_key = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};

seq_max_with_range = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};

seq_max_with_range_and_key = {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = REQUIRE(seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &$empty;
	if (seq_enumerate_index)
		return &$with__seq_enumerate_index;
};
/*[[[end]]]*/

