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
/* deemon.Sequence.operator iter()                                      */
/************************************************************************/
__seq_iter__()->?DIterator {
	return CALL_DEPENDENCY(seq_operator_iter, self);
}

/* The "iter" operator callbacks are special, in that methods hints
 * here can be loaded from "tp_iter" and friends, no matter what the
 * sequence class of the associated type is (even if it doesn't seem
 * to be a sequence class at all). This is because even types that
 * don't implement the "Sequence" interface (or "Set" / "Mapping")
 * can still provide "operator iter", and there is no way to mis-
 * understand what that operator is supposed to do (and even in the
 * very special cases of "rt.SeqEach", that type can just provide
 * a custom "__seq_iter__" method which would be used here instead) */
[[accept_any_base_class_for_method_hint]]
[[operator(Sequence: tp_seq->tp_iter)]]
[[wunused]] DREF DeeObject *
__seq_iter__.seq_operator_iter([[nonnull]] DeeObject *__restrict self)
%{unsupported(auto("operator iter"))}
%{$none = return_none}
%{$empty = return DeeIterator_NewEmpty()}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFast_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = REQUIRE_DEPENDENCY(seq_operator_trygetitem_index);
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	size_t size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->disgi_seq              = self;
	result->disgi_tp_getitem_index = REQUIRE_DEPENDENCY(seq_operator_getitem_index);
	result->disgi_index            = 0;
	result->disgi_end              = size;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_getitem_index = {
	DREF DefaultIterator_WithGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->digi_seq              = self;
	result->digi_tp_getitem_index = REQUIRE_DEPENDENCY(seq_operator_getitem_index);
	result->digi_index            = 0;
	DeeObject_Init(result, &DefaultIterator_WithGetItemIndex_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err_size_ob;
	result->disg_index = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!result->disg_index) {
		DeeGCObject_FREE(result);
		goto err_size_ob;
	}
	Dee_Incref(self);
	result->disg_seq        = self; /* Inherit reference */
	result->disg_tp_getitem = REQUIRE_DEPENDENCY(seq_operator_getitem);
	result->disg_end        = sizeob; /* Inherit reference */
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItem_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err_size_ob:
	Dee_Decref(sizeob);
err:
	return NULL;
}}
%{$with__seq_operator_getitem = {
	DREF DefaultIterator_WithGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithGetItem);
	if unlikely(!result)
		goto err;
	result->dig_index = DeeInt_NewZero();
	Dee_Incref(self);
	result->dig_seq        = self; /* Inherit reference */
	result->dig_tp_getitem = REQUIRE_DEPENDENCY(seq_operator_getitem);
	Dee_atomic_lock_init(&result->dig_lock);
	DeeObject_Init(result, &DefaultIterator_WithGetItem_Type);
	return DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
}}
%{$with__map_enumerate = {
	/* TODO: Custom iterator type that uses "map_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}}
%{$with__seq_enumerate = {
	/* TODO: Custom iterator type that uses "seq_enumerate" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}}
%{$with__seq_enumerate_index = {
	/* TODO: Custom iterator type that uses "seq_enumerate_index" */
	(void)self;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}}
%{$with__map_iterkeys__and__map_operator_trygetitem = {
	/* Custom iterator type:
	 * >> local it = self.operator iterkeys();
	 * >> return (() -> {
	 * >>     foreach (local key: it) {
	 * >>         local value = self.trygetitem(key);
	 * >>         if (value != ITER_DONE)
	 * >>             yield (key, value);
	 * >>     }
	 * >> })().operator iter();
	 */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = CALL_DEPENDENCY(map_iterkeys, self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	itertyp                   = Dee_TYPE(result->diikgi_iter);
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->diikgi_tp_getitem = REQUIRE_DEPENDENCY(map_operator_trygetitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndTryGetItemMap_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}}
%{$with__map_iterkeys__and__map_operator_getitem = {
	/* Custom iterator type:
	 * >> local it = self.operator iterkeys();
	 * >> return (() -> {
	 * >>     foreach (local key: it) {
	 * >>         local value = self.trygetitem(key);
	 * >>         if (value != ITER_DONE)
	 * >>             yield (key, value);
	 * >>     }
	 * >> })().operator iter();
	 */
	DeeTypeObject *itertyp;
	DREF DefaultIterator_WithIterKeysAndGetItem *result;
	result = DeeObject_MALLOC(DefaultIterator_WithIterKeysAndGetItem);
	if unlikely(!result)
		goto err;
	result->diikgi_iter = CALL_DEPENDENCY(map_iterkeys, self);
	if unlikely(!result->diikgi_iter)
		goto err_r;
	Dee_Incref(self);
	result->diikgi_seq        = self;
	itertyp                   = Dee_TYPE(result->diikgi_iter);
	result->diikgi_tp_next    = DeeType_RequireNativeOperator(itertyp, iter_next);
	result->diikgi_tp_getitem = REQUIRE_DEPENDENCY(map_operator_getitem);
	DeeObject_Init(result, &DefaultIterator_WithIterKeysAndGetItemMap_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 0, NULL);
}



%[define(DEFINE_default_foreach_with_map_enumerate_cb =
#ifndef DEFINED_default_foreach_with_map_enumerate_cb
#define DEFINED_default_foreach_with_map_enumerate_cb
struct default_foreach_with_map_enumerate_data {
	Dee_foreach_t dfwme_cb;  /* [1..1] Underlying callback */
	void         *dfwme_arg; /* [?..?] Cookie for `dfwme_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_foreach_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t result;
	DREF DeeTupleObject *pair;
	struct default_foreach_with_map_enumerate_data *data;
	data = (struct default_foreach_with_map_enumerate_data *)arg;
	if unlikely(!value)
		return 0;
	pair = DeeTuple_NewUninitializedPair();
	if unlikely(!pair)
		goto err;
	pair->t_elem[0] = key;
	pair->t_elem[1] = value;
	result = (*data->dfwme_cb)(data->dfwme_arg, (DeeObject *)pair);
	DeeTuple_DecrefSymbolic((DREF DeeObject *)pair);
	return result;
err:
	return -1;
}
#endif /* !DEFINED_default_foreach_with_map_enumerate_cb */
)]

%[define(DEFINE_default_foreach_with_seq_enumerate_cb =
#ifndef DEFINED_default_foreach_with_seq_enumerate_cb
#define DEFINED_default_foreach_with_seq_enumerate_cb
struct default_foreach_with_seq_enumerate_data {
	Dee_foreach_t dfwse_cb;  /* [1..1] Underlying callback */
	void         *dfwse_arg; /* [?..?] Cookie for `dfwse_cb' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_foreach_with_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	struct default_foreach_with_seq_enumerate_data *data;
	data = (struct default_foreach_with_seq_enumerate_data *)arg;
	(void)index;
	if unlikely(!value)
		return 0;
	return (*data->dfwse_cb)(data->dfwse_arg, value);
}
#endif /* !DEFINED_default_foreach_with_seq_enumerate_cb */
)]

/* accept_any_base_class_for_method_hint -- see comment on "seq_operator_iter" */
[[accept_any_base_class_for_method_hint]]
[[operator(Sequence: tp_seq->tp_foreach)]]
[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach([[nonnull]] DeeObject *__restrict self,
                                  [[nonnull]] Dee_foreach_t cb,
                                  void *arg)
%{$empty = 0}
%{$with__seq_enumerate = [[prefix(DEFINE_default_foreach_with_seq_enumerate_cb)]] {
	struct default_foreach_with_seq_enumerate_data data;
	data.dfwse_cb  = cb;
	data.dfwse_arg = arg;
	return CALL_DEPENDENCY(seq_enumerate, self, &default_foreach_with_seq_enumerate_cb, &data);
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_foreach_with_seq_enumerate_cb)]] {
	struct default_foreach_with_seq_enumerate_data data;
	data.dfwse_cb  = cb;
	data.dfwse_arg = arg;
	return CALL_DEPENDENCY(seq_enumerate_index, self,
	                       (Dee_ssize_t (DCALL *)(void *, size_t, DeeObject *))
	                       (Dee_funptr_t)&default_foreach_with_seq_enumerate_cb,
	                       &data, 0, (size_t)-1);
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_ssize_t temp, result = 0;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	tp_getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	ASSERT(tp_getitem_index_fast);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = (*tp_getitem_index_fast)(self, i);
		if unlikely(!elem)
			continue;
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	Dee_ssize_t temp, result = 0;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem == ITER_DONE)
				continue; /* Unbound item */
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	Dee_ssize_t temp, result = 0;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	if unlikely(size == (size_t)-1)
		goto err;
	for (i = 0; i < size; ++i) {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
err:
	return -1;
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	Dee_ssize_t temp, result = 0;
	DREF DeeObject *i, *size = CALL_DEPENDENCY(seq_operator_sizeob, self);
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	if unlikely(!size)
		goto err;
	i = DeeObject_NewDefault(Dee_TYPE(size));
	if unlikely(!i)
		goto err_size;
	for (;;) {
		DREF DeeObject *elem;
		int cmp_status;
		cmp_status = DeeObject_CmpLoAsBool(i, size);
		if unlikely(cmp_status < 0)
			goto err_size_i;
		if (!cmp_status)
			break;
		elem = CALL_DEPENDENCY(seq_operator_getitem, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break; /* In case the sequence's length got truncated since we checked above. */
			goto err_size_i;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0) {
			result = temp;
			break;
		}
		result += temp;
		if (DeeObject_Inc(&i))
			goto err_size_i;
	}
	Dee_Decref(i);
	Dee_Decref(size);
	return result;
err_size_i:
	Dee_Decref(i);
err_size:
	Dee_Decref(size);
err:
	return -1;
}}
%{using seq_operator_foreach_pair: [[prefix(DEFINE_default_foreach_with_foreach_pair_cb)]] {
	struct default_foreach_with_foreach_pair_data data;
	data.dfwfp_cb  = cb;
	data.dfwfp_arg = arg;
	return CALL_DEPENDENCY(seq_operator_foreach_pair, self, &default_foreach_with_foreach_pair_cb, &data);
}}
%{$with__seq_operator_getitem_index = {
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0;; ++i) {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return -1;
}}
%{$with__seq_operator_getitem = {
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	Dee_ssize_t temp, result = 0;
	DREF DeeIntObject *index = (DREF DeeIntObject *)DeeInt_NewZero();
	for (;;) {
		DREF DeeObject *elem;
		elem = CALL_DEPENDENCY(seq_operator_getitem, self, (DeeObject *)index);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			goto err_index;
		}
		temp = (*cb)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			return temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_index;
		if (int_inc(&index))
			goto err_index;
	}
	Dee_Decref(index);
	return result;
err_index:
	Dee_Decref(index);
/*err:*/
	return -1;
}}
%{using seq_operator_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_Foreach(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_foreach_with_map_enumerate_cb)]] {
	struct default_foreach_with_map_enumerate_data data;
	data.dfwme_cb  = cb;
	data.dfwme_arg = arg;
	return CALL_DEPENDENCY(map_enumerate, self, &default_foreach_with_map_enumerate_cb, &data);
}} = $with__seq_operator_iter;








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

/* accept_any_base_class_for_method_hint -- see comment on "seq_operator_iter" */
[[accept_any_base_class_for_method_hint]]
[[operator(Sequence: tp_seq->tp_foreach_pair)]]
[[wunused]] Dee_ssize_t
__seq_iter__.seq_operator_foreach_pair([[nonnull]] DeeObject *__restrict self,
                                       [[nonnull]] Dee_foreach_pair_t cb,
                                       void *arg)
%{$empty = 0}
%{using seq_operator_foreach: [[prefix(DEFINE_default_foreach_pair_with_foreach_cb)]] {
	struct default_foreach_pair_with_foreach_data data;
	data.dfpwf_cb  = cb;
	data.dfpwf_arg = arg;
	return CALL_DEPENDENCY(seq_operator_foreach, self, &default_foreach_pair_with_foreach_cb, &data);
}}
%{using seq_operator_iter: {
	Dee_ssize_t result;
	DREF DeeObject *iter;
	iter = CALL_DEPENDENCY(seq_operator_iter, self);
	if unlikely(!iter)
		goto err;
	result = DeeIterator_ForeachPair(iter, cb, arg);
	Dee_Decref_likely(iter);
	return result;
err:
	return -1;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_foreach_pair_with_map_enumerate_cb)]] {
	struct default_foreach_pair_with_map_enumerate_data data;
	data.dfpwme_cb  = cb;
	data.dfpwme_arg = arg;
	return CALL_DEPENDENCY(map_enumerate, self, &default_foreach_pair_with_map_enumerate_cb, &data);
}} = $with__seq_operator_foreach;




seq_operator_iter = {
	DeeMH_map_operator_iter_t map_operator_iter;
	DeeMH_set_operator_iter_t set_operator_iter;
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE_NODEFAULT(seq_operator_size);
	if (seq_operator_size) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &$empty;
with_seq_operator_size:
		if (THIS_TYPE->tp_seq && THIS_TYPE->tp_seq->tp_getitem_index_fast)
			return &$with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = REQUIRE_NODEFAULT(seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index) {
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
				return &$with__seq_operator_size__and__seq_operator_getitem_index;
			return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
		if (REQUIRE_NODEFAULT(seq_operator_getitem_index))
			return &$with__seq_operator_size__and__seq_operator_getitem_index;
		if (REQUIRE_NODEFAULT(seq_operator_getitem) || REQUIRE_NODEFAULT(seq_operator_trygetitem))
			return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	} else {
		DeeMH_seq_operator_sizeob_t seq_operator_sizeob = REQUIRE_NODEFAULT(seq_operator_sizeob);
		if (seq_operator_sizeob) {
			if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
				return &$empty;
			goto with_seq_operator_size;
		}
		if (REQUIRE_NODEFAULT(seq_operator_getitem_index) ||
		    REQUIRE_NODEFAULT(seq_operator_trygetitem_index))
			return &$with__seq_operator_getitem_index;
		if (REQUIRE_NODEFAULT(seq_operator_getitem) ||
		    REQUIRE_NODEFAULT(seq_operator_trygetitem))
			return &$with__seq_operator_getitem;
	}

	/* If provided, can also use explicitly defined "Set.operator iter()" */
	set_operator_iter = REQUIRE_NODEFAULT(set_operator_iter);
	if (set_operator_iter)
		return set_operator_iter;

	/* If provided, can also use explicitly defined "Mapping.operator iter()" */
	map_operator_iter = REQUIRE_NODEFAULT(map_operator_iter);
	if (map_operator_iter)
		return map_operator_iter;

	if (REQUIRE_NODEFAULT(map_iterkeys) || REQUIRE_NODEFAULT(map_keys)) {
		if (REQUIRE_NODEFAULT(map_operator_trygetitem))
			return &$with__map_iterkeys__and__map_operator_trygetitem;
		if (REQUIRE_NODEFAULT(map_operator_getitem))
			return &$with__map_iterkeys__and__map_operator_getitem;
	}
	if (REQUIRE_NODEFAULT(seq_enumerate_index) ||
	    REQUIRE_NODEFAULT(seq_operator_foreach) ||
	    REQUIRE_NODEFAULT(seq_operator_foreach_pair))
		return &$with__seq_enumerate_index;
	if (REQUIRE_NODEFAULT(seq_enumerate))
		return &$with__seq_enumerate;
	if (REQUIRE_NODEFAULT(map_enumerate) || REQUIRE_NODEFAULT(map_enumerate_range))
		return &$with__map_enumerate;
};


seq_operator_foreach = {
	DeeMH_seq_operator_iter_t seq_operator_iter;
	/*if (REQUIRE_NODEFAULT(seq_operator_foreach_pair))
		return &$with__seq_operator_foreach_pair;*/
	if (REQUIRE_NODEFAULT(map_operator_foreach_pair))
		return &$with__seq_operator_foreach_pair;
	seq_operator_iter = REQUIRE(seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &$empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate)
		return &$with__seq_enumerate;
	if (seq_operator_iter) {
		if (seq_operator_iter == REQUIRE_NODEFAULT(set_operator_iter)) {
			DeeMH_set_operator_foreach_t set_operator_foreach = REQUIRE_NODEFAULT(set_operator_foreach);
			if (set_operator_foreach)
				return set_operator_foreach;
		}
		return &$with__seq_operator_iter;
	}
};

seq_operator_foreach_pair = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = REQUIRE_NODEFAULT(map_operator_foreach_pair);
	if (map_operator_foreach_pair)
		return map_operator_foreach_pair;
	seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &$with__seq_operator_iter;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};
