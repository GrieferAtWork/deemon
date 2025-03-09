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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_UTILS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_UTILS_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>

#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT64_MAX__ */

/**/
#include "operator-hint-errors.h"
#include "runtime_error.h"

/**/
#include <stdbool.h> /* true/false */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* intN_t */

#ifndef INT8_MIN
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN */
#ifndef INT8_MAX
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX */
#ifndef UINT8_MAX
#define UINT8_MAX __UINT8_MAX__
#endif /* !UINT8_MAX */
#ifndef INT16_MIN
#define INT16_MIN __INT16_MIN__
#endif /* !INT16_MIN */
#ifndef INT16_MAX
#define INT16_MAX __INT16_MAX__
#endif /* !INT16_MAX */
#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */

DECL_BEGIN

/************************************************************************/
/* MISC. RUNTIME OBJECT WRAPPERS                                        */
/************************************************************************/

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeObject_Get128Bit(DeeObject *__restrict self,
                    Dee_int128_t *__restrict result) {
	int retval;
	DREF DeeObject *intob;
	__register DeeNO_int_t tp_int;
	__register DeeTypeObject *tp_self = Dee_TYPE(self);
	if (DeeInt_Check(self))
		return DeeInt_Get128Bit(self, result);
	if unlikely(!tp_self->tp_math || (tp_int = tp_self->tp_math->tp_int) == NULL)
		tp_int = _DeeType_RequireNativeOperator(tp_self, int);
	/* Cast to integer, then read its value. */
	intob = (*tp_int)(self);
	if unlikely(!intob)
		goto err;
	retval = DeeInt_Get128Bit(intob, result);
	Dee_Decref(intob);
	return retval;
err:
	return -1;
}


/* All of these return (T)-1 on error. When the object's actual value is `(T)-1', throw `IntegerOverflow' */
PUBLIC WUNUSED NONNULL((1)) uint8_t
(DCALL DeeObject_AsDirectUInt8)(DeeObject *__restrict self) {
	uint8_t result;
	if unlikely(DeeObject_AsUInt8(self, &result))
		goto err;
	if unlikely(result == (uint8_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(8, true);
err:
	return (uint8_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t
(DCALL DeeObject_AsDirectUInt16)(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_AsUInt16(self, &result))
		goto err;
	if unlikely(result == (uint16_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(16, true);
err:
	return (uint16_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t
(DCALL DeeObject_AsDirectUInt32)(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_AsUInt32(self, &result))
		goto err;
	if unlikely(result == (uint32_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(32, true);
err:
	return (uint32_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) uint64_t
(DCALL DeeObject_AsDirectUInt64)(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_AsUInt64(self, &result))
		goto err;
	if unlikely(result == (uint64_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(64, true);
err:
	return (uint64_t)-1;
}




PUBLIC WUNUSED NONNULL((1)) uint8_t
(DCALL DeeObject_AsDirectUInt8Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint8_t result = DeeObject_AsDirectUInt8(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint16_t
(DCALL DeeObject_AsDirectUInt16Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint16_t result = DeeObject_AsDirectUInt16(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint32_t
(DCALL DeeObject_AsDirectUInt32Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint32_t result = DeeObject_AsDirectUInt32(self);
	Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint64_t
(DCALL DeeObject_AsDirectUInt64Inherited)(/*inherit(always)*/ DREF DeeObject *__restrict self) {
	uint64_t result = DeeObject_AsDirectUInt64(self);
	Dee_Decref(self);
	return result;
}


PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt32)(DeeObject *__restrict self,
                          int32_t *__restrict result) {
	int error = DeeObject_Get32Bit(self, result);
	if (error == INT_UNSIGNED) {
		if ((int32_t)*result < 0 &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 32, true);
#if INT_UNSIGNED != 0
		return 0;
#endif /* INT_UNSIGNED != 0 */
	} else {
#if INT_UNSIGNED != 0
		ASSERT(error <= 0);
#else /* INT_UNSIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_UNSIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt32)(DeeObject *__restrict self,
                           uint32_t *__restrict result) {
	int error = DeeObject_Get32Bit(self, (int32_t *)result);
	if (error == INT_SIGNED) {
		if ((int32_t)*result < 0 &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 32, false);
#if INT_SIGNED != 0
		return 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt64)(DeeObject *__restrict self,
                          int64_t *__restrict result) {
	int error = DeeObject_Get64Bit(self, result);
	if (error == INT_UNSIGNED) {
		if ((int64_t)*result < 0 &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 64, true);
#if INT_UNSIGNED != 0
		return 0;
#endif /* INT_UNSIGNED != 0 */
	} else {
#if INT_UNSIGNED != 0
		ASSERT(error <= 0);
#else /* INT_UNSIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_UNSIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt64)(DeeObject *__restrict self,
                           uint64_t *__restrict result) {
	int error = DeeObject_Get64Bit(self, (int64_t *)result);
	if (error == INT_SIGNED) {
		if ((int64_t)*result < 0 &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 64, false);
#if INT_SIGNED != 0
		return 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt128)(DeeObject *__restrict self,
                           Dee_int128_t *__restrict result) {
	int error = DeeObject_Get128Bit(self, result);
	if (error == INT_UNSIGNED) {
		if (__hybrid_int128_isneg(*result) &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 128, true);
#if INT_UNSIGNED != 0
		return 0;
#endif /* INT_UNSIGNED != 0 */
	} else {
#if INT_UNSIGNED != 0
		ASSERT(error <= 0);
#else /* INT_UNSIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_UNSIGNED == 0 */
	}
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt128)(DeeObject *__restrict self,
                            Dee_uint128_t *__restrict result) {
	int error = DeeObject_Get128Bit(self, (Dee_int128_t *)result);
	if (error == INT_SIGNED) {
		if (__hybrid_int128_isneg(*(Dee_int128_t const *)result) &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return err_integer_overflow(self, 128, false);
#if INT_SIGNED != 0
		return 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	return error;
}


PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeObject_Get8Bit(DeeObject *__restrict self,
                  int8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (error == INT_SIGNED) {
		if (val32 < INT8_MIN || val32 > INT8_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (int8_t)val32;
				return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
			}
			return err_integer_overflow(self, 8, val32 > 0);
		}
		*result = (int8_t)val32;
	} else {
		if ((uint32_t)val32 > UINT8_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (uint8_t)val32;
				return INT_UNSIGNED;
			}
			return err_integer_overflow(self, 8, true);
		}
		*result = (int8_t)(uint8_t)val32;
	}
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeObject_Get16Bit(DeeObject *__restrict self,
                   int16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (error == INT_SIGNED) {
		if (val32 < INT16_MIN || val32 > INT16_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (int16_t)val32;
				return *result < 0 ? INT_SIGNED : INT_UNSIGNED;
			}
			return err_integer_overflow(self, 16, val32 > 0);
		}
		*result = (int16_t)val32;
	} else {
		if ((uint32_t)val32 > UINT16_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (uint16_t)val32;
				return INT_UNSIGNED;
			}
			return err_integer_overflow(self, 16, true);
		}
		*result = (int16_t)(uint16_t)val32;
	}
done:
	return error;
}


PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt8)(DeeObject *__restrict self,
                         int8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_AsInt32(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (val32 < INT8_MIN || val32 > INT8_MAX) {
		if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 8, val32 > 0);
	}
return_value:
	*result = (int8_t)val32;
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt8)(DeeObject *__restrict self,
                          uint8_t *__restrict result) {
	uint32_t val32;
	int error = DeeObject_AsUInt32(self, &val32);
	if unlikely(error < 0)
		goto done;
	if (val32 > UINT8_MAX) {
		if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 8, true);
	}
return_value:
	*result = (uint8_t)val32;
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsInt16)(DeeObject *__restrict self,
                          int16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_AsInt32(self, &val32);
	if unlikely(error < 0)
		goto err;
	if (val32 < INT16_MIN || val32 > INT16_MAX) {
		if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 16, val32 > 0);
	}
return_value:
	*result = (int16_t)val32;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt16)(DeeObject *__restrict self,
                           uint16_t *__restrict result) {
	uint32_t val32;
	int error = DeeObject_AsUInt32(self, &val32);
	if unlikely(error < 0)
		goto err;
	if (val32 > UINT16_MAX) {
		if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)
			goto return_value;
		return err_integer_overflow(self, 16, true);
	}
return_value:
	*result = (uint16_t)val32;
	return 0;
err:
	return -1;
}



PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallf(DeeObject *self, char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_Call(self,
	                        DeeTuple_SIZE(args_tuple),
	                        DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_VThisCallf(DeeObject *self, DeeObject *this_arg,
                     char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_ThisCall(self, this_arg,
	                            DeeTuple_SIZE(args_tuple),
	                            DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1)) DREF DeeObject *
DeeObject_CallPack(DeeObject *self, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallPack(self, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_Callf(DeeObject *self,
                char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallf(self, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeObject_ThisCallf(DeeObject *self, DeeObject *this_arg,
                    char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VThisCallf(self, this_arg, format, args);
	va_end(args);
	return result;
}





DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_UTILS_C */
