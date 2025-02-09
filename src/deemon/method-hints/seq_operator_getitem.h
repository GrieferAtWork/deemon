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
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__seq_getitem__", &index))
		goto err;
	return CALL_DEPENDENCY(seq_operator_getitem, self, index);
err:
	return NULL;
}





[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_getitem)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_getitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_index_out_of_bounds_ob(self, index);
	return NULL;
}}
%{$with__seq_operator_getitem_index = {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_getitem_index, self, index_value);
err:
	return NULL;
}} {
	return LOCAL_CALLATTR(self, 1, &index);
}


%[define(DEFINE_default_getitem_index_with_foreach =
#ifndef DEFINED_default_getitem_index_with_foreach
#define DEFINED_default_getitem_index_with_foreach
struct default_getitem_index_with_foreach_data {
	DREF DeeObject *dgiiwfd_result; /* [?..1][out] Item lookup result */
	size_t          dgiiwfd_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_getitem_index_with_foreach_data *data;
	data = (struct default_getitem_index_with_foreach_data *)arg;
	if (data->dgiiwfd_nskip == 0) {
		Dee_Incref(elem);
		data->dgiiwfd_result = elem; /* Inherit reference */
		return -2;                   /* Stop enumeration */
	}
	--data->dgiiwfd_nskip;
	return 0;
}
#endif /* !DEFINED_default_getitem_index_with_foreach */
)]


%[define(DEFINE_default_getitem_index_with_map_enumerate =
#ifndef DEFINED_default_getitem_index_with_map_enumerate
#define DEFINED_default_getitem_index_with_map_enumerate
struct default_getitem_index_with_map_enumerate_data {
	DREF DeeObject *dgiiwme_result; /* [0..1][out] Item lookup result */
	size_t          dgiiwme_nskip;  /* Number of indices left to skip. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_getitem_index_with_map_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_getitem_index_with_map_enumerate_data *data;
	data = (struct default_getitem_index_with_map_enumerate_data *)arg;
	if (data->dgiiwme_nskip == 0) {
		if (value) {
			DREF DeeTupleObject *pair;
			pair = DeeTuple_NewUninitializedPair();
			if unlikely(!pair)
				goto err;
			Dee_Incref(key);
			pair->t_elem[0] = key;                         /* Inherit reference */
			Dee_Incref(value);
			pair->t_elem[1] = value;                       /* Inherit reference */
			data->dgiiwme_result = (DREF DeeObject *)pair; /* Inherit reference */
		} else {
			data->dgiiwme_result = NULL;
		}
		return -2; /* Stop enumeration */
	}
	--data->dgiiwme_nskip;
	return 0;
err:
	return -1;
}
#endif /* !DEFINED_default_getitem_index_with_map_enumerate */
)]



[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_getitem_index)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_getitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator []"))}
%{$empty = {
	err_index_out_of_bounds(self, index, 0);
	return NULL;
}}
%{$with__seq_operator_foreach = [[prefix(DEFINE_default_getitem_index_with_foreach)]] {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dgiiwfd_result;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dgiiwfd_nskip);
err:
	return NULL;
}}
%{$with__seq_operator_getitem = {
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
%{$with__map_enumerate = [[prefix(DEFINE_default_getitem_index_with_map_enumerate)]] {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	return data.dgiiwme_result;
err_unbound:
	err_unbound_index(self, index);
	goto err;
err_bad_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, index - data.dgiiwme_nskip);
err:
	return NULL;
}} = $with__seq_operator_getitem;





seq_operator_getitem = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};

seq_operator_getitem_index = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach)
		return &$with__seq_operator_foreach;
};






[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_trygetitem)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_trygetitem([[nonnull]] DeeObject *self,
                                        [[nonnull]] DeeObject *index)
%{unsupported_alias("default__seq_operator_getitem__unsupported")}
%{$empty = ITER_DONE}
%{$with__seq_operator_getitem = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_getitem, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{$with__seq_operator_trygetitem_index = {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_trygetitem_index, self, index_value);
err:
	return NULL;
}} = $with__seq_operator_getitem;

seq_operator_trygetitem = {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = REQUIRE(seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &$empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_trygetitem_index)
		return &$with__seq_operator_trygetitem_index;
};





[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_trygetitem_index)]]
[[wunused]] DREF DeeObject *
__seq_getitem__.seq_operator_trygetitem_index([[nonnull]] DeeObject *__restrict self,
                                              size_t index)
%{unsupported_alias("default__seq_operator_getitem_index__unsupported")}
%{$empty = ITER_DONE}
%{$with__seq_operator_getitem_index = {
	DREF DeeObject *result = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_default_getitem_index_with_foreach)]] {
	struct default_getitem_index_with_foreach_data data;
	Dee_ssize_t status;
	data.dgiiwfd_nskip = index;
	status = CALL_DEPENDENCY(seq_operator_foreach, self, &default_getitem_index_with_foreach_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	return data.dgiiwfd_result;
err_bad_bounds:
	return ITER_DONE;
err:
	return NULL;
}}
%{$with__map_enumerate = [[prefix(DEFINE_default_getitem_index_with_map_enumerate)]] {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	return data.dgiiwme_result;
err_unbound:
err_bad_bounds:
	return ITER_DONE;
err:
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
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};





[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_hasitem)]]
[[wunused]] int
__seq_getitem__.seq_operator_hasitem([[nonnull]] DeeObject *self,
                                     [[nonnull]] DeeObject *index)
%{unsupported(auto("operator []"))}
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
%{$with__seq_operator_hasitem_index = {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_hasitem_index, self, index_value);
err:
	return -1;
}}
%{$with__seq_operator_getitem = {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem, self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}} = $with__seq_operator_getitem;

seq_operator_hasitem = {
	DeeMH_seq_operator_hasitem_index_t seq_operator_hasitem_index = REQUIRE(seq_operator_hasitem_index);
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__empty)
		return &$empty;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_size)
		return $with__seq_operator_sizeob;
	if (seq_operator_hasitem_index)
		return &$with__seq_operator_hasitem_index;
};



[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_hasitem_index)]]
[[wunused]] int
__seq_getitem__.seq_operator_hasitem_index([[nonnull]] DeeObject *__restrict self,
                                           size_t index)
%{unsupported(auto("operator []"))}
%{$empty = 0}
%{$with__seq_operator_size = {
	size_t seqsize = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return index < seqsize ? 1 : 0;
err:
	return -1;
}}
%{$with__seq_operator_getitem_index = {
	DREF DeeObject *value = CALL_DEPENDENCY(seq_operator_getitem_index, self, index);
	if (value) {
		Dee_Decref(value);
		return 1;
	}
	if (DeeError_Catch(&DeeError_IndexError))
		return 0;
	return -1;
}} = $with__seq_operator_getitem_index;

seq_operator_hasitem_index = {
	DeeMH_seq_operator_size_t seq_operator_size = REQUIRE(seq_operator_size);
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &$empty;
	if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach)
		return &$with__seq_operator_size;
	seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};




[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_bounditem)]]
[[wunused]] int
__seq_getitem__.seq_operator_bounditem([[nonnull]] DeeObject *self,
                                       [[nonnull]] DeeObject *index)
%{unsupported({
	err_seq_unsupportedf(self, "operator [](%r)", index);
	return Dee_BOUND_ERR;
})}
%{$empty = 0}
%{$with__seq_operator_bounditem_index = {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(seq_operator_bounditem_index, self, index_value);
err:
	return Dee_BOUND_ERR;
}}
%{$with__seq_operator_getitem = {
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
}} = $with__seq_operator_getitem;

seq_operator_bounditem = {
	DeeMH_seq_operator_bounditem_index_t seq_operator_bounditem_index = REQUIRE(seq_operator_bounditem_index);
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__empty)
		return &$empty;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
		return &$with__seq_operator_getitem;
	if (seq_operator_bounditem_index)
		return &$with__seq_operator_bounditem_index;
};



[[operator(Sequence.OPERATOR_GETITEM: tp_seq->tp_bounditem_index)]]
[[wunused]] int
__seq_getitem__.seq_operator_bounditem_index([[nonnull]] DeeObject *__restrict self,
                                             size_t index)
%{unsupported({
	err_seq_unsupportedf(self, "operator [](%" PRFuSIZ ")", index);
	return Dee_BOUND_ERR;
})}
%{$empty = 0}
%{$with__seq_operator_getitem_index = {
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
%{$with__map_enumerate = [[prefix(DEFINE_default_getitem_index_with_map_enumerate)]] {
	struct default_getitem_index_with_map_enumerate_data data;
	Dee_ssize_t status;
	data.dgiiwme_nskip = index;
	status = CALL_DEPENDENCY(map_enumerate, self, &default_getitem_index_with_map_enumerate_cb, &data);
	if unlikely(status != -2) {
		if (status == 0)
			goto err_bad_bounds;
		ASSERT(status == -1);
		goto err;
	}
	if unlikely(!data.dgiiwme_result)
		goto err_unbound;
	Dee_Decref(data.dgiiwme_result);
	return Dee_BOUND_YES;
err_unbound:
	return Dee_BOUND_NO;
err_bad_bounds:
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}} = $with__seq_operator_getitem_index;

seq_operator_bounditem_index = {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = REQUIRE(seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &$empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &$with__map_enumerate;
	if (seq_operator_getitem_index)
		return &$with__seq_operator_getitem_index;
};

