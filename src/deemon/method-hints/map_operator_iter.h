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
/* deemon.Mapping.operator iter()                                           */
/************************************************************************/
__map_iter__()->?DIterator {
	return CALL_DEPENDENCY(map_operator_iter, self);
}


[[operator(Mapping: tp_seq->tp_iter)]]
[[wunused]] DREF DeeObject *
__map_iter__.map_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$none = return_none}
%{$empty = "default__seq_operator_iter__empty"}
%{$with__map_enumerate = "default__seq_operator_iter__with__map_enumerate"}
%{$with__map_iterkeys__and__map_operator_trygetitem = "default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem"}
%{$with__map_iterkeys__and__map_operator_getitem = "default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem"}
%{$with__seq_operator_iter = {
	DREF DeeObject *iter;
	DeeTypeObject *itertyp;
	DREF DistinctMappingIterator *result;
	iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctMappingIterator);
	if unlikely(!result)
		goto err_iter;
	itertyp                 = Dee_TYPE(iter);
	result->dmi_tp_nextpair = DeeType_RequireNativeOperator(itertyp, nextpair);
	result->dmi_iter        = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->dmi_encountered);
	DeeObject_Init(result, &DistinctMappingIterator_Type);
	return DeeGC_Track(Dee_AsObject(result));
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}



%[define(DEFINE_default_foreach_pair_with_map_enumerate_cb =
#ifndef DEFINED_default_foreach_pair_with_map_enumerate_cb
#define DEFINED_default_foreach_pair_with_map_enumerate_cb
struct default_foreach_pair_with_map_enumerate_data {
	Dee_foreach_pair_t dfpwme_cb;  /* [1..1] Underlying callback */
	void              *dfpwme_arg; /* [?..?] Cookie for `dfpwme_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_pair_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_foreach_pair_with_map_enumerate_data *data;
	data = (struct default_foreach_pair_with_map_enumerate_data *)arg;
	if likely(value)
		return (*data->dfpwme_cb)(data->dfpwme_arg, key, value);
	return 0;
}
#endif /* !DEFINED_default_foreach_pair_with_map_enumerate_cb */
)]



%[define(DEFINE_default_map_foreach_pair_unique_cb =
#ifndef DEFINED_default_map_foreach_pair_unique_cb
#define DEFINED_default_map_foreach_pair_unique_cb
struct default_map_foreach_pair_unique_data {
	struct Dee_simple_hashset dmfpud_encountered; /* Set of keys already encountered. */
	Dee_foreach_pair_t        dmfpud_cb;          /* [1..1] user-defined callback */
	void                     *dmfpud_arg;         /* [?..?] Cookie for `dmfpud_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_map_foreach_pair_unique_cb(void *arg, DeeObject *key, DeeObject *value) {
	int insert_status;
	struct default_map_foreach_pair_unique_data *data;
	data = (struct default_map_foreach_pair_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dmfpud_encountered, key);
	if likely(insert_status > 0)
		return (*data->dmfpud_cb)(data->dmfpud_arg, key, value);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_map_foreach_pair_unique_cb */
)]


[[operator(Mapping: tp_seq->tp_foreach_pair)]]
[[wunused]] Dee_ssize_t
__map_iter__.map_operator_foreach_pair([[nonnull]] DeeObject *__restrict self,
                                       [[nonnull]] Dee_foreach_pair_t cb,
                                       void *arg)
%{$empty = 0}
%{using map_operator_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = CALL_DEPENDENCY(map_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}}
%{$with__seq_operator_foreach_pair = [[prefix(DEFINE_default_map_foreach_pair_unique_cb)]] {
	Dee_ssize_t result;
	struct default_map_foreach_pair_unique_data data;
	data.dmfpud_cb  = cb;
	data.dmfpud_arg = arg;
	Dee_simple_hashset_init(&data.dmfpud_encountered);
	result = CALL_DEPENDENCY(seq_operator_foreach_pair, self, &default_map_foreach_pair_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dmfpud_encountered);
	return result;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_foreach_pair_with_map_enumerate_cb)]] {
	struct default_foreach_pair_with_map_enumerate_data data;
	data.dfpwme_cb  = cb;
	data.dfpwme_arg = arg;
	return CALL_DEPENDENCY(map_enumerate, self, &default_foreach_pair_with_map_enumerate_cb, &data);
}} = $with__map_operator_iter;



map_operator_iter = {
	DeeMH_seq_operator_iter_t seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
		return seq_operator_iter;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

map_operator_foreach_pair = {
	DeeMH_map_operator_iter_t map_operator_iter;
	if (REQUIRE_NODEFAULT(map_enumerate) || REQUIRE_NODEFAULT(map_enumerate_range))
		return &$with__map_enumerate;
	map_operator_iter = REQUIRE(map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__empty)
		return &$empty;
	if (map_operator_iter == &default__map_operator_iter__with__seq_operator_iter)
		return &$with__seq_operator_foreach_pair;
	if (map_operator_iter)
		return &$with__map_operator_iter;
};
