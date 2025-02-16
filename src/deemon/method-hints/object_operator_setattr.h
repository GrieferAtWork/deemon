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
/* deemon.Object.operator .= ()                                         */
/************************************************************************/

operator {

[[wunused]] int
tp_attr->tp_setattr([[nonnull]] DeeObject *self,
                    [[nonnull]] DeeObject *attr,
                    [[nonnull]] DeeObject *value)
%{class using OPERATOR_SETATTR: {
	DREF DeeObject *result;
	DeeObject *args[2];
	args[0] = attr;
	args[1] = value;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_SETATTR, 2, args);
	Dee_Decref_unlikely(result); /* *_unlikely because it's probably `Dee_None' */
	return 0;
err:
	return -1;
}} /*= OPERATOR_SETATTR*/;

[[wunused]] int
tp_attr->tp_setattr_string_hash([[nonnull]] DeeObject *self,
                                [[nonnull]] char const *attr, Dee_hash_t hash,
                                [[nonnull]] DeeObject *value)
%{using tp_attr->tp_setattr: {
	int result;
	DREF DeeObject *attrob = DeeString_NewWithHash(attr, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_setattr, self, attrob, value);
	Dee_Decref_likely(attrob);
	return result;
err:
	return -1;
}} /*= OPERATOR_SETATTR*/;

[[wunused]] int
tp_attr->tp_setattr_string_len_hash([[nonnull]] DeeObject *self,
                                    [[nonnull]] char const *attr,
                                    size_t attrlen, Dee_hash_t hash,
                                    [[nonnull]] DeeObject *value)
%{using tp_attr->tp_setattr: {
	int result;
	DREF DeeObject *attrob = DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attrob)
		goto err;
	result = CALL_DEPENDENCY(tp_attr->tp_setattr, self, attrob, value);
	Dee_Decref_likely(attrob);
	return result;
err:
	return -1;
}} /*= OPERATOR_SETATTR*/;

} /* operator */
