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
/* deemon.Object.operator []=()                                         */
/************************************************************************/

operator {

[[wunused]] int
tp_seq->tp_setitem([[nonnull]] DeeObject *self,
                   [[nonnull]] DeeObject *index,
                   [[nonnull]] DeeObject *value)
%{class {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = index;
	args[1] = value;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_SETITEM, 2, args);
	Dee_Decref_unlikely(result); /* "unlikely" because return is probably "none" */
	return 0;
err:
	return -1;
}}
%{using [tp_seq->tp_setitem_index, tp_seq->tp_setitem_string_len_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_setitem_string_len_hash, self,
		                       DeeString_STR(index),
		                       DeeString_SIZE(index),
		                       DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_setitem_index, self, index_value, value);
err:
	return -1;
}}
%{using [tp_seq->tp_setitem_index, tp_seq->tp_setitem_string_hash]: {
	size_t index_value;
	if (DeeString_Check(index)) {
		return CALL_DEPENDENCY(tp_seq->tp_setitem_string_hash, self,
		                       DeeString_STR(index),
		                       DeeString_Hash(index), value);
	}
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_setitem_index, self, index_value, value);
err:
	return -1;
}}
%{using tp_seq->tp_setitem_index: {
	size_t index_value;
	if (DeeObject_AsSize(index, &index_value))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_setitem_index, self, index_value, value);
err:
	return -1;
}}
%{using tp_seq->tp_setitem_string_len_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_setitem_string_len_hash, self,
	                       DeeString_STR(index),
	                       DeeString_SIZE(index),
	                       DeeString_Hash(index), value);
err:
	return -1;
}}
%{using tp_seq->tp_setitem_string_hash: {
	if (DeeObject_AssertTypeExact(index, &DeeString_Type))
		goto err;
	return CALL_DEPENDENCY(tp_seq->tp_setitem_string_hash, self,
	                       DeeString_STR(index),
	                       DeeString_Hash(index), value);
err:
	return -1;
}} = OPERATOR_SETITEM;



[[wunused]] int
tp_seq->tp_setitem_index([[nonnull]] DeeObject *self, size_t index,
                         [[nonnull]] DeeObject *value)
%{using tp_seq->tp_setitem: {
	int result;
	DREF DeeObject *indexob = DeeInt_NewSize(index);
	if unlikely(!indexob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_setitem, self, indexob, value);
	Dee_Decref_likely(indexob);
	return result;
err:
	return -1;
}} = OPERATOR_SETITEM;



[[wunused]] int
tp_seq->tp_setitem_string_hash([[nonnull]] DeeObject *self,
                               [[nonnull]] char const *key, Dee_hash_t hash,
                               [[nonnull]] DeeObject *value)
%{using tp_seq->tp_setitem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_setitem, self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}} = OPERATOR_SETITEM;


[[wunused]] int
tp_seq->tp_setitem_string_len_hash([[nonnull]] DeeObject *self,
                                   [[nonnull]] char const *key,
                                   size_t keylen, Dee_hash_t hash,
                                   [[nonnull]] DeeObject *value)
%{using tp_seq->tp_setitem: {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_seq->tp_setitem, self, keyob, value);
	Dee_Decref_likely(keyob);
	return result;
err:
	return -1;
}} = OPERATOR_SETITEM;

} /* operator */
