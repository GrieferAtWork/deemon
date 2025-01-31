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
 * "(cb:?DCallable,startkey:?Dint,endkey:?Dint)->?X2?O?N" */

/* function cb(key, value?) */
__map_enumerate__(cb:?DCallable,startkey?:?Dint,endkey?:?Dint)->?X2?O?N {
	Dee_ssize_t foreach_status;
	struct seq_enumerate_data data;
	size_t startkey = 0;
	size_t endkey = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":__map_enumerate__", &data.sed_cb, &startkey, &endkey))
		goto err;
	if (startkey == 0 && endkey == (size_t)-1) {
		foreach_status = DeeMap_OperatorEnumerate(self, &seq_enumerate_cb, &data);
	} else {
		foreach_status = DeeMap_OperatorEnumerateIndex(self, &seq_enumerate_index_cb, &data, startkey, endkey);
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
__map_enumerate__.map_operator_enumerate([[nonnull]] DeeObject *__restrict self,
                                         [[nonnull]] Dee_enumerate_t proc,
                                         void *arg)
%{unsupported({ return err_map_unsupportedf(self, "__map_enumerate__(...)"); })}
%{$empty = "default__set_operator_foreach_pair__empty"}
/*%{$with__set_operator_foreach_pair = {
	// Not explicitly defined; we just directly alias "set_operator_foreach_pair"!
	return DeeSeq_OperatorForeachPair(self, proc, arg);
}}*/ {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = LOCAL_CALLATTR(self, 1, (DeeObject *const *)&wrapper);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}


%[define(DEFINE_default_enumerate_index_with_enumerate_cb =
#ifndef DEFINED_default_enumerate_index_with_enumerate_cb
#define DEFINED_default_enumerate_index_with_enumerate_cb
/* tp_enumerate_index */
struct default_enumerate_index_with_enumerate_data {
	Dee_enumerate_index_t deiwe_proc;  /* [1..1] Underlying callback. */
	void                 *deiwe_arg;   /* [?..?] Cookie for `deiwe_proc' */
	size_t                deiwe_start; /* Enumeration start index */
	size_t                deiwe_end;   /* Enumeration end index */
};

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_enumerate_index_with_enumerate_cb(void *arg, DeeObject *key, DeeObject *value);
#endif /* !DEFINED_default_enumerate_index_with_enumerate_cb */
)]

[[wunused]] Dee_ssize_t
__map_enumerate__.map_operator_enumerate_index([[nonnull]] DeeObject *__restrict self,
                                               [[nonnull]] Dee_enumerate_index_t proc,
                                               void *arg, size_t start, size_t end)
%{unsupported({ return err_map_unsupportedf(self, "__map_enumerate__(..., %" PRFuSIZ ", %" PRFuSIZ ")", start, end); })}
%{$empty = 0}
%{$with__map_operator_enumerate = [[prefix(DEFINE_default_enumerate_index_with_enumerate_cb)]] {
	struct default_enumerate_index_with_enumerate_data data;
	data.deiwe_proc  = proc;
	data.deiwe_arg   = arg;
	data.deiwe_start = start;
	data.deiwe_end   = end;
	return DeeMap_OperatorEnumerate(self, &default_enumerate_index_with_enumerate_cb, &data);
}} {
	DREF DeeObject *result;
	DREF EnumerateWrapper *wrapper;
	wrapper = EnumerateIndexWrapper_New(proc, arg);
	if unlikely(!wrapper)
		goto err;
	result = LOCAL_CALLATTRF(self, "o" PCKuSIZ PCKuSIZ, wrapper, start, end);
	return EnumerateWrapper_Decref(wrapper, result);
err:
	return -1;
}


map_operator_enumerate = {
	DeeMH_set_operator_foreach_pair_t set_operator_foreach_pair;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_MAP && DeeType_RequireEnumerate(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	set_operator_foreach_pair = REQUIRE(set_operator_foreach_pair);
	if (set_operator_foreach_pair)
		return set_operator_foreach_pair; /* Binary-compatible */
};

map_operator_enumerate_index = {
	DeeMH_map_operator_enumerate_t map_operator_enumerate;
#ifndef LOCAL_FOR_OPTIMIZE
	if (SEQ_CLASS == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	map_operator_enumerate = REQUIRE(map_operator_enumerate);
	if (map_operator_enumerate == &default__map_operator_enumerate__empty)
		return &$empty;
	if (map_operator_enumerate)
		return &$with__map_operator_enumerate;
};
