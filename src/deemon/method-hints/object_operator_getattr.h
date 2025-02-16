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
/* deemon.Object.operator . ()                                          */
/************************************************************************/

operator {

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] DREF DeeObject *
tp_attr->tp_getattr([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *attr)
%{class using OPERATOR_GETATTR: {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_GETATTR, 1, &attr);
}} = OPERATOR_GETATTR;

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] DREF DeeObject *
tp_attr->tp_getattr_string_hash([[nonnull]] DeeObject *self,
                                [[nonnull]] char const *attr, Dee_hash_t hash)
%{using tp_attr->tp_getattr: {
	DREF DeeObject *result;
	DREF DeeObject *attrob = DeeString_NewWithHash(attr, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_getattr, self, attrob);
	Dee_Decref_likely(attrob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETATTR;

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] DREF DeeObject *
tp_attr->tp_getattr_string_len_hash([[nonnull]] DeeObject *self,
                                    [[nonnull]] char const *attr,
                                    size_t attrlen, Dee_hash_t hash)
%{using tp_attr->tp_getattr: {
	DREF DeeObject *result;
	DREF DeeObject *attrob = DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_getattr, self, attrob);
	Dee_Decref_likely(attrob);
	return result;
err:
	return NULL;
}} = OPERATOR_GETATTR;






[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_boundattr([[nonnull]] DeeObject *self,
                      [[nonnull]] DeeObject *attr)
%{using tp_attr->tp_getattr: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_attr->tp_getattr, self, attr);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_AttributeError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}} = OPERATOR_GETATTR;


[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_boundattr_string_hash([[nonnull]] DeeObject *self,
                                  [[nonnull]] char const *key,
                                  Dee_hash_t hash)
%{using tp_attr->tp_getattr_string_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_attr->tp_getattr_string_hash, self, key, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_AttributeError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using tp_attr->tp_boundattr: {
	int result;
	DREF DeeObject *keyob = DeeString_NewWithHash(key, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_boundattr, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETATTR;



[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_boundattr_string_len_hash([[nonnull]] DeeObject *self,
                                      [[nonnull]] char const *key,
                                      size_t keylen, Dee_hash_t hash)
%{using tp_attr->tp_getattr_string_len_hash: {
	DREF DeeObject *value = CALL_DEPENDENCY(tp_attr->tp_getattr_string_len_hash, self, key, keylen, hash);
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_AttributeError))
		return Dee_BOUND_MISSING;
	return Dee_BOUND_ERR;
}}
%{using tp_attr->tp_boundattr: {
	int result;
	DREF DeeObject *keyob = DeeString_NewSizedWithHash(key, keylen, hash);
	if unlikely(!keyob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_boundattr, self, keyob);
	Dee_Decref_likely(keyob);
	return result;
err:
	return Dee_BOUND_ERR;
}} = OPERATOR_GETATTR;




[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_hasattr([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *attr)
%{using tp_attr->tp_boundattr: {
	int result = CALL_DEPENDENCY(tp_attr->tp_boundattr, self, attr);
	return Dee_BOUND_ASHAS(result);
}} = OPERATOR_GETATTR;


[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_hasattr_string_hash([[nonnull]] DeeObject *self,
                                [[nonnull]] char const *key, Dee_hash_t hash)
%{using tp_attr->tp_boundattr_string_hash: {
	int result = CALL_DEPENDENCY(tp_attr->tp_boundattr_string_hash, self, key, hash);
	return Dee_BOUND_ASHAS(result);
}} = OPERATOR_GETATTR;


[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_hasattr_string_len_hash([[nonnull]] DeeObject *self,
                                    [[nonnull]] char const *key,
                                    size_t keylen, Dee_hash_t hash)
%{using tp_attr->tp_boundattr_string_len_hash: {
	int result = CALL_DEPENDENCY(tp_attr->tp_boundattr_string_len_hash, self, key, keylen, hash);
	return Dee_BOUND_ASHAS(result);
}} = OPERATOR_GETATTR;


} /* operator */
