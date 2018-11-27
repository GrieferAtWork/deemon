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
#ifdef __INTELLISENSE__
#define T         double
#define TYPE_NAME DeeCDouble_Type
#endif

#ifndef T
#error "Must #define T before including this file"
#endif
#ifndef TYPE_NAME
#error "Must #define TYPE_NAME before including this file"
#endif

#include "libctypes.h"
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/string.h>
#include <deemon/super.h>

DECL_BEGIN

#ifndef FLOATID_float
#define FLOATID_float       0
#define FLOATID_double      1
#define FLOATID_long_double 2
#define PRIVATE_FLOATID(x) FLOATID_##x
#define FLOATID(x) PRIVATE_FLOATID(x)
#endif

#define TYPE_ID  FLOATID(T)
#define X(x)     PP_CAT2(x,TYPE_NAME)
#define F(x)     PP_CAT2(x,T)

#undef DEFINE_FLOAT_FUNCTIONS
#if TYPE_ID == 0
#ifndef FLOAT0_FUNCTIONS_DEFINED
#define FLOAT0_FUNCTIONS_DEFINED 1
#define DEFINE_FLOAT_FUNCTIONS 1
#endif
#elif TYPE_ID == 1
#ifndef FLOAT1_FUNCTIONS_DEFINED
#define FLOAT1_FUNCTIONS_DEFINED 1
#define DEFINE_FLOAT_FUNCTIONS 1
#endif
#elif TYPE_ID == 2
#ifndef FLOAT2_FUNCTIONS_DEFINED
#define FLOAT2_FUNCTIONS_DEFINED 1
#define DEFINE_FLOAT_FUNCTIONS 1
#endif
#else
#define DEFINE_FLOAT_FUNCTIONS 1
#endif

/* TODO: Unaligned memory access */

typedef struct {
    OBJECT_HEAD
    T      f_value; /* The floating point value. */
} X(Float);


#ifdef DEFINE_FLOAT_FUNCTIONS
#undef DEFINE_FLOAT_FUNCTIONS

#if TYPE_ID <= 1
/* Promotion to double. */
#define NEW_FLOAT(val) float_newdouble((CONFIG_CTYPES_DOUBLE_TYPE)(val))
#ifndef FLOAT_NEWDOUBLE_DEFINED
#define FLOAT_NEWDOUBLE_DEFINED 1
typedef struct {
 OBJECT_HEAD
 CONFIG_CTYPES_DOUBLE_TYPE f_value; /* The integer value. */
} Float_double_object;
PRIVATE DREF DeeObject *DCALL float_newdouble(CONFIG_CTYPES_DOUBLE_TYPE val) {
 Float_double_object *result;
 result = DeeObject_MALLOC(Float_double_object);
 if unlikely(!result) goto done;
 DeeObject_Init(result,(DeeTypeObject *)&DeeCDouble_Type);
 result->f_value = val;
done:
 return (DREF DeeObject *)result;
}
#endif /* !INT_NEWINT_DEFINED */
#else
#define NEW_FLOAT(val) F(fltnew)(val)
PRIVATE DREF DeeObject *DCALL F(fltnew)(T val) {
 X(Float) *result;
 result = DeeObject_MALLOC(X(Float));
 if unlikely(!result) goto done;
 DeeObject_Init(result,(DeeTypeObject *)&TYPE_NAME);
 result->f_value = val;
done:
 return (DREF DeeObject *)result;
}
#endif


PRIVATE int DCALL
F(floatinit)(DeeSTypeObject *__restrict UNUSED(tp_self),
             T *self, size_t argc, DeeObject **__restrict argv) {
 double value; DeeObject *arg;
#ifdef NAME
 if (DeeArg_Unpack(argc,argv,"o:" NAME,&arg))
     goto err;
#else
 if (DeeArg_Unpack(argc,argv,"o:" PP_STR(T),&arg))
     goto err;
#endif
 if (DeeObject_AsDouble(arg,&value))
     goto err;
 CTYPES_FAULTPROTECT(*self = (T)value,goto err);
 return 0;
err:
 return -1;
}
PRIVATE int DCALL
F(floatass)(DeeSTypeObject *__restrict UNUSED(tp_self),
            T *self, DeeObject *__restrict arg) {
 double value;
 if (DeeObject_AsDouble(arg,&value))
     goto err;
 CTYPES_FAULTPROTECT(*self = (T)value,goto err);
 return 0;
err:
 return -1;
}
PRIVATE DREF DeeObject *DCALL
F(floatstr)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
 double value;
 CTYPES_FAULTPROTECT(value = (double)*self,return NULL);
 return DeeString_Newf("%f",value);
}
PRIVATE int DCALL
F(floatbool)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return -1);
 return value != (T)0.0;
}

PRIVATE int DCALL
F(float_int32)(DeeSTypeObject *__restrict UNUSED(tp_self),
               T *self, int32_t *__restrict result) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return -1);
 *result = (int32_t)value;
 return INT_SIGNED;
}
PRIVATE int DCALL
F(float_int64)(DeeSTypeObject *__restrict UNUSED(tp_self),
               T *self, int64_t *__restrict result) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return -1);
 *result = (int64_t)value;
 return INT_SIGNED;
}
PRIVATE int DCALL
F(float_double)(DeeSTypeObject *__restrict UNUSED(tp_self),
                T *self, double *__restrict result) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return -1);
 *result = (double)value;
 return 0;
}
PRIVATE DREF DeeObject *DCALL
F(float_int)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 return DeeInt_NewS64((int64_t)value);
}
PRIVATE DREF DeeObject *DCALL
F(float_pos)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 return NEW_FLOAT(+value);
}
PRIVATE DREF DeeObject *DCALL
F(float_neg)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
 T value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 return NEW_FLOAT(-value);
}
PRIVATE DREF DeeObject *DCALL
F(float_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
             DeeObject *__restrict some_object) {
 T value; double other_value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 if (DeeObject_AsDouble(some_object,&other_value))
     return NULL;
 return NEW_FLOAT(value+other_value);
}
PRIVATE DREF DeeObject *DCALL
F(float_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
             DeeObject *__restrict some_object) {
 T value; double other_value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 if (DeeObject_AsDouble(some_object,&other_value))
     return NULL;
 return NEW_FLOAT(value-other_value);
}
PRIVATE DREF DeeObject *DCALL
F(float_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
             DeeObject *__restrict some_object) {
 T value; double other_value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 if (DeeObject_AsDouble(some_object,&other_value))
     return NULL;
 return NEW_FLOAT(value*other_value);
}

PRIVATE ATTR_COLD void DCALL
F(float_divzero)(T value, DeeObject *__restrict some_object) {
 DeeError_Throwf(&DeeError_DivideByZero,
                 "Divide by Zero: `%f / %k'",
                (double)value,some_object);
}

PRIVATE DREF DeeObject *DCALL
F(float_div)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
             DeeObject *__restrict some_object) {
 T value; double other_value;
 CTYPES_FAULTPROTECT(value = *self,return NULL);
 if (DeeObject_AsDouble(some_object,&other_value))
     return NULL;
 if unlikely(!other_value) { F(float_divzero)(value,some_object); return NULL; }
 return NEW_FLOAT(value/other_value);
}
PRIVATE int DCALL
F(float_inplace_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                     DeeObject *__restrict some_object) {
 double other_value;
 if (DeeObject_AsDouble(some_object,&other_value))
     return -1;
 CTYPES_FAULTPROTECT(*self += (T)other_value,return -1);
 return 0;
}
PRIVATE int DCALL
F(float_inplace_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                     DeeObject *__restrict some_object) {
 double other_value;
 if (DeeObject_AsDouble(some_object,&other_value))
     return -1;
 CTYPES_FAULTPROTECT(*self -= (T)other_value,return -1);
 return 0;
}
PRIVATE int DCALL
F(float_inplace_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                     DeeObject *__restrict some_object) {
 double other_value;
 if (DeeObject_AsDouble(some_object,&other_value))
     return -1;
 CTYPES_FAULTPROTECT(*self *= (T)other_value,return -1);
 return 0;
}
PRIVATE int DCALL
F(float_inplace_div)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                     DeeObject *__restrict some_object) {
 double other_value;
 if (DeeObject_AsDouble(some_object,&other_value))
     return -1;
 if unlikely(!other_value) {
  T value;
  CTYPES_FAULTPROTECT(value = *self,return -1);
  F(float_divzero)(value,some_object);
  return -1;
 }
 CTYPES_FAULTPROTECT(*self /= (T)other_value,return -1);
 return 0;
}
PRIVATE DREF DeeObject *DCALL
F(float_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
             DeeObject *__restrict some_object) {
 (void)self;
 (void)some_object;
 /* TODO */
 DERROR_NOTIMPLEMENTED();
 return NULL;
}
PRIVATE int DCALL
F(float_inplace_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                     DeeObject *__restrict some_object) {
 (void)self;
 (void)some_object;
 /* TODO */
 DERROR_NOTIMPLEMENTED();
 return -1;
}


PRIVATE struct stype_math F(floatmath) = {
    /* .st_int32       = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,int32_t *__restrict))&F(float_int32),
    /* .st_int64       = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,int64_t *__restrict))&F(float_int64),
    /* .st_double      = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,double *__restrict))&F(float_double),
    /* .st_int         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*))&F(float_int),
    /* .st_inv         = */NULL,
    /* .st_pos         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*))&F(float_pos),
    /* .st_neg         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*))&F(float_neg),
    /* .st_add         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_add),
    /* .st_sub         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_sub),
    /* .st_mul         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_mul),
    /* .st_div         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_div),
    /* .st_mod         = */NULL,
    /* .st_shl         = */NULL,
    /* .st_shr         = */NULL,
    /* .st_and         = */NULL,
    /* .st_or          = */NULL,
    /* .st_xor         = */NULL,
    /* .st_pow         = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_pow),
    /* .st_inc         = */NULL,
    /* .st_dec         = */NULL,
    /* .st_inplace_add = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_inplace_add),
    /* .st_inplace_sub = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_inplace_sub),
    /* .st_inplace_mul = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_inplace_mul),
    /* .st_inplace_div = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_inplace_div),
    /* .st_inplace_mod = */NULL,
    /* .st_inplace_shl = */NULL,
    /* .st_inplace_shr = */NULL,
    /* .st_inplace_and = */NULL,
    /* .st_inplace_or  = */NULL,
    /* .st_inplace_xor = */NULL,
    /* .st_inplace_pow = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_inplace_pow)
};

#define DEFINE_COMPARE_OPERATOR(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(DeeSTypeObject *__restrict UNUSED(tp_self), \
     T *self, \
     DeeObject *__restrict some_object) { \
 T value; double other_value; \
 CTYPES_FAULTPROTECT(value = *self,return NULL); \
 if (DeeObject_AsDouble(some_object,&other_value)) \
     return NULL; \
 return_bool(value op other_value); \
}
DEFINE_COMPARE_OPERATOR(F(float_eq),==)
DEFINE_COMPARE_OPERATOR(F(float_ne),!=)
DEFINE_COMPARE_OPERATOR(F(float_lo),<)
DEFINE_COMPARE_OPERATOR(F(float_le),<=)
DEFINE_COMPARE_OPERATOR(F(float_gr),>)
DEFINE_COMPARE_OPERATOR(F(float_ge),>=)
#undef DEFINE_COMPARE_OPERATOR


PRIVATE struct stype_cmp F(floatcmp) = {
     /* .st_eq = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_eq),
     /* .st_ne = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_ne),
     /* .st_lo = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_lo),
     /* .st_le = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_le),
     /* .st_gr = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_gr),
     /* .st_ge = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(float_ge)
};

#undef NEW_FLOAT
#endif /* DEFINE_FLOAT_FUNCTIONS */


INTERN DeeSTypeObject TYPE_NAME = {
    /* .st_base = */{
        OBJECT_HEAD_INIT((DeeTypeObject *)&DeeSType_Type),
#ifdef NAME
        /* .tp_name     = */NAME,
#else
        /* .tp_name     = */PP_STR(T),
#endif
        /* .tp_doc      = */NULL,
        /* .tp_flags    = */TP_FNORMAL|TP_FTRUNCATE|TP_FINHERITCTOR,
        /* .tp_weakrefs = */0,
        /* .tp_features = */TF_NONE,
        /* .tp_base     = */(DeeTypeObject *)&DeeStructured_Type,
        /* .tp_init = */{
            {
                /* .tp_alloc = */{
                    /* .tp_ctor      = */NULL,
                    /* .tp_copy_ctor = */NULL,
                    /* .tp_deep_ctor = */NULL,
                    /* .tp_any_ctor  = */NULL,
                    TYPE_FIXED_ALLOCATOR(X(Float))
                }
            },
            /* .tp_dtor        = */NULL,
            /* .tp_assign      = */NULL,
            /* .tp_move_assign = */NULL
        },
        /* .tp_cast = */{
            /* .tp_str  = */NULL,
            /* .tp_repr = */NULL,
            /* .tp_bool = */NULL
        },
        /* .tp_call          = */NULL,
        /* .tp_visit         = */NULL,
        /* .tp_gc            = */NULL,
        /* .tp_math          = */NULL,
        /* .tp_cmp           = */NULL,
        /* .tp_seq           = */NULL,
        /* .tp_iter_next     = */NULL,
        /* .tp_attr          = */NULL,
        /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
        /* .tp_methods       = */NULL,
        /* .tp_getsets       = */NULL,
        /* .tp_members       = */NULL,
        /* .tp_class_methods = */NULL,
        /* .tp_class_getsets = */NULL,
        /* .tp_class_members = */NULL
    },
#ifndef CONFIG_NO_THREADS
    /* .st_cachelock = */RWLOCK_INIT,
#endif
    /* .st_pointer  = */NULL,
    /* .st_lvalue   = */NULL,
    /* .st_array    = */STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
    /* .st_cfunction= */STYPE_CFUNCTION_INIT,
#if TYPE_ID == 0
    /* .st_ffitype  = */&ffi_type_float,
#elif TYPE_ID == 1
    /* .st_ffitype  = */&ffi_type_double,
#elif TYPE_ID == 2
    /* .st_ffitype  = */&ffi_type_longdouble,
#else
    /* .st_ffitype  = */&ffi_type_void,
#endif
#endif /* !CONFIG_NO_CFUNCTION */
#if TYPE_ID == 0
    /* .st_align    = */CONFIG_ALIGNOF_FLOAT,
#elif TYPE_ID == 1
    /* .st_align    = */CONFIG_ALIGNOF_DOUBLE,
#elif TYPE_ID == 2
    /* .st_align    = */CONFIG_ALIGNOF_LDOUBLE,
#else
    /* .st_align    = */COMPILER_ALIGNOF(T),
#endif
    /* .st_init     = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,size_t,DeeObject **__restrict))&F(floatinit),
    /* .st_assign   = */(int(DCALL *)(DeeSTypeObject *__restrict,void*,DeeObject *__restrict))&F(floatass),
    /* .st_cast     = */{
        /* .st_str  = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*))&F(floatstr),
        /* .st_repr = */(DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict,void*))&F(floatstr),
        /* .st_bool = */(int(DCALL *)(DeeSTypeObject *__restrict,void*))&F(floatbool)
    },
    /* .st_call     = */NULL,
    /* .st_math     = */&F(floatmath),
    /* .st_cmp      = */&F(floatcmp),
    /* .st_seq      = */NULL,
    /* .st_attr     = */NULL
};

#undef NAME
#undef F
#undef X
#undef TYPE_ID
#undef T
#undef TYPE_NAME

DECL_END
