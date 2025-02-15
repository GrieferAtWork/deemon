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
/* deemon.Object.operator int()                                         */
/************************************************************************/

operator {

[[export("DeeObject_{|T}Int")]]
[[wunused]] DREF DeeObject *
tp_math->tp_int([[nonnull]] DeeObject *__restrict self)
%{class {
	DREF DeeObject *result;
	store_DeeClass_CallOperator(err, result, THIS_TYPE, self, OPERATOR_INT, 0, NULL);
	if (DeeObject_AssertTypeExact(result, &DeeInt_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}}
%{using tp_math->tp_int64: {
	int64_t value;
	int status = CALL_DEPENDENCY(tp_math->tp_int64, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt64((uint64_t)value);
	return DeeInt_NewInt64(value);
err:
	return NULL;
}}
%{using tp_math->tp_int32: {
	int32_t value;
	int status = CALL_DEPENDENCY(tp_math->tp_int32, self, &value);
	if unlikely(status < 0)
		goto err;
	if (status == Dee_INT_UNSIGNED)
		return DeeInt_NewUInt32((uint32_t)value);
	return DeeInt_NewInt32(value);
err:
	return NULL;
}}
%{using tp_math->tp_double: {
	double value;
	int status = CALL_DEPENDENCY(tp_math->tp_double, self, &value);
	if unlikely(status < 0)
		goto err;
	return DeeInt_NewDouble(value);
err:
	return NULL;
}} = OPERATOR_INT;




[[export("DeeObject_{|T}Get32Bit")]]
[[wunused]] int
tp_math->tp_int32([[nonnull]] DeeObject *__restrict self,
                  [[nonnull]] int32_t *__restrict p_result)
%{using tp_math->tp_int64: {
	int64_t value;
	int status = CALL_DEPENDENCY(tp_math->tp_int64, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		if ((uint64_t)value > UINT32_MAX && !(THIS_TYPE->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)(uint32_t)(uint64_t)value;
	} else {
		if ((value < INT32_MIN || value > INT32_MAX) && status == Dee_INT_SIGNED &&
		    !(THIS_TYPE->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
		*p_result = (int32_t)value;
	}
	return status;
err_overflow:
	return err_integer_overflow(self, 32, status == Dee_INT_SIGNED);
}}
%{using tp_math->tp_int: {
	int status;
	DREF DeeObject *temp = CALL_DEPENDENCY(tp_math->tp_int, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get32Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}}
%{using tp_math->tp_double: {
	double value;
	int status = CALL_DEPENDENCY(tp_math->tp_double, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT32_MIN && !(THIS_TYPE->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT32_MAX) {
		if (value <= UINT32_MAX) {
			*p_result = (int32_t)(uint32_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(THIS_TYPE->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int32_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 32, value >= 0);
err:
	return -1;
}} = OPERATOR_INT;




[[export("DeeObject_{|T}Get64Bit")]]
[[wunused]] int
tp_math->tp_int64([[nonnull]] DeeObject *__restrict self,
                  [[nonnull]] int64_t *__restrict p_result)
%{using tp_math->tp_int32: {
	int32_t value;
	int status = CALL_DEPENDENCY(tp_math->tp_int32, self, &value);
	if (status == Dee_INT_UNSIGNED) {
		*p_result = (int64_t)(uint64_t)(uint32_t)value;
	} else {
		*p_result = (int64_t)value;
	}
	return status;
}}
%{using tp_math->tp_int: {
	int status;
	DREF DeeObject *temp = CALL_DEPENDENCY(tp_math->tp_int, self);
	if unlikely(!temp)
		goto err;
	status = DeeInt_Get64Bit(temp, p_result);
	Dee_Decref_likely(temp);
	return status;
err:
	return -1;
}}
%{using tp_math->tp_double: {
	double value;
	int status = CALL_DEPENDENCY(tp_math->tp_double, self, &value);
	if unlikely(status < 0)
		goto err;
	if (value < INT64_MIN && !(THIS_TYPE->tp_flags & TP_FTRUNCATE))
		goto err_overflow;
	if (value > INT64_MAX) {
		if (value <= UINT64_MAX) {
			*p_result = (int64_t)(uint64_t)value;
			return Dee_INT_UNSIGNED;
		}
		if (!(THIS_TYPE->tp_flags & TP_FTRUNCATE))
			goto err_overflow;
	}
	*p_result = (int64_t)value;
	return Dee_INT_SIGNED;
err_overflow:
	err_integer_overflow(self, 64, value >= 0);
err:
	return -1;
}} = OPERATOR_INT;

} /* operator */
