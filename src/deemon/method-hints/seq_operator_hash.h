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
/* deemon.Sequence.operator hash()                                      */
/************************************************************************/
__seq_hash__()->?Dint {
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__seq_hash__"))
		goto err;
	result = CALL_DEPENDENCY(seq_operator_hash, self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

%[define(DEFINE_default_seq_hash_with_foreach_cb =
#ifndef DEFINED_default_seq_hash_with_foreach_cb
#define DEFINED_default_seq_hash_with_foreach_cb
struct default_seq_hash_with_foreach_data {
	Dee_hash_t sqhwf_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwf_nonempty=false) */
	bool       sqhwf_nonempty; /* True after the first element */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_cb(void *arg, DeeObject *elem) {
	struct default_seq_hash_with_foreach_data *data;
	Dee_hash_t elem_hash;
	data = (struct default_seq_hash_with_foreach_data *)arg;
	elem_hash = DeeObject_Hash(elem);
	if (data->sqhwf_nonempty) {
		data->sqhwf_result = Dee_HashCombine(data->sqhwf_result, elem_hash);
	} else {
		data->sqhwf_result = elem_hash;
		data->sqhwf_nonempty = true;
	}
	return 0;
}
#endif /* !DEFINED_default_seq_hash_with_foreach_cb */
)]

%[define(DEFINE_default_seq_hash_with_foreach_pair_cb =
#ifndef DEFINED_default_seq_hash_with_foreach_pair_cb
#define DEFINED_default_seq_hash_with_foreach_pair_cb
struct default_seq_hash_with_foreach_pair_data {
	Dee_hash_t sqhwfp_result;   /* Hash result (or DEE_HASHOF_EMPTY_SEQUENCE when sqhwfp_nonempty=false) */
	bool       sqhwfp_nonempty; /* True after the first element */
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
default_seq_hash_with_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct default_seq_hash_with_foreach_pair_data *data;
	Dee_hash_t elem_hash;
	data = (struct default_seq_hash_with_foreach_pair_data *)arg;
	elem_hash = Dee_HashCombine(DeeObject_Hash(key), DeeObject_Hash(value));
	if (data->sqhwfp_nonempty) {
		data->sqhwfp_result = Dee_HashCombine(data->sqhwfp_result, elem_hash);
	} else {
		data->sqhwfp_result = elem_hash;
		data->sqhwfp_nonempty = true;
	}
	return 0;
}
#endif /* !DEFINED_default_seq_hash_with_foreach_pair_cb */
)]

%[define(DEFINE_seq_handle_hash_error =
#ifndef DEFINED_seq_handle_hash_error
#define DEFINED_seq_handle_hash_error
PRIVATE NONNULL((1)) Dee_hash_t DCALL
seq_handle_hash_error(DeeObject *self) {
	DeeError_Print("Unhandled error in `Sequence.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}
#endif /* !DEFINED_seq_handle_hash_error */
)]


[[operator(Sequence: tp_cmp->tp_hash)]]
[[wunused]] Dee_hash_t
__seq_hash__.seq_operator_hash([[nonnull]] DeeObject *__restrict self)
%{unsupported({
	return DeeObject_HashGeneric(self);
})}
%{$empty = DEE_HASHOF_EMPTY_SEQUENCE}
%{$with__seq_operator_foreach =
[[prefix(DEFINE_default_seq_hash_with_foreach_cb)]]
[[prefix(DEFINE_seq_handle_hash_error)]] {
	struct default_seq_hash_with_foreach_data data;
	data.sqhwf_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwf_nonempty = false;
	if unlikely(CALL_DEPENDENCY(seq_operator_foreach, self, &default_seq_hash_with_foreach_cb, &data))
		goto err;
	return data.sqhwf_result;
err:
	return seq_handle_hash_error(self);
}}
%{$with__seq_operator_foreach_pair =
[[prefix(DEFINE_default_seq_hash_with_foreach_pair_cb)]]
[[prefix(DEFINE_seq_handle_hash_error)]] {
	struct default_seq_hash_with_foreach_pair_data data;
	data.sqhwfp_result   = DEE_HASHOF_EMPTY_SEQUENCE;
	data.sqhwfp_nonempty = false;
	if unlikely(CALL_DEPENDENCY(seq_operator_foreach_pair, self, &default_seq_hash_with_foreach_pair_cb, &data))
		goto err;
	return data.sqhwfp_result;
err:
	return seq_handle_hash_error(self);
}}
%{$with__seq_operator_size__and__operator_getitem_index_fast =
[[inherit_as($with__seq_operator_size__and__seq_operator_trygetitem_index)]] {
	DeeNO_getitem_index_fast_t tp_getitem_index_fast;
	Dee_hash_t result;
	DREF DeeObject *elem;
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	tp_getitem_index_fast = THIS_TYPE->tp_seq->tp_getitem_index_fast;
	elem = (*tp_getitem_index_fast)(self, 0);
	if unlikely(!elem) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = (*tp_getitem_index_fast)(self, i);
		if unlikely(!elem) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
	}
	return result;
err:
	return seq_handle_hash_error(self);
}}
%{$with__seq_operator_size__and__seq_operator_trygetitem_index = {
	Dee_hash_t result;
	DREF DeeObject *elem;
	PRELOAD_DEPENDENCY(seq_operator_trygetitem_index)
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, 0);
	if unlikely(!elem)
		goto err;
	if unlikely(elem == ITER_DONE) {
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = CALL_DEPENDENCY(seq_operator_trygetitem_index, self, i);
		if unlikely(!elem)
			goto err;
		if unlikely(elem == ITER_DONE) {
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return seq_handle_hash_error(self);
}}
%{$with__seq_operator_size__and__seq_operator_getitem_index = {
	Dee_hash_t result;
	DREF DeeObject *elem;
	PRELOAD_DEPENDENCY(seq_operator_getitem_index)
	size_t i, size = CALL_DEPENDENCY(seq_operator_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size == 0)
		return DEE_HASHOF_EMPTY_SEQUENCE;
	elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, 0);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (i = 1; i < size; ++i) {
		Dee_hash_t elem_hash;
		elem = CALL_DEPENDENCY(seq_operator_getitem_index, self, i);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return seq_handle_hash_error(self);
}}
%{$with__seq_operator_sizeob__and__seq_operator_getitem = {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *indexob, *elem;
	PRELOAD_DEPENDENCY(seq_operator_getitem)
	DREF DeeObject *sizeob = CALL_DEPENDENCY(seq_operator_sizeob, self);
	if unlikely(!sizeob)
		goto err;
	indexob = DeeObject_NewDefault(Dee_TYPE(sizeob));
	if unlikely(!indexob)
		goto err_sizeob;
	temp = DeeObject_CmpLoAsBool(indexob, sizeob);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err_sizeob_indexob;
		Dee_Decref(indexob);
		Dee_Decref(sizeob);
		return DEE_HASHOF_EMPTY_SEQUENCE;
	}
	elem = CALL_DEPENDENCY(seq_operator_getitem, self, indexob);
	if unlikely(!elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_sizeob_indexob;
		result = DEE_HASHOF_UNBOUND_ITEM;
	} else {
		result = DeeObject_Hash(elem);
		Dee_Decref(elem);
	}
	for (;;) {
		Dee_hash_t elem_hash;
		if (DeeObject_Inc(&indexob))
			goto err_sizeob_indexob;
		temp = DeeObject_CmpLoAsBool(indexob, sizeob);
		if (temp <= 0) {
			if unlikely(temp < 0)
				goto err_sizeob_indexob;
			break;
		}
		elem = CALL_DEPENDENCY(seq_operator_getitem, self, indexob);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_indexob;
			elem_hash = DEE_HASHOF_UNBOUND_ITEM;
		} else {
			elem_hash = DeeObject_Hash(elem);
			Dee_Decref(elem);
		}
		result = Dee_HashCombine(result, elem_hash);
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_indexob;
	}
	Dee_Decref(indexob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_indexob:
	Dee_Decref(indexob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return seq_handle_hash_error(self);
}}
[[prefix(DEFINE_seq_handle_hash_error)]] {
	int temp;
	Dee_hash_t result;
	DREF DeeObject *resultob;
	resultob = LOCAL_CALLATTR(self, 0, NULL);
	if unlikely(!resultob)
		goto err;
	temp = DeeObject_AsUIntX(resultob, &result);
	Dee_Decref(resultob);
	if unlikely(temp)
		goto err;
	return result;
err:
	return seq_handle_hash_error(self);
}


seq_operator_hash = {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = REQUIRE(seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &$empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &$with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &$with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &$with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &$with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair)
		return &$with__seq_operator_foreach_pair;
	if (seq_operator_foreach) {
		if (REQUIRE_NODEFAULT(seq_operator_foreach_pair))
			return &$with__seq_operator_foreach_pair;
		return $with__seq_operator_foreach;
	}
};
