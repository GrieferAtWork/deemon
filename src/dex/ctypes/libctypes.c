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
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_C
#define GUARD_DEX_CTYPES_LIBCTYPES_C 1
#define DEE_SOURCE

/* WARNING: Using this module trips the unspoken warranty
 *          of me feeling responsible when deemon crashes. */

#include "libctypes.h"
/**/

#include "c_api.h"
/**/

#ifdef CONFIG_HAVE_CTYPES_SEH_GUARD
#include <Windows.h>
#endif /* CONFIG_HAVE_CTYPES_SEH_GUARD */

#ifdef CONFIG_HAVE_CTYPES_KOS_GUARD
#include <kos/except.h>
#endif /* CONFIG_HAVE_CTYPES_KOS_GUARD */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/int128.h>
#include <hybrid/typecore.h>
/**/

#include <stdint.h> /* intN_t, uintN_t */

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
#endif


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


/*[[[deemon (print_CMethod from rt.gen.unpack)("sizeof", "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_sizeof_params "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_sizeof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_sizeof, &libctypes_sizeof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_sizeof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	DeeSTypeObject *type;
	size_t result;
	if (DeeBytes_Check(ob))
		return DeeInt_NewSize(DeeBytes_SIZE(ob));
	type = DeeSType_GetTypeOf(ob);
	if unlikely(!type)
		goto err;
	if (DeeLValueType_Check(type))
		type = DeeSType_AsLValueType(type)->lt_orig;
	result = DeeSType_Sizeof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("alignof", "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_alignof_params "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_alignof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_alignof, &libctypes_alignof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_alignof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	size_t result;
	DeeSTypeObject *type = DeeSType_GetTypeOf(ob);
	if unlikely(!type)
		goto err;
	if (DeeLValueType_Check(type))
		type = DeeSType_AsLValueType(type)->lt_orig;
	result = DeeSType_Alignof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("typeof", "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_typeof_params "ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat"
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_typeof_f_impl(DeeObject *ob);
PRIVATE DEFINE_CMETHOD1(libctypes_typeof, &libctypes_typeof_f_impl, METHOD_FCONSTCALL);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_typeof_f_impl(DeeObject *ob)
/*[[[end]]]*/
{
	DeeSTypeObject *result = DeeSType_GetTypeOf(ob);
	if unlikely(!result)
		goto err;
	if (DeeLValueType_Check(result))
		result = DeeSType_AsLValueType(result)->lt_orig;
	Dee_Incref(DeeSType_AsType(result));
	return DeeSType_AsObject(result);
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("intfor", """
	size_t size,
	bool $signed = true
""", methodFlags: "METHOD_FCONSTCALL");]]]*/
#define libctypes_intfor_params "size:?Dint,signed=!t"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_intfor_f_impl(size_t size, bool signed_);
PRIVATE WUNUSED DREF DeeObject *DCALL libctypes_intfor_f(size_t argc, DeeObject *const *argv) {
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_intfor_f_impl(size_t size, bool signed_)
/*[[[end]]]*/
{
	DeeSTypeObject *result = NULL;
	switch (size) {

	case 1:
		result = signed_
		         ? &DeeCInt8_Type
		         : &DeeCUInt8_Type;
		break;

	case 2:
		result = signed_
		         ? &DeeCInt16_Type
		         : &DeeCUInt16_Type;
		break;

	case 4:
		result = signed_
		         ? &DeeCInt32_Type
		         : &DeeCUInt32_Type;
		break;

	case 8:
		result = signed_
		         ? &DeeCInt64_Type
		         : &DeeCUInt64_Type;
		break;

	case 16:
		result = signed_
		         ? &DeeCInt128_Type
		         : &DeeCUInt128_Type;
		break;

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_CHAR:
		result = signed_
		         ? &DeeCSChar_Type
		         : &DeeCUChar_Type;
		break;
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_SHORT:
		result = signed_
		         ? &DeeCShort_Type
		         : &DeeCUShort_Type;
		break;
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_INT:
		result = signed_
		         ? &DeeCInt_Type
		         : &DeeCUInt_Type;
		break;
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
#if CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT
	case CONFIG_CTYPES_SIZEOF_LONG:
		result = signed_
		         ? &DeeCLong_Type
		         : &DeeCULong_Type;
		break;
#endif /* CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT */
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_LLONG:
		result = signed_
		         ? &DeeCLLong_Type
		         : &DeeCULLong_Type;
		break;
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */

	default: break;
	}
	if (result) {
		Dee_Incref(DeeSType_AsType(result));
		return DeeSType_AsObject(result);
	}
	DeeError_Throwf(&DeeError_ValueError,
	                "No C integer type with a width of `%" PRFuSIZ "' bytes exists",
	                size);
	return NULL;
}


#define libctypes_struct DeeStructType_Type

/*[[[deemon (print_CMethod from rt.gen.unpack)("union", """
	DeeObject *fields_or_name;
	DeeObject *fields = NULL;
""", methodFlags: "METHOD_FCONSTCALL", returnType: "DeeStructTypeObject", docStringPrefix: none);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeStructTypeObject *DCALL libctypes_union_f_impl(DeeObject *fields_or_name, DeeObject *fields);
PRIVATE WUNUSED DREF DeeStructTypeObject *DCALL libctypes_union_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *fields_or_name;
		DeeObject *fields;
	} args;
	args.fields = NULL;
	DeeArg_UnpackStruct1Or2(err, argc, argv, "union", &args, &args.fields_or_name, &args.fields);
	return libctypes_union_f_impl(args.fields_or_name, args.fields);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD(libctypes_union, &libctypes_union_f, METHOD_FCONSTCALL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeStructTypeObject *DCALL libctypes_union_f_impl(DeeObject *fields_or_name, DeeObject *fields)
/*[[[end]]]*/
{
	if (!fields)
		return DeeStructType_FromSequence(NULL, fields_or_name, STRUCT_TYPE_FUNION);
	if (DeeObject_AssertTypeExact(fields_or_name, &DeeString_Type))
		goto err;
	return DeeStructType_FromSequence(fields_or_name, fields, STRUCT_TYPE_FUNION);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap16", "uint16_t x:?Guint16_t", isconst: true);]]]*/
#define libctypes_bswap16_params "x:?Guint16_t"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap16_f_impl(uint16_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_bswap16_f(DeeObject *__restrict arg0) {
	uint16_t x;
	if (DeeObject_AsUInt16(arg0, &x))
		goto err;
	return libctypes_bswap16_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap16, &libctypes_bswap16_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap16_f_impl(uint16_t x)
/*[[[end]]]*/
{
	return int_newu16(BSWAP16(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap32", "uint32_t x:?Guint32_t", isconst: true);]]]*/
#define libctypes_bswap32_params "x:?Guint32_t"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap32_f_impl(uint32_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_bswap32_f(DeeObject *__restrict arg0) {
	uint32_t x;
	if (DeeObject_AsUInt32(arg0, &x))
		goto err;
	return libctypes_bswap32_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap32, &libctypes_bswap32_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap32_f_impl(uint32_t x)
/*[[[end]]]*/
{
	return int_newu32(BSWAP32(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap64", "uint64_t x:?Guint64_t", isconst: true);]]]*/
#define libctypes_bswap64_params "x:?Guint64_t"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap64_f_impl(uint64_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_bswap64_f(DeeObject *__restrict arg0) {
	uint64_t x;
	if (DeeObject_AsUInt64(arg0, &x))
		goto err;
	return libctypes_bswap64_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap64, &libctypes_bswap64_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap64_f_impl(uint64_t x)
/*[[[end]]]*/
{
	return int_newu64(BSWAP64(x));
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("bswap128", "Dee_uint128_t x:?Guint128_t", isconst: true);]]]*/
#define libctypes_bswap128_params "x:?Guint128_t"
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap128_f_impl(Dee_uint128_t x);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL libctypes_bswap128_f(DeeObject *__restrict arg0) {
	Dee_uint128_t x;
	if (DeeObject_AsUInt128(arg0, &x))
		goto err;
	return libctypes_bswap128_f_impl(x);
err:
	return NULL;
}
PRIVATE DEFINE_CMETHOD1(libctypes_bswap128, &libctypes_bswap128_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
FORCELOCAL WUNUSED DREF DeeObject *DCALL libctypes_bswap128_f_impl(Dee_uint128_t x)
/*[[[end]]]*/
{
	Dee_uint128_t res;
#ifdef BSWAP128
	res = BSWAP128(x);
#else /* BSWAP128 */
	__hybrid_uint128_setword64(res, 0, BSWAP64(__hybrid_uint128_getword64(x, 1)));
	__hybrid_uint128_setword64(res, 1, BSWAP64(__hybrid_uint128_getword64(x, 0)));
#endif /* !BSWAP128 */
	return int_newu128(res);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define libctypes_htole16  DeeCUInt16_Type
#define libctypes_letoh16  DeeCUInt16_Type
#define libctypes_htole32  DeeCUInt32_Type
#define libctypes_letoh32  DeeCUInt32_Type
#define libctypes_htole64  DeeCUInt64_Type
#define libctypes_letoh64  DeeCUInt64_Type
#define libctypes_htole128 DeeCUInt128_Type
#define libctypes_letoh128 DeeCUInt128_Type
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
#define libctypes_htobe16  DeeCUInt16_Type
#define libctypes_betoh16  DeeCUInt16_Type
#define libctypes_htobe32  DeeCUInt32_Type
#define libctypes_betoh32  DeeCUInt32_Type
#define libctypes_htobe64  DeeCUInt64_Type
#define libctypes_betoh64  DeeCUInt64_Type
#define libctypes_htobe128 DeeCUInt128_Type
#define libctypes_betoh128 DeeCUInt128_Type
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported `__BYTE_ORDER__'"
#endif /* __BYTE_ORDER__ != ... */

PRIVATE struct dex_symbol symbols[] = {
	/* Export the underlying type-system used by ctypes. */
	{ "StructuredType", (DeeObject *)&DeeSType_Type, MODSYM_FREADONLY },
	{ "PointerType", (DeeObject *)&DeePointerType_Type, MODSYM_FREADONLY },
	{ "LValueType", (DeeObject *)&DeeLValueType_Type, MODSYM_FREADONLY },
	{ "ArrayType", (DeeObject *)&DeeArrayType_Type, MODSYM_FREADONLY },
	{ "StructType", (DeeObject *)&DeeStructType_Type, MODSYM_FREADONLY },
	{ "FunctionType", (DeeObject *)&DeeCFunctionType_Type, MODSYM_FREADONLY },
	{ "Structured", DeeSType_AsObject(&DeeStructured_Type), MODSYM_FREADONLY },
	{ "Pointer", DeePointerType_AsObject(&DeePointer_Type), MODSYM_FREADONLY },
	{ "LValue", DeeLValueType_AsObject(&DeeLValue_Type), MODSYM_FREADONLY },
	{ "Array", DeeArrayType_AsObject(&DeeArray_Type), MODSYM_FREADONLY },
	{ "Struct", DeeStructType_AsObject(&DeeStruct_Type), MODSYM_FREADONLY },
	{ "Function", DeeCFunctionType_AsObject(&DeeCFunction_Type), MODSYM_FREADONLY },
	{ "struct", (DeeObject *)&libctypes_struct, MODSYM_FREADONLY },
	{ "union", (DeeObject *)&libctypes_union, MODSYM_FREADONLY,
	  DOC("(fields:?X2?S?T2?Dstring?GStructuredType?M?Dstring?GStructuredType)->?GStructType\n"
	      "(name:?Dstring,fields:?X2?S?T2?Dstring?GStructuredType?M?Dstring?GStructuredType)->?GStructType") },

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
	{ "ShLib", (DeeObject *)&DeeShLib_Type, MODSYM_FREADONLY },
	{ "dlopen", (DeeObject *)&DeeShLib_Type, MODSYM_FREADONLY }, /* Convenience alias for `ShLib' */

	/* Export all the C-types. */
	{ "void", DeeSType_AsObject(&DeeCVoid_Type), MODSYM_FREADONLY },
	{ "char", DeeSType_AsObject(&DeeCChar_Type), MODSYM_FREADONLY },
	{ "wchar_t", DeeSType_AsObject(&DeeCWChar_Type), MODSYM_FREADONLY },
	{ "char16_t", DeeSType_AsObject(&DeeCChar16_Type), MODSYM_FREADONLY },
	{ "char32_t", DeeSType_AsObject(&DeeCChar32_Type), MODSYM_FREADONLY },
	{ "bool", DeeSType_AsObject(&DeeCBool_Type), MODSYM_FREADONLY },
	{ "int8_t", DeeSType_AsObject(&DeeCInt8_Type), MODSYM_FREADONLY },
	{ "int16_t", DeeSType_AsObject(&DeeCInt16_Type), MODSYM_FREADONLY },
	{ "int32_t", DeeSType_AsObject(&DeeCInt32_Type), MODSYM_FREADONLY },
	{ "int64_t", DeeSType_AsObject(&DeeCInt64_Type), MODSYM_FREADONLY },
	{ "int128_t", DeeSType_AsObject(&DeeCInt128_Type), MODSYM_FREADONLY },
	{ "uint8_t", DeeSType_AsObject(&DeeCUInt8_Type), MODSYM_FREADONLY },
	{ "uint16_t", DeeSType_AsObject(&DeeCUInt16_Type), MODSYM_FREADONLY },
	{ "uint32_t", DeeSType_AsObject(&DeeCUInt32_Type), MODSYM_FREADONLY },
	{ "uint64_t", DeeSType_AsObject(&DeeCUInt64_Type), MODSYM_FREADONLY },
	{ "uint128_t", DeeSType_AsObject(&DeeCUInt128_Type), MODSYM_FREADONLY },
	/* TODO: endian-specific integer types (e.g. `le16' and `be16')
	 *       These could be used in structures and always encode/decode
	 *       the underlying value to/from a regular deemon `int' object
	 *       to automatically apply the necessary endian conversion
	 * >> import * from ctypes;
	 * >> union foo = {
	 * >> 	.le = le32,
	 * >> 	.be = be32,
	 * >> };
	 * >> x = foo(le: 0x12345678);
	 * >> print x.be; // 0x78563412
	 */
	{ "float", DeeSType_AsObject(&DeeCFloat_Type), MODSYM_FREADONLY },
	{ "double", DeeSType_AsObject(&DeeCDouble_Type), MODSYM_FREADONLY },
	{ "ldouble", DeeSType_AsObject(&DeeCLDouble_Type), MODSYM_FREADONLY },
	{ "schar", DeeSType_AsObject(&DeeCSChar_Type), MODSYM_FREADONLY },
	{ "uchar", DeeSType_AsObject(&DeeCUChar_Type), MODSYM_FREADONLY },
	{ "short", DeeSType_AsObject(&DeeCShort_Type), MODSYM_FREADONLY },
	{ "ushort", DeeSType_AsObject(&DeeCUShort_Type), MODSYM_FREADONLY },
	{ "int", DeeSType_AsObject(&DeeCInt_Type), MODSYM_FREADONLY },
	{ "uint", DeeSType_AsObject(&DeeCUInt_Type), MODSYM_FREADONLY },
	{ "long", DeeSType_AsObject(&DeeCLong_Type), MODSYM_FREADONLY },
	{ "ulong", DeeSType_AsObject(&DeeCULong_Type), MODSYM_FREADONLY },
	{ "llong", DeeSType_AsObject(&DeeCLLong_Type), MODSYM_FREADONLY },
	{ "ullong", DeeSType_AsObject(&DeeCULLong_Type), MODSYM_FREADONLY },

	/* Other, platform-specific C-types. */
	{ "size_t", DeeSType_AsObject(&CUINT_SIZED(__SIZEOF_SIZE_T__)), MODSYM_FREADONLY },
	{ "ssize_t", DeeSType_AsObject(&CINT_SIZED(__SIZEOF_SIZE_T__)), MODSYM_FREADONLY },
	{ "ptrdiff_t", DeeSType_AsObject(&CINT_SIZED(__SIZEOF_PTRDIFF_T__)), MODSYM_FREADONLY },
	{ "intptr_t", DeeSType_AsObject(&CINT_SIZED(__SIZEOF_POINTER__)), MODSYM_FREADONLY },
	{ "uintptr_t", DeeSType_AsObject(&CUINT_SIZED(__SIZEOF_POINTER__)), MODSYM_FREADONLY },

	/* Export some helper functions */
	{ "sizeof", (DeeObject *)&libctypes_sizeof, MODSYM_FREADONLY,
	  DOC("(" libctypes_sizeof_params ")->?Dint\n"
	      "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
	      "Returns the size of a given structured type or object in bytes\n"
	      "\n"

	      "(ob:?DBytes)->?Dint\n"
	      "Returns the size of the given ?DBytes ob, which is the same as ${##ob}") },
	{ "alignof", (DeeObject *)&libctypes_alignof, MODSYM_FREADONLY,
	  DOC("(" libctypes_alignof_params ")->?Dint\n"
	      "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
	      "Returns the alignment of a given structured type or object in bytes") },
	{ "typeof", (DeeObject *)&libctypes_typeof, MODSYM_FREADONLY,
	  DOC("(ob:?X7?GStructuredType?GStructured?DType?N?Dbool?Dint?Dfloat)->?GStructuredType\n"
	      "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
	      "Returns the type of a given structured type or object") },
	{ "intfor", (DeeObject *)&libctypes_intfor, MODSYM_FREADONLY,
	  DOC("(" libctypes_intfor_params ")->?GStructuredType\n"
	      "#tValueError{No integer matching the requirements of @size is supported}") },

	{ "bswap16", (DeeObject *)&libctypes_bswap16, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap16_params ")->?Guint16_t\n"
	      "Return @x with reversed endian") },
	{ "bswap32", (DeeObject *)&libctypes_bswap32, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap32_params ")->?Guint32_t\n"
	      "Return @x with reversed endian") },
	{ "bswap64", (DeeObject *)&libctypes_bswap64, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap64_params ")->?Guint64_t\n"
	      "Return @x with reversed endian") },
	{ "bswap128", (DeeObject *)&libctypes_bswap128, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap128_params ")->?Guint128_t\n"
	      "Return @x with reversed endian") },

	{ "htole16", (DeeObject *)&libctypes_htole16, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap16_params ")->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to little-endian") },
	{ "letoh16", (DeeObject *)&libctypes_letoh16, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap16_params ")->?Guint16_t\n"
	      "Convert a 16-bit integer from little-endian to host-endian") },
	{ "htole32", (DeeObject *)&libctypes_htole32, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap32_params ")->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to little-endian") },
	{ "letoh32", (DeeObject *)&libctypes_letoh32, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap32_params ")->?Guint32_t\n"
	      "Convert a 32-bit integer from little-endian to host-endian") },
	{ "htole64", (DeeObject *)&libctypes_htole64, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap64_params ")->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to little-endian") },
	{ "letoh64", (DeeObject *)&libctypes_letoh64, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap64_params ")->?Guint64_t\n"
	      "Convert a 64-bit integer from little-endian to host-endian") },
	{ "htole128", (DeeObject *)&libctypes_htole128, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap128_params ")->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to little-endian") },
	{ "letoh128", (DeeObject *)&libctypes_letoh128, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap128_params ")->?Guint128_t\n"
	      "Convert a 128-bit integer from little-endian to host-endian") },

	{ "htobe16", (DeeObject *)&libctypes_htobe16, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap16_params ")->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to big-endian") },
	{ "betoh16", (DeeObject *)&libctypes_betoh16, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap16_params ")->?Guint16_t\n"
	      "Convert a 16-bit integer from big-endian to host-endian") },
	{ "htobe32", (DeeObject *)&libctypes_htobe32, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap32_params ")->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to big-endian") },
	{ "betoh32", (DeeObject *)&libctypes_betoh32, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap32_params ")->?Guint32_t\n"
	      "Convert a 32-bit integer from big-endian to host-endian") },
	{ "htobe64", (DeeObject *)&libctypes_htobe64, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap64_params ")->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to big-endian") },
	{ "betoh64", (DeeObject *)&libctypes_betoh64, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap64_params ")->?Guint64_t\n"
	      "Convert a 64-bit integer from big-endian to host-endian") },
	{ "htobe128", (DeeObject *)&libctypes_htobe128, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap128_params ")->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to big-endian") },
	{ "betoh128", (DeeObject *)&libctypes_betoh128, MODSYM_FREADONLY,
	  DOC("(" libctypes_bswap128_params ")->?Guint128_t\n"
	      "Convert a 128-bit integer from big-endian to host-endian") },

	/* <string.h> & <stdlib.h> - style ctypes functions */
	{ "malloc", (DeeObject *)&c_malloc_malloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "#tNoMemory{Insufficient memory to allocate @size bytes}"
	      "Allocate a raw buffer of @size bytes from a heap and return a "
	      /**/ "pointer to its base\n"
	      "Passing a value of $0 for @size will allocate a minimal-sized heap-block") },
	{ "calloc", (DeeObject *)&c_malloc_calloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#tNoMemory{Insufficient memory to allocate ${count * size} bytes}"
	      "#tIntegerOverflow{The given @count and @size overflow ?Gsize_t when multiplied}"
	      "Same as ?Gmalloc, but rather than leaving the newly allocated memory uninitialized, "
	      /**/ "fill it with all zeroes, the same way ${memset(malloc(size), 0, size)} would\n"
	      /**/ "If the product of @count and @size equals ${0}, a minimal-sized heap-block is allocated, "
	      /**/ "however the caller must still assume that the memory range they are allowed to access "
	      /**/ "is non-existent") },
	{ "realloc", (DeeObject *)&c_malloc_realloc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
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
	      "In the event of failure, the pre-existing heap-block passed for @ptr will remain unchanged") },
	{ "free", (DeeObject *)&c_malloc_free, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?Gvoid)\n"
	      "Release a previously allocated heap-block returned by one of ?Gmalloc, ?Gcalloc or ?Grealloc\n"
	      "If a NULL-pointer is passed for @ptr, this function has no effect and returns immediately") },
	{ "trymalloc", (DeeObject *)&c_malloc_trymalloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmalloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "trycalloc", (DeeObject *)&c_malloc_trycalloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gcalloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "tryrealloc", (DeeObject *)&c_malloc_tryrealloc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grealloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error\n"
	      "In this event, the pre-existing heap-block passed for @ptr is not freed") },
	{ "strdup", (DeeObject *)&c_malloc_strdup, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint=!A!Pmax!Gsize_t)->?Aptr?Gchar\n"
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
	      "}") },
	{ "trystrdup", (DeeObject *)&c_malloc_trystrdup, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint=!A!Pmax!Gsize_t)->?Aptr?Gchar\n"
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
	      "}") },

	{ "memcpy", (DeeObject *)&c_string_memcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Copies @size * @count bytes of memory from @src to @dst\n"
	      "Note that the source and destination ranges may not overlap") },
	{ "mempcpy", (DeeObject *)&c_string_mempcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size * count} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but returns ${dst + size * count}") },
	{ "memccpy", (DeeObject *)&c_string_memccpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{The last modified by in @dst}"
	      "Same as ?Gmemcpy, but stop after a byte equal to @needle is encountered in @src") },
	{ "memset", (DeeObject *)&c_string_memset, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Set every byte in the range @dst+@size to equal @byte") },
	{ "mempset", (DeeObject *)&c_string_mempset, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemset, but returns ${dst + size}") },
	{ "bzero", (DeeObject *)&c_string_bzero, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,size:?Dint)\n"
	      "(dst:?Aptr?Gvoid,count:?Dint,size:?Dint)\n"
	      "Same as ?Gmemset, but always fills memory with ${0}s") },
	{ "memmove", (DeeObject *)&c_string_memmove, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but the source and destination ranges are allowed to overlap") },
	{ "mempmove", (DeeObject *)&c_string_mempmove, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but returns ${dst + size}") },
	{ "memchr", (DeeObject *)&c_string_memchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in the specified @haystack+@haystack_size "
	      /**/ "that equals @needle, or return a NULL-pointer if not found") },
	{ "memrchr", (DeeObject *)&c_string_memrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memchr, but if @needle appears multiple times, return a pointer to the last instance") },
	{ "memlen", (DeeObject *)&c_string_memlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Return the offset from @haystack of the first byte equal to @needle, or @haystack_size if now found") },
	{ "memrlen", (DeeObject *)&c_string_memrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memlen, but if @needle appears multiple times, return the offset of the last instance") },
	{ "memend", (DeeObject *)&c_string_memend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in @haystack that is equal to @needle, or @haystack+@haystack_size if now found") },
	{ "memrend", (DeeObject *)&c_string_memrend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemend, but if @needle appears multiple times, return a pointer to the last instance\n"
	      "If @needle doesn't appear at all, return a pointer to @haystack") },
	{ "rawmemchr", (DeeObject *)&c_string_rawmemchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Search for the byte equal to @needle, starting at @haystack and only "
	      /**/ "stopping once one is found, or unmapped memory is reached and "
	      /**/ "the host provides support for a MMU") },
	{ "rawmemlen", (DeeObject *)&c_string_rawmemlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemchr, but return the offset from @haystack") },
	{ "rawmemrchr", (DeeObject *)&c_string_rawmemrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemchr, but search in reverse, starting with ${haystack[-1]}\n"
	      "You can think of this function as a variant of ?Gmemrend that operates "
	      /**/ "on a buffer that spans the entirety of the available address space") },
	{ "rawmemrlen", (DeeObject *)&c_string_rawmemrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrchr, but return the positive (unsigned) offset of the "
	      /**/ "matching byte, such that ${haystack + return} points to the byte in question") },
	{ "memxchr", (DeeObject *)&c_string_memxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxlen", (DeeObject *)&c_string_memxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxend", (DeeObject *)&c_string_memxend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memrxchr", (DeeObject *)&c_string_memrxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memrxlen", (DeeObject *)&c_string_memrxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memrxend", (DeeObject *)&c_string_memrxend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxchr", (DeeObject *)&c_string_rawmemxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxlen", (DeeObject *)&c_string_rawmemxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemrxchr", (DeeObject *)&c_string_rawmemrxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemrxlen", (DeeObject *)&c_string_rawmemrxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "bcmp", (DeeObject *)&c_string_bcmp, MODSYM_FREADONLY,
	  DOC("(a:?Aptr?Gvoid,b:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but only returns $0 when buffer are equal, and non-$0 otherwise") },
	{ "memcmp", (DeeObject *)&c_string_memcmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Compare bytes from 2 buffers in @lhs and @rhs of equal @size, and "
	      /**/ "search for the first non-matching byte, returning ${< 0} if that byte "
	      /**/ "in @lhs is smaller than its counterpart in @rhs, ${> 0} if the opposite "
	      /**/ "is true, and ${== 0} no such byte exists") },
	{ "memcasecmp", (DeeObject *)&c_string_memcasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but bytes are casted as ASCII characters "
	      /**/ "into a common casing prior to comparison") },
	{ "memmem", (DeeObject *)&c_string_memmem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Search for the first memory block in @haystack+@haystack_size that is equal to @needle+@needle_size "
	      /**/ "such that ${memcmp(candidate, needle, needle_size) == 0} and return a pointer to its starting "
	      /**/ "location in @haystack\n"
	      "If no such memory block exists, or @needle_size is $0, return a NULL-pointer instead") },
	{ "memcasemem", (DeeObject *)&c_string_memcasemem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but perform case-insensitive comparisons, using ?Gmemcasecmp instead of ?Gmemcmp") },
	{ "memrmem", (DeeObject *)&c_string_memrmem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memcasermem", (DeeObject *)&c_string_memcasermem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemcasemem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memrev", (DeeObject *)&c_string_memrev, MODSYM_FREADONLY,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Reverse the order of bytes in the given @buf+@size, such that upon return its first "
	      /**/ "byte contains the old value of the last byte, and the last byte the value of the first, "
	      /**/ "and so on.") },
	{ "memfrob", (DeeObject *)&c_string_memfrob, MODSYM_FREADONLY,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "xor all bytes within @buf with $42, implementing _very_ simplistic encryption") },

	{ "strlen", (DeeObject *)&c_string_strlen, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Dint\n"
	      "Returns the length of a given @str in characters") },
	{ "strend", (DeeObject *)&c_string_strend, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the end of a given @str") },
	{ "strnlen", (DeeObject *)&c_string_strnlen, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrlen, but limit the max number of scanned characters to @maxlen") },
	{ "strnend", (DeeObject *)&c_string_strnend, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrend, but limit the max number of scanned characters to @maxlen") },
	{ "strchr", (DeeObject *)&c_string_strchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the first character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strrchr", (DeeObject *)&c_string_strrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the last character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strnchr", (DeeObject *)&c_string_strnchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrchr, but limit the max number of scanned characters to @maxlen") },
	{ "strnrchr", (DeeObject *)&c_string_strnrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but limit the max number of scanned characters to @maxlen") },
	{ "stroff", (DeeObject *)&c_string_stroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${strlen(haystack)} if @needle wasn't found") },
	{ "strroff", (DeeObject *)&c_string_strroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${size_t.max} if @needle wasn't found") },
	{ "strnoff", (DeeObject *)&c_string_strnoff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${strnlen(haystack, maxlen)} if @needle wasn't found") },
	{ "strnroff", (DeeObject *)&c_string_strnroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${size_t.max} if @needle wasn't found") },
	{ "strchrnul", (DeeObject *)&c_string_strchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrchr, but return ${strend(haystack)} if @needle wasn't found") },
	{ "strrchrnul", (DeeObject *)&c_string_strrchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strnchrnul", (DeeObject *)&c_string_strnchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnchr, but return ${strnend(haystack, maxlen)} if @needle wasn't found") },
	{ "strnrchrnul", (DeeObject *)&c_string_strnrchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strcmp", (DeeObject *)&c_string_strcmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Compare the given strings @lhs and @rhs, returning ${<0}, "
	      "${==0} or ${>0} indicative of their relation to one-another") },
	{ "strncmp", (DeeObject *)&c_string_strncmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrcmp, but limit the max number of compared characters to @maxlen") },
	{ "strcasecmp", (DeeObject *)&c_string_strcasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but ignore casing") },
	{ "strncasecmp", (DeeObject *)&c_string_strncasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrncmp, but ignore casing") },
	{ "strcpy", (DeeObject *)&c_string_strcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)memcpy(dst, src, (strlen(src) + 1) * sizeof(char))}") },
	{ "strcat", (DeeObject *)&c_string_strcat, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${({ local r = dst; strcpy(strend(dst), src); (char.ptr)r; })}") },
	{ "strncpy", (DeeObject *)&c_string_strncpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      "${"
	      /**/ "function strncpy(dst: char.ptr, src: char.ptr, count: int): char.ptr {\n"
	      /**/ "	local srclen = strnlen(src, count);\n"
	      /**/ "	memcpy(dst, src, srclen * sizeof(char));\n"
	      /**/ "	memset(dst + strlen, 0, (count - srclen) * sizeof(char));\n"
	      /**/ "	return dst;\n"
	      /**/ "}"
	      "}") },
	{ "strncat", (DeeObject *)&c_string_strncat, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      "${"
	      /**/ "function strncat(dst: char.ptr, src: char.ptr, count: int): char.ptr {\n"
	      /**/ "	local srclen = strnlen(src, count);\n"
	      /**/ "	local buf = strend(dst);\n"
	      /**/ "	memcpy(buf, src, srclen * sizeof(char));\n"
	      /**/ "	buf[srclen] = 0;\n"
	      /**/ "	return dst;\n"
	      /**/ "}"
	      "}") },
	{ "stpcpy", (DeeObject *)&c_string_stpcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)mempcpy(dst, src, (strlen(src) + 1) * sizeof(char)) - 1}") },
	{ "stpncpy", (DeeObject *)&c_string_stpncpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,dstsize:?Dint)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      "${"
	      /**/ "function stpncpy(dst: char.ptr, src: char.ptr, dstsize: int): char.ptr {\n"
	      /**/ "	local srclen = strnlen(src, dstsize);\n"
	      /**/ "	memcpy(dst, src, srclen * sizeof(char));\n"
	      /**/ "	memset(dst + srclen, 0, (dstsize - srclen) * sizeof(char));\n"
	      /**/ "	return dst + srclen;\n"
	      /**/ "}"
	      "}") },
	{ "strstr", (DeeObject *)&c_string_strstr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strcasestr", (DeeObject *)&c_string_strcasestr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ?Gstrstr, but ignore casing") },
	{ "strnstr", (DeeObject *)&c_string_strnstr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strncasestr", (DeeObject *)&c_string_strncasestr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnstr, but ignore casing") },
	{ "strverscmp", (DeeObject *)&c_string_strverscmp, MODSYM_FREADONLY,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but do special handling for version strings (s.a. ?Avercompare?Dstring)") },
	{ "index", (DeeObject *)&c_string_strchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrchr") },
	{ "rindex", (DeeObject *)&c_string_strrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrrchr") },
	{ "strspn", (DeeObject *)&c_string_strspn, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,accept:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that is also "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strcspn", (DeeObject *)&c_string_strcspn, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,reject:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that isn't "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strpbrk", (DeeObject *)&c_string_strpbrk, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,accept:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the first character in @str, that is also apart of @accept\n"
	      "If no such character exists, a NULL-pointer is returned") },
	{ "strrev", (DeeObject *)&c_string_strrev, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Reverse the order of characters in @str and re-return the given @str") },
	{ "strnrev", (DeeObject *)&c_string_strnrev, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Reverse the order of up to @maxlen characters in @str and re-return the given @str") },
	{ "strlwr", (DeeObject *)&c_string_strlwr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strupr", (DeeObject *)&c_string_strupr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strnlwr", (DeeObject *)&c_string_strnlwr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strnupr", (DeeObject *)&c_string_strnupr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strset", (DeeObject *)&c_string_strset, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,chr:?Dint)->?Aptr?Gchar\n"
	      "Set all characters in @str to @chr and re-return the given @str") },
	{ "strnset", (DeeObject *)&c_string_strnset, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,chr:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrset, but limit the operator to up to @maxlen characters") },
	{ "strfry", (DeeObject *)&c_string_strfry, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Randomly shuffle the order of characters in @str, creating an anagram") },
	{ "strsep", (DeeObject *)&c_string_strsep, MODSYM_FREADONLY,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar") },
	{ "stresep", (DeeObject *)&c_string_stresep, MODSYM_FREADONLY,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar,escape:?Dint)->?Aptr?Gchar") },
	{ "strtok", (DeeObject *)&c_string_strtok, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Split @str at each occurrence of @delim and return the resulting strings individually") },
	{ "strtok_r", (DeeObject *)&c_string_strtok_r, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar,save_ptr:?Aptr?Aptr?Gchar)->?Aptr?Gchar") },
	{ "basename", (DeeObject *)&c_string_basename, MODSYM_FREADONLY,
	  DOC("(filename:?Aptr?Gchar)->?Aptr?Gchar") },

	/* Atomic functions */
	{ "atomic_cmpxch", (DeeObject *)&c_atomic_atomic_cmpxch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind],weak=!f)->?Dbool\n"
	      "Do an atomic compare-and-exchange of memory at @ptr from @oldval to @newval\n"
	      /**/ "When @weak is true, the operation is allowed to fail sporadically, even when "
	      /**/ "memory at @ptr and @oldval are identical\n"
	      "This is a type-generic operation, with the address-width of the atomic operation "
	      /**/ "depending on the typing of @ptr. Supported widths are $1, $2, $4 and $8 bytes") },
	{ "atomic_cmpxch_val", (DeeObject *)&c_atomic_atomic_cmpxch_val, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Same as ?Gatomic_cmpxch, except that rather than returning ?t or ?f indicative of "
	      /**/ "the success of the exchange, the #Ireal old value read from @ptr is returned. If "
	      /**/ "this is equal to @oldval, the operation was successful. If not, memory pointed-to "
	      /**/ "by @ptr remains unchanged") },
	{ "atomic_fetchadd", (DeeObject *)&c_atomic_atomic_fetchadd, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind += addend; r; })}") },
	{ "atomic_fetchsub", (DeeObject *)&c_atomic_atomic_fetchsub, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind -= addend; r; })}") },
	{ "atomic_fetchand", (DeeObject *)&c_atomic_atomic_fetchand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind &= mask; r; })}") },
	{ "atomic_fetchor", (DeeObject *)&c_atomic_atomic_fetchor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind |= mask; r; })}") },
	{ "atomic_fetchxor", (DeeObject *)&c_atomic_atomic_fetchxor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind ^= mask; r; })}") },
	{ "atomic_fetchnand", (DeeObject *)&c_atomic_atomic_fetchnand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind = ~(ptr.ind & mask); r; })}") },
	{ "atomic_addfetch", (DeeObject *)&c_atomic_atomic_addfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind += addend; copy ptr.ind; })}") },
	{ "atomic_subfetch", (DeeObject *)&c_atomic_atomic_subfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind -= addend; copy ptr.ind; })}") },
	{ "atomic_andfetch", (DeeObject *)&c_atomic_atomic_andfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind &= mask; copy ptr.ind; })}") },
	{ "atomic_orfetch", (DeeObject *)&c_atomic_atomic_orfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind |= mask; copy ptr.ind; })}") },
	{ "atomic_xorfetch", (DeeObject *)&c_atomic_atomic_xorfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind ^= mask; copy ptr.ind; })}") },
	{ "atomic_nandfetch", (DeeObject *)&c_atomic_atomic_nandfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind = ~(ptr.ind & mask); copy ptr.ind; })}") },
	{ "atomic_add", (DeeObject *)&c_atomic_atomic_add, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind += addend}") },
	{ "atomic_sub", (DeeObject *)&c_atomic_atomic_sub, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind -= addend}") },
	{ "atomic_and", (DeeObject *)&c_atomic_atomic_and, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind &= mask}") },
	{ "atomic_or", (DeeObject *)&c_atomic_atomic_or, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind |= mask}") },
	{ "atomic_xor", (DeeObject *)&c_atomic_atomic_xor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind ^= mask}") },
	{ "atomic_nand", (DeeObject *)&c_atomic_atomic_nand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = ~(ptr.ind & mask)}") },
	{ "atomic_write", (DeeObject *)&c_atomic_atomic_write, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,value:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = value}") },
	{ "atomic_fetchinc", (DeeObject *)&c_atomic_atomic_fetchinc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind++}") },
	{ "atomic_fetchdec", (DeeObject *)&c_atomic_atomic_fetchdec, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind--}") },
	{ "atomic_incfetch", (DeeObject *)&c_atomic_atomic_incfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_decfetch", (DeeObject *)&c_atomic_atomic_decfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${--ptr.ind}") },
	{ "atomic_read", (DeeObject *)&c_atomic_atomic_read, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${copy ptr.ind}") },
	{ "atomic_inc", (DeeObject *)&c_atomic_atomic_inc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_dec", (DeeObject *)&c_atomic_atomic_dec, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${--ptr.ind}") },

	/* Futex functions */
	{ "futex_wakeone", (DeeObject *)&c_atomic_futex_wakeone, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake at most 1 thread that is waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wakeall", (DeeObject *)&c_atomic_futex_wakeall, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake all threads that are waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wait", (DeeObject *)&c_atomic_futex_wait, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind])\n"
	      "Atomically check if ${ptr.ind == expected}. If this isn't the case, return immediately. "
	      /**/ "Otherwise, wait until another thread makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, "
	      /**/ "or until the #I{stars align} (by which I mean that this function might also return sporadically)\n"
	      "This function can be used to form the basis of any other synchronization "
	      /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)") },
	{ "futex_timedwait", (DeeObject *)&c_atomic_futex_timedwait, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind],timeout_nanoseconds:?Dint)->?Dbool\n"
	      "#r{true You were woken up, either sporadically, because the value of ${ptr.ind} differs "
	      /*   */ "from @expected, or because another thread called ?Gfutex_wakeone or ?Gfutex_wakeall}"
	      "#r{false The given @timeout_nanoseconds has expired}"
	      "Atomically check if ${ptr.ind == expected}. If this isn't the case, immediately return ?t. "
	      /**/ "Otherwise, wait until either @timeout_nanoseconds have elapsed, or another thread "
	      /**/ "makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, or until the #I{stars align} "
	      /**/ "(by which I mean that this function might also return sporadically)\n"
	      "This function can be used to form the basis of any other synchronization "
	      /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)") },

	{ NULL }
};

#ifndef NDEBUG
PRIVATE void DCALL
libctypes_fini_type(DeeSTypeObject *__restrict self) {
	Dee_Free(self->st_array.sa_list);
#ifndef CONFIG_NO_CFUNCTION
	Dee_Free(self->st_cfunction.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */
}

PRIVATE void DCALL
libctypes_fini(DeeDexObject *__restrict UNUSED(self)) {
	/* Clear caches for static C-types, so they don't show up as leaks. */
	libctypes_fini_type(&DeeStructured_Type);
	libctypes_fini_type(DeePointerType_AsSType(&DeePointer_Type));
	libctypes_fini_type(DeeLValueType_AsSType(&DeeLValue_Type));
	libctypes_fini_type(DeeArrayType_AsSType(&DeeArray_Type));
	libctypes_fini_type(DeeCFunctionType_AsSType(&DeeCFunction_Type));
	libctypes_fini_type(&DeeCVoid_Type);
	libctypes_fini_type(&DeeCChar_Type);
	libctypes_fini_type(&DeeCWChar_Type);
	libctypes_fini_type(&DeeCChar16_Type);
	libctypes_fini_type(&DeeCChar32_Type);
	libctypes_fini_type(&DeeCBool_Type);
	libctypes_fini_type(&DeeCInt8_Type);
	libctypes_fini_type(&DeeCInt16_Type);
	libctypes_fini_type(&DeeCInt32_Type);
	libctypes_fini_type(&DeeCInt64_Type);
	libctypes_fini_type(&DeeCUInt8_Type);
	libctypes_fini_type(&DeeCUInt16_Type);
	libctypes_fini_type(&DeeCUInt32_Type);
	libctypes_fini_type(&DeeCUInt64_Type);
	libctypes_fini_type(&DeeCFloat_Type);
	libctypes_fini_type(&DeeCDouble_Type);
	libctypes_fini_type(&DeeCLDouble_Type);

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	libctypes_fini_type(&DeeCSChar_Type);
	libctypes_fini_type(&DeeCUChar_Type);
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	libctypes_fini_type(&DeeCShort_Type);
	libctypes_fini_type(&DeeCUShort_Type);
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	libctypes_fini_type(&DeeCInt_Type);
	libctypes_fini_type(&DeeCUInt_Type);
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
	libctypes_fini_type(&DeeCLong_Type);
	libctypes_fini_type(&DeeCULong_Type);
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	libctypes_fini_type(&DeeCLLong_Type);
	libctypes_fini_type(&DeeCULLong_Type);
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
}
#endif /* !NDEBUG */

INTDEF bool DCALL clear_void_pointer(void);
PRIVATE bool DCALL
libctypes_clear(DeeDexObject *__restrict UNUSED(self)) {
	return clear_void_pointer();
}


PUBLIC struct dex DEX = {
	/* .d_symbols = */ symbols,
	/* .d_init    = */ NULL,
#ifndef NDEBUG
	/* .d_fini    = */ &libctypes_fini,
#else  /* !NDEBUG */
	/* .d_fini    = */ NULL,
#endif /* NDEBUG */
	/* .d_clear   = */ &libctypes_clear
};

DECL_END

#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_C */
