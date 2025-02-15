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
/* deemon.Object.operator call()                                        */
/************************************************************************/

operator {

[[export("DeeObject_{|T}Call")]]
[[wunused]] DREF DeeObject *
tp_call([[nonnull]] DeeObject *self,
        size_t argc, DeeObject *const *argv)
%{class {
	return_DeeClass_CallOperator(THIS_TYPE, self, OPERATOR_CALL, argc, argv);
}}
%{using tp_call_kw: {
	return CALL_DEPENDENCY(tp_call_kw, self, argc, argv, NULL);
}} = OPERATOR_CALL;


[[export("DeeObject_{|T}CallKw")]]
[[wunused]] DREF DeeObject *
tp_call_kw([[nonnull]] DeeObject *self,
           size_t argc, DeeObject *const *argv,
           DeeObject *kw)
%{class {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(THIS_TYPE, OPERATOR_CALL);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	Dee_Decref_unlikely(func);
	return result;
err:
	return NULL;
}}
%{using tp_call: {
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_length;
			kw_length = DeeObject_Size(kw);
			if unlikely(kw_length == (size_t)-1)
				goto err;
			if (kw_length != 0)
				goto err_no_keywords;
		}
	}
	return CALL_DEPENDENCY(tp_call, self, argc, argv);
err_no_keywords:
	err_keywords_not_accepted(THIS_TYPE, kw);
err:
	return NULL;
}} = OPERATOR_CALL;

} /* operator */
