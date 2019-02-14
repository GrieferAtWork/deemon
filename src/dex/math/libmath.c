/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FILES_LIBMATH_C
#define GUARD_DEX_FILES_LIBMATH_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1
#define _USE_MATH_DEFINES 1

#include "libmath.h"

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/objmethod.h>
#include <deemon/tuple.h>
#include <deemon/arg.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/bool.h>

#include <hybrid/typecore.h>
#include <limits.h>
#include <math.h>

#ifdef CONFIG_NO_ERRNO_H
#undef CONFIG_HAVE_ERRNO_H
#elif !defined(CONFIG_HAVE_ERRNO_H)
#ifdef __NO_has_include
#define CONFIG_HAVE_ERRNO_H 1
#elif __has_include(<errno.h>)
#define CONFIG_HAVE_ERRNO_H 1
#endif
#endif

#ifdef CONFIG_HAVE_ERRNO_H
#include <errno.h>
#endif /* CONFIG_HAVE_ERRNO_H */


DECL_BEGIN

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif
#ifndef M_TAU
#define M_TAU 6.28318530717958647692 /* M_PI * 2 */
#endif
#ifndef M_E
#define M_E   2.71828182845904523536
#endif

#if 0
#define isnan(x) ((x) != (x))
#endif


#ifdef CONFIG_HAVE_ERRNO_H

#ifndef EOK
#ifdef ENOERR
#define EOK ENOERR
#elif defined(ENOERROR)
#define EOK ENOERROR
#else
#define EOK 0
#endif
#endif

#define SET_OK()  errno = EOK
PRIVATE ATTR_COLD int DCALL err_domain(void) {
 return DeeError_Throwf(&DeeError_ValueError,"math domain error");
}
PRIVATE ATTR_COLD int DCALL err_overflow(void) {
 return DeeError_Throwf(&DeeError_IntegerOverflow,"math range error");
}
PRIVATE ATTR_COLD int DCALL err_math(int e) {
 return DeeError_Throwf(&DeeError_ValueError,"Math error %d",e);
}

LOCAL int DCALL math_checkerr(double x) {
 int e = errno;
 if (e == EDOM)
     return err_domain();
 if (e == ERANGE) {
  /* Prevent exceptions on underflow. */
  if (fabs(x) >= 1.0)
      return err_overflow();
 } else if (e != EOK) {
  return err_math(e);
 }
 return 0;
}
LOCAL int DCALL math_checkerr_i(int x) {
 int e = errno;
 if (e == EDOM)
     return err_domain();
 if (e == ERANGE) {
  /* Prevent exceptions on underflow. */
  if (x != 0)
      return err_overflow();
 } else if (e != EOK) {
  return err_math(e);
 }
 return 0;
}
#else /* CONFIG_HAVE_ERRNO_H */
#define SET_OK()           0
#define math_checkerr(x)   0
#define math_checkerr_i(x) 0
#endif /* !CONFIG_HAVE_ERRNO_H */


#define DEFINE_MATH_CONVERSION_1(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x; \
 if (DeeArg_Unpack(argc,argv,"D:" #name,&x)) \
     goto err; \
 return DeeFloat_New(name(x)); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);
#define DEFINE_MATH_CONVERSION_1_E(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x,result; \
 if (DeeArg_Unpack(argc,argv,"D:" #name,&x)) \
     goto err; \
 SET_OK(); \
 result = name(x); \
 if (math_checkerr(result)) \
     goto err; \
 return DeeFloat_New(result); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);
#define DEFINE_MATH_CONVERSION_2(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x,y; \
 if (DeeArg_Unpack(argc,argv,"DD:" #name,&x,&y)) \
     goto err; \
 return DeeFloat_New(name(x,y)); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);
#define DEFINE_MATH_CONVERSION_2_E(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x,y,result; \
 if (DeeArg_Unpack(argc,argv,"DD:" #name,&x,&y)) \
     goto err; \
 SET_OK(); \
 result = name(x,y); \
 if (math_checkerr(result)) \
     goto err; \
 return DeeFloat_New(result); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);
#define DEFINE_MATH_FLOAT_TRAIT(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x; \
 if (DeeArg_Unpack(argc,argv,"D:" #name,&x)) \
     goto err; \
 return_bool(name(x)); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);
#define DEFINE_MATH_FLOAT_TRAIT2(name) \
PRIVATE DREF DeeObject *DCALL \
f_math_##name(size_t argc, DeeObject **__restrict argv) \
{ \
 double x,y; \
 if (DeeArg_Unpack(argc,argv,"DD:" #name,&x,&y)) \
     goto err; \
 return_bool(name(x,y)); \
err: \
 return NULL; \
} \
PRIVATE DEFINE_CMETHOD(math_##name,f_math_##name);

#ifndef CONFIG_HAVE_ERRNO_H
#undef DEFINE_MATH_CONVERSION_1_E
#undef DEFINE_MATH_CONVERSION_2_E
#define DEFINE_MATH_CONVERSION_1_E DEFINE_MATH_CONVERSION_1
#define DEFINE_MATH_CONVERSION_2_E DEFINE_MATH_CONVERSION_2
#endif

DEFINE_MATH_CONVERSION_1(sin)
DEFINE_MATH_CONVERSION_1(cos)
DEFINE_MATH_CONVERSION_1(tan)
DEFINE_MATH_CONVERSION_1_E(asin)
DEFINE_MATH_CONVERSION_1_E(acos)
DEFINE_MATH_CONVERSION_1_E(atan)
DEFINE_MATH_CONVERSION_1_E(sinh)
DEFINE_MATH_CONVERSION_1_E(cosh)
DEFINE_MATH_CONVERSION_1_E(tanh)
DEFINE_MATH_CONVERSION_1_E(asinh)
DEFINE_MATH_CONVERSION_1_E(acosh)
DEFINE_MATH_CONVERSION_1_E(atanh)
DEFINE_MATH_CONVERSION_2(copysign)
DEFINE_MATH_CONVERSION_2_E(atan2)
DEFINE_MATH_CONVERSION_1_E(exp)
DEFINE_MATH_CONVERSION_1_E(exp2)
DEFINE_MATH_CONVERSION_1_E(expm1)
DEFINE_MATH_CONVERSION_1(erf)
DEFINE_MATH_CONVERSION_1_E(erfc)
DEFINE_MATH_CONVERSION_1(fabs)
DEFINE_MATH_CONVERSION_1_E(sqrt)
DEFINE_MATH_CONVERSION_1_E(log)
DEFINE_MATH_CONVERSION_1_E(log2)
DEFINE_MATH_CONVERSION_1_E(logb)
DEFINE_MATH_CONVERSION_1_E(log1p)
DEFINE_MATH_CONVERSION_1_E(log10)
DEFINE_MATH_CONVERSION_1_E(cbrt)
DEFINE_MATH_CONVERSION_1_E(tgamma)
DEFINE_MATH_CONVERSION_1_E(lgamma)
DEFINE_MATH_CONVERSION_2_E(pow)
DEFINE_MATH_CONVERSION_1(ceil)
DEFINE_MATH_CONVERSION_1(floor)
DEFINE_MATH_CONVERSION_1(round)
DEFINE_MATH_CONVERSION_2_E(fmod)
DEFINE_MATH_CONVERSION_2_E(hypot)
DEFINE_MATH_CONVERSION_2_E(remainder)
DEFINE_MATH_CONVERSION_2_E(nextafter)
DEFINE_MATH_CONVERSION_2(fdim)
DEFINE_MATH_FLOAT_TRAIT(isnan)
DEFINE_MATH_FLOAT_TRAIT(isinf)
DEFINE_MATH_FLOAT_TRAIT(isfinite)
DEFINE_MATH_FLOAT_TRAIT(isnormal)
DEFINE_MATH_FLOAT_TRAIT(signbit)
DEFINE_MATH_FLOAT_TRAIT2(isgreater)
DEFINE_MATH_FLOAT_TRAIT2(isgreaterequal)
DEFINE_MATH_FLOAT_TRAIT2(isless)
DEFINE_MATH_FLOAT_TRAIT2(islessequal)
DEFINE_MATH_FLOAT_TRAIT2(islessgreater)
DEFINE_MATH_FLOAT_TRAIT2(isunordered)
#undef DEFINE_MATH_FLOAT_TRAIT2
#undef DEFINE_MATH_FLOAT_TRAIT
#undef DEFINE_MATH_CONVERSION_2_E
#undef DEFINE_MATH_CONVERSION_2
#undef DEFINE_MATH_CONVERSION_1_E
#undef DEFINE_MATH_CONVERSION_1

PRIVATE DREF DeeObject *DCALL
f_math_ilogb(size_t argc, DeeObject **__restrict argv) {
 double x; int result;
 if (DeeArg_Unpack(argc,argv,"D:ilogb",&x))
     goto err;
 SET_OK();
 result = ilogb(x);
 if (math_checkerr_i(result))
     goto err;
 return DeeInt_NewInt(result);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_ilogb,f_math_ilogb);

PRIVATE DREF DeeObject *DCALL
f_math_frexp(size_t argc, DeeObject **__restrict argv) {
 double x,y,mat; int ex;
 DREF DeeObject *result,*a,*b;
 if (DeeArg_Unpack(argc,argv,"DD:frexp",&x,&y))
     goto err;
 SET_OK();
 mat = frexp(x,&ex);
 if (math_checkerr(mat))
     goto err;
 a = DeeFloat_New(mat);
 if unlikely(!a) goto err;
 b = DeeInt_NewInt(ex);
 if unlikely(!b) goto err_a;
 result = DeeTuple_PackSymbolic(2,a,b);
 if unlikely(!result) goto err_b;
 return result;
err_b:
 Dee_Decref(b);
err_a:
 Dee_Decref(a);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_frexp,f_math_frexp);

PRIVATE DREF DeeObject *DCALL
f_math_modf(size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result,*a,*b;
 double x,y;
 if (DeeArg_Unpack(argc,argv,"D:modf",&x))
     goto err;
 x = modf(x,&y);
 a = DeeFloat_New(x);
 if unlikely(!a) goto err;
 b = DeeFloat_New(y);
 if unlikely(!b) goto err_a;
 result = DeeTuple_PackSymbolic(2,a,b);
 if unlikely(!result) goto err_b;
 return result;
err_b:
 Dee_Decref(b);
err_a:
 Dee_Decref(a);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_modf,f_math_modf);

PRIVATE DREF DeeObject *DCALL
f_math_ldexp(size_t argc, DeeObject **__restrict argv) {
 double x,result; DeeObject *y;
 int error,y_value;
 if (DeeArg_Unpack(argc,argv,"Do:ldexp",&x,&y))
     goto err;
 y = DeeObject_Int(y);
 if unlikely(!y) goto err;
#if __SIZEOF_INT__ == 4
 error = DeeInt_TryAs32(y,&y_value);
#elif __SIZEOF_INT__ == 2
 error = DeeInt_TryAs16(y,&y_value);
#elif __SIZEOF_INT__ == 8
 error = DeeInt_TryAs64(y,&y_value);
#elif __SIZEOF_INT__ == 1
 error = DeeInt_TryAs8(y,&y_value);
#else
#error "Unsupported sizeof(int)"
#endif
 if (error == INT_POS_OVERFLOW)
     y_value = INT_MAX;
 else if (error == INT_NEG_OVERFLOW)
     y_value = INT_MIN;
 SET_OK();
 result = ldexp(x,y_value);
 if (math_checkerr(result))
     goto err;
 return DeeFloat_New(result);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_ldexp,f_math_ldexp);

PRIVATE DREF DeeObject *DCALL
f_math_sincos(size_t argc, DeeObject **__restrict argv) {
 double x;
 if (DeeArg_Unpack(argc,argv,"D:sincos",&x))
     goto err;
 /* XXX: sin() and cos() can be done faster when combined! */
 return DeeTuple_Newf("ff",sin(x),cos(x));
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_math_asincos(size_t argc, DeeObject **__restrict argv) {
 double x,rx,ry;
 if (DeeArg_Unpack(argc,argv,"D:asincos",&x))
     goto err;
 SET_OK();
 rx = asin(x);
 ry = acos(x);
 if (math_checkerr(ry))
     goto err;
 return DeeTuple_Newf("ff",rx,ry);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_math_sincosh(size_t argc, DeeObject **__restrict argv) {
 double x,rx,ry;
 if (DeeArg_Unpack(argc,argv,"D:sincosh",&x))
     goto err;
 SET_OK();
 rx = sinh(x);
 ry = cosh(x);
 if (math_checkerr(ry))
     goto err;
 return DeeTuple_Newf("ff",rx,ry);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
f_math_asincosh(size_t argc, DeeObject **__restrict argv) {
 double x,rx,ry;
 if (DeeArg_Unpack(argc,argv,"D:asincosh",&x))
     goto err;
 SET_OK();
 rx = asinh(x);
 ry = acosh(x);
 if (math_checkerr(ry))
     goto err;
 return DeeTuple_Newf("ff",rx,ry);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_sincos,f_math_sincos);
PRIVATE DEFINE_CMETHOD(math_asincos,f_math_asincos);
PRIVATE DEFINE_CMETHOD(math_sincosh,f_math_sincosh);
PRIVATE DEFINE_CMETHOD(math_asincosh,f_math_asincosh);

PRIVATE DREF DeeObject *DCALL
f_math_scalbn(size_t argc, DeeObject **__restrict argv) {
 double x,result; int n;
 if (DeeArg_Unpack(argc,argv,"Dd:scalbn",&x,&n))
     goto err;
 SET_OK();
 /* XXX: scalbln */
 result = scalbn(x,n);
 if (math_checkerr(result))
     goto err;
 return DeeFloat_New(result);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_scalbn,f_math_scalbn);

PRIVATE DREF DeeObject *DCALL
f_math_remquo(size_t argc, DeeObject **__restrict argv) {
 double x,y,result; int z;
 if (DeeArg_Unpack(argc,argv,"DD:remquo",&x,&y))
     goto err;
 SET_OK();
 result = remquo(x,y,&z);
 if (math_checkerr(result))
     goto err;
 return DeeTuple_Newf("fd",result,z);
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_remquo,f_math_remquo);

PRIVATE DEFINE_FLOAT(math_pi,M_PI);
PRIVATE DEFINE_FLOAT(math_tau,M_TAU);
PRIVATE DEFINE_FLOAT(math_e,M_E);
PRIVATE DEFINE_FLOAT(math_nan,NAN);
PRIVATE DEFINE_FLOAT(math_inf,INFINITY);


PRIVATE struct dex_symbol symbols[] = {
    /* NOTE: Some doc comments are gathered from the following sources:
     *   - Python source tree: /Modules/mathmodule.c
     *   - http://www.cplusplus.com/reference/cmath
     * Other comments I wrote myself.
     */
    { "acos", (DeeObject *)&math_acos, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the arc cosine of @x") },
    { "acosh", (DeeObject *)&math_acosh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the area hyperbolic cosine of @x") },
    { "asin", (DeeObject *)&math_asin, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the arc sinus of @x") },
    { "asinh", (DeeObject *)&math_asinh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the area hyperbolic sine of @x") },
    { "atan", (DeeObject *)&math_atan, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the arc tangent of @x") },
    { "atan2", (DeeObject *)&math_atan2, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturns the arc tangent of @x and @y") },
    { "atanh", (DeeObject *)&math_atanh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the area hyperbolic tangent of @x") },
    { "cbrt", (DeeObject *)&math_cbrt, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the cubic root of @x") },
    { "ceil", (DeeObject *)&math_ceil, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns @x rounded to the next greater decimal") },
    { "copysign", (DeeObject *)&math_copysign, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturn @x with the sign copied from @y") },
    { "cos", (DeeObject *)&math_cos, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the cosine of @x") },
    { "cosh", (DeeObject *)&math_cosh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the hyperbolic cosine of @x") },
    { "erf", (DeeObject *)&math_erf, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the error of @x") },
    { "erfc", (DeeObject *)&math_erfc, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the complementary error of @x") },
    { "exp", (DeeObject *)&math_exp, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the exponential of @x") },
    { "exp2", (DeeObject *)&math_exp2, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the binary exponential of @x") },
    { "expm1", (DeeObject *)&math_expm1, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the exponential of @x minus ${1.0}") },
    { "fabs", (DeeObject *)&math_fabs, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the absolute value of @x") },
    { "floor", (DeeObject *)&math_floor, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns @x rounded to the next smaller decimal") },
    { "fmod", (DeeObject *)&math_fmod, MODSYM_FNORMAL, DOC("(x:?Dfloat,x:?Dfloat)->?Dfloat\nReturns the floating point remainder of @x divided by @y") },
    { "frexp", (DeeObject *)&math_frexp, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dint\nReturns the mantissa and exponent of @x") },
    { "ilogb", (DeeObject *)&math_ilogb, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dint\nreturn binary logarithm of @x") },
    { "inf", (DeeObject *)&math_inf, MODSYM_FNORMAL, DOC("->?Dfloat\nA special floating point value, representing INFinity") },
    { "isfinite", (DeeObject *)&math_isfinite, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dbool\nReturns :true if @x is infinite") },
    { "isinf", (DeeObject *)&math_isinf, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dbool\nReturns :true if @x is infinite") },
    { "isnan", (DeeObject *)&math_isnan, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dbool\nReturns :true if @x is NotANumber") },
    { "isnormal", (DeeObject *)&math_isnormal, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dbool\nReturns :true if @x is a normal number") },
    { "ldexp", (DeeObject *)&math_ldexp, MODSYM_FNORMAL, DOC("(float significand,exponent:?Dint)->?Dfloat\nGenerate value from significand and exponent of @x") },
    { "log", (DeeObject *)&math_log, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the natural logarithm of @x") },
    { "log10", (DeeObject *)&math_log10, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the common logarithm of @x") },
    { "log1p", (DeeObject *)&math_log1p, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the logarithm of @x logarithm plus ${1.0}") },
    { "log2", (DeeObject *)&math_log2, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the binary logarithm of @x") },
    { "logb", (DeeObject *)&math_logb, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the floating-point base logarithm of @x") },
    { "modf", (DeeObject *)&math_modf, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\nReturns the fractional and integral parts of @x") },
    { "nan", (DeeObject *)&math_nan, MODSYM_FNORMAL, DOC("->?Dfloat\nA special floating point value, representing NotANumber") },
    { "tau", (DeeObject *)&math_tau, MODSYM_FNORMAL, DOC("->?Dfloat\nThe mathematical constant TAU ${pi * 2}") },
    { "pi", (DeeObject *)&math_pi, MODSYM_FNORMAL, DOC("->?Dfloat\nThe mathematical constant PI") },
    { "e", (DeeObject *)&math_e, MODSYM_FNORMAL, DOC("->?Dfloat\nThe mathematical constant E") },
    { "pow", (DeeObject *)&math_pow, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturns @x to the power of @y") },
    { "round", (DeeObject *)&math_round, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns @x rounded to the next nearest decimal") },
    { "sin", (DeeObject *)&math_sin, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the sinus of @x") },
    { "sinh", (DeeObject *)&math_sinh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the hyperbolic sinus of @x") },
    { "sqrt", (DeeObject *)&math_sqrt, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the square root of @x") },
    { "tan", (DeeObject *)&math_tan, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the tangent of @x") },
    { "tanh", (DeeObject *)&math_tanh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the hyperbolic tangent of @x") },
    { "sincos", (DeeObject *)&math_sincos, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\nReturns a tuple equivalent to ${(sin(x),cos(x))}") },
    { "asincos", (DeeObject *)&math_asincos, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\nReturns a tuple equivalent to ${(asin(x),acos(x))}") },
    { "sincosh", (DeeObject *)&math_sincosh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\nReturns a tuple equivalent to ${(sinh(x),cosh(x))}") },
    { "asincosh", (DeeObject *)&math_asincosh, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?T2?Dfloat?Dfloat\nReturns a tuple equivalent to ${(asinh(x),acosh(x))}") },
    { "scalbn", (DeeObject *)&math_scalbn, MODSYM_FNORMAL, DOC("(x:?Dfloat,n:?Dint)->?Dfloat\nScales @x by :float.radix raised to the power of @n") },
    { "hypot", (DeeObject *)&math_hypot, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturns the hypotenuse of a right-angled triangle whose legs are @x and @y") },
    { "tgamma", (DeeObject *)&math_tgamma, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the gamma function of @x") },
    { "lgamma", (DeeObject *)&math_lgamma, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dfloat\nReturns the natural logarithm of the absolute value of the gamma function of @x") },
    { "remainder", (DeeObject *)&math_remainder, MODSYM_FNORMAL, DOC("(numer:?Dfloat,denom:?Dfloat)->?Dfloat\nReturns the floating-point remainder of numer/denom (rounded to nearest)") },
    { "remquo", (DeeObject *)&math_remquo, MODSYM_FNORMAL, DOC("(numer:?Dfloat,denom:?Dfloat)->?T2?Dfloat?Dint\nReturns the same as #remainder, but additionally returns the quotient internally used to determine its result") },
    { "nextafter", (DeeObject *)&math_nextafter, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturns the next representable value after @x in the direction of @y") },
    { "fdim", (DeeObject *)&math_fdim, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dfloat\nReturns the positive difference between @x and @y") },
    { "signbit", (DeeObject *)&math_signbit, MODSYM_FNORMAL, DOC("(x:?Dfloat)->?Dbool\nReturns whether the sign of @x is negative") },
    { "isgreater", (DeeObject *)&math_isgreater, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns whether @x is greater than @y, returning :false if either is #nan") },
    { "isgreaterequal", (DeeObject *)&math_isgreaterequal, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns whether @x is greater than or equal to @y, returning :false if either is #nan") },
    { "isless", (DeeObject *)&math_isless, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns whether @x is less than @y, returning :false if either is #nan") },
    { "islessequal", (DeeObject *)&math_islessequal, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns whether @x is less than or equal to @y, returning :false if either is #nan") },
    { "islessgreater", (DeeObject *)&math_islessgreater, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns whether @x is less than or greater than @y, returning :false if either is #nan") },
    { "isunordered", (DeeObject *)&math_isunordered, MODSYM_FNORMAL, DOC("(x:?Dfloat,y:?Dfloat)->?Dbool\nReturns :true if either @x or @y is #nan") },
    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END


#endif /* !GUARD_DEX_FILES_LIBMATH_C */
