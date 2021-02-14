/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* CONFIG_HAVE_FLOAT_H */

#include <hybrid/floatcore.h>

#include "../runtime/strings.h"

#ifdef CONFIG_HAVE_FLOAT_H
#include <float.h>
#endif /* CONFIG_HAVE_FLOAT_H */

DECL_BEGIN

typedef DeeFloatObject Float;


LOCAL WUNUSED DREF Float *DCALL
DeeFloat_NewReuse(Float *__restrict self, double value) {
	DREF Float *result;
	if (!DeeObject_IsShared(self)) {
		result = self;
		Dee_Incref(result);
		goto done_ok;
	}
	result = DeeObject_MALLOC(Float);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeFloat_Type);
done_ok:
	result->f_value = value;
done:
	return result;
}

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
float_init(Float *__restrict self) {
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
float_ctor(Float *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeObject *arg;
	char *str;
	if (DeeArg_Unpack(argc, argv, "o:float", &arg))
		goto err;
	/* Invoke the float-operator on anything that isn't a string. */
	if (!DeeString_Check(arg))
		return DeeObject_AsDouble(arg, &self->f_value);
	/* TODO: String encodings? */
	str = DeeString_STR(arg);
	/* Skip leading space. */
	while (DeeUni_IsSpace(*str))
		++str;
	self->f_value = strtod(str, &str); /* FIXME: This needs a feature check! */
	/* Skip trailing space. */
	while (DeeUni_IsSpace(*str))
		++str;
	if (*str) {
		/* There is more here than just a floating point number. */
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Not a float point number %k",
		                       arg);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
float_str(Float *__restrict self) {
	return DeeString_Newf("%f", self->f_value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
float_bool(Float *__restrict self) {
	return self->f_value != 0.0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
float_double(Float *__restrict self, double *__restrict result) {
	*result = self->f_value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF Float *DCALL
float_neg(Float *__restrict self) {
	return DeeFloat_NewReuse(self, -self->f_value);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_add(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		return NULL;
	return DeeFloat_NewReuse(self, self->f_value + other_val);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_sub(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		return NULL;
	return DeeFloat_NewReuse(self, self->f_value - other_val);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_mul(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		return NULL;
	return DeeFloat_NewReuse(self, self->f_value * other_val);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Float *DCALL
float_div(Float *__restrict self,
          DeeObject *__restrict other) {
	double other_val;
	if (DeeObject_AsDouble(other, &other_val))
		return NULL;
	return DeeFloat_NewReuse(self, self->f_value / other_val);
}

PRIVATE struct type_math float_math = {
	/* .tp_int32  = */ (int (DCALL *)(DeeObject *__restrict, int32_t *__restrict))NULL,
	/* .tp_int64  = */ (int (DCALL *)(DeeObject *__restrict, int64_t *__restrict))NULL,
	/* .tp_double = */ (int (DCALL *)(DeeObject *__restrict, double *__restrict))&float_double,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
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
	/* .tp_pow    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif /* _MSC_VER */

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
float_hash(Float *__restrict self) {
	if (sizeof(double) == sizeof(dhash_t))
		return *(dhash_t *)&self->f_value;
	if (sizeof(double) >= sizeof(uint64_t)) {
#if __SIZEOF_POINTER__ == 4
		return ((dhash_t)((uint32_t *)&self->f_value)[0] ^
		        (dhash_t)((uint32_t *)&self->f_value)[1]);
#else /* __SIZEOF_POINTER__ == 4 */
		return (dhash_t) * (uint64_t *)&self->f_value;
#endif /* __SIZEOF_POINTER__ != 4 */
	}
	if (sizeof(double) >= sizeof(uint32_t))
		return (dhash_t) * (uint32_t *)&self->f_value;
	if (sizeof(double) >= sizeof(uint16_t))
		return (dhash_t) * (uint16_t *)&self->f_value;
	return (dhash_t) * (uint8_t *)&self->f_value;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */


#define DEFINE_FLOAT_CMP(name, op)                        \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL \
	name(Float *self, DeeObject *other) {                 \
		double other_val;                                 \
		if (DeeObject_AsDouble(other, &other_val))        \
			return NULL;                                  \
		return_bool(self->f_value op other_val);          \
	}
DEFINE_FLOAT_CMP(float_eq, ==)
DEFINE_FLOAT_CMP(float_ne, !=)
DEFINE_FLOAT_CMP(float_lo, <)
DEFINE_FLOAT_CMP(float_le, <=)
DEFINE_FLOAT_CMP(float_gr, >)
DEFINE_FLOAT_CMP(float_ge, >=)
#undef DEFINE_FLOAT_CMP

PRIVATE struct type_cmp float_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&float_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&float_ge,
};



DEFINE_FLOAT(float_min, DBL_MIN);
DEFINE_INT15(float_min_exp, DBL_MIN_EXP);
DEFINE_INT15(float_min_10_exp, DBL_MIN_10_EXP);
DEFINE_FLOAT(float_max, DBL_MAX);
DEFINE_INT15(float_max_exp, DBL_MAX_EXP);
DEFINE_INT15(float_max_10_exp, DBL_MAX_10_EXP);
DEFINE_INT15(float_dig, DBL_DIG);
DEFINE_INT15(float_mant_dig, DBL_MANT_DIG);
DEFINE_FLOAT(float_epsilon, DBL_EPSILON);

#ifdef _DBL_RADIX
DEFINE_INT15(float_radix, _DBL_RADIX);
#elif defined(DBL_RADIX)
DEFINE_INT15(float_radix, DBL_RADIX);
#elif defined(__DBL_RADIX__)
DEFINE_INT15(float_radix, __DBL_RADIX__);
#elif defined(FLT_RADIX)
DEFINE_INT15(float_radix, FLT_RADIX);
#elif defined(__FLT_RADIX__)
DEFINE_INT15(float_radix, __FLT_RADIX__);
#else
DEFINE_INT15(float_radix, 2);
#endif

#ifdef _DBL_ROUNDS
DEFINE_INT15(float_rounds, _DBL_ROUNDS);
#elif defined(DBL_ROUNDS)
DEFINE_INT15(float_rounds, DBL_ROUNDS);
#elif defined(__DBL_ROUNDS__)
DEFINE_INT15(float_rounds, __DBL_ROUNDS__);
#elif defined(FLT_ROUNDS)
DEFINE_INT15(float_rounds, FLT_ROUNDS);
#elif defined(__FLT_ROUNDS__)
DEFINE_INT15(float_rounds, __FLT_ROUNDS__);
#else
DEFINE_INT15(float_rounds, 1);
#endif

PRIVATE struct type_member tpconst float_class_members[] = {
	TYPE_MEMBER_CONST_DOC("min", &float_min, "The lowest possible floating point value"),
	TYPE_MEMBER_CONST_DOC("max", &float_max, "The greatest possible floating point value"),
	TYPE_MEMBER_CONST_DOC("min_exp", &float_min_exp, "Lowest binary exponent ($e such that ${radix ** (e - 1)} is a normalized :float)"),
	TYPE_MEMBER_CONST_DOC("min_10_exp", &float_min_10_exp, "Lowest decimal exponent ($e such that ${10 ** e} is a normalized :float)"),
	TYPE_MEMBER_CONST_DOC("max_exp", &float_max_exp, "Greatest binary exponent ($e such that ${radix ** (e - 1)} is representible as a :float)"),
	TYPE_MEMBER_CONST_DOC("max_10_exp", &float_max_10_exp, "Greatest decimal exponent ($e such that ${10 ** e} is representible as a :float)"),
	TYPE_MEMBER_CONST_DOC("dig", &float_dig, "The number of decimal precision digits"),
	TYPE_MEMBER_CONST_DOC("mant_dig", &float_mant_dig, "the number of bits in mantissa"),
	TYPE_MEMBER_CONST_DOC("epsilon", &float_epsilon, "Difference between ${1.0} and the next floating point value, such that ${1.0 + float.epsilon != 1.0}"),
	TYPE_MEMBER_CONST_DOC("radix", &float_radix, "Exponent radix"),
	TYPE_MEMBER_CONST_DOC("rounds", &float_rounds, "Rounding mode"),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeFloat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_float),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (int (DCALL *)(DeeTypeObject *__restrict, DeeObject *__restrict))&float_init,
				/* .tp_copy_ctor = */ (int (DCALL *)(DeeTypeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))&float_copy,
				/* .tp_deep_ctor = */ (int (DCALL *)(DeeTypeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))&float_copy,
				/* .tp_any_ctor  = */ (int (DCALL *)(DeeTypeObject *__restrict, size_t, DeeObject **__restrict))&float_ctor,
				TYPE_FIXED_ALLOCATOR(Float)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&float_str,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&float_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &float_math,
	/* .tp_cmp           = */ &float_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ float_class_members
};

DECL_END

#ifndef __INTELLISENSE__
#undef PRINT_LONG_DOUBLE
#ifdef __COMPILER_HAVE_LONGDOUBLE
#define PRINT_LONG_DOUBLE 1
#include "float-print.c.inl"
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#include "float-print.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_FLOAT_C */
