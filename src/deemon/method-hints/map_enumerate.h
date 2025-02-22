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
/* deemon.Mapping.__map_enumerate__()                                   */
/************************************************************************/

/* TODO: Doc string should be:
 * "(cb:?DCallable)->?X2?O?N\n"
 * "(cb:?DCallable,start,end)->?X2?O?N" */

/* function cb(key, value?) */
__map_enumerate__(cb:?DCallable,start?,end?)->?X2?O?N {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	DeeObject *start, *end = NULL;
	if (DeeArg_Unpack(argc, argv, "o|oo:__map_enumerate__", &data.sed_cb, &start, &end))
		goto err;
	if (end) {
		foreach_status = CALL_DEPENDENCY(map_enumerate_range, self, &seq_enumerate_cb, &data, start, end);
	} else {
		foreach_status = CALL_DEPENDENCY(map_enumerate, self, &seq_enumerate_cb, &data);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}


[[no_self_invocation_wrapper]]
[[operator(Mapping: tp_seq->tp_foreach_pair)]]
[[wunused]] Dee_ssize_t
__map_enumerate__.map_enumerate([[nonnull]] DeeObject *__restrict self,
                                [[nonnull]] Dee_seq_enumerate_t cb,
                                void *arg)
%{unsupported({
	return err_map_unsupportedf(self, "__map_enumerate__(<callback>)");
})}
%{$empty = "default__map_operator_foreach_pair__empty"}
/*%{$with__map_operator_foreach_pair = {
	// Not explicitly defined; we just directly alias "map_operator_foreach_pair"!
	return CALL_DEPENDENCY(seq_operator_foreach_pair, self, cb, arg);
}}*/
%{using map_enumerate_range: {
	/* TODO */
	(void)self;
	(void)cb;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}}
%{$with__map_iterkeys__and__map_operator_trygetitem = {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *key;
	DREF DeeObject *iterkeys;
	PRELOAD_DEPENDENCY(map_operator_trygetitem)
	iterkeys = CALL_DEPENDENCY(map_iterkeys, self);
	if unlikely(!iterkeys)
		goto err;
	while (ITER_ISOK(key = DeeObject_IterNext(iterkeys))) {
		DREF DeeObject *value;
		value = CALL_DEPENDENCY(map_operator_trygetitem, self, key);
		if unlikely(!value)
			goto err_iterkeys_key;
		if (value != ITER_DONE) {
			temp = (*cb)(arg, key, value);
			Dee_Decref(value);
		} else {
			temp = (*cb)(arg, key, NULL);
		}
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err_iterkeys_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_iterkeys;
	}
	Dee_Decref(iterkeys);
	if unlikely(!key)
		goto err;
	return result;
err_iterkeys_temp:
	Dee_Decref(iterkeys);
	return temp;
err_iterkeys_key:
	Dee_Decref(key);
err_iterkeys:
	Dee_Decref(iterkeys);
err:
	return -1;
}} {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	result = LOCAL_CALLATTR(self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}


%[define(DEFINE_map_enumerate_with_filter_cb =
#ifndef DEFINED_map_enumerate_with_filter_cb
#define DEFINED_map_enumerate_with_filter_cb
struct map_enumerate_with_filter_data {
	Dee_seq_enumerate_t mewfd_cb;           /* [1..1] Underlying callback. */
	void               *mewfd_arg;          /* Cookie for `mewfd_cb' */
	DeeObject          *mewfd_filter_start; /* [1..1] Filter start. */
	DeeObject          *mewfd_filter_end;   /* [1..1] Filter end. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_enumerate_with_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	int temp;
	struct map_enumerate_with_filter_data *data;
	data = (struct map_enumerate_with_filter_data *)arg;
	/* if (!(mewfd_filter_start <= index))
	 *     return 0; */
	temp = DeeObject_CmpLeAsBool(data->mewfd_filter_start, index);
	if (temp <= 0)
		return temp;

	/* if (!(mewfd_filter_end > index))
	 *     return 0; */
	temp = DeeObject_CmpGrAsBool(data->mewfd_filter_end, index);
	if (temp <= 0)
		return temp;

	return (*data->mewfd_cb)(data->mewfd_arg, index, value);
}
#endif /* !DEFINED_map_enumerate_with_filter_cb */
)]

[[wunused]] Dee_ssize_t
__map_enumerate__.map_enumerate_range([[nonnull]] DeeObject *self,
                                      [[nonnull]] Dee_seq_enumerate_t cb, void *arg,
                                      [[nonnull]] DeeObject *start,
                                      [[nonnull]] DeeObject *end)
%{unsupported({
	return err_map_unsupportedf(self, "__map_enumerate__(<callable>, %r, %r)", start, end);
})}
%{$empty = 0}
%{using map_enumerate: [[prefix(DEFINE_map_enumerate_with_filter_cb)]] {
	struct map_enumerate_with_filter_data data;
	data.mewfd_cb           = cb;
	data.mewfd_arg          = arg;
	data.mewfd_filter_start = start;
	data.mewfd_filter_end   = end;
	return CALL_DEPENDENCY(map_enumerate, self, &map_enumerate_with_filter_cb, &data);
}}
%{$with__map_iterkeys__and__map_operator_trygetitem = {
	/* TODO */
	(void)self;
	(void)cb;
	(void)arg;
	return DeeError_NOTIMPLEMENTED();
}} {
	DeeObject *args[3];
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(cb, arg);
	if unlikely(!wrapper)
		goto err;
	args[0] = (DeeObject *)wrapper;
	args[1] = start;
	args[2] = end;
	result  = LOCAL_CALLATTR(self, 3, args);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}


map_enumerate = {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	DeeMH_map_iterkeys_t map_iterkeys;
	/*if (REQUIRE_NODEFAULT(map_enumerate_range))
		return &$with__map_enumerate_range;*/
	map_iterkeys = REQUIRE_NODEFAULT(map_iterkeys);
	if (map_iterkeys) {
		if (map_iterkeys == &default__map_iterkeys__empty)
			return &$empty;
		if (map_iterkeys != &default__map_iterkeys__with__map_operator_iter) {
			DeeMH_map_operator_trygetitem_t map_operator_trygetitem = REQUIRE_NODEFAULT(map_operator_trygetitem);
			if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
				return &$empty;
			if (map_operator_trygetitem)
				return &$with__map_iterkeys__and__map_operator_trygetitem;
		}
	}
	map_operator_foreach_pair = REQUIRE(map_operator_foreach_pair);
	if (map_operator_foreach_pair)
		return map_operator_foreach_pair; /* Binary-compatible */
};

map_enumerate_range = {
	DeeMH_map_enumerate_t map_enumerate = REQUIRE(map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &$empty;
	if (map_enumerate == &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem)
		return &$with__map_iterkeys__and__map_operator_trygetitem;
	if (map_enumerate)
		return &$with__map_enumerate;
};
