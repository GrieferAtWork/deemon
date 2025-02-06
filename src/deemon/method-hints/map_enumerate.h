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
 * "(cb:?DCallable,startkey,endkey)->?X2?O?N" */

/* function cb(key, value?) */
__map_enumerate__(cb:?DCallable,startkey?,endkey?)->?X2?O?N {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	DeeObject *startkey, *endkey = NULL;
	if (DeeArg_Unpack(argc, argv, "o|oo:__map_enumerate__", &data.sed_cb, &startkey, &endkey))
		goto err;
	if (endkey) {
		foreach_status = DeeType_InvokeMethodHint(self, map_enumerate_range, &seq_enumerate_cb, &data);
	} else {
		foreach_status = DeeType_InvokeMethodHint(self, map_enumerate, &seq_enumerate_cb, &data);
	}
	if unlikely(foreach_status == -1)
		goto err;
	if (foreach_status == -2)
		return data.sed_result;
	return_none;
err:
	return NULL;
}

[[wunused]] Dee_ssize_t
__map_enumerate__.map_enumerate([[nonnull]] DeeObject *__restrict self,
                                [[nonnull]] Dee_seq_enumerate_t cb,
                                void *arg)
%{unsupported({ return err_map_unsupportedf(self, "__map_enumerate__(<cb>)"); })}
%{$empty = "default__set_operator_foreach_pair__empty"}
/*%{$with__set_operator_foreach_pair = {
	// Not explicitly defined; we just directly alias "set_operator_foreach_pair"!
	return DeeType_InvokeMethodHint(self, seq_operator_foreach_pair, cb, arg);
}}*/ {
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


//%[define(DEFINE_map_enumerate_with_filter_cb =
//#ifndef DEFINED_map_enumerate_with_filter_cb
//#define DEFINED_map_enumerate_with_filter_cb
//struct map_enumerate_with_filter_data {
//	Dee_seq_enumerate_t mewfd_cb;           /* [1..1] Underlying callback. */
//	void               *mewfd_arg;          /* Cookie for `mewfd_cb' */
//	DeeObject          *mewfd_filter_start; /* [1..1] Filter start. */
//	DeeObject          *mewfd_filter_end;   /* [1..1] Filter end. */
//};
//
//PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
//map_enumerate_with_filter_cb(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
//	int temp;
//	struct map_enumerate_with_filter_data *data;
//	data = (struct map_enumerate_with_filter_data *)arg;
//	/* if (!(mewfd_filter_start <= index))
//	 *     return 0; */
//	temp = DeeObject_CmpLeAsBool(data->mewfd_filter_start, index);
//	if (temp <= 0)
//		return temp;
//
//	/* if (!(mewfd_filter_end > index))
//	 *     return 0; */
//	temp = DeeObject_CmpGrAsBool(data->mewfd_filter_end, index);
//	if (temp <= 0)
//		return temp;
//
//	return (*data->mewfd_cb)(data->mewfd_arg, index, value);
//}
//#endif /* !DEFINED_map_enumerate_with_filter_cb */
//)]
//
//[[wunused]] Dee_ssize_t
//__map_enumerate__.map_enumerate_range([[nonnull]] DeeObject *self,
//                                      [[nonnull]] Dee_seq_enumerate_t cb, void *arg,
//                                      [[nonnull]] DeeObject *start,
//                                      [[nonnull]] DeeObject *end)
//%{unsupported({ return err_map_unsupportedf(self, "__map_enumerate__(<cb>, %r, %r)", start, end); })}
//%{$empty = 0}
//%{$with__map_enumerate = [[prefix(DEFINE_map_enumerate_with_filter_cb)]] {
//	struct map_enumerate_with_filter_data data;
//	data.mewfd_cb           = cb;
//	data.mewfd_arg          = arg;
//	data.mewfd_filter_start = start;
//	data.mewfd_filter_end   = end;
//	return DeeType_InvokeMethodHint(self, map_enumerate, &map_enumerate_with_filter_cb, &data);
//}} {
//	DeeObject *args[3];
//	DREF DeeObject *result;
//	DREF EnumerateWrapper *wrapper;
//	wrapper = EnumerateWrapper_New(cb, arg);
//	if unlikely(!wrapper)
//		goto err;
//	args[0] = (DeeObject *)wrapper;
//	args[1] = start;
//	args[2] = end;
//	result  = LOCAL_CALLATTR(self, 3, args);
//	return EnumerateWrapper_Decref(wrapper, result);
//err:
//	return -1;
//}


map_enumerate = {
	DeeMH_set_operator_foreach_pair_t set_operator_foreach_pair = REQUIRE(set_operator_foreach_pair);
	if (set_operator_foreach_pair)
		return set_operator_foreach_pair; /* Binary-compatible */
};

//map_enumerate_range = {
//	DeeMH_map_enumerate_t map_enumerate = REQUIRE(map_enumerate);
//	if (map_enumerate == &default__map_enumerate__empty)
//		return &$empty;
//	if (map_enumerate)
//		return &$with__map_enumerate;
//};
