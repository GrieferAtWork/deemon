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
	return DeeSet_OperatorIter(self);
err:
	return NULL;
}


[[wunused]] DREF DeeObject *
__set_iter__.set_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$empty = "default__seq_operator_iter__empty"}
%{$with__seq_operator_iter = {
	DREF DeeObject *iter;
	DREF DistinctIterator *result;
	iter = DeeSeq_OperatorIter(self);
	if unlikely(!iter)
		goto err;
	result = DeeGCObject_MALLOC(DistinctIterator);
	if unlikely(!result)
		goto err_iter;
	result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
	if unlikely(!result->di_tp_next) {
		if unlikely(!DeeType_InheritIterNext(Dee_TYPE(iter))) {
			err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
			goto err_iter_result;
		}
		result->di_tp_next = Dee_TYPE(iter)->tp_iter_next;
		ASSERT(result->di_tp_next);
	}
	result->di_iter = iter; /* Inherit reference */
	Dee_simple_hashset_with_lock_init(&result->di_encountered);
	DeeObject_Init(result, &DistinctIterator_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_iter_result:
	DeeGCObject_FREE(result);
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
struct default_set_foreach_unique_cb_data {
	Dee_foreach_t dsfucd_cb;  /* [1..1] user-defined callback */
	void         *dsfucd_arg; /* [?..?] Cookie for `dsfucd_cb' */
};

struct default_set_foreach_unique_data {
	struct Dee_simple_hashset                 dsfud_encountered; /* Set of objects already encountered. */
	struct default_set_foreach_unique_cb_data dsfud_cb;          /* Callback data */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_set_foreach_unique_cb(void *arg, DeeObject *item) {
	int insert_status;
	struct default_set_foreach_unique_data *data;
	data = (struct default_set_foreach_unique_data *)arg;
	insert_status = Dee_simple_hashset_insert(&data->dsfud_encountered, item);
	if likely(insert_status > 0)
		return (*data->dsfud_cb.dsfucd_cb)(data->dsfud_cb.dsfucd_arg, item);
	return insert_status; /* error, or already-exists */
}
#endif /* !DEFINED_default_set_foreach_unique_cb */
)]

[[wunused]] Dee_ssize_t
__set_iter__.set_operator_foreach([[nonnull]] DeeObject *__restrict self,
                                  [[nonnull]] Dee_foreach_t cb,
                                  void *arg)
%{$empty = 0}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_set_foreach_unique_cb)]] {
	Dee_ssize_t result;
	struct default_set_foreach_unique_data data;
	data.dsfud_cb.dsfucd_cb  = cb;
	data.dsfud_cb.dsfucd_arg = arg;
	Dee_simple_hashset_init(&data.dsfud_encountered);
	result = DeeSeq_OperatorForeach(self, &default_set_foreach_unique_cb, &data);
	Dee_simple_hashset_fini(&data.dsfud_encountered);
	return result;
}}
%{$with__set_operator_iter = {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = DeeSet_OperatorIter(self);
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
#ifndef LOCAL_FOR_OPTIMIZE
	if (Dee_SEQCLASS_ISSETORMAP(SEQ_CLASS) && DeeType_RequireIter(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter)
		return &$with__seq_operator_iter;
};

set_operator_foreach = {
	DeeMH_set_operator_iter_t set_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (Dee_SEQCLASS_ISSETORMAP(SEQ_CLASS) && DeeType_RequireForeach(THIS_TYPE))
		return THIS_TYPE->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
	set_operator_iter = REQUIRE(set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__empty)
		return &$empty;
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter)
		return &$with__seq_operator_foreach;
	if (set_operator_iter)
		return &$with__set_operator_iter;
};
