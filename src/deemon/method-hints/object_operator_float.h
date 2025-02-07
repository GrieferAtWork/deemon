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
/* deemon.Object.operator float()                                       */
/************************************************************************/

operator {

[[wunused]] int
tp_math->tp_double([[nonnull]] DeeObject *__restrict self,
                   [[nonnull]] double *__restrict p_result)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_FLOAT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeFloat_Type))
		goto err_r;
	*p_result = DeeFloat_VALUE(result);
	Dee_Decref_likely(result);
	return 0;
err_r:
	Dee_Decref(result);
err:
	return -1;
}}
%{using tp_math->tp_int: {
	int status;
	DREF DeeObject *temp = CALL_DEPENDENCY(tp_math->tp_int, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_AsDouble(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}}
%{using tp_math->tp_int64: {
	int64_t intval;
	int status = CALL_DEPENDENCY(tp_math->tp_int64, self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint64_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
}}
%{using tp_math->tp_int32: {
	int32_t intval;
	int status = CALL_DEPENDENCY(tp_math->tp_int32, self, &intval);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (double)(uint32_t)intval;
		status = 0;
	} else {
		*p_result = (double)intval;
	}
	return status;
}};

} /* operator */
