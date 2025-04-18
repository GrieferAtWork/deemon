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
#ifndef GUARD_DEEMON_OBJECTS_FLOAT_C
#define GUARD_DEEMON_OBJECTS_FLOAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>

#include <hybrid/typecore.h>
/**/

#include "../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* INTPTR_MIN */

#ifdef CONFIG_HAVE_FPU
#include <hybrid/floatcore.h>

#ifdef CONFIG_HAVE_FLOAT_H
#include <float.h>
#endif /* CONFIG_HAVE_FLOAT_H */

#ifdef CONFIG_HAVE_MATH_H
#include <math.h>
#endif /* CONFIG_HAVE_MATH_H */
/**/

#include "int_logic.h" /* `DeeInt_Alloc()' */
#endif /* CONFIG_HAVE_FPU */

DECL_BEGIN

typedef DeeFloatObject Float;

#if defined(CONFIG_HAVE_FPU) || defined(__DEEMON__)
/* Create and return a new floating point object. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeFloat_New(double value) {
	/* Allocate a new float object descriptor. */
	DREF Float *result;
	result = DeeObject_MALLOC(Float);
	if unlikely(!result)
		goto done;
	/* Initialize the float object and assign its value. */
	DeeObject_Init(result, &DeeFloat_Type);
	result->f_value = value;
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
float_ctor(Float *__restrict self) {
	self->f_value = 0.0; /* Default to 0. */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
float_copy(Float *__restrict self,
           Float *__restrict other) {
	self->f_value = other->f_value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
float_init(Float *__restrict self,
           size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("float", params: "
	DeeObject *ob: ?X2?DNumeric?Dstring;
", docStringPrefix: "float");]]]*/
#define float_float_params "ob:?X2?DNumeric?Dstring"
	struct {
		DeeObject *ob;
	} args;
	_DeeArg_Unpack1(err, argc, argv, "float", &args.ob);
/*[[[end]]]*/

	/* Special case for when a string is given. */
	if (DeeString_Check(args.ob)) {
		char const *str = DeeString_STR(args.ob);
		self->f_value = Dee_Strtod(str, (char **)&str);
		if (str != DeeString_END(args.ob)) {
			/* There is more here than just a floating point number. */
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Not a float point number %k",
			                       args.ob);
		}
		return 0;
	}

	/* Invoke the float-operator on everything else */
	return DeeObject_AsDouble(args.ob, &self->f_value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
float_bool(Float *__restrict self) {
	return self->f_value != 0.0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_str(Float *__restrict self) {
	return DeeString_Newf("%f", self->f_value);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
float_print(Float *__restrict self, Dee_formatprinter_t printer, void *arg) {
	return DeeFloat_PrintRepr(self, printer, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
float_double(Float *__restrict self, double *__restrict result) {
	*result = self->f_value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF Float *DCALL
float_neg(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(-self->f_value);
}

INTERN WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_add(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		goto err;
	return (DREF Float *)DeeFloat_New(self->f_value + other_val);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_sub(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		goto err;
	return (DREF Float *)DeeFloat_New(self->f_value - other_val);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_mul(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		goto err;
	return (DREF Float *)DeeFloat_New(self->f_value * other_val);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_div(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		goto err;
	return (DREF Float *)DeeFloat_New(self->f_value / other_val);
err:
	return NULL;
}

#ifndef INTPTR_MIN
#define INTPTR_MIN __INTPTR_MIN__
#endif /* !INTPTR_MIN */
#ifndef INTPTR_MAX
#define INTPTR_MAX __INTPTR_MAX__
#endif /* !INTPTR_MAX */

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
float_int(Float *__restrict self) {
	DREF DeeIntObject *result;
	double frac, v = self->f_value;
	int exp;
	/* Check if we can just directly cast to an intptr_t
	 * NOTE: We add +1.0 here because that results in a clean power-of-2,
	 *       which (combined with the use of ">" and "<") means that there
	 *       are no rounding inaccuracies here. */
	PRIVATE double const fmt_INTPTR_MIN = -((double)INTPTR_MAX + 1.0);
	PRIVATE double const fmt_INTPTR_MAX = +((double)INTPTR_MAX + 1.0);
	if (v > fmt_INTPTR_MIN && v < fmt_INTPTR_MAX)
		return (DREF DeeIntObject *)DeeInt_NewIntptr((intptr_t)v);


	/* Decompose "v" into it an expression:
	 * >> v == frac * 2**exp
	 * Where "frac" is in the range [0.5,1.0), and "exp" is positive
	 * NOTE: "frexp" can produce negative "exp", but only for small
	 *       float values, which we already filter by handling all
	 *       values that fit directly into intptr_t above. */
	v    = fabs(v);
	frac = frexp(v, &exp);
	ASSERT(frac >= 0.5);
	ASSERT(frac < 1.0);
	ASSERT(exp > 0);
	--exp; /* Treat as a bit-shift; needed because: 2 << (v-1) <==> 2**v */
	{
		div_t exp_rem = div(exp, Dee_DIGIT_BITS);
		frac   = ldexp(frac, exp_rem.rem + 1); /* Upscale for most significant digit */
		result = DeeInt_Alloc(exp_rem.quot + 1);
	}
	if likely(result) {
		size_t i = (size_t)result->ob_size - 1;
		for (;;) {
			digit d = (digit)frac;
			result->ob_digit[i] = (digit)d;
			if (i == 0)
				break;
			--i;
			frac = frac - (double)d;
			frac = ldexp(frac, Dee_DIGIT_BITS);
		}
		if (self->f_value < 0)
			result->ob_size = -result->ob_size;
	}
	return result;
}

PRIVATE struct type_math float_math = {
	/* .tp_int32  = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))NULL,
	/* .tp_int64  = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))NULL,
	/* .tp_double = */ (int (DCALL *)(DeeObject *__restrict, double *__restrict))&float_double,
	/* .tp_int    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_int,
	/* .tp_inv    = */ DEFIMPL_UNSUPPORTED(&default__inv__unsupported),
	/* .tp_pos    = */ &DeeObject_NewRef,
	/* .tp_neg    = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_neg,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_add,
	/* .tp_sub    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_sub,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_mul,
	/* .tp_div    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_div,
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_shl    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_shr    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_and    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_or     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_xor    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL(&default__dec__with__sub),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__inplace_sub__with__sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL(&default__inplace_div__with__div),
	/* .tp_inplace_mod = */ DEFIMPL_UNSUPPORTED(&default__inplace_mod__unsupported),
	/* .tp_inplace_shl = */ DEFIMPL_UNSUPPORTED(&default__inplace_shl__unsupported),
	/* .tp_inplace_shr = */ DEFIMPL_UNSUPPORTED(&default__inplace_shr__unsupported),
	/* .tp_inplace_and = */ DEFIMPL_UNSUPPORTED(&default__inplace_and__unsupported),
	/* .tp_inplace_or  = */ DEFIMPL_UNSUPPORTED(&default__inplace_or__unsupported),
	/* .tp_inplace_xor = */ DEFIMPL_UNSUPPORTED(&default__inplace_xor__unsupported),
	/* .tp_inplace_pow = */ DEFIMPL_UNSUPPORTED(&default__inplace_pow__unsupported),
};

STATIC_ASSERT(sizeof(double) == __SIZEOF_DOUBLE__);

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
float_hash(Float *__restrict self) {
#if __SIZEOF_DOUBLE__ == Dee_SIZEOF_HASH_T
	union {
		double f;
		Dee_hash_t i;
	} value;
	value.f = self->f_value;
	return value.i;
#elif __SIZEOF_DOUBLE__ >= 8
	union {
		double f;
		uint64_t i64;
		uint32_t i32[2];
	} value;
	value.f = self->f_value;
#if Dee_SIZEOF_HASH_T >= 8
	return (Dee_hash_t)value.i64;
#else /* Dee_SIZEOF_HASH_T >= 8 */
	return ((Dee_hash_t)value.i32[0] ^
	        (Dee_hash_t)value.i32[1]);
#endif /* Dee_SIZEOF_HASH_T < 8 */
#elif __SIZEOF_DOUBLE__ >= 4
	union {
		double f;
		uint32_t i;
	} value;
	value.f = self->f_value;
	return (Dee_hash_t)value.i;
#elif __SIZEOF_DOUBLE__ >= 2
	union {
		double f;
		uint16_t i;
	} value;
	value.f = self->f_value;
	return (Dee_hash_t)value.i;
#else /* __SIZEOF_DOUBLE__ >= ... */
	union {
		double f;
		uint8_t i;
	} value;
	value.f = self->f_value;
	return (Dee_hash_t)value.i;
#endif /* __SIZEOF_DOUBLE__ < ... */
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
float_compare(Float *self, DeeObject *other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		goto err;
	Dee_return_compare(self->f_value, other_val);
err:
	return Dee_COMPARE_ERR;
}

#define DEFINE_FLOAT_COMPARE(name, op)                    \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Float *self, DeeObject *other) {                 \
		double other_val;                                 \
		if (DeeObject_AsDouble(other, &other_val))        \
			goto err;                                     \
		return_bool(self->f_value op other_val);          \
	err:                                                  \
		return NULL;                                      \
	}
DEFINE_FLOAT_COMPARE(float_eq, ==)
DEFINE_FLOAT_COMPARE(float_ne, !=)
DEFINE_FLOAT_COMPARE(float_lo, <)
DEFINE_FLOAT_COMPARE(float_le, <=)
DEFINE_FLOAT_COMPARE(float_gr, >)
DEFINE_FLOAT_COMPARE(float_ge, >=)
#undef DEFINE_FLOAT_COMPARE

PRIVATE struct type_cmp float_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&float_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&float_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_ge,
};

#ifdef DBL_MIN
#define float_min_IS_CONSTANT
PRIVATE DEFINE_FLOAT(float_min, DBL_MIN);
#endif /* DBL_MIN */

#ifdef DBL_MAX
#define float_max_IS_CONSTANT
PRIVATE DEFINE_FLOAT(float_max, DBL_MAX);
#endif /* DBL_MAX */

#ifdef DBL_EPSILON
#define float_epsilon_IS_CONSTANT
PRIVATE DEFINE_FLOAT(float_epsilon, DBL_EPSILON);
#endif /* DBL_EPSILON */

#ifdef DBL_MIN_EXP
#define float_min_exp_IS_CONSTANT
PRIVATE DEFINE_INT15(float_min_exp, DBL_MIN_EXP);
#endif /* DBL_MIN_EXP */

#ifdef DBL_MIN_10_EXP
#define float_min_10_exp_IS_CONSTANT
PRIVATE DEFINE_INT15(float_min_10_exp, DBL_MIN_10_EXP);
#endif /* DBL_MIN_10_EXP */

#ifdef DBL_MAX_EXP
#define float_max_exp_IS_CONSTANT
PRIVATE DEFINE_INT15(float_max_exp, DBL_MAX_EXP);
#endif /* DBL_MAX_EXP */

#ifdef DBL_MAX_10_EXP
#define float_max_10_exp_IS_CONSTANT
PRIVATE DEFINE_INT15(float_max_10_exp, DBL_MAX_10_EXP);
#endif /* DBL_MAX_10_EXP */

#ifdef DBL_DIG
#define float_dig_IS_CONSTANT
PRIVATE DEFINE_INT15(float_dig, DBL_DIG);
#endif /* DBL_DIG */

#ifdef DBL_MANT_DIG
#define float_mant_dig_IS_CONSTANT
PRIVATE DEFINE_INT15(float_mant_dig, DBL_MANT_DIG);
#endif /* DBL_MANT_DIG */

#ifdef DBL_RADIX
#define float_radix_IS_CONSTANT
PRIVATE DEFINE_INT15(float_radix, DBL_RADIX);
#endif /* DBL_RADIX */

#undef float_rounds_IS_VARIABLE
#undef float_rounds_IS_CONSTANT
#ifdef CONFIG_HAVE_CONSTANT_DBL_ROUNDS
#define float_rounds_IS_CONSTANT
PRIVATE DEFINE_INT15(float_rounds, DBL_ROUNDS);
#elif defined(DBL_ROUNDS)
#define float_HAVE_VARIABLE
#define float_rounds_IS_VARIABLE
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_rounds(DeeObject *__restrict self) {
	(void)self;
	return DeeInt_NEWS(DBL_ROUNDS);
}
#endif /* ... */

#undef float_inf_IS_CONSTANT
#undef float_nan_IS_CONSTANT
#ifdef CONFIG_HAVE_IEEE754
#define float_inf_IS_CONSTANT
#define float_nan_IS_CONSTANT
struct Dee_float_ieee754_object {
#undef f_words
#undef f_double
	Dee_OBJECT_HEAD
	union {
		uint32_t f_words[2];
		double   f_double;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_ieee754_v
#define f_words  _ieee754_v.f_words
#define f_double _ieee754_v.f_double
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};
#ifdef CONFIG_HAVE_IEEE754_LE
#define FLOAT_IEEE754_INIT(msw, lsw) { Dee_OBJECT_HEAD_INIT(&DeeFloat_Type), { { lsw, msw } } }
#define float_ieee754_word0(x) (((struct Dee_float_ieee754_object *)(x))->f_words[0])
#define float_ieee754_word1(x) (((struct Dee_float_ieee754_object *)(x))->f_words[1])
#else /* CONFIG_HAVE_IEEE754_LE */
#define FLOAT_IEEE754_INIT(msw, lsw) { Dee_OBJECT_HEAD_INIT(&DeeFloat_Type), { { msw, lsw } } }
#define float_ieee754_word0(x) (((struct Dee_float_ieee754_object *)(x))->f_words[1])
#define float_ieee754_word1(x) (((struct Dee_float_ieee754_object *)(x))->f_words[0])
#endif /* !CONFIG_HAVE_IEEE754_LE */

PRIVATE struct Dee_float_ieee754_object float_inf =
FLOAT_IEEE754_INIT(UINT32_C(0x7ff00000), UINT32_C(0x00000000));
PRIVATE struct Dee_float_ieee754_object float_nan =
FLOAT_IEEE754_INIT(UINT32_C(0x7ff80000), UINT32_C(0x00000000));
#define float_inf_value float_inf.f_double
#define float_nan_value float_nan.f_double

#undef FLOAT_IEEE754_INIT
#elif defined(CONFIG_HAVE_CONSTANT_HUGE_VAL)
#define float_inf_IS_CONSTANT
PRIVATE DEFINE_FLOAT(float_inf, HUGE_VAL);
#define float_inf_value HUGE_VAL
#elif defined(HUGE_VAL)
#define float_inf_value HUGE_VAL
#define float_inf_IS_VARIABLE
#define float_HAVE_VARIABLE
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_inf(DeeObject *__restrict self) {
	(void)self;
	return DeeFloat_New(HUGE_VAL);
}
#endif /* ... */

#if !defined(float_nan_IS_CONSTANT)
#ifdef CONFIG_HAVE_CONSTANT_NAN
#define float_nan_IS_CONSTANT
#define float_nan_value NAN
PRIVATE DEFINE_FLOAT(float_nan, NAN);
#elif defined(NAN) || defined(CONFIG_HAVE_nan)
#define float_nan_IS_VARIABLE
#define float_HAVE_VARIABLE
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_nan(DeeObject *__restrict self) {
	(void)self;
#ifdef NAN
	return DeeFloat_New(NAN);
#define float_nan_value NAN
#else /* NAN */
	return DeeFloat_New(nan(""));
#define float_nan_value nan("")
#endif /* !NAN */
}
#endif /* CONFIG_HAVE_nan */
#endif /* !float_nan_IS_CONSTANT */



PRIVATE struct type_member tpconst float_class_members[] = {
#ifdef float_min_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("min", &float_min, "The lowest possible floating point value"),
#endif /* float_min_IS_CONSTANT */
#ifdef float_max_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("max", &float_max, "The greatest possible floating point value"),
#endif /* float_max_IS_CONSTANT */
#ifdef float_min_exp_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("min_exp", &float_min_exp, "Lowest binary exponent ($e such that ${radix ** (e - 1)} is a normalized ?Dfloat)"),
#endif /* float_min_exp_IS_CONSTANT */
#ifdef float_min_10_exp_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("min_10_exp", &float_min_10_exp, "Lowest decimal exponent ($e such that ${10 ** e} is a normalized ?Dfloat)"),
#endif /* float_min_10_exp_IS_CONSTANT */
#ifdef float_max_exp_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("max_exp", &float_max_exp, "Greatest binary exponent ($e such that ${radix ** (e - 1)} is representible as a ?Dfloat)"),
#endif /* float_max_exp_IS_CONSTANT */
#ifdef float_max_10_exp_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("max_10_exp", &float_max_10_exp, "Greatest decimal exponent ($e such that ${10 ** e} is representible as a ?Dfloat)"),
#endif /* float_max_10_exp_IS_CONSTANT */
#ifdef float_dig_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("dig", &float_dig, "The number of decimal precision digits"),
#endif /* float_dig_IS_CONSTANT */
#ifdef float_mant_dig_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("mant_dig", &float_mant_dig, "the number of bits in mantissa"),
#endif /* float_mant_dig_IS_CONSTANT */
#ifdef float_epsilon_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("epsilon", &float_epsilon, "Difference between ${1.0} and the next floating point value, such that ${1.0 + float.epsilon != 1.0}"),
#endif /* float_epsilon_IS_CONSTANT */
#ifdef float_radix_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("radix", &float_radix, "Exponent radix"),
#endif /* float_radix_IS_CONSTANT */
#ifdef float_rounds_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("rounds", &float_rounds, "Rounding mode"),
#endif /* float_rounds_IS_CONSTANT */
#ifdef float_inf_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("inf", &float_inf, "Positive infinity"),
#endif /* float_inf_IS_CONSTANT */
#ifdef float_nan_IS_CONSTANT
	TYPE_MEMBER_CONST_DOC("nan", &float_nan, "Not-a-number"),
#endif /* float_nan_IS_CONSTANT */
	TYPE_MEMBER_CONST(STR_isfloat, Dee_True),
	TYPE_MEMBER_END
};


#ifndef float_HAVE_VARIABLE
#define float_class_getsets NULL
#else /* !float_HAVE_VARIABLE */
PRIVATE struct type_getset tpconst float_class_getsets[] = {
#ifdef float_rounds_IS_VARIABLE
	TYPE_GETTER_AB("rounds", &float_rounds, "->?Dint\nRounding mode"),
#endif /* float_rounds_IS_VARIABLE */
#ifdef float_inf_IS_VARIABLE
	TYPE_GETTER_AB("inf", &float_inf, "->?.\nPositive infinity"),
#endif /* float_inf_IS_VARIABLE */
#ifdef float_nan_IS_VARIABLE
	TYPE_GETTER_AB("nan", &float_nan, "->?.\nNot-a-number"),
#endif /* float_nan_IS_VARIABLE */
	TYPE_GETSET_END
};
#endif /* float_HAVE_VARIABLE */


#ifdef CONFIG_HAVE_IEEE754
#define HAVE_float_get_abs
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_abs(Float *__restrict self) {
	DREF Float *result;
	result = (DREF Float *)DeeFloat_New(self->f_value);
	if likely(result)
		float_ieee754_word1(result) &= ~UINT32_C(0x80000000);
	return result;
}
#elif defined(CONFIG_HAVE_fabs)
#define HAVE_float_get_abs
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_abs(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(fabs(self->f_value));
}
#endif /* ... */

#ifdef CONFIG_HAVE_trunc
#define HAVE_float_get_trunc
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_trunc(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(trunc(self->f_value));
}
#endif /* CONFIG_HAVE_trunc */

#ifdef CONFIG_HAVE_floor
#define HAVE_float_get_floor
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_floor(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(floor(self->f_value));
}
#endif /* CONFIG_HAVE_floor */

#ifdef CONFIG_HAVE_ceil
#define HAVE_float_get_ceil
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_ceil(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(ceil(self->f_value));
}
#endif /* CONFIG_HAVE_ceil */

#ifdef CONFIG_HAVE_round
#define HAVE_float_get_round
INTERN WUNUSED NONNULL((1)) DREF Float *DCALL
float_get_round(Float *__restrict self) {
	return (DREF Float *)DeeFloat_New(round(self->f_value));
}
#endif /* CONFIG_HAVE_round */

#ifdef CONFIG_HAVE_IEEE754
#define HAVE_float_get_isnan
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isnan(Float *__restrict self) {
	uint32_t msw, lsw;
	lsw = float_ieee754_word0(self);
	msw = float_ieee754_word1(self);
	lsw |= msw & __UINT32_C(0xfffff);
	msw &= __UINT32_C(0x7ff00000);
	return_bool(msw == __UINT32_C(0x7ff00000) && lsw != 0);
}
#elif defined(CONFIG_HAVE_isnan)
#define HAVE_float_get_isnan
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isnan(Float *__restrict self) {
	return_bool(isnan(self->f_value));
}
#endif /* ... */

#ifdef CONFIG_HAVE_IEEE754
#define HAVE_float_get_isinf
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isinf(Float *__restrict self) {
	uint32_t msw, lsw;
	lsw = float_ieee754_word0(self);
	msw = float_ieee754_word1(self);
	lsw |= msw & __UINT32_C(0xfffff);
	msw &= __UINT32_C(0x7ff00000);
	return_bool(msw == __UINT32_C(0x7ff00000) && lsw == 0);
}
#elif defined(CONFIG_HAVE_isinf)
#define HAVE_float_get_isinf
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isinf(Float *__restrict self) {
	return_bool(isinf(self->f_value));
}
#endif /* ... */

#ifdef CONFIG_HAVE_IEEE754
#define HAVE_float_get_isfinite
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isfinite(Float *__restrict self) {
	int32_t hx = float_ieee754_word1(self);
	return_bool((int)((uint32_t)((hx & 0x7fffffff) - 0x7ff00000) >> 31));
}
#elif defined(CONFIG_HAVE_isfinite)
#define HAVE_float_get_isfinite
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isfinite(Float *__restrict self) {
	return_bool(isfinite(self->f_value));
}
#elif defined(CONFIG_HAVE_finite)
#define HAVE_float_get_isfinite
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isfinite(Float *__restrict self) {
	return_bool(finite(self->f_value));
}
#endif /* ... */


#ifdef CONFIG_HAVE_IEEE754
#define HAVE_float_get_isnormal
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isnormal(Float *__restrict self) {
	uint32_t msw, lsw;
	lsw = float_ieee754_word0(self);
	msw = float_ieee754_word1(self);
	lsw |= msw & __UINT32_C(0xfffff);
	msw &= __UINT32_C(0x7ff00000);
	return_bool(!((msw | lsw) == 0 ||
	              (msw == 0) ||
	              (msw == __UINT32_C(0x7ff00000))));
}
#elif defined(CONFIG_HAVE_isnormal)
#define HAVE_float_get_isnormal
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_get_isnormal(Float *__restrict self) {
	return_bool(isnormal(self->f_value));
}
#endif /* ... */

PRIVATE struct type_getset tpconst float_getsets[] = {
#ifdef HAVE_float_get_abs
	TYPE_GETTER_AB_F("abs", &float_get_abs, METHOD_FNOREFESCAPE, "->?."),
#endif /* HAVE_float_get_abs */
#ifdef HAVE_float_get_trunc
	TYPE_GETTER_AB_F("trunc", &float_get_trunc, METHOD_FNOREFESCAPE, "->?."),
#endif /* HAVE_float_get_trunc */
#ifdef HAVE_float_get_floor
	TYPE_GETTER_AB_F("floor", &float_get_floor, METHOD_FNOREFESCAPE, "->?."),
#endif /* HAVE_float_get_floor */
#ifdef HAVE_float_get_ceil
	TYPE_GETTER_AB_F("ceil", &float_get_ceil, METHOD_FNOREFESCAPE, "->?."),
#endif /* HAVE_float_get_ceil */
#ifdef HAVE_float_get_round
	TYPE_GETTER_AB_F("round", &float_get_round, METHOD_FNOREFESCAPE, "->?."),
#endif /* HAVE_float_get_round */
#ifdef HAVE_float_get_isnan
	TYPE_GETTER_AB_F("isnan", &float_get_isnan, METHOD_FNOREFESCAPE, "->?Dbool"),
#endif /* HAVE_float_get_isnan */
#ifdef HAVE_float_get_isinf
	TYPE_GETTER_AB_F("isinf", &float_get_isinf, METHOD_FNOREFESCAPE, "->?Dbool"),
#endif /* HAVE_float_get_isinf */
#ifdef HAVE_float_get_isfinite
	TYPE_GETTER_AB_F("isfinite", &float_get_isfinite, METHOD_FNOREFESCAPE, "->?Dbool"),
#endif /* HAVE_float_get_isfinite */
#ifdef HAVE_float_get_isnormal
	TYPE_GETTER_AB_F("isnormal", &float_get_isnormal, METHOD_FNOREFESCAPE, "->?Dbool"),
#endif /* HAVE_float_get_isnormal */
	TYPE_GETSET_END
};


#define DEFINE_FLOAT_COMPARE_FUNCTION(name)                          \
	INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL                \
	float_##name(Float *self, size_t argc, DeeObject *const *argv) { \
		double y;                                                    \
		if (DeeArg_UnpackStruct(argc, argv, "D:" #name, &y))         \
			goto err;                                                \
		return_bool(name(self->f_value, y));                         \
	err:                                                             \
		return NULL;                                                 \
	}
#ifdef CONFIG_HAVE_isgreater
#define HAVE_float_isgreater
DEFINE_FLOAT_COMPARE_FUNCTION(isgreater)
#endif /* CONFIG_HAVE_isgreater */
#ifdef CONFIG_HAVE_isgreaterequal
#define HAVE_float_isgreaterequal
DEFINE_FLOAT_COMPARE_FUNCTION(isgreaterequal)
#endif /* CONFIG_HAVE_isgreaterequal */
#ifdef CONFIG_HAVE_isless
#define HAVE_float_isless
DEFINE_FLOAT_COMPARE_FUNCTION(isless)
#endif /* CONFIG_HAVE_isless */
#ifdef CONFIG_HAVE_islessequal
#define HAVE_float_islessequal
DEFINE_FLOAT_COMPARE_FUNCTION(islessequal)
#endif /* CONFIG_HAVE_islessequal */
#ifdef CONFIG_HAVE_islessgreater
#define HAVE_float_islessgreater
DEFINE_FLOAT_COMPARE_FUNCTION(islessgreater)
#endif /* CONFIG_HAVE_islessgreater */
#undef DEFINE_FLOAT_COMPARE_FUNCTION

#ifdef HAVE_float_get_isnan
#define HAVE_float_isunordered
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_isunordered(Float *self, size_t argc, DeeObject *const *argv) {
	DeeObject *y_obj;
	double y;
	_DeeArg_Unpack1(err, argc, argv, "isunordered", &y_obj);
	if (DeeInt_Check(y_obj))
		return float_get_isnan(self);
	if (DeeObject_AsDouble(y_obj, &y))
		goto err;
#ifdef CONFIG_HAVE_isunordered
	return_bool(isunordered(self->f_value, y));
#elif defined(CONFIG_HAVE_isinf)
	return_bool(isinf(self->f_value) || isinf(y));
#else /* CONFIG_HAVE_isunordered */
	{
		DREF DeeObject *result;
		result = float_get_isnan(self);
		if (result != Dee_False)
			return result;
		ASSERT(result == Dee_True);
		Dee_DecrefNokill(Dee_True);
		return float_get_isnan(COMPILER_CONTAINER_OF(&y, Float, f_value));
	}
#endif /* !CONFIG_HAVE_isunordered */
err:
	return NULL;
}
#elif defined(CONFIG_HAVE_isunordered)
#define HAVE_float_isunordered
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_isunordered(Float *self, size_t argc, DeeObject *const *argv) {
	DeeObject *y_obj;
	double y;
	_DeeArg_Unpack1(err, argc, argv, "isunordered", &y_obj);
	if (DeeInt_Check(y_obj)) {
		y = 0.0;
	} else {
		if (DeeObject_AsDouble(y_obj, &y))
			goto err;
	}
	return_bool(isunordered(self->f_value, y));
err:
	return NULL;
}
#endif /* ... */

#if defined(CONFIG_HAVE_nextafter) || defined(CONFIG_HAVE_nexttoward)
#define HAVE_float_nextafter
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_nextafter(Float *self, size_t argc, DeeObject *const *argv) {
	double y;
	if (DeeArg_Unpack(argc, argv, "D:nextafter", &y))
		goto err;
#ifdef CONFIG_HAVE_nextafter
	return DeeFloat_New(nextafter(self->f_value, y));
#else /* CONFIG_HAVE_nextafter */
	return DeeFloat_New(nexttoward(self->f_value, (long double)y));
#endif /* !CONFIG_HAVE_nextafter */
err:
	return NULL;
}
#endif /* CONFIG_HAVE_nextafter || CONFIG_HAVE_nexttoward */


PRIVATE struct type_method tpconst float_methods[] = {
#ifdef HAVE_float_nextafter
	TYPE_METHOD_F("nextafter", &float_nextafter, METHOD_FNOREFESCAPE, "(y:?.)->?."),
#endif /* HAVE_float_nextafter */
#ifdef HAVE_float_isgreater
	TYPE_METHOD_F("isgreater", &float_isgreater, METHOD_FNOREFESCAPE, "(y:?.)->?Dbool"),
#endif /* HAVE_float_isgreater */
#ifdef HAVE_float_isgreaterequal
	TYPE_METHOD_F("isgreaterequal", &float_isgreaterequal, METHOD_FNOREFESCAPE, "(y:?.)->?Dbool"),
#endif /* HAVE_float_isgreaterequal */
#ifdef HAVE_float_isless
	TYPE_METHOD_F("isless", &float_isless, METHOD_FNOREFESCAPE, "(y:?.)->?Dbool"),
#endif /* HAVE_float_isless */
#ifdef HAVE_float_islessequal
	TYPE_METHOD_F("islessequal", &float_islessequal, METHOD_FNOREFESCAPE, "(y:?.)->?Dbool"),
#endif /* HAVE_float_islessequal */
#ifdef HAVE_float_islessgreater
	TYPE_METHOD_F("islessgreater", &float_islessgreater, METHOD_FNOREFESCAPE, "(y:?.)->?Dbool"),
#endif /* HAVE_float_islessgreater */
#ifdef HAVE_float_isunordered
	TYPE_METHOD_F("isunordered", &float_isunordered, METHOD_FNOREFESCAPE, "(y:?X2?.?Dint)->?Dbool"),
#endif /* HAVE_float_isunordered */
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst float_members[] = {
	TYPE_MEMBER_CONST(STR_isfloat, Dee_True),
	TYPE_MEMBER_END
};

#endif /* CONFIG_HAVE_FPU */

#ifdef CONFIG_HAVE_FPU
PRIVATE struct type_operator const float_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_000B_INT, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_000C_FLOAT, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_000E_POS, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000F_NEG, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0013_DIV, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
};
#endif /* CONFIG_HAVE_FPU */


#ifdef CONFIG_HAVE_FPU
PUBLIC DeeTypeObject DeeFloat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_float),
	/* .tp_doc      = */ DOC("()\n"
	                         "Returns the floating point constant ${0.0}\n"
	                         "\n"
	                         "(" float_float_params ")\n"
	                         "Parse a string into a floating point number, or convert a ?DNumeric @ob into a ?."),
	/* .tp_flags    = */ TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&float_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&float_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&float_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&float_init,
				TYPE_FIXED_ALLOCATOR(Float)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_str,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&float_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&float_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&float_print,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &float_math,
	/* .tp_cmp           = */ &float_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ float_methods,
	/* .tp_getsets       = */ float_getsets,
	/* .tp_members       = */ float_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ float_class_getsets,
	/* .tp_class_members = */ float_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ float_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(float_operators),
};
#else /* CONFIG_HAVE_FPU */
/* Prevent the computed-operator system from seeing this one */
#define _nofpu_DeeFloat_Type DeeFloat_Type
#define _nofpu_DeeFloat_name DeeString_STR(&str_float)
PUBLIC DeeTypeObject _nofpu_DeeFloat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ _nofpu_DeeFloat_name,
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(Float)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &float_math,
	/* .tp_cmp           = */ &float_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
#endif /* !CONFIG_HAVE_FPU */

DECL_END

#ifdef CONFIG_HAVE_FPU
#ifndef __INTELLISENSE__
#define DEFINE_DeeFloat_Print
#include "float-print.c.inl"
#define DEFINE_Dee_Strtod
#include "float-parse.c.inl"
#ifdef __COMPILER_HAVE_LONGDOUBLE
#define DEFINE_DeeFloat_LPrint
#include "float-print.c.inl"
#define DEFINE_Dee_Strtold
#include "float-parse.c.inl"
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#endif /* !__INTELLISENSE__ */
#endif /* CONFIG_HAVE_FPU */

#endif /* !GUARD_DEEMON_OBJECTS_FLOAT_C */
