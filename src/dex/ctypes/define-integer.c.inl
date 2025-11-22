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
#ifdef __INTELLISENSE__
#include "builtins.c"
#define SIZEOF 16
#define SIGNED 1
#endif /* __INTELLISENSE__ */

#ifndef SIZEOF
#error "Must #define SIZEOF before including this file"
#endif /* !SIZEOF */

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/util/lock.h>
#include <deemon/variant.h>

#include <hybrid/int128.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t */

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
#define INT8_MIN   (-INT8_C(127)-INT8_C(1))
#define INT8_MAX     INT8_C(127)
#define UINT8_MAX    UINT8_C(0xff)
#define INT16_MIN  (-INT16_C(32767)-INT16_C(1))
#define INT16_MAX    INT16_C(32767)
#define UINT16_MAX   UINT16_C(0xffff)
#define INT32_MIN  (-INT32_C(2147483647)-INT32_C(1))
#define INT32_MAX    INT32_C(2147483647)
#define UINT32_MAX   UINT32_C(0xffffffff)
#define INT64_MIN  (-INT64_C(9223372036854775807)-INT64_C(1))
#define INT64_MAX    INT64_C(9223372036854775807)
#define UINT64_MAX   UINT64_C(0xffffffffffffffff)
#endif /* !INT8_MIN */



#ifndef F
#ifdef SIGNED
#if SIZEOF == 1
#define F(x) x##s8
#elif SIZEOF == 2
#define F(x) x##s16
#elif SIZEOF == 4
#define F(x) x##s32
#elif SIZEOF == 8
#define F(x) x##s64
#elif SIZEOF == 16
#define F(x) x##s128
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
#elif SIZEOF == 16
#define F(x) x##u128
#else /* SIZEOF == ... */
#define F(x) X2(x, TYPE_NAME)
#endif /* SIZEOF != ... */
#endif /* !SIGNED */
#endif /* !F */

#ifndef SIGNED_TYPE_NAME
#if SIZEOF == 1
#define SIGNED_TYPE_NAME DeeCInt8_Type
#elif SIZEOF == 2
#define SIGNED_TYPE_NAME DeeCInt16_Type
#elif SIZEOF == 4
#define SIGNED_TYPE_NAME DeeCInt32_Type
#elif SIZEOF == 8
#define SIGNED_TYPE_NAME DeeCInt64_Type
#elif SIZEOF == 16
#define SIGNED_TYPE_NAME DeeCInt128_Type
#else /* SIZEOF == ... */
#error "Must #define SIGNED_TYPE_NAME"
#endif /* SIZEOF != ... */
#endif /* !SIGNED_TYPE_NAME */

#ifndef UNSIGNED_TYPE_NAME
#if SIZEOF == 1
#define UNSIGNED_TYPE_NAME DeeCUInt8_Type
#elif SIZEOF == 2
#define UNSIGNED_TYPE_NAME DeeCUInt16_Type
#elif SIZEOF == 4
#define UNSIGNED_TYPE_NAME DeeCUInt32_Type
#elif SIZEOF == 8
#define UNSIGNED_TYPE_NAME DeeCUInt64_Type
#elif SIZEOF == 16
#define UNSIGNED_TYPE_NAME DeeCUInt128_Type
#else /* SIZEOF == ... */
#error "Must #define UNSIGNED_TYPE_NAME"
#endif /* SIZEOF != ... */
#endif /* !UNSIGNED_TYPE_NAME */

#ifndef TYPE_NAME
#ifdef SIGNED
#define TYPE_NAME SIGNED_TYPE_NAME
#else /* SIGNED */
#define TYPE_NAME UNSIGNED_TYPE_NAME
#endif /* !SIGNED */
#ifndef TYPE_NAME
#error "Must #define TYPE_NAME for custom-width integer"
#endif /* !TYPE_NAME */
#endif /* !TYPE_NAME */

#ifndef FORMAT_STR
#ifdef SIGNED
#if SIZEOF <= 1
#define FORMAT_STR "%" PRFd8
#define FORMAT_TYP int8_t
#elif SIZEOF <= 2
#define FORMAT_STR "%" PRFd16
#define FORMAT_TYP int16_t
#elif SIZEOF <= 4
#define FORMAT_STR "%" PRFd32
#define FORMAT_TYP int32_t
#elif SIZEOF <= 8
#define FORMAT_STR "%" PRFd64
#define FORMAT_TYP int64_t
#elif SIZEOF <= 16
#define FORMAT_STR "%" PRFd128
#define FORMAT_TYP Dee_int128_t
#endif /* SIZEOF <=´... */
#else /* SIGNED */
#if SIZEOF <= 1
#define FORMAT_STR "%" PRFd8
#define FORMAT_TYP uint8_t
#elif SIZEOF <= 2
#define FORMAT_STR "%" PRFd16
#define FORMAT_TYP uint16_t
#elif SIZEOF <= 4
#define FORMAT_STR "%" PRFd32
#define FORMAT_TYP uint32_t
#elif SIZEOF <= 8
#define FORMAT_STR "%" PRFd64
#define FORMAT_TYP uint64_t
#elif SIZEOF <= 16
#define FORMAT_STR "%" PRFd128
#define FORMAT_TYP Dee_uint128_t
#endif /* SIZEOF <= ... */
#endif /* !SIGNED */
#ifndef FORMAT_STR
#error "Must #define FORMAT_STR for extended-width integer"
#endif /* !FORMAT_STR */
#endif /* !FORMAT_STR */

#ifndef T
#ifdef SIGNED
#if SIZEOF == 1
#define T int8_t
#elif SIZEOF == 2
#define T int16_t
#elif SIZEOF == 4
#define T int32_t
#elif SIZEOF == 8
#define T int64_t
#elif SIZEOF == 16
#define T Dee_int128_t
#endif /* SIZEOF == ... */
#else /* SIGNED */
#if SIZEOF == 1
#define T uint8_t
#elif SIZEOF == 2
#define T uint16_t
#elif SIZEOF == 4
#define T uint32_t
#elif SIZEOF == 8
#define T uint64_t
#elif SIZEOF == 16
#define T Dee_uint128_t
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
#elif SIZEOF == 16
#define F_NOSIGN(x) x##ns128
#else /* SIZEOF == ... */
#define F_NOSIGN(x) X(x##ns)
#endif /* SIZEOF != ... */

#ifndef GET
#ifdef SIGNED
#if SIZEOF == 1
#define GET(ptr)    ((int8_t)UNALIGNED_GET8(ptr))
#define SET(ptr, v) UNALIGNED_SET8(ptr, (uint8_t)(v))
#elif SIZEOF == 2
#define GET(ptr)    ((int16_t)UNALIGNED_GET16(ptr))
#define SET(ptr, v) UNALIGNED_SET16(ptr, (uint16_t)(v))
#elif SIZEOF == 4
#define GET(ptr)    ((int32_t)UNALIGNED_GET32(ptr))
#define SET(ptr, v) UNALIGNED_SET32(ptr, (uint32_t)(v))
#elif SIZEOF == 8
#define GET(ptr)    ((int64_t)UNALIGNED_GET64(ptr))
#define SET(ptr, v) UNALIGNED_SET64(ptr, (uint64_t)(v))
#endif /* SIZEOF == ... */
#else /* SIGNED */
#if SIZEOF == 1
#define GET(ptr)    UNALIGNED_GET8(ptr)
#define SET(ptr, v) UNALIGNED_SET8(ptr, v)
#elif SIZEOF == 2
#define GET(ptr)    UNALIGNED_GET16(ptr)
#define SET(ptr, v) UNALIGNED_SET16(ptr, v)
#elif SIZEOF == 4
#define GET(ptr)    UNALIGNED_GET32(ptr)
#define SET(ptr, v) UNALIGNED_SET32(ptr, v)
#elif SIZEOF == 8
#define GET(ptr)    UNALIGNED_GET64(ptr)
#define SET(ptr, v) UNALIGNED_SET64(ptr, v)
#endif /* SIZEOF == ... */
#endif /* !SIGNED */
#endif /* !GET */

#ifndef GET
#if SIZEOF == 16
#ifdef UNALIGNED_GET128
#ifdef SIGNED
#define GET(ptr)    ((__INT128_TYPE__)UNALIGNED_GET128(ptr))
#define SET(ptr, v) UNALIGNED_SET128(ptr, (__UINT128_TYPE__)(v))
#else /* SIGNED */
#define GET(ptr)    ((__INT128_TYPE__)UNALIGNED_GET128(ptr))
#define SET(ptr, v) UNALIGNED_SET128(ptr, (__UINT128_TYPE__)(v))
#endif /* !SIGNED */
#else /* UNALIGNED_GET128 */
#ifdef SIGNED
#define GET(ptr)    Dee_UNALIGNED_GETS128(ptr)
#define SET(ptr, v) Dee_UNALIGNED_SETS128(ptr, v)
#else /* SIGNED */
#define GET(ptr)    Dee_UNALIGNED_GETU128(ptr)
#define SET(ptr, v) Dee_UNALIGNED_SETU128(ptr, v)
#endif /* !SIGNED */
#ifndef DEE_UNALIGNED128_DEFINED
#define DEE_UNALIGNED128_DEFINED
PRIVATE Dee_uint128_t Dee_UNALIGNED_GETU128(void const *p) {
	Dee_uint128_t result;
	__hybrid_uint128_setword64(result, 0, UNALIGNED_GET64((__BYTE_TYPE__ const *)p + 0));
	__hybrid_uint128_setword64(result, 1, UNALIGNED_GET64((__BYTE_TYPE__ const *)p + 8));
	return result;
}
PRIVATE Dee_int128_t Dee_UNALIGNED_GETS128(void const *p) {
	Dee_int128_t result;
	__hybrid_int128_setword64(result, 0, (int64_t)UNALIGNED_GET64((__BYTE_TYPE__ const *)p + 0));
	__hybrid_int128_setword64(result, 1, (int64_t)UNALIGNED_GET64((__BYTE_TYPE__ const *)p + 8));
	return result;
}
PRIVATE void Dee_UNALIGNED_SETU128(void *p, Dee_uint128_t val) {
	(void)memcpy(p, &val, sizeof(Dee_uint128_t));
}
PRIVATE void Dee_UNALIGNED_SETS128(void *p, Dee_int128_t val) {
	(void)memcpy(p, &val, sizeof(Dee_int128_t));
}
#endif /* !DEE_UNALIGNED128_DEFINED */
#endif /* !UNALIGNED_GET128 */
#else /* SIZEOF == 16 */
#error "Unsupported SIZEOF"
#endif /* SIZEOF != 16 */
#endif /* !GET */



typedef struct {
	OBJECT_HEAD
	T i_value; /* The integer value. */
} X(Integer);


#if (((SIZEOF != 1 || (defined(SIGNED) ? !defined(INT8_FUNCTIONS_DEFINED) : !defined(UINT8_FUNCTIONS_DEFINED))) &&       \
      (SIZEOF != 2 || (defined(SIGNED) ? !defined(INT16_FUNCTIONS_DEFINED) : !defined(UINT16_FUNCTIONS_DEFINED))) &&     \
      (SIZEOF != 4 || (defined(SIGNED) ? !defined(INT32_FUNCTIONS_DEFINED) : !defined(UINT32_FUNCTIONS_DEFINED))) &&     \
      (SIZEOF != 8 || (defined(SIGNED) ? !defined(INT64_FUNCTIONS_DEFINED) : !defined(UINT64_FUNCTIONS_DEFINED))) &&     \
      (SIZEOF != 16 || (defined(SIGNED) ? !defined(INT128_FUNCTIONS_DEFINED) : !defined(UINT128_FUNCTIONS_DEFINED)))) || \
     defined(CONFIG_DONT_PROMOTE_TO_INTEGER))
#ifndef CONFIG_DONT_PROMOTE_TO_INTEGER
#ifdef SIGNED
#if SIZEOF == 1
#define INT8_FUNCTIONS_DEFINED  1
#elif SIZEOF == 2
#define INT16_FUNCTIONS_DEFINED 1
#elif SIZEOF == 4
#define INT32_FUNCTIONS_DEFINED 1
#elif SIZEOF == 8
#define INT64_FUNCTIONS_DEFINED 1
#elif SIZEOF == 128
#define INT128_FUNCTIONS_DEFINED 1
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
#elif SIZEOF == 16
#define UINT128_FUNCTIONS_DEFINED 1
#endif /* SIZEOF == ... */
#endif /* !SIGNED */
#endif /* !CONFIG_DONT_PROMOTE_TO_INTEGER */

#if SIZEOF == 1
#ifndef INT8_SIGNLESS_FUNCTIONS_DEFINED
#define INT8_SIGNLESS_FUNCTIONS_DEFINED  1
#else /* !INT8_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED 1
#endif /* INT8_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 2
#ifndef INT16_SIGNLESS_FUNCTIONS_DEFINED
#define INT16_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT16_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED 1
#endif /* INT16_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 4
#ifndef INT32_SIGNLESS_FUNCTIONS_DEFINED
#define INT32_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT32_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED 1
#endif /* INT32_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 8
#ifndef INT64_SIGNLESS_FUNCTIONS_DEFINED
#define INT64_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT64_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED 1
#endif /* INT64_SIGNLESS_FUNCTIONS_DEFINED */
#elif SIZEOF == 16
#ifndef INT128_SIGNLESS_FUNCTIONS_DEFINED
#define INT128_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT128_SIGNLESS_FUNCTIONS_DEFINED */
#define SIGNLESS_DEFINED 1
#endif /* INT128_SIGNLESS_FUNCTIONS_DEFINED */
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
	DeeObject_Init(result, DeeSType_AsType(&DeeCInt_Type));
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
	DeeObject_Init(result, DeeSType_AsType(&TYPE_NAME));
	result->i_value = val;
done:
	return (DREF DeeObject *)result;
}
#endif /* !SIGNED || SIZEOF != CONFIG_CTYPES_SIZEOF_INT */

#ifdef CONFIG_DONT_PROMOTE_TO_INTEGER
#define NEW_INTEGER(val) F(int_new_)(val)
PRIVATE WUNUSED DREF DeeObject *DCALL F(int_new_)(T val) {
	X(Integer) * result;
	result = DeeObject_MALLOC(X(Integer));
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeSType_AsType(&TYPE_NAME));
	result->i_value = val;
done:
	return (DREF DeeObject *)result;
}
#elif ((SIZEOF < CONFIG_CTYPES_SIZEOF_INT) || \
       (defined(SIGNED) && SIZEOF == CONFIG_CTYPES_SIZEOF_INT))
/* Integer promotion. */
#define NEW_INTEGER(val) int_newint((CTYPES_INT)(val))
#else /* ... */
#define NEW_INTEGER(val) F(int_new)(val)
#endif /* !... */

#ifdef SIGNED
#define HYBRID128_(x) __hybrid_int128_##x
#else /* SIGNED */
#define HYBRID128_(x) __hybrid_uint128_##x
#endif /* !SIGNED */


/* Define functions. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
F(intinit)(DeeSTypeObject *__restrict UNUSED(tp_self),
           T *self, size_t argc, DeeObject *const *argv) {
	T value;
	DeeObject *arg;
#ifdef NAME
	DeeArg_Unpack1(err, argc, argv, NAME, &arg);
#else /* NAME */
	DeeArg_Unpack1(err, argc, argv, PP_STR(T), &arg);
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
#if SIZEOF == 16
	return !HYBRID128_(iszero)(value);
#else /* SIZEOF == 16 */
	return value != 0;
#endif /* SIZEOF != 16 */
}
#endif /* !SIGNLESS_DEFINED */

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_int32)(DeeSTypeObject *__restrict UNUSED(tp_self),
             T *self, int32_t *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
#ifdef SIGNED
#if SIZEOF == 16
	*result = __hybrid_int128_get32(value);
	return INT_SIGNED;
#else /* SIZEOF == 16 */
	*result = (int32_t)value;
	return INT_SIGNED;
#endif /* SIZEOF != 16 */
#else /* SIGNED */
#if SIZEOF == 16
	*result = __hybrid_uint128_get32(value);
	return INT_UNSIGNED;
#else /* SIZEOF == 16 */
	*(uint32_t *)result = (uint32_t)value;
	return INT_UNSIGNED;
#endif /* SIZEOF != 16 */
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_int64)(DeeSTypeObject *__restrict UNUSED(tp_self),
             T *self, int64_t *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
#ifdef SIGNED
#if SIZEOF == 16
	*result = __hybrid_int128_get64(value);
	return INT_SIGNED;
#else /* SIZEOF == 16 */
	*result = (int64_t)value;
	return INT_SIGNED;
#endif /* SIZEOF != 16 */
#else /* SIGNED */
#if SIZEOF == 16
	*result = __hybrid_uint128_get64(value);
	return INT_UNSIGNED;
#else /* SIZEOF == 16 */
	*(uint64_t *)result = (uint64_t)value;
	return INT_UNSIGNED;
#endif /* SIZEOF != 16 */
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_double)(DeeSTypeObject *__restrict UNUSED(tp_self),
              T *self, double *__restrict result) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return -1);
#if SIZEOF == 16
	*result = (double)__hybrid_int128_get64(value);
#else /* SIZEOF == 16 */
	*result = (double)value;
#endif /* SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_int)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#ifdef SIGNED
	return _DeeInt_NewS(SIZEOF, value);
#else /* SIGNED */
	return _DeeInt_NewU(SIZEOF, value);
#endif /* !SIGNED */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_inv)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#if SIZEOF == 16
	HYBRID128_(inv)(value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(~value);
#endif /* SIZEOF != 16 */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_pos)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#if SIZEOF == 16
	/*HYBRID128_(pos)(value);*/
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(+value);
#endif /* SIZEOF != 16 */
}

#ifdef SIGNED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(int_neg)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
	T value;
	CTYPES_FAULTPROTECT(value = GET(self), return NULL);
#if SIZEOF == 16
	HYBRID128_(neg)(value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(-value);
#endif /* SIZEOF != 16 */
}
#endif /* SIGNED */

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	HYBRID128_(add128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value + other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	HYBRID128_(sub128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value - other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	HYBRID128_(mul128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value * other_value);
#endif /* SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
F(int_divzero)(T value, DeeObject *__restrict some_object) {
	int result;
	struct Dee_variant lhs, rhs;
#if SIZEOF >= 16 && defined(SIGNED)
	Dee_variant_init_int128(&lhs, value);
#elif SIZEOF >= 16
	Dee_variant_init_uint128(&lhs, value);
#elif SIZEOF >= 8 && defined(SIGNED)
	Dee_variant_init_int64(&lhs, value);
#elif SIZEOF >= 8
	Dee_variant_init_uint64(&lhs, value);
#elif defined(SIGNED)
	Dee_variant_init_int32(&lhs, value);
#else /* ... */
	Dee_variant_init_uint32(&lhs, value);
#endif /* ... */
	Dee_variant_init_object(&rhs, some_object);
	result = DeeRT_ErrDivideByZeroEx(&lhs, &rhs);
	Dee_variant_fini(&rhs);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_div)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value, other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	if unlikely(HYBRID128_(iszero)(other_value)) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	HYBRID128_(div128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	if unlikely(!other_value) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	return NEW_INTEGER(value / other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	if unlikely(HYBRID128_(iszero)(other_value)) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	HYBRID128_(mod128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	if unlikely(!other_value) {
		(F(int_divzero)(value, some_object));
		goto err;
	}
	return NEW_INTEGER(value % other_value);
#endif /* SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value;
	__SHIFT_TYPE__ other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	HYBRID128_(shl)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value << other_value);
#endif /* SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(int_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
           DeeObject *__restrict some_object) {
	T value;
	__SHIFT_TYPE__ other_value;
	CTYPES_FAULTPROTECT(value = GET(self), goto err);
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	HYBRID128_(shr)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value >> other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	HYBRID128_(and128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value & other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	HYBRID128_(or128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value | other_value);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	HYBRID128_(xor128)(value, other_value);
	return NEW_INTEGER(value);
#else /* SIZEOF == 16 */
	return NEW_INTEGER(value ^ other_value);
#endif /* SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(int_inc)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(inc)(temp);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) + 1)), return -1);
#endif /* SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(int_dec)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self) {
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(dec)(temp);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) - 1)), return -1);
#endif /* SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_add)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(add128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) + other_value)), goto err);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(sub128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) - other_value)), goto err);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(mul128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) * other_value)), goto err);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	if unlikely(HYBRID128_(iszero)(other_value))
#else /* SIZEOF == 16 */
	if unlikely(!other_value)
#endif /* SIZEOF != 16 */
	{
		T value;
		CTYPES_FAULTPROTECT(value = GET(self), goto err);
		(F(int_divzero)(value, some_object));
		goto err;
	}
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(div128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) / other_value)), goto err);
#endif /* SIZEOF != 16 */
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
#if SIZEOF == 16
	if unlikely(HYBRID128_(iszero)(other_value))
#else /* SIZEOF == 16 */
	if unlikely(!other_value)
#endif /* SIZEOF != 16 */
	{
		T value;
		CTYPES_FAULTPROTECT(value = GET(self), goto err);
		(F(int_divzero)(value, some_object));
		goto err;
	}
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(mod128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) % other_value)), goto err);
#endif /* SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	__SHIFT_TYPE__ other_value;
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(shl)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) << other_value)), goto err);
#endif /* SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	__SHIFT_TYPE__ other_value;
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(shr)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) >> other_value)), goto err);
#endif /* SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_and)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(and128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) & other_value)), goto err);
#endif /* SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_or)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                  DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(or128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) | other_value)), goto err);
#endif /* SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_xor)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	T other_value;
	if (OBJECT_AS_T(some_object, &other_value))
		goto err;
#if SIZEOF == 16
	CTYPES_FAULTPROTECT({
		T temp = GET(self);
		HYBRID128_(xor128)(temp, other_value);
		SET(self, temp);
	}, return -1);
#else /* SIZEOF == 16 */
	CTYPES_FAULTPROTECT(SET(self, (T)(GET(self) ^ other_value)), goto err);
#endif /* SIZEOF != 16 */
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
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(int_inplace_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), T *self,
                   DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
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

#if SIZEOF == 16
#define CTYPES_INT_DEFINE_COMPARE_OPERATOR(name, op)            \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL       \
	F(int_##name)(DeeSTypeObject *UNUSED(tp_self),              \
	              T *self, DeeObject *some_object) {            \
		T value, other_value;                                   \
		CTYPES_FAULTPROTECT(value = GET(self), goto err);       \
		if (OBJECT_AS_T(some_object, &other_value))             \
			goto err;                                           \
		return_bool(HYBRID128_(name##128)(value, other_value)); \
	err:                                                        \
		return NULL;                                            \
	}
#else /* SIZEOF == 16 */
#define CTYPES_INT_DEFINE_COMPARE_OPERATOR(name, op)      \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL \
	F(int_##name)(DeeSTypeObject *UNUSED(tp_self),        \
	              T *self, DeeObject *some_object) {      \
		T value, other_value;                             \
		CTYPES_FAULTPROTECT(value = GET(self), goto err); \
		if (OBJECT_AS_T(some_object, &other_value))       \
			goto err;                                     \
		return_bool(value op other_value);                \
	err:                                                  \
		return NULL;                                      \
	}
#endif /* SIZEOF != 16 */
CTYPES_INT_DEFINE_COMPARE_OPERATOR(eq, ==)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(ne, !=)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(lo, <)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(le, <=)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(gr, >)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(ge, >=)
#undef CTYPES_INT_DEFINE_COMPARE_OPERATOR

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
	OBJECT_HEAD_INIT(DeeSType_AsType(&TYPE_NAME)),
#ifdef SIGNED
#if SIZEOF == 1
	INT8_MIN
#elif SIZEOF == 2
	INT16_MIN
#elif SIZEOF == 4
	INT32_MIN
#elif SIZEOF == 8
	INT64_MIN
#elif SIZEOF == 16
	__HYBRID_INT128_INIT32N(UINT32_C(0x80000000), 0, 0, 0)
#else /* SIZEOF == ... */
#error "Invalid `SIZEOF'"
#endif /* SIZEOF != ... */
#else /* SIGNED */
#if SIZEOF == 16
	__HYBRID_UINT128_INIT32N(0, 0, 0, 0)
#else /* SIZEOF == ... */
	0
#endif /* SIZEOF != ... */
#endif /* !SIGNED */
};

PRIVATE X(Integer) X(int_max) = {
	OBJECT_HEAD_INIT(DeeSType_AsType(&TYPE_NAME)),
#ifdef SIGNED
#if SIZEOF == 1
	INT8_MAX
#elif SIZEOF == 2
	INT16_MAX
#elif SIZEOF == 4
	INT32_MAX
#elif SIZEOF == 8
	INT64_MAX
#elif SIZEOF == 16
	__HYBRID_INT128_INIT32N(UINT32_C(0x7fffffff), UINT32_C(0xffffffff),
	                        UINT32_C(0xffffffff), UINT32_C(0xffffffff))
#else /* SIZEOF == ... */
#error "Invalid `SIZEOF'"
#endif /* SIZEOF != ... */
#else /* SIGNED */
#if SIZEOF == 1
	UINT8_MAX
#elif SIZEOF == 2
	UINT16_MAX
#elif SIZEOF == 4
	UINT32_MAX
#elif SIZEOF == 8
	UINT64_MAX
#elif SIZEOF == 16
	__HYBRID_UINT128_INIT32N(UINT32_C(0xffffffff), UINT32_C(0xffffffff),
	                         UINT32_C(0xffffffff), UINT32_C(0xffffffff))
#else /* SIZEOF == ... */
#error "Invalid `SIZEOF'"
#endif /* SIZEOF != ... */
#endif /* !SIGNED */
};

PRIVATE struct type_member tpconst X(int_class_members)[] = {
#if !defined(NO_SIGNED_TYPE_NAME) && !defined(NO_UNSIGNED_TYPE_NAME)
	TYPE_MEMBER_CONST("signed", DeeSType_AsType(&SIGNED_TYPE_NAME)),
	TYPE_MEMBER_CONST("unsigned", DeeSType_AsType(&UNSIGNED_TYPE_NAME)),
#endif /* !NO_SIGNED_TYPE_NAME && !NO_UNSIGNED_TYPE_NAME */
	TYPE_MEMBER_CONST("min", &X(int_min)),
	TYPE_MEMBER_CONST("max", &X(int_max)),
	TYPE_MEMBER_END
};


INTERN DeeSTypeObject TYPE_NAME = {
	/* .st_base = */ {
		OBJECT_HEAD_INIT(&DeeSType_Type),
#ifdef NAME
		/* .tp_name     = */ NAME,
#else /* NAME */
		/* .tp_name     = */ PP_STR(T),
#endif /* !NAME */
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (Dee_funptr_t)NULL,
					/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
					/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
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
		/* .tp_class_members = */ X(int_class_members),
		/* .tp_method_hints  = */ NULL,
		/* .tp_call          = */ NULL,
		/* .tp_callable      = */ NULL,
		/* .tp_mro           = */ ctypes_numeric_mro,
	},
#ifndef CONFIG_NO_THREADS
	/* .st_cachelock = */ Dee_ATOMIC_RWLOCK_INIT,
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
#elif SIZEOF == 16
#ifdef SIGNED
	/* .st_ffitype  = */ NULL, /* TODO */
#else /* SIGNED */
	/* .st_ffitype  = */ NULL, /* TODO */
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

#undef HYBRID128_

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
