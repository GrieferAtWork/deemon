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
#ifndef GUARD_DEX_CTYPES_CTYPES_OPERATORS_C
#define GUARD_DEX_CTYPES_CTYPES_OPERATORS_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#ifdef CONFIG_EXPERIMENTAL_REWORKED_CTYPES
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>

DECL_BEGIN

#define UNSUPPORTED_OPERATOR_FMT(name) "Unsupported ctypes operator " name ": %k"

/* Generic, unsupported types */
PRIVATE WUNUSED NONNULL((1)) int DCALL
unsupported__bool(CType *tp_self, void const *self) {
	(void)self;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("bool"),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__compare(CType *tp_self, void const *lhs, DeeObject *rhs) {
	(void)lhs;
	(void)rhs;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("<=>"),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
unsupported__int32(CType *tp_self, void const *self, int32_t *result) {
	(void)self;
	(void)result;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("int"),
	                       tp_self);
}

#if 1
#define unsupported__int64 (*(int (DCALL *)(CType *, void const *, int64_t *))&unsupported__int32)
#else
PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
unsupported__int64(CType *tp_self, void const *self, int64_t *result) {
	return unsupported__int32(tp_self, self, (int32_t *)result);
}
#endif

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__double(CType *tp_self, void const *self, double *result) {
	(void)self;
	(void)result;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("float"),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unsupported__int(CType *tp_self, void const *self) {
	unsupported__int32(tp_self, self, NULL);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unsupported__inv(CType *tp_self, void const *self) {
	(void)self;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("inv"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unsupported__pos(CType *tp_self, void const *self) {
	(void)self;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("pos"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unsupported__neg(CType *tp_self, void const *self) {
	(void)self;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("neg"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__add(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("add"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__sub(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("sub"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__mul(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("*"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__div(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("/"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__mod(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("%"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__shl(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("<<"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__shr(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT(">>"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__and(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("&"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__or(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("|"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__xor(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("^"),
	                tp_self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
unsupported__pow(CType *tp_self, void const *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	DeeError_Throwf(&DeeError_TypeError,
	                UNSUPPORTED_OPERATOR_FMT("**"),
	                tp_self);
	return NULL;
}

	/* Inplace operators */
PRIVATE WUNUSED NONNULL((1)) int DCALL
unsupported__inc(CType *tp_self, void *self) {
	(void)self;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("++"),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
unsupported__dec(CType *tp_self, void *self) {
	(void)self;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("--"),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_add(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("+="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_sub(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("-="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_mul(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("*="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_div(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("/="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_mod(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("%="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_shl(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("<<="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_shr(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT(">>="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_and(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("&="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_or(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("|="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_xor(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("^="),
	                       tp_self);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
unsupported__inplace_pow(CType *tp_self, void *self, DeeObject *some_object) {
	(void)self;
	(void)some_object;
	return DeeError_Throwf(&DeeError_TypeError,
	                       UNSUPPORTED_OPERATOR_FMT("**="),
	                       tp_self);
}





/************************************************************************/
/* Void type                                                            */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
void_initfrom(CType *tp_self, void *self, DeeObject *value) {
	(void)tp_self;
	(void)self;
	(void)value;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
void_initwith(CType *tp_self, void *self,
              size_t argc, DeeObject *const *argv,
              DeeObject *kw) {
	(void)tp_self;
	(void)argc;
	(void)argv;
	(void)kw;
	(void)self;
	return 0;
}

#define void_printdrepr void_printcrepr
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
void_printcrepr(CType *tp_self, void const *self,
                Dee_formatprinter_t printer, void *arg) {
	(void)tp_self;
	(void)self;
	return DeeFormat_PRINT(printer, arg, "void");
}

#define void_bool        unsupported__bool
#define void_compare     unsupported__compare
#define void_int32       unsupported__int32
#define void_int64       unsupported__int64
#define void_double      unsupported__double
#define void_int         unsupported__int
#define void_inv         unsupported__inv
#define void_pos         unsupported__pos
#define void_neg         unsupported__neg
#define void_add         unsupported__add
#define void_sub         unsupported__sub
#define void_mul         unsupported__mul
#define void_div         unsupported__div
#define void_mod         unsupported__mod
#define void_shl         unsupported__shl
#define void_shr         unsupported__shr
#define void_and         unsupported__and
#define void_or          unsupported__or
#define void_xor         unsupported__xor
#define void_pow         unsupported__pow
#define void_inc         unsupported__inc
#define void_dec         unsupported__dec
#define void_inplace_add unsupported__inplace_add
#define void_inplace_sub unsupported__inplace_sub
#define void_inplace_mul unsupported__inplace_mul
#define void_inplace_div unsupported__inplace_div
#define void_inplace_mod unsupported__inplace_mod
#define void_inplace_shl unsupported__inplace_shl
#define void_inplace_shr unsupported__inplace_shr
#define void_inplace_and unsupported__inplace_and
#define void_inplace_or  unsupported__inplace_or
#define void_inplace_xor unsupported__inplace_xor
#define void_inplace_pow unsupported__inplace_pow

INTERN struct ctype_operators Dee_tpconst cvoid_operators = {
	/* .co_initfrom    = */ &void_initfrom,
	/* .co_initwith    = */ &void_initwith,
	/* .co_bool        = */ &void_bool,
	/* .co_printcrepr  = */ &void_printcrepr,
	/* .co_printdrepr  = */ &void_printdrepr,
	/* .co_compare     = */ &void_compare,
	/* .co_int32       = */ &void_int32,
	/* .co_int64       = */ &void_int64,
	/* .co_double      = */ &void_double,
	/* .co_int         = */ &void_int,
	/* .co_inv         = */ &void_inv,
	/* .co_pos         = */ &void_pos,
	/* .co_neg         = */ &void_neg,
	/* .co_add         = */ &void_add,
	/* .co_sub         = */ &void_sub,
	/* .co_mul         = */ &void_mul,
	/* .co_div         = */ &void_div,
	/* .co_mod         = */ &void_mod,
	/* .co_shl         = */ &void_shl,
	/* .co_shr         = */ &void_shr,
	/* .co_and         = */ &void_and,
	/* .co_or          = */ &void_or,
	/* .co_xor         = */ &void_xor,
	/* .co_pow         = */ &void_pow,
	/* .co_inc         = */ &void_inc,
	/* .co_dec         = */ &void_dec,
	/* .co_inplace_add = */ &void_inplace_add,
	/* .co_inplace_sub = */ &void_inplace_sub,
	/* .co_inplace_mul = */ &void_inplace_mul,
	/* .co_inplace_div = */ &void_inplace_div,
	/* .co_inplace_mod = */ &void_inplace_mod,
	/* .co_inplace_shl = */ &void_inplace_shl,
	/* .co_inplace_shr = */ &void_inplace_shr,
	/* .co_inplace_and = */ &void_inplace_and,
	/* .co_inplace_or  = */ &void_inplace_or,
	/* .co_inplace_xor = */ &void_inplace_xor,
	/* .co_inplace_pow = */ &void_inplace_pow,
};



/* Integer types */
#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_cint8_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint16_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint32_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint64_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint128_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint8_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint16_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint32_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint64_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint128_operators
#include "ctypes-operators-integral.c.inl"

#define DEFINE_cint16_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint32_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint64_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cint128_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint16_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint32_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint64_bswap_operators
#include "ctypes-operators-integral.c.inl"
#define DEFINE_cuint128_bswap_operators
#include "ctypes-operators-integral.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

/* Floating point types */
INTERN struct ctype_operators Dee_tpconst cfloat_operators = { /* TODO */ };
INTERN struct ctype_operators Dee_tpconst cdouble_operators = { /* TODO */ };
INTERN struct ctype_operators Dee_tpconst cldouble_operators = { /* TODO */ };

/* Operators for pointer types */
INTERN struct ctype_operators Dee_tpconst cpointer_operators = { /* TODO */ };

/* "tp_self" is "CLValueType"; forward operator to "tp_self->clt_orig" and "*(void **)self"
 * This is only really needed when user-defined structure types contain lvalue fields:
 * >> local a = (union { .x = int })(x: 42);
 * >> local b = (union { .x = int.ptr, .y = int.lvalue })(y: a.x);
 * >> assert a.x == 42;
 * >> assert b.x == a.x.ptr;
 * >> assert b.y == 42;
 * >> b.y = 99;
 * >> assert a.x == 99; */
INTERN struct ctype_operators Dee_tpconst clvalue_operators = { /* TODO */ };

/* Operators for user-defined structure types */
INTERN struct ctype_operators Dee_tpconst cstruct_operators = { /* TODO */ };

/* Operators for array types */
INTERN struct ctype_operators Dee_tpconst carray0_operators = { /* TODO */ }; /* when "cat_orig->ct_sizeof == 0" */
INTERN struct ctype_operators Dee_tpconst carrayN_operators = { /* TODO */ }; /* when "cat_orig->ct_sizeof != 0" */



DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */

#endif /* !GUARD_DEX_CTYPES_CTYPES_OPERATORS_C */
