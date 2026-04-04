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
#include <deemon/arg.h>
#include <deemon/int.h>

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AsCFloat)(DeeObject *__restrict self,
                           CONFIG_CTYPES_FLOAT_TYPE *__restrict p_result) {
	double double_value;
	int result = DeeObject_AsDouble(self, &double_value);
	if likely(result == 0)
		*p_result = (CONFIG_CTYPES_FLOAT_TYPE)double_value;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AsCLDouble)(DeeObject *__restrict self,
                             CONFIG_CTYPES_LDOUBLE_TYPE *__restrict p_result) {
	double double_value;
	DeeTypeObject *tp = Dee_TYPE(self);
	if (tp == CType_AsType(&CLDouble_Type)) {
		byte_t *data = CObject_Data(self);
		memcpy(p_result, data, sizeof(CONFIG_CTYPES_LDOUBLE_TYPE));
		return 0;
	} else if (Type_IsCLValueType(tp)) {
		CType *base = CLValueType_PointedToType(Type_AsCLValueType(tp));
		if (base == &CLDouble_Type) {
			byte_t *data = Object_AsCLValue(self)->cl_value.pbyte;
			CTYPES_FAULTPROTECT({
				memcpy(p_result, data, sizeof(CONFIG_CTYPES_LDOUBLE_TYPE));
			}, goto err);
			return 0;
		}
	}
	if (DeeObject_AsDouble(self, &double_value))
		goto err;
	*p_result = (CONFIG_CTYPES_FLOAT_TYPE)double_value;
	return 0;
err:
	return -1;
}


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

#define DEFINE_cfloat_operators
#include "ctypes-operators-float.c.inl"
#define DEFINE_cdouble_operators
#include "ctypes-operators-float.c.inl"
#define DEFINE_cldouble_operators
#include "ctypes-operators-float.c.inl"

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

/* Operators for pointer types */
PRIVATE WUNUSED NONNULL((1)) int DCALL
pointer_initfrom(CPointerType *tp_self, union pointer *self, DeeObject *value) {
	union pointer cvalue;
	if (DeeObject_AsPointer(value, CPointerType_PointedToType(tp_self), &cvalue))
		goto err;
	CTYPES_FAULTPROTECT(pointer_set(self, cvalue.ptr), goto err);
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
pointer_initwith(CPointerType *tp_self, union pointer *self,
                 size_t argc, DeeObject *const *argv,
                 DeeObject *kw) {
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:Pointer", &value))
		goto err;
	return pointer_initfrom(tp_self, self, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
pointer_bool(CPointerType *tp_self, union pointer const *self) {
	union pointer cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	return cvalue.ptr ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_compare(CPointerType *tp_self, union pointer const *lhs, DeeObject *rhs) {
	union pointer lhs_cvalue;
	union pointer rhs_cvalue;
	if (DeeObject_AsPointer(rhs, CPointerType_PointedToType(tp_self), &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), goto err);
	if (lhs_cvalue.ptr < rhs_cvalue.ptr)
		return Dee_COMPARE_LO;
	if (lhs_cvalue.ptr > rhs_cvalue.ptr)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
pointer_int32(CPointerType *tp_self, union pointer const *self, int32_t *result) {
	union pointer cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return Dee_INT_ERROR);
	*result = (int32_t)(uint32_t)cvalue.uint;
	return Dee_INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1/*, 3*/)) int DCALL
pointer_int64(CPointerType *tp_self, union pointer const *self, int64_t *result) {
	union pointer cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return Dee_INT_ERROR);
	*result = (int64_t)(uint64_t)cvalue.uint;
	return Dee_INT_UNSIGNED;
}

#undef pointer_double
#define pointer_double unsupported__double

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pointer_int(CPointerType *tp_self, union pointer const *self) {
	union pointer cvalue;
	(void)tp_self;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return NULL);
	return DeeInt_NewUIntptr(cvalue.uint);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
pointer_printcrepr(CPointerType *tp_self, union pointer const *self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	union pointer cvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintCRepr(CPointerType_AsCType(tp_self), printer, arg, NULL);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	if (cvalue.uint == 0) {
		temp = DeeFormat_PRINT(printer, arg, ")NULL");
	} else {
		temp = DeeFormat_Printf(printer, arg, ")%#" PRFxPTR, cvalue.uint);
	}
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
pointer_printdrepr(CPointerType *tp_self, union pointer const *self,
                   Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	union pointer cvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintDRepr(CPointerType_AsCType(tp_self), printer, arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	if (cvalue.uint == 0) {
		temp = DeeFormat_PRINT(printer, arg, ")none");
	} else {
		temp = DeeFormat_Printf(printer, arg, ")%#" PRFxPTR, cvalue.uint);
	}
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

#undef pointer_inv
#define pointer_inv unsupported__inv
#undef pointer_pos
#define pointer_pos unsupported__pos
#undef pointer_neg
#define pointer_neg unsupported__neg

PRIVATE WUNUSED NONNULL((1, 3)) DREF CPointer *DCALL
pointer_add(CPointerType *tp_self, union pointer const *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue;
	union pointer lhs_cvalue;
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), goto err);
	rhs_cvalue *= CPointerType_SizeofPointedToType(tp_self);
	lhs_cvalue.sint += rhs_cvalue;
	/* NOTE: CPointer R-values need a custom "operator +"
	 *       that passes along the pointer's owner reference! */
	return CPointer_New(tp_self, lhs_cvalue.ptr);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
pointer_sub(CPointerType *tp_self, union pointer const *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue_int;
	union pointer rhs_cvalue;
	union pointer lhs_cvalue;
	CType *rhs_pointer_base;
	int status;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), goto err);
	status = DeeObject_TryAsGenericPointer(rhs, &rhs_pointer_base, &rhs_cvalue);
	if (status <= 0) {
		ptrdiff_t result;
		if unlikely(status < 0)
			goto err;
		if (CPointerType_PointedToType(tp_self) != rhs_pointer_base) {
			DREF CPointerType *rhs_pointer_type;
			rhs_pointer_type = CPointerType_Of(rhs_pointer_base);
			if unlikely(!rhs_pointer_type)
				goto err;
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot subtract incompatible pointer types: %k - %k",
			                tp_self, rhs_pointer_type);
			Dee_Decref_unlikely(CPointerType_AsType(rhs_pointer_type));
			goto err;
		}
		result = lhs_cvalue.uint - rhs_cvalue.uint;
		if (CPointerType_SizeofPointedToType(tp_self))
			result /= CPointerType_SizeofPointedToType(tp_self);
		return DeeInt_NewPtrdiff(result);
	}
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue_int))
		goto err;
	rhs_cvalue_int *= CPointerType_SizeofPointedToType(tp_self);
	lhs_cvalue.sint -= rhs_cvalue_int;
	/* NOTE: CPointer R-values need a custom "operator -"
	 *       that passes along the pointer's owner reference! */
	return Dee_AsObject(CPointer_New(tp_self, lhs_cvalue.ptr));
err:
	return NULL;
}

#undef pointer_mul
#define pointer_mul unsupported__mul
#undef pointer_div
#define pointer_div unsupported__div
#undef pointer_mod
#define pointer_mod unsupported__mod
#undef pointer_shl
#define pointer_shl unsupported__shl
#undef pointer_shr
#define pointer_shr unsupported__shr
#undef pointer_and
#define pointer_and unsupported__and
#undef pointer_or
#define pointer_or unsupported__or
#undef pointer_xor
#define pointer_xor unsupported__xor
#undef pointer_pow
#define pointer_pow unsupported__pow


PRIVATE WUNUSED NONNULL((1)) int DCALL
pointer_inc(CPointerType *tp_self, union pointer *self) {
	union pointer cvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	cvalue.uint += CPointerType_SizeofPointedToType(tp_self);
	CTYPES_FAULTPROTECT(pointer_set(self, cvalue.ptr), return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
pointer_dec(CPointerType *tp_self, union pointer *self) {
	union pointer cvalue;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	cvalue.uint -= CPointerType_SizeofPointedToType(tp_self);
	CTYPES_FAULTPROTECT(pointer_set(self, cvalue.ptr), return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_inplace_add(CPointerType *tp_self, union pointer *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue;
	union pointer lhs_cvalue;
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), goto err);
	rhs_cvalue *= CPointerType_SizeofPointedToType(tp_self);
	lhs_cvalue.sint += rhs_cvalue;
	CTYPES_FAULTPROTECT(pointer_set(lhs, lhs_cvalue.ptr), goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_inplace_sub(CPointerType *tp_self, union pointer *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue;
	union pointer lhs_cvalue;
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue))
		goto err;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), goto err);
	rhs_cvalue *= CPointerType_SizeofPointedToType(tp_self);
	lhs_cvalue.sint -= rhs_cvalue;
	CTYPES_FAULTPROTECT(pointer_set(lhs, lhs_cvalue.ptr), goto err);
	return 0;
err:
	return -1;
}


#undef pointer_inplace_mul
#define pointer_inplace_mul unsupported__inplace_mul
#undef pointer_inplace_div
#define pointer_inplace_div unsupported__inplace_div
#undef pointer_inplace_mod
#define pointer_inplace_mod unsupported__inplace_mod
#undef pointer_inplace_shl
#define pointer_inplace_shl unsupported__inplace_shl
#undef pointer_inplace_shr
#define pointer_inplace_shr unsupported__inplace_shr
#undef pointer_inplace_and
#define pointer_inplace_and unsupported__inplace_and
#undef pointer_inplace_or
#define pointer_inplace_or unsupported__inplace_or
#undef pointer_inplace_xor
#define pointer_inplace_xor unsupported__inplace_xor
#undef pointer_inplace_pow
#define pointer_inplace_pow unsupported__inplace_pow


INTERN struct ctype_operators Dee_tpconst cpointer_operators = {
	/* .co_initfrom    = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_initfrom,
	/* .co_initwith    = */ (int (DCALL *)(CType *, void *, size_t, DeeObject *const *, DeeObject *))&pointer_initwith,
	/* .co_bool        = */ (int (DCALL *)(CType *, void const *))&pointer_bool,
	/* .co_printcrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&pointer_printcrepr,
	/* .co_printdrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&pointer_printdrepr,
	/* .co_compare     = */ (int (DCALL *)(CType *, void const *, DeeObject *))&pointer_compare,
	/* .co_int32       = */ (int (DCALL *)(CType *, void const *, int32_t *))&pointer_int32,
	/* .co_int64       = */ (int (DCALL *)(CType *, void const *, int64_t *))&pointer_int64,
	/* .co_double      = */ (int (DCALL *)(CType *, void const *, double *))&pointer_double,
	/* .co_int         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&pointer_int,
	/* .co_inv         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&pointer_inv,
	/* .co_pos         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&pointer_pos,
	/* .co_neg         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&pointer_neg,
	/* .co_add         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_add,
	/* .co_sub         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_sub,
	/* .co_mul         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_mul,
	/* .co_div         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_div,
	/* .co_mod         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_mod,
	/* .co_shl         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_shl,
	/* .co_shr         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_shr,
	/* .co_and         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_and,
	/* .co_or          = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_or,
	/* .co_xor         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_xor,
	/* .co_pow         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&pointer_pow,
	/* .co_inc         = */ (int (DCALL *)(CType *, void *))&pointer_inc,
	/* .co_dec         = */ (int (DCALL *)(CType *, void *))&pointer_dec,
	/* .co_inplace_add = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_add,
	/* .co_inplace_sub = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_sub,
	/* .co_inplace_mul = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_mul,
	/* .co_inplace_div = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_div,
	/* .co_inplace_mod = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_mod,
	/* .co_inplace_shl = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_shl,
	/* .co_inplace_shr = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_shr,
	/* .co_inplace_and = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_and,
	/* .co_inplace_or  = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_or,
	/* .co_inplace_xor = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_xor,
	/* .co_inplace_pow = */ (int (DCALL *)(CType *, void *, DeeObject *))&pointer_inplace_pow,
};



/* "tp_self" is "CLValueType"; forward operator to "tp_self->clt_orig" and "*(void **)self"
 * This is only really needed when user-defined structure types contain lvalue fields:
 * >> local a = (union { .x = int })(x: 42);
 * >> local b = (union { .x = int.ptr, .y = int.lvalue })(y: a.x);
 * >> assert a.x == 42;
 * >> assert b.x == a.x.ptr;
 * >> assert b.y == 42;
 * >> b.y = 99;
 * >> assert a.x == 99; */


PRIVATE WUNUSED NONNULL((1)) int DCALL
lvalue_initfrom(CLValueType *tp_self, union pointer *self, DeeObject *value) {
	union pointer cvalue;
	CLValue *value_lvalue;
	if (DeeObject_AssertTypeExact(value, CLValueType_AsType(tp_self)))
		goto err;
	value_lvalue = Object_AsCLValue(value);
	cvalue = value_lvalue->cl_value;
	CTYPES_FAULTPROTECT(pointer_set(self, cvalue.ptr), goto err);
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
lvalue_initwith(CLValueType *tp_self, union pointer *self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:LValue", &value))
		goto err;
	return lvalue_initfrom(tp_self, self, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lvalue_bool(CLValueType *tp_self, union pointer const *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_bool)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
lvalue_printcrepr(CLValueType *tp_self, union pointer const *self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintCRepr(CLValueType_AsCType(tp_self), printer, arg, NULL);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")<LVALUE(");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	temp = (*ops->co_printcrepr)(CLValueType_PointedToType(tp_self), cvalue.ptr, printer, arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")>");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
lvalue_printdrepr(CLValueType *tp_self, union pointer const *self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintDRepr(CLValueType_AsCType(tp_self), printer, arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")<LVALUE(");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	temp = (*ops->co_printdrepr)(CLValueType_PointedToType(tp_self), cvalue.ptr, printer, arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, ")>");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
lvalue_compare(CLValueType *tp_self, union pointer const *lhs, DeeObject *rhs) {
	union pointer lhs_cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), return Dee_COMPARE_ERR);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_compare)(CLValueType_PointedToType(tp_self), lhs_cvalue.ptr, rhs);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
lvalue_int32(CLValueType *tp_self, union pointer const *self, int32_t *p_result) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return Dee_INT_ERROR);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_int32)(CLValueType_PointedToType(tp_self), cvalue.ptr, p_result);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
lvalue_int64(CLValueType *tp_self, union pointer const *self, int64_t *p_result) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return Dee_INT_ERROR);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_int64)(CLValueType_PointedToType(tp_self), cvalue.ptr, p_result);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
lvalue_double(CLValueType *tp_self, union pointer const *self, double *p_result) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_double)(CLValueType_PointedToType(tp_self), cvalue.ptr, p_result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lvalue_int(CLValueType *tp_self, union pointer const *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return NULL);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_int)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lvalue_inv(CLValueType *tp_self, union pointer const *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return NULL);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_inv)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lvalue_pos(CLValueType *tp_self, union pointer const *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return NULL);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_pos)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lvalue_neg(CLValueType *tp_self, union pointer const *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return NULL);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_neg)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

#define DEFINE_LVALUE_BINARY_OP(lvalue_foo, co_foo)                                     \
	PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL                               \
	lvalue_foo(CLValueType *tp_self, union pointer const *lhs, DeeObject *rhs) {        \
		union pointer lhs_cvalue;                                                       \
		struct ctype_operators const *ops;                                              \
		CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), return NULL);            \
		/* Forward to pointed-to memory location */                                     \
		ops = CLValueType_PointedToOperators(tp_self);                                  \
		return (*ops->co_foo)(CLValueType_PointedToType(tp_self), lhs_cvalue.ptr, rhs); \
	}
DEFINE_LVALUE_BINARY_OP(lvalue_add, co_add)
DEFINE_LVALUE_BINARY_OP(lvalue_sub, co_sub)
DEFINE_LVALUE_BINARY_OP(lvalue_mul, co_mul)
DEFINE_LVALUE_BINARY_OP(lvalue_div, co_div)
DEFINE_LVALUE_BINARY_OP(lvalue_mod, co_mod)
DEFINE_LVALUE_BINARY_OP(lvalue_shl, co_shl)
DEFINE_LVALUE_BINARY_OP(lvalue_shr, co_shr)
DEFINE_LVALUE_BINARY_OP(lvalue_and, co_and)
DEFINE_LVALUE_BINARY_OP(lvalue_or, co_or)
DEFINE_LVALUE_BINARY_OP(lvalue_xor, co_xor)
DEFINE_LVALUE_BINARY_OP(lvalue_pow, co_pow)
#undef DEFINE_LVALUE_BINARY_OP

PRIVATE WUNUSED NONNULL((1)) int DCALL
lvalue_inc(CLValueType *tp_self, union pointer *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_inc)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lvalue_dec(CLValueType *tp_self, union pointer *self) {
	union pointer cvalue;
	struct ctype_operators const *ops;
	CTYPES_FAULTPROTECT(cvalue.ptr = pointer_get(self), return -1);

	/* Forward to pointed-to memory location */
	ops = CLValueType_PointedToOperators(tp_self);
	return (*ops->co_dec)(CLValueType_PointedToType(tp_self), cvalue.ptr);
}

#define DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_foo, co_inplace_foo)                     \
	PRIVATE WUNUSED NONNULL((1, 3)) int DCALL                                                   \
	lvalue_inplace_foo(CLValueType *tp_self, union pointer *lhs, DeeObject *rhs) {              \
		union pointer lhs_cvalue;                                                               \
		struct ctype_operators const *ops;                                                      \
		CTYPES_FAULTPROTECT(lhs_cvalue.ptr = pointer_get(lhs), return -1);                      \
		/* Forward to pointed-to memory location */                                             \
		ops = CLValueType_PointedToOperators(tp_self);                                          \
		return (*ops->co_inplace_foo)(CLValueType_PointedToType(tp_self), lhs_cvalue.ptr, rhs); \
	}
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_add, co_inplace_add)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_sub, co_inplace_sub)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_mul, co_inplace_mul)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_div, co_inplace_div)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_mod, co_inplace_mod)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_shl, co_inplace_shl)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_shr, co_inplace_shr)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_and, co_inplace_and)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_or, co_inplace_or)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_xor, co_inplace_xor)
DEFINE_LVALUE_BINARY_INPLACE_OP(lvalue_inplace_pow, co_inplace_pow)
#undef DEFINE_LVALUE_BINARY_INPLACE_OP

INTERN struct ctype_operators Dee_tpconst clvalue_operators = {
	/* .co_initfrom    = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_initfrom,
	/* .co_initwith    = */ (int (DCALL *)(CType *, void *, size_t, DeeObject *const *, DeeObject *))&lvalue_initwith,
	/* .co_bool        = */ (int (DCALL *)(CType *, void const *))&lvalue_bool,
	/* .co_printcrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&lvalue_printcrepr,
	/* .co_printdrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&lvalue_printdrepr,
	/* .co_compare     = */ (int (DCALL *)(CType *, void const *, DeeObject *))&lvalue_compare,
	/* .co_int32       = */ (int (DCALL *)(CType *, void const *, int32_t *))&lvalue_int32,
	/* .co_int64       = */ (int (DCALL *)(CType *, void const *, int64_t *))&lvalue_int64,
	/* .co_double      = */ (int (DCALL *)(CType *, void const *, double *))&lvalue_double,
	/* .co_int         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&lvalue_int,
	/* .co_inv         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&lvalue_inv,
	/* .co_pos         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&lvalue_pos,
	/* .co_neg         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&lvalue_neg,
	/* .co_add         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_add,
	/* .co_sub         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_sub,
	/* .co_mul         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_mul,
	/* .co_div         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_div,
	/* .co_mod         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_mod,
	/* .co_shl         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_shl,
	/* .co_shr         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_shr,
	/* .co_and         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_and,
	/* .co_or          = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_or,
	/* .co_xor         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_xor,
	/* .co_pow         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&lvalue_pow,
	/* .co_inc         = */ (int (DCALL *)(CType *, void *))&lvalue_inc,
	/* .co_dec         = */ (int (DCALL *)(CType *, void *))&lvalue_dec,
	/* .co_inplace_add = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_add,
	/* .co_inplace_sub = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_sub,
	/* .co_inplace_mul = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_mul,
	/* .co_inplace_div = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_div,
	/* .co_inplace_mod = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_mod,
	/* .co_inplace_shl = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_shl,
	/* .co_inplace_shr = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_shr,
	/* .co_inplace_and = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_and,
	/* .co_inplace_or  = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_or,
	/* .co_inplace_xor = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_xor,
	/* .co_inplace_pow = */ (int (DCALL *)(CType *, void *, DeeObject *))&lvalue_inplace_pow,
};



/* Operators for user-defined structure types */

PRIVATE WUNUSED NONNULL((1)) int DCALL
struct_initfrom(CStructType *tp_self, void *self, DeeObject *value) {
	/* >> struct MyStruct {
	 * >>     .x = int,
	 * >>     .y = int,
	 * >> };
	 * >> local a = MyStruct({ 10, 20 });
	 * >> local b = MyStruct({ .x = 10, .y = 20 });
	 * >> local c = MyStruct(a);  // Must also accept when "a" is the l-value variant of "MyStruct"
	 */

	/* TODO */
	(void)tp_self;
	(void)self;
	(void)value;
	DeeError_NOTIMPLEMENTED();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
struct_initwith(CStructType *tp_self, void *self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	if (!kw) {
		switch (argc) {
		case 0:
			CTYPES_FAULTPROTECT({
				bzero(self, CType_Sizeof(CStructType_AsCType(tp_self)));
			}, goto err);
			return 0;
		case 1:
			return struct_initfrom(tp_self, self, argv[0]);
		default: break;
		}
		DeeArg_BadArgcEx(DeeType_GetName(CStructType_AsType(tp_self)),
		                 argc, 0, 1);
		goto err;
	}
	/* >> struct MyStruct {
	 * >>     .x = int,
	 * >>     .y = int,
	 * >> };
	 * >> local c = MyStruct(10, y: 20); */
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return 0;
err:
	return -1;
}

#undef struct_bool
#define struct_bool unsupported__bool

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
struct_printcrepr(CStructType *tp_self, void const *self,
                  Dee_formatprinter_t printer, void *arg) {
	bool is_first = true;
	Dee_ssize_t result, temp;
	struct cstruct_field *field;
	result = DeeFormat_PRINT(printer, arg, "{");
	if unlikely(result < 0)
		goto done;
	CStructType_ForeachField(field, tp_self) {
		CType *field_type;
		void const *field_address;
		struct ctype_operators const *field_type_operators;
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		ASSERT(field->csf_name);
		if (!DeeString_IsEmpty(field->csf_name)) {
			temp = DeeFormat_Printf(printer, arg, ".%k = ", field->csf_name);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		field_type = CLValueType_PointedToType(field->csf_lvtype);
		field_type_operators = CType_Operators(field_type);
		field_address = (void const *)((byte_t *)self + field->csf_offset);
		temp = (*field_type_operators->co_printcrepr)(field_type, field_address, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		is_first = false;
	}
	temp = is_first ? DeeFormat_PRINT(printer, arg, "}")
	                : DeeFormat_PRINT(printer, arg, " }");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
struct_printdrepr(CStructType *tp_self, void const *self,
                  Dee_formatprinter_t printer, void *arg) {
	bool is_first = true;
	Dee_ssize_t result, temp;
	struct cstruct_field *field;
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintDRepr(CStructType_AsCType(tp_self), printer, arg);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, "){");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	CStructType_ForeachField(field, tp_self) {
		CType *field_type;
		void const *field_address;
		struct ctype_operators const *field_type_operators;
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		ASSERT(field->csf_name);
		if (!DeeString_IsEmpty(field->csf_name)) {
			temp = DeeFormat_Printf(printer, arg, "%r: ", field->csf_name);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		field_type = CLValueType_PointedToType(field->csf_lvtype);
		field_type_operators = CType_Operators(field_type);
		field_address = (void const *)((byte_t *)self + field->csf_offset);
		temp = (*field_type_operators->co_printdrepr)(field_type, field_address, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		is_first = false;
	}
	temp = is_first ? DeeFormat_PRINT(printer, arg, "}")
	                : DeeFormat_PRINT(printer, arg, " }");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

#undef struct_compare
#define struct_compare unsupported__compare
#undef struct_int32
#define struct_int32 unsupported__int32
#undef struct_int64
#define struct_int64 unsupported__int64
#undef struct_double
#define struct_double unsupported__double
#undef struct_int
#define struct_int unsupported__int
#undef struct_inv
#define struct_inv unsupported__inv
#undef struct_pos
#define struct_pos unsupported__pos
#undef struct_neg
#define struct_neg unsupported__neg
#undef struct_add
#define struct_add unsupported__add
#undef struct_sub
#define struct_sub unsupported__sub
#undef struct_mul
#define struct_mul unsupported__mul
#undef struct_div
#define struct_div unsupported__div
#undef struct_mod
#define struct_mod unsupported__mod
#undef struct_shl
#define struct_shl unsupported__shl
#undef struct_shr
#define struct_shr unsupported__shr
#undef struct_and
#define struct_and unsupported__and
#undef struct_or
#define struct_or unsupported__or
#undef struct_xor
#define struct_xor unsupported__xor
#undef struct_pow
#define struct_pow unsupported__pow

#undef struct_inc
#define struct_inc unsupported__inc
#undef struct_dec
#define struct_dec unsupported__dec

#undef struct_inplace_add
#define struct_inplace_add unsupported__inplace_add
#undef struct_inplace_sub
#define struct_inplace_sub unsupported__inplace_sub
#undef struct_inplace_mul
#define struct_inplace_mul unsupported__inplace_mul
#undef struct_inplace_div
#define struct_inplace_div unsupported__inplace_div
#undef struct_inplace_mod
#define struct_inplace_mod unsupported__inplace_mod
#undef struct_inplace_shl
#define struct_inplace_shl unsupported__inplace_shl
#undef struct_inplace_shr
#define struct_inplace_shr unsupported__inplace_shr
#undef struct_inplace_and
#define struct_inplace_and unsupported__inplace_and
#undef struct_inplace_or
#define struct_inplace_or unsupported__inplace_or
#undef struct_inplace_xor
#define struct_inplace_xor unsupported__inplace_xor
#undef struct_inplace_pow
#define struct_inplace_pow unsupported__inplace_pow


INTERN struct ctype_operators Dee_tpconst cstruct_operators = {
	/* .co_initfrom    = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_initfrom,
	/* .co_initwith    = */ (int (DCALL *)(CType *, void *, size_t, DeeObject *const *, DeeObject *))&struct_initwith,
	/* .co_bool        = */ (int (DCALL *)(CType *, void const *))&struct_bool,
	/* .co_printcrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&struct_printcrepr,
	/* .co_printdrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&struct_printdrepr,
	/* .co_compare     = */ (int (DCALL *)(CType *, void const *, DeeObject *))&struct_compare,
	/* .co_int32       = */ (int (DCALL *)(CType *, void const *, int32_t *))&struct_int32,
	/* .co_int64       = */ (int (DCALL *)(CType *, void const *, int64_t *))&struct_int64,
	/* .co_double      = */ (int (DCALL *)(CType *, void const *, double *))&struct_double,
	/* .co_int         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&struct_int,
	/* .co_inv         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&struct_inv,
	/* .co_pos         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&struct_pos,
	/* .co_neg         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&struct_neg,
	/* .co_add         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_add,
	/* .co_sub         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_sub,
	/* .co_mul         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_mul,
	/* .co_div         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_div,
	/* .co_mod         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_mod,
	/* .co_shl         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_shl,
	/* .co_shr         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_shr,
	/* .co_and         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_and,
	/* .co_or          = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_or,
	/* .co_xor         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_xor,
	/* .co_pow         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&struct_pow,
	/* .co_inc         = */ (int (DCALL *)(CType *, void *))&struct_inc,
	/* .co_dec         = */ (int (DCALL *)(CType *, void *))&struct_dec,
	/* .co_inplace_add = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_add,
	/* .co_inplace_sub = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_sub,
	/* .co_inplace_mul = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_mul,
	/* .co_inplace_div = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_div,
	/* .co_inplace_mod = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_mod,
	/* .co_inplace_shl = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_shl,
	/* .co_inplace_shr = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_shr,
	/* .co_inplace_and = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_and,
	/* .co_inplace_or  = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_or,
	/* .co_inplace_xor = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_xor,
	/* .co_inplace_pow = */ (int (DCALL *)(CType *, void *, DeeObject *))&struct_inplace_pow,
};




/* Operators for array types */

PRIVATE WUNUSED NONNULL((1)) int DCALL
array_initfrom(CArrayType *tp_self, void *self, DeeObject *value) {
	/* >> local MyArray = int[42];
	 * >> local a = MyArray(none);
	 * >> local a = MyArray({ });
	 * >> local a = MyArray({ 10, 20 });
	 * >> local a = MyArray({ [7] = 10 }); // TBA-syntax; tldr: must support "seq_enumerate_items" for init
	 */
	/* TODO */
	(void)tp_self;
	(void)self;
	(void)value;
	DeeError_NOTIMPLEMENTED();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
array_initwith(CArrayType *tp_self, void *self,
               size_t argc, DeeObject *const *argv,
               DeeObject *kw) {
	DeeObject *value;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__value,
	                          "o:Array", &value))
		goto err;
	return array_initfrom(tp_self, self, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
array_bool(CArrayType *tp_self, void const *self) {
	(void)tp_self;
	return self ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
array_compare(CArrayType *tp_self, void const *lhs, DeeObject *rhs) {
	union pointer rhs_cvalue; /* Support array-to-pointer decay */
	if (DeeObject_AsPointer(rhs, CArrayType_PointedToType(tp_self), &rhs_cvalue))
		goto err;
	if (lhs < rhs_cvalue.ptr)
		return Dee_COMPARE_LO;
	if (lhs > rhs_cvalue.ptr)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
array_int32(CArrayType *tp_self, void const *lhs, int32_t *__restrict p_result) {
	(void)tp_self; /* Support array-to-pointer decay */
	*p_result = (int32_t)(uint32_t)(uintptr_t)lhs;
	return Dee_INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
array_int64(CArrayType *tp_self, void const *lhs, int64_t *__restrict p_result) {
	(void)tp_self; /* Support array-to-pointer decay */
	*p_result = (int64_t)(uint64_t)(uintptr_t)lhs;
	return Dee_INT_UNSIGNED;
}

#undef array_double
#define array_double unsupported__double

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
array_int(CArrayType *tp_self, void const *self) {
	(void)tp_self; /* Support array-to-pointer decay */
	return DeeInt_NewUIntptr((uintptr_t)self);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
array_printcrepr(CArrayType *tp_self, void const *self,
                 Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t result, temp;
	CType *item_type;
	struct ctype_operators const *item_type_operators;
	result = DeeFormat_PRINT(printer, arg, "(");
	if unlikely(result < 0)
		goto done;
	temp = CType_PrintCRepr(CArrayType_AsCType(tp_self), printer, arg, NULL);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	temp = DeeFormat_PRINT(printer, arg, "){");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	item_type = CArrayType_PointedToType(tp_self);
	item_type_operators = CType_Operators(item_type);
	for (i = 0; i < CArrayType_Count(tp_self); ++i) {
		void const *item_addr;
		if (i != 0) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		item_addr = (byte_t const *)self + (i * CArrayType_SizeofPointedToType(tp_self));
		temp = (*item_type_operators->co_printcrepr)(item_type, item_addr, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	temp = DeeFormat_PRINT(printer, arg, "}");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
array_printdrepr(CArrayType *tp_self, void const *self,
                 Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t result, temp;
	CType *item_type;
	struct ctype_operators const *item_type_operators;
	result = CType_PrintDRepr(CArrayType_AsCType(tp_self), printer, arg);
	if unlikely(result < 0)
		goto done;
	temp = DeeFormat_PRINT(printer, arg, "{");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	item_type = CArrayType_PointedToType(tp_self);
	item_type_operators = CType_Operators(item_type);
	for (i = 0; i < CArrayType_Count(tp_self); ++i) {
		void const *item_addr;
		if (i != 0) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		item_addr = (byte_t const *)self + (i * CArrayType_SizeofPointedToType(tp_self));
		temp = (*item_type_operators->co_printdrepr)(item_type, item_addr, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	temp = DeeFormat_PRINT(printer, arg, "}");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF CPointer *DCALL
array_add(CArrayType *tp_self, void const *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue;
	union pointer lhs_cvalue;
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue))
		goto err;
	rhs_cvalue *= CArrayType_SizeofPointedToType(tp_self);
	lhs_cvalue.ptr = (void *)lhs;
	lhs_cvalue.sint += rhs_cvalue;
	/* NOTE: CArray R-values need a custom "operator +"
	 *       that passes along the array as the owner reference! */
	return CPointer_For(CArrayType_PointedToType(tp_self), lhs_cvalue.ptr);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
array_sub(CArrayType *tp_self, void const *lhs, DeeObject *rhs) {
	ptrdiff_t rhs_cvalue_int;
	union pointer rhs_cvalue;
	union pointer lhs_cvalue;
	CType *rhs_pointer_base;
	int status;
	lhs_cvalue.ptr = (void *)lhs;
	status = DeeObject_TryAsGenericPointer(rhs, &rhs_pointer_base, &rhs_cvalue);
	if (status <= 0) {
		ptrdiff_t result;
		if unlikely(status < 0)
			goto err;
		if (CArrayType_PointedToType(tp_self) != rhs_pointer_base) {
			DREF CPointerType *lhs_pointer_type;
			DREF CPointerType *rhs_pointer_type;
			lhs_pointer_type = CPointerType_Of(CArrayType_PointedToType(tp_self));
			if unlikely(!lhs_pointer_type)
				goto err;
			rhs_pointer_type = CPointerType_Of(rhs_pointer_base);
			if unlikely(!rhs_pointer_type) {
				Dee_Decref_unlikely(CPointerType_AsType(lhs_pointer_type));
				goto err;
			}
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot subtract incompatible pointer types: %k - %k",
			                lhs_pointer_type, rhs_pointer_type);
			Dee_Decref_unlikely(CPointerType_AsType(rhs_pointer_type));
			Dee_Decref_unlikely(CPointerType_AsType(lhs_pointer_type));
			goto err;
		}
		result = lhs_cvalue.uint - rhs_cvalue.uint;
		if (CArrayType_SizeofPointedToType(tp_self))
			result /= CArrayType_SizeofPointedToType(tp_self);
		return DeeInt_NewPtrdiff(result);
	}
	if (DeeObject_AsPtrdiff(rhs, &rhs_cvalue_int))
		goto err;
	rhs_cvalue_int *= CArrayType_SizeofPointedToType(tp_self);
	lhs_cvalue.sint -= rhs_cvalue_int;
	/* NOTE: CPointer R-values need a custom "operator -"
	 *       that passes along the pointer's owner reference! */
	return Dee_AsObject(CPointer_For(CArrayType_PointedToType(tp_self), lhs_cvalue.ptr));
err:
	return NULL;
}



#undef array_inv
#define array_inv unsupported__inv
#undef array_pos
#define array_pos unsupported__pos
#undef array_neg
#define array_neg unsupported__neg
#undef array_mul
#define array_mul unsupported__mul
#undef array_div
#define array_div unsupported__div
#undef array_mod
#define array_mod unsupported__mod
#undef array_shl
#define array_shl unsupported__shl
#undef array_shr
#define array_shr unsupported__shr
#undef array_and
#define array_and unsupported__and
#undef array_or
#define array_or unsupported__or
#undef array_xor
#define array_xor unsupported__xor
#undef array_pow
#define array_pow unsupported__pow
#undef array_inc
#define array_inc unsupported__inc
#undef array_dec
#define array_dec unsupported__dec
#undef array_inplace_add
#define array_inplace_add unsupported__inplace_add
#undef array_inplace_sub
#define array_inplace_sub unsupported__inplace_sub
#undef array_inplace_mul
#define array_inplace_mul unsupported__inplace_mul
#undef array_inplace_div
#define array_inplace_div unsupported__inplace_div
#undef array_inplace_mod
#define array_inplace_mod unsupported__inplace_mod
#undef array_inplace_shl
#define array_inplace_shl unsupported__inplace_shl
#undef array_inplace_shr
#define array_inplace_shr unsupported__inplace_shr
#undef array_inplace_and
#define array_inplace_and unsupported__inplace_and
#undef array_inplace_or
#define array_inplace_or unsupported__inplace_or
#undef array_inplace_xor
#define array_inplace_xor unsupported__inplace_xor
#undef array_inplace_pow
#define array_inplace_pow unsupported__inplace_pow

INTERN struct ctype_operators Dee_tpconst carray_operators = {
	/* .co_initfrom    = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_initfrom,
	/* .co_initwith    = */ (int (DCALL *)(CType *, void *, size_t, DeeObject *const *, DeeObject *))&array_initwith,
	/* .co_bool        = */ (int (DCALL *)(CType *, void const *))&array_bool,
	/* .co_printcrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&array_printcrepr,
	/* .co_printdrepr  = */ (Dee_ssize_t (DCALL *)(CType *, void const *, Dee_formatprinter_t, void *))&array_printdrepr,
	/* .co_compare     = */ (int (DCALL *)(CType *, void const *, DeeObject *))&array_compare,
	/* .co_int32       = */ (int (DCALL *)(CType *, void const *, int32_t *))&array_int32,
	/* .co_int64       = */ (int (DCALL *)(CType *, void const *, int64_t *))&array_int64,
	/* .co_double      = */ (int (DCALL *)(CType *, void const *, double *))&array_double,
	/* .co_int         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&array_int,
	/* .co_inv         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&array_inv,
	/* .co_pos         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&array_pos,
	/* .co_neg         = */ (DREF DeeObject *(DCALL *)(CType *, void const *))&array_neg,
	/* .co_add         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_add,
	/* .co_sub         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_sub,
	/* .co_mul         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_mul,
	/* .co_div         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_div,
	/* .co_mod         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_mod,
	/* .co_shl         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_shl,
	/* .co_shr         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_shr,
	/* .co_and         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_and,
	/* .co_or          = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_or,
	/* .co_xor         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_xor,
	/* .co_pow         = */ (DREF DeeObject *(DCALL *)(CType *, void const *, DeeObject *))&array_pow,
	/* .co_inc         = */ (int (DCALL *)(CType *, void *))&array_inc,
	/* .co_dec         = */ (int (DCALL *)(CType *, void *))&array_dec,
	/* .co_inplace_add = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_add,
	/* .co_inplace_sub = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_sub,
	/* .co_inplace_mul = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_mul,
	/* .co_inplace_div = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_div,
	/* .co_inplace_mod = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_mod,
	/* .co_inplace_shl = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_shl,
	/* .co_inplace_shr = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_shr,
	/* .co_inplace_and = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_and,
	/* .co_inplace_or  = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_or,
	/* .co_inplace_xor = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_xor,
	/* .co_inplace_pow = */ (int (DCALL *)(CType *, void *, DeeObject *))&array_inplace_pow,
};

DECL_END
#endif /* CONFIG_EXPERIMENTAL_REWORKED_CTYPES */

#endif /* !GUARD_DEX_CTYPES_CTYPES_OPERATORS_C */
