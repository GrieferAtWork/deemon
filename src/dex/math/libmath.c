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
#define _KOS_SOURCE 1
#define _USE_MATH_DEFINES 1

#include "libmath.h"

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/objmethod.h>
#include <deemon/tuple.h>
#include <deemon/arg.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/bool.h>

#include <hybrid/typecore.h>
#include <limits.h>
#include <math.h>

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
DEFINE_MATH_CONVERSION_1(sin)
DEFINE_MATH_CONVERSION_1(cos)
DEFINE_MATH_CONVERSION_1(tan)
DEFINE_MATH_CONVERSION_1(asin)
DEFINE_MATH_CONVERSION_1(acos)
DEFINE_MATH_CONVERSION_1(atan)
DEFINE_MATH_CONVERSION_1(sinh)
DEFINE_MATH_CONVERSION_1(cosh)
DEFINE_MATH_CONVERSION_1(tanh)
DEFINE_MATH_CONVERSION_1(asinh)
DEFINE_MATH_CONVERSION_1(acosh)
DEFINE_MATH_CONVERSION_1(atanh)
DEFINE_MATH_CONVERSION_2(copysign)
DEFINE_MATH_CONVERSION_2(atan2)
DEFINE_MATH_CONVERSION_1(exp)
DEFINE_MATH_CONVERSION_1(exp2)
DEFINE_MATH_CONVERSION_1(expm1)
DEFINE_MATH_CONVERSION_1(erf)
DEFINE_MATH_CONVERSION_1(erfc)
DEFINE_MATH_CONVERSION_1(fabs)
DEFINE_MATH_CONVERSION_1(sqrt)
DEFINE_MATH_CONVERSION_1(log)
DEFINE_MATH_CONVERSION_1(log2)
DEFINE_MATH_CONVERSION_1(logb)
DEFINE_MATH_CONVERSION_1(log1p)
DEFINE_MATH_CONVERSION_1(log10)
DEFINE_MATH_CONVERSION_1(cbrt)
DEFINE_MATH_CONVERSION_2(pow)
DEFINE_MATH_CONVERSION_1(ceil)
DEFINE_MATH_CONVERSION_1(floor)
DEFINE_MATH_CONVERSION_1(round)
DEFINE_MATH_CONVERSION_2(fmod)
DEFINE_MATH_FLOAT_TRAIT(isnan)
DEFINE_MATH_FLOAT_TRAIT(isinf)
DEFINE_MATH_FLOAT_TRAIT(isfinite)
DEFINE_MATH_FLOAT_TRAIT(isnormal)
#undef DEFINE_MATH_FLOAT_TRAIT
#undef DEFINE_MATH_CONVERSION_2
#undef DEFINE_MATH_CONVERSION_1

PRIVATE DREF DeeObject *DCALL
f_math_ilogb(size_t argc, DeeObject **__restrict argv) {
 double x;
 if (DeeArg_Unpack(argc,argv,"D:ilogb",&x))
     goto err;
 return DeeInt_NewInt(ilogb(x));
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
 mat = frexp(x,&ex);
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
 double x; DeeObject *y;
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
 return DeeFloat_New(ldexp(x,y_value));
err:
 return NULL;
}
PRIVATE DEFINE_CMETHOD(math_ldexp,f_math_ldexp);

PRIVATE DEFINE_FLOAT(math_pi,M_PI);
PRIVATE DEFINE_FLOAT(math_tau,M_TAU);
PRIVATE DEFINE_FLOAT(math_e,M_E);
PRIVATE DEFINE_FLOAT(math_nan,NAN);
PRIVATE DEFINE_FLOAT(math_inf,INFINITY);


PRIVATE struct dex_symbol symbols[] = {
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
    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END


#endif /* !GUARD_DEX_FILES_LIBMATH_C */
