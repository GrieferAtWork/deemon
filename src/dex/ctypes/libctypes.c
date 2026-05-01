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
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_C
#define GUARD_DEX_CTYPES_LIBCTYPES_C 1
#define DEE_SOURCE

/* WARNING: Using this module trips the unspoken warranty
 *          of me feeling responsible when deemon crashes. */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>     /* Dee_Free */
#include <deemon/arg.h>       /* DeeArg_UnpackStruct, DeeArg_UnpackStruct1XOr2X, UNPuSIZ */
#include <deemon/bytes.h>     /* DeeBytes_Check, DeeBytes_SIZE */
#include <deemon/dex.h>       /* DEX_*, Dee_DEXSYM_READONLY */
#include <deemon/error.h>     /* DeeError_* */
#include <deemon/format.h>    /* PRFuSIZ */
#include <deemon/int.h>       /* DeeInt_NewSize */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_*, Dee_Incref, Dee_uint128_t */
#include <deemon/objmethod.h> /*  */
#include <deemon/type.h>      /* METHOD_FCONSTCALL, METHOD_FCONSTCALL_IF_ARGS_CONSTCAST */

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_BIG_ENDIAN__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/byteswap.h>  /* BSWAP* */
#include <hybrid/int128.h>    /* __hybrid_uint128_getword64, __hybrid_uint128_setword64 */
#include <hybrid/typecore.h>  /* __SIZEOF_*__ */

#include "c_api.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t, uint32_t, uint64_t */

#ifdef CONFIG_HAVE_CTYPES_SEH_GUARD
#include <Windows.h>
#endif /* CONFIG_HAVE_CTYPES_SEH_GUARD */

#ifdef CONFIG_HAVE_CTYPES_KOS_GUARD
#include <kos/except.h>
#endif /* CONFIG_HAVE_CTYPES_KOS_GUARD */

DECL_BEGIN

#ifdef CONFIG_HAVE_CTYPES_SEH_GUARD
INTERN int ctypes_seh_guard(struct _EXCEPTION_POINTERS *info) {
	DWORD dwExcept;
	if (!info)
		goto done;
	dwExcept = info->ExceptionRecord->ExceptionCode;
	switch (dwExcept) {

	case EXCEPTION_ACCESS_VIOLATION:
		/* Throw a segfault error and run the associated handler. */
		DeeError_Throwf(&DeeError_SegFault, "Segmentation fault when %s %p",
		                info->ExceptionRecord->ExceptionInformation[0] == 0
		                ? "reading from"
		                : info->ExceptionRecord->ExceptionInformation[0] == 1
		                  ? "writing to"
		                  : "executing",
		                info->ExceptionRecord->ExceptionInformation[1]);
		goto did_handle;

	/* NOTE: This SEH guard is also used to protect foreign function calls,
	 *       so we also handle other exceptions such as divide-by-zero, etc. */
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		DeeError_Throwf(&DeeError_SegFault, "Data misalignment");
		goto did_handle;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		DeeError_Throwf(&DeeError_IndexError, "Array bounds exceeded");
		goto did_handle;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		DeeError_Throwf(&DeeError_ArithmeticError, "Denormal floating point operand");
		goto did_handle;

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		DeeError_Throwf(&DeeError_DivideByZero, "Floating point divide by zero");
		goto did_handle;

	case EXCEPTION_FLT_INEXACT_RESULT:
		DeeError_Throwf(&DeeError_ArithmeticError, "Inexact floating point results");
		goto did_handle;

	case EXCEPTION_FLT_INVALID_OPERATION:
		DeeError_Throwf(&DeeError_ArithmeticError, "Invalid floating point operation");
		goto did_handle;

	case EXCEPTION_FLT_OVERFLOW:
		DeeError_Throwf(&DeeError_IntegerOverflow, "floating point overflow");
		goto did_handle;

	case EXCEPTION_FLT_STACK_CHECK:
		DeeError_Throwf(&DeeError_StackOverflow, "floating point stack overflow");
		goto did_handle;

	case EXCEPTION_FLT_UNDERFLOW:
		DeeError_Throwf(&DeeError_IntegerOverflow, "floating point underflow");
		goto did_handle;

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		DeeError_Throwf(&DeeError_DivideByZero, "Integer divide by zero");
		goto did_handle;

	case EXCEPTION_INT_OVERFLOW:
		DeeError_Throwf(&DeeError_IntegerOverflow, "Integer overflow");
		goto did_handle;

	case EXCEPTION_PRIV_INSTRUCTION:
		DeeError_Throwf(&DeeError_IllegalInstruction, "Privileged instruction");
		goto did_handle;

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		DeeError_Throwf(&DeeError_IllegalInstruction, "Illegal instruction");
		goto did_handle;

	case EXCEPTION_STACK_OVERFLOW:
		DeeError_Throwf(&DeeError_StackOverflow, "Stack overflow");
		goto did_handle;

	case EXCEPTION_INVALID_HANDLE:
		DeeError_Throwf(&DeeError_FileClosed, "Invalid handle");
		goto did_handle;

	default: break;
	}
done:
	return EXCEPTION_CONTINUE_SEARCH;
did_handle:
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif /* CONFIG_HAVE_CTYPES_SEH_GUARD */


#ifdef CONFIG_HAVE_CTYPES_KOS_GUARD
INTERN void ctypes_kos_guard(void) {
	struct exception_data *data;
	data = except_data();
	switch (data->e_class) {

#ifdef E_SEGFAULT
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_SEGFAULT)):
		DeeError_Throwf(&DeeError_SegFault, "Segmentation fault when %s %p",
		                data->e_subclass == EXCEPT_SUBCLASS(E_SEGFAULT_NOTEXECUTABLE)
		                ? "executing"
		                : data->e_subclass == EXCEPT_SUBCLASS(E_SEGFAULT_READONLY)
		                  ? "writing to"
		                  : "reading from",
		                data->e_args.e_segfault.s_addr);
		break;
#endif /* E_SEGFAULT */

#ifdef E_DIVIDE_BY_ZERO
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_DIVIDE_BY_ZERO)):
		DeeError_Throwf(&DeeError_DivideByZero, "Integer divide by zero");
		break;
#endif /* E_DIVIDE_BY_ZERO */

#ifdef E_OVERFLOW
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_OVERFLOW)):
		DeeError_Throwf(&DeeError_IntegerOverflow, "Integer overflow");
		break;
#endif /* E_OVERFLOW */

#ifdef E_ILLEGAL_INSTRUCTION
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_ILLEGAL_INSTRUCTION)):
#ifdef E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE
		if (data->e_subclass == EXCEPT_SUBCLASS(E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE)) {
			DeeError_Throwf(&DeeError_IllegalInstruction, "Privileged instruction");
			break;
		}
#endif /* E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE */
		DeeError_Throwf(&DeeError_IllegalInstruction, "Illegal instruction");
		break;
#endif /* E_ILLEGAL_INSTRUCTION */

#ifdef E_STACK_OVERFLOW
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_STACK_OVERFLOW)):
		DeeError_Throwf(&DeeError_StackOverflow, "Stack overflow");
		break;
#endif /* E_STACK_OVERFLOW */

#ifdef E_INDEX_ERROR
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_INDEX_ERROR)):
		DeeError_Throwf(&DeeError_IndexError, "Array bounds exceeded");
		break;
#endif /* E_INDEX_ERROR */

#ifdef E_INVALID_ALIGNMENT
	case EXCEPT_CLASS(EXCEPT_CODEOF(E_INVALID_ALIGNMENT)):
		DeeError_Throwf(&DeeError_SegFault, "Data misalignment");
		break;
#endif /* E_INVALID_ALIGNMENT */

	default:
		goto do_rethrow;
	}
	return;
do_rethrow:
	RETHROW();
}
#endif /* CONFIG_HAVE_CTYPES_KOS_GUARD */


/*[[[deemon (print_CMethod from rt.gen.unpack)("sizeof", """
	ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat
""", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_sizeof_params "ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_sizeof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_sizeof, &libctypes_sizeof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_sizeof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	CType *type;
	size_t result;
	if (DeeBytes_Check(ob))
		return DeeInt_NewSize(DeeBytes_SIZE(ob));
	type = CType_TypeOf(ob);
	if unlikely(!type)
		goto err;
	if (CType_IsCLValueType(type))
		type = CLValueType_LogicalPointedToType(CType_AsCLValueType(type));
	result = CType_Sizeof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("alignof", """
	ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat
""", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_alignof_params "ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_alignof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_alignof, &libctypes_alignof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_alignof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	size_t result;
	CType *type = CType_TypeOf(ob);
	if unlikely(!type)
		goto err;
	if (CType_IsCLValueType(type))
		type = CLValueType_LogicalPointedToType(CType_AsCLValueType(type));
	result = CType_Alignof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("typeof", """
	ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat
""", methodFlags: "METHOD_FCONSTCALL", returnType: "CType");]]]*/
#define libctypes_typeof_params "ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF CType *DCALL libctypes_typeof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_typeof, &libctypes_typeof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF CType *DCALL libctypes_typeof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	CType *result = CType_TypeOf(ob);
	if unlikely(!result)
		goto err;
	if (CType_IsCLValueType(result))
		result = CLValueType_LogicalPointedToType(CType_AsCLValueType(result));
	Dee_Incref(CType_AsType(result));
	return result;
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("intfor", """
	size_t size,
	bool $signed = true
""", methodFlags: "METHOD_FCONSTCALL", returnType: "CType");]]]*/
#define libctypes_intfor_params "size:?Dint,signed=!t"
FORCELOCAL WUNUSED DREF CType *DCALL libctypes_intfor_f_impl(size_t size, bool signed_);
PRIVATE WUNUSED DREF CType *DCALL libctypes_intfor_f(size_t argc, DeeObject *const *argv) {
	struct {
		size_t size;
		bool signed_;
	} args;
	args.signed_ = true;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "intfor", &args, &args.size, UNPuSIZ, DeeObject_AsSize, &args.signed_, "b", DeeObject_AsBool);
	return libctypes_intfor_f_impl(args.size, args.signed_);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(libctypes_intfor, &libctypes_intfor_f, METHOD_FCONSTCALL);
FORCELOCAL WUNUSED DREF CType *DCALL libctypes_intfor_f_impl(size_t size, bool signed_)
/*[[[end]]]*/
{
	CType *result;
	switch (size) {
	case 1: result = signed_ ? &CInt8_Type : &CUInt8_Type; break;
	case 2: result = signed_ ? &CInt16_Type : &CUInt16_Type; break;
	case 4: result = signed_ ? &CInt32_Type : &CUInt32_Type; break;
	case 8: result = signed_ ? &CInt64_Type : &CUInt64_Type; break;
	case 16: result = signed_ ? &CInt128_Type : &CUInt128_Type; break;
#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	case CTYPES_sizeof_char: result = signed_ ? &CSChar_Type : &CUChar_Type; break;
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */
#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	case CTYPES_sizeof_short: result = signed_ ? &CShort_Type : &CUShort_Type; break;
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */
#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	case CTYPES_sizeof_int: result = signed_ ? &CInt_Type : &CUInt_Type; break;
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */
#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
#if CTYPES_sizeof_long != CTYPES_sizeof_int
	case CTYPES_sizeof_long: result = signed_ ? &CLong_Type : &CULong_Type; break;
#endif /* CTYPES_sizeof_long != CTYPES_sizeof_int */
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */
#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	case CTYPES_sizeof_llong: result = signed_ ? &CLLong_Type : &CULLong_Type; break;
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
	default:
		DeeError_Throwf(&DeeError_ValueError,
		                "No C integer type with a width of `%" PRFuSIZ "' bytes exists",
		                size);
		return NULL;
	}
	Dee_Incref(CType_AsType(result));
	return result;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap_intfor", """
	size_t size,
	bool $signed = true
""", methodFlags: "METHOD_FCONSTCALL", returnType: "CType");]]]*/
#define libctypes_bswap_intfor_params "size:?Dint,signed=!t"
FORCELOCAL WUNUSED DREF CType *DCALL libctypes_bswap_intfor_f_impl(size_t size, bool signed_);
PRIVATE WUNUSED DREF CType *DCALL libctypes_bswap_intfor_f(size_t argc, DeeObject *const *argv) {
	struct {
		size_t size;
		bool signed_;
	} args;
	args.signed_ = true;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "bswap_intfor", &args, &args.size, UNPuSIZ, DeeObject_AsSize, &args.signed_, "b", DeeObject_AsBool);
	return libctypes_bswap_intfor_f_impl(args.size, args.signed_);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(libctypes_bswap_intfor, &libctypes_bswap_intfor_f, METHOD_FCONSTCALL);
FORCELOCAL WUNUSED DREF CType *DCALL libctypes_bswap_intfor_f_impl(size_t size, bool signed_)
/*[[[end]]]*/
{
	CType *result;
	switch (size) {
	case 1: result = signed_ ? &CBSwapInt8_Type : &CBSwapUInt8_Type; break;
	case 2: result = signed_ ? &CBSwapInt16_Type : &CBSwapUInt16_Type; break;
	case 4: result = signed_ ? &CBSwapInt32_Type : &CBSwapUInt32_Type; break;
	case 8: result = signed_ ? &CBSwapInt64_Type : &CBSwapUInt64_Type; break;
	case 16: result = signed_ ? &CBSwapInt128_Type : &CBSwapUInt128_Type; break;
	default:
		DeeError_Throwf(&DeeError_ValueError,
		                "No C integer type with a width of `%" PRFuSIZ "' bytes exists",
		                size);
		return NULL;
	}
	Dee_Incref(CType_AsType(result));
	return result;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("union", """
	fields:?S?X2?T2?Dstring?DType?GStructType,
	bool packed = false,
	size_t alignment = 1,
""", methodFlags: "METHOD_FCONSTCALL", returnType: "CStructType");]]]*/
#define libctypes_union_params "fields:?S?X2?T2?Dstring?DType?GStructType,packed=!f,alignment=!1"
FORCELOCAL WUNUSED NONNULL((1)) DREF CStructType *DCALL libctypes_union_f_impl(DeeObject *fields, bool packed_, size_t alignment);
PRIVATE WUNUSED DREF CStructType *DCALL libctypes_union_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *fields;
		bool packed_;
		size_t alignment;
	} args;
	args.packed_ = false;
	args.alignment = 1;
	if (DeeArg_UnpackStruct(argc, argv, "o|b" UNPuSIZ ":union", &args))
		goto err;
	return libctypes_union_f_impl(args.fields, args.packed_, args.alignment);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(libctypes_union, &libctypes_union_f, METHOD_FCONSTCALL);
FORCELOCAL WUNUSED NONNULL((1)) DREF CStructType *DCALL libctypes_union_f_impl(DeeObject *fields, bool packed_, size_t alignment)
/*[[[end]]]*/
{
	unsigned int flags = CSTRUCTTYPE_F_UNION;
	if (packed_)
		flags |= CSTRUCTTYPE_F_PACKED;
	return CStructType_Of(fields, flags, alignment);
}



/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap16", """
	uint16_t x:?Guint16_t
""", isconst: true, returnType: "CObject");]]]*/
#define libctypes_bswap16_params "x:?Guint16_t"
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap16_f_impl(uint16_t x);
PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL libctypes_bswap16_f(DeeObject *__restrict arg0) {
	uint16_t x;
	if (DeeObject_AsUInt16(arg0, &x))
		goto err;
	return libctypes_bswap16_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap16, &libctypes_bswap16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap16_f_impl(uint16_t x)
/*[[[end]]]*/
{
	return CUInt16_New(BSWAP16(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap32", """
	uint32_t x:?Guint32_t
""", isconst: true, returnType: "CObject");]]]*/
#define libctypes_bswap32_params "x:?Guint32_t"
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap32_f_impl(uint32_t x);
PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL libctypes_bswap32_f(DeeObject *__restrict arg0) {
	uint32_t x;
	if (DeeObject_AsUInt32(arg0, &x))
		goto err;
	return libctypes_bswap32_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap32, &libctypes_bswap32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap32_f_impl(uint32_t x)
/*[[[end]]]*/
{
	return CUInt32_New(BSWAP32(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap64", """
	uint64_t x:?Guint64_t
""", isconst: true, returnType: "CObject");]]]*/
#define libctypes_bswap64_params "x:?Guint64_t"
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap64_f_impl(uint64_t x);
PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL libctypes_bswap64_f(DeeObject *__restrict arg0) {
	uint64_t x;
	if (DeeObject_AsUInt64(arg0, &x))
		goto err;
	return libctypes_bswap64_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap64, &libctypes_bswap64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap64_f_impl(uint64_t x)
/*[[[end]]]*/
{
	return CUInt64_New(BSWAP64(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap128", """
	Dee_uint128_t x:?Guint128_t
""", isconst: true, returnType: "CObject");]]]*/
#define libctypes_bswap128_params "x:?Guint128_t"
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap128_f_impl(Dee_uint128_t x);
PRIVATE WUNUSED NONNULL((1)) DREF CObject *DCALL libctypes_bswap128_f(DeeObject *__restrict arg0) {
	Dee_uint128_t x;
	if (DeeObject_AsUInt128(arg0, &x))
		goto err;
	return libctypes_bswap128_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap128, &libctypes_bswap128_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF CObject *DCALL libctypes_bswap128_f_impl(Dee_uint128_t x)
/*[[[end]]]*/
{
	Dee_uint128_t res;
#ifdef BSWAP128
	res = BSWAP128(x);
#else /* BSWAP128 */
	__hybrid_uint128_setword64(res, 0, BSWAP64(__hybrid_uint128_getword64(x, 1)));
	__hybrid_uint128_setword64(res, 1, BSWAP64(__hybrid_uint128_getword64(x, 0)));
#endif /* !BSWAP128 */
	return CUInt128_New(res);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define libctypes_htole16  CUInt16_Type.ct_base
#define libctypes_letoh16  CUInt16_Type.ct_base
#define libctypes_htole32  CUInt32_Type.ct_base
#define libctypes_letoh32  CUInt32_Type.ct_base
#define libctypes_htole64  CUInt64_Type.ct_base
#define libctypes_letoh64  CUInt64_Type.ct_base
#define libctypes_htole128 CUInt128_Type.ct_base
#define libctypes_letoh128 CUInt128_Type.ct_base
#define libctypes_htobe16  libctypes_bswap16
#define libctypes_betoh16  libctypes_bswap16
#define libctypes_htobe32  libctypes_bswap32
#define libctypes_betoh32  libctypes_bswap32
#define libctypes_htobe64  libctypes_bswap64
#define libctypes_betoh64  libctypes_bswap64
#define libctypes_htobe128 libctypes_bswap128
#define libctypes_betoh128 libctypes_bswap128
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define libctypes_htole16  libctypes_bswap16
#define libctypes_letoh16  libctypes_bswap16
#define libctypes_htole32  libctypes_bswap32
#define libctypes_letoh32  libctypes_bswap32
#define libctypes_htole64  libctypes_bswap64
#define libctypes_letoh64  libctypes_bswap64
#define libctypes_htole128 libctypes_bswap128
#define libctypes_letoh128 libctypes_bswap128
#define libctypes_htobe16  CUInt16_Type.ct_base
#define libctypes_betoh16  CUInt16_Type.ct_base
#define libctypes_htobe32  CUInt32_Type.ct_base
#define libctypes_betoh32  CUInt32_Type.ct_base
#define libctypes_htobe64  CUInt64_Type.ct_base
#define libctypes_betoh64  CUInt64_Type.ct_base
#define libctypes_htobe128 CUInt128_Type.ct_base
#define libctypes_betoh128 CUInt128_Type.ct_base
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported `__BYTE_ORDER__'"
#endif /* __BYTE_ORDER__ != ... */


#ifndef NDEBUG
PRIVATE void DCALL
libctypes_fini_type(CType *__restrict self) {
	Dee_Free(self->ct_arrays.sa_list);
#ifndef CONFIG_NO_CFUNCTION
	Dee_Free(self->ct_functions.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */
}

#define PTR_libctypes_fini &libctypes_fini
PRIVATE void DCALL libctypes_fini(void) {
	/* Clear caches for static C-types, so they don't show up as leaks. */
	libctypes_fini_type(&AbstractCObject_Type);
	libctypes_fini_type(CPointerType_AsCType(&AbstractCPointer_Type));
	libctypes_fini_type(CLValueType_AsCType(&AbstractCLValue_Type));
	libctypes_fini_type(CArrayType_AsCType(&AbstractCArray_Type));
	libctypes_fini_type(CFunctionType_AsCType(&AbstractCFunction_Type));
	libctypes_fini_type(&CVoid_Type);
	libctypes_fini_type(&CChar_Type);
	libctypes_fini_type(&CWChar_Type);
	libctypes_fini_type(&CChar16_Type);
	libctypes_fini_type(&CChar32_Type);
	libctypes_fini_type(&CBool_Type);
	libctypes_fini_type(&CInt8_Type);
	libctypes_fini_type(&CInt16_Type);
	libctypes_fini_type(&CInt32_Type);
	libctypes_fini_type(&CInt64_Type);
	libctypes_fini_type(&CUInt8_Type);
	libctypes_fini_type(&CUInt16_Type);
	libctypes_fini_type(&CUInt32_Type);
	libctypes_fini_type(&CUInt64_Type);
	libctypes_fini_type(&CFloat_Type);
	libctypes_fini_type(&CDouble_Type);
	libctypes_fini_type(&CLDouble_Type);

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	libctypes_fini_type(&CSChar_Type);
	libctypes_fini_type(&CUChar_Type);
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	libctypes_fini_type(&CShort_Type);
	libctypes_fini_type(&CUShort_Type);
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	libctypes_fini_type(&CInt_Type);
	libctypes_fini_type(&CUInt_Type);
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
	libctypes_fini_type(&CLong_Type);
	libctypes_fini_type(&CULong_Type);
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	libctypes_fini_type(&CLLong_Type);
	libctypes_fini_type(&CULLong_Type);
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
}
#endif /* !NDEBUG */

#ifndef PTR_libctypes_fini
#define PTR_libctypes_fini NULL
#endif /* !PTR_libctypes_fini */

/* Define some magic constants that may be of interest to user-code. */
/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "ctypes";

include("constants.def");
gi("BYTE_ORDER", "__BYTE_ORDER__");
gi("FLOAT_WORD_ORDER", "__FLOAT_WORD_ORDER__");
gi("ORDER_LITTLE_ENDIAN", "__ORDER_LITTLE_ENDIAN__");
gi("ORDER_BIG_ENDIAN", "__ORDER_BIG_ENDIAN__");
gi("ORDER_PDP_ENDIAN", "__ORDER_PDP_ENDIAN__");
]]]*/
#include "constants.def"
/*[[[end]]]*/


DEX_BEGIN

/* Export the underlying type-system used by ctypes. */
DEX_MEMBER_F_NODOC("CType", &CType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("PointerType", &CPointerType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("LValueType", &CLValueType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ArrayType", &CArrayType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("StructType", &CStructType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("FunctionType", &CFunctionType_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("CObject", CType_AsType(&AbstractCObject_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("Pointer", CPointerType_AsType(&AbstractCPointer_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("LValue", CLValueType_AsType(&AbstractCLValue_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("Array", CArrayType_AsType(&AbstractCArray_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("Struct", CStructType_AsType(&AbstractCStruct_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("Function", CFunctionType_AsType(&AbstractCFunction_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("struct", &CStructType_Type, Dee_DEXSYM_READONLY), /* Alias for "StructType" */
DEX_MEMBER_F("union", &libctypes_union, Dee_DEXSYM_READONLY,
             "(" libctypes_union_params ")->?GStructType\n"
             "#palignment{Minimum alignment of returned struct (if non-empty)}"
             "Convenience alias for ?GStructType's constructor with ${union: true}"),

/* TODO: Both Pointer and LValue types need 1 sub-class each: RefPointer and RefLValue
 *       These sub-classes behave the same as the original Pointer/LValue-type, except
 *       that they don't have constructors, and instance have 1 additional field that
 *       follows after the underlying pointer: `DREF DeeObject *ob_ref; // [1..1]'.
 * This field is a reference to the owning object. These extra types are then used
 * whenever you do a ctypes operation where it is clear which object a pointer/lvalue
 * points into. e.g.:
 * >> union foo {
 * >>     .s = int32_t,
 * >>     .u = uint32_t,
 * >> };
 * >> local x = (foo){ .u = 0x80000000 };
 * >> local p1 = x.s.ptr;
 * >> local p2 = ((foo){ .u = 0x40000000 }).s.ptr;
 * >> print p1.ind.hex();
 * >> print p2.ind.hex();
 * Here, both `p1' and `p2' are RefPointer-objects, with `p1' holding a reference
 * to `x', and `p2' holding a reference to `(foo){ .u = 0x40000000 }'. This way,
 * using ctypes in user-code becomes much easier, as there's no need to make sure
 * that the objects pointed-to by pointer/lvalue instances stay alive for as long
 * as the pointer/lvalue type does (Because currently, `(foo){ .u = 0x40000000 }'
 * has already been destroyed by the time `print p2.ind.hex();' tries to use it)
 */

/* A wrapper around the native shared-library loader. */
DEX_MEMBER_F_NODOC("ShLib", &ShLib_Type, Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("dlopen", &ShLib_Type, Dee_DEXSYM_READONLY), /* Convenience alias for `ShLib' */

/* Export all the C-types. */
DEX_MEMBER_F_NODOC("void", CType_AsType(&CVoid_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("char", CType_AsType(&CChar_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("wchar_t", CType_AsType(&CWChar_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("char16_t", CType_AsType(&CChar16_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("char32_t", CType_AsType(&CChar32_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bool", CType_AsType(&CBool_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int8_t", CType_AsType(&CInt8_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int16_t", CType_AsType(&CInt16_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int32_t", CType_AsType(&CInt32_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int64_t", CType_AsType(&CInt64_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int128_t", CType_AsType(&CInt128_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint8_t", CType_AsType(&CUInt8_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint16_t", CType_AsType(&CUInt16_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint32_t", CType_AsType(&CUInt32_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint64_t", CType_AsType(&CUInt64_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint128_t", CType_AsType(&CUInt128_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_int8_t", CType_AsType(&CBSwapInt8_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_int16_t", CType_AsType(&CBSwapInt16_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_int32_t", CType_AsType(&CBSwapInt32_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_int64_t", CType_AsType(&CBSwapInt64_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_int128_t", CType_AsType(&CBSwapInt128_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_uint8_t", CType_AsType(&CBSwapUInt8_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_uint16_t", CType_AsType(&CBSwapUInt16_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_uint32_t", CType_AsType(&CBSwapUInt32_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_uint64_t", CType_AsType(&CBSwapUInt64_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("bswap_uint128_t", CType_AsType(&CBSwapUInt128_Type), Dee_DEXSYM_READONLY),

/* Endian-specific integer types (e.g. `le16' and `be16')
 * These could be used in structures and always encode/decode
 * the underlying value to/from a regular deemon `int' object
 * to automatically apply the necessary endian conversion
 *
 * >> import * from ctypes;
 * >> union foo = {
 * >> 	.le = le32,
 * >> 	.be = be32,
 * >> };
 * >> x = foo(le: 0x12345678);
 * >> print x.be; // 0x78563412 */
DEX_MEMBER_F_NODOC("le8", CType_AsType(&CLeUIntN_Type(1)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("le16", CType_AsType(&CLeUIntN_Type(2)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("le32", CType_AsType(&CLeUIntN_Type(4)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("le64", CType_AsType(&CLeUIntN_Type(8)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("le128", CType_AsType(&CLeUIntN_Type(16)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("be8", CType_AsType(&CBeUIntN_Type(1)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("be16", CType_AsType(&CBeUIntN_Type(2)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("be32", CType_AsType(&CBeUIntN_Type(4)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("be64", CType_AsType(&CBeUIntN_Type(8)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("be128", CType_AsType(&CBeUIntN_Type(16)), Dee_DEXSYM_READONLY),

DEX_MEMBER_F_NODOC("float", CType_AsType(&CFloat_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("double", CType_AsType(&CDouble_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ldouble", CType_AsType(&CLDouble_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("schar", CType_AsType(&CSChar_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uchar", CType_AsType(&CUChar_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("short", CType_AsType(&CShort_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ushort", CType_AsType(&CUShort_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("int", CType_AsType(&CInt_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uint", CType_AsType(&CUInt_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("long", CType_AsType(&CLong_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ulong", CType_AsType(&CULong_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("llong", CType_AsType(&CLLong_Type), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ullong", CType_AsType(&CULLong_Type), Dee_DEXSYM_READONLY),

/* Other, platform-specific C-types. */
DEX_MEMBER_F_NODOC("size_t", CType_AsType(&CUIntN_Type(__SIZEOF_SIZE_T__)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ssize_t", CType_AsType(&CIntN_Type(__SIZEOF_SIZE_T__)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("ptrdiff_t", CType_AsType(&CIntN_Type(__SIZEOF_PTRDIFF_T__)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("intptr_t", CType_AsType(&CIntN_Type(__SIZEOF_POINTER__)), Dee_DEXSYM_READONLY),
DEX_MEMBER_F_NODOC("uintptr_t", CType_AsType(&CUIntN_Type(__SIZEOF_POINTER__)), Dee_DEXSYM_READONLY),

/* Export some helper functions */
DEX_MEMBER_F("sizeof", &libctypes_sizeof, Dee_DEXSYM_READONLY,
             "(" libctypes_sizeof_params ")->?Dint\n"
             "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
             "Returns the size of a given structured type or object in bytes\n"
             "\n"

             "(ob:?DBytes)->?Dint\n"
             "Returns the size of the given ?DBytes ob, which is the same as ${##ob}"),
DEX_MEMBER_F("alignof", &libctypes_alignof, Dee_DEXSYM_READONLY,
             "(" libctypes_alignof_params ")->?Dint\n"
             "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
             "Returns the alignment of a given structured type or object in bytes"),
DEX_MEMBER_F("typeof", &libctypes_typeof, Dee_DEXSYM_READONLY,
             "(ob:?X7?GCType?GCObject?DType?N?Dbool?Dint?Dfloat)->?GCType\n"
             "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
             "Returns the type of a given structured type or object"),
DEX_MEMBER_F("intfor", &libctypes_intfor, Dee_DEXSYM_READONLY,
             "(" libctypes_intfor_params ")->?GCType\n"
             "#tValueError{No integer matching the requirements of @size is supported}"),
DEX_MEMBER_F("bswap_intfor", &libctypes_bswap_intfor, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap_intfor_params ")->?GCType\n"
             "#tValueError{No integer matching the requirements of @size is supported}\n"
             "Same as ?Gintfor, but returns the relevant byte-swapped type "
             /**/ "instead (e.g. ?Gbswap_int32_t instead of ?Gint32_t)"),
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define libctypes_leintfor        libctypes_intfor
#define libctypes_leintfor_params libctypes_intfor_params
#define libctypes_beintfor        libctypes_bswap_intfor
#define libctypes_beintfor_params libctypes_bswap_intfor_params
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define libctypes_leintfor        libctypes_bswap_intfor
#define libctypes_leintfor_params libctypes_bswap_intfor_params
#define libctypes_beintfor        libctypes_intfor
#define libctypes_beintfor_params libctypes_intfor_params
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported `__BYTE_ORDER__'"
#endif /* __BYTE_ORDER__ != ... */
DEX_MEMBER_F("leintfor", &libctypes_leintfor, Dee_DEXSYM_READONLY,
             "(" libctypes_leintfor_params ")->?GCType\n"
             "#tValueError{No integer matching the requirements of @size is supported}\n"
             "Return the relevant little-endian integer type (s.a. ?Gintfor, ?Gbswap_intfor, ?GBYTE_ORDER)"),
DEX_MEMBER_F("beintfor", &libctypes_beintfor, Dee_DEXSYM_READONLY,
             "(" libctypes_beintfor_params ")->?GCType\n"
             "#tValueError{No integer matching the requirements of @size is supported}\n"
             "Return the relevant big-endian integer type (s.a. ?Gintfor, ?Gbswap_intfor, ?GBYTE_ORDER)"),

DEX_MEMBER_F("bswap16", &libctypes_bswap16, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap16_params ")->?Guint16_t\n"
             "Return @x with reversed endian"),
DEX_MEMBER_F("bswap32", &libctypes_bswap32, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap32_params ")->?Guint32_t\n"
             "Return @x with reversed endian"),
DEX_MEMBER_F("bswap64", &libctypes_bswap64, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap64_params ")->?Guint64_t\n"
             "Return @x with reversed endian"),
DEX_MEMBER_F("bswap128", &libctypes_bswap128, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap128_params ")->?Guint128_t\n"
             "Return @x with reversed endian"),

DEX_MEMBER_F("htole16", &libctypes_htole16, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap16_params ")->?Guint16_t\n"
             "Convert a 16-bit integer from host-endian to little-endian"),
DEX_MEMBER_F("letoh16", &libctypes_letoh16, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap16_params ")->?Guint16_t\n"
             "Convert a 16-bit integer from little-endian to host-endian"),
DEX_MEMBER_F("htole32", &libctypes_htole32, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap32_params ")->?Guint32_t\n"
             "Convert a 32-bit integer from host-endian to little-endian"),
DEX_MEMBER_F("letoh32", &libctypes_letoh32, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap32_params ")->?Guint32_t\n"
             "Convert a 32-bit integer from little-endian to host-endian"),
DEX_MEMBER_F("htole64", &libctypes_htole64, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap64_params ")->?Guint64_t\n"
             "Convert a 64-bit integer from host-endian to little-endian"),
DEX_MEMBER_F("letoh64", &libctypes_letoh64, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap64_params ")->?Guint64_t\n"
             "Convert a 64-bit integer from little-endian to host-endian"),
DEX_MEMBER_F("htole128", &libctypes_htole128, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap128_params ")->?Guint128_t\n"
             "Convert a 128-bit integer from host-endian to little-endian"),
DEX_MEMBER_F("letoh128", &libctypes_letoh128, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap128_params ")->?Guint128_t\n"
             "Convert a 128-bit integer from little-endian to host-endian"),

DEX_MEMBER_F("htobe16", &libctypes_htobe16, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap16_params ")->?Guint16_t\n"
             "Convert a 16-bit integer from host-endian to big-endian"),
DEX_MEMBER_F("betoh16", &libctypes_betoh16, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap16_params ")->?Guint16_t\n"
             "Convert a 16-bit integer from big-endian to host-endian"),
DEX_MEMBER_F("htobe32", &libctypes_htobe32, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap32_params ")->?Guint32_t\n"
             "Convert a 32-bit integer from host-endian to big-endian"),
DEX_MEMBER_F("betoh32", &libctypes_betoh32, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap32_params ")->?Guint32_t\n"
             "Convert a 32-bit integer from big-endian to host-endian"),
DEX_MEMBER_F("htobe64", &libctypes_htobe64, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap64_params ")->?Guint64_t\n"
             "Convert a 64-bit integer from host-endian to big-endian"),
DEX_MEMBER_F("betoh64", &libctypes_betoh64, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap64_params ")->?Guint64_t\n"
             "Convert a 64-bit integer from big-endian to host-endian"),
DEX_MEMBER_F("htobe128", &libctypes_htobe128, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap128_params ")->?Guint128_t\n"
             "Convert a 128-bit integer from host-endian to big-endian"),
DEX_MEMBER_F("betoh128", &libctypes_betoh128, Dee_DEXSYM_READONLY,
             "(" libctypes_bswap128_params ")->?Guint128_t\n"
             "Convert a 128-bit integer from big-endian to host-endian"),

/* <string.h> & <stdlib.h> - style ctypes functions */
DEX_MEMBER_F("malloc", &c_malloc_malloc, Dee_DEXSYM_READONLY,
             "(size:?Dint)->?Aptr?Gvoid\n"
             "#tNoMemory{Insufficient memory to allocate @size bytes}"
             "Allocate a raw buffer of @size bytes from a heap and return a "
             /**/ "pointer to its base\n"
             "Passing a value of $0 for @size will allocate a minimal-sized heap-block"),
DEX_MEMBER_F("calloc", &c_malloc_calloc, Dee_DEXSYM_READONLY,
             "(size:?Dint)->?Aptr?Gvoid\n"
             "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
             "#tNoMemory{Insufficient memory to allocate ${count * size} bytes}"
             "#tIntegerOverflow{The given @count and @size overflow ?Gsize_t when multiplied}"
             "Same as ?Gmalloc, but rather than leaving the newly allocated memory uninitialized, "
             /**/ "fill it with all zeroes, the same way ${memset(malloc(size), 0, size)} would\n"
             /**/ "If the product of @count and @size equals ${0}, a minimal-sized heap-block is allocated, "
             /**/ "however the caller must still assume that the memory range they are allowed to access "
             /**/ "is non-existent"),
DEX_MEMBER_F("realloc", &c_malloc_realloc, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "#tNoMemory{Insufficient memory to allocate @size bytes}"
             "Given a heap-pointer previously allocated using either ?Gmalloc, ?Gcalloc or "
             /**/ "a prior call to ?Grealloc, change its size to @size, either releasing then "
             /**/ "unused trailing memory resulting from the difference between its old size "
             /**/ "and a smaller, newer size, or try to extend it if its new size is larger "
             /**/ "than its own, in which case a collision with another block located at the "
             /**/ "location where the extension attempt was made will result in an entirely "
             /**/ "new block being allocated, with all pre-existing data being copied inside.\n"
             "In all cases, a pointer to the new heap block is returned, which may be "
             /**/ "identical to the old block\n"
             "If a NULL-pointer is passed for @ptr, the function behaves the same as "
             /**/ "a call to ?Gmalloc with the given @{size}. Alternatively, if a valid heap "
             /**/ "pointer is passed for @ptr, and @size is passed as ${0}, the heap block is "
             /**/ "truncated to a minimal size and a heap block is returned that is semantically "
             /**/ "equivalent to one returned by ${malloc(0)}\n"
             "In the event of failure, the pre-existing heap-block passed for @ptr will remain unchanged"),
DEX_MEMBER_F("free", &c_malloc_free, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?Gvoid)\n"
             "Release a previously allocated heap-block returned by one of ?Gmalloc, ?Gcalloc or ?Grealloc\n"
             "If a NULL-pointer is passed for @ptr, this function has no effect and returns immediately"),
DEX_MEMBER_F("trymalloc", &c_malloc_trymalloc, Dee_DEXSYM_READONLY,
             "(size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmalloc, but return a NULL-pointer if allocation isn't "
             /**/ "possible due to lack of memory, rather than throwing a :NoMemory error"),
DEX_MEMBER_F("trycalloc", &c_malloc_trycalloc, Dee_DEXSYM_READONLY,
             "(size:?Dint)->?Aptr?Gvoid\n"
             "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gcalloc, but return a NULL-pointer if allocation isn't "
             /**/ "possible due to lack of memory, rather than throwing a :NoMemory error"),
DEX_MEMBER_F("tryrealloc", &c_malloc_tryrealloc, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Grealloc, but return a NULL-pointer if allocation isn't "
             /**/ "possible due to lack of memory, rather than throwing a :NoMemory error\n"
             "In this event, the pre-existing heap-block passed for @ptr is not freed"),
DEX_MEMBER_F("strdup", &c_malloc_strdup, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint=!A!Pmax!Gsize_t)->?Aptr?Gchar\n"
             "#tNoMemory{Insufficient memory}"
             "Duplicate the given @str into a heap-allocated memory block\n"
             "${"
             /**/ "function strndup(str: char.ptr, maxlen?: int): char.ptr {\n"
             /**/ "	if (maxlen !is bound)\n"
             /**/ "		maxlen = size_t.max;\n"
             /**/ "	local len = strnlen(str, maxlen) * sizeof(char);\n"
             /**/ "	local res = (char.ptr)memcpy(malloc(len + sizeof(char)), str, len);\n"
             /**/ "	res[len] = 0;\n"
             /**/ "	return res;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("trystrdup", &c_malloc_trystrdup, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint=!A!Pmax!Gsize_t)->?Aptr?Gchar\n"
             "Try to duplicate the given @str into a heap-allocated memory block\n"
             "${"
             /**/ "function trystrndup(str: char.ptr, maxlen?: int): char.ptr {\n"
             /**/ "	if (maxlen !is bound)\n"
             /**/ "		maxlen = size_t.max;\n"
             /**/ "	local len = strnlen(str, maxlen) * sizeof(char);\n"
             /**/ "	local res = (char.ptr)trymalloc(len + sizeof(char));\n"
             /**/ "	if (res) {\n"
             /**/ "		memcpy(res, str, len);\n"
             /**/ "		res[len] = 0;\n"
             /**/ "	}\n"
             /**/ "	return res;\n"
             /**/ "}"
             "}"),
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
DEX_MEMBER_F("malloc_usable_size", &c_malloc_malloc_usable_size, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?Gvoid)->?Dint\n"
             "Return the usable amount of heap-memory available via @ptr\n"
             "The is the same number of bytes as would be copied if ?Grealloc "
             /**/ "needed to allocate a new heap block when given @ptr"),
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

DEX_MEMBER_F("memcpy", &c_string_memcpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
             "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
             "Copies @size * @count bytes of memory from @src to @dst\n"
             "Note that the source and destination ranges may not overlap"),
DEX_MEMBER_F("mempcpy", &c_string_mempcpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
             "#r{Always re-returns ${dst + size * count} as a ?Aptr?Gvoid}"
             "Same as ?Gmemcpy, but returns ${dst + size * count}"),
DEX_MEMBER_F("memccpy", &c_string_memccpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,size:?Dint)->?Aptr?Gvoid\n"
             "#r{The last modified by in @dst}"
             "Same as ?Gmemcpy, but stop after a byte equal to @needle is encountered in @src"),
DEX_MEMBER_F("memset", &c_string_memset, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
             "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
             "Set every byte in the range @dst+@size to equal @byte"),
DEX_MEMBER_F("mempset", &c_string_mempset, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
             "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
             "Same as ?Gmemset, but returns ${dst + size}"),
DEX_MEMBER_F("bzero", &c_string_bzero, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,size:?Dint)\n"
             "(dst:?Aptr?Gvoid,count:?Dint,size:?Dint)\n"
             "Same as ?Gmemset, but always fills memory with ${0}s"),
DEX_MEMBER_F("memmove", &c_string_memmove, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
             "Same as ?Gmemcpy, but the source and destination ranges are allowed to overlap"),
DEX_MEMBER_F("mempmove", &c_string_mempmove, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
             "Same as ?Gmemcpy, but returns ${dst + size}"),
DEX_MEMBER_F("memchr", &c_string_memchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Return a pointer to the first byte in the specified @haystack+@haystack_size "
             /**/ "that equals @needle, or return a NULL-pointer if not found"),
DEX_MEMBER_F("memrchr", &c_string_memrchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as :memchr, but if @needle appears multiple times, return a pointer to the last instance"),
DEX_MEMBER_F("memlen", &c_string_memlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
             "Return the offset from @haystack of the first byte equal to @needle, or @haystack_size if now found"),
DEX_MEMBER_F("memrlen", &c_string_memrlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
             "Same as :memlen, but if @needle appears multiple times, return the offset of the last instance"),
DEX_MEMBER_F("memend", &c_string_memend, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Return a pointer to the first byte in @haystack that is equal to @needle, or @haystack+@haystack_size if now found"),
DEX_MEMBER_F("memrend", &c_string_memrend, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemend, but if @needle appears multiple times, return a pointer to the last instance\n"
             "If @needle doesn't appear at all, return a pointer to @haystack"),
DEX_MEMBER_F("rawmemchr", &c_string_rawmemchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
             "Search for the byte equal to @needle, starting at @haystack and only "
             /**/ "stopping once one is found, or unmapped memory is reached and "
             /**/ "the host provides support for a MMU"),
DEX_MEMBER_F("rawmemlen", &c_string_rawmemlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
             "Same as ?Grawmemchr, but return the offset from @haystack"),
DEX_MEMBER_F("rawmemrchr", &c_string_rawmemrchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
             "Same as :rawmemchr, but search in reverse, starting with ${haystack[-1]}\n"
             "You can think of this function as a variant of ?Gmemrend that operates "
             /**/ "on a buffer that spans the entirety of the available address space"),
DEX_MEMBER_F("rawmemrlen", &c_string_rawmemrlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
             "Same as ?Grawmemrchr, but return the positive (unsigned) offset of the "
             /**/ "matching byte, such that ${haystack + return} points to the byte in question"),
DEX_MEMBER_F("memxchr", &c_string_memxchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemchr, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("memxlen", &c_string_memxlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
             "Same as ?Gmemlen, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("memxend", &c_string_memxend, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemend, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("memrxchr", &c_string_memrxchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemrchr, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("memrxlen", &c_string_memrxlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
             "Same as ?Gmemrlen, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("memrxend", &c_string_memrxend, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemrend, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("rawmemxchr", &c_string_rawmemxchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Grawmemchr, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("rawmemxlen", &c_string_rawmemxlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
             "Same as ?Grawmemlen, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("rawmemrxchr", &c_string_rawmemrxchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Grawmemrchr, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("rawmemrxlen", &c_string_rawmemrxlen, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
             "Same as ?Grawmemrlen, but instead of comparing bytes for being equal, compare them for being different"),
DEX_MEMBER_F("bcmp", &c_string_bcmp, Dee_DEXSYM_READONLY,
             "(a:?Aptr?Gvoid,b:?Aptr?Gvoid,size:?Dint)->?Dint\n"
             "Same as ?Gmemcmp, but only returns $0 when buffer are equal, and non-$0 otherwise"),
DEX_MEMBER_F("memcmp", &c_string_memcmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
             "Compare bytes from 2 buffers in @lhs and @rhs of equal @size, and "
             /**/ "search for the first non-matching byte, returning ${< 0} if that byte "
             /**/ "in @lhs is smaller than its counterpart in @rhs, ${> 0} if the opposite "
             /**/ "is true, and ${== 0} no such byte exists"),
DEX_MEMBER_F("memcasecmp", &c_string_memcasecmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
             "Same as ?Gmemcmp, but bytes are casted as ASCII characters "
             /**/ "into a common casing prior to comparison"),
DEX_MEMBER_F("memmem", &c_string_memmem, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
             "Search for the first memory block in @haystack+@haystack_size that is equal to @needle+@needle_size "
             /**/ "such that ${memcmp(candidate, needle, needle_size) == 0} and return a pointer to its starting "
             /**/ "location in @haystack\n"
             "If no such memory block exists, or @needle_size is $0, return a NULL-pointer instead"),
DEX_MEMBER_F("memcasemem", &c_string_memcasemem, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemmem, but perform case-insensitive comparisons, using ?Gmemcasecmp instead of ?Gmemcmp"),
DEX_MEMBER_F("memrmem", &c_string_memrmem, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemmem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
             "When @needle_size is $0, always return a NULL-pointer"),
DEX_MEMBER_F("memcasermem", &c_string_memcasermem, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
             "Same as ?Gmemcasemem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
             "When @needle_size is $0, always return a NULL-pointer"),
DEX_MEMBER_F("memrev", &c_string_memrev, Dee_DEXSYM_READONLY,
             "(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "Reverse the order of bytes in the given @buf+@size, such that upon return its first "
             /**/ "byte contains the old value of the last byte, and the last byte the value of the first, "
             /**/ "and so on."),
DEX_MEMBER_F("memfrob", &c_string_memfrob, Dee_DEXSYM_READONLY,
             "(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
             "xor all bytes within @buf with $42, implementing _very_ simplistic encryption"),

DEX_MEMBER_F("strlen", &c_string_strlen, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Dint\n"
             "Returns the length of a given @str in characters"),
DEX_MEMBER_F("strend", &c_string_strend, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Return a pointer to the end of a given @str"),
DEX_MEMBER_F("strnlen", &c_string_strnlen, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
             "Same as ?Gstrlen, but limit the max number of scanned characters to @maxlen"),
DEX_MEMBER_F("strnend", &c_string_strnend, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrend, but limit the max number of scanned characters to @maxlen"),
DEX_MEMBER_F("strchr", &c_string_strchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Search for the first character equal to @needle within @haystack\n"
             "If no such character exists, return a NULL-pointer instead"),
DEX_MEMBER_F("strrchr", &c_string_strrchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Search for the last character equal to @needle within @haystack\n"
             "If no such character exists, return a NULL-pointer instead"),
DEX_MEMBER_F("strnchr", &c_string_strnchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrchr, but limit the max number of scanned characters to @maxlen"),
DEX_MEMBER_F("strnrchr", &c_string_strnrchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrrchr, but limit the max number of scanned characters to @maxlen"),
DEX_MEMBER_F("stroff", &c_string_stroff, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
             "Same ?Gstrchr, but return the offset of the found character from @haystack, "
             /**/ "or ${strlen(haystack)} if @needle wasn't found"),
DEX_MEMBER_F("strroff", &c_string_strroff, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
             "Same ?Gstrrchr, but return the offset of the found character from @haystack, "
             /**/ "or ${size_t.max} if @needle wasn't found"),
DEX_MEMBER_F("strnoff", &c_string_strnoff, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
             "Same ?Gstrnchr, but return the offset of the found character from @haystack, "
             /**/ "or ${strnlen(haystack, maxlen)} if @needle wasn't found"),
DEX_MEMBER_F("strnroff", &c_string_strnroff, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
             "Same ?Gstrnrchr, but return the offset of the found character from @haystack, "
             /**/ "or ${size_t.max} if @needle wasn't found"),
DEX_MEMBER_F("strchrnul", &c_string_strchrnul, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrchr, but return ${strend(haystack)} if @needle wasn't found"),
DEX_MEMBER_F("strrchrnul", &c_string_strrchrnul, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrrchr, but return ${haystack - 1} if @needle wasn't found"),
DEX_MEMBER_F("strnchrnul", &c_string_strnchrnul, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrnchr, but return ${strnend(haystack, maxlen)} if @needle wasn't found"),
DEX_MEMBER_F("strnrchrnul", &c_string_strnrchrnul, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrnrchr, but return ${haystack - 1} if @needle wasn't found"),
DEX_MEMBER_F("strcmp", &c_string_strcmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
             "Compare the given strings @lhs and @rhs, returning ${<0}, "
             "${==0} or ${>0} indicative of their relation to one-another"),
DEX_MEMBER_F("strncmp", &c_string_strncmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
             "Same as ?Gstrcmp, but limit the max number of compared characters to @maxlen"),
DEX_MEMBER_F("strcasecmp", &c_string_strcasecmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
             "Same as ?Gstrcmp, but ignore casing"),
DEX_MEMBER_F("strncasecmp", &c_string_strncasecmp, Dee_DEXSYM_READONLY,
             "(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
             "Same as ?Gstrncmp, but ignore casing"),
DEX_MEMBER_F("strcpy", &c_string_strcpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Same as ${(char.ptr)memcpy(dst, src, (strlen(src) + 1) * sizeof(char))}"),
DEX_MEMBER_F("strcat", &c_string_strcat, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Same as ${({ local r = dst; strcpy(strend(dst), src); (char.ptr)r; })}"),
DEX_MEMBER_F("strncpy", &c_string_strncpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
             "Implemented as:\n"
             "${"
             /**/ "function strncpy(dst: char.ptr, src: char.ptr, count: int): char.ptr {\n"
             /**/ "	local srclen = strnlen(src, count);\n"
             /**/ "	memcpy(dst, src, srclen * sizeof(char));\n"
             /**/ "	memset(dst + strlen, 0, (count - srclen) * sizeof(char));\n"
             /**/ "	return dst;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("strncat", &c_string_strncat, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
             "Implemented as:\n"
             "${"
             /**/ "function strncat(dst: char.ptr, src: char.ptr, count: int): char.ptr {\n"
             /**/ "	local srclen = strnlen(src, count);\n"
             /**/ "	local buf = strend(dst);\n"
             /**/ "	memcpy(buf, src, srclen * sizeof(char));\n"
             /**/ "	buf[srclen] = 0;\n"
             /**/ "	return dst;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("stpcpy", &c_string_stpcpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Same as ${(char.ptr)mempcpy(dst, src, (strlen(src) + 1) * sizeof(char)) - 1}"),
DEX_MEMBER_F("stpncpy", &c_string_stpncpy, Dee_DEXSYM_READONLY,
             "(dst:?Aptr?Gchar,src:?Aptr?Gchar,dstsize:?Dint)->?Aptr?Gchar\n"
             "Implemented as:\n"
             "${"
             /**/ "function stpncpy(dst: char.ptr, src: char.ptr, dstsize: int): char.ptr {\n"
             /**/ "	local srclen = strnlen(src, dstsize);\n"
             /**/ "	memcpy(dst, src, srclen * sizeof(char));\n"
             /**/ "	memset(dst + srclen, 0, (dstsize - srclen) * sizeof(char));\n"
             /**/ "	return dst + srclen;\n"
             /**/ "}"
             "}"),
DEX_MEMBER_F("strstr", &c_string_strstr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists"),
DEX_MEMBER_F("strcasestr", &c_string_strcasestr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Same as ?Gstrstr, but ignore casing"),
DEX_MEMBER_F("strnstr", &c_string_strnstr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
             "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists"),
DEX_MEMBER_F("strncasestr", &c_string_strncasestr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrnstr, but ignore casing"),
DEX_MEMBER_F("strverscmp", &c_string_strverscmp, Dee_DEXSYM_READONLY,
             "(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
             "Same as ?Gstrcmp, but do special handling for version strings (s.a. ?Avercompare?Dstring)"),
DEX_MEMBER_F("index", &c_string_strchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Alias for ?Gstrchr"),
DEX_MEMBER_F("rindex", &c_string_strrchr, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
             "Alias for ?Gstrrchr"),
DEX_MEMBER_F("strspn", &c_string_strspn, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,accept:?Aptr?Gchar)->?Dint\n"
             "Returns the offset to the first character in @str that is also "
             "apart of @accept, or ${strlen(str)} when no such character exists"),
DEX_MEMBER_F("strcspn", &c_string_strcspn, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,reject:?Aptr?Gchar)->?Dint\n"
             "Returns the offset to the first character in @str that isn't "
             "apart of @accept, or ${strlen(str)} when no such character exists"),
DEX_MEMBER_F("strpbrk", &c_string_strpbrk, Dee_DEXSYM_READONLY,
             "(haystack:?Aptr?Gchar,accept:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Return a pointer to the first character in @str, that is also apart of @accept\n"
             "If no such character exists, a NULL-pointer is returned"),
DEX_MEMBER_F("strrev", &c_string_strrev, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Reverse the order of characters in @str and re-return the given @str"),
DEX_MEMBER_F("strnrev", &c_string_strnrev, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
             "Reverse the order of up to @maxlen characters in @str and re-return the given @str"),
DEX_MEMBER_F("strlwr", &c_string_strlwr, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Convert all characters in @str to lower-case and re-return the given @str"),
DEX_MEMBER_F("strupr", &c_string_strupr, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Convert all characters in @str to upper-case and re-return the given @str"),
DEX_MEMBER_F("strnlwr", &c_string_strnlwr, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
             "Convert all characters in @str to lower-case and re-return the given @str"),
DEX_MEMBER_F("strnupr", &c_string_strnupr, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
             "Convert all characters in @str to upper-case and re-return the given @str"),
DEX_MEMBER_F("strset", &c_string_strset, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,chr:?Dint)->?Aptr?Gchar\n"
             "Set all characters in @str to @chr and re-return the given @str"),
DEX_MEMBER_F("strnset", &c_string_strnset, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,chr:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
             "Same as ?Gstrset, but limit the operator to up to @maxlen characters"),
DEX_MEMBER_F("strfry", &c_string_strfry, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Randomly shuffle the order of characters in @str, creating an anagram"),
DEX_MEMBER_F("strsep", &c_string_strsep, Dee_DEXSYM_READONLY,
             "(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar"),
DEX_MEMBER_F("stresep", &c_string_stresep, Dee_DEXSYM_READONLY,
             "(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar,escape:?Dint)->?Aptr?Gchar"),
DEX_MEMBER_F("strtok", &c_string_strtok, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar\n"
             "Split @str at each occurrence of @delim and return the resulting strings individually"),
DEX_MEMBER_F("strtok_r", &c_string_strtok_r, Dee_DEXSYM_READONLY,
             "(str:?Aptr?Gchar,delim:?Aptr?Gchar,save_ptr:?Aptr?Aptr?Gchar)->?Aptr?Gchar"),
DEX_MEMBER_F("basename", &c_string_basename, Dee_DEXSYM_READONLY,
             "(filename:?Aptr?Gchar)->?Aptr?Gchar"),

/* Atomic functions */
DEX_MEMBER_F("atomic_cmpxch", &c_atomic_atomic_cmpxch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind],weak=!f)->?Dbool\n"
             "Do an atomic compare-and-exchange of memory at @ptr from @oldval to @newval\n"
             /**/ "When @weak is true, the operation is allowed to fail sporadically, even when "
             /**/ "memory at @ptr and @oldval are identical\n"
             "This is a type-generic operation, with the address-width of the atomic operation "
             /**/ "depending on the typing of @ptr. Supported widths are $1, $2, $4 and $8 bytes"),
DEX_MEMBER_F("atomic_cmpxch_val", &c_atomic_atomic_cmpxch_val, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Same as ?Gatomic_cmpxch, except that rather than returning ?t or ?f indicative of "
             /**/ "the success of the exchange, the #Ireal old value read from @ptr is returned. If "
             /**/ "this is equal to @oldval, the operation was successful. If not, memory pointed-to "
             /**/ "by @ptr remains unchanged"),
DEX_MEMBER_F("atomic_fetchadd", &c_atomic_atomic_fetchadd, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind += addend; r; })}"),
DEX_MEMBER_F("atomic_fetchsub", &c_atomic_atomic_fetchsub, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind -= addend; r; })}"),
DEX_MEMBER_F("atomic_fetchand", &c_atomic_atomic_fetchand, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind &= mask; r; })}"),
DEX_MEMBER_F("atomic_fetchor", &c_atomic_atomic_fetchor, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind |= mask; r; })}"),
DEX_MEMBER_F("atomic_fetchxor", &c_atomic_atomic_fetchxor, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind ^= mask; r; })}"),
DEX_MEMBER_F("atomic_fetchnand", &c_atomic_atomic_fetchnand, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind = ~(ptr.ind & mask); r; })}"),
DEX_MEMBER_F("atomic_addfetch", &c_atomic_atomic_addfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind += addend; copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_subfetch", &c_atomic_atomic_subfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind -= addend; copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_andfetch", &c_atomic_atomic_andfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind &= mask; copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_orfetch", &c_atomic_atomic_orfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind |= mask; copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_xorfetch", &c_atomic_atomic_xorfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind ^= mask; copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_nandfetch", &c_atomic_atomic_nandfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${({ ptr.ind = ~(ptr.ind & mask); copy ptr.ind; })}"),
DEX_MEMBER_F("atomic_add", &c_atomic_atomic_add, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind += addend}"),
DEX_MEMBER_F("atomic_sub", &c_atomic_atomic_sub, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,addend:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind -= addend}"),
DEX_MEMBER_F("atomic_and", &c_atomic_atomic_and, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind &= mask}"),
DEX_MEMBER_F("atomic_or", &c_atomic_atomic_or, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind |= mask}"),
DEX_MEMBER_F("atomic_xor", &c_atomic_atomic_xor, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind ^= mask}"),
DEX_MEMBER_F("atomic_nand", &c_atomic_atomic_nand, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,mask:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind = ~(ptr.ind & mask)}"),
DEX_MEMBER_F("atomic_write", &c_atomic_atomic_write, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,value:?Q!A!Aptr!Pind])\n"
             "Atomic operation for ${ptr.ind = value}"),
DEX_MEMBER_F("atomic_fetchinc", &c_atomic_atomic_fetchinc, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${ptr.ind++}"),
DEX_MEMBER_F("atomic_fetchdec", &c_atomic_atomic_fetchdec, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${ptr.ind--}"),
DEX_MEMBER_F("atomic_incfetch", &c_atomic_atomic_incfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${++ptr.ind}"),
DEX_MEMBER_F("atomic_decfetch", &c_atomic_atomic_decfetch, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${--ptr.ind}"),
DEX_MEMBER_F("atomic_read", &c_atomic_atomic_read, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)->?Q!A!Aptr!Pind]\n"
             "Atomic operation for ${copy ptr.ind}"),
DEX_MEMBER_F("atomic_inc", &c_atomic_atomic_inc, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)\n"
             "Atomic operation for ${++ptr.ind}"),
DEX_MEMBER_F("atomic_dec", &c_atomic_atomic_dec, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)\n"
             "Atomic operation for ${--ptr.ind}"),

/* Futex functions */
DEX_MEMBER_F("futex_wakeone", &c_atomic_futex_wakeone, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)\n"
             "Wake at most 1 thread that is waiting for @ptr to change (s.a. ?Gfutex_wait)"),
DEX_MEMBER_F("futex_wakeall", &c_atomic_futex_wakeall, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject)\n"
             "Wake all threads that are waiting for @ptr to change (s.a. ?Gfutex_wait)"),
DEX_MEMBER_F("futex_wait", &c_atomic_futex_wait, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,expected:?Q!A!Aptr!Pind])\n"
             "Atomically check if ${ptr.ind == expected}. If this isn't the case, return immediately. "
             /**/ "Otherwise, wait until another thread makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, "
             /**/ "or until the #I{stars align} (by which I mean that this function might also return sporadically)\n"
             "This function can be used to form the basis of any other synchronization "
             /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)"),
DEX_MEMBER_F("futex_timedwait", &c_atomic_futex_timedwait, Dee_DEXSYM_READONLY,
             "(ptr:?Aptr?GCObject,expected:?Q!A!Aptr!Pind],timeout:?X3?Etime:Time?Dint?N)->?Dbool\n"
             "#r{true You were woken up, either sporadically, because the value of ${ptr.ind} differs "
             /*   */ "from @expected, or because another thread called ?Gfutex_wakeone or ?Gfutex_wakeall}"
             "#r{false The given @timeout has expired}"
             "Atomically check if ${ptr.ind == expected}. If this isn't the case, immediately return ?t. "
             /**/ "Otherwise, wait until either @timeout have elapsed, or another thread "
             /**/ "makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, or until the #I{stars align} "
             /**/ "(by which I mean that this function might also return sporadically)\n"
             "This function can be used to form the basis of any other synchronization "
             /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)"),

/* Magic constants */
CTYPES_BYTE_ORDER_DEF
CTYPES_FLOAT_WORD_ORDER_DEF
CTYPES_ORDER_LITTLE_ENDIAN_DEF
CTYPES_ORDER_BIG_ENDIAN_DEF
CTYPES_ORDER_PDP_ENDIAN_DEF

#ifdef CONFIG_HAVE_CTYPES_DEPRECATED_ALIASES
DEX_MEMBER_F_NODOC("StructuredType", &CType_Type, Dee_DEXSYM_READONLY),                     /* Deprecated alias */
DEX_MEMBER_F_NODOC("Structured", CType_AsType(&AbstractCObject_Type), Dee_DEXSYM_READONLY), /* Deprecated alias */
#endif /* CONFIG_HAVE_CTYPES_DEPRECATED_ALIASES */

/* clang-format off */
DEX_END(
	/* init:  */ NULL,
	/* fini:  */ PTR_libctypes_fini,
	/* clear: */ NULL
);
/* clang-format on */

DECL_END

#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_C */
