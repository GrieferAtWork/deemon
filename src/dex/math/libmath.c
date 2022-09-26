/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FILES_LIBMATH_C
#define GUARD_DEX_FILES_LIBMATH_C 1
#define DEE_SOURCE
#define _USE_MATH_DEFINES 1

#include "libmath.h"

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <hybrid/typecore.h>
#include <hybrid/limitcore.h>

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#ifdef CONFIG_HAVE_MATH_H
#include <math.h>
#endif /* CONFIG_HAVE_MATH_H */

DECL_BEGIN

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif /* !M_PI */
#ifndef M_TAU
#define M_TAU 6.28318530717958647692 /* M_PI * 2 */
#endif /* !M_TAU */
#ifndef M_E
#define M_E   2.71828182845904523536
#endif /* !M_E */

#ifndef INT_MIN
#define INT_MIN __INT_MIN__
#endif /* !INT_MIN */

#ifndef INT_MAX
#define INT_MAX __INT_MAX__
#endif /* !INT_MAX */

#if 0
#define isnan(x) ((x) != (x))
#endif


#ifdef CONFIG_HAVE_errno

#define SET_OK()  DeeSystem_SetErrno(EOK)

#ifdef EDOM
PRIVATE ATTR_COLD int DCALL err_domain(void) {
	return DeeError_Throwf(&DeeError_ValueError, "math domain error");
}
#endif /* EDOM */

#ifdef ERANGE
PRIVATE ATTR_COLD int DCALL err_overflow(void) {
	return DeeError_Throwf(&DeeError_IntegerOverflow, "math range error");
}
#endif /* ERANGE */

PRIVATE ATTR_COLD int DCALL err_math(int e) {
	return DeeError_Throwf(&DeeError_ValueError, "Math error %d", e);
}

LOCAL int DCALL math_checkerr(double x) {
	int e = DeeSystem_GetErrno();
#ifdef EDOM
	if (e == EDOM)
		return err_domain();
#endif /* EDOM */
#ifdef ERANGE
	if (e == ERANGE) {
		/* Prevent exceptions on underflow. */
#ifdef CONFIG_HAVE_fabs
		if (fabs(x) >= 1.0)
			return err_overflow();
#else /* CONFIG_HAVE_fabs */
		if (x >= 1.0 || x <= -1.0)
			return err_overflow();
#endif /* !CONFIG_HAVE_fabs */
	} else
#endif /* ERANGE */
	if (e != EOK) {
		return err_math(e);
	}
	return 0;
}

LOCAL int DCALL math_checkerr_i(int x) {
	int e = DeeSystem_GetErrno();
#ifdef EDOM
	if (e == EDOM)
		return err_domain();
#endif /* EDOM */
#ifdef ERANGE
	if (e == ERANGE) {
		/* Prevent exceptions on underflow. */
		if (x != 0)
			return err_overflow();
	} else
#endif /* ERANGE */
	if (e != EOK) {
		return err_math(e);
	}
	return 0;
}
#else /* CONFIG_HAVE_errno */
#define SET_OK()           0
#define math_checkerr(x)   0
#define math_checkerr_i(x) 0
#endif /* !CONFIG_HAVE_errno */


/* Substitute a couple of simple functions. */
#ifndef CONFIG_HAVE_fabs
#define CONFIG_HAVE_fabs
#define fabs(x) ((x) < 0.0 ? -(x) : (x))
#endif /* !CONFIG_HAVE_fabs */

#ifndef CONFIG_HAVE_signbit
#define CONFIG_HAVE_signbit
#define signbit(x) (0.0 == -0.0 ? (x) < 0.0 : (x) <= -0.0)
#endif /* !CONFIG_HAVE_signbit */




#define DEFINE_MATH_CONVERSION_1(name)                   \
	PRIVATE WUNUSED DREF DeeObject *DCALL                \
	f_math_##name(size_t argc, DeeObject *const *argv) { \
		double x;                                        \
		if (DeeArg_Unpack(argc, argv, "D:" #name, &x))   \
			goto err;                                    \
		return DeeFloat_New(name(x));                    \
	err:                                                 \
		return NULL;                                     \
	}                                                    \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);
#define DEFINE_MATH_CONVERSION_1_E(name)                 \
	PRIVATE WUNUSED DREF DeeObject *DCALL                \
	f_math_##name(size_t argc, DeeObject *const *argv) { \
		double x, result;                                \
		if (DeeArg_Unpack(argc, argv, "D:" #name, &x))   \
			goto err;                                    \
		SET_OK();                                        \
		result = name(x);                                \
		if (math_checkerr(result))                       \
			goto err;                                    \
		return DeeFloat_New(result);                     \
	err:                                                 \
		return NULL;                                     \
	}                                                    \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);
#define DEFINE_MATH_CONVERSION_2(name)                      \
	PRIVATE WUNUSED DREF DeeObject *DCALL                   \
	f_math_##name(size_t argc, DeeObject *const *argv) {    \
		double x, y;                                        \
		if (DeeArg_Unpack(argc, argv, "DD:" #name, &x, &y)) \
			goto err;                                       \
		return DeeFloat_New(name(x, y));                    \
	err:                                                    \
		return NULL;                                        \
	}                                                       \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);
#define DEFINE_MATH_CONVERSION_2_E(name)                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                   \
	f_math_##name(size_t argc, DeeObject *const *argv) {    \
		double x, y, result;                                \
		if (DeeArg_Unpack(argc, argv, "DD:" #name, &x, &y)) \
			goto err;                                       \
		SET_OK();                                           \
		result = name(x, y);                                \
		if (math_checkerr(result))                          \
			goto err;                                       \
		return DeeFloat_New(result);                        \
	err:                                                    \
		return NULL;                                        \
	}                                                       \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);
#define DEFINE_MATH_FLOAT_TRAIT(name)                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                \
	f_math_##name(size_t argc, DeeObject *const *argv) { \
		double x;                                        \
		if (DeeArg_Unpack(argc, argv, "D:" #name, &x))   \
			goto err;                                    \
		return_bool(name(x));                            \
	err:                                                 \
		return NULL;                                     \
	}                                                    \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);
#define DEFINE_MATH_FLOAT_TRAIT2(name)                      \
	PRIVATE WUNUSED DREF DeeObject *DCALL                   \
	f_math_##name(size_t argc, DeeObject *const *argv) {    \
		double x, y;                                        \
		if (DeeArg_Unpack(argc, argv, "DD:" #name, &x, &y)) \
			goto err;                                       \
		return_bool(name(x, y));                            \
	err:                                                    \
		return NULL;                                        \
	}                                                       \
	PRIVATE DEFINE_CMETHOD(math_##name, f_math_##name);

#ifndef CONFIG_HAVE_errno
#undef DEFINE_MATH_CONVERSION_1_E
#undef DEFINE_MATH_CONVERSION_2_E
#define DEFINE_MATH_CONVERSION_1_E DEFINE_MATH_CONVERSION_1
#define DEFINE_MATH_CONVERSION_2_E DEFINE_MATH_CONVERSION_2
#endif /* !CONFIG_HAVE_errno */

#ifdef CONFIG_HAVE_sin
#define MATH_HAVE_sin
DEFINE_MATH_CONVERSION_1(sin)
#endif /* CONFIG_HAVE_sin */

#ifdef CONFIG_HAVE_cos
#define MATH_HAVE_cos 1
DEFINE_MATH_CONVERSION_1(cos)
#endif /* CONFIG_HAVE_cos */

#ifdef CONFIG_HAVE_tan
#define MATH_HAVE_tan 1
DEFINE_MATH_CONVERSION_1(tan)
#endif /* CONFIG_HAVE_tan */

#ifdef CONFIG_HAVE_asin
#define MATH_HAVE_asin 1
DEFINE_MATH_CONVERSION_1_E(asin)
#endif /* CONFIG_HAVE_asin */

#ifdef CONFIG_HAVE_acos
#define MATH_HAVE_acos 1
DEFINE_MATH_CONVERSION_1_E(acos)
#endif /* CONFIG_HAVE_acos */

#ifdef CONFIG_HAVE_atan
#define MATH_HAVE_atan 1
DEFINE_MATH_CONVERSION_1_E(atan)
#endif /* CONFIG_HAVE_atan */

#ifdef CONFIG_HAVE_sinh
#define MATH_HAVE_sinh 1
DEFINE_MATH_CONVERSION_1_E(sinh)
#endif /* CONFIG_HAVE_sinh */

#ifdef CONFIG_HAVE_cosh
#define MATH_HAVE_cosh 1
DEFINE_MATH_CONVERSION_1_E(cosh)
#endif /* CONFIG_HAVE_cosh */

#ifdef CONFIG_HAVE_tanh
#define MATH_HAVE_tanh 1
DEFINE_MATH_CONVERSION_1_E(tanh)
#endif /* CONFIG_HAVE_tanh */

#ifdef CONFIG_HAVE_asinh
#define MATH_HAVE_asinh 1
DEFINE_MATH_CONVERSION_1_E(asinh)
#endif /* CONFIG_HAVE_asinh */

#ifdef CONFIG_HAVE_acosh
#define MATH_HAVE_acosh 1
DEFINE_MATH_CONVERSION_1_E(acosh)
#endif /* CONFIG_HAVE_acosh */

#ifdef CONFIG_HAVE_atanh
#define MATH_HAVE_atanh 1
DEFINE_MATH_CONVERSION_1_E(atanh)
#endif /* CONFIG_HAVE_atanh */

#ifdef CONFIG_HAVE_copysign
#define MATH_HAVE_copysign 1
DEFINE_MATH_CONVERSION_2(copysign)
#endif /* CONFIG_HAVE_copysign */

#ifdef CONFIG_HAVE_atan2
#define MATH_HAVE_atan2 1
DEFINE_MATH_CONVERSION_2_E(atan2)
#endif /* CONFIG_HAVE_atan2 */

#ifdef CONFIG_HAVE_exp
#define MATH_HAVE_exp 1
DEFINE_MATH_CONVERSION_1_E(exp)
#endif /* CONFIG_HAVE_exp */

#ifdef CONFIG_HAVE_exp2
#define MATH_HAVE_exp2 1
DEFINE_MATH_CONVERSION_1_E(exp2)
#endif /* CONFIG_HAVE_exp2 */

#ifdef CONFIG_HAVE_expm1
#define MATH_HAVE_expm1 1
DEFINE_MATH_CONVERSION_1_E(expm1)
#endif /* CONFIG_HAVE_expm1 */

#ifdef CONFIG_HAVE_erf
#define MATH_HAVE_erf 1
DEFINE_MATH_CONVERSION_1(erf)
#endif /* CONFIG_HAVE_erf */

#ifdef CONFIG_HAVE_erfc
#define MATH_HAVE_erfc 1
DEFINE_MATH_CONVERSION_1_E(erfc)
#endif /* CONFIG_HAVE_erfc */

#ifdef CONFIG_HAVE_fabs
#define MATH_HAVE_fabs 1
DEFINE_MATH_CONVERSION_1(fabs)
#endif /* CONFIG_HAVE_fabs */

#ifdef CONFIG_HAVE_sqrt
#define MATH_HAVE_sqrt 1
DEFINE_MATH_CONVERSION_1_E(sqrt)
#endif /* CONFIG_HAVE_sqrt */

#ifdef CONFIG_HAVE_log
#define MATH_HAVE_log 1
DEFINE_MATH_CONVERSION_1_E(log)
#endif /* CONFIG_HAVE_log */

#ifdef CONFIG_HAVE_log2
#define MATH_HAVE_log2 1
DEFINE_MATH_CONVERSION_1_E(log2)
#endif /* CONFIG_HAVE_log2 */

#ifdef CONFIG_HAVE_logb
#define MATH_HAVE_logb 1
DEFINE_MATH_CONVERSION_1_E(logb)
#endif /* CONFIG_HAVE_logb */

#ifdef CONFIG_HAVE_log1p
#define MATH_HAVE_log1p 1
DEFINE_MATH_CONVERSION_1_E(log1p)
#endif /* CONFIG_HAVE_log1p */

#ifdef CONFIG_HAVE_log10
#define MATH_HAVE_log10 1
DEFINE_MATH_CONVERSION_1_E(log10)
#endif /* CONFIG_HAVE_log10 */

#ifdef CONFIG_HAVE_cbrt
#define MATH_HAVE_cbrt 1
DEFINE_MATH_CONVERSION_1_E(cbrt)
#endif /* CONFIG_HAVE_cbrt */

#ifdef CONFIG_HAVE_tgamma
#define MATH_HAVE_tgamma 1
DEFINE_MATH_CONVERSION_1_E(tgamma)
#endif /* CONFIG_HAVE_tgamma */

#ifdef CONFIG_HAVE_lgamma
#define MATH_HAVE_lgamma 1
DEFINE_MATH_CONVERSION_1_E(lgamma)
#endif /* CONFIG_HAVE_lgamma */

#ifdef CONFIG_HAVE_pow
#define MATH_HAVE_pow 1
DEFINE_MATH_CONVERSION_2_E(pow)
#endif /* CONFIG_HAVE_pow */

#ifdef CONFIG_HAVE_ceil
#define MATH_HAVE_ceil 1
DEFINE_MATH_CONVERSION_1(ceil)
#endif /* CONFIG_HAVE_ceil */

#ifdef CONFIG_HAVE_floor
#define MATH_HAVE_floor 1
DEFINE_MATH_CONVERSION_1(floor)
#endif /* CONFIG_HAVE_floor */

#ifdef CONFIG_HAVE_round
#define MATH_HAVE_round 1
DEFINE_MATH_CONVERSION_1(round)
#endif /* CONFIG_HAVE_round */

#ifdef CONFIG_HAVE_fmod
#define MATH_HAVE_fmod 1
DEFINE_MATH_CONVERSION_2_E(fmod)
#endif /* CONFIG_HAVE_fmod */

#ifdef CONFIG_HAVE_hypot
#define MATH_HAVE_hypot 1
DEFINE_MATH_CONVERSION_2_E(hypot)
#endif /* CONFIG_HAVE_hypot */

#ifdef CONFIG_HAVE_remainder
#define MATH_HAVE_remainder 1
DEFINE_MATH_CONVERSION_2_E(remainder)
#endif /* CONFIG_HAVE_remainder */

#ifdef CONFIG_HAVE_nextafter
#define MATH_HAVE_nextafter 1
DEFINE_MATH_CONVERSION_2_E(nextafter)
#endif /* CONFIG_HAVE_nextafter */

#ifdef CONFIG_HAVE_fdim
#define MATH_HAVE_fdim 1
DEFINE_MATH_CONVERSION_2(fdim)
#endif /* CONFIG_HAVE_fdim */

#ifdef CONFIG_HAVE_isnan
#define MATH_HAVE_isnan 1
DEFINE_MATH_FLOAT_TRAIT(isnan)
#endif /* CONFIG_HAVE_isnan */

#ifdef CONFIG_HAVE_isinf
#define MATH_HAVE_isinf 1
DEFINE_MATH_FLOAT_TRAIT(isinf)
#endif /* CONFIG_HAVE_isinf */

#ifdef CONFIG_HAVE_isfinite
#define MATH_HAVE_isfinite 1
DEFINE_MATH_FLOAT_TRAIT(isfinite)
#endif /* CONFIG_HAVE_isfinite */

#ifdef CONFIG_HAVE_isnormal
#define MATH_HAVE_isnormal 1
DEFINE_MATH_FLOAT_TRAIT(isnormal)
#endif /* CONFIG_HAVE_isnormal */

#ifdef CONFIG_HAVE_signbit
#define MATH_HAVE_signbit 1
DEFINE_MATH_FLOAT_TRAIT(signbit)
#endif /* CONFIG_HAVE_signbit */

#ifdef CONFIG_HAVE_isgreater
#define MATH_HAVE_isgreater 1
DEFINE_MATH_FLOAT_TRAIT2(isgreater)
#endif /* CONFIG_HAVE_isgreater */

#ifdef CONFIG_HAVE_isgreaterequal
#define MATH_HAVE_isgreaterequal 1
DEFINE_MATH_FLOAT_TRAIT2(isgreaterequal)
#endif /* CONFIG_HAVE_isgreaterequal */

#ifdef CONFIG_HAVE_isless
#define MATH_HAVE_isless 1
DEFINE_MATH_FLOAT_TRAIT2(isless)
#endif /* CONFIG_HAVE_isless */

#ifdef CONFIG_HAVE_islessequal
#define MATH_HAVE_islessequal 1
DEFINE_MATH_FLOAT_TRAIT2(islessequal)
#endif /* CONFIG_HAVE_islessequal */

#ifdef CONFIG_HAVE_islessgreater
#define MATH_HAVE_islessgreater 1
DEFINE_MATH_FLOAT_TRAIT2(islessgreater)
#endif /* CONFIG_HAVE_islessgreater */

#ifdef CONFIG_HAVE_isunordered
#define MATH_HAVE_isunordered 1
DEFINE_MATH_FLOAT_TRAIT2(isunordered)
#endif /* CONFIG_HAVE_isunordered */

#undef DEFINE_MATH_FLOAT_TRAIT2
#undef DEFINE_MATH_FLOAT_TRAIT
#undef DEFINE_MATH_CONVERSION_2_E
#undef DEFINE_MATH_CONVERSION_2
#undef DEFINE_MATH_CONVERSION_1_E
#undef DEFINE_MATH_CONVERSION_1

#ifdef CONFIG_HAVE_ilogb
#define MATH_HAVE_ilogb 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_ilogb(size_t argc, DeeObject *const *argv) {
	double x;
	int result;
	if (DeeArg_Unpack(argc, argv, "D:ilogb", &x))
		goto err;
	SET_OK();
	result = ilogb(x);
	if (math_checkerr_i(result))
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_ilogb, f_math_ilogb);
#endif /* CONFIG_HAVE_ilogb */

#ifdef CONFIG_HAVE_frexp
#define MATH_HAVE_frexp 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_frexp(size_t argc, DeeObject *const *argv) {
	double x, y, mat;
	int ex;
	DREF DeeObject *result, *a, *b;
	if (DeeArg_Unpack(argc, argv, "DD:frexp", &x, &y))
		goto err;
	SET_OK();
	mat = frexp(x, &ex);
	if (math_checkerr(mat))
		goto err;
	a = DeeFloat_New(mat);
	if unlikely(!a)
		goto err;
	b = DeeInt_NewInt(ex);
	if unlikely(!b)
		goto err_a;
	result = DeeTuple_PackSymbolic(2, a, b);
	if unlikely(!result)
		goto err_b;
	return result;
err_b:
	Dee_Decref(b);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_frexp, f_math_frexp);
#endif /* CONFIG_HAVE_frexp */

#ifdef CONFIG_HAVE_modf
#define MATH_HAVE_modf 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_modf(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *a, *b;
	double x, y;
	if (DeeArg_Unpack(argc, argv, "D:modf", &x))
		goto err;
	x = modf(x, &y);
	a = DeeFloat_New(x);
	if unlikely(!a)
		goto err;
	b = DeeFloat_New(y);
	if unlikely(!b)
		goto err_a;
	result = DeeTuple_PackSymbolic(2, a, b);
	if unlikely(!result)
		goto err_b;
	return result;
err_b:
	Dee_Decref(b);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_modf, f_math_modf);
#endif /* CONFIG_HAVE_modf */

#ifdef CONFIG_HAVE_ldexp
#define MATH_HAVE_ldexp 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_ldexp(size_t argc, DeeObject *const *argv) {
	double x, result;
	DeeObject *y;
	int error, y_value;
	if (DeeArg_Unpack(argc, argv, "Do:ldexp", &x, &y))
		goto err;
	y = DeeObject_Int(y);
	if unlikely(!y)
		goto err;
	error = DeeInt_TryAsInt(y, &y_value);
	if (error == INT_POS_OVERFLOW)
		y_value = INT_MAX;
	else if (error == INT_NEG_OVERFLOW) {
		y_value = INT_MIN;
	}
	SET_OK();
	result = ldexp(x, y_value);
	if (math_checkerr(result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_ldexp, f_math_ldexp);
#endif /* CONFIG_HAVE_ldexp */

#if (defined(CONFIG_HAVE_sincos) || \
     (defined(CONFIG_HAVE_sin) && defined(CONFIG_HAVE_cos)))
#define MATH_HAVE_sincos 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_sincos(size_t argc, DeeObject *const *argv) {
	double x, sinx, cosx;
	if (DeeArg_Unpack(argc, argv, "D:sincos", &x))
		goto err;
#ifdef CONFIG_HAVE_sincos
	/* sin() and cos() are faster when combined! */
	sincos(x, &sinx, &cosx);
#else /* CONFIG_HAVE_sincos */
	sinx = sin(x);
	cosx = cos(x);
#endif /* !CONFIG_HAVE_sincos */

	return DeeTuple_Newf("ff", sinx, cosx);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_sincos, f_math_sincos);
#endif /* CONFIG_HAVE_sincos || (CONFIG_HAVE_sin && CONFIG_HAVE_cos) */

#if (defined(CONFIG_HAVE_asincos) || \
     (defined(CONFIG_HAVE_asin) && defined(CONFIG_HAVE_acos)))
#define MATH_HAVE_asincos 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_asincos(size_t argc, DeeObject *const *argv) {
	double x, asinx, acosx;
	if (DeeArg_Unpack(argc, argv, "D:asincos", &x))
		goto err;
	SET_OK();
#ifdef CONFIG_HAVE_asincos
	asincos(x, &asinx, &acosx);
#else /* CONFIG_HAVE_asincos */
	asinx = asin(x);
	acosx = acos(x);
#endif /* !CONFIG_HAVE_asincos */
	if (math_checkerr(acosx))
		goto err;
	return DeeTuple_Newf("ff", asinx, acosx);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_asincos, f_math_asincos);
#endif /* CONFIG_HAVE_asincos || (CONFIG_HAVE_asin && CONFIG_HAVE_acos) */

#if (defined(CONFIG_HAVE_sincosh) || \
     (defined(CONFIG_HAVE_sinh) && defined(CONFIG_HAVE_cosh)))
#define MATH_HAVE_sincosh 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_sincosh(size_t argc, DeeObject *const *argv) {
	double x, sinhx, coshx;
	if (DeeArg_Unpack(argc, argv, "D:sincosh", &x))
		goto err;
	SET_OK();
#ifdef CONFIG_HAVE_sincosh
	sincosh(x, &sinhx, &coshx);
#else /* CONFIG_HAVE_sincosh */
	sinhx = sinh(x);
	coshx = cosh(x);
#endif /* !CONFIG_HAVE_sincosh */
	if (math_checkerr(coshx))
		goto err;
	return DeeTuple_Newf("ff", sinhx, coshx);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_sincosh, f_math_sincosh);
#endif /* CONFIG_HAVE_sincosh || (CONFIG_HAVE_sinh && CONFIG_HAVE_cosh) */

#if (defined(CONFIG_HAVE_asincosh) || \
     (defined(CONFIG_HAVE_asinh) && defined(CONFIG_HAVE_acosh)))
#define MATH_HAVE_asincosh 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_asincosh(size_t argc, DeeObject *const *argv) {
	double x, asinhx, acoshx;
	if (DeeArg_Unpack(argc, argv, "D:asincosh", &x))
		goto err;
	SET_OK();
#ifdef CONFIG_HAVE_asincosh
	asincosh(x, &asinhx, &acoshx);
#else /* CONFIG_HAVE_asincosh */
	asinhx = asinh(x);
	acoshx = acosh(x);
#endif /* !CONFIG_HAVE_asincosh */
	if (math_checkerr(acoshx))
		goto err;
	return DeeTuple_Newf("ff", asinhx, acoshx);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_asincosh, f_math_asincosh);
#endif /* CONFIG_HAVE_asincosh || (CONFIG_HAVE_asinh && CONFIG_HAVE_acosh) */


#if defined(CONFIG_HAVE_scalbln) || defined(CONFIG_HAVE_scalbn)
#define MATH_HAVE_scalbn 1
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_scalbn(size_t argc, DeeObject *const *argv) {
	double x, result;
#ifdef CONFIG_HAVE_scalbln
	long n;
	if (DeeArg_Unpack(argc, argv, "Dld:scalbn", &x, &n))
		goto err;
#else /* CONFIG_HAVE_scalbln */
	int n;
	if (DeeArg_Unpack(argc, argv, "Dd:scalbn", &x, &n))
		goto err;
#endif /* !CONFIG_HAVE_scalbln */
	SET_OK();
#ifdef CONFIG_HAVE_scalbln
	result = scalbln(x, n);
#else /* CONFIG_HAVE_scalbln */
	result = scalbn(x, n);
#endif /* !CONFIG_HAVE_scalbln */
	if (math_checkerr(result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_scalbn, f_math_scalbn);
#endif /* CONFIG_HAVE_scalbln || CONFIG_HAVE_scalbn */

#ifdef CONFIG_HAVE_remquo
#define MATH_HAVE_remquo
PRIVATE WUNUSED DREF DeeObject *DCALL
f_math_remquo(size_t argc, DeeObject *const *argv) {
	double x, y, result;
	int z;
	if (DeeArg_Unpack(argc, argv, "DD:remquo", &x, &y))
		goto err;
	SET_OK();
	result = remquo(x, y, &z);
	if (math_checkerr(result))
		goto err;
	return DeeTuple_Newf("fd", result, z);
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(math_remquo, f_math_remquo);
#endif /* CONFIG_HAVE_remquo */


#ifdef M_PI
#define MATH_HAVE_pi 1
PRIVATE DEFINE_FLOAT(math_pi, M_PI);
#endif /* M_PI */

#ifdef M_TAU
#define MATH_HAVE_tau 1
PRIVATE DEFINE_FLOAT(math_tau, M_TAU);
#endif /* M_TAU */

#ifdef M_E
#define MATH_HAVE_e 1
PRIVATE DEFINE_FLOAT(math_e, M_E);
#endif /* M_E */

#ifdef NAN
#define MATH_HAVE_nan 1
PRIVATE DEFINE_FLOAT(math_nan, NAN);
#endif /* NAN */

#ifdef INFINITY
#define MATH_HAVE_inf
PRIVATE DEFINE_FLOAT(math_inf, INFINITY);
#elif defined(HUGE_VALD)
#define MATH_HAVE_inf
PRIVATE DEFINE_FLOAT(math_inf, HUGE_VALD);
#endif /* ... */


PRIVATE struct dex_symbol symbols[] = {
	/* NOTE: Some doc comments are gathered from the following sources:
	 *   - Python source tree: /Modules/mathmodule.c
	 *   - http://www.cplusplus.com/reference/cmath
	 * Other comments I wrote myself.
	 */
#ifdef MATH_HAVE_acos
	{ "acos", (DeeObject *)&math_acos, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the arc cosine of @x") },
#endif /* MATH_HAVE_acos */

#ifdef MATH_HAVE_acosh
	{ "acosh", (DeeObject *)&math_acosh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the area hyperbolic cosine of @x") },
#endif /* MATH_HAVE_acosh */

#ifdef MATH_HAVE_asin
	{ "asin", (DeeObject *)&math_asin, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the arc sinus of @x") },
#endif /* MATH_HAVE_asin */

#ifdef MATH_HAVE_asinh
	{ "asinh", (DeeObject *)&math_asinh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the area hyperbolic sine of @x") },
#endif /* MATH_HAVE_asinh */

#ifdef MATH_HAVE_atan
	{ "atan", (DeeObject *)&math_atan, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the arc tangent of @x") },
#endif /* MATH_HAVE_atan */

#ifdef MATH_HAVE_atan2
	{ "atan2", (DeeObject *)&math_atan2, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Returns the arc tangent of @x and @y") },
#endif /* MATH_HAVE_atan2 */

#ifdef MATH_HAVE_atanh
	{ "atanh", (DeeObject *)&math_atanh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the area hyperbolic tangent of @x") },
#endif /* MATH_HAVE_atanh */

#ifdef MATH_HAVE_cbrt
	{ "cbrt", (DeeObject *)&math_cbrt, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the cubic root of @x") },
#endif /* MATH_HAVE_cbrt */

#ifdef MATH_HAVE_ceil
	{ "ceil", (DeeObject *)&math_ceil, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns @x rounded to the next greater decimal") },
#endif /* MATH_HAVE_ceil */

#ifdef MATH_HAVE_copysign
	{ "copysign", (DeeObject *)&math_copysign, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Return @x with the sign copied from @y") },
#endif /* MATH_HAVE_copysign */

#ifdef MATH_HAVE_cos
	{ "cos", (DeeObject *)&math_cos, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the cosine of @x") },
#endif /* MATH_HAVE_cos */

#ifdef MATH_HAVE_cosh
	{ "cosh", (DeeObject *)&math_cosh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the hyperbolic cosine of @x") },
#endif /* MATH_HAVE_cosh */

#ifdef MATH_HAVE_erf
	{ "erf", (DeeObject *)&math_erf, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the error of @x") },
#endif /* MATH_HAVE_erf */

#ifdef MATH_HAVE_erfc
	{ "erfc", (DeeObject *)&math_erfc, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the complementary error of @x") },
#endif /* MATH_HAVE_erfc */

#ifdef MATH_HAVE_exp
	{ "exp", (DeeObject *)&math_exp, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the exponential of @x") },
#endif /* MATH_HAVE_exp */

#ifdef MATH_HAVE_exp2
	{ "exp2", (DeeObject *)&math_exp2, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the binary exponential of @x") },
#endif /* MATH_HAVE_exp2 */

#ifdef MATH_HAVE_expm1
	{ "expm1", (DeeObject *)&math_expm1, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the exponential of @x minus ${1.0}") },
#endif /* MATH_HAVE_expm1 */

#ifdef MATH_HAVE_fabs
	{ "fabs", (DeeObject *)&math_fabs, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the absolute value of @x") },
#endif /* MATH_HAVE_fabs */

#ifdef MATH_HAVE_floor
	{ "floor", (DeeObject *)&math_floor, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns @x rounded to the next smaller decimal") },
#endif /* MATH_HAVE_floor */

#ifdef MATH_HAVE_fmod
	{ "fmod", (DeeObject *)&math_fmod, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,x:?Dfloat)->?Dfloat\n"
	      "Returns the floating point remainder of @x divided by @y") },
#endif /* MATH_HAVE_fmod */

#ifdef MATH_HAVE_frexp
	{ "frexp", (DeeObject *)&math_frexp, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dint\n"
	      "Returns the mantissa and exponent of @x") },
#endif /* MATH_HAVE_frexp */

#ifdef MATH_HAVE_ilogb
	{ "ilogb", (DeeObject *)&math_ilogb, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dint\n"
	      "return binary logarithm of @x") },
#endif /* MATH_HAVE_ilogb */

#ifdef MATH_HAVE_isfinite
	{ "isfinite", (DeeObject *)&math_isfinite, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dbool\n"
	      "Returns ?t if @x is infinite") },
#endif /* MATH_HAVE_isfinite */

#ifdef MATH_HAVE_isinf
	{ "isinf", (DeeObject *)&math_isinf, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dbool\n"
	      "Returns ?t if @x is infinite") },
#endif /* MATH_HAVE_isinf */

#ifdef MATH_HAVE_isnan
	{ "isnan", (DeeObject *)&math_isnan, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dbool\n"
	      "Returns ?t if @x is NotANumber") },
#endif /* MATH_HAVE_isnan */

#ifdef MATH_HAVE_isnormal
	{ "isnormal", (DeeObject *)&math_isnormal, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dbool\n"
	      "Returns ?t if @x is a normal number") },
#endif /* MATH_HAVE_isnormal */

#ifdef MATH_HAVE_ldexp
	{ "ldexp", (DeeObject *)&math_ldexp, MODSYM_FNORMAL,
	  DOC("(float significand,exponent:?Dint)->?Dfloat\n"
	      "Generate value from significand and exponent of @x") },
#endif /* MATH_HAVE_ldexp */

#ifdef MATH_HAVE_log
	{ "log", (DeeObject *)&math_log, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the natural logarithm of @x") },
#endif /* MATH_HAVE_log */

#ifdef MATH_HAVE_log10
	{ "log10", (DeeObject *)&math_log10, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the common logarithm of @x") },
#endif /* MATH_HAVE_log10 */

#ifdef MATH_HAVE_log1p
	{ "log1p", (DeeObject *)&math_log1p, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the logarithm of @x logarithm plus ${1.0}") },
#endif /* MATH_HAVE_log1p */

#ifdef MATH_HAVE_log2
	{ "log2", (DeeObject *)&math_log2, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the binary logarithm of @x") },
#endif /* MATH_HAVE_log2 */

#ifdef MATH_HAVE_logb
	{ "logb", (DeeObject *)&math_logb, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the floating-point base logarithm of @x") },
#endif /* MATH_HAVE_logb */

#ifdef MATH_HAVE_modf
	{ "modf", (DeeObject *)&math_modf, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\n"
	      "Returns the fractional and integral parts of @x") },
#endif /* MATH_HAVE_modf */

#ifdef MATH_HAVE_pow
	{ "pow", (DeeObject *)&math_pow, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Returns @x to the power of @y") },
#endif /* MATH_HAVE_pow */

#ifdef MATH_HAVE_round
	{ "round", (DeeObject *)&math_round, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns @x rounded to the next nearest decimal") },
#endif /* MATH_HAVE_round */

#ifdef MATH_HAVE_sin
	{ "sin", (DeeObject *)&math_sin, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the sinus of @x") },
#endif /* MATH_HAVE_sin */

#ifdef MATH_HAVE_sinh
	{ "sinh", (DeeObject *)&math_sinh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the hyperbolic sinus of @x") },
#endif /* MATH_HAVE_sinh */

#ifdef MATH_HAVE_sqrt
	{ "sqrt", (DeeObject *)&math_sqrt, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the square root of @x") },
#endif /* MATH_HAVE_sqrt */

#ifdef MATH_HAVE_tan
	{ "tan", (DeeObject *)&math_tan, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the tangent of @x") },
#endif /* MATH_HAVE_tan */

#ifdef MATH_HAVE_tanh
	{ "tanh", (DeeObject *)&math_tanh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the hyperbolic tangent of @x") },
#endif /* MATH_HAVE_tanh */

#ifdef MATH_HAVE_sincos
	{ "sincos", (DeeObject *)&math_sincos, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\n"
	      "Returns a tuple equivalent to ${(sin(x), cos(x))}") },
#endif /* MATH_HAVE_sincos */

#ifdef MATH_HAVE_asincos
	{ "asincos", (DeeObject *)&math_asincos, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\n"
	      "Returns a tuple equivalent to ${(asin(x), acos(x))}") },
#endif /* MATH_HAVE_asincos */

#ifdef MATH_HAVE_sincosh
	{ "sincosh", (DeeObject *)&math_sincosh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\n"
	      "Returns a tuple equivalent to ${(sinh(x), cosh(x))}") },
#endif /* MATH_HAVE_sincosh */

#ifdef MATH_HAVE_asincosh
	{ "asincosh", (DeeObject *)&math_asincosh, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\n"
	      "Returns a tuple equivalent to ${(asinh(x), acosh(x))}") },
#endif /* MATH_HAVE_asincosh */

#ifdef MATH_HAVE_scalbn
	{ "scalbn", (DeeObject *)&math_scalbn, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,n:?Dint)->?Dfloat\n"
	      "Scales @x by :float.radix raised to the power of @n") },
#endif /* MATH_HAVE_scalbn */

#ifdef MATH_HAVE_hypot
	{ "hypot", (DeeObject *)&math_hypot, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Returns the hypotenuse of a right-angled triangle whose legs are @x and @y") },
#endif /* MATH_HAVE_hypot */

#ifdef MATH_HAVE_tgamma
	{ "tgamma", (DeeObject *)&math_tgamma, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the gamma function of @x") },
#endif /* MATH_HAVE_tgamma */

#ifdef MATH_HAVE_lgamma
	{ "lgamma", (DeeObject *)&math_lgamma, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dfloat\n"
	      "Returns the natural logarithm of the absolute value of the gamma function of @x") },
#endif /* MATH_HAVE_lgamma */

#ifdef MATH_HAVE_remainder
	{ "remainder", (DeeObject *)&math_remainder, MODSYM_FNORMAL,
	  DOC("(numer:?Dfloat,denom:?Dfloat)->?Dfloat\n"
	      "Returns the floating-point remainder of numer/denom (rounded to nearest)") },
#endif /* MATH_HAVE_remainder */

#ifdef MATH_HAVE_remquo
	{ "remquo", (DeeObject *)&math_remquo, MODSYM_FNORMAL,
	  DOC("(numer:?Dfloat,denom:?Dfloat)->?T2?Dfloat?Dint\n"
	      "Returns the same as ?#remainder, but additionally returns the quotient internally used to determine its result") },
#endif /* MATH_HAVE_remquo */

#ifdef MATH_HAVE_nextafter
	{ "nextafter", (DeeObject *)&math_nextafter, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Returns the next representable value after @x in the direction of @y") },
#endif /* MATH_HAVE_nextafter */

#ifdef MATH_HAVE_fdim
	{ "fdim", (DeeObject *)&math_fdim, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\n"
	      "Returns the positive difference between @x and @y") },
#endif /* MATH_HAVE_fdim */

#ifdef MATH_HAVE_signbit
	{ "signbit", (DeeObject *)&math_signbit, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat)->?Dbool\n"
	      "Returns whether the sign of @x is negative") },
#endif /* MATH_HAVE_signbit */

#ifdef MATH_HAVE_isgreater
	{ "isgreater", (DeeObject *)&math_isgreater, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns whether @x is greater than @y, returning ?f if either is ?#nan") },
#endif /* MATH_HAVE_isgreater */

#ifdef MATH_HAVE_isgreaterequal
	{ "isgreaterequal", (DeeObject *)&math_isgreaterequal, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns whether @x is greater than or equal to @y, returning ?f if either is ?#nan") },
#endif /* MATH_HAVE_isgreaterequal */

#ifdef MATH_HAVE_isless
	{ "isless", (DeeObject *)&math_isless, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns whether @x is less than @y, returning ?f if either is ?#nan") },
#endif /* MATH_HAVE_isless */

#ifdef MATH_HAVE_islessequal
	{ "islessequal", (DeeObject *)&math_islessequal, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns whether @x is less than or equal to @y, returning ?f if either is ?#nan") },
#endif /* MATH_HAVE_islessequal */

#ifdef MATH_HAVE_islessgreater
	{ "islessgreater", (DeeObject *)&math_islessgreater, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns whether @x is less than or greater than @y, returning ?f if either is ?#nan") },
#endif /* MATH_HAVE_islessgreater */

#ifdef MATH_HAVE_isunordered
	{ "isunordered", (DeeObject *)&math_isunordered, MODSYM_FNORMAL,
	  DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\n"
	      "Returns ?t if either @x or @y is ?#nan") },
#endif /* MATH_HAVE_isunordered */

#ifdef MATH_HAVE_inf
	{ "inf", (DeeObject *)&math_inf, MODSYM_FNORMAL,
	  DOC("->?Dfloat\n"
	      "A special floating point value, representing INFinity") },
#endif /* MATH_HAVE_inf */

#ifdef MATH_HAVE_nan
	{ "nan", (DeeObject *)&math_nan, MODSYM_FNORMAL,
	  DOC("->?Dfloat\n"
	      "A special floating point value, representing NotANumber") },
#endif /* MATH_HAVE_nan */

#ifdef MATH_HAVE_tau
	{ "tau", (DeeObject *)&math_tau, MODSYM_FNORMAL,
	  DOC("->?Dfloat\n"
	      "The mathematical constant TAU ${pi * 2}") },
#endif /* MATH_HAVE_tau */

#ifdef MATH_HAVE_pi
	{ "pi", (DeeObject *)&math_pi, MODSYM_FNORMAL,
	  DOC("->?Dfloat\n"
	      "The mathematical constant PI") },
#endif /* MATH_HAVE_pi */

#ifdef MATH_HAVE_e
	{ "e", (DeeObject *)&math_e, MODSYM_FNORMAL,
	  DOC("->?Dfloat\n"
	      "The mathematical constant E") },
#endif /* MATH_HAVE_e */

	{ NULL }
};

PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols
};

DECL_END


#endif /* !GUARD_DEX_FILES_LIBMATH_C */
