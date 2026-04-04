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
#define DEFINE_cfloat_operators
//#define DEFINE_cdouble_operators
//#define DEFINE_cldouble_operators
#include "ctypes-operators.c"
#endif /* __INTELLISENSE__ */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStructKw */
#include <deemon/float.h>           /* DeeFloat_LPrint, DeeFloat_Print, Dee_FLOAT_PRINT_FNORMAL */
#include <deemon/int.h>             /* DeeInt_NewDouble, Dee_INT_SIGNED */
#include <deemon/object.h>          /* DREF, DeeObject, Dee_COMPARE_*, Dee_formatprinter_t, Dee_ssize_t */
#include <deemon/system-features.h> /* memcpy */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* int32_t, int64_t */

#if (defined(DEFINE_cfloat_operators) +         \
     defined(DEFINE_cdouble_operators) +        \
     defined(DEFINE_cldouble_operators)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_cfloat_operators
#define LOCAL_operators         cfloat_operators
#define LOCAL_float_t           CTYPES_float
#define LOCAL_STR_float_t       "float"
#define LOCAL_DeeObject_AsFloat DeeObject_AsCFloat
#define LOCAL_CFloat_New        CDouble_New               /* Float-to-double promotion */
#define LOCAL_CFloat_New_t      CTYPES_double /* Float-to-double promotion */
#define LOCAL_float_IS_float
#elif defined(DEFINE_cdouble_operators)
#define LOCAL_operators         cdouble_operators
#define LOCAL_float_t           CTYPES_double
#define LOCAL_STR_float_t       "double"
#define LOCAL_DeeObject_AsFloat DeeObject_AsCDouble
#define LOCAL_CFloat_New        CDouble_New
#define LOCAL_CFloat_New_t      CTYPES_double
#define LOCAL_float_IS_double
#elif defined(DEFINE_cldouble_operators)
#define LOCAL_operators         cldouble_operators
#define LOCAL_float_t           CTYPES_ldouble
#define LOCAL_STR_float_t       "long double"
#define LOCAL_DeeObject_AsFloat DeeObject_AsCLDouble
#define LOCAL_CFloat_New        CLDouble_New
#define LOCAL_CFloat_New_t      CTYPES_ldouble
#define LOCAL_float_IS_ldouble
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

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


PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_initfrom(CType *tp_self, void *self, DeeObject *value) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	if (LOCAL_DeeObject_AsFloat(value, &cvalue))
		goto err;
	CTYPES_FAULTPROTECT({
		memcpy(self, &cvalue, sizeof(LOCAL_float_t));
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
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:" LOCAL_STR_float_t, &value))
		goto err;
	return LOCAL_initfrom(tp_self, self, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_bool(CType *tp_self, void const *self) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return -1);
	return cvalue ? 0 : 1;
}


#undef LOCAL_printdrepr
#define LOCAL_printdrepr LOCAL_printcrepr
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
LOCAL_printcrepr(CType *tp_self, void const *self,
                 Dee_formatprinter_t printer, void *arg) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return -1);
#ifdef LOCAL_float_IS_ldouble
	return DeeFloat_LPrint(cvalue, printer, arg, 0, 0, Dee_FLOAT_PRINT_FNORMAL);
#else /* LOCAL_float_IS_ldouble */
	return DeeFloat_Print(cvalue, printer, arg, 0, 0, Dee_FLOAT_PRINT_FNORMAL);
#endif /* !LOCAL_float_IS_ldouble */
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_compare(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue;
	LOCAL_float_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, lhs, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	if (lhs_cvalue < rhs_cvalue)
		return Dee_COMPARE_LO;
	if (lhs_cvalue > rhs_cvalue)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
LOCAL_int32(CType *tp_self, void const *self, int32_t *result) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return -1);
	*result = (int32_t)cvalue;
	return Dee_INT_SIGNED;
}


PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
LOCAL_int64(CType *tp_self, void const *self, int64_t *result) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return -1);
	*result = (int64_t)cvalue;
	return Dee_INT_SIGNED;
}


PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_double(CType *tp_self, void const *self, double *result) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return -1);
	*result = (double)cvalue;
	return Dee_INT_SIGNED;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_int(CType *tp_self, void const *self) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return NULL);
	return DeeInt_NewDouble((double)cvalue);
}

#undef LOCAL_inv
#define LOCAL_inv unsupported__inv

PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
LOCAL_pos(CType *tp_self, void const *self) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return NULL);
	return LOCAL_CFloat_New(cvalue);
}

PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL
LOCAL_neg(CType *tp_self, void const *self) {
	LOCAL_float_t cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&cvalue, self, sizeof(LOCAL_float_t));
	}, return NULL);
	return LOCAL_CFloat_New(-cvalue);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_add(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue;
	LOCAL_float_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, lhs, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	return LOCAL_CFloat_New(lhs_cvalue + rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_sub(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue;
	LOCAL_float_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, lhs, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	return LOCAL_CFloat_New(lhs_cvalue - rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_mul(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue;
	LOCAL_float_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, lhs, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	return LOCAL_CFloat_New(lhs_cvalue * rhs_cvalue);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF CObject *DCALL
LOCAL_div(CType *tp_self, void const *lhs, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue;
	LOCAL_float_t rhs_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, lhs, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	return LOCAL_CFloat_New(lhs_cvalue / rhs_cvalue);
err:
	return NULL;
}

#undef LOCAL_mod
#define LOCAL_mod unsupported__mod
#undef LOCAL_shl
#define LOCAL_shl unsupported__shl
#undef LOCAL_shr
#define LOCAL_shr unsupported__shr
#undef LOCAL_and
#define LOCAL_and unsupported__and
#undef LOCAL_or
#define LOCAL_or unsupported__or
#undef LOCAL_xor
#define LOCAL_xor unsupported__xor
#undef LOCAL_inc
#define LOCAL_inc unsupported__inc
#undef LOCAL_dec
#define LOCAL_dec unsupported__dec

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_add(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, self, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	res_cvalue = lhs_cvalue + rhs_cvalue;
	CTYPES_FAULTPROTECT({
		memcpy(self, &res_cvalue, sizeof(LOCAL_float_t));
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_sub(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, self, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	res_cvalue = lhs_cvalue - rhs_cvalue;
	CTYPES_FAULTPROTECT({
		memcpy(self, &res_cvalue, sizeof(LOCAL_float_t));
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_mul(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, self, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	res_cvalue = lhs_cvalue * rhs_cvalue;
	CTYPES_FAULTPROTECT({
		memcpy(self, &res_cvalue, sizeof(LOCAL_float_t));
	}, goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_inplace_div(CType *tp_self, void *self, DeeObject *rhs) {
	LOCAL_float_t lhs_cvalue, rhs_cvalue, res_cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT({
		memcpy(&lhs_cvalue, self, sizeof(LOCAL_float_t));
	}, goto err);
	if (LOCAL_DeeObject_AsFloat(rhs, &rhs_cvalue))
		goto err;
	res_cvalue = lhs_cvalue / rhs_cvalue;
	CTYPES_FAULTPROTECT({
		memcpy(self, &res_cvalue, sizeof(LOCAL_float_t));
	}, goto err);
	return 0;
err:
	return -1;
}

#undef LOCAL_inplace_mod
#define LOCAL_inplace_mod unsupported__inplace_mod
#undef LOCAL_inplace_shl
#define LOCAL_inplace_shl unsupported__inplace_shl
#undef LOCAL_inplace_shr
#define LOCAL_inplace_shr unsupported__inplace_shr
#undef LOCAL_inplace_and
#define LOCAL_inplace_and unsupported__inplace_and
#undef LOCAL_inplace_or
#define LOCAL_inplace_or unsupported__inplace_or
#undef LOCAL_inplace_xor
#define LOCAL_inplace_xor unsupported__inplace_xor

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

#undef LOCAL_operators
#undef LOCAL_float_t
#undef LOCAL_STR_float_t
#undef LOCAL_DeeObject_AsFloat
#undef LOCAL_CFloat_New
#undef LOCAL_CFloat_New_t
#undef LOCAL_float_IS_float
#undef LOCAL_float_IS_double
#undef LOCAL_float_IS_ldouble

DECL_END

#undef DEFINE_cfloat_operators
#undef DEFINE_cdouble_operators
#undef DEFINE_cldouble_operators
