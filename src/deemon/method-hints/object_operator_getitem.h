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
/* deemon.Object.operator []()                                          */
/************************************************************************/

operator {

[[wunused]] DREF DeeObject *
tp_seq->tp_getitem([[nonnull]] DeeObject *self,
                   [[nonnull]] DeeObject *index)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_GETITEM, 1, &index);
}}
%{using tp_seq->tp_trygetitem: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, index);
	if unlikely(result == ITER_DONE)
		goto err_unbound;
	return result;
err_unbound:
	err_unknown_key(self, index);
	return NULL;
}}
%{using [tp_seq->tp_getitem_index, tp_seq->tp_getitem_string_len_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_getitem_string_len_hash, self,
		                       DeeString_STR(index),
		                       DeeString_SIZE(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getitem_index, self, index_value);
err:
	return NULL;
}}
%{using [tp_seq->tp_getitem_index, tp_seq->tp_getitem_string_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_getitem_string_hash, self,
		                       DeeString_STR(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getitem_index, self, index_value);
err:
	return NULL;
}}
%{using tp_seq->tp_getitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getitem_index, self, index_value);
err:
	return NULL;
}}
%{using tp_seq->tp_getitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getitem_string_len_hash, self,
	                       DeeString_STR(index),
	                       DeeString_SIZE(index),
	                       DeeString_Hash(index));
err:
	return NULL;
}}
%{using tp_seq->tp_getitem_string_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_getitem_string_hash, self,
	                       DeeString_STR(index),
	                       DeeString_Hash(index));
err:
	return NULL;
}} = OPERATOR_GETITEM;



/* Same as `tp_getitem', but returns `ITER_DONE' instead of throwing
 * `KeyError' or `IndexError' (or `UnboundItem', which is a given
 * since that one's a sub-class of `IndexError') */
[[wunused]] DREF DeeObject *
tp_seq->tp_trygetitem([[nonnull]] DeeObject *self,
                      [[nonnull]] DeeObject *index)
%{using tp_seq->tp_getitem: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_getitem, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{using [tp_seq->tp_trygetitem_index, tp_seq->tp_trygetitem_string_len_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self,
		                       DeeString_STR(index),
		                       DeeString_SIZE(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index_value);
err:
	return NULL;
}}
%{using [tp_seq->tp_trygetitem_index, tp_seq->tp_trygetitem_string_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self,
		                       DeeString_STR(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index_value);
err:
	return NULL;
}}
%{using tp_seq->tp_trygetitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index_value);
err:
	return NULL;
}}
%{using tp_seq->tp_trygetitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self,
	                       DeeString_STR(index),
	                       DeeString_SIZE(index),
	                       DeeString_Hash(index));
err:
	return NULL;
}}
%{using tp_seq->tp_trygetitem_string_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self,
	                       DeeString_STR(index),
	                       DeeString_Hash(index));
err:
	return NULL;
}} = OPERATOR_GETITEM;


/* Same as `tp_getitem_index', but never throws an exception:
 * NOTE: This operator can NOT be used to substitute `tp_getitem_index'!
 * @param: index: Index of item to access. Guarantied to be `<' some preceding
 *                call to `tp_size_fast', `tp_size', or `tp_sizeob' (from the
 *                same type; i.e. the size operator will not be from a sub-class).
 * @return: * :   A reference to the index.
 * @return: NULL: The sequence is resizable and `index >= CURRENT_SIZE'
 * @return: NULL: Sequence indices can be unbound, and nothing is bound to `index' right now. */
[[wunused]] DREF DeeObject *
tp_seq->tp_getitem_index_fast([[nonnull]] DeeObject *self, size_t index);


[[wunused]] DREF DeeObject *
tp_seq->tp_getitem_index([[nonnull]] DeeObject *self, size_t index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	DREF DeeObject *result;
	size_t size = CALL_DEPENDENCY(tp_seq->tp_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		goto err_oob;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem_index_fast, self, index);
	if unlikely(!result)
		err_unbound_index(self, index);
	return result;
err_oob:
	err_index_out_of_bounds(self, index, size);
err:
	return NULL;
}}
%{using tp_seq->tp_trygetitem_index: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index);
	if unlikely(result == ITER_DONE) {
		err_unbound_index(self, index);
		result = NULL;
	}
	return result;
}}
%{using tp_seq->tp_getitem: {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;



[[wunused]] DREF DeeObject *
tp_seq->tp_trygetitem_index([[nonnull]] DeeObject *self, size_t index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	DREF DeeObject *result;
	size_t size = CALL_DEPENDENCY(tp_seq->tp_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return ITER_DONE;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem_index_fast, self, index);
	if (result == NULL)
		result = ITER_DONE;
	return result;
err:
	return NULL;
}}
%{using tp_seq->tp_getitem_index: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_getitem_index, self, index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{using tp_seq->tp_trygetitem: {
	DREF DeeObject *result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, indexob);
	Dee_Decref(indexob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;



[[wunused]] DREF DeeObject *
tp_seq->tp_getitem_string_hash([[nonnull]] DeeObject *self,
                               [[nonnull]] char const *key, Dee_hash_t hash)
%{using tp_seq->tp_trygetitem_string_hash: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
}}
%{using tp_seq->tp_getitem: {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;


[[wunused]] DREF DeeObject *
tp_seq->tp_trygetitem_string_hash([[nonnull]] DeeObject *self,
                                  [[nonnull]] char const *key, Dee_hash_t hash)
%{using tp_seq->tp_getitem_string_hash: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_getitem_string_hash, self, key, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{using tp_seq->tp_trygetitem: {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;



[[wunused]] DREF DeeObject *
tp_seq->tp_getitem_string_len_hash([[nonnull]] DeeObject *self,
                                   [[nonnull]] char const *key,
                                   size_t keylen, Dee_hash_t hash)
%{using tp_seq->tp_trygetitem_string_len_hash: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unbound_key_str(self, key);
		result = NULL;
	}
	return result;
}}
%{using tp_seq->tp_getitem: {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;


[[wunused]] DREF DeeObject *
tp_seq->tp_trygetitem_string_len_hash([[nonnull]] DeeObject *self,
                                      [[nonnull]] char const *key,
                                      size_t keylen, Dee_hash_t hash)
%{using tp_seq->tp_getitem_string_len_hash: {
	DREF DeeObject *result = CALL_DEPENDENCY(tp_seq->tp_getitem_string_len_hash, self, key, keylen, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_KeyError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
}}
%{using tp_seq->tp_trygetitem: {
	DREF DeeObject *result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETITEM;



[[wunused]] int
tp_seq->tp_bounditem([[nonnull]] DeeObject *self,
                     [[nonnull]] DeeObject *index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return IF_TYPED_ELSE(tdefault__bounditem_index__with__size__and__getitem_index_fast(tp_self, self, index_value),
	                     default__bounditem_index__with__size__and__getitem_index_fast(self, index_value));
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_getitem: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_getitem, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_bounditem_index, tp_seq->tp_bounditem_string_len_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_bounditem_string_len_hash, self,
		                       DeeString_STR(index),
		                       DeeString_SIZE(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_bounditem_index, self, index_value);
err:
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_bounditem_index, tp_seq->tp_bounditem_string_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_bounditem_string_hash, self,
		                       DeeString_STR(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_bounditem_index, self, index_value);
err:
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_trygetitem, tp_seq->tp_hasitem]: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem, self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_bounditem_index, self, index_value);
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem_string_len_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_bounditem_string_len_hash, self,
	                       DeeString_STR(index),
	                       DeeString_SIZE(index),
	                       DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem_string_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_bounditem_string_hash, self,
	                       DeeString_STR(index),
	                       DeeString_Hash(index));
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_trygetitem: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETITEM;




[[wunused]] int
tp_seq->tp_bounditem_index([[nonnull]] DeeObject *self, size_t index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	DREF DeeObject *result;
	size_t size = CALL_DEPENDENCY(tp_seq->tp_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(index >= size)
		return Dee_BOUND_MISSING;
	result = CALL_DEPENDENCY(tp_seq->tp_getitem_index_fast, self, index);
	if unlikely(!result)
		return Dee_BOUND_NO;
	Dee_Decref(result);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_getitem_index: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_getitem_index, self, index);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem: {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_bounditem, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_trygetitem_index, tp_seq->tp_hasitem_index]: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem_index, self, index);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_trygetitem_index: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETITEM;



[[wunused]] int
tp_seq->tp_bounditem_string_hash([[nonnull]] DeeObject *self,
                                 [[nonnull]] char const *key, Dee_hash_t hash)
%{using tp_seq->tp_getitem_string_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_getitem_string_hash, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_bounditem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_trygetitem_string_hash, tp_seq->tp_hasitem_string_hash]: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem_string_hash, self, key, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_trygetitem_string_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETITEM;



[[wunused]] int
tp_seq->tp_bounditem_string_len_hash([[nonnull]] DeeObject *self,
                                     [[nonnull]] char const *key,
                                     size_t keylen, Dee_hash_t hash)
%{using tp_seq->tp_getitem_string_len_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_getitem_string_len_hash, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError) ||
	    DeeError_Catch(&DeeError_IndexError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_bounditem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_bounditem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}}
%{using [tp_seq->tp_trygetitem_string_len_hash, tp_seq->tp_hasitem_string_len_hash]: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value != ITER_DONE) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem_string_len_hash, self, key, keylen, hash);
	return Dee_BOUND_FROMHAS_UNBOUND(result);
err:
	return Dee_BOUND_ERR;
}}
%{using tp_seq->tp_trygetitem_string_len_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return Dee_BOUND_NO;
	Dee_Decref(value);
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETITEM;







[[wunused]] int
tp_seq->tp_hasitem([[nonnull]] DeeObject *self,
                   [[nonnull]] DeeObject *index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return IF_TYPED_ELSE(tdefault__hasitem_index__with__size__and__getitem_index_fast(tp_self, self, index_value),
	                     default__hasitem_index__with__size__and__getitem_index_fast(self, index_value));
err:
	return -1;
}}
%{using tp_seq->tp_trygetitem: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}}
%{using tp_seq->tp_bounditem: {
	int result = CALL_DEPENDENCY(tp_seq->tp_bounditem, self, index);
	return Dee_BOUND_ASHAS(result);
}}
%{using [tp_seq->tp_hasitem_index, tp_seq->tp_hasitem_string_len_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_hasitem_string_len_hash, self,
		                       DeeString_STR(index),
		                       DeeString_SIZE(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_hasitem_index, self, index_value);
err:
	return -1;
}}
%{using [tp_seq->tp_hasitem_index, tp_seq->tp_hasitem_string_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_hasitem_string_hash, self,
		                       DeeString_STR(index),
		                       DeeString_Hash(index));
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_hasitem_index, self, index_value);
err:
	return -1;
}}
%{using tp_seq->tp_hasitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_hasitem_index, self, index_value);
err:
	return -1;
}}
%{using tp_seq->tp_hasitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_hasitem_string_len_hash, self,
	                       DeeString_STR(index),
	                       DeeString_SIZE(index),
	                       DeeString_Hash(index));
err:
	return -1;
}}
%{using tp_seq->tp_hasitem_string_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_hasitem_string_hash, self,
	                       DeeString_STR(index),
	                       DeeString_Hash(index));
err:
	return -1;
}} = OPERATOR_GETITEM;




[[wunused]] int
tp_seq->tp_hasitem_index([[nonnull]] DeeObject *self, size_t index)
%{using [tp_seq->tp_size, tp_seq->tp_getitem_index_fast]: {
	size_t size = CALL_DEPENDENCY(tp_seq->tp_size, self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
}}
%{using tp_seq->tp_trygetitem_index: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_index, self, index);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}}
%{using tp_seq->tp_bounditem_index: {
	int result = CALL_DEPENDENCY(tp_seq->tp_bounditem_index, self, index);
	return Dee_BOUND_ASHAS(result);
}}
%{using tp_seq->tp_hasitem: {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem, self, indexob);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
}} = OPERATOR_GETITEM;




[[wunused]] int
tp_seq->tp_hasitem_string_hash([[nonnull]] DeeObject *self,
                               [[nonnull]] char const *key, Dee_hash_t hash)
%{using tp_seq->tp_trygetitem_string_hash: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_hash, self, key, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}}
%{using tp_seq->tp_bounditem_string_hash: {
	int result = CALL_DEPENDENCY(tp_seq->tp_bounditem_string_hash, self, key, hash);
	return Dee_BOUND_ASHAS(result);
}}
%{using tp_seq->tp_hasitem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}} = OPERATOR_GETITEM;


[[wunused]] int
tp_seq->tp_hasitem_string_len_hash([[nonnull]] DeeObject *self,
                                   [[nonnull]] char const *key,
                                   size_t keylen, Dee_hash_t hash)
%{using tp_seq->tp_trygetitem_string_len_hash: {
	int result;
	DREF DeeObject *value = CALL_DEPENDENCY(tp_seq->tp_trygetitem_string_len_hash, self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
err:
	return -1;
}}
%{using tp_seq->tp_bounditem_string_len_hash: {
	int result = CALL_DEPENDENCY(tp_seq->tp_bounditem_string_len_hash, self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}}
%{using tp_seq->tp_hasitem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_hasitem, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}} = OPERATOR_GETITEM;


} /* operator */
