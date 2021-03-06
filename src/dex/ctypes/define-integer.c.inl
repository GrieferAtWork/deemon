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
#ifdef __INTELLISENSE__
#define DEE_SOURCE 1
#define SIZEOF 4
#define SIGNED 1
#endif /* __INTELLISENSE__ */

#ifndef SIZEOF
#error "Must #define SIZEOF before including this file"
#endif /* !SIZEOF */

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/super.h>

#include <hybrid/unaligned.h>

#include <stdint.h>

DECL_BEGIN

#ifndef X2
#define X3(x, y) x##y
#define X2(x, y) X3(x, y)
#endif /* !X2 */
#define X(x) X2(x, TYPE_NAME)



#ifndef INT8_C
#if defined(_MSC_VER) || __has_extension(tpp_msvc_integer_suffix)
#   define INT8_C(c)    c##i8
#   define INT16_C(c)   c##i16
#   define INT32_C(c)   c##i32
#   define INT64_C(c)   c##i64
#   define UINT8_C(c)   c##ui8
#   define UINT16_C(c)  c##ui16
#   define UINT32_C(c)  c##ui32
#   define UINT64_C(c)  c##ui64
#else /* _MSC_VER */
#   define INT8_C(c)    c
#   define INT16_C(c)   c
#if __SIZEOF_INT__ >= 4
#   define INT32_C(c)   c
#elif __SIZEOF_LONG__ >= 4
#   define INT32_C(c)   c##l
#else
#   error FIXME
#endif
#   define INT64_C(c)   c##ll
#   define UINT8_C(c)   c##u
#   define UINT16_C(c)  c##u
#   define UINT32_C(c)  c##u
#   define UINT64_C(c)  c##ull
#endif /* ... */
#endif /* !INT8_C */

#ifndef INT8_MIN
#define INT8_MIN  (-INT8_C(127)-INT8_C(1))
#define INT8_MAX    INT8_C(127)
#define UINT8_MAX   UINT8_C(0xff)
#define INT8_MIN  (-INT16_C(32767)-INT16_C(1))
#define INT8_MAX    INT16_C(32767)
#define UINT8_MAX   UINT16_C(0xffff)
#define INT8_MIN  (-INT32_C(2147483647)-INT32_C(1))
#define INT8_MAX    INT32_C(2147483647)
#define UINT8_MAX   UINT32_C(0xffffffff)
#define INT8_MIN  (-INT64_C(9223372036854775807)-INT64_C(1))
#define INT8_MAX    INT64_C(9223372036854775807)
#define UINT8_MAX   UINT64_C(0xffffffffffffffff)
#endif /* !INT8_MIN */



#ifdef SIGNED
#if SIZEOF == 1
#define F(x) x##s8
#elif SIZEOF == 2
#define F(x) x##s16
#elif SIZEOF == 4
#define F(x) x##s32
#elif SIZEOF == 8
#define F(x) x##s64
#else /* SIZEOF == ... */
#define F(x) X2(x, TYPE_NAME)
#endif /* SIZEOF != ... */
#else /* SIGNED */
#if SIZEOF == 1
#define F(x) x##u8
#elif SIZEOF == 2
#define F(x) x##u16
#elif SIZEOF == 4
#define F(x) x##u32
#elif SIZEOF == 8
#define F(x) x##u64
#else /* SIZEOF == ... */
#define F(x) X2(x, TYPE_NAME)
#endif /* SIZEOF != ... */
#endif /* !SIGNED */

#ifndef SIGNED_TYPE_NAME
#if SIZEOF == 1
#define SIGNED_TYPE_NAME  DeeCInt8_Type
#elif SIZEOF == 2
#define SIGNED_TYPE_NAME  DeeCInt16_Type
#elif SIZEOF == 4
#define SIGNED_TYPE_NAME  DeeCInt32_Type
#elif SIZEOF == 8
#define SIGNED_TYPE_NAME  DeeCInt64_Type
#else /* SIZEOF == ... */
#error "Must #define SIGNED_TYPE_NAME"
#endif /* SIZEOF != ... */
#endif /* !SIGNED_TYPE_NAME */

#ifndef UNSIGNED_TYPE_NAME
#if SIZEOF == 1
#define UNSIGNED_TYPE_NAME  DeeCUInt8_Type
#elif SIZEOF == 2
#define UNSIGNED_TYPE_NAME  DeeCUInt16_Type
#elif SIZEOF == 4
#define UNSIGNED_TYPE_NAME  DeeCUInt32_Type
#elif SIZEOF == 8
#define UNSIGNED_TYPE_NAME  DeeCUInt64_Type
#else /* SIZEOF == ... */
#error "Must #define UNSIGNED_TYPE_NAME"
#endif /* SIZEOF != ... */
#endif /* !UNSIGNED_TYPE_NAME */

#ifndef TYPE_NAME
#ifdef SIGNED
#define TYPE_NAME  SIGNED_TYPE_NAME
#else /* SIGNED */
#define TYPE_NAME  UNSIGNED_TYPE_NAME
#endif /* !SIGNED */
#ifndef TYPE_NAME
#error "Must #define TYPE_NAME for custom-width integer"
#endif /* !TYPE_NAME */
#endif /* !TYPE_NAME */

#ifndef FORMAT_STR
#ifdef SIGNED
#if SIZEOF <= 1
#define FORMAT_STR  "%I8d"
#define FORMAT_TYP  int8_t
#elif SIZEOF <= 2
#define FORMAT_STR  "%I16d"
#define FORMAT_TYP  int16_t
#elif SIZEOF <= 4
#define FORMAT_STR  "%I32d"
#define FORMAT_TYP  int32_t
#elif SIZEOF <= 8
#define FORMAT_STR  "%I64d"
#define FORMAT_TYP  int64_t
#endif /* SIZEOF <= ... */
#else /* SIGNED */
#if SIZEOF <= 1
#define FORMAT_STR  "%I8u"
#define FORMAT_TYP  uint8_t
#elif SIZEOF <= 2
#define FORMAT_STR  "%I16u"
#define FORMAT_TYP  uint16_t
#elif SIZEOF <= 4
#define FORMAT_STR  "%I32u"
#define FORMAT_TYP  uint32_t
#elif SIZEOF <= 8
#define FORMAT_STR  "%I64u"
#define FORMAT_TYP  uint64_t
#endif /* SIZEOF <= ... */
#endif /* !SIGNED */
#ifndef FORMAT_STR
#error "Must #define FORMAT_STR for extended-width integer"
#endif /* !FORMAT_STR */
#endif /* !FORMAT_STR */

#ifndef T
#ifdef SIGNED
#if SIZEOF == 1
#define T  int8_t
#elif SIZEOF == 2
#define T  int16_t
#elif SIZEOF == 4
#define T  int32_t
#elif SIZEOF == 8
#define T  int64_t
#endif /* SIZEOF == ... */
#else /* SIGNED */
#if SIZEOF == 1
#define T  uint8_t
#elif SIZEOF == 2
#define T  uint16_t
#elif SIZEOF == 4
#define T  uint32_t
#elif SIZEOF == 8
#define T  uint64_t
#endif /* SIZEOF == ... */
#endif /* !SIGNED */
#ifndef T
#error "Must #define T for custom-width integer"
#endif /* !T */
#endif /* !T */

#if SIZEOF == 1
#define F_NOSIGN(x) x##ns8
#elif SIZEOF == 2
#define F_NOSIGN(x) x##ns16
#elif SIZEOF == 4
#define F_NOSIGN(x) x##ns32
#elif SIZEOF == 8
#define F_NOSIGN(x) x##ns64
#else /* SIZEOF == ... */
#define F_NOSIGN(x) X(x##ns)
#endif /* SIZEOF != ... */

#ifndef GET
#ifdef SIGNED
#if SIZEOF == 1
#define GET(ptr)    (*(int8_t *)(ptr))
#define SET(ptr, v) (void)(*(int8_t *)(ptr) = (v))
#elif SIZEOF == 2
#define GET(ptr)    ((int16_t)UNALIGNED_GET16(ptr))
#define SET(ptr, v) UNALIGNED_SET16(ptr, (uint16_t)(v))
#elif SIZEOF == 4
#define GET(ptr)    ((int32_t)UNALIGNED_GET32(ptr))
#define SET(ptr, v) UNALIGNED_SET32(ptr, (uint32_t)(v))
#elif SIZEOF == 8
#define GET(ptr)    ((int64_t)UNALIGNED_GET64(ptr))
#define SET(ptr, v) UNALIGNED_SET64(ptr, (uint64_t)(v))
#elif SIZEOF == 16
#define GET(ptr)    ((__INT128_TYPE__)UNALIGNED_GET128(ptr))
#define SET(ptr, v) UNALIGNED_SET128(ptr, (__UINT128_TYPE__)(v))
#endif /* SIZEOF == ... */
#else /* SIGNED */
#if SIZEOF == 1
#define GET(ptr)    (*(uint8_t *)(ptr))
#define SET(ptr, v) (void)(*(uint8_t *)(ptr) = (v))
#elif SIZEOF == 2
#define GET(ptr)    UNALIGNED_GET16(ptr)
#define SET(ptr, v) UNALIGNED_SET16(ptr, v)
#elif SIZEOF == 4
#define GET(ptr)    UNALIGNED_GET32(ptr)
#define SET(ptr, v) UNALIGNED_SET32(ptr, v)
#elif SIZEOF == 8
#define GET(ptr)    UNALIGNED_GET64(ptr)
#define SET(ptr, v) UNALIGNED_SET64(ptr, v)
#elif SIZEOF == 16
#define GET(ptr)    UNALIGNED_GET128(ptr)
#define SET(ptr, v) UNALIGNED_SET128(ptr, v)
#endif /* SIZEOF == ... */
#endif /* !SIGNED */
#endif /* !GET */



typedef struct {
	OBJECT_HEAD
	T      i_value; /* The integer value. */
} X(Integer);


#if ((SIZEOF != 1 || (defined(SIGNED) ? !defined(INT8_FUNCTIONS_DEFINED) : !defined(UINT8_FUNCTIONS_DEFINED))) &&   \
     (SIZEOF != 2 || (defined(SIGNED) ? !defined(INT16_FUNCTIONS_DEFINED) : !defined(UINT16_FUNCTIONS_DEFINED))) && \
     (SIZEOF != 4 || (defined(SIGNED) ? !defined(INT32_FUNCTIONS_DEFINED) : !defined(UINT32_FUNCTIONS_DEFINED))) && \
     (SIZEOF != 8 || (defined(SIGNED) ? !defined(INT64_FUNCTIONS_DEFINED) : !defined(UINT64_FUNCTIONS_DEFINED))))
#ifdef SIGNED
#if SIZEOF == 1
#define INT8_FUNCTIONS_DEFINED  1
#elif SIZEOF == 2
#define INT16_FUNCTIONS_DEFINED 1
#elif SIZEOF == 4
#define INT32_FUNCTIONS_DEFINED 1
#elif SIZEOF == 8
#define INT64_FUNCTIONS_DEFINED 1
#endif /* SIZEOF == ... */
#else /* SIGNED */
#if SIZEOF == 1
#define UINT8_FUNCTIONS_DEFINED  1
#elif SIZEOF == 2
#define UINT16_FUNCTIONS_DEFINED 1
#elif SIZEOF == 4
#define UINT32_FUNCTIONS_DEFINED 1
#elif SIZEOF == 8
#define UINT64_FUNCTIONS_DEFINED 1
#endif /* SIZEOF == ... */
#endif /* !SIGNED */

#if SIZEOF == 1
#ifndef INT8_SIGNLESS_FUNCTIONS_DEFINED
#define INT8_SIGNLESS_FUNCTIONS_DEFINED  1
#else /* !INT8_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED                 1
#endif /* INT8_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 2
#ifndef INT16_SIGNLESS_FUNCTIONS_DEFINED
#define INT16_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT16_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED                 1
#endif /* INT16_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 4
#ifndef INT32_SIGNLESS_FUNCTIONS_DEFINED
#define INT32_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT32_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED                 1
#endif /* INT32_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 8
#ifndef INT64_SIGNLESS_FUNCTIONS_DEFINED
#define INT64_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT64_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED                 1
#endif /* INT64_SIGNLESS_FUNCTIONS_DEFINED */
#endif /* SIZEOF == ... */


#ifdef SIGNED
#define OBJECT_AS_T(x, result) DeeObject_AsXInt(SIZEOF, x, result)
#else /* SIGNED */
#define OBJECT_AS_T(x, result) DeeObject_AsXUInt(SIZEOF, x, result)
#endif /* !SIGNED */

#ifndef INT_NEWINT_DEFINED
#define INT_NEWINT_DEFINED 1
typedef struct {
	OBJECT_HEAD
	CTYPES_INT i_value; /* The integer value. */
} Integer_int_object;

INTERN WUNUSED DREF DeeObject *DCALL int_newint(CTYPES_INT val) {
	Integer_int_object *result;
	result = DeeObject_MALLOC(Integer_int_object);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, (DeeTypeObject *)&DeeCInt_Type);
	result->i_value = val;
done:
	return (DREF DeeObject *)result;
}
#endif /* !INT_NEWINT_DEFINED */

#if !defined(SIGNED) || SIZEOF != CONFIG_CTYPES_SIZEOF_INT
INTERN WUNUSED DREF DeeObject *DCALL F(int_new)(T val) {
	X(Integer) * result;
	result = DeeObject_MALLOC(X(Integer));
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, (DeeTypeObject *)&TYPE_NAME);
	result->i_value = val;
done:
	return (DREF DeeObject *)result;
}
#endif /* !SIGNED || SIZEOF != CONFIG_CTYPES_SIZEOF_INT */

#if ((SIZEOF < CONFIG_CTYPES_SIZEOF_INT) || \
     (defined(SIGNED) && SIZEOF == CONFIG_CTYPES_SIZEOF_INT))
/* Integer promotion. */
#define NEW_INTEGER(val) int_newint((CTYPES_INT)(val))
#else /* ... */
#define NEW_INTEGER(val) F(int_new)(val)
#endif /* !... */


/* Define functions. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
F(intinit)(DeeSTypeObject *__restrict UNUSED(tp_self),
           T *self, size_t argc, DeeObject *const *argv) {
	T value;
	DeeObject *arg;
#ifdef NAME
	if (DeeArg_Unpack(argc, argv, "o:" NAME, &arg))
		goto err;
#else /* NAME */
	if (DeeArg_Unpack(argc, argv, "o:" PP_STR(T), &arg))
		goto err;
#endif /* !NAME */
	if (OBJECT_AS_T(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, value), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(intass)(DeeSTypeObject *__restrict UNUSED(tp_self),
          T *self, DeeObject *__restrict arg) {
	T value;
	if (OBJECT_AS_T(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, value), goto err);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_BOOL_STRING
#ifndef BOOL_STRINGS_DEFINED
#define BOOL_STRINGS_DEFINED 1
PRIVATE DEFINE_STRING(str_true, "true");
PRIVATE DEFINE_STRING(str_false, "false");
#endif /* !BOOL_STRINGS_DEFINED */
#endif /* CONFIG_BOOL_STRING */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(intstr)(DeeSTypeObject *__restrict UNUSED(tp_self),
          T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#ifdef CONFIG_BOOL_STRING
	return_reference_(value ? (DeeObject *)&str_true : (DeeObject *)&str_false);
#else /* CONFIG_BOOL_STRING */
	return DeeString_Newf(FORMAT_STR, (FORMAT_TYP)value);
#endif /* !CONFIG_BOOL_STRING */
}

#ifndef SIGNLESS_DEFINED
PRIVATE WUNUSED NONNULL((1)) int DCALL
F_NOSIGN(intbool)(DeeSTypeObject *__restrict UNUSED(tp_self),
                  T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self),
	                    return -1);
	return value != 0;
}
#endif /* !SIGNLESS_DEFINED */

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_int32)(DeeSTypeObject *__restrict UNUSED(tp_self),
             T *self, int32_t *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
#ifdef SIGNED
	*result = (int32_t)value;
	return INT_SIGNED;
#else /* SIGNED */
	*(uint32_t *)result = (uint32_t)value;
	return INT_UNSIGNED;
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_int64)(DeeSTypeObject *__restrict UNUSED(tp_self),
             T *self, int64_t *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
#ifdef SIGNED
	*result = (int64_t)value;
	return INT_SIGNED;
#else /* SIGNED */
	*(uint64_t *)result = (uint64_t)value;
	return INT_UNSIGNED;
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_double)(DeeSTypeObject *__restrict UNUSED(tp_self),
              T *self, double *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
	*result = (double)value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_int)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#ifdef SIGNED
	return DeeInt_New(SIZEOF, value);
#else /* SIGNED */
	return DeeInt_Newu(SIZEOF, value);
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_inv)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
	return NEW_INTEGER(~value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_pos)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
	return NEW_INTEGER(+value);
}

#ifdef SIGNED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_neg)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
	return NEW_INTEGER(-value);
}
#endif /* SIGNED */

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value + other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value - other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value * other_value);
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
F(int_divzero)(T value, DeeObject *__restrict some_object) {
	return DeeError_Throwf(&DeeError_DivideByZero,
	                       "Divide by Zero: `" FORMAT_STR " / %k'",
	                       (FORMAT_TYP)value, some_object);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_div)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	if unlikely(!other_value) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	return NEW_INTEGER(value / other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_mod)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	if unlikely(!other_value) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	return NEW_INTEGER(value % other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value;
	unsigned int other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value << other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value;
	unsigned int other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value >> other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_and)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value & other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_or)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
          DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value | other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_xor)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	return NEW_INTEGER(value ^ other_value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(int_inc)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) + 1)), return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(int_dec)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) - 1)), return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) + other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) - other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) * other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_div)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	if unlikely(!other_value) {
		T value;
		CTYPES_FAULTPROTECT(value = GET(self), goto err);
		(F(int_divzero)(value, some_object));
		goto err;
	}
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) / other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_mod)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
	if unlikely(!other_value) {
		T value;
		CTYPES_FAULTPROTECT(value = GET(self), goto err);
		(F(int_divzero)(value, some_object));
		goto err;
	}
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) % other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	unsigned int other_value;
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) << other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	unsigned int other_value;
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) >> other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_and)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	unsigned int other_value;
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) & other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_or)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                  DeeObject *__restrict some_object) {
	unsigned int other_value;
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) | other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_xor)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	unsigned int other_value;
	if (DeeObject_AsUInt(some_object, &other_value))
		goto err;
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) ^ other_value)), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return -1;
}


PRIVATE struct stype_math F(intmath) = {
	/* .st_int32       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int32_t *__restrict))&F(int_int32),
	/* .st_int64       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int64_t *__restrict))&F(int_int64),
	/* .st_double      = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, double *__restrict))&F(int_double),
	/* .st_int         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_int),
	/* .st_inv         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_inv),
	/* .st_pos         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_pos),
#ifdef SIGNED
	/* .st_neg         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_neg),
#else /* SIGNED */
	/* .st_neg         = */ NULL,
#endif /* !SIGNED */
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_add),
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_sub),
	/* .st_mul         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_mul),
	/* .st_div         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_div),
	/* .st_mod         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_mod),
	/* .st_shl         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_shl),
	/* .st_shr         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_shr),
	/* .st_and         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_and),
	/* .st_or          = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_or),
	/* .st_xor         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_xor),
	/* .st_pow         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_pow),
	/* .st_inc         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_inc),
	/* .st_dec         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&F(int_dec),
	/* .st_inplace_add = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_add),
	/* .st_inplace_sub = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_sub),
	/* .st_inplace_mul = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_mul),
	/* .st_inplace_div = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_div),
	/* .st_inplace_mod = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_mod),
	/* .st_inplace_shl = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_shl),
	/* .st_inplace_shr = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_shr),
	/* .st_inplace_and = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_and),
	/* .st_inplace_or  = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_or),
	/* .st_inplace_xor = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_xor),
	/* .st_inplace_pow = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_inplace_pow)
};

#define DEFINE_COMPARE_OPERATOR(name, op)                 \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL \
	name(DeeSTypeObject *UNUSED(tp_self),                 \
	     T *self, DeeObject *some_object) {               \
		T value, other_value;                             \
		CTYPES_FAULTPROTECT(value = GET(self), goto err); \
		if (OBJECT_AS_T(some_object, &other_value))       \
			goto err;                                     \
		return_bool(value op other_value);                \
	err:                                                  \
		return NULL;                                      \
	}
DEFINE_COMPARE_OPERATOR(F(int_eq), ==)
DEFINE_COMPARE_OPERATOR(F(int_ne), !=)
DEFINE_COMPARE_OPERATOR(F(int_lo), <)
DEFINE_COMPARE_OPERATOR(F(int_le), <=)
DEFINE_COMPARE_OPERATOR(F(int_gr), >)
DEFINE_COMPARE_OPERATOR(F(int_ge), >=)
#undef DEFINE_COMPARE_OPERATOR

PRIVATE struct stype_cmp F(intcmp) = {
	/* .st_eq = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_eq),
	/* .st_ne = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_ne),
	/* .st_lo = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_lo),
	/* .st_le = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_le),
	/* .st_gr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_gr),
	/* .st_ge = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(int_ge)
};

#undef SIGNLESS_DEFINED
#undef NEW_INTEGER
#undef OBJECT_AS_T
#endif /* Integer functions. */


PRIVATE X(Integer) X(int_min) = {
	OBJECT_HEAD_INIT((DeeTypeObject *)&TYPE_NAME),
#ifdef SIGNED
#if SIZEOF == 1
	INT8_MIN
#elif SIZEOF == 2
	INT16_MIN
#elif SIZEOF == 4
	INT32_MIN
#else /* SIZEOF == ... */
	INT64_MIN
#endif /* SIZEOF != ... */
#else /* SIGNED */
	0
#endif /* !SIGNED */
};

PRIVATE X(Integer) X(int_max) = {
	OBJECT_HEAD_INIT((DeeTypeObject *)&TYPE_NAME),
#ifdef SIGNED
#if SIZEOF == 1
	INT8_MAX
#elif SIZEOF == 2
	INT16_MAX
#elif SIZEOF == 4
	INT32_MAX
#else /* SIZEOF == ... */
	INT64_MAX
#endif /* SIZEOF != ... */
#else /* SIGNED */
#if SIZEOF == 1
	UINT8_MAX
#elif SIZEOF == 2
	UINT16_MAX
#elif SIZEOF == 4
	UINT32_MAX
#else /* SIZEOF == ... */
	UINT64_MAX
#endif /* SIZEOF != ... */
#endif /* !SIGNED */
};

PRIVATE struct type_member tpconst X(int_class_members)[] = {
#if !defined(NO_SIGNED_TYPE_NAME) && !defined(NO_UNSIGNED_TYPE_NAME)
	TYPE_MEMBER_CONST("signed", (DeeObject *)&SIGNED_TYPE_NAME),
	TYPE_MEMBER_CONST("unsigned", (DeeObject *)&UNSIGNED_TYPE_NAME),
#endif /* !NO_SIGNED_TYPE_NAME && !NO_UNSIGNED_TYPE_NAME */
	TYPE_MEMBER_CONST("min", &X(int_min)),
	TYPE_MEMBER_CONST("max", &X(int_max)),
	TYPE_MEMBER_END
};


INTERN DeeSTypeObject TYPE_NAME = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT((DeeTypeObject *)&DeeSType_Type),
#ifdef NAME
		/* .tp_name     = */ NAME,
#else /* NAME */
		/* .tp_name     = */ PP_STR(T),
#endif /* !NAME */
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeeStructured_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(X(Integer))
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
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
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
		/* .tp_class_members = */ X(int_class_members)
	},
#ifndef CONFIG_NO_THREADS
	/* .st_cachelock = */ RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .st_pointer  = */ NULL,
	/* .st_lvalue   = */ NULL,
	/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
	/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
#ifdef FFI_TYPE
	/* .st_ffitype  = */ &FFI_TYPE,
#elif SIZEOF == 1
#ifdef SIGNED
	/* .st_ffitype  = */ &ffi_type_sint8,
#else /* SIGNED */
	/* .st_ffitype  = */ &ffi_type_uint8,
#endif /* !SIGNED */
#elif SIZEOF == 2
#ifdef SIGNED
	/* .st_ffitype  = */ &ffi_type_sint16,
#else /* SIGNED */
	/* .st_ffitype  = */ &ffi_type_uint16,
#endif /* !SIGNED */
#elif SIZEOF == 4
#ifdef SIGNED
	/* .st_ffitype  = */ &ffi_type_sint32,
#else /* SIGNED */
	/* .st_ffitype  = */ &ffi_type_uint32,
#endif /* !SIGNED */
#elif SIZEOF == 8
#ifdef SIGNED
	/* .st_ffitype  = */ &ffi_type_sint64,
#else /* SIGNED */
	/* .st_ffitype  = */ &ffi_type_uint64,
#endif /* !SIGNED */
#else
#error "Unsupported integral type for use with FFY (Try builtin with `-DCONFIG_NO_CFUNCTION')"
#endif
#endif /* !CONFIG_NO_CFUNCTION */
	/* .st_sizeof   = */ SIZEOF,
#ifdef ALIGNOF
	/* .st_align    = */ ALIGNOF,
#else /* ALIGNOF */
	/* .st_align    = */ SIZEOF,
#endif /* !ALIGNOF */
	/* .st_init     = */ (int (DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&F(intinit),
	/* .st_assign   = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(intass),
	/* .st_cast     = */ {
		/* .st_str  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(intstr),
		/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&F(intstr),
		/* .st_bool = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&F_NOSIGN(intbool)
	},
	/* .st_call     = */ NULL,
	/* .st_math     = */ &F(intmath),
	/* .st_cmp      = */ &F(intcmp),
	/* .st_seq      = */ NULL,
	/* .st_attr     = */ NULL
};

#undef GET
#undef SET

#undef FFI_TYPE
#undef F_NOSIGN
#undef NAME
#undef FORMAT_TYP
#undef FORMAT_STR
#undef NO_SIGNED_TYPE_NAME
#undef NO_UNSIGNED_TYPE_NAME
#undef SIGNED_TYPE_NAME
#undef UNSIGNED_TYPE_NAME
#undef TYPE_NAME
#undef SIGNED
#undef SIZEOF
#undef ALIGNOF
#undef T
#undef X
#undef F

DECL_END
