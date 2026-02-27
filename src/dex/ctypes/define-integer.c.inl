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
#include "builtins.c"
#define DEFINE_SIZEOF 16
#define DEFINE_SIGNED 1
#endif /* __INTELLISENSE__ */

#ifndef DEFINE_SIZEOF
#error "Must #define DEFINE_SIZEOF before including this file"
#endif /* !DEFINE_SIZEOF */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DeeArg_Unpack1 */
#include <deemon/bool.h>            /* return_bool */
#include <deemon/error-rt.h>        /* DeeRT_ErrDivideByZeroEx */
#include <deemon/error.h>           /* DeeError_NOTIMPLEMENTED */
#include <deemon/format.h>          /* PRF* */
#include <deemon/int.h>             /* INT_SIGNED, INT_UNSIGNED, _DeeInt_NewS, _DeeInt_NewU */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, Dee_AsObject, Dee_int128_t, Dee_uint128_t, OBJECT_HEAD, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/string.h>          /* DeeString_Newf */
#include <deemon/system-features.h> /* memcpy */
#include <deemon/type.h>            /* DeeObject_Init, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, INT_SIGNED, INT_UNSIGNED, TF_NONE, TP_F*, TYPE_MEMBER_CONST, TYPE_MEMBER_END, type_member */
#include <deemon/util/lock.h>       /* Dee_ATOMIC_RWLOCK_INIT */
#include <deemon/variant.h>         /* Dee_variant, Dee_variant_* */

#include <hybrid/int128.h>    /* __HYBRID_INT128_INIT32N, __HYBRID_UINT128_INIT32N, __hybrid_int128_*, __hybrid_uint128_* */
#include <hybrid/typecore.h>  /* __*_TYPE__, __SIZEOF_INT__, __SIZEOF_LONG__ */
#include <hybrid/unaligned.h> /* UNALIGNED_GET*, UNALIGNED_SET* */
#include <hybrid/limitcore.h> /* UNALIGNED_GET*, UNALIGNED_SET* */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* intN_t, uintN_t */

#if !defined(INT8_C) && defined(__INT8_C)
#define INT8_C __INT8_C
#endif /* !INT8_C && __INT8_C */
#if !defined(INT16_C) && defined(__INT16_C)
#define INT16_C __INT16_C
#endif /* !INT16_C && __INT16_C */
#if !defined(INT32_C) && defined(__INT32_C)
#define INT32_C __INT32_C
#endif /* !INT32_C && __INT32_C */
#if !defined(INT64_C) && defined(__INT64_C)
#define INT64_C __INT64_C
#endif /* !INT64_C && __INT64_C */
#if !defined(UINT8_C) && defined(__UINT8_C)
#define UINT8_C __UINT8_C
#endif /* !UINT8_C && __UINT8_C */
#if !defined(UINT16_C) && defined(__UINT16_C)
#define UINT16_C __UINT16_C
#endif /* !UINT16_C && __UINT16_C */
#if !defined(UINT32_C) && defined(__UINT32_C)
#define UINT32_C __UINT32_C
#endif /* !UINT32_C && __UINT32_C */
#if !defined(UINT64_C) && defined(__UINT64_C)
#define UINT64_C __UINT64_C
#endif /* !UINT64_C && __UINT64_C */

#if !defined(INT8_MIN) && defined(__INT8_MIN__)
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN && __INT8_MIN__ */
#if !defined(INT8_MAX) && defined(__INT8_MAX__)
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX && __INT8_MAX__ */
#if !defined(UINT8_MAX) && defined(__UINT8_MAX__)
#define UINT8_MAX __UINT8_MAX__
#endif /* !UINT8_MAX && __UINT8_MAX__ */
#if !defined(INT16_MIN) && defined(__INT16_MIN__)
#define INT16_MIN __INT16_MIN__
#endif /* !INT16_MIN && __INT16_MIN__ */
#if !defined(INT16_MAX) && defined(__INT16_MAX__)
#define INT16_MAX __INT16_MAX__
#endif /* !INT16_MAX && __INT16_MAX__ */
#if !defined(UINT16_MAX) && defined(__UINT16_MAX__)
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX && __UINT16_MAX__ */
#if !defined(INT32_MIN) && defined(__INT32_MIN__)
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN && __INT32_MIN__ */
#if !defined(INT32_MAX) && defined(__INT32_MAX__)
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX && __INT32_MAX__ */
#if !defined(UINT32_MAX) && defined(__UINT32_MAX__)
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX && __UINT32_MAX__ */
#if !defined(INT64_MIN) && defined(__INT64_MIN__)
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN && __INT64_MIN__ */
#if !defined(INT64_MAX) && defined(__INT64_MAX__)
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX && __INT64_MAX__ */
#if !defined(UINT64_MAX) && defined(__UINT64_MAX__)
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX && __UINT64_MAX__ */

#ifndef DEFINE_F
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
#define DEFINE_F(x) x##s8
#elif DEFINE_SIZEOF == 2
#define DEFINE_F(x) x##s16
#elif DEFINE_SIZEOF == 4
#define DEFINE_F(x) x##s32
#elif DEFINE_SIZEOF == 8
#define DEFINE_F(x) x##s64
#elif DEFINE_SIZEOF == 16
#define DEFINE_F(x) x##s128
#else /* DEFINE_SIZEOF == ... */
#define DEFINE_F(x) X2(x, DEFINE_TYPE_NAME)
#endif /* DEFINE_SIZEOF != ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 1
#define DEFINE_F(x) x##u8
#elif DEFINE_SIZEOF == 2
#define DEFINE_F(x) x##u16
#elif DEFINE_SIZEOF == 4
#define DEFINE_F(x) x##u32
#elif DEFINE_SIZEOF == 8
#define DEFINE_F(x) x##u64
#elif DEFINE_SIZEOF == 16
#define DEFINE_F(x) x##u128
#else /* DEFINE_SIZEOF == ... */
#define DEFINE_F(x) X2(x, DEFINE_TYPE_NAME)
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_SIGNED */
#endif /* !DEFINE_F */

#ifndef DEFINE_ALIGNOF
#define DEFINE_ALIGNOF DEFINE_SIZEOF
#endif /* !DEFINE_ALIGNOF */

#ifndef DEFINE_TYPE_NAME_SIGNED
#if DEFINE_SIZEOF == 1
#define DEFINE_TYPE_NAME_SIGNED DeeCInt8_Type
#elif DEFINE_SIZEOF == 2
#define DEFINE_TYPE_NAME_SIGNED DeeCInt16_Type
#elif DEFINE_SIZEOF == 4
#define DEFINE_TYPE_NAME_SIGNED DeeCInt32_Type
#elif DEFINE_SIZEOF == 8
#define DEFINE_TYPE_NAME_SIGNED DeeCInt64_Type
#elif DEFINE_SIZEOF == 16
#define DEFINE_TYPE_NAME_SIGNED DeeCInt128_Type
#else /* DEFINE_SIZEOF == ... */
#error "Must #define DEFINE_TYPE_NAME_SIGNED"
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_TYPE_NAME_SIGNED */

#ifndef DEFINE_TYPE_NAME_UNSIGNED
#if DEFINE_SIZEOF == 1
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUInt8_Type
#elif DEFINE_SIZEOF == 2
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUInt16_Type
#elif DEFINE_SIZEOF == 4
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUInt32_Type
#elif DEFINE_SIZEOF == 8
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUInt64_Type
#elif DEFINE_SIZEOF == 16
#define DEFINE_TYPE_NAME_UNSIGNED DeeCUInt128_Type
#else /* DEFINE_SIZEOF == ... */
#error "Must #define DEFINE_TYPE_NAME_UNSIGNED"
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_TYPE_NAME_UNSIGNED */

#ifndef DEFINE_TYPE_NAME
#ifdef DEFINE_SIGNED
#define DEFINE_TYPE_NAME DEFINE_TYPE_NAME_SIGNED
#else /* DEFINE_SIGNED */
#define DEFINE_TYPE_NAME DEFINE_TYPE_NAME_UNSIGNED
#endif /* !DEFINE_SIGNED */
#ifndef DEFINE_TYPE_NAME
#error "Must #define DEFINE_TYPE_NAME for custom-width integer"
#endif /* !DEFINE_TYPE_NAME */
#endif /* !DEFINE_TYPE_NAME */

#ifndef DEFINE_PRINTF_FMT
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF <= 1
#define DEFINE_PRINTF_FMT "%" PRFd8
#define DEFINE_PRINTF_TYP int8_t
#elif DEFINE_SIZEOF <= 2
#define DEFINE_PRINTF_FMT "%" PRFd16
#define DEFINE_PRINTF_TYP int16_t
#elif DEFINE_SIZEOF <= 4
#define DEFINE_PRINTF_FMT "%" PRFd32
#define DEFINE_PRINTF_TYP int32_t
#elif DEFINE_SIZEOF <= 8
#define DEFINE_PRINTF_FMT "%" PRFd64
#define DEFINE_PRINTF_TYP int64_t
#elif DEFINE_SIZEOF <= 16
#define DEFINE_PRINTF_FMT "%" PRFd128
#define DEFINE_PRINTF_TYP Dee_int128_t
#endif /* DEFINE_SIZEOF <=´... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF <= 1
#define DEFINE_PRINTF_FMT "%" PRFd8
#define DEFINE_PRINTF_TYP uint8_t
#elif DEFINE_SIZEOF <= 2
#define DEFINE_PRINTF_FMT "%" PRFd16
#define DEFINE_PRINTF_TYP uint16_t
#elif DEFINE_SIZEOF <= 4
#define DEFINE_PRINTF_FMT "%" PRFd32
#define DEFINE_PRINTF_TYP uint32_t
#elif DEFINE_SIZEOF <= 8
#define DEFINE_PRINTF_FMT "%" PRFd64
#define DEFINE_PRINTF_TYP uint64_t
#elif DEFINE_SIZEOF <= 16
#define DEFINE_PRINTF_FMT "%" PRFd128
#define DEFINE_PRINTF_TYP Dee_uint128_t
#endif /* DEFINE_SIZEOF <= ... */
#endif /* !DEFINE_SIGNED */
#ifndef DEFINE_PRINTF_FMT
#error "Must #define DEFINE_PRINTF_FMT for extended-width integer"
#endif /* !DEFINE_PRINTF_FMT */
#endif /* !DEFINE_PRINTF_FMT */

#ifndef DEFINE_T
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
#define DEFINE_T int8_t
#elif DEFINE_SIZEOF == 2
#define DEFINE_T int16_t
#elif DEFINE_SIZEOF == 4
#define DEFINE_T int32_t
#elif DEFINE_SIZEOF == 8
#define DEFINE_T int64_t
#elif DEFINE_SIZEOF == 16
#define DEFINE_T Dee_int128_t
#endif /* DEFINE_SIZEOF == ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 1
#define DEFINE_T uint8_t
#elif DEFINE_SIZEOF == 2
#define DEFINE_T uint16_t
#elif DEFINE_SIZEOF == 4
#define DEFINE_T uint32_t
#elif DEFINE_SIZEOF == 8
#define DEFINE_T uint64_t
#elif DEFINE_SIZEOF == 16
#define DEFINE_T Dee_uint128_t
#endif /* DEFINE_SIZEOF == ... */
#endif /* !DEFINE_SIGNED */
#ifndef DEFINE_T
#error "Must #define DEFINE_T for custom-width integer"
#endif /* !DEFINE_T */
#endif /* !DEFINE_T */

#ifndef DEFINE_NAME
#define DEFINE_NAME PP_STR(DEFINE_T)
#endif /* !DEFINE_NAME */

DECL_BEGIN

#undef LOCAL_X
#define LOCAL_X(x) PP_CAT2(x, DEFINE_TYPE_NAME)

#undef LOCAL_F_NOSIGN
#if DEFINE_SIZEOF == 1
#define LOCAL_F_NOSIGN(x) x##ns8
#elif DEFINE_SIZEOF == 2
#define LOCAL_F_NOSIGN(x) x##ns16
#elif DEFINE_SIZEOF == 4
#define LOCAL_F_NOSIGN(x) x##ns32
#elif DEFINE_SIZEOF == 8
#define LOCAL_F_NOSIGN(x) x##ns64
#elif DEFINE_SIZEOF == 16
#define LOCAL_F_NOSIGN(x) x##ns128
#else /* DEFINE_SIZEOF == ... */
#define LOCAL_F_NOSIGN(x) LOCAL_X(x##ns)
#endif /* DEFINE_SIZEOF != ... */

#undef LOCAL_GET_UNALIGNED
#undef LOCAL_SET_UNALIGNED
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
#define LOCAL_GET_UNALIGNED(ptr)    ((int8_t)UNALIGNED_GET8(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET8(ptr, (uint8_t)(v))
#elif DEFINE_SIZEOF == 2
#define LOCAL_GET_UNALIGNED(ptr)    ((int16_t)UNALIGNED_GET16(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET16(ptr, (uint16_t)(v))
#elif DEFINE_SIZEOF == 4
#define LOCAL_GET_UNALIGNED(ptr)    ((int32_t)UNALIGNED_GET32(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET32(ptr, (uint32_t)(v))
#elif DEFINE_SIZEOF == 8
#define LOCAL_GET_UNALIGNED(ptr)    ((int64_t)UNALIGNED_GET64(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET64(ptr, (uint64_t)(v))
#endif /* DEFINE_SIZEOF == ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 1
#define LOCAL_GET_UNALIGNED(ptr)    UNALIGNED_GET8(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET8(ptr, v)
#elif DEFINE_SIZEOF == 2
#define LOCAL_GET_UNALIGNED(ptr)    UNALIGNED_GET16(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET16(ptr, v)
#elif DEFINE_SIZEOF == 4
#define LOCAL_GET_UNALIGNED(ptr)    UNALIGNED_GET32(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET32(ptr, v)
#elif DEFINE_SIZEOF == 8
#define LOCAL_GET_UNALIGNED(ptr)    UNALIGNED_GET64(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET64(ptr, v)
#endif /* DEFINE_SIZEOF == ... */
#endif /* !DEFINE_SIGNED */

#ifndef LOCAL_GET_UNALIGNED
#if DEFINE_SIZEOF == 16
#ifdef UNALIGNED_GET128
#ifdef DEFINE_SIGNED
#define LOCAL_GET_UNALIGNED(ptr)    ((__INT128_TYPE__)UNALIGNED_GET128(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET128(ptr, (__UINT128_TYPE__)(v))
#else /* DEFINE_SIGNED */
#define LOCAL_GET_UNALIGNED(ptr)    ((__INT128_TYPE__)UNALIGNED_GET128(ptr))
#define LOCAL_SET_UNALIGNED(ptr, v) UNALIGNED_SET128(ptr, (__UINT128_TYPE__)(v))
#endif /* !DEFINE_SIGNED */
#else /* UNALIGNED_GET128 */
#ifdef DEFINE_SIGNED
#define LOCAL_GET_UNALIGNED(ptr)    Dee_UNALIGNED_GETS128(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) Dee_UNALIGNED_SETS128(ptr, v)
#else /* DEFINE_SIGNED */
#define LOCAL_GET_UNALIGNED(ptr)    Dee_UNALIGNED_GETU128(ptr)
#define LOCAL_SET_UNALIGNED(ptr, v) Dee_UNALIGNED_SETU128(ptr, v)
#endif /* !DEFINE_SIGNED */
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
#else /* DEFINE_SIZEOF == 16 */
#error "Unsupported DEFINE_SIZEOF"
#endif /* DEFINE_SIZEOF != 16 */
#endif /* !LOCAL_GET_UNALIGNED */


#define LOCAL_Integer LOCAL_X(Integer)

typedef struct {
	OBJECT_HEAD
	DEFINE_T i_value; /* The integer value. */
} LOCAL_Integer;


#if (((DEFINE_SIZEOF != 1 || (defined(DEFINE_SIGNED) ? !defined(INT8_FUNCTIONS_DEFINED) : !defined(UINT8_FUNCTIONS_DEFINED))) &&       \
      (DEFINE_SIZEOF != 2 || (defined(DEFINE_SIGNED) ? !defined(INT16_FUNCTIONS_DEFINED) : !defined(UINT16_FUNCTIONS_DEFINED))) &&     \
      (DEFINE_SIZEOF != 4 || (defined(DEFINE_SIGNED) ? !defined(INT32_FUNCTIONS_DEFINED) : !defined(UINT32_FUNCTIONS_DEFINED))) &&     \
      (DEFINE_SIZEOF != 8 || (defined(DEFINE_SIGNED) ? !defined(INT64_FUNCTIONS_DEFINED) : !defined(UINT64_FUNCTIONS_DEFINED))) &&     \
      (DEFINE_SIZEOF != 16 || (defined(DEFINE_SIGNED) ? !defined(INT128_FUNCTIONS_DEFINED) : !defined(UINT128_FUNCTIONS_DEFINED)))) || \
     defined(CONFIG_DONT_PROMOTE_TO_INTEGER))
#ifndef CONFIG_DONT_PROMOTE_TO_INTEGER
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
#define INT8_FUNCTIONS_DEFINED  1
#elif DEFINE_SIZEOF == 2
#define INT16_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 4
#define INT32_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 8
#define INT64_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 128
#define INT128_FUNCTIONS_DEFINED 1
#endif /* DEFINE_SIZEOF == ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 1
#define UINT8_FUNCTIONS_DEFINED  1
#elif DEFINE_SIZEOF == 2
#define UINT16_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 4
#define UINT32_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 8
#define UINT64_FUNCTIONS_DEFINED 1
#elif DEFINE_SIZEOF == 16
#define UINT128_FUNCTIONS_DEFINED 1
#endif /* DEFINE_SIZEOF == ... */
#endif /* !DEFINE_SIGNED */
#endif /* !CONFIG_DONT_PROMOTE_TO_INTEGER */

#if DEFINE_SIZEOF == 1
#ifndef INT8_SIGNLESS_FUNCTIONS_DEFINED
#define INT8_SIGNLESS_FUNCTIONS_DEFINED  1
#else /* !INT8_SIGNLESS_FUNCTIONS_DEFINED */
#define LOCAL_SIGNLESS_DEFINED 1
#endif /* INT8_SIGNLESS_FUNCTIONS_DEFINED */
#elif DEFINE_SIZEOF == 2
#ifndef INT16_SIGNLESS_FUNCTIONS_DEFINED
#define INT16_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT16_SIGNLESS_FUNCTIONS_DEFINED */
#define LOCAL_SIGNLESS_DEFINED 1
#endif /* INT16_SIGNLESS_FUNCTIONS_DEFINED */
#elif DEFINE_SIZEOF == 4
#ifndef INT32_SIGNLESS_FUNCTIONS_DEFINED
#define INT32_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT32_SIGNLESS_FUNCTIONS_DEFINED */
#define LOCAL_SIGNLESS_DEFINED 1
#endif /* INT32_SIGNLESS_FUNCTIONS_DEFINED */
#elif DEFINE_SIZEOF == 8
#ifndef INT64_SIGNLESS_FUNCTIONS_DEFINED
#define INT64_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT64_SIGNLESS_FUNCTIONS_DEFINED */
#define LOCAL_SIGNLESS_DEFINED 1
#endif /* INT64_SIGNLESS_FUNCTIONS_DEFINED */
#elif DEFINE_SIZEOF == 16
#ifndef INT128_SIGNLESS_FUNCTIONS_DEFINED
#define INT128_SIGNLESS_FUNCTIONS_DEFINED 1
#else /* !INT128_SIGNLESS_FUNCTIONS_DEFINED */
#define LOCAL_SIGNLESS_DEFINED 1
#endif /* INT128_SIGNLESS_FUNCTIONS_DEFINED */
#endif /* DEFINE_SIZEOF == ... */


#ifdef DEFINE_SIGNED
#define LOCAL_DeeObject_AsInt(x, result) DeeObject_AsXInt(DEFINE_SIZEOF, x, result)
#else /* DEFINE_SIGNED */
#define LOCAL_DeeObject_AsInt(x, result) DeeObject_AsXUInt(DEFINE_SIZEOF, x, result)
#endif /* !DEFINE_SIGNED */

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
	return Dee_AsObject(result);
}
#endif /* !INT_NEWINT_DEFINED */

#if !defined(DEFINE_SIGNED) || DEFINE_SIZEOF != CONFIG_CTYPES_SIZEOF_INT
INTERN WUNUSED DREF DeeObject *DCALL DEFINE_F(int_new)(DEFINE_T val) {
	LOCAL_Integer * result;
	result = DeeObject_MALLOC(LOCAL_Integer);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeSType_AsType(&DEFINE_TYPE_NAME));
	result->i_value = val;
done:
	return Dee_AsObject(result);
}
#endif /* !DEFINE_SIGNED || DEFINE_SIZEOF != CONFIG_CTYPES_SIZEOF_INT */

#ifdef CONFIG_DONT_PROMOTE_TO_INTEGER
#define LOCAL_newint_promoted(val) DEFINE_F(int_new_)(val)
PRIVATE WUNUSED DREF DeeObject *DCALL DEFINE_F(int_new_)(DEFINE_T val) {
	LOCAL_Integer * result;
	result = DeeObject_MALLOC(LOCAL_Integer);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeSType_AsType(&DEFINE_TYPE_NAME));
	result->i_value = val;
done:
	return Dee_AsObject(result);
}
#elif ((DEFINE_SIZEOF < CONFIG_CTYPES_SIZEOF_INT) || \
       (defined(DEFINE_SIGNED) && DEFINE_SIZEOF == CONFIG_CTYPES_SIZEOF_INT))
/* Integer promotion. */
#define LOCAL_newint_promoted(val) int_newint((CTYPES_INT)(val))
#else /* ... */
#define LOCAL_newint_promoted(val) DEFINE_F(int_new)(val)
#endif /* !... */

#ifdef DEFINE_SIGNED
#define LOCAL___hybrid_int128_(x) __hybrid_int128_##x
#else /* DEFINE_SIGNED */
#define LOCAL___hybrid_int128_(x) __hybrid_uint128_##x
#endif /* !DEFINE_SIGNED */


/* Define functions. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DEFINE_F(intinit)(DeeSTypeObject *__restrict UNUSED(tp_self),
           DEFINE_T *self, size_t argc, DeeObject *const *argv) {
	DEFINE_T value;
	DeeObject *arg;
	DeeArg_Unpack1(err, argc, argv, DEFINE_NAME, &arg);
	if (LOCAL_DeeObject_AsInt(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, value), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(intass)(DeeSTypeObject *__restrict UNUSED(tp_self),
          DEFINE_T *self, DeeObject *__restrict arg) {
	DEFINE_T value;
	if (LOCAL_DeeObject_AsInt(arg, &value))
		goto err;
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, value), goto err);
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
DEFINE_F(intstr)(DeeSTypeObject *__restrict UNUSED(tp_self),
          DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return NULL);
#ifdef CONFIG_BOOL_STRING
	return_reference_(value ? Dee_AsObject(&str_true)
	                        : Dee_AsObject(&str_false));
#else /* CONFIG_BOOL_STRING */
	return DeeString_Newf(DEFINE_PRINTF_FMT, (DEFINE_PRINTF_TYP)value);
#endif /* !CONFIG_BOOL_STRING */
}

#ifndef LOCAL_SIGNLESS_DEFINED
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_F_NOSIGN(intbool)(DeeSTypeObject *__restrict UNUSED(tp_self),
                  DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self),
	                    return -1);
#if DEFINE_SIZEOF == 16
	return !LOCAL___hybrid_int128_(iszero)(value);
#else /* DEFINE_SIZEOF == 16 */
	return value != 0;
#endif /* DEFINE_SIZEOF != 16 */
}
#endif /* !LOCAL_SIGNLESS_DEFINED */

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_int32)(DeeSTypeObject *__restrict UNUSED(tp_self),
                    DEFINE_T *self, int32_t *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return -1);
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 16
	*result = __hybrid_int128_get32(value);
	return INT_SIGNED;
#else /* DEFINE_SIZEOF == 16 */
	*result = (int32_t)value;
	return INT_SIGNED;
#endif /* DEFINE_SIZEOF != 16 */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 16
	*result = __hybrid_uint128_get32(value);
	return INT_UNSIGNED;
#else /* DEFINE_SIZEOF == 16 */
	*(uint32_t *)result = (uint32_t)value;
	return INT_UNSIGNED;
#endif /* DEFINE_SIZEOF != 16 */
#endif /* !DEFINE_SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_int64)(DeeSTypeObject *__restrict UNUSED(tp_self),
             DEFINE_T *self, int64_t *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return -1);
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 16
	*result = __hybrid_int128_get64(value);
	return INT_SIGNED;
#else /* DEFINE_SIZEOF == 16 */
	*result = (int64_t)value;
	return INT_SIGNED;
#endif /* DEFINE_SIZEOF != 16 */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 16
	*result = __hybrid_uint128_get64(value);
	return INT_UNSIGNED;
#else /* DEFINE_SIZEOF == 16 */
	*(uint64_t *)result = (uint64_t)value;
	return INT_UNSIGNED;
#endif /* DEFINE_SIZEOF != 16 */
#endif /* !DEFINE_SIGNED */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_double)(DeeSTypeObject *__restrict UNUSED(tp_self),
                     DEFINE_T *self, double *__restrict result) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return -1);
#if DEFINE_SIZEOF == 16
	*result = (double)__hybrid_int128_get64(value);
#else /* DEFINE_SIZEOF == 16 */
	*result = (double)value;
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DEFINE_F(int_int)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return NULL);
#ifdef DEFINE_SIGNED
	return _DeeInt_NewS(DEFINE_SIZEOF, value);
#else /* DEFINE_SIGNED */
	return _DeeInt_NewU(DEFINE_SIZEOF, value);
#endif /* !DEFINE_SIGNED */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DEFINE_F(int_inv)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return NULL);
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(inv)(value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(~value);
#endif /* DEFINE_SIZEOF != 16 */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DEFINE_F(int_pos)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return NULL);
#if DEFINE_SIZEOF == 16
	/*LOCAL___hybrid_int128_(pos)(value);*/
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(+value);
#endif /* DEFINE_SIZEOF != 16 */
}

#ifdef DEFINE_SIGNED
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DEFINE_F(int_neg)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
	DEFINE_T value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), return NULL);
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(neg)(value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(-value);
#endif /* DEFINE_SIZEOF != 16 */
}
#endif /* DEFINE_SIGNED */

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_add)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(add128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value + other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(sub128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value - other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(mul128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value * other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((2)) int DCALL
DEFINE_F(int_divzero)(DEFINE_T value, DeeObject *__restrict some_object) {
	int result;
	struct Dee_variant lhs, rhs;
#if DEFINE_SIZEOF >= 16 && defined(DEFINE_SIGNED)
	Dee_variant_init_int128(&lhs, value);
#elif DEFINE_SIZEOF >= 16
	Dee_variant_init_uint128(&lhs, value);
#elif DEFINE_SIZEOF >= 8 && defined(DEFINE_SIGNED)
	Dee_variant_init_int64(&lhs, value);
#elif DEFINE_SIZEOF >= 8
	Dee_variant_init_uint64(&lhs, value);
#elif defined(DEFINE_SIGNED)
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
DEFINE_F(int_div)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	if unlikely(LOCAL___hybrid_int128_(iszero)(other_value)) {
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
	LOCAL___hybrid_int128_(div128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	if unlikely(!other_value) {
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
	return LOCAL_newint_promoted(value / other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_mod)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	if unlikely(LOCAL___hybrid_int128_(iszero)(other_value)) {
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
	LOCAL___hybrid_int128_(mod128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	if unlikely(!other_value) {
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
	return LOCAL_newint_promoted(value % other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value;
	__SHIFT_TYPE__ other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(shl)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value << other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value;
	__SHIFT_TYPE__ other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(shr)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value >> other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_and)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(and128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value & other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_or)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
          DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(or128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value | other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_xor)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	DEFINE_T value, other_value;
	CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	LOCAL___hybrid_int128_(xor128)(value, other_value);
	return LOCAL_newint_promoted(value);
#else /* DEFINE_SIZEOF == 16 */
	return LOCAL_newint_promoted(value ^ other_value);
#endif /* DEFINE_SIZEOF != 16 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DEFINE_F(int_inc)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(inc)(temp);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) + 1)), return -1);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DEFINE_F(int_dec)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self) {
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(dec)(temp);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) - 1)), return -1);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_add)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(add128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) + other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_sub)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(sub128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) - other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_mul)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(mul128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) * other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_div)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	if unlikely(LOCAL___hybrid_int128_(iszero)(other_value))
#else /* DEFINE_SIZEOF == 16 */
	if unlikely(!other_value)
#endif /* DEFINE_SIZEOF != 16 */
	{
		DEFINE_T value;
		CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(div128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) / other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_mod)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	if unlikely(LOCAL___hybrid_int128_(iszero)(other_value))
#else /* DEFINE_SIZEOF == 16 */
	if unlikely(!other_value)
#endif /* DEFINE_SIZEOF != 16 */
	{
		DEFINE_T value;
		CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);
		(DEFINE_F(int_divzero)(value, some_object));
		goto err;
	}
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(mod128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) % other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_shl)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	__SHIFT_TYPE__ other_value;
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(shl)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) << other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_shr)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	__SHIFT_TYPE__ other_value;
	if (DeeObject_AsUIntX(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(shr)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) >> other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_and)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(and128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) & other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_or)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                  DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(or128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) | other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_xor)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	DEFINE_T other_value;
	if (LOCAL_DeeObject_AsInt(some_object, &other_value))
		goto err;
#if DEFINE_SIZEOF == 16
	CTYPES_FAULTPROTECT({
		DEFINE_T temp = LOCAL_GET_UNALIGNED(self);
		LOCAL___hybrid_int128_(xor128)(temp, other_value);
		LOCAL_SET_UNALIGNED(self, temp);
	}, return -1);
#else /* DEFINE_SIZEOF == 16 */
	CTYPES_FAULTPROTECT(LOCAL_SET_UNALIGNED(self, (DEFINE_T)(LOCAL_GET_UNALIGNED(self) ^ other_value)), goto err);
#endif /* DEFINE_SIZEOF != 16 */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
DEFINE_F(int_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
           DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
DEFINE_F(int_inplace_pow)(DeeSTypeObject *__restrict UNUSED(tp_self), DEFINE_T *self,
                   DeeObject *__restrict some_object) {
	(void)self;
	(void)some_object;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return -1;
}


PRIVATE struct stype_math DEFINE_F(intmath) = {
	/* .st_int32       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int32_t *__restrict))&DEFINE_F(int_int32),
	/* .st_int64       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int64_t *__restrict))&DEFINE_F(int_int64),
	/* .st_double      = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, double *__restrict))&DEFINE_F(int_double),
	/* .st_int         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_int),
	/* .st_inv         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_inv),
	/* .st_pos         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_pos),
#ifdef DEFINE_SIGNED
	/* .st_neg         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_neg),
#else /* DEFINE_SIGNED */
	/* .st_neg         = */ NULL,
#endif /* !DEFINE_SIGNED */
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_add),
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_sub),
	/* .st_mul         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_mul),
	/* .st_div         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_div),
	/* .st_mod         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_mod),
	/* .st_shl         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_shl),
	/* .st_shr         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_shr),
	/* .st_and         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_and),
	/* .st_or          = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_or),
	/* .st_xor         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_xor),
	/* .st_pow         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_pow),
	/* .st_inc         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_inc),
	/* .st_dec         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(int_dec),
	/* .st_inplace_add = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_add),
	/* .st_inplace_sub = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_sub),
	/* .st_inplace_mul = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_mul),
	/* .st_inplace_div = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_div),
	/* .st_inplace_mod = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_mod),
	/* .st_inplace_shl = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_shl),
	/* .st_inplace_shr = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_shr),
	/* .st_inplace_and = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_and),
	/* .st_inplace_or  = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_or),
	/* .st_inplace_xor = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_xor),
	/* .st_inplace_pow = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_inplace_pow)
};

#if DEFINE_SIZEOF == 16
#define CTYPES_INT_DEFINE_COMPARE_OPERATOR(name, op)            \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL       \
	DEFINE_F(int_##name)(DeeSTypeObject *UNUSED(tp_self),              \
	              DEFINE_T *self, DeeObject *some_object) {            \
		DEFINE_T value, other_value;                                   \
		CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err);       \
		if (LOCAL_DeeObject_AsInt(some_object, &other_value))             \
			goto err;                                           \
		return_bool(LOCAL___hybrid_int128_(name##128)(value, other_value)); \
	err:                                                        \
		return NULL;                                            \
	}
#else /* DEFINE_SIZEOF == 16 */
#define CTYPES_INT_DEFINE_COMPARE_OPERATOR(name, op)      \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL \
	DEFINE_F(int_##name)(DeeSTypeObject *UNUSED(tp_self),        \
	              DEFINE_T *self, DeeObject *some_object) {      \
		DEFINE_T value, other_value;                             \
		CTYPES_FAULTPROTECT(value = LOCAL_GET_UNALIGNED(self), goto err); \
		if (LOCAL_DeeObject_AsInt(some_object, &other_value))       \
			goto err;                                     \
		return_bool(value op other_value);                \
	err:                                                  \
		return NULL;                                      \
	}
#endif /* DEFINE_SIZEOF != 16 */
CTYPES_INT_DEFINE_COMPARE_OPERATOR(eq, ==)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(ne, !=)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(lo, <)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(le, <=)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(gr, >)
CTYPES_INT_DEFINE_COMPARE_OPERATOR(ge, >=)
#undef CTYPES_INT_DEFINE_COMPARE_OPERATOR

PRIVATE struct stype_cmp DEFINE_F(intcmp) = {
	/* .st_eq = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_eq),
	/* .st_ne = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_ne),
	/* .st_lo = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_lo),
	/* .st_le = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_le),
	/* .st_gr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_gr),
	/* .st_ge = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(int_ge)
};

#undef LOCAL_SIGNLESS_DEFINED
#undef LOCAL_newint_promoted
#undef LOCAL_DeeObject_AsInt
#endif /* Integer functions. */


PRIVATE LOCAL_Integer LOCAL_X(int_min) = {
	OBJECT_HEAD_INIT(DeeSType_AsType(&DEFINE_TYPE_NAME)),
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
	INT8_MIN
#elif DEFINE_SIZEOF == 2
	INT16_MIN
#elif DEFINE_SIZEOF == 4
	INT32_MIN
#elif DEFINE_SIZEOF == 8
	INT64_MIN
#elif DEFINE_SIZEOF == 16
	__HYBRID_INT128_INIT32N(UINT32_C(0x80000000), 0, 0, 0)
#else /* DEFINE_SIZEOF == ... */
#error "Invalid `DEFINE_SIZEOF'"
#endif /* DEFINE_SIZEOF != ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 16
	__HYBRID_UINT128_INIT32N(0, 0, 0, 0)
#else /* DEFINE_SIZEOF == ... */
	0
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_SIGNED */
};

PRIVATE LOCAL_Integer LOCAL_X(int_max) = {
	OBJECT_HEAD_INIT(DeeSType_AsType(&DEFINE_TYPE_NAME)),
#ifdef DEFINE_SIGNED
#if DEFINE_SIZEOF == 1
	INT8_MAX
#elif DEFINE_SIZEOF == 2
	INT16_MAX
#elif DEFINE_SIZEOF == 4
	INT32_MAX
#elif DEFINE_SIZEOF == 8
	INT64_MAX
#elif DEFINE_SIZEOF == 16
	__HYBRID_INT128_INIT32N(UINT32_C(0x7fffffff), UINT32_C(0xffffffff),
	                        UINT32_C(0xffffffff), UINT32_C(0xffffffff))
#else /* DEFINE_SIZEOF == ... */
#error "Invalid `DEFINE_SIZEOF'"
#endif /* DEFINE_SIZEOF != ... */
#else /* DEFINE_SIGNED */
#if DEFINE_SIZEOF == 1
	UINT8_MAX
#elif DEFINE_SIZEOF == 2
	UINT16_MAX
#elif DEFINE_SIZEOF == 4
	UINT32_MAX
#elif DEFINE_SIZEOF == 8
	UINT64_MAX
#elif DEFINE_SIZEOF == 16
	__HYBRID_UINT128_INIT32N(UINT32_C(0xffffffff), UINT32_C(0xffffffff),
	                         UINT32_C(0xffffffff), UINT32_C(0xffffffff))
#else /* DEFINE_SIZEOF == ... */
#error "Invalid `DEFINE_SIZEOF'"
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_SIGNED */
};

PRIVATE struct type_member tpconst LOCAL_X(int_class_members)[] = {
#if !defined(DEFINE_NO_SIGNED_TYPE_NAME) && !defined(DEFINE_NO_UNSIGNED_TYPE_NAME)
	TYPE_MEMBER_CONST("signed", DeeSType_AsType(&DEFINE_TYPE_NAME_SIGNED)),
	TYPE_MEMBER_CONST("unsigned", DeeSType_AsType(&DEFINE_TYPE_NAME_UNSIGNED)),
#endif /* !DEFINE_NO_SIGNED_TYPE_NAME && !DEFINE_NO_UNSIGNED_TYPE_NAME */
	TYPE_MEMBER_CONST("min", &LOCAL_X(int_min)),
	TYPE_MEMBER_CONST("max", &LOCAL_X(int_max)),
	TYPE_MEMBER_END
};


#ifndef CONFIG_NO_CFUNCTION
#ifndef DEFINE_FFI_TYPE
#if DEFINE_SIZEOF == 1 && defined(DEFINE_SIGNED)
#define DEFINE_FFI_TYPE ffi_type_sint8
#elif DEFINE_SIZEOF == 1
#define DEFINE_FFI_TYPE ffi_type_uint8
#elif DEFINE_SIZEOF == 2 && defined(DEFINE_SIGNED)
#define DEFINE_FFI_TYPE ffi_type_sint16
#elif DEFINE_SIZEOF == 2
#define DEFINE_FFI_TYPE ffi_type_uint16
#elif DEFINE_SIZEOF == 4 && defined(DEFINE_SIGNED)
#define DEFINE_FFI_TYPE ffi_type_sint32
#elif DEFINE_SIZEOF == 4
#define DEFINE_FFI_TYPE ffi_type_uint32
#elif DEFINE_SIZEOF == 8 && defined(DEFINE_SIGNED)
#define DEFINE_FFI_TYPE ffi_type_sint64
#elif DEFINE_SIZEOF == 8
#define DEFINE_FFI_TYPE ffi_type_uint64
#elif DEFINE_SIZEOF == 16 && defined(DEFINE_SIGNED)
//#define DEFINE_FFI_TYPE ffi_type_sint128 /* TODO */
#elif DEFINE_SIZEOF == 16
//#define DEFINE_FFI_TYPE ffi_type_uint128 /* TODO */
#else /* DEFINE_SIZEOF == ... */
#error "Unsupported integral type for use with FFY (Try builtin with `-DCONFIG_NO_CFUNCTION')"
#endif /* DEFINE_SIZEOF != ... */
#endif /* !DEFINE_FFI_TYPE */
#endif /* !CONFIG_NO_CFUNCTION */


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
				/* T:              */ LOCAL_Integer,
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
		/* .tp_class_members = */ LOCAL_X(int_class_members),
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
#ifdef DEFINE_FFI_TYPE
	/* .st_ffitype  = */ &DEFINE_FFI_TYPE,
#else /* DEFINE_FFI_TYPE */
	/* .st_ffitype  = */ NULL,
#endif /* !DEFINE_FFI_TYPE */
#endif /* !CONFIG_NO_CFUNCTION */
	/* .st_sizeof   = */ DEFINE_SIZEOF,
	/* .st_align    = */ DEFINE_ALIGNOF,
	/* .st_init     = */ (int (DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&DEFINE_F(intinit),
	/* .st_assign   = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&DEFINE_F(intass),
	/* .st_cast     = */ {
		/* .st_str  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(intstr),
		/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&DEFINE_F(intstr),
		/* .st_bool = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&LOCAL_F_NOSIGN(intbool)
	},
	/* .st_call     = */ NULL,
	/* .st_math     = */ &DEFINE_F(intmath),
	/* .st_cmp      = */ &DEFINE_F(intcmp),
	/* .st_seq      = */ NULL,
	/* .st_attr     = */ NULL
};

#undef LOCAL_Integer

#undef LOCAL_GET_UNALIGNED
#undef LOCAL_SET_UNALIGNED
#undef LOCAL___hybrid_int128_
#undef LOCAL_X
#undef LOCAL_F_NOSIGN

DECL_END

/* Configuration parameters that (may) be defined externally */
#undef DEFINE_NO_SIGNED_TYPE_NAME
#undef DEFINE_NO_UNSIGNED_TYPE_NAME
#undef DEFINE_FFI_TYPE
#undef DEFINE_NAME
#undef DEFINE_PRINTF_TYP
#undef DEFINE_PRINTF_FMT
#undef DEFINE_TYPE_NAME_SIGNED
#undef DEFINE_TYPE_NAME_UNSIGNED
#undef DEFINE_TYPE_NAME
#undef DEFINE_SIGNED
#undef DEFINE_SIZEOF
#undef DEFINE_ALIGNOF
#undef DEFINE_T
#undef DEFINE_F
