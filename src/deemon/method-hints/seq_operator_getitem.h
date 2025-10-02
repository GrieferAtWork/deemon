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
/* deemon.Sequence.operator []()                                        */
/************************************************************************/
__seq_getitem__(index:?Dint)->?O {
	return CALL_DEPENDENCY(seq_operator_getitem, self, index);
}





[[operator(Sequence: tp_seq->tp_getitem)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_getitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator []"))}
%{$none = return_none}
%{$empty = {
	err_index_out_of_bounds_ob(self, index);
	return NULL;
}}
%{using seq_operator_getitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_getitem_index, self, index_value);
err:
	return NULL;
}}
%{$with__seq_enumerate = [[prefix(DEFINE_default_map_getitem_with_map_enumerate_cb)]] {
	struct default_map_getitem_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dmgiwme_key = index;
	status = CALL_DEPENDENCY(seq_enumerate, self, &default_map_getitem_with_map_enumerate_cb, &data);
	if likely(status == -2)
		return data.dmgiwme_result;
	if unlikely(status == -3) {
		DeeRT_ErrUnboundIndexObj(self, index);
		goto err;
	}
	ASSERT(status == -1 || status == 0);
	if (status < 0)
		goto err;
	err_index_out_of_bounds_ob(self, index);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &index);
}


%[define(DEFINE_default_seq_getitem_index_with_foreach =
#ifndef DEFINED_default_seq_getitem_index_with_foreach
#define DEFINED_default_seq_getitem_index_with_foreach
struct default_seq_getitem_index_with_foreach_data {
	DREF DeeObject *dsgiiwfd_result; /* [?..1][out] Item lookup result */
	size_t          dsgiiwfd_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_getitem_index_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_seq_getitem_index_with_foreach_data *data;
	data = (struct default_seq_getitem_index_with_foreach_data *)arg;
	if (data->dsgiiwfd_nskip == 0) {
		Dee_Incref(elem);
		data->dsgiiwfd_result = elem; /* Inherit reference */
		return -2;                   /* Stop enumeration */
	}
	--data->dsgiiwfd_nskip;
	return 0;
}
#endif /* !DEFINED_default_seq_getitem_index_with_foreach */
)]


%[define(DEFINE_default_seq_getitem_index_with_map_enumerate =
#ifndef DEFINED_default_seq_getitem_index_with_map_enumerate
#define DEFINED_default_seq_getitem_index_with_map_enumerate
struct default_seq_getitem_index_with_map_enumerate_data {
	DREF DeeObject *dsgiiwme_result; /* [0..1][out] Item lookup result */
	size_t          dsgiiwme_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_getitem_index_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_seq_getitem_index_with_map_enumerate_data *data;
	data = (struct default_seq_getitem_index_with_map_enumerate_data *)arg;
	if (data->dsgiiwme_nskip == 0) {
		if (value) {
			DREF DeeTupleObject *pair;
			pair = DeeTuple_NewUninitializedPair();
			if unlikely(!pair)
				goto err;
			Dee_Incref(key);
			pair->t_elem[0] = key;                         /* Inherit reference */
			Dee_Incref(value);
			pair->t_elem[1] = value;                       /* Inherit reference */
			data->dsgiiwme_result = (DREF DeeObject *)pair; /* Inherit reference */
		} else {
			data->dsgiiwme_result = NULL;
		}
		return -2; /* Stop enumeration */
	}
	--data->dsgiiwme_nskip;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_seq_getitem_index_with_map_enumerate */
)]



%[define(DEFINE_default_seq_getitem_index_with_seq_enumerate_index =
#ifndef DEFINED_default_seq_getitem_index_with_seq_enumerate_index
#define DEFINED_default_seq_getitem_index_with_seq_enumerate_index
struct default_seq_getitem_index_with_seq_enumerate_index_data {
	DREF DeeObject *dsgiiwsei_result;    /* [0..1][out] Item lookup result */
	size_t          dsgiiwsei_index;     /* Intended index */
	size_t          dsgiiwsei_lastindex; /* Last encountered index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_seq_getitem_index_with_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	struct default_seq_getitem_index_with_seq_enumerate_index_data *data;
	data = (struct default_seq_getitem_index_with_seq_enumerate_index_data *)arg;
	if (data->dsgiiwsei_index == index) {
		data->dsgiiwsei_result = value;
		Dee_XIncref(value);
		return -2;
	} else {
		data->dsgiiwsei_lastindex = index;
	}
	return 0;
}
#endif /* !DEFINED_default_seq_getitem_index_with_seq_enumerate_index */
)]



[[operator(Sequence: tp_seq->tp_getitem_index)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_getitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator []"))}
%{$none = return_none}
%{$empty = {
	err_index_out_of_bounds(self, index, 0);
	return NULL;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_seq_getitem_index_with_foreach)]] {
	struct default_seq_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dsgiiwfd_nskip = index;
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dsgiiwfd_result;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dsgiiwfd_nskip);
err:
	return NULL;
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	size_t size;
	DREF DeeObject *result;
	result = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, index);
	if likely(ITER_ISOK(result))
		return result;
	if unlikely(!result)
		goto err;
	size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_bad_bounds;
	DeeRT_ErrUnboundIndex(self, index);
	goto err;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, size);
err:
	return NULL;
}}
%{using seq_operator_getitem: {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(seq_operator_getitem, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_seq_getitem_index_with_map_enumerate)]] {
	struct default_seq_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dsgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_seq_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dsgiiwme_result)
		goto err_unbound;
	return data.dsgiiwme_result;
err_unbound:
	DeeRT_ErrUnboundIndex(self, index);
	goto err;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dsgiiwme_nskip);
err:
	return NULL;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_getitem_index_with_seq_enumerate_index)]] {
	Dee_ssize_t status;
	struct default_seq_getitem_index_with_seq_enumerate_index_data data;
	data.dsgiiwsei_index     = index;
	data.dsgiiwsei_lastindex = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                         &default_seq_getitem_index_with_seq_enumerate_index_cb,
	                         &data, index, (size_t)-1);
	if likely(status == -2) {
		if unlikely(!data.dsgiiwsei_result)
			DeeRT_ErrUnboundIndex(self, index);
		return data.dsgiiwsei_result;
	}
	if likely(status == 0)
		err_index_out_of_bounds(self, index, data.dsgiiwsei_lastindex + 1);
	return NULL;
}} = $with__seq_operator_getitem;





seq_operator_getitem = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index) {
		if (REQUIRE_NODEFAULT(seq_enumerate))
			return &$with__seq_enumerate;
	}
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};

seq_operator_getitem_index = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem)
		return &$with__seq_operator_getitem;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_foreach) {
		/* Using "seq_operator_foreach" works, but is inefficient -> try to use other operators. */
		if (REQUIRE_NODEFAULT(seq_operator_getitem))
			return &$with__seq_operator_getitem;
		return &$with__seq_operator_foreach;
	}
};






[[operator(Sequence: tp_seq->tp_trygetitem)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_trygetitem([[nonnull]] DeeObject *self,
                                        [[nonnull]] DeeObject *index)
%{unsupported_alias("default__seq_operator_getitem__unsupported")}
%{$none = return_none}
%{$empty = ITER_DONE}
%{using seq_operator_getitem: {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_getitem, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{using seq_operator_trygetitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_trygetitem_index, self, index_value);
err:
	return NULL;
}}
%{$with__seq_enumerate = [[prefix(DEFINE_default_map_getitem_with_map_enumerate_cb)]] {
	struct default_map_getitem_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dmgiwme_key = index;
	status = CALL_DEPENDENCY(seq_enumerate, self, &default_map_getitem_with_map_enumerate_cb, &data);
	if likely(status == -2)
		return data.dmgiwme_result;
	ASSERT(status == -3 || status == -1 || status == 0);
	if unlikely(status == -1)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}} = $with__seq_operator_getitem;

seq_operator_trygetitem = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_enumerate_index) {
		if (REQUIRE_NODEFAULT(seq_enumerate))
			return &$with__seq_enumerate;
	}
	if (seq_operator_trygetitem_index)
		return &$with__seq_operator_trygetitem_index;
};





[[operator(Sequence: tp_seq->tp_trygetitem_index)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_trygetitem_index([[nonnull]] DeeObject *__restrict self,
                                              size_t index)
%{unsupported_alias("default__seq_operator_getitem_index__unsupported")}
%{$none = return_none}
%{$empty = ITER_DONE}
%{using seq_operator_getitem_index: {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_default_seq_getitem_index_with_foreach)]] {
	struct default_seq_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dsgiiwfd_nskip = index;
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dsgiiwfd_result;
err_bad_bounds:
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_seq_getitem_index_with_map_enumerate)]] {
	struct default_seq_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dsgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_seq_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dsgiiwme_result)
		goto err_unbound;
	return data.dsgiiwme_result;
err_unbound:
err_bad_bounds:
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_seq_getitem_index_with_seq_enumerate_index)]] {
	size_t end_index;
	Dee_ssize_t status;
	struct default_seq_getitem_index_with_seq_enumerate_index_data data;
	data.dsgiiwsei_index     = index;
	data.dsgiiwsei_lastindex = (size_t)-1;
	if (OVERFLOW_UADD(index, 1, &end_index))
		end_index = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                         &default_seq_getitem_index_with_seq_enumerate_index_cb,
	                         &data, index, end_index);
	if likely(status == -2) {
		if unlikely(!data.dsgiiwsei_result)
			return ITER_DONE;
		return data.dsgiiwsei_result;
	}
	if likely(status == 0)
		return ITER_DONE;
	return NULL;
}} = $with__seq_operator_getitem_index;

seq_operator_trygetitem_index = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
		return &$with__seq_operator_foreach;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};





%[define(DEFINE_default_bounditem_with_seq_enumerate =
#ifndef DEFINED_default_bounditem_with_seq_enumerate
#define DEFINED_default_bounditem_with_seq_enumerate
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_bounditem_with_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	int cmp;
	(void)value;
	cmp = DeeObject_TryCompareEq((DeeObject *)arg, index);
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	if (cmp == 0)
		return value ? -3 : -2;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_bounditem_with_seq_enumerate */
)]



[[operator(Sequence: tp_seq->tp_hasitem)]]
[[wunused]] int
__seq_getitem__.seq_operator_hasitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator []"))}
%{$none = 1}
%{$empty = 0}
%{$with__seq_operator_sizeob = {
	int result;
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	result = DeeObject_CmpLoAsBool(index, sizeob);
	Dee_Decref(sizeob);
	return result;
err:
	return -1;
}}
%{using seq_operator_hasitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_hasitem_index, self, index_value);
err:
	return -1;
}}
%{using seq_operator_getitem: {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem, self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}}
%{$with__seq_enumerate = [[prefix(DEFINE_default_bounditem_with_seq_enumerate)]] {
	Dee_ssize_t status = CALL_DEPENDENCY(seq_enumerate, self,
	                                     &default_bounditem_with_seq_enumerate_cb,
	                                     (void *)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3 || status == -2)
		return 1;
	return (int)status; /* 0 (index doesn't exist) or -1 (error) */
}} = $with__seq_operator_getitem;

seq_operator_hasitem = {
	DeeMH_seq_operator_hasitem_index_t seq_operator_hasitem_index = REQUIRE(seq_operator_hasitem_index);
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__empty)
		return &$empty;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_size) {
		DeeMH_seq_operator_sizeob_t seq_operator_sizeob = REQUIRE(seq_operator_sizeob);
		if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
			return &$empty;
		if (seq_operator_sizeob == &default__seq_operator_sizeob__with__seq_operator_size)
			return &$with__seq_operator_hasitem_index; /* This way, sizeob isn't called, and no int-object gets created */
		return &$with__seq_operator_sizeob;
	}
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_enumerate_index) {
		if (REQUIRE_NODEFAULT(seq_enumerate))
			return &$with__seq_enumerate;
	}
	if (seq_operator_hasitem_index)
		return &$with__seq_operator_hasitem_index;
};



%[define(DEFINE_default_bounditem_index_with_seq_enumerate_index =
#ifndef DEFINED_default_bounditem_index_with_seq_enumerate_index
#define DEFINED_default_bounditem_index_with_seq_enumerate_index
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
default_bounditem_index_with_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	if ((size_t)(uintptr_t)arg == index)
		return value ? -4 : -3;
	return -2;
}
#endif /* !DEFINED_default_bounditem_index_with_seq_enumerate_index */
)]



[[operator(Sequence: tp_seq->tp_hasitem_index)]]
[[wunused]] int
__seq_getitem__.seq_operator_hasitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator []"))}
%{$none = 1}
%{$empty = 0}
%{$with__seq_operator_size = {
	size_t seqsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return index < seqsize ? 1 : 0;
err:
	return -1;
}}
%{using seq_operator_getitem_index: {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_bounditem_index_with_seq_enumerate_index)]] {
	size_t end_index;
	Dee_ssize_t status;
	if (OVERFLOW_UADD(index, 1, &end_index))
		end_index = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                         &default_bounditem_index_with_seq_enumerate_index_cb,
	                         (void *)(uintptr_t)index, index, end_index);
	ASSERT(status == -4 || status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -2)
		return 0;
	if (status == -4 || status == -3)
		return 1;
	return (int)status; /* 0 (empty; aka: index doesn't exist) or -1 (error) */
}} = $with__seq_operator_getitem_index;

seq_operator_hasitem_index = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE(seq_operator_size);
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	if (DeeType_HasTraitHint(THIS_TYPE, __seq_getitem_always_bound__)) {
		if (REQUIRE_ANY(seq_operator_size) != &default__seq_operator_size__unsupported)
			return &$with__seq_operator_size;
	}
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
	    seq_operator_size != &default__seq_operator_size__with__seq_operator_iter)
		return &$with__seq_operator_size;
	if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};




[[operator(Sequence: tp_seq->tp_bounditem)]]
[[wunused]] int
__seq_getitem__.seq_operator_bounditem([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *index)
%{unsupported({
	err_seq_unsupportedf(self, "operator [](%r)", index);
	return Dee_BOUND_ERR;
})}
%{$none = Dee_BOUND_YES}
%{$empty = Dee_BOUND_MISSING}
%{using seq_operator_bounditem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_bounditem_index, self, index_value);
err:
	return Dee_BOUND_ERR;
}}
%{using seq_operator_getitem: {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{$with__seq_enumerate = [[prefix(DEFINE_default_bounditem_with_seq_enumerate)]] {
	Dee_ssize_t status = CALL_DEPENDENCY(seq_enumerate, self,
	                                     &default_bounditem_with_seq_enumerate_cb,
	                                     (void *)index);
	ASSERT(status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -3)
		return Dee_BOUND_YES;
	if (status == -2)
		return Dee_BOUND_NO;
	if (status == -1)
		return Dee_BOUND_ERR;
	return Dee_BOUND_MISSING;
}} = $with__seq_operator_getitem;

seq_operator_bounditem = {
	DeeMH_seq_operator_bounditem_index_t seq_operator_bounditem_index;
	if (DeeType_HasTraitHint(THIS_TYPE, __seq_getitem_always_bound__)) {
		/* TODO: Optimizations */
	}
	seq_operator_bounditem_index = REQUIRE(seq_operator_bounditem_index);
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__empty)
		return &$empty;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_enumerate_index) {
		if (REQUIRE_NODEFAULT(seq_enumerate))
			return &$with__seq_enumerate;
	}
	if (seq_operator_bounditem_index)
		return &$with__seq_operator_bounditem_index;
};



[[operator(Sequence: tp_seq->tp_bounditem_index)]]
[[wunused]] int
__seq_getitem__.seq_operator_bounditem_index([[nonnull]] DeeObject *__restrict self,
                                             size_t index)
%{unsupported({
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return Dee_BOUND_ERR;
})}
%{$none = Dee_BOUND_YES}
%{$empty = Dee_BOUND_MISSING}
%{using seq_operator_getitem_index: {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_seq_getitem_index_with_map_enumerate)]] {
	struct default_seq_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dsgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_seq_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dsgiiwme_result)
		goto err_unbound;
	Dee_Decref(data.dsgiiwme_result);
	return Dee_BOUND_YES;
err_unbound:
	return Dee_BOUND_NO;
err_bad_bounds:
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}}
%{$with__seq_enumerate_index = [[prefix(DEFINE_default_bounditem_index_with_seq_enumerate_index)]] {
	size_t end_index;
	Dee_ssize_t status;
	if (OVERFLOW_UADD(index, 1, &end_index))
		end_index = (size_t)-1;
	status = CALL_DEPENDENCY(seq_enumerate_index, self,
	                         &default_bounditem_index_with_seq_enumerate_index_cb,
	                         (void *)(uintptr_t)index, index, end_index);
	ASSERT(status == -4 || status == -3 || status == -2 || status == -1 || status == 0);
	if (status == -1)
		return Dee_BOUND_ERR;
	if (status == -3)
		return Dee_BOUND_NO;
	if (status == -4)
		return Dee_BOUND_YES;
	ASSERT(status == -2 || status == 0);
	return Dee_BOUND_MISSING;
}} = $with__seq_operator_getitem_index;

seq_operator_bounditem_index = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	if (DeeType_HasTraitHint(THIS_TYPE, __seq_getitem_always_bound__)) {
		/* TODO: Optimizations */
	}
	seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &$with__seq_enumerate_index;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};

