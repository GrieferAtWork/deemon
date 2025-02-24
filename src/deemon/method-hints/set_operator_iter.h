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
/* deemon.Set.operator iter()                                           */
/************************************************************************/
__set_iter__()->?DIterator {
	if (DeeArg_Unpack(argc, argv, ":__set_iter__"))
		goto err;
	return CALL_DEPENDENCY(set_operator_iter, self);
err:
	return NULL;
}


[[operator(Set: tp_seq->tp_iter)]] /* TODO: Allow hint init from Mapping, but not operator loading into Mapping */
[[wunused]] DREF DeeObject *
__set_iter__.set_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$none = return_none}
%{$empty = "default__seq_operator_iter__empty"}
%{$with__seq_operator_iter = {
	DREF DeeObject *iter;
	DeeTypeObject *itertyp;
	DREF DistinctIterator *result;
	iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctIterator);
	if unlikely(!result)
		goto err_iter;
	itertyp            = Dee_TYPE(iter);
	result->di_tp_next = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->di_iter    = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->di_encountered);
	DeeObject_Init(result, &DistinctIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}


%[define(DEFINE_default_set_foreach_unique_cb =
#ifndef DEFINED_default_set_foreach_unique_cb
#define DEFINED_default_set_foreach_unique_cb
struct default_set_foreach_unique_data {
	struct Dee_simple_hashset dsfud_encountered; /* Set of objects already encountered. */
	Dee_foreach_t             dsfud_cb;          /* [1..1] user-defined callback */
	void                     *dsfud_arg;         /* [?..?] Cookie for `dsfud_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_foreach_unique_cb(void *arg, DeeObject *item) {
	int insert_status;
	struct default_set_foreach_unique_data *data;
	data = (struct default_set_foreach_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dsfud_encountered, item);
	if likely(insert_status > 0)
		return (*data->dsfud_cb)(data->dsfud_arg, item);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_set_foreach_unique_cb */
)]


[[operator(Set: tp_seq->tp_foreach)]] /* TODO: Allow hint init from Mapping, but not operator loading into Mapping */
[[wunused]] Dee_ssize_t
__set_iter__.set_operator_foreach([[nonnull]] DeeObject *__restrict self,
                                  [[nonnull]] Dee_foreach_t cb,
                                  void *arg)
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_set_foreach_unique_cb)]] {
	Dee_ssize_t result;
	struct default_set_foreach_unique_data data;
	data.dsfud_cb  = cb;
	data.dsfud_arg = arg;
	Dee_simple_hashset_init(&data.dsfud_encountered);
	result = CALL_DEPENDENCY(seq_operator_foreach, self, &default_set_foreach_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dsfud_encountered);
	return result;
}}
%{$with__map_operator_foreach_pair = [[prefix(DEFINE_default_foreach_with_foreach_pair_cb)]] {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return CALL_DEPENDENCY(map_operator_foreach_pair, self, &default_foreach_with_foreach_pair_cb, &data);
}}
%{using set_operator_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = CALL_DEPENDENCY(set_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}} = $with__set_operator_iter;



set_operator_iter = {
	DeeMH_seq_operator_iter_t seq_operator_iter;
	DeeMH_map_operator_iter_t map_operator_iter = REQUIRE_NODEFAULT(map_operator_iter);
	if (map_operator_iter)
		return map_operator_iter;
	seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
		return seq_operator_iter;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

set_operator_foreach = {
	DeeMH_set_operator_iter_t set_operator_iter;
	if (REQUIRE_NODEFAULT(map_operator_foreach_pair))
		return &$with__map_operator_foreach_pair;
	set_operator_iter = REQUIRE(set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__empty)
		return &$empty;
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter)
		return &$with__seq_operator_foreach;
	if (set_operator_iter)
		return &$with__set_operator_iter;
};
