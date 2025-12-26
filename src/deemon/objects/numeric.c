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
#ifndef GUARD_DEEMON_OBJECTS_NUMERIC_C
#define GUARD_DEEMON_OBJECTS_NUMERIC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none-operator.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/int128.h>
/**/

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"
/**/

#include <stdint.h> /* int8_t */

#ifdef CONFIG_HAVE_MATH_H
#include <math.h>
#endif /* CONFIG_HAVE_MATH_H */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asflt(DeeObject *__restrict self) {
	double result;
	if (DeeObject_AsDouble(self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass8(DeeObject *__restrict self) {
	int8_t result;
	if (DeeObject_AsInt8(self, &result))
		goto err;
	return DeeInt_NewInt8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass16(DeeObject *__restrict self) {
	int16_t result;
	if (DeeObject_AsInt16(self, &result))
		goto err;
	return DeeInt_NewInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass32(DeeObject *__restrict self) {
	int32_t result;
	if (DeeObject_AsInt32(self, &result))
		goto err;
	return DeeInt_NewInt32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass64(DeeObject *__restrict self) {
	int64_t result;
	if (DeeObject_AsInt64(self, &result))
		goto err;
	return DeeInt_NewInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass128(DeeObject *__restrict self) {
	Dee_int128_t result;
	if (DeeObject_AsInt128(self, &result))
		goto err;
	return DeeInt_NewInt128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu8(DeeObject *__restrict self) {
	uint8_t result;
	if (DeeObject_AsUInt8(self, &result))
		goto err;
	return DeeInt_NewUInt8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu16(DeeObject *__restrict self) {
	uint16_t result;
	if (DeeObject_AsUInt16(self, &result))
		goto err;
	return DeeInt_NewUInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu32(DeeObject *__restrict self) {
	uint32_t result;
	if (DeeObject_AsUInt32(self, &result))
		goto err;
	return DeeInt_NewUInt32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu64(DeeObject *__restrict self) {
	uint64_t result;
	if (DeeObject_AsUInt64(self, &result))
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu128(DeeObject *__restrict self) {
	Dee_uint128_t result;
	if (DeeObject_AsUInt128(self, &result))
		goto err;
	return DeeInt_NewUInt128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed8(DeeObject *__restrict self) {
	int8_t result;
	if unlikely(DeeObject_Get8Bit(self, &result) < 0)
		goto err;
	return DeeInt_NewInt8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned8(DeeObject *__restrict self) {
	uint8_t result;
	if unlikely(DeeObject_Get8Bit(self, (int8_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed16(DeeObject *__restrict self) {
	int16_t result;
	if unlikely(DeeObject_Get16Bit(self, &result) < 0)
		goto err;
	return DeeInt_NewInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_Get16Bit(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed32(DeeObject *__restrict self) {
	int32_t result;
	if unlikely(DeeObject_Get32Bit(self, &result) < 0)
		goto err;
	return DeeInt_NewInt32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_Get32Bit(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed64(DeeObject *__restrict self) {
	int64_t result;
	if unlikely(DeeObject_Get64Bit(self, &result) < 0)
		goto err;
	return DeeInt_NewInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_Get64Bit(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed128(DeeObject *__restrict self) {
	Dee_int128_t result;
	if unlikely(DeeObject_Get128Bit(self, &result) < 0)
		goto err;
	return DeeInt_NewInt128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned128(DeeObject *__restrict self) {
	Dee_uint128_t result;
	if unlikely(DeeObject_Get128Bit(self, (Dee_int128_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_Get16Bit(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt16(BSWAP16(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_Get32Bit(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt32(BSWAP32(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_Get64Bit(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewUInt64(BSWAP64(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap128(DeeObject *__restrict self) {
	Dee_uint128_t result;
	if unlikely(DeeObject_Get128Bit(self, (Dee_int128_t *)&result) < 0)
		goto err;
	__hybrid_uint128_bswap(result);
	return DeeInt_NewUInt128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_Get16Bit(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewInt16((int16_t)BSWAP16(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_Get32Bit(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewInt32((int64_t)BSWAP32(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_Get64Bit(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewInt64((int64_t)BSWAP64(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap128(DeeObject *__restrict self) {
	Dee_int128_t result;
	if unlikely(DeeObject_Get128Bit(self, (Dee_int128_t *)&result) < 0)
		goto err;
	__hybrid_int128_bswap(result);
	return DeeInt_NewInt128(result);
err:
	return NULL;
}


INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_popcount(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_ffs(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_fls(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_parity(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_ctz(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_ct1(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_msb(DeeIntObject *__restrict self);
INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_bitmask(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL int_get_nth(DeeIntObject *__restrict self);

#ifdef CONFIG_HAVE_FPU
#ifdef CONFIG_HAVE_trunc
#define HAVE_float_get_trunc
INTDEF WUNUSED NONNULL((1)) DREF DeeFloatObject *DCALL float_get_trunc(DeeFloatObject *__restrict self);
#endif /* CONFIG_HAVE_trunc */
#if defined(CONFIG_HAVE_IEEE754) || defined(CONFIG_HAVE_isnan)
#define HAVE_float_get_isnan
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_get_isnan(DeeFloatObject *__restrict self);
#endif /* CONFIG_HAVE_IEEE754 || CONFIG_HAVE_isnan */
#if defined(CONFIG_HAVE_IEEE754) || defined(CONFIG_HAVE_isinf)
#define HAVE_float_get_isinf
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_get_isinf(DeeFloatObject *__restrict self);
#endif /* CONFIG_HAVE_IEEE754 || CONFIG_HAVE_isinf */
#if defined(CONFIG_HAVE_IEEE754) || defined(CONFIG_HAVE_isfinite) || defined(CONFIG_HAVE_finite)
#define HAVE_float_get_isfinite
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_get_isfinite(DeeFloatObject *__restrict self);
#endif /* CONFIG_HAVE_IEEE754 || CONFIG_HAVE_isfinite || CONFIG_HAVE_finite */
#if defined(CONFIG_HAVE_IEEE754) || defined(CONFIG_HAVE_isnormal)
#define HAVE_float_get_isnormal
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_get_isnormal(DeeFloatObject *__restrict self);
#endif /* CONFIG_HAVE_IEEE754 || CONFIG_HAVE_isnormal */
#endif /* CONFIG_HAVE_FPU */

#ifndef HAVE_float_get_trunc
#define float_get_trunc(self) ((DREF DeeFloatObject *)DeeObject_GetAttrString((DeeObject *)(self), "trunc"))
#endif /* !HAVE_float_get_trunc */
#ifndef HAVE_float_get_isnan
#define float_get_isnan(self) DeeObject_GetAttrString((DeeObject *)(self), "isnan")
#endif /* !HAVE_float_get_isnan */
#ifndef HAVE_float_get_isinf
#define float_get_isinf(self) DeeObject_GetAttrString((DeeObject *)(self), "isinf")
#endif /* !HAVE_float_get_isinf */
#ifndef HAVE_float_get_isfinite
#define float_get_isfinite(self) DeeObject_GetAttrString((DeeObject *)(self), "isfinite")
#endif /* !HAVE_float_get_isfinite */
#ifndef HAVE_float_get_isnormal
#define float_get_isnormal(self) DeeObject_GetAttrString((DeeObject *)(self), "isnormal")
#endif /* !HAVE_float_get_isnormal */




PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_popcount(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_popcount(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_ffs(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_ffs(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_fls(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_fls(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_parity(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_parity(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_ctz(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_ctz(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_ct1(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_ct1(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_msb(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_msb(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_bitmask(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_bitmask(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_abs(DeeObject *__restrict self) {
	int isneg;
	isneg = DeeObject_CmpLoAsBool(self, DeeInt_Zero);
	if unlikely(isneg < 0)
		goto err;
	return isneg ? DeeObject_Neg(self)
	             : DeeObject_Pos(self);
err:
	return NULL;
}

#define DeeObject_Float(self) DeeObject_New(&DeeFloat_Type, 1, (DeeObject *const *)&(self))

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_isfloat(DeeObject *__restrict self) {
	DREF DeeFloatObject *flt_val, *flt_trunc_val, *int_as_flt_val;
	DREF DeeIntObject *int_val;
	DeeTypeObject *tp = Dee_TYPE(self);
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, tp);
	do {
		bool has_float, has_int;
		has_float = DeeType_HasPrivateOperator(tp, OPERATOR_FLOAT);
		has_int   = DeeType_HasPrivateOperator(tp, OPERATOR_INT);
		if (has_float || has_int) {
			int error;
			if (!has_float)
				return_false;
			if (!has_int)
				return_true;

			/* >> local fltVal = this.operator float(); */
			flt_val = (DREF DeeFloatObject *)DeeObject_Float(self);
			if unlikely(!flt_val)
				goto err;

			/* >> if (fltVal != fltVal.trunc)
			 * >>     return true; */
			flt_trunc_val = float_get_trunc(flt_val);
			if unlikely(!flt_trunc_val)
				goto err_flt_val;
			error = DeeObject_CmpNeAsBool((DeeObject *)flt_val,
			                            (DeeObject *)flt_trunc_val);
			Dee_Decref(flt_trunc_val);
			if unlikely(error < 0)
				goto err_flt_val;
			if (error) {
				Dee_Decref(flt_val);
				return_true;
			}

			/* >> intVal = this.operator int(); */
			int_val = (DREF DeeIntObject *)DeeObject_Int(self);
			if unlikely(!int_val)
				goto err_flt_val;

			/* >> return fltVal != intVal.operator float(); */
			int_as_flt_val = (DREF DeeFloatObject *)DeeObject_Float(int_val);
			Dee_Decref(int_val);
			if unlikely(!int_as_flt_val)
				goto err_flt_val;
			error = DeeObject_CmpNeAsBool((DeeObject *)flt_val,
			                            (DeeObject *)int_as_flt_val);
			Dee_Decref(int_as_flt_val);
			if unlikely(error < 0)
				goto err_flt_val;
			Dee_Decref(flt_val);
			return_bool(error);
		}
	} while ((tp = DeeTypeMRO_Next(&mro, tp)) != NULL);
	return_false;
err_flt_val:
	Dee_Decref(flt_val);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeObject_IsFloat(DeeObject *__restrict self) {
	DREF DeeObject *attr;
	attr = DeeObject_GetAttr(self, (DeeObject *)&str_isfloat);
	if unlikely(!attr)
		goto err;
	return DeeObject_BoolInherited(attr);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_trunc(DeeObject *__restrict self) {
	int error;
	DREF DeeObject *res, *result;

	/* >> if (!this.isfloat)
	 * >>     return this; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_(self);

	/* >> local res = (int from deemon)this; */
	res = DeeObject_Int(self);
	if unlikely(!res)
		goto err;

	/* >> if (this == res)
	 * >>     return this; */
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		Dee_Decref(res);
		return_reference_(self);
	}

	/* >> return (type this)res; */
	result = DeeObject_New(Dee_TYPE(self), 1, &res);
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_floor(DeeObject *__restrict self) {
	int error;
	DREF DeeObject *res, *result;

	/* >> if (!this.isfloat)
	 * >>     return this; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_(self);

	/* >> local res = (int from deemon)this; */
	res = DeeObject_Int(self);
	if unlikely(!res)
		goto err;

	/* >> if (this == res)
	 * >>     return this; */
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		Dee_Decref(res);
		return_reference_(self);
	}

	/* >> if (this < 0)
	 * >>     --res; */
	error = DeeObject_CmpLoAsBool(self, DeeInt_Zero);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		if (DeeObject_Dec(&res))
			goto err_res;
	}

	/* >> return (type this)res; */
	result = DeeObject_New(Dee_TYPE(self), 1, &res);
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_ceil(DeeObject *__restrict self) {
	int error;
	DREF DeeObject *res, *result;

	/* >> if (!this.isfloat)
	 * >>     return this; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_(self);

	/* >> local res = (int from deemon)this; */
	res = DeeObject_Int(self);
	if unlikely(!res)
		goto err;

	/* >> if (this == res)
	 * >>     return this; */
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		Dee_Decref(res);
		return_reference_(self);
	}

	/* >> if (this > 0)
	 * >>     ++res; */
	error = DeeObject_CmpGrAsBool(self, DeeInt_Zero);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		if (DeeObject_Inc(&res))
			goto err_res;
	}

	/* >> return (type this)res; */
	result = DeeObject_New(Dee_TYPE(self), 1, &res);
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE Dee_DEFINE_FLOAT(flt_half, 0.5);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_round(DeeObject *__restrict self) {
	int error;
	DREF DeeObject *res, *result, *delta;

	/* >> if (!this.isfloat)
	 * >>     return this; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_(self);

	/* >> local res = (int from deemon)this; */
	res = DeeObject_Int(self);
	if unlikely(!res)
		goto err;

	/* >> if (this == res)
	 * >>     return this; */
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		Dee_Decref(res);
		return_reference_(self);
	}

	/* >> if (this > 0) { ... */
	error = DeeObject_CmpGrAsBool(self, DeeInt_Zero);
	if unlikely(error < 0)
		goto err_res;
	if (error) {
		/* >> ...
		 * >> {
		 * >>     local delta = this - res;
		 * >>     if (delta >= 0.5)
		 * >>         ++res;
		 */
		delta = DeeObject_Sub(self, res);
		if unlikely(!delta)
			goto err_res;
		error = DeeObject_CmpGeAsBool(delta, (DeeObject *)&flt_half);
		Dee_Decref(delta);
		if unlikely(error < 0)
			goto err_res;
		if (error) {
			if (DeeObject_Inc(&res))
				goto err_res;
		}
	} else {
		/* >>     ...
		 * >> } else {
		 * >>     local delta = res - this;"
		 * >>     if (delta > 0.5)
		 * >>         --res;
		 * >> } */
		delta = DeeObject_Sub(res, self);
		if unlikely(!delta)
			goto err_res;
		error = DeeObject_CmpGrAsBool(delta, (DeeObject *)&flt_half);
		Dee_Decref(delta);
		if unlikely(error < 0)
			goto err_res;
		if (error) {
			if (DeeObject_Dec(&res))
				goto err_res;
		}
	}

	/* >> return (type this)res; */
	result = DeeObject_New(Dee_TYPE(self), 1, &res);
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_isnan(DeeObject *__restrict self) {
	DREF DeeObject *res, *result;
	int error;

	/* >> if (!this.isfloat)
	 * >>     return false; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_false;

	res = DeeObject_Float(self);
	if unlikely(!res)
		goto err;
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (!error) {
		result = DeeBool_NewFalse();
	} else {
		result = float_get_isnan((DeeFloatObject *)res);
	}
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_isinf(DeeObject *__restrict self) {
	DREF DeeObject *res, *result;
	int error;

	/* >> if (!this.isfloat)
	 * >>     return false; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_false;

	res = DeeObject_Float(self);
	if unlikely(!res)
		goto err;
	error = DeeObject_CmpEqAsBool(self, res);
	if unlikely(error < 0)
		goto err_res;
	if (!error) {
		result = DeeBool_NewFalse();
	} else {
		result = float_get_isinf((DeeFloatObject *)res);
	}
	Dee_Decref(res);
	return result;
err_res:
	Dee_Decref(res);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_isfinite(DeeObject *__restrict self) {
	DREF DeeObject *res, *result;
	int error;

	/* >> if (!this.isfloat)
	 * >>     return true; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_true;

	res = DeeObject_Float(self);
	if unlikely(!res)
		goto err;
	result = float_get_isfinite((DeeFloatObject *)res);
	Dee_Decref(res);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_isnormal(DeeObject *__restrict self) {
	DREF DeeObject *res, *result;
	int error;

	/* >> if (!this.isfloat)
	 * >>     return true; */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_true;

	res = DeeObject_Float(self);
	if unlikely(!res)
		goto err;
	result = float_get_isnormal((DeeFloatObject *)res);
	Dee_Decref(res);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_nth(DeeObject *__restrict self) {
	DREF DeeObject *intob, *result;
	intob = DeeObject_Int(self);
	if unlikely(!intob)
		goto err;
	result = int_get_nth((DeeIntObject *)intob);
	Dee_Decref(intob);
	return result;
err:
	return NULL;
}


PRIVATE struct type_getset tpconst numeric_getsets[] = {
	TYPE_GETTER(STR_int, &DeeObject_Int,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "Return @this number as an integer, truncating any decimal digits"),
	TYPE_GETTER(STR_float, &numeric_asflt,
	            "->?Dfloat\n"
	            "#tNotImplemented{@this number does not implement ${operator float}}"
	            "Return @this number as a floating point value"),
	TYPE_GETTER("s8", &numeric_ass8,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${-128 ... 127}"),
	TYPE_GETTER("s16", &numeric_ass16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${-32768 ... 32767}"),
	TYPE_GETTER("s32", &numeric_ass32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${-2147483648 ... 2147483647}"),
	TYPE_GETTER("s64", &numeric_ass64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${-9223372036854775808 ... 9223372036854775807}"),
	TYPE_GETTER("s128", &numeric_ass128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${-170141183460469231731687303715884105728 ... 170141183460469231731687303715884105727}"),
	TYPE_GETTER("u8", &numeric_asu8,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${0x0 ... 0xff}"),
	TYPE_GETTER("u16", &numeric_asu16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffff}"),
	TYPE_GETTER("u32", &numeric_asu32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffff}"),
	TYPE_GETTER("u64", &numeric_asu64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffffffffffff}"),
	TYPE_GETTER("u128", &numeric_asu128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffffffffffffffffffffffffffff}"),
	TYPE_GETTER("signed8", &numeric_signed8,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${-128 ... 127}, return the same value as @s8\n"
	            "Otherwise, an integer in the range ${128 ... 255} is returned as ${256 - this}\n"
	            "This is the same behavior as casting an 8-bit integer to becoming signed, "
	            /**/ "by re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed16", &numeric_signed16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${-32768 ... 32767}, return the same value as @s16\n"
	            "Otherwise, an integer in the range ${32768 ... 65535} is returned as ${65536 - this}\n"
	            "This is the same behavior as casting an 16-bit integer to becoming signed, "
	            /**/ "by re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed32", &numeric_signed32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${-2147483648 ... 2147483647}, return the same value as @s32\n"
	            "Otherwise, an integer in the range ${2147483648 ... 4294967295} is returned as ${4294967296 - this}\n"
	            "This is the same behavior as casting an 32-bit integer to becoming signed, "
	            /**/ "by re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed64", &numeric_signed64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${-9223372036854775808 ... 9223372036854775807}, return the same value as @s64\n"
	            "Otherwise, an integer in the range ${9223372036854775808 ... 18446744073709551615} is returned as ${18446744073709551616 - this}\n"
	            "This is the same behavior as casting an 64-bit integer to becoming signed, "
	            /**/ "by re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed128", &numeric_signed128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${-170141183460469231731687303715884105728 ... 170141183460469231731687303715884105727}, return the same value as @s128\n"
	            "Otherwise, an integer in the range ${170141183460469231731687303715884105728 ... 340282366920938463463374607431768211455} is returned as ${340282366920938463463374607431768211456 - this}\n"
	            "This is the same behavior as casting an 128-bit integer to becoming signed, "
	            /**/ "by re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("unsigned8", &numeric_unsigned8,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${0 ... 255}, return the same value as @u8\n"
	            "Otherwise, an integer in the range ${-128 ... -1} is returned as ${256 + this}\n"
	            "This is the same behavior as casting an 8-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned16", &numeric_unsigned16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${0 ... 65535}, return the same value as @u16\n"
	            "Otherwise, an integer in the range ${-32768 ... -1} is returned as ${65536 + this}\n"
	            "This is the same behavior as casting an 16-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned32", &numeric_unsigned32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${0 ... 4294967295}, return the same value as @u32\n"
	            "Otherwise, an integer in the range ${-2147483648 ... -1} is returned as ${4294967296 + this}\n"
	            "This is the same behavior as casting an 32-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned64", &numeric_unsigned64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${0 ... 18446744073709551615}, return the same value as @u64\n"
	            "Otherwise, an integer in the range ${-9223372036854775808 ... -1} is returned as ${18446744073709551616 + this}\n"
	            "This is the same behavior as casting an 64-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned128", &numeric_unsigned128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "When @this number fits the range ${0 ... 340282366920938463463374607431768211455}, return the same value as @u128\n"
	            "Otherwise, an integer in the range ${-170141183460469231731687303715884105728 ... -1} is returned as ${340282366920938463463374607431768211456 + this}\n"
	            "This is the same behavior as casting an 128-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),

	TYPE_GETTER("swap16", &numeric_swap16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Take the integer generated by ?#u16 and exchange its low and high 8 bits"),
	TYPE_GETTER("swap32", &numeric_swap32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap16, but ?#u32 is taken as input, and the 8-bit tuples $abcd are re-arranged as $dcba"),
	TYPE_GETTER("swap64", &numeric_swap64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap16, but ?#u64 is taken as input, and the 8-bit tuples $abcdefgh are re-arranged as $hgfedcba"),
	TYPE_GETTER("swap128", &numeric_swap128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap16, but ?#u128 is taken as input, and the 8-bit tuples $abcdefghijklmnop are re-arranged as $ponmlkjihgfedcba"),
	TYPE_GETTER("sswap16", &numeric_sswap16,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Same as ${this.swap16.signed16}"),
	TYPE_GETTER("sswap32", &numeric_sswap32,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Same as ${this.swap32.signed32}"),
	TYPE_GETTER("sswap64", &numeric_sswap64,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Same as ${this.swap64.signed64}"),
	TYPE_GETTER("sswap128", &numeric_sswap128,
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Same as ${this.swap128.signed128}"),

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LE_SEL(cast, swap) cast
#define BE_SEL(cast, swap) swap
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define LE_SEL(cast, swap) swap
#define BE_SEL(cast, swap) cast
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	TYPE_GETTER("leswap16", &LE_SEL(numeric_unsigned16, numeric_swap16),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap16, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned16"),
	TYPE_GETTER("leswap32", &LE_SEL(numeric_unsigned32, numeric_swap32),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap32, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned32"),
	TYPE_GETTER("leswap64", &LE_SEL(numeric_unsigned64, numeric_swap64),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap64, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned64"),
	TYPE_GETTER("leswap128", &LE_SEL(numeric_unsigned128, numeric_swap128),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap128, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned128"),
	TYPE_GETTER("beswap16", &BE_SEL(numeric_unsigned16, numeric_swap16),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap16, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned16"),
	TYPE_GETTER("beswap32", &BE_SEL(numeric_unsigned32, numeric_swap32),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap32, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned32"),
	TYPE_GETTER("beswap64", &BE_SEL(numeric_unsigned64, numeric_swap64),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap64, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned64"),
	TYPE_GETTER("beswap128", &BE_SEL(numeric_unsigned128, numeric_swap128),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#swap128, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned128"),
	TYPE_GETTER("lesswap16", &LE_SEL(numeric_signed16, numeric_sswap16),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap16, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed16"),
	TYPE_GETTER("lesswap32", &LE_SEL(numeric_signed32, numeric_sswap32),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap32, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed32"),
	TYPE_GETTER("lesswap64", &LE_SEL(numeric_signed64, numeric_sswap64),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap64, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed64"),
	TYPE_GETTER("lesswap128", &LE_SEL(numeric_signed128, numeric_sswap128),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap128, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed128"),
	TYPE_GETTER("besswap16", &BE_SEL(numeric_signed16, numeric_sswap16),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap16, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed16"),
	TYPE_GETTER("besswap32", &BE_SEL(numeric_signed32, numeric_sswap32),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap32, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed32"),
	TYPE_GETTER("besswap64", &BE_SEL(numeric_signed64, numeric_sswap64),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap64, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed64"),
	TYPE_GETTER("besswap128", &BE_SEL(numeric_signed128, numeric_sswap128),
	            "->?Dint\n"
	            "#tNotImplemented{@this number does not implement ${operator int}}"
	            "#tIntegerOverflow{The value of @this number is outside the requested range}"
	            "Similar to ?#sswap128, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed128"),
#undef LE_SEL
#undef BE_SEL

	/* Binary property helper functions */
	TYPE_GETTER("popcount", &numeric_get_popcount,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this < 0}}"
	            "Return the number of 1-bits in this integer. Implemented as:\n"
	            "${"
	            /**/ "property popcount: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).popcount;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("ffs", &numeric_get_ffs,
	            "->?Dint\n"
	            "FindFirstSet: same as ?#ctz +1, but returns $0 when ${this == 0}. Implemented as:\n"
	            "${"
	            /**/ "property ffs: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).ffs;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("fls", &numeric_get_fls,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this < 0}}"
	            "FindFirstSet: same as ?#msb +1, but returns $0 when ${this == 0}. Implemented as:\n"
	            "${"
	            /**/ "property fls: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).fls;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("parity", &numeric_get_parity,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this < 0}}"
	            "Return $0 or $1 indivative of the even/odd parity of @this. Same as ${this.popcount % 2}. Implemented as:\n"
	            "${"
	            /**/ "property parity: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).parity;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("ctz", &numeric_get_ctz,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this == 0}}"
	            "CountTrailingZeros: return the number of trailing zero-bits:\n"
	            "${"
	            /**/ "local n = this.ctz;\n"
	            /**/ "assert this == (this >> n) << n;"
	            "}\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "property ctz: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).ctz;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("ct1", &numeric_get_ct1,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this == -1}}"
	            "CountTrailingOnes: return the number of trailing 1-bits:\n"
	            "${"
	            /**/ "local n = this.ct1;\n"
	            /**/ "assert this == ((this >> n) << n) | ((1 << ct1) - 1);"
	            "}\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "property ct1: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).ct1;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("msb", &numeric_get_msb,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this <= 0}}"
	            "MostSignificantBit: return the index of the most significant 1-bit (0-based):\n"
	            "${"
	            /**/ "assert (this >> this.msb) == 1;"
	            "}\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "property msb: int {\n"
	            /**/ "	get(): int {\n"
	            /**/ "		return ((int)this).msb;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("bitmask", &numeric_get_bitmask,
	            "->?Dint\n"
	            "#tIntegerOverflow{When ${this < 0}}"
	            "Same as ${(1 << this) - 1}"),
	TYPE_GETTER("abs", &numeric_get_abs,
	            "->?.\n"
	            "Return the absolute value of @this. Implemented as:\n"
	            "${"
	            /**/ "property abs: Numeric {\n"
	            /**/ "	get(): Numeric {\n"
	            /**/ "		if (this < 0)\n"
	            /**/ "			return -this;\n"
	            /**/ "		return +this;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER(STR_isfloat, &numeric_get_isfloat,
	            "->?Dbool\n"
	            "Returns ?t if @this number might have a decimal point. Implemented as:\n"
	            "${"
	            /**/ "property isfloat: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		for (local tp = type(this).__mro__) {\n"
	            /**/ "			local hasFloat = tp.hasprivateoperator(\"float\"));\n"
	            /**/ "			local hasInt   = tp.hasprivateoperator(\"int\"));\n"
	            /**/ "			if (hasFloat || hasInt) {\n"
	            /**/ "				if (!hasFloat)\n"
	            /**/ "					return false;\n"
	            /**/ "				if (!hasInt)\n"
	            /**/ "					return true;\n"
	            /**/ "				local fltVal = this.operator float();\n"
	            /**/ "				if (fltVal != fltVal.trunc)\n"
	            /**/ "					return true;\n"
	            /**/ "				local intVal = this.operator int();\n"
	            /**/ "				return fltVal != intVal.operator float();\n"
	            /**/ "			}\n"
	            /**/ "		}\n"
	            /**/ "		return false;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("trunc", &numeric_get_trunc,
	            "->?.\n"
	            "Return the value of @this rounded towards zero. Implemented as:\n"
	            "${"
	            /**/ "property trunc: Numeric {\n"
	            /**/ "	get(): Numeric {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return this;\n"
	            /**/ "		local res = (int from deemon)this;\n"
	            /**/ "		if (this == res)\n"
	            /**/ "			return this;\n"
	            /**/ "		return (type this)res;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("floor", &numeric_get_floor,
	            "->?.\n"
	            "Return the value of @this rounded downwards. Implemented as:\n"
	            "${"
	            /**/ "property floor: Numeric {\n"
	            /**/ "	get(): Numeric {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return this;\n"
	            /**/ "		local res = (int from deemon)this;\n"
	            /**/ "		if (this == res)\n"
	            /**/ "			return this;\n"
	            /**/ "		if (this < 0)\n"
	            /**/ "			--res;\n"
	            /**/ "		return (type this)res;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("ceil", &numeric_get_ceil,
	            "->?.\n"
	            "Return the value of @this rounded upwards. Implemented as:\n"
	            "${"
	            /**/ "property ceil: Numeric {\n"
	            /**/ "	get(): Numeric {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return this;\n"
	            /**/ "		local res = (int from deemon)this;\n"
	            /**/ "		if (this == res)\n"
	            /**/ "			return this;\n"
	            /**/ "		if (this > 0)\n"
	            /**/ "			++res;\n"
	            /**/ "		return (type this)res;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("round", &numeric_get_round,
	            "->?.\n"
	            "Return the value of @this rounded towards the closest whole number. Implemented as:\n"
	            "${"
	            /**/ "property round: Numeric {\n"
	            /**/ "	get(): Numeric {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return this;\n"
	            /**/ "		local res = (int from deemon)this;\n"
	            /**/ "		if (this == res)\n"
	            /**/ "			return this;\n"
	            /**/ "		if (this > 0) {\n"
	            /**/ "			local delta = this - res;\n"
	            /**/ "			if (delta >= 0.5)\n"
	            /**/ "				++res;\n"
	            /**/ "		} else {\n"
	            /**/ "			local delta = res - this;\n"
	            /**/ "			if (delta > 0.5)\n"
	            /**/ "				--res;\n"
	            /**/ "		}\n"
	            /**/ "		return (type this)res;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isnan", &numeric_get_isnan,
	            "->?Dbool\n"
	            "Return ?t if @this number is not-a-number. Implemented as:\n"
	            "${"
	            /**/ "property isnan: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return false;\n"
	            /**/ "		local res = (float from deemon)this;\n"
	            /**/ "		return this == res && res.isnan;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isinf", &numeric_get_isinf,
	            "->?Dbool\n"
	            "Return ?t if @this number is infinite. Implemented as:\n"
	            "${"
	            /**/ "property isinf: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return false;\n"
	            /**/ "		local res = (float from deemon)this;\n"
	            /**/ "		return this == res && res.isinf;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isfinite", &numeric_get_isfinite,
	            "->?Dbool\n"
	            "Return ?t if @this number is finite. Implemented as:\n"
	            "${"
	            /**/ "property isfinite: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return true;\n"
	            /**/ "		local res = (float from deemon)this;\n"
	            /**/ "		return res.isfinite;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isnormal", &numeric_get_isnormal,
	            "->?Dbool\n"
	            "Return ?t if @this number is normal. Implemented as:\n"
	            "${"
	            /**/ "property isnormal: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		if (!this.isfloat)\n"
	            /**/ "			return true;\n"
	            /**/ "		local res = (float from deemon)this;\n"
	            /**/ "		return res.isnormal;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("nth", &numeric_get_nth,
	            "->?Dstring\n"
	            "Convert @this number into an integer and call ?Anth?Dint"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_get_class_isfloat(DeeTypeObject *__restrict self) {
	DREF DeeObject *instance, *result;
	DeeObject *args[1];
	args[0]  = (DeeObject *)&flt_half;
	instance = DeeObject_New(self, 1, args);
	if unlikely(!instance)
		goto err;
	result = DeeObject_GetAttr(instance, (DeeObject *)&str_isfloat);
	Dee_Decref(instance);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst numeric_class_getsets[] = {
	TYPE_GETTER(STR_isfloat, &numeric_get_class_isfloat,
	            "->?Dbool\n"
	            "Returns ?t if instances of @this number might have a decimal point. Implemented as:\n"
	            "${"
	            /**/ "class property isfloat: bool {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		return ((this)0.5).isfloat;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETSET_END
};



DOC_DEF(numeric_hex_doc,
        "(precision=!0)->?Dstring\n"
        "Short-hand alias for ${this.tostr(radix: 16, precision: precision, mode: \"##\")} (s.a. ?#tostr)");
DOC_DEF(numeric_bin_doc,
        "(precision=!0)->?Dstring\n"
        "Short-hand alias for ${this.tostr(radix: 2, precision: precision, mode: \"##\")} (s.a. ?#tostr)");
DOC_DEF(numeric_oct_doc,
        "(precision=!0)->?Dstring\n"
        "Short-hand alias for ${this.tostr(radix: 8, precision: precision, mode: \"##\")} (s.a. ?#tostr)");
DOC_DEF(numeric_divmod_doc,
        "(y:?.)->?T2?.?.\n"
        "Devide+modulo. Alias for ${(this / y, this % y)}");

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tostr(DeeIntObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_tostr(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeIntObject *as_int;
	as_int = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!as_int)
		goto err;
	result = int_tostr(as_int, argc, argv, kw);
	Dee_Decref(as_int);
	return result;
err:
	return NULL;
}


PRIVATE DEFINE_INT15(int_2, 2);
PRIVATE DEFINE_INT15(int_8, 8);
PRIVATE DEFINE_INT15(int_16, 16);
/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_pound", "#");
]]]*/
PRIVATE DEFINE_STRING_EX(str_pound, "#", 0x44a5674f, 0xf0e41d188dc355aa);
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_hex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *args[3];
	args[0] = (DeeObject *)&int_16;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&str_pound;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__precision, "|o:hex", &args[1]))
		goto err;
	return DeeObject_CallAttr(self, (DeeObject *)&str_tostr, 3, args);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_bin(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *args[3];
	args[0] = (DeeObject *)&int_2;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&str_pound;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__precision, "|o:bin", &args[1]))
		goto err;
	return DeeObject_CallAttr(self, (DeeObject *)&str_tostr, 3, args);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_oct(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *args[3];
	args[0] = (DeeObject *)&int_8;
	args[1] = DeeInt_Zero;
	args[2] = (DeeObject *)&str_pound;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__precision, "|o:oct", &args[1]))
		goto err;
	return DeeObject_CallAttr(self, (DeeObject *)&str_tostr, 3, args);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tobytes(DeeIntObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_tobytes(DeeObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *as_int, *result;
	as_int = DeeObject_Int(self);
	if unlikely(!as_int)
		goto err;
	result = int_tobytes((DeeIntObject *)as_int, argc, argv, kw);
	Dee_Decref(as_int);
	return result;
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_bitcount(DeeIntObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_bitcount(DeeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *as_int, *result;
	as_int = DeeObject_Int(self);
	if unlikely(!as_int)
		goto err;
	result = int_bitcount((DeeIntObject *)as_int, argc, argv, kw);
	Dee_Decref(as_int);
	return result;
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_pext_f(DeeIntObject *self, size_t argc, DeeObject *const *argv);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_pext(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *as_int, *result;
	as_int = DeeObject_Int(self);
	if unlikely(!as_int)
		goto err;
	result = int_pext_f((DeeIntObject *)as_int, argc, argv);
	Dee_Decref(as_int);
	return result;
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_pdep_f(DeeIntObject *self, size_t argc, DeeObject *const *argv);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_pdep(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *as_int, *result;
	as_int = DeeObject_Int(self);
	if unlikely(!as_int)
		goto err;
	result = int_pdep_f((DeeIntObject *)as_int, argc, argv);
	Dee_Decref(as_int);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
numeric_divmod(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DREF DeeObject *d, *m;
	DeeObject *y;
	DeeArg_Unpack1(err, argc, argv, "divmod", &y);
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	d = DeeObject_Div(self, y);
	if unlikely(!d)
		goto err_r;
	m = DeeObject_Mod(self, y);
	if unlikely(!m)
		goto err_r_d;
	DeeTuple_SET(result, 0, d); /* Inherit reference */
	DeeTuple_SET(result, 1, m); /* Inherit reference */
	return result;
err_r_d:
	Dee_Decref(d);
err_r:
	DeeTuple_FreeUninitializedPair(result);
err:
	return NULL;
}

#ifdef CONFIG_HAVE_isgreater
#define HAVE_float_isgreater
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_isgreater(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_isgreater */
#ifdef CONFIG_HAVE_isgreaterequal
#define HAVE_float_isgreaterequal
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_isgreaterequal(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_isgreaterequal */
#ifdef CONFIG_HAVE_isless
#define HAVE_float_isless
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_isless(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_isless */
#ifdef CONFIG_HAVE_islessequal
#define HAVE_float_islessequal
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_islessequal(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_islessequal */
#ifdef CONFIG_HAVE_islessgreater
#define HAVE_float_islessgreater
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_islessgreater(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_islessgreater */
#if defined(HAVE_float_get_isnan) || defined(CONFIG_HAVE_isunordered)
#define HAVE_float_isunordered
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_isunordered(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* HAVE_float_get_isnan || CONFIG_HAVE_isunordered */
#if defined(CONFIG_HAVE_nextafter) || defined(CONFIG_HAVE_nexttoward)
#define HAVE_float_nextafter
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL float_nextafter(DeeFloatObject *self, size_t argc, DeeObject *const *argv);
#endif /* CONFIG_HAVE_nextafter || CONFIG_HAVE_nexttoward */


#ifndef HAVE_float_isgreater
#define float_isgreater(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "isgreater", argc, argv)
#endif /* !HAVE_float_isgreater */
#ifndef HAVE_float_isgreaterequal
#define float_isgreaterequal(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "isgreaterequal", argc, argv)
#endif /* !HAVE_float_isgreaterequal */
#ifndef HAVE_float_isless
#define float_isless(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "isless", argc, argv)
#endif /* !HAVE_float_isless */
#ifndef HAVE_float_islessequal
#define float_islessequal(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "islessequal", argc, argv)
#endif /* !HAVE_float_islessequal */
#ifndef HAVE_float_islessgreater
#define float_islessgreater(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "islessgreater", argc, argv)
#endif /* !HAVE_float_islessgreater */
#ifndef HAVE_float_isunordered
#define float_isunordered(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "isunordered", argc, argv)
#endif /* !HAVE_float_isunordered */
#ifndef HAVE_float_nextafter
#define float_nextafter(self, argc, argv) \
	DeeObject_CallAttrString((DeeObject *)(self), "nextafter", argc, argv)
#endif /* !HAVE_float_nextafter */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_nextafter(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *y;
	DREF DeeObject *result;
	int error;
	DeeArg_Unpack1(err, argc, argv, "nextafter", &y);

	/* >> if (this.isfloat)
	 * >>     return this.operator float().nextafter(y); */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (error) {
		DREF DeeFloatObject *asflt;
		asflt = (DREF DeeFloatObject *)DeeObject_Float(self);
		if unlikely(!asflt)
			goto err;
		result = float_nextafter(asflt, 1, &y);
		Dee_Decref(asflt);
		return result;
	}

	/* >> if (this > y)
	 * >>     return this - 1; */
	error = DeeObject_CmpGrAsBool(self, y);
	if unlikely(error < 0)
		goto err;
	if (error)
		return DeeObject_Sub(self, DeeInt_One);

	/* >> if (this < y)
	 * >>     return this + 1; */
	error = DeeObject_CmpLoAsBool(self, y);
	if unlikely(error < 0)
		goto err;
	if (error)
		return DeeObject_Add(self, DeeInt_One);

	/* >> return this; */
	return_reference_(self);
err:
	return NULL;
}

#define DEFINE_NUMERIC_ISCMP(name, DeeObject_CompareXObject)               \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                     \
	numeric_##name(DeeObject *self, size_t argc, DeeObject *const *argv) { \
		DeeObject *y;                                                      \
		int error;                                                         \
		DeeArg_Unpack1(err, argc, argv, #name, &y);                       \
		/* >> if (this.isfloat)                                            \
		 * >>     return this.operator float().name(y); */                 \
		error = DeeObject_IsFloat(self);                                   \
		if unlikely(error < 0)                                             \
			goto err;                                                      \
		if (error) {                                                       \
			DREF DeeObject *result;                                        \
			DREF DeeFloatObject *asflt;                                    \
			asflt = (DREF DeeFloatObject *)DeeObject_Float(self);          \
			if unlikely(!asflt)                                            \
				goto err;                                                  \
			result = float_##name(asflt, 1, &y);                           \
			Dee_Decref(asflt);                                             \
			return result;                                                 \
		}                                                                  \
		return DeeObject_CompareXObject(self, y);                          \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_NUMERIC_ISCMP(isgreater, DeeObject_CmpGr)
DEFINE_NUMERIC_ISCMP(isgreaterequal, DeeObject_CmpGe)
DEFINE_NUMERIC_ISCMP(isless, DeeObject_CmpLo)
DEFINE_NUMERIC_ISCMP(islessequal, DeeObject_CmpLe)
DEFINE_NUMERIC_ISCMP(islessgreater, DeeObject_CmpNe)
#undef DEFINE_NUMERIC_ISCMP

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_isunordered(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *y;
	int error;
	DeeArg_Unpack1(err, argc, argv, "isunordered", &y);

	/* >> if (this.isfloat)
	 * >>     return this.operator float().isunordered(y); */
	error = DeeObject_IsFloat(self);
	if unlikely(error < 0)
		goto err;
	if (error) {
		DREF DeeObject *result;
		DREF DeeFloatObject *asflt;
		asflt = (DREF DeeFloatObject *)DeeObject_Float(self);
		if unlikely(!asflt)
			goto err;
		result = float_isunordered(asflt, 1, &y);
		Dee_Decref(asflt);
		return result;
	}

	/* >> return y.isnan; */
	return DeeObject_GetAttrString(y, "isnan");
err:
	return NULL;
}

PRIVATE struct type_method tpconst numeric_methods[] = {
	/* TODO: function gcd(other: Numeric): Numeric (GreatestCommonDivisor)
	 * >> function gcd(a, b) {
	 * >>	do {
	 * >>		local x;
	 * >>		x = a % b;
	 * >>		a = b;
	 * >>		b = x;
	 * >>	} while (b);
	 * >>	return a;
	 * >> } */
	/* TODO: function lcm(other: Numeric): Numeric (LeastCommonMultiple)
	 * >> function lcm(a, b) {
	 * >>	return a * (b / gcd(a, b));
	 * >> } */
	TYPE_KWMETHOD(STR_tostr, &numeric_tostr,
	              "(radix=!10,precision=!0,mode=!P{})->?Dstring\n"
	              "Convert @this number into an integer and call ?Atostr?Dint\n"
	              "TODO: This also needs handling for floats!"),
	TYPE_KWMETHOD("hex", &numeric_hex, numeric_hex_doc),
	TYPE_KWMETHOD("bin", &numeric_bin, numeric_bin_doc),
	TYPE_KWMETHOD("oct", &numeric_oct, numeric_oct_doc),
	TYPE_KWMETHOD("tobytes", &numeric_tobytes,
	              "(length?:?.,byteorder:?Dstring=!N,signed=!f)->?DBytes\n"
	              "Convert @this number into an integer and call ?Atobytes?Dint\n"
	              "TODO: This also needs handling for floats!"),
	TYPE_KWMETHOD("bitcount", &numeric_bitcount,
	              "(signed=!f)->?.\n"
	              "Convert @this number into an integer and call ?Abitcount?Dint\n"
	              "TODO: This also needs handling for floats!"),
	TYPE_METHOD("pext", &numeric_pext,
	            "(mask:?.)->?Dint\n"
	            "Convert @this number into an integer and call ?Apext?Dint\n"),
	TYPE_METHOD("pdep", &numeric_pdep,
	            "(mask:?.)->?Dint\n"
	            "Convert @this number into an integer and call ?Apdep?Dint\n"),
	TYPE_METHOD("divmod", &numeric_divmod, numeric_divmod_doc),
	TYPE_METHOD("nextafter", &numeric_nextafter,
	            "(y:?.)->?.\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function nextafter(y: Numeric): Numeric {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().nextafter(y);\n"
	            /**/ "	if (this > y)\n"
	            /**/ "		return this - 1;\n"
	            /**/ "	if (this < y)\n"
	            /**/ "		return this + 1;\n"
	            /**/ "	return this;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("isgreater", &numeric_isgreater,
	            "(y:?.)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function isgreater(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().isgreater(y);\n"
	            /**/ "	return this > y;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("isgreaterequal", &numeric_isgreaterequal,
	            "(y:?.)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function isgreaterequal(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().isgreaterequal(y);\n"
	            /**/ "	return this >= y;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("isless", &numeric_isless,
	            "(y:?.)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function isless(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().isless(y);\n"
	            /**/ "	return this < y;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("islessequal", &numeric_islessequal,
	            "(y:?.)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function islessequal(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().islessequal(y);\n"
	            /**/ "	return this <= y;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("islessgreater", &numeric_islessgreater,
	            "(y:?.)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function islessgreater(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().islessgreater(y);\n"
	            /**/ "	return this != y;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("isunordered", &numeric_isunordered,
	            "(y:?X2?.?Dfloat)->?Dbool\n"
	            "Implemented as:\n"
	            "${"
	            /**/ "function isunordered(y: Numeric): bool {\n"
	            /**/ "	if (this.isfloat)\n"
	            /**/ "		return this.operator float().isunordered(y);\n"
	            /**/ "	return y.isnan;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD_END
};


PUBLIC DeeTypeObject DeeNumeric_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Numeric),
	/* .tp_doc      = */ DOC("Base class for ?Dint, ?Dfloat, and ?Dbool"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&DeeNone_OperatorCtor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject),
				/* .tp_any_ctor_kw = */ (Dee_funptr_t)NULL,
				/* .tp_serialize = */ (Dee_funptr_t)&DeeNone_OperatorWriteDec
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ numeric_methods,
	/* .tp_getsets       = */ numeric_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ numeric_class_getsets,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NUMERIC_C */
