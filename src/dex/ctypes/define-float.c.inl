/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#define DEE_SOURCE
#define DEFINE_T         double
#define DEFINE_TYPE_NAME DeeCDouble_Type
#endif /* __INTELLISENSE__ */

#ifndef DEFINE_T
#error "Must #define DEFINE_T before including this file"
#endif /* !DEFINE_T */

#ifndef DEFINE_TYPE_NAME
#error "Must #define DEFINE_TYPE_NAME before including this file"
#endif /* !DEFINE_TYPE_NAME */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>     /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>       /* DeeArg_Unpack1 */
#include <deemon/bool.h>      /* return_bool */
#include <deemon/error.h>     /* DeeError_NOTIMPLEMENTED */
#include <deemon/int.h>       /* DeeInt_NewInt64, INT_SIGNED */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_AsDouble, Dee_AsObject, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/string.h>    /* DeeString_Newf */
#include <deemon/type.h>      /* DeeObject_Init, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, INT_SIGNED, TF_NONE, TP_F* */
#include <deemon/util/lock.h> /* Dee_ATOMIC_RWLOCK_INIT */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* int32_t, int64_t */

#ifndef DEFINE_NAME
#define DEFINE_NAME PP_STR(DEFINE_T)
#endif /* !DEFINE_NAME */

DECL_BEGIN

#ifndef FLOATID_float
#define FLOATID_float       0
#define FLOATID_double      1
#define FLOATID_long_double 2
#define PRIVATE_FLOATID(x)  FLOATID_##x
#define FLOATID(x)          PRIVATE_FLOATID(x)
#endif /* !FLOATID_float */

#if LOCAL_TYPE_ID == 0
#define LOCAL_ALIGNOF CONFIG_ALIGNOF_FLOAT
#elif LOCAL_TYPE_ID == 1
#define LOCAL_ALIGNOF CONFIG_ALIGNOF_DOUBLE
#elif LOCAL_TYPE_ID == 2
#define LOCAL_ALIGNOF CONFIG_ALIGNOF_LDOUBLE
#else /* LOCAL_TYPE_ID == ... */
#define LOCAL_ALIGNOF COMPILER_ALIGNOF(DEFINE_T)
#endif /* LOCAL_TYPE_ID != ... */

#ifndef CONFIG_NO_CFUNCTION
#if LOCAL_TYPE_ID == 0
#define LOCAL_FFI_TYPE ffi_type_float
#elif LOCAL_TYPE_ID == 1
#define LOCAL_FFI_TYPE ffi_type_double
#elif LOCAL_TYPE_ID == 2
#define LOCAL_FFI_TYPE ffi_type_longdouble
#else /* LOCAL_TYPE_ID == ... */
#define LOCAL_FFI_TYPE ffi_type_void
#endif /* LOCAL_TYPE_ID != ... */
#endif /* !CONFIG_NO_CFUNCTION */


#define LOCAL_TYPE_ID FLOATID(DEFINE_T)
#define LOCAL_X(x)    PP_CAT2(x, DEFINE_TYPE_NAME)
#define LOCAL_F(x)    PP_CAT2(x, DEFINE_T)

#undef LOCAL_FLOAT_FUNCTIONS
#if LOCAL_TYPE_ID == 0
#ifndef FLOAT0_FUNCTIONS_DEFINED
#define FLOAT0_FUNCTIONS_DEFINED
#define LOCAL_FLOAT_FUNCTIONS
#endif /* !FLOAT0_FUNCTIONS_DEFINED */
#elif LOCAL_TYPE_ID == 1
#ifndef FLOAT1_FUNCTIONS_DEFINED
#define FLOAT1_FUNCTIONS_DEFINED
#define LOCAL_FLOAT_FUNCTIONS
#endif /* !FLOAT1_FUNCTIONS_DEFINED */
#elif LOCAL_TYPE_ID == 2
#ifndef FLOAT2_FUNCTIONS_DEFINED
#define FLOAT2_FUNCTIONS_DEFINED
#define LOCAL_FLOAT_FUNCTIONS
#endif /* !FLOAT2_FUNCTIONS_DEFINED */
#else /* LOCAL_TYPE_ID == ... */
#define LOCAL_FLOAT_FUNCTIONS
#endif /* LOCAL_TYPE_ID != ... */

/* TODO: Unaligned memory access */

#define LOCAL_Float             LOCAL_X(Float)
#define LOCAL_floatinit         LOCAL_F(floatinit)
#define LOCAL_floatass          LOCAL_F(floatass)
#define LOCAL_floatstr          LOCAL_F(floatstr)
#define LOCAL_floatbool         LOCAL_F(floatbool)
#define LOCAL_float_int32       LOCAL_F(float_int32)
#define LOCAL_float_int64       LOCAL_F(float_int64)
#define LOCAL_float_double      LOCAL_F(float_double)
#define LOCAL_float_int         LOCAL_F(float_int)
#define LOCAL_float_pos         LOCAL_F(float_pos)
#define LOCAL_float_neg         LOCAL_F(float_neg)
#define LOCAL_float_add         LOCAL_F(float_add)
#define LOCAL_float_sub         LOCAL_F(float_sub)
#define LOCAL_float_mul         LOCAL_F(float_mul)
#define LOCAL_float_div         LOCAL_F(float_div)
#define LOCAL_float_inplace_add LOCAL_F(float_inplace_add)
#define LOCAL_float_inplace_sub LOCAL_F(float_inplace_sub)
#define LOCAL_float_inplace_mul LOCAL_F(float_inplace_mul)
#define LOCAL_float_inplace_div LOCAL_F(float_inplace_div)
#define LOCAL_float_pow         LOCAL_F(float_pow)
#define LOCAL_float_inplace_pow LOCAL_F(float_inplace_pow)
#define LOCAL_floatmath         LOCAL_F(floatmath)
#define LOCAL_float_eq          LOCAL_F(float_eq)
#define LOCAL_float_ne          LOCAL_F(float_ne)
#define LOCAL_float_lo          LOCAL_F(float_lo)
#define LOCAL_float_le          LOCAL_F(float_le)
#define LOCAL_float_gr          LOCAL_F(float_gr)
#define LOCAL_float_ge          LOCAL_F(float_ge)
#define LOCAL_floatcmp          LOCAL_F(floatcmp)

typedef struct {
	OBJECT_HEAD
	DEFINE_T f_value; /* The floating point value. */
} LOCAL_Float;


#ifdef LOCAL_FLOAT_FUNCTIONS
#undef LOCAL_FLOAT_FUNCTIONS

#if LOCAL_TYPE_ID <= 1
/* Promotion to double. */
#define LOCAL_fltnew(val) float_newdouble((CONFIG_CTYPES_DOUBLE_TYPE)(val))
#ifndef FLOAT_NEWDOUBLE_DEFINED
#define FLOAT_NEWDOUBLE_DEFINED
typedef struct {
	OBJECT_HEAD
	CONFIG_CTYPES_DOUBLE_TYPE f_value; /* The floating point value. */
} Float_double_object;

PRIVATE WUNUSED DREF DeeObject *DCALL
float_newdouble(CONFIG_CTYPES_DOUBLE_TYPE val) {
	Float_double_object *result;
	result = DeeObject_MALLOC(Float_double_object);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeSType_AsType(&DeeCDouble_Type));
	result->f_value = val;
done:
	return Dee_AsObject(result);
}
#endif /* !INT_NEWINT_DEFINED */

#else /* LOCAL_TYPE_ID <= 1 */

#define LOCAL_fltnew LOCAL_F(fltnew)
PRIVATE WUNUSED DREF DeeObject *DCALL LOCAL_fltnew(DEFINE_T val) {
	LOCAL_Float * result;
	result = DeeObject_MALLOC(LOCAL_Float);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeSType_AsType(&DEFINE_TYPE_NAME));
	result->f_value = val;
done:
	return Dee_AsObject(result);
}
#endif /* LOCAL_TYPE_ID > 1 */

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_floatinit(DeeSTypeObject *__restrict UNUSED(tp_self),
                DEFINE_T *self, size_t argc, DeeObject *const *argv) {
	double value;
	DeeObject *arg;
	DeeArg_Unpack1(err, argc, argv, DEFINE_NAME, &arg);
	if (DeeObject_AsDouble(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(*self = (DEFINE_T)value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_floatass(DeeSTypeObject *__restrict UNUSED(tp_self),
               DEFINE_T *self, DeeObject *__restrict arg) {
	double value;
	if (DeeObject_AsDouble(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(*self = (DEFINE_T)value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_floatstr(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	double value;
	CTYPES_FAULTPROTECT(value = (double)*self, return NULL);
	return DeeString_Newf("%f", value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_floatbool(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return -1);
	return value != (DEFINE_T)0.0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_int32(DeeSTypeObject *__restrict UNUSED(tp_self),
                  DEFINE_T *self, int32_t *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return -1);
	*result = (int32_t)value;
	return INT_SIGNED;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_int64(DeeSTypeObject *__restrict UNUSED(tp_self),
				 DEFINE_T *self, int64_t *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return -1);
	*result = (int64_t)value;
	return INT_SIGNED;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_double(DeeSTypeObject *__restrict UNUSED(tp_self),
                   DEFINE_T *self, double *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return -1);
	*result = (double)value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_float_int(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return NULL);
	return DeeInt_NewInt64((int64_t)value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_float_pos(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return NULL);
	return LOCAL_fltnew(+value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_float_neg(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = *self, return NULL);
	return LOCAL_fltnew(-value);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_float_add(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                DeeObject *__restrict some_object) {
	DEFINE_T value;
	double other_value;
	CTYPES_FAULTPROTECT(value = *self, goto err);
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	return LOCAL_fltnew(value + other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_float_sub(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                DeeObject *__restrict some_object) {
	DEFINE_T value;
	double other_value;
	CTYPES_FAULTPROTECT(value = *self, goto err);
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	return LOCAL_fltnew(value - other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_float_mul(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                DeeObject *__restrict some_object) {
	DEFINE_T value;
	double other_value;
	CTYPES_FAULTPROTECT(value = *self, goto err);
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	return LOCAL_fltnew(value * other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_float_div(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                DeeObject *__restrict some_object) {
	DEFINE_T value;
	double other_value;
	CTYPES_FAULTPROTECT(value = *self, return NULL);
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	return LOCAL_fltnew(value / other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_inplace_add(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                        DeeObject *__restrict some_object) {
	double other_value;
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(*self += (DEFINE_T)other_value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_inplace_sub(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                        DeeObject *__restrict some_object) {
	double other_value;
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(*self -= (DEFINE_T)other_value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_inplace_mul(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                        DeeObject *__restrict some_object) {
	double other_value;
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(*self *= (DEFINE_T)other_value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_inplace_div(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                        DeeObject *__restrict some_object) {
	double other_value;
	if (DeeObject_AsDouble(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(*self /= (DEFINE_T)other_value, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
LOCAL_float_pow(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_float_inplace_pow(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                        DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return -1;
}


PRIVATE struct stype_math LOCAL_floatmath = {
	/* .st_int32       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int32_t *__restrict))&LOCAL_float_int32,
	/* .st_int64       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int64_t *__restrict))&LOCAL_float_int64,
	/* .st_double      = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, double *__restrict))&LOCAL_float_double,
	/* .st_int         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_float_int,
	/* .st_inv         = */ NULL,
	/* .st_pos         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_float_pos,
	/* .st_neg         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_float_neg,
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_add,
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_sub,
	/* .st_mul         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_mul,
	/* .st_div         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_div,
	/* .st_mod         = */ NULL,
	/* .st_shl         = */ NULL,
	/* .st_shr         = */ NULL,
	/* .st_and         = */ NULL,
	/* .st_or          = */ NULL,
	/* .st_xor         = */ NULL,
	/* .st_pow         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_pow,
	/* .st_inc         = */ NULL,
	/* .st_dec         = */ NULL,
	/* .st_inplace_add = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_inplace_add,
	/* .st_inplace_sub = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_inplace_sub,
	/* .st_inplace_mul = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_inplace_mul,
	/* .st_inplace_div = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_inplace_div,
	/* .st_inplace_mod = */ NULL,
	/* .st_inplace_shl = */ NULL,
	/* .st_inplace_shr = */ NULL,
	/* .st_inplace_and = */ NULL,
	/* .st_inplace_or  = */ NULL,
	/* .st_inplace_xor = */ NULL,
	/* .st_inplace_pow = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_inplace_pow
};

#define DEFINE_CTYPES_FLOAT_COMPARE(name, op)                 \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL     \
	name(DeeSTypeObject *__restrict UNUSED(tp_self),          \
	     DEFINE_T *self, DeeObject *__restrict some_object) { \
		DEFINE_T value;                                       \
		double other_value;                                   \
		CTYPES_FAULTPROTECT(value = *self, goto err);         \
		if (DeeObject_AsDouble(some_object, &other_value))    \
			goto err;                                         \
		return_bool(value op other_value);                    \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_eq, ==)
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_ne, !=)
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_lo, <)
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_le, <=)
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_gr, >)
DEFINE_CTYPES_FLOAT_COMPARE(LOCAL_float_ge, >=)
#undef DEFINE_CTYPES_FLOAT_COMPARE


PRIVATE struct stype_cmp LOCAL_floatcmp = {
     /* .st_eq = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_eq,
     /* .st_ne = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_ne,
     /* .st_lo = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_lo,
     /* .st_le = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_le,
     /* .st_gr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_gr,
     /* .st_ge = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_float_ge
};

#undef LOCAL_fltnew
#endif /* LOCAL_FLOAT_FUNCTIONS */


INTERN DeeSTypeObject DEFINE_TYPE_NAME = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT(&DeeSType_Type),
		/* .tp_name     = */ DEFINE_NAME,
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ LOCAL_Float,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ NULL,
				/* tp_serialize:   */ NULL /* TODO */
			),
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
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
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
	},
#ifndef CONFIG_NO_THREADS
	/* .st_cachelock = */ Dee_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .st_pointer  = */ NULL,
	/* .st_lvalue   = */ NULL,
	/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
	/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
	/* .st_ffitype  = */ &LOCAL_FFI_TYPE,
#endif /* !CONFIG_NO_CFUNCTION */
	/* .st_sizeof   = */ sizeof(DEFINE_T),
	/* .st_align    = */ LOCAL_ALIGNOF,
	/* .st_init     = */ (int (DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&LOCAL_floatinit,
	/* .st_assign   = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&LOCAL_floatass,
	/* .st_cast     = */ {
		/* .st_str  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_floatstr,
		/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_floatstr,
		/* .st_bool = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_floatbool
	},
	/* .st_call     = */ NULL,
	/* .st_math     = */ &LOCAL_floatmath,
	/* .st_cmp      = */ &LOCAL_floatcmp,
	/* .st_seq      = */ NULL,
	/* .st_attr     = */ NULL
};

#undef LOCAL_Float
#undef LOCAL_floatinit
#undef LOCAL_floatass
#undef LOCAL_floatstr
#undef LOCAL_floatbool
#undef LOCAL_float_int32
#undef LOCAL_float_int64
#undef LOCAL_float_double
#undef LOCAL_float_int
#undef LOCAL_float_pos
#undef LOCAL_float_neg
#undef LOCAL_float_add
#undef LOCAL_float_sub
#undef LOCAL_float_mul
#undef LOCAL_float_div
#undef LOCAL_float_inplace_add
#undef LOCAL_float_inplace_sub
#undef LOCAL_float_inplace_mul
#undef LOCAL_float_inplace_div
#undef LOCAL_float_pow
#undef LOCAL_float_inplace_pow
#undef LOCAL_floatmath
#undef LOCAL_float_eq
#undef LOCAL_float_ne
#undef LOCAL_float_lo
#undef LOCAL_float_le
#undef LOCAL_float_gr
#undef LOCAL_float_ge
#undef LOCAL_floatcmp

#undef LOCAL_X
#undef LOCAL_TYPE_ID
#undef LOCAL_ALIGNOF
#undef LOCAL_F
#undef LOCAL_FFI_TYPE

DECL_END

#undef DEFINE_NAME
#undef DEFINE_T
#undef DEFINE_TYPE_NAME
