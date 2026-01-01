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
/* deemon.Object.operator del. ()                                       */
/************************************************************************/

operator {

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_delattr([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *attr)
%{class using OPERATOR_DELATTR: {
	DREF DeeObject *result;
	store_DeeClass_CallOperator_1Arg(err, result, THIS_TYPE, self, OPERATOR_DELATTR, attr);
	Dee_Decref_probably_none(result);
	return 0;
err:
	return -1;
}} = OPERATOR_DELATTR;

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_delattr_string_hash([[nonnull]] DeeObject *self,
                                [[nonnull]] char const *attr, Dee_hash_t hash)
%{using tp_attr->tp_delattr: /*[[disliked]]*/ {
	int result;
	DREF DeeObject *attrob = DeeString_NewWithHash(attr, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_delattr, self, attrob);
	Dee_Decref_likely(attrob);
	return result;
err:
	return -1;
}} = OPERATOR_DELATTR;

[[custom_unsupported_impl_name(NULL)]]
[[wunused]] int
tp_attr->tp_delattr_string_len_hash([[nonnull]] DeeObject *self,
                                    [[nonnull]] char const *attr,
                                    size_t attrlen, Dee_hash_t hash)
%{using tp_attr->tp_delattr: /*[[disliked]]*/ {
	int result;
	DREF DeeObject *attrob = DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_delattr, self, attrob);
	Dee_Decref_likely(attrob);
	return result;
err:
	return -1;
}} = OPERATOR_DELATTR;

} /* operator */
