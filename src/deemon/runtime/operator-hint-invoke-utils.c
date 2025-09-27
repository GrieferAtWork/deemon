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
#include <deemon/error-rt.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/super.h> /* DeeObject_TCall, ... */
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include <hybrid/int128.h>
#include <hybrid/limitcore.h> /* __INT32_MAX__, __INT64_MAX__ */

/**/
#include "runtime_error.h"

/**/
#include <stdarg.h>  /* va_list */
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
#ifndef INT32_MIN
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */
#ifndef INT64_MIN
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN */
#ifndef INT64_MAX
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */
#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

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
	DeeRT_ErrIntegerOverflowU8(result, (uint8_t)-2);
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
	DeeRT_ErrIntegerOverflowU16(result, (uint16_t)-2);
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
	DeeRT_ErrIntegerOverflowU32(result, (uint32_t)-2);
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
	DeeRT_ErrIntegerOverflowU64(result, (uint64_t)-2);
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
		if ((int32_t)*result < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 32,
			                                  DeeRT_ErrIntegerOverflowEx_F_SIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_POSITIVE);
		}
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
		if ((int32_t)*result < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 32,
			                                  DeeRT_ErrIntegerOverflowEx_F_UNSIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_NEGATIVE);
		}
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
		if ((int64_t)*result < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 64,
			                                  DeeRT_ErrIntegerOverflowEx_F_SIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_POSITIVE);
		}
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
		if ((int64_t)*result < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 64,
			                                  DeeRT_ErrIntegerOverflowEx_F_UNSIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_NEGATIVE);
		}
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
		if (__hybrid_int128_isneg(*result) && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 128,
			                                  DeeRT_ErrIntegerOverflowEx_F_SIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_POSITIVE);
		}
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
		if (__hybrid_int128_isneg(*(Dee_int128_t const *)result) && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflowEx(self, 128,
			                                  DeeRT_ErrIntegerOverflowEx_F_UNSIGNED |
			                                  DeeRT_ErrIntegerOverflowEx_F_NEGATIVE);
		}
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
			return DeeRT_ErrIntegerOverflowEx(self, 8,
			                                  DeeRT_ErrIntegerOverflowEx_F_ANYSIGN |
			                                  ((val32 > 0)
			                                   ? DeeRT_ErrIntegerOverflowEx_F_POSITIVE
			                                   : DeeRT_ErrIntegerOverflowEx_F_NEGATIVE));
		}
		*result = (int8_t)val32;
	} else {
		if ((uint32_t)val32 > UINT8_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (uint8_t)val32;
				return INT_UNSIGNED;
			}
			return DeeRT_ErrIntegerOverflowEx(self, 8,
			                                  DeeRT_ErrIntegerOverflowEx_F_ANYSIGN |
			                                  DeeRT_ErrIntegerOverflowEx_F_POSITIVE);
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
			return DeeRT_ErrIntegerOverflowEx(self, 16,
			                                  DeeRT_ErrIntegerOverflowEx_F_ANYSIGN |
			                                  ((val32 > 0)
			                                   ? DeeRT_ErrIntegerOverflowEx_F_POSITIVE
			                                   : DeeRT_ErrIntegerOverflowEx_F_NEGATIVE));
		}
		*result = (int16_t)val32;
	} else {
		if ((uint32_t)val32 > UINT16_MAX) {
			if (Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) {
				*result = (uint16_t)val32;
				return INT_UNSIGNED;
			}
			return DeeRT_ErrIntegerOverflowEx(self, 16,
			                                  DeeRT_ErrIntegerOverflowEx_F_ANYSIGN |
			                                  DeeRT_ErrIntegerOverflowEx_F_POSITIVE);
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
		return DeeRT_ErrIntegerOverflowS32(val32, INT8_MIN, INT8_MAX);
	}
return_value:
	*result = (int8_t)val32;
done:
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt8)(DeeObject *__restrict self,
                          uint8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if (error == INT_SIGNED) {
		if (val32 < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return DeeRT_ErrIntegerOverflowS32(val32, 0, UINT8_MAX);
#if INT_SIGNED != 0
		error = 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	if unlikely((uint32_t)val32 > UINT8_MAX) {
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) && error == 0)
			return DeeRT_ErrIntegerOverflowU32((uint32_t)val32, UINT8_MAX);
	}
	*result = (uint8_t)(uint32_t)val32;
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
		return DeeRT_ErrIntegerOverflowS32(val32, INT16_MIN, INT16_MAX);
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
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if (error == INT_SIGNED) {
		if (val32 < 0 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return DeeRT_ErrIntegerOverflowS32(val32, 0, UINT16_MAX);
#if INT_SIGNED != 0
		error = 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	if unlikely((uint32_t)val32 > UINT16_MAX) {
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) && error == 0)
			return DeeRT_ErrIntegerOverflowU32((uint32_t)val32, UINT16_MAX);
	}
	*result = (uint16_t)(uint32_t)val32;
	return error;
}




PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt8M1)(DeeObject *__restrict self,
                            uint8_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if (error == INT_SIGNED) {
		if (val32 < -1 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return DeeRT_ErrIntegerOverflowS32(val32, -1, UINT8_MAX);
#if INT_SIGNED != 0
		error = 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	if unlikely((uint32_t)val32 > UINT8_MAX) {
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) && error == 0)
			return DeeRT_ErrIntegerOverflowS64((uint32_t)val32, -1, UINT8_MAX);
	}
	*result = (uint8_t)(uint32_t)val32;
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt16M1)(DeeObject *__restrict self,
                             uint16_t *__restrict result) {
	int32_t val32;
	int error = DeeObject_Get32Bit(self, &val32);
	if (error == INT_SIGNED) {
		if (val32 < -1 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return DeeRT_ErrIntegerOverflowS32(val32, -1, UINT16_MAX);
#if INT_SIGNED != 0
		error = 0;
#endif /* INT_SIGNED != 0 */
	} else {
#if INT_SIGNED != 0
		ASSERT(error <= 0);
#else /* INT_SIGNED != 0 */
		if likely(error > 0)
			error = 0;
#endif /* INT_SIGNED == 0 */
	}
	if unlikely((uint32_t)val32 > UINT16_MAX) {
		if (!(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE) && error == 0)
			return DeeRT_ErrIntegerOverflowS64((uint32_t)val32, -1, UINT16_MAX);
	}
	*result = (uint16_t)(uint32_t)val32;
	return error;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt32M1)(DeeObject *__restrict self,
                             uint32_t *__restrict result) {
	int error = DeeObject_Get32Bit(self, (int32_t *)result);
	if (error == INT_SIGNED) {
		if ((int32_t)*result < -1 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE))
			return DeeRT_ErrIntegerOverflowS64((int32_t)*result, -1, UINT32_MAX);
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
(DCALL DeeObject_AsUInt64M1)(DeeObject *__restrict self,
                             uint64_t *__restrict result) {
	int error = DeeObject_Get64Bit(self, (int64_t *)result);
	if (error == INT_SIGNED) {
		if ((int64_t)*result < -1 && !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			Dee_int128_t val128, minval;
			Dee_uint128_t maxval;
			__hybrid_int128_set64(val128, (int64_t)*result);
			__hybrid_int128_setminusone(minval);
			__hybrid_uint128_set64(maxval, UINT64_MAX);
			return DeeRT_ErrIntegerOverflowS128(val128, minval, *(Dee_int128_t *)&maxval);
		}
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

INTDEF DeeIntObject dee_uint128_max;

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeObject_AsUInt128M1)(DeeObject *__restrict self,
                              Dee_uint128_t *__restrict result) {
	int error = DeeObject_Get128Bit(self, (Dee_int128_t *)result);
	if (error == INT_SIGNED) {
		if (__hybrid_int128_isneg(*(Dee_int128_t const *)result) &&
		    !__hybrid_int128_isminusone(*(Dee_int128_t const *)result) &&
		    !(Dee_TYPE(self)->tp_flags & TP_FTRUNCATE)) {
			return DeeRT_ErrIntegerOverflow(self, DeeInt_MinusOne,
			                                (DeeObject *)&dee_uint128_max,
			                                false);
		}
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

#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTuple)(DeeObject *self, /*Tuple*/ DeeObject *args) {
	return DeeObject_Call(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleKw)(DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	return DeeObject_CallKw(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}

DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTuple)(DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args) {
	return DeeObject_ThisCall(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleKw)(DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	return DeeObject_ThisCallKw(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}

DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args) {
	return DeeObject_CallInherited(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	return DeeObject_CallKwInherited(self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}

DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args) {
	return DeeObject_ThisCallInherited(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

DFUNDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_ThisCallTupleKwInherited)(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg, /*Tuple*/ DeeObject *args, DeeObject *kw) {
	return DeeObject_ThisCallKwInherited(self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args) {
	return DeeObject_TCall(tp_self, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_TCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *args, DeeObject *kw) {
	return DeeObject_TCallKw(tp_self, self, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TThisCallTuple)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args) {
	return DeeObject_TThisCall(tp_self, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TThisCallTupleKw)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *thisarg, DeeObject *args, DeeObject *kw) {
	return DeeObject_TThisCallKw(tp_self, self, thisarg, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */


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

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_CallInheritedf(/*inherit(always)*/ DREF DeeObject *self,
                         char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallInheritedf(self, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeObject_ThisCallInheritedf(/*inherit(always)*/ DREF DeeObject *self,
                             DeeObject *thisarg, char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VThisCallInheritedf(self, thisarg, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallInheritedf(/*inherit(always)*/ DREF DeeObject *self,
                          char const *__restrict format, va_list args) {
	DREF DeeObject *result = DeeObject_VCallf(self, format, args);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_VThisCallInheritedf(/*inherit(always)*/ DREF DeeObject *self, DeeObject *thisarg,
                              char const *__restrict format, va_list args) {
	DREF DeeObject *result = DeeObject_VThisCallf(self, thisarg, format, args);
	Dee_Decref_unlikely(self); /* *_unlikely because functions usually live until the module dies */
	return result;
}



PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeObject_ContainsAsBool(DeeObject *self, DeeObject *some_object) {
	DREF DeeObject *resultob;
	resultob = DeeObject_Contains(self, some_object);
	if unlikely(!resultob)
		goto err;
	return DeeObject_BoolInherited(resultob);
err:
	return -1;
}





PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_InplaceDeepCopy)(DREF DeeObject **__restrict p_self) {
	DeeObject *objcopy, *old_object;
	old_object = *p_self;
	ASSERT_OBJECT(old_object);
	objcopy = DeeObject_DeepCopy(old_object);
	if unlikely(!objcopy)
		goto err;
	Dee_Decref(old_object);
	*p_self = objcopy;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeObject_InplaceDeepCopyv)(/*in|out*/ DREF DeeObject **__restrict object_vector,
                                   size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&object_vector[i]))
			goto err;
	}
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceDeepCopyWithLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                          Dee_atomic_lock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_lock_acquire(p_lock);
	temp = *p_self;
	Dee_Incref(temp);
	Dee_atomic_lock_release(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_lock_acquire(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_lock_release(p_lock);
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_XInplaceDeepCopyWithLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                           Dee_atomic_lock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_lock_acquire(p_lock);
	temp = *p_self;
	if (!temp) {
		Dee_atomic_lock_release(p_lock);
		goto done;
	}
	Dee_Incref(temp);
	Dee_atomic_lock_release(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_lock_acquire(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_lock_release(p_lock);
	Dee_XDecref(temp);
done:
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_InplaceDeepCopyWithRWLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                            Dee_atomic_rwlock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_rwlock_read(p_lock);
	temp = *p_self;
	Dee_Incref(temp);
	Dee_atomic_rwlock_endread(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_rwlock_write(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_rwlock_endwrite(p_lock);
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_XInplaceDeepCopyWithRWLock)(/*in|out*/ DREF DeeObject **__restrict p_self,
                                             Dee_atomic_rwlock_t *__restrict p_lock) {
	DREF DeeObject *temp, *copy;
	(void)p_lock;

	/* Step #1: Extract the existing object. */
	Dee_atomic_rwlock_read(p_lock);
	temp = *p_self;
	if (!temp) {
		Dee_atomic_rwlock_endread(p_lock);
		goto done;
	}
	Dee_Incref(temp);
	Dee_atomic_rwlock_endread(p_lock);

	/* Step #2: Create a deep copy for it. */
	copy = DeeObject_DeepCopy(temp);
	Dee_Decref(temp);
	if unlikely(!copy)
		goto err;

	/* Step #3: Write back the newly created deep copy. */
	Dee_atomic_rwlock_write(p_lock);
	temp   = *p_self; /* Inherit */
	*p_self = copy;   /* Inherit */
	Dee_atomic_rwlock_endwrite(p_lock);
	Dee_XDecref(temp);
done:
	return 0;
err:
	return -1;
}




PUBLIC WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t
(DCALL DeeObject_Hashv)(DeeObject *const *__restrict object_vector,
                        size_t object_count) {
	size_t i;
	Dee_hash_t result;
	/* Check for special case: no objects, i.e.: an empty sequence */
	if unlikely(!object_count)
		return DEE_HASHOF_EMPTY_SEQUENCE;

	/* Important: when only a single object is given, our
	 * return value must be equal to `DeeObject_Hash()'.
	 *
	 * This is required so that:
	 * >> import hash from deemon;
	 * >> assert hash(42) == (42).operator hash();
	 * >> assert hash(42) == (42); */
	result = DeeObject_Hash(object_vector[0]);
	for (i = 1; i < object_count; ++i) {
		Dee_hash_t hsitem;
		hsitem = DeeObject_Hash(object_vector[i]);
		result = Dee_HashCombine(result, hsitem);
	}
	return result;
}

PUBLIC WUNUSED /*ATTR_PURE*/ ATTR_INS(1, 2) Dee_hash_t
(DCALL DeeObject_XHashv)(DeeObject *const *__restrict object_vector,
                         size_t object_count) {
	size_t i;
	Dee_hash_t result;
	/* Check for special case: no objects, i.e.: an empty sequence */
	if unlikely(!object_count)
		return DEE_HASHOF_EMPTY_SEQUENCE;

	/* Important: when only a single object is given, our
	 * return value must be equal to `DeeObject_Hash()'.
	 *
	 * This is required so that:
	 * >> import hash from deemon;
	 * >> assert hash(42) == (42).operator hash();
	 * >> assert hash(42) == (42); */
	{
		DeeObject *item = object_vector[0];
		result = item ? DeeObject_Hash(item) : DEE_HASHOF_UNBOUND_ITEM;
	}
	for (i = 1; i < object_count; ++i) {
		Dee_hash_t hsitem;
		DeeObject *item = object_vector[i];
		hsitem = item ? DeeObject_Hash(item) : DEE_HASHOF_UNBOUND_ITEM;
		result = Dee_HashCombine(result, hsitem);
	}
	return result;
}






/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed < key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed > key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKey)(DeeObject *lhs_keyed,
                             DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_Compare(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_CompareKeyEq)(DeeObject *lhs_keyed,
                               DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_CompareEq(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

/* Compare a pre-keyed `lhs_keyed' with `rhs' using the given `key' function
 * @return: == -1: `lhs_keyed != key(rhs)'
 * @return: == 0:  `lhs_keyed == key(rhs)'
 * @return: == 1:  `lhs_keyed != key(rhs)'
 * @return: == Dee_COMPARE_ERR: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_TryCompareKeyEq)(DeeObject *lhs_keyed,
                                  DeeObject *rhs, DeeObject *key) {
	int result;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err;
	result = DeeObject_TryCompareEq(lhs_keyed, rhs);
	Dee_Decref(rhs);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PUBLIC WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DeeObject_GetRangeBeginIndex(DeeObject *self, Dee_ssize_t start, DeeObject *end) {
	DREF DeeObject *result;
	DREF DeeObject *start_ob;
	Dee_ssize_t end_index;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index))
		return DeeObject_GetRangeIndex(self, start, end_index);
	if (DeeNone_Check(end))
		return DeeObject_GetRangeIndexN(self, start);
	start_ob = DeeInt_NewSSize(start);
	if unlikely(!start_ob)
		goto err;
	result = DeeObject_GetRange(self, start_ob, end);
	Dee_Decref(start_ob);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetRangeEndIndex(DeeObject *self, DeeObject *start, Dee_ssize_t end) {
	DREF DeeObject *result;
	DREF DeeObject *end_ob;
	Dee_ssize_t start_index;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index))
		return DeeObject_GetRangeIndex(self, start_index, end);
	if (DeeNone_Check(start))
		return DeeObject_GetRangeIndex(self, 0, end);
	end_ob = DeeInt_NewSSize(end);
	if unlikely(!end_ob)
		goto err;
	result = DeeObject_GetRange(self, start, end_ob);
	Dee_Decref(end_ob);
	return result;
err:
	return NULL;
}


PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeObject_DelRangeBeginIndex)(DeeObject *self, Dee_ssize_t start, DeeObject *end) {
	int result;
	DREF DeeObject *start_ob;
	Dee_ssize_t end_index;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index))
		return DeeObject_DelRangeIndex(self, start, end_index);
	if (DeeNone_Check(end))
		return DeeObject_DelRangeIndexN(self, start);
	start_ob = DeeInt_NewSSize(start);
	if unlikely(!start_ob)
		goto err;
	result = DeeObject_DelRange(self, start_ob, end);
	Dee_Decref(start_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelRangeEndIndex)(DeeObject *self, DeeObject *start, Dee_ssize_t end) {
	int result;
	DREF DeeObject *end_ob;
	Dee_ssize_t start_index;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index))
		return DeeObject_DelRangeIndex(self, start_index, end);
	if (DeeNone_Check(start))
		return DeeObject_DelRangeIndex(self, 0, end);
	end_ob = DeeInt_NewSSize(end);
	if unlikely(!end_ob)
		goto err;
	result = DeeObject_DelRange(self, start, end_ob);
	Dee_Decref(end_ob);
	return result;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 3, 4)) int
(DCALL DeeObject_SetRangeBeginIndex)(DeeObject *self, Dee_ssize_t start,
                                     DeeObject *end, DeeObject *values) {
	int result;
	DREF DeeObject *start_ob;
	Dee_ssize_t end_index;
	if (DeeInt_Check(end) && DeeInt_TryAsSSize(end, &end_index))
		return DeeObject_SetRangeIndex(self, start, end_index, values);
	if (DeeNone_Check(end))
		return DeeObject_SetRangeIndexN(self, start, values);
	start_ob = DeeInt_NewSSize(start);
	if unlikely(!start_ob)
		goto err;
	result = DeeObject_SetRange(self, start_ob, end, values);
	Dee_Decref(start_ob);
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetRangeEndIndex)(DeeObject *self, DeeObject *start,
                                   Dee_ssize_t end, DeeObject *values) {
	int result;
	DREF DeeObject *end_ob;
	Dee_ssize_t start_index;
	if (DeeInt_Check(start) && DeeInt_TryAsSSize(start, &start_index))
		return DeeObject_SetRangeIndex(self, start_index, end, values);
	if (DeeNone_Check(start))
		return DeeObject_SetRangeIndex(self, 0, end, values);
	end_ob = DeeInt_NewSSize(end);
	if unlikely(!end_ob)
		goto err;
	result = DeeObject_SetRange(self, start, end_ob, values);
	Dee_Decref(end_ob);
	return result;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_INVOKE_UTILS_C */
