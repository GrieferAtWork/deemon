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
//#define DEFINE_cint8_operators
//#define DEFINE_cint16_operators
//#define DEFINE_cint32_operators
//#define DEFINE_cint64_operators
//#define DEFINE_cint128_operators
//#define DEFINE_cuint8_operators
//#define DEFINE_cuint16_operators
//#define DEFINE_cuint32_operators
//#define DEFINE_cuint64_operators
//#define DEFINE_cuint128_operators
//#define DEFINE_cint16_bswap_operators
//#define DEFINE_cint32_bswap_operators
//#define DEFINE_cint64_bswap_operators
#define DEFINE_cint128_bswap_operators
//#define DEFINE_cuint16_bswap_operators
//#define DEFINE_cuint32_bswap_operators
//#define DEFINE_cuint64_bswap_operators
//#define DEFINE_cuint128_bswap_operators
#include "ctypes-operators.c"
#endif /* __INTELLISENSE__ */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStructKw, UNP* */
#include <deemon/error-rt.h>        /* DeeRT_Err* */
#include <deemon/format.h>          /* DeeFormat_Printf, PRF* */
#include <deemon/int.h>             /* DeeInt_*, Dee_INT_SIGNED, Dee_INT_UNSIGNED */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, Dee_COMPARE_*, Dee_formatprinter_t, Dee_int128_t, Dee_ssize_t, Dee_uint128_t */
#include <deemon/system-features.h> /* memcpy */
#include <deemon/type.h>            /* Dee_INT_* */
#include <deemon/variant.h>         /* Dee_variant, Dee_variant_* */

#include <hybrid/int128.h>    /* __hybrid_int128_*, __hybrid_uint128_* */
#include <hybrid/limitcore.h> /* __INTn_MAX__, __INTn_MIN__, __UINTn_MAX__ */
#include <hybrid/typecore.h>  /* __SHIFT_TYPE__ */
#include <hybrid/unaligned.h> /* UNALIGNED_GET*, UNALIGNED_SET* */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* intN_t, uintN_t */

#if (defined(DEFINE_cint8_operators) +         \
     defined(DEFINE_cint16_operators) +        \
     defined(DEFINE_cint32_operators) +        \
     defined(DEFINE_cint64_operators) +        \
     defined(DEFINE_cint128_operators) +       \
     defined(DEFINE_cuint8_operators) +        \
     defined(DEFINE_cuint16_operators) +       \
     defined(DEFINE_cuint32_operators) +       \
     defined(DEFINE_cuint64_operators) +       \
     defined(DEFINE_cuint128_operators) +      \
     defined(DEFINE_cint16_bswap_operators) +  \
     defined(DEFINE_cint32_bswap_operators) +  \
     defined(DEFINE_cint64_bswap_operators) +  \
     defined(DEFINE_cint128_bswap_operators) + \
     defined(DEFINE_cuint16_bswap_operators) + \
     defined(DEFINE_cuint32_bswap_operators) + \
     defined(DEFINE_cuint64_bswap_operators) + \
     defined(DEFINE_cuint128_bswap_operators)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

#ifndef INT8_MIN
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN */
#ifndef INT8_MAX
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX */
#ifndef UINT8_MAX
#define UINT8_MAX __UINT8_MAX__
#endif /* !UINT8_MAX */
#ifndef INT16_MIN
#define INT16_MIN __INT16_MIN__
#endif /* !INT16_MIN */
#ifndef INT16_MAX
#define INT16_MAX __INT16_MAX__
#endif /* !INT16_MAX */
#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */
#ifndef INT32_MIN
#define INT32_MIN __INT32_MIN__
#endif /* !INT32_MIN */
#ifndef INT32_MAX
#define INT32_MAX __INT32_MAX__
#endif /* !INT32_MAX */
#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */
#ifndef INT64_MIN
#define INT64_MIN __INT64_MIN__
#endif /* !INT64_MIN */
#ifndef INT64_MAX
#define INT64_MAX __INT64_MAX__
#endif /* !INT64_MAX */
#ifndef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif /* !UINT64_MAX */

#undef shift_t
#define shift_t __SHIFT_TYPE__

DECL_BEGIN

#ifdef DEFINE_cint8_operators
#define LOCAL_operators     cint8_operators
#define LOCAL_sizeof_intN_t 1
#elif defined(DEFINE_cint16_operators)
#define LOCAL_operators     cint16_operators
#define LOCAL_sizeof_intN_t 2
#elif defined(DEFINE_cint32_operators)
#define LOCAL_operators     cint32_operators
#define LOCAL_sizeof_intN_t 4
#elif defined(DEFINE_cint64_operators)
#define LOCAL_operators     cint64_operators
#define LOCAL_sizeof_intN_t 8
#elif defined(DEFINE_cint128_operators)
#define LOCAL_operators     cint128_operators
#define LOCAL_sizeof_intN_t 16

#define LOCAL_UNALIGNED_GET cint128_unaligned_get
LOCAL ATTR_PURE WUNUSED Dee_int128_t DCALL
cint128_unaligned_get(void const *p) {
	Dee_int128_t result;
	memcpy(&result, p, 16);
	return result;
}
#define LOCAL_UNALIGNED_SET              cint128_unaligned_set
#define cint128_unaligned_set(p, cvalue) memcpy(p, &(cvalue), 16)
#elif defined(DEFINE_cuint8_operators)
#define LOCAL_operators     cuint8_operators
#define LOCAL_sizeof_intN_t 1
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint16_operators)
#define LOCAL_operators     cuint16_operators
#define LOCAL_sizeof_intN_t 2
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint32_operators)
#define LOCAL_operators     cuint32_operators
#define LOCAL_sizeof_intN_t 4
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint64_operators)
#define LOCAL_operators     cuint64_operators
#define LOCAL_sizeof_intN_t 8
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint128_operators)
#define LOCAL_operators     cuint128_operators
#define LOCAL_sizeof_intN_t 16
#define LOCAL_UNSIGNED

#define LOCAL_UNALIGNED_GET cuint128_unaligned_get
LOCAL ATTR_PURE WUNUSED Dee_uint128_t DCALL
cuint128_unaligned_get(void const *p) {
	Dee_uint128_t result;
	memcpy(&result, p, 16);
	return result;
}
#define LOCAL_UNALIGNED_SET               cuint128_unaligned_set
#define cuint128_unaligned_set(p, cvalue) memcpy(p, &(cvalue), 16)
#elif defined(DEFINE_cint16_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cint16_bswap_operators
#define LOCAL_sizeof_intN_t 2
#elif defined(DEFINE_cint32_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cint32_bswap_operators
#define LOCAL_sizeof_intN_t 4
#elif defined(DEFINE_cint64_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cint64_bswap_operators
#define LOCAL_sizeof_intN_t 8
#elif defined(DEFINE_cint128_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cint128_bswap_operators
#define LOCAL_sizeof_intN_t 16

#define LOCAL_UNALIGNED_GET cint128_unaligned_get_bswap
LOCAL ATTR_PURE WUNUSED Dee_int128_t DCALL
cint128_unaligned_get_bswap(void const *p) {
	Dee_int128_t result;
	memcpy(&result, p, 16);
	__hybrid_int128_bswap(result);
	return result;
}
#define LOCAL_UNALIGNED_SET cint128_unaligned_set_bswap
#define cint128_unaligned_set_bswap(p, cvalue) \
	do {                                       \
		Dee_int128_t _set_temp = (cvalue);     \
		__hybrid_int128_bswap(_set_temp);      \
		memcpy(p, &_set_temp, 16);             \
	}	__WHILE0
#elif defined(DEFINE_cuint16_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cuint16_bswap_operators
#define LOCAL_sizeof_intN_t 2
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint32_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cuint32_bswap_operators
#define LOCAL_sizeof_intN_t 4
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint64_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cuint64_bswap_operators
#define LOCAL_sizeof_intN_t 8
#define LOCAL_UNSIGNED
#elif defined(DEFINE_cuint128_bswap_operators)
#define LOCAL_BSWAP
#define LOCAL_operators     cuint128_bswap_operators
#define LOCAL_sizeof_intN_t 16
#define LOCAL_UNSIGNED

#define LOCAL_UNALIGNED_GET cuint128_unaligned_get_bswap
LOCAL ATTR_PURE WUNUSED Dee_uint128_t DCALL
cuint128_unaligned_get_bswap(void const *p) {
	Dee_uint128_t result;
	memcpy(&result, p, 16);
	__hybrid_uint128_bswap(result);
	return result;
}
#define LOCAL_UNALIGNED_SET cuint128_unaligned_set_bswap
#define cuint128_unaligned_set_bswap(p, cvalue) \
	do {                                        \
		Dee_uint128_t _set_temp = (cvalue);     \
		__hybrid_uint128_bswap(_set_temp);      \
		memcpy(p, &_set_temp, 16);              \
	}	__WHILE0
#else /* ... */
#error "Invalid configuration"
#endif /* !... */


#ifndef LOCAL_intN_t
#if LOCAL_sizeof_intN_t == 1
#define LOCAL_signed_intN_t   int8_t
#define LOCAL_unsigned_intN_t uint8_t
#elif LOCAL_sizeof_intN_t == 2
#define LOCAL_signed_intN_t   int16_t
#define LOCAL_unsigned_intN_t uint16_t
#elif LOCAL_sizeof_intN_t == 4
#define LOCAL_signed_intN_t   int32_t
#define LOCAL_unsigned_intN_t uint32_t
#elif LOCAL_sizeof_intN_t == 8
#define LOCAL_signed_intN_t   int64_t
#define LOCAL_unsigned_intN_t uint64_t
#elif LOCAL_sizeof_intN_t == 16
#define LOCAL_signed_intN_t   Dee_int128_t
#define LOCAL_unsigned_intN_t Dee_uint128_t
#else /* LOCAL_sizeof_intN_t == ... */
#error "Invalid configuration"
#endif /* LOCAL_sizeof_intN_t != ... */

#ifndef LOCAL_UNSIGNED
#define LOCAL_intN_t LOCAL_signed_intN_t
#if LOCAL_sizeof_intN_t == 1
#define LOCAL_STR_intN_t            "int8_t"
#define LOCAL_UNPduN                UNPd8
#define LOCAL_PRFduN                PRFd8
#define LOCAL_DeeObject_AsIntN      DeeObject_AsInt8
#define LOCAL_DeeInt_New            DeeInt_NewInt8
#define LOCAL_CInt_New              CInt8_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_int8
#elif LOCAL_sizeof_intN_t == 2
#define LOCAL_STR_intN_t            "int16_t"
#define LOCAL_UNPduN                UNPd16
#define LOCAL_PRFduN                PRFd16
#define LOCAL_DeeObject_AsIntN      DeeObject_AsInt16
#define LOCAL_DeeInt_New            DeeInt_NewInt16
#define LOCAL_CInt_New              CInt16_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_int16
#elif LOCAL_sizeof_intN_t == 4
#define LOCAL_STR_intN_t            "int32_t"
#define LOCAL_UNPduN                UNPd32
#define LOCAL_PRFduN                PRFd32
#define LOCAL_DeeObject_AsIntN      DeeObject_AsInt32
#define LOCAL_DeeInt_New            DeeInt_NewInt32
#define LOCAL_CInt_New              CInt32_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_int32
#elif LOCAL_sizeof_intN_t == 8
#define LOCAL_STR_intN_t            "int64_t"
#define LOCAL_UNPduN                UNPd64
#define LOCAL_PRFduN                PRFd64
#define LOCAL_DeeObject_AsIntN      DeeObject_AsInt64
#define LOCAL_DeeInt_New            DeeInt_NewInt64
#define LOCAL_CInt_New              CInt64_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_int64
#elif LOCAL_sizeof_intN_t == 16
#define LOCAL_STR_intN_t            "int128_t"
#define LOCAL_UNPduN                UNPd128
#define LOCAL_PRFduN                PRFd128
#define LOCAL_DeeObject_AsIntN      DeeObject_AsInt128
#define LOCAL_DeeInt_New            DeeInt_NewInt128
#define LOCAL_CInt_New              CInt128_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_int128
#else /* LOCAL_sizeof_intN_t == ... */
#error "Invalid configuration"
#endif /* LOCAL_sizeof_intN_t != ... */
#else  /* !LOCAL_UNSIGNED */
#define LOCAL_intN_t LOCAL_unsigned_intN_t
#if LOCAL_sizeof_intN_t == 1
#define LOCAL_STR_intN_t            "uint8_t"
#define LOCAL_UNPduN                UNPx8
#define LOCAL_PRFduN                PRFu8
#define LOCAL_DeeObject_AsIntN      DeeObject_AsUInt8M1
#define LOCAL_DeeInt_New            DeeInt_NewUInt8
#define LOCAL_CInt_New              CUInt8_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_uint8
#elif LOCAL_sizeof_intN_t == 2
#define LOCAL_STR_intN_t            "uint16_t"
#define LOCAL_UNPduN                UNPx16
#define LOCAL_PRFduN                PRFu16
#define LOCAL_DeeObject_AsIntN      DeeObject_AsUInt16M1
#define LOCAL_DeeInt_New            DeeInt_NewUInt16
#define LOCAL_CInt_New              CUInt16_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_uint16
#elif LOCAL_sizeof_intN_t == 4
#define LOCAL_STR_intN_t            "uint32_t"
#define LOCAL_UNPduN                UNPx32
#define LOCAL_PRFduN                PRFu32
#define LOCAL_DeeObject_AsIntN      DeeObject_AsUInt32M1
#define LOCAL_DeeInt_New            DeeInt_NewUInt32
#define LOCAL_CInt_New              CUInt32_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_uint32
#elif LOCAL_sizeof_intN_t == 8
#define LOCAL_STR_intN_t            "uint64_t"
#define LOCAL_UNPduN                UNPx64
#define LOCAL_PRFduN                PRFu64
#define LOCAL_DeeObject_AsIntN      DeeObject_AsUInt64M1
#define LOCAL_DeeInt_New            DeeInt_NewUInt64
#define LOCAL_CInt_New              CUInt64_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_uint64
#elif LOCAL_sizeof_intN_t == 16
#define LOCAL_STR_intN_t            "uint128_t"
#define LOCAL_UNPduN                UNPx128
#define LOCAL_PRFduN                PRFu128
#define LOCAL_DeeObject_AsIntN      DeeObject_AsUInt128M1
#define LOCAL_DeeInt_New            DeeInt_NewUInt128
#define LOCAL_CInt_New              CUInt128_New
#define LOCAL_Dee_variant_init_intN Dee_variant_init_uint128
#else /* LOCAL_sizeof_intN_t == ... */
#error "Invalid configuration"
#endif /* LOCAL_sizeof_intN_t != ... */
#endif /* LOCAL_UNSIGNED */

#define LOCAL_CInt_New_t LOCAL_intN_t
#if LOCAL_sizeof_intN_t < CTYPES_sizeof_int
/* Integer promotion */
#undef LOCAL_CInt_New
#undef LOCAL_CInt_New_t
#define LOCAL_CInt_New   CIntN_New(CTYPES_sizeof_int)
#define LOCAL_CInt_New_t HOST_INTFOR(CTYPES_sizeof_int)
#endif /* LOCAL_sizeof_intN_t < CTYPES_sizeof_int */
#endif /* !LOCAL_intN_t */

#ifndef LOCAL_UNALIGNED_GET
#if LOCAL_sizeof_intN_t == 1
#define LOCAL_UNALIGNED_GET(p)    (*(LOCAL_intN_t const *)(p))
#define LOCAL_UNALIGNED_SET(p, v) (void)(*(LOCAL_intN_t *)(p) = (LOCAL_intN_t)(v))
#else /* LOCAL_sizeof_intN_t == 1 */
#if LOCAL_sizeof_intN_t == 2
#ifdef LOCAL_BSWAP
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET16_SWAP
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET16_SWAP
#else /* LOCAL_BSWAP */
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET16
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET16
#endif /* !LOCAL_BSWAP */
#elif LOCAL_sizeof_intN_t == 4
#ifdef LOCAL_BSWAP
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET32_SWAP
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET32_SWAP
#else /* LOCAL_BSWAP */
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET32
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET32
#endif /* !LOCAL_BSWAP */
#elif LOCAL_sizeof_intN_t == 8
#ifdef LOCAL_BSWAP
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET64_SWAP
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET64_SWAP
#else /* LOCAL_BSWAP */
#define _LOCAL_UNALIGNED_GET UNALIGNED_GET64
#define _LOCAL_UNALIGNED_SET UNALIGNED_SET64
#endif /* !LOCAL_BSWAP */
#else
#error "Missing 'LOCAL_UNALIGNED_GET' / 'LOCAL_UNALIGNED_SET'"
#endif
#define LOCAL_UNALIGNED_GET(p)    ((LOCAL_intN_t)_LOCAL_UNALIGNED_GET((LOCAL_unsigned_intN_t const *)(p)))
#define LOCAL_UNALIGNED_SET(p, v) _LOCAL_UNALIGNED_SET((LOCAL_unsigned_intN_t *)(p), (LOCAL_unsigned_intN_t)(v))
#endif /* LOCAL_sizeof_intN_t != 1 */
#endif /* !LOCAL_UNALIGNED_GET */


#define LOCAL_FUNC(x)     PP_CAT2(LOCAL_operators, x)
#define LOCAL_initfrom    LOCAL_FUNC(_initfrom)
#define LOCAL_initwith    LOCAL_FUNC(_initwith)
#define LOCAL_bool        LOCAL_FUNC(_bool)
#define LOCAL_printcrepr  LOCAL_FUNC(_printcrepr)
#define LOCAL_printdrepr  LOCAL_FUNC(_printdrepr)
#define LOCAL_compare     LOCAL_FUNC(_compare)
#define LOCAL_int32       LOCAL_FUNC(_int32)
#define LOCAL_int64       LOCAL_FUNC(_int64)
#define LOCAL_double      LOCAL_FUNC(_double)
#define LOCAL_int         LOCAL_FUNC(_int)
#define LOCAL_inv         LOCAL_FUNC(_inv)
#define LOCAL_pos         LOCAL_FUNC(_pos)
#define LOCAL_neg         LOCAL_FUNC(_neg)
#define LOCAL_add         LOCAL_FUNC(_add)
#define LOCAL_sub         LOCAL_FUNC(_sub)
#define LOCAL_mul         LOCAL_FUNC(_mul)
#define LOCAL_div         LOCAL_FUNC(_div)
#define LOCAL_mod         LOCAL_FUNC(_mod)
#define LOCAL_shl         LOCAL_FUNC(_shl)
#define LOCAL_shr         LOCAL_FUNC(_shr)
#define LOCAL_and         LOCAL_FUNC(_and)
#define LOCAL_or          LOCAL_FUNC(_or)
#define LOCAL_xor         LOCAL_FUNC(_xor)
#define LOCAL_pow         LOCAL_FUNC(_pow)
#define LOCAL_inc         LOCAL_FUNC(_inc)
#define LOCAL_dec         LOCAL_FUNC(_dec)
#define LOCAL_inplace_add LOCAL_FUNC(_inplace_add)
#define LOCAL_inplace_sub LOCAL_FUNC(_inplace_sub)
#define LOCAL_inplace_mul LOCAL_FUNC(_inplace_mul)
#define LOCAL_inplace_div LOCAL_FUNC(_inplace_div)
#define LOCAL_inplace_mod LOCAL_FUNC(_inplace_mod)
#define LOCAL_inplace_shl LOCAL_FUNC(_inplace_shl)
#define LOCAL_inplace_shr LOCAL_FUNC(_inplace_shr)
#define LOCAL_inplace_and LOCAL_FUNC(_inplace_and)
#define LOCAL_inplace_or  LOCAL_FUNC(_inplace_or)
#define LOCAL_inplace_xor LOCAL_FUNC(_inplace_xor)
#define LOCAL_inplace_pow LOCAL_FUNC(_inplace_pow)


#if LOCAL_sizeof_intN_t == 16
#ifdef LOCAL_UNSIGNED
#define LOCAL_int128_foo(foo) __hybrid_uint128_##foo
#else /* LOCAL_UNSIGNED */
#define LOCAL_int128_foo(foo) __hybrid_int128_##foo
#endif /* !LOCAL_UNSIGNED */
#define LOCAL_operator(op, op128) op128
#else /* LOCAL_sizeof_intN_t == 16 */
#define LOCAL_operator(op, op128) op
#endif /* LOCAL_sizeof_intN_t != 16 */
#define LOCAL_op_iszero(v)    LOCAL_operator(v == 0, LOCAL_int128_foo(iszero)(v))
#define LOCAL_op_cmp_lo(a, b) LOCAL_operator(a < b, LOCAL_int128_foo(lo128)(a, b))
#define LOCAL_op_inv(v)       LOCAL_operator(v = ~v, LOCAL_int128_foo(inv)(v))
#define LOCAL_op_neg(v)       LOCAL_operator(v = -v, LOCAL_int128_foo(neg)(v))
#define LOCAL_op_add(r, a, b) LOCAL_operator(r = a + b, (r = a, LOCAL_int128_foo(add128)(r, b)))
#define LOCAL_op_sub(r, a, b) LOCAL_operator(r = a - b, (r = a, LOCAL_int128_foo(sub128)(r, b)))
#define LOCAL_op_mul(r, a, b) LOCAL_operator(r = a * b, (r = a, LOCAL_int128_foo(mul128)(r, b)))
#define LOCAL_op_div(r, a, b) LOCAL_operator(r = a / b, (r = a, LOCAL_int128_foo(div128)(r, b)))
#define LOCAL_op_mod(r, a, b) LOCAL_operator(r = a % b, (r = a, LOCAL_int128_foo(mod128)(r, b)))
#define LOCAL_op_shl(r, a, b) LOCAL_operator(r = a << b, (r = a, LOCAL_int128_foo(shl)(r, b)))
#define LOCAL_op_shr(r, a, b) LOCAL_operator(r = a >> b, (r = a, LOCAL_int128_foo(shr)(r, b)))
#define LOCAL_op_and(r, a, b) LOCAL_operator(r = a & b, (r = a, LOCAL_int128_foo(and128)(r, b)))
#define LOCAL_op_or(r, a, b)  LOCAL_operator(r = a | b, (r = a, LOCAL_int128_foo(or128)(r, b)))
#define LOCAL_op_xor(r, a, b) LOCAL_operator(r = a ^ b, (r = a, LOCAL_int128_foo(xor128)(r, b)))
#define LOCAL_op_inc(v)       LOCAL_operator(++v, LOCAL_int128_foo(inc)(v))
#define LOCAL_op_dec(v)       LOCAL_operator(--v, LOCAL_int128_foo(dec)(v))


PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_initfrom(CType *tp_self, void *self, DeeObject *value) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(value, &cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		LOCAL_UNALIGNED_SET(self, cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "value" });]]]*/
#ifndef DEFINED_kwlist__value
#define DEFINED_kwlist__value
PRIVATE DEFINE_KWLIST(kwlist__value, { KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
#endif /* !DEFINED_kwlist__value */
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_initwith(CType *tp_self, void *self,
               size_t argc, DeeObject *const *argv,
               DeeObject *kw) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          LOCAL_UNPduN ":" LOCAL_STR_intN_t,
	                          &cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		LOCAL_UNALIGNED_SET(self, cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_bool(CType *tp_self, void const *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, goto err);
	return LOCAL_op_iszero(cvalue) ? 0 : 1;
err:
	return -1;
}



#undef LOCAL_printdrepr
#define LOCAL_printdrepr LOCAL_printcrepr
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
LOCAL_printcrepr(CType *tp_self, void const *self,
                 Dee_formatprinter_t printer, void *arg) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return -1);
	return DeeFormat_Printf(printer, arg, LOCAL_PRFduN, cvalue);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_compare(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	if (LOCAL_op_cmp_lo(lhs_cvalue, rhs_cvalue))
		return Dee_COMPARE_LO;
	if (LOCAL_op_cmp_lo(rhs_cvalue, lhs_cvalue))
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
LOCAL_int32(CType *tp_self, void const *self, int32_t *result) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return Dee_INT_ERROR);
#if LOCAL_sizeof_intN_t <= 4
	*result = (int32_t)cvalue;
#elif !defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t != 16
	if (cvalue != (LOCAL_intN_t)(int32_t)cvalue) {
		DeeRT_ErrIntegerOverflowS64(cvalue, INT32_MIN, INT32_MAX);
		return Dee_INT_ERROR;
	}
	*result = (int32_t)cvalue;
#elif defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t != 16
	if (cvalue != (LOCAL_intN_t)(uint32_t)cvalue) {
		DeeRT_ErrIntegerOverflowU64(cvalue, UINT32_MAX);
		return Dee_INT_ERROR;
	}
	*result = (int32_t)(uint32_t)cvalue;
#elif !defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t == 16
	if (!__hybrid_int128_is32bit(cvalue)) {
		Dee_int128_t minval, maxval;
		__hybrid_int128_set32(minval, INT32_MIN);
		__hybrid_int128_set32(maxval, INT32_MAX);
		DeeRT_ErrIntegerOverflowS128(cvalue, minval, maxval);
		return Dee_INT_ERROR;
	}
	*result = __hybrid_int128_get32(cvalue);
#elif defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t == 16
	if (!__hybrid_uint128_is32bit(cvalue)) {
		Dee_uint128_t maxval;
		__hybrid_uint128_set32(maxval, UINT32_MAX);
		DeeRT_ErrIntegerOverflowU128(cvalue, maxval);
		return Dee_INT_ERROR;
	}
	*result = (int32_t)__hybrid_uint128_get32(cvalue);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#ifdef LOCAL_UNSIGNED
	return Dee_INT_UNSIGNED;
#else /* LOCAL_UNSIGNED */
	return Dee_INT_SIGNED;
#endif /* !LOCAL_UNSIGNED */
}


PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
LOCAL_int64(CType *tp_self, void const *self, int64_t *result) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return Dee_INT_ERROR);
#if LOCAL_sizeof_intN_t <= 8
	*result = (int64_t)cvalue;
#elif !defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t == 16
	if (!__hybrid_int128_is64bit(cvalue)) {
		Dee_int128_t minval, maxval;
		__hybrid_int128_set64(minval, INT64_MIN);
		__hybrid_int128_set64(maxval, INT64_MAX);
		DeeRT_ErrIntegerOverflowS128(cvalue, minval, maxval);
		return Dee_INT_ERROR;
	}
	*result = __hybrid_int128_get64(cvalue);
#elif defined(LOCAL_UNSIGNED) && LOCAL_sizeof_intN_t == 16
	if (!__hybrid_uint128_is64bit(cvalue)) {
		Dee_uint128_t maxval;
		__hybrid_uint128_set64(maxval, UINT64_MAX);
		DeeRT_ErrIntegerOverflowU128(cvalue, maxval);
		return Dee_INT_ERROR;
	}
	*result = (int64_t)__hybrid_uint128_get64(cvalue);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#ifdef LOCAL_UNSIGNED
	return Dee_INT_UNSIGNED;
#else /* LOCAL_UNSIGNED */
	return Dee_INT_SIGNED;
#endif /* !LOCAL_UNSIGNED */
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_double(CType *tp_self, void const *self, double *result) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return Dee_INT_ERROR);
#if LOCAL_sizeof_intN_t < 16
	*result = (double)cvalue;
#elif defined(LOCAL_UNSIGNED)
	*result = (double)__hybrid_uint128_get64(cvalue);
#else /* ... */
	*result = (double)__hybrid_int128_get64(cvalue);
#endif /* !... */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_int(CType *tp_self, void const *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return NULL);
	return LOCAL_DeeInt_New(cvalue);
}

PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
LOCAL_inv(CType *tp_self, void const *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return NULL);
	LOCAL_op_inv(cvalue);
	return LOCAL_CInt_New(cvalue);
}

PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
LOCAL_pos(CType *tp_self, void const *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return NULL);
	return LOCAL_CInt_New(cvalue);
}

#ifdef LOCAL_UNSIGNED
#undef LOCAL_neg
#define LOCAL_neg unsupported__neg
#else /* LOCAL_UNSIGNED */
PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
LOCAL_neg(CType *tp_self, void const *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
	}, return NULL);
	LOCAL_op_neg(cvalue);
	return LOCAL_CInt_New(cvalue);
}
#endif /* !LOCAL_UNSIGNED */

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_add(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_add(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_sub(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_sub(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_mul(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_mul(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}


PRIVATE WUNUSED ATTR_COLD int DCALL
LOCAL_FUNC(divide_by_zero)(LOCAL_intN_t lhs_cvalue,
                           LOCAL_intN_t rhs_cvalue) {
	struct Dee_variant lhs_var;
	struct Dee_variant rhs_var;
	LOCAL_Dee_variant_init_intN(&lhs_var, lhs_cvalue);
	LOCAL_Dee_variant_init_intN(&rhs_var, rhs_cvalue);
	return DeeRT_ErrDivideByZeroEx(&lhs_var, &rhs_var);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_div(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	if unlikely(LOCAL_op_iszero(rhs_cvalue))
		goto err_rhs_zero;
	LOCAL_op_div(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err_rhs_zero:
	LOCAL_FUNC(divide_by_zero)(lhs_cvalue, rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_mod(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	if unlikely(LOCAL_op_iszero(rhs_cvalue))
		goto err_rhs_zero;
	LOCAL_op_mod(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err_rhs_zero:
	LOCAL_FUNC(divide_by_zero)(lhs_cvalue, rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_shl(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	shift_t shift;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (DeeObject_AsUIntX(rhs, &shift))
		goto err;
	LOCAL_op_shl(res_cvalue, lhs_cvalue, shift);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_shr(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	shift_t shift;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (DeeObject_AsUIntX(rhs, &shift))
		goto err;
	LOCAL_op_shr(res_cvalue, lhs_cvalue, shift);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_and(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_and(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_or(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_or(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_xor(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue;
	LOCAL_intN_t rhs_cvalue;
	LOCAL_CInt_New_t res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(lhs);
	}, goto err);
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	LOCAL_op_xor(res_cvalue, lhs_cvalue, rhs_cvalue);
	return LOCAL_CInt_New(res_cvalue);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_inc(CType *tp_self, void *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_inc(cvalue);
		LOCAL_UNALIGNED_SET(self, cvalue);
	}, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dec(CType *tp_self, void *self) {
	LOCAL_intN_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_dec(cvalue);
		LOCAL_UNALIGNED_SET(self, cvalue);
	}, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_add(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_add(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_sub(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_sub(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_mul(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_mul(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_div(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	if unlikely(LOCAL_op_iszero(rhs_cvalue))
		goto err_rhs_zero;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_div(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err_rhs_zero:
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
	}, goto err);
	LOCAL_FUNC(divide_by_zero)(lhs_cvalue, rhs_cvalue);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_mod(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	if unlikely(LOCAL_op_iszero(rhs_cvalue))
		goto err_rhs_zero;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_mod(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err_rhs_zero:
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
	}, goto err);
	LOCAL_FUNC(divide_by_zero)(lhs_cvalue, rhs_cvalue);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_shl(CType *tp_self, void *self, DeeObject *rhs) {
	shift_t shift;
	LOCAL_intN_t lhs_cvalue, res_cvalue;
	(void)tp_self;
	if (DeeObject_AsUIntX(rhs, &shift))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_shl(res_cvalue, lhs_cvalue, shift);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_shr(CType *tp_self, void *self, DeeObject *rhs) {
	shift_t shift;
	LOCAL_intN_t lhs_cvalue, res_cvalue;
	(void)tp_self;
	if (DeeObject_AsUIntX(rhs, &shift))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_shr(res_cvalue, lhs_cvalue, shift);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_and(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_and(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_or(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_or(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_xor(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_intN_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsIntN(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		lhs_cvalue = LOCAL_UNALIGNED_GET(self);
		LOCAL_op_xor(res_cvalue, lhs_cvalue, rhs_cvalue);
		LOCAL_UNALIGNED_SET(self, res_cvalue);
	}, goto err);
	return 0;
err:
	return -1;
}




#undef LOCAL_pow
#define LOCAL_pow unsupported__pow /* TODO */
#undef LOCAL_inplace_pow
#define LOCAL_inplace_pow unsupported__inplace_pow /* TODO */


INTERN struct ctype_operators Dee_tpconst LOCAL_operators = {
	/* .co_initfrom    = */ &LOCAL_initfrom,
	/* .co_initwith    = */ &LOCAL_initwith,
	/* .co_bool        = */ &LOCAL_bool,
	/* .co_printcrepr  = */ &LOCAL_printcrepr,
	/* .co_printdrepr  = */ &LOCAL_printdrepr,
	/* .co_compare     = */ &LOCAL_compare,
	/* .co_int32       = */ &LOCAL_int32,
	/* .co_int64       = */ &LOCAL_int64,
	/* .co_double      = */ &LOCAL_double,
	/* .co_int         = */ &LOCAL_int,
	/* .co_inv         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&LOCAL_inv,
	/* .co_pos         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&LOCAL_pos,
	/* .co_neg         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&LOCAL_neg,
	/* .co_add         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_add,
	/* .co_sub         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_sub,
	/* .co_mul         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_mul,
	/* .co_div         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_div,
	/* .co_mod         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_mod,
	/* .co_shl         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_shl,
	/* .co_shr         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_shr,
	/* .co_and         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_and,
	/* .co_or          = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_or,
	/* .co_xor         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_xor,
	/* .co_pow         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&LOCAL_pow,
	/* .co_inc         = */ &LOCAL_inc,
	/* .co_dec         = */ &LOCAL_dec,
	/* .co_inplace_add = */ &LOCAL_inplace_add,
	/* .co_inplace_sub = */ &LOCAL_inplace_sub,
	/* .co_inplace_mul = */ &LOCAL_inplace_mul,
	/* .co_inplace_div = */ &LOCAL_inplace_div,
	/* .co_inplace_mod = */ &LOCAL_inplace_mod,
	/* .co_inplace_shl = */ &LOCAL_inplace_shl,
	/* .co_inplace_shr = */ &LOCAL_inplace_shr,
	/* .co_inplace_and = */ &LOCAL_inplace_and,
	/* .co_inplace_or  = */ &LOCAL_inplace_or,
	/* .co_inplace_xor = */ &LOCAL_inplace_xor,
	/* .co_inplace_pow = */ &LOCAL_inplace_pow,
};

#undef LOCAL_FUNC
#undef LOCAL_initfrom
#undef LOCAL_initwith
#undef LOCAL_bool
#undef LOCAL_printcrepr
#undef LOCAL_printdrepr
#undef LOCAL_compare
#undef LOCAL_int32
#undef LOCAL_int64
#undef LOCAL_double
#undef LOCAL_int
#undef LOCAL_inv
#undef LOCAL_pos
#undef LOCAL_neg
#undef LOCAL_add
#undef LOCAL_sub
#undef LOCAL_mul
#undef LOCAL_div
#undef LOCAL_mod
#undef LOCAL_shl
#undef LOCAL_shr
#undef LOCAL_and
#undef LOCAL_or
#undef LOCAL_xor
#undef LOCAL_pow
#undef LOCAL_inc
#undef LOCAL_dec
#undef LOCAL_inplace_add
#undef LOCAL_inplace_sub
#undef LOCAL_inplace_mul
#undef LOCAL_inplace_div
#undef LOCAL_inplace_mod
#undef LOCAL_inplace_shl
#undef LOCAL_inplace_shr
#undef LOCAL_inplace_and
#undef LOCAL_inplace_or
#undef LOCAL_inplace_xor
#undef LOCAL_inplace_pow

#undef LOCAL_int128_foo
#undef LOCAL_operator
#undef LOCAL_op_iszero
#undef LOCAL_op_cmp_lo
#undef LOCAL_op_inv
#undef LOCAL_op_neg
#undef LOCAL_op_add
#undef LOCAL_op_sub
#undef LOCAL_op_mul
#undef LOCAL_op_div
#undef LOCAL_op_mod
#undef LOCAL_op_shl
#undef LOCAL_op_shr
#undef LOCAL_op_and
#undef LOCAL_op_or
#undef LOCAL_op_xor

#undef LOCAL_operators
#undef LOCAL_intN_t
#undef LOCAL_signed_intN_t
#undef LOCAL_unsigned_intN_t
#undef LOCAL_STR_intN_t
#undef LOCAL_UNPduN
#undef LOCAL_PRFduN
#undef LOCAL_sizeof_intN_t
#undef LOCAL_UNSIGNED
#undef _LOCAL_UNALIGNED_GET
#undef _LOCAL_UNALIGNED_SET
#undef LOCAL_UNALIGNED_GET
#undef LOCAL_UNALIGNED_SET
#undef LOCAL_DeeObject_AsIntN
#undef LOCAL_DeeInt_New
#undef LOCAL_BSWAP
#undef LOCAL_CInt_New
#undef LOCAL_CInt_New_t
#undef LOCAL_Dee_variant_init_intN

DECL_END


#undef DEFINE_cint8_operators
#undef DEFINE_cint16_operators
#undef DEFINE_cint32_operators
#undef DEFINE_cint64_operators
#undef DEFINE_cint128_operators
#undef DEFINE_cuint8_operators
#undef DEFINE_cuint16_operators
#undef DEFINE_cuint32_operators
#undef DEFINE_cuint64_operators
#undef DEFINE_cuint128_operators
#undef DEFINE_cint16_bswap_operators
#undef DEFINE_cint32_bswap_operators
#undef DEFINE_cint64_bswap_operators
#undef DEFINE_cint128_bswap_operators
#undef DEFINE_cuint16_bswap_operators
#undef DEFINE_cuint32_bswap_operators
#undef DEFINE_cuint64_bswap_operators
#undef DEFINE_cuint128_bswap_operators
