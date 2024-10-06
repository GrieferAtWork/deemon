/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_C
#define GUARD_DEX_CTYPES_LIBCTYPES_C 1
#define DEE_SOURCE

/* WARNING: Using this module trips the unspoken warranty
 *          of me being responsible when deemon crashes. */


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
#include <deemon/none.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/typecore.h>

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



PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_sizeof(size_t argc, DeeObject *const *argv) {
	DeeSTypeObject *type;
	DeeObject *arg;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o:sizeof", &arg))
		goto err;
	if (DeeStruct_Check(arg)) {
		type = DeeType_AsSType(Dee_TYPE(arg));
	} else {
		if (DeeBytes_Check(arg))
			return DeeInt_NewSize(DeeBytes_SIZE(arg));
		type = DeeSType_Get(arg);
		if unlikely(!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = DeeSType_AsLValueType(type)->lt_orig;
	result = DeeSType_Sizeof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_alignof(size_t argc, DeeObject *const *argv) {
	DeeSTypeObject *type;
	DeeObject *arg;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o:alignof", &arg))
		goto err;
	if (DeeStruct_Check(arg)) {
		type = DeeType_AsSType(Dee_TYPE(arg));
	} else {
		type = DeeSType_Get(arg);
		if unlikely(!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = DeeSType_AsLValueType(type)->lt_orig;
	result = DeeSType_Alignof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_intfor(size_t argc, DeeObject *const *argv) {
	DeeSTypeObject *result = NULL;
	bool return_signed     = true;
	size_t intsize;
	if (DeeArg_Unpack(argc, argv, "o|b:intfor", &intsize, &return_signed))
		goto err;
	switch (intsize) {

	case 1:
		result = return_signed
		         ? &DeeCInt8_Type
		         : &DeeCUInt8_Type;
		break;

	case 2:
		result = return_signed
		         ? &DeeCInt16_Type
		         : &DeeCUInt16_Type;
		break;

	case 4:
		result = return_signed
		         ? &DeeCInt32_Type
		         : &DeeCUInt32_Type;
		break;

	case 8:
		result = return_signed
		         ? &DeeCInt64_Type
		         : &DeeCUInt64_Type;
		break;

	case 16:
		result = return_signed
		         ? &DeeCInt128_Type
		         : &DeeCUInt128_Type;
		break;

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_CHAR:
		result = return_signed
		         ? &DeeCSChar_Type
		         : &DeeCUChar_Type;
		break;
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_SHORT:
		result = return_signed
		         ? &DeeCShort_Type
		         : &DeeCUShort_Type;
		break;
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_INT:
		result = return_signed
		         ? &DeeCInt_Type
		         : &DeeCUInt_Type;
		break;
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
#if CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT
	case CONFIG_CTYPES_SIZEOF_LONG:
		result = return_signed
		         ? &DeeCLong_Type
		         : &DeeCULong_Type;
		break;
#endif /* CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT */
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_LLONG:
		result = return_signed
		         ? &DeeCLLong_Type
		         : &DeeCULLong_Type;
		break;
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */

	default: break;
	}
	if (result)
		return_reference_(DeeSType_AsObject(result));
	DeeError_Throwf(&DeeError_ValueError,
	                "No C integer type with a width of `%" PRFuSIZ "' bytes exists",
	                intsize);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_union(size_t argc, DeeObject *const *argv) {
	DeeObject *fields;
	if (DeeArg_Unpack(argc, argv, "o:union", &fields))
		goto err;
	return (DREF DeeObject *)DeeStructType_FromSequence(NULL, fields, STRUCT_TYPE_FUNION);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_bswap16(size_t argc, DeeObject *const *argv) {
	uint16_t i;
	if (DeeArg_Unpack(argc, argv, UNPu16 ":bswap16", &i))
		goto err;
	return int_newu16(BSWAP16(i));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_bswap32(size_t argc, DeeObject *const *argv) {
	uint32_t i;
	if (DeeArg_Unpack(argc, argv, UNPu32 ":bswap32", &i))
		goto err;
	return int_newu32(BSWAP32(i));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_bswap64(size_t argc, DeeObject *const *argv) {
	uint64_t i;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":bswap64", &i))
		goto err;
	return int_newu64(BSWAP64(i));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
f_ctypes_bswap128(size_t argc, DeeObject *const *argv) {
	Dee_uint128_t i, res;
	if (DeeArg_Unpack(argc, argv, UNPu128 ":bswap128", &i))
		goto err;
#ifdef BSWAP128
	res = BSWAP128(i);
#else /* BSWAP128 */
	__hybrid_uint128_vec64(res)[0] = BSWAP64(__hybrid_uint128_vec64(i)[1]);
	__hybrid_uint128_vec64(res)[1] = BSWAP64(__hybrid_uint128_vec64(i)[0]);
#endif /* !BSWAP128 */
	return int_newu128(res);
err:
	return NULL;
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ctypes_htole16  DeeCUInt16_Type
#define ctypes_letoh16  DeeCUInt16_Type
#define ctypes_htole32  DeeCUInt32_Type
#define ctypes_letoh32  DeeCUInt32_Type
#define ctypes_htole64  DeeCUInt64_Type
#define ctypes_letoh64  DeeCUInt64_Type
#define ctypes_htole128 DeeCUInt128_Type
#define ctypes_letoh128 DeeCUInt128_Type
#define ctypes_htobe16  ctypes_bswap16
#define ctypes_betoh16  ctypes_bswap16
#define ctypes_htobe32  ctypes_bswap32
#define ctypes_betoh32  ctypes_bswap32
#define ctypes_htobe64  ctypes_bswap64
#define ctypes_betoh64  ctypes_bswap64
#define ctypes_htobe128 ctypes_bswap128
#define ctypes_betoh128 ctypes_bswap128
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ctypes_htole16  ctypes_bswap16
#define ctypes_letoh16  ctypes_bswap16
#define ctypes_htole32  ctypes_bswap32
#define ctypes_letoh32  ctypes_bswap32
#define ctypes_htole64  ctypes_bswap64
#define ctypes_letoh64  ctypes_bswap64
#define ctypes_htole128 ctypes_bswap128
#define ctypes_letoh128 ctypes_bswap128
#define ctypes_htobe16  DeeCUInt16_Type
#define ctypes_betoh16  DeeCUInt16_Type
#define ctypes_htobe32  DeeCUInt32_Type
#define ctypes_betoh32  DeeCUInt32_Type
#define ctypes_htobe64  DeeCUInt64_Type
#define ctypes_betoh64  DeeCUInt64_Type
#define ctypes_htobe128 DeeCUInt128_Type
#define ctypes_betoh128 DeeCUInt128_Type
#else /* __BYTE_ORDER__ == ... */
#error "Unsupported `__BYTE_ORDER__'"
#endif /* __BYTE_ORDER__ != ... */


PRIVATE DEFINE_CMETHOD(ctypes_sizeof, &f_ctypes_sizeof, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(ctypes_alignof, &f_ctypes_alignof, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(ctypes_intfor, &f_ctypes_intfor, METHOD_FCONSTCALL);
PRIVATE DEFINE_CMETHOD(ctypes_union, &f_ctypes_union, METHOD_FCONSTCALL);
#define ctypes_struct DeeStructType_Type

PRIVATE DEFINE_CMETHOD(ctypes_bswap16, &f_ctypes_bswap16, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_bswap32, &f_ctypes_bswap32, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_bswap64, &f_ctypes_bswap64, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_bswap128, &f_ctypes_bswap128, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);

PRIVATE DEFINE_CMETHOD(ctypes_malloc, &capi_malloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_free, &capi_free, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_realloc, &capi_realloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_calloc, &capi_calloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strdup, &capi_strdup, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_trymalloc, &capi_trymalloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_tryrealloc, &capi_tryrealloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_trycalloc, &capi_trycalloc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_trystrdup, &capi_trystrdup, METHOD_FNORMAL);

PRIVATE DEFINE_CMETHOD(ctypes_memcpy, &capi_memcpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_mempcpy, &capi_mempcpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memccpy, &capi_memccpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memset, &capi_memset, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_mempset, &capi_mempset, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_bzero, &capi_bzero, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memmove, &capi_memmove, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_mempmove, &capi_mempmove, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memchr, &capi_memchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memrchr, &capi_memrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memlen, &capi_memlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memrlen, &capi_memrlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memend, &capi_memend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memrend, &capi_memrend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemchr, &capi_rawmemchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemlen, &capi_rawmemlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemrchr, &capi_rawmemrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemrlen, &capi_rawmemrlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxchr, &capi_memxchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxlen, &capi_memxlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxend, &capi_memxend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxrchr, &capi_memxrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxrlen, &capi_memxrlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memxrend, &capi_memxrend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxchr, &capi_rawmemxchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxlen, &capi_rawmemxlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxrchr, &capi_rawmemxrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxrlen, &capi_rawmemxrlen, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_bcmp, &capi_bcmp, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memcmp, &capi_memcmp, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memcasecmp, &capi_memcasecmp, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memmem, &capi_memmem, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memcasemem, &capi_memcasemem, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memrmem, &capi_memrmem, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memcasermem, &capi_memcasermem, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memrev, &capi_memrev, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_memfrob, &capi_memfrob, METHOD_FNORMAL);

PRIVATE DEFINE_CMETHOD(ctypes_strlen, &capi_strlen, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strend, &capi_strend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnlen, &capi_strnlen, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strnend, &capi_strnend, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strchr, &capi_strchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strrchr, &capi_strrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnchr, &capi_strnchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnrchr, &capi_strnrchr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_stroff, &capi_stroff, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strroff, &capi_strroff, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strnoff, &capi_strnoff, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strnroff, &capi_strnroff, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strchrnul, &capi_strchrnul, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strrchrnul, &capi_strrchrnul, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnchrnul, &capi_strnchrnul, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnrchrnul, &capi_strnrchrnul, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strcmp, &capi_strcmp, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strncmp, &capi_strncmp, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strcasecmp, &capi_strcasecmp, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strncasecmp, &capi_strncasecmp, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strcpy, &capi_strcpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strcat, &capi_strcat, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strncpy, &capi_strncpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strncat, &capi_strncat, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_stpcpy, &capi_stpcpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_stpncpy, &capi_stpncpy, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strstr, &capi_strstr, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strcasestr, &capi_strcasestr, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strnstr, &capi_strnstr, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strncasestr, &capi_strncasestr, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strverscmp, &capi_strverscmp, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strspn, &capi_strspn, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strcspn, &capi_strcspn, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
PRIVATE DEFINE_CMETHOD(ctypes_strpbrk, &capi_strpbrk, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strrev, &capi_strrev, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnrev, &capi_strnrev, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strlwr, &capi_strlwr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strupr, &capi_strupr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnlwr, &capi_strnlwr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnupr, &capi_strnupr, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strset, &capi_strset, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strnset, &capi_strnset, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strfry, &capi_strfry, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strsep, &capi_strsep, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_stresep, &capi_stresep, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strtok, &capi_strtok, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_strtok_r, &capi_strtok_r, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_basename, &capi_basename, METHOD_FNORMAL);

/* Atomic functions */
PRIVATE DEFINE_CMETHOD(ctypes_atomic_cmpxch, &capi_atomic_cmpxch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_cmpxch_val, &capi_atomic_cmpxch_val, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchadd, &capi_atomic_fetchadd, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchsub, &capi_atomic_fetchsub, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchand, &capi_atomic_fetchand, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchor, &capi_atomic_fetchor, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchxor, &capi_atomic_fetchxor, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchnand, &capi_atomic_fetchnand, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_addfetch, &capi_atomic_addfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_subfetch, &capi_atomic_subfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_andfetch, &capi_atomic_andfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_orfetch, &capi_atomic_orfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_xorfetch, &capi_atomic_xorfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_nandfetch, &capi_atomic_nandfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_add, &capi_atomic_add, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_sub, &capi_atomic_sub, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_and, &capi_atomic_and, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_or, &capi_atomic_or, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_xor, &capi_atomic_xor, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_nand, &capi_atomic_nand, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_write, &capi_atomic_write, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchinc, &capi_atomic_fetchinc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchdec, &capi_atomic_fetchdec, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_incfetch, &capi_atomic_incfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_decfetch, &capi_atomic_decfetch, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_read, &capi_atomic_read, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_inc, &capi_atomic_inc, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_dec, &capi_atomic_dec, METHOD_FNORMAL);

/* Futex functions */
PRIVATE DEFINE_CMETHOD(ctypes_futex_wakeone, &capi_futex_wakeone, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_futex_wakeall, &capi_futex_wakeall, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_futex_wait, &capi_futex_wait, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(ctypes_futex_timedwait, &capi_futex_timedwait, METHOD_FNORMAL);


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
	{ "struct", (DeeObject *)&ctypes_struct, MODSYM_FREADONLY },
	{ "union", (DeeObject *)&ctypes_union, MODSYM_FREADONLY },

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
	{ "ShLib", (DeeObject *)&DeeShlib_Type, MODSYM_FREADONLY },
	{ "dlopen", (DeeObject *)&DeeShlib_Type, MODSYM_FREADONLY }, /* Convenience alias for `ShLib' */

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
	{ "ptrdiff_t", DeeSType_AsObject(&CUINT_SIZED(__SIZEOF_PTRDIFF_T__)), MODSYM_FREADONLY },
	{ "intptr_t", DeeSType_AsObject(&CINT_SIZED(__SIZEOF_POINTER__)), MODSYM_FREADONLY },
	{ "uintptr_t", DeeSType_AsObject(&CUINT_SIZED(__SIZEOF_POINTER__)), MODSYM_FREADONLY },

	/* Export some helper functions */
	{ "sizeof", (DeeObject *)&ctypes_sizeof, MODSYM_FREADONLY,
	  DOC("(ob:?GStructuredType)->?Dint\n"
	      "(ob:?GStructured)->?Dint\n"
	      "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
	      "Returns the size of a given structured type or object in bytes\n"
	      "\n"

	      "(ob:?DBytes)->?Dint\n"
	      "Returns the size of the given ?DBytes ob, which is the same as ${##ob}") },
	{ "alignof", (DeeObject *)&ctypes_alignof, MODSYM_FREADONLY,
	  DOC("(ob:?GStructuredType)->?Dint\n"
	      "(ob:?GStructured)->?Dint\n"
	      "#tTypeError{The given @tp or @ob are not recognized c-types, nor aliases}"
	      "Returns the alignment of a given structured type or object in bytes") },
	{ "intfor", (DeeObject *)&ctypes_intfor, MODSYM_FREADONLY,
	  DOC("(size:?Dint,signed=!t)->?GStructuredType\n"
	      "#tValueError{No integer matching the requirements of @size is supported}") },

	{ "bswap16", (DeeObject *)&ctypes_bswap16, MODSYM_FREADONLY,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Return @x with reversed endian") },
	{ "bswap32", (DeeObject *)&ctypes_bswap32, MODSYM_FREADONLY,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Return @x with reversed endian") },
	{ "bswap64", (DeeObject *)&ctypes_bswap64, MODSYM_FREADONLY,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Return @x with reversed endian") },
	{ "bswap128", (DeeObject *)&ctypes_bswap128, MODSYM_FREADONLY,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Return @x with reversed endian") },

	{ "htole16", (DeeObject *)&ctypes_htole16, MODSYM_FREADONLY,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to little-endian") },
	{ "letoh16", (DeeObject *)&ctypes_letoh16, MODSYM_FREADONLY,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from little-endian to host-endian") },
	{ "htole32", (DeeObject *)&ctypes_htole32, MODSYM_FREADONLY,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to little-endian") },
	{ "letoh32", (DeeObject *)&ctypes_letoh32, MODSYM_FREADONLY,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from little-endian to host-endian") },
	{ "htole64", (DeeObject *)&ctypes_htole64, MODSYM_FREADONLY,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to little-endian") },
	{ "letoh64", (DeeObject *)&ctypes_letoh64, MODSYM_FREADONLY,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from little-endian to host-endian") },
	{ "htole128", (DeeObject *)&ctypes_htole128, MODSYM_FREADONLY,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to little-endian") },
	{ "letoh128", (DeeObject *)&ctypes_letoh128, MODSYM_FREADONLY,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from little-endian to host-endian") },

	{ "htobe16", (DeeObject *)&ctypes_htobe16, MODSYM_FREADONLY,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to big-endian") },
	{ "betoh16", (DeeObject *)&ctypes_betoh16, MODSYM_FREADONLY,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from big-endian to host-endian") },
	{ "htobe32", (DeeObject *)&ctypes_htobe32, MODSYM_FREADONLY,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to big-endian") },
	{ "betoh32", (DeeObject *)&ctypes_betoh32, MODSYM_FREADONLY,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from big-endian to host-endian") },
	{ "htobe64", (DeeObject *)&ctypes_htobe64, MODSYM_FREADONLY,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to big-endian") },
	{ "betoh64", (DeeObject *)&ctypes_betoh64, MODSYM_FREADONLY,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from big-endian to host-endian") },
	{ "htobe128", (DeeObject *)&ctypes_htobe128, MODSYM_FREADONLY,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to big-endian") },
	{ "betoh128", (DeeObject *)&ctypes_betoh128, MODSYM_FREADONLY,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from big-endian to host-endian") },

	/* <string.h> & <stdlib.h> - style ctypes functions */
	{ "malloc", (DeeObject *)&ctypes_malloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "#tNoMemory{Insufficient memory to allocate @size bytes}"
	      "Allocate a raw buffer of @size bytes from a heap and return a "
	      /**/ "pointer to its base\n"
	      "Passing a value of $0 for @size will allocate a minimal-sized heap-block") },
	{ "calloc", (DeeObject *)&ctypes_calloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#tNoMemory{Insufficient memory to allocate ${count * size} bytes}"
	      "#tIntegerOverflow{The given @count and @size overflow ?Gsize_t when multiplied}"
	      "Same as ?Gmalloc, but rather than leaving the newly allocated memory uninitialized, "
	      /**/ "fill it with all zeroes, the same way ${memset(malloc(size), 0, size)} would\n"
	      /**/ "If the product of @count and @size equals ${0}, a minimal-sized heap-block is allocated, "
	      /**/ "however the caller must still assume that the memory range they are allowed to access "
	      /**/ "is non-existent") },
	{ "realloc", (DeeObject *)&ctypes_realloc, MODSYM_FREADONLY,
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
	{ "free", (DeeObject *)&ctypes_free, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?Gvoid)\n"
	      "Release a previously allocated heap-block returned by one of ?Gmalloc, ?Gcalloc or ?Grealloc\n"
	      "If a NULL-pointer is passed for @ptr, this function has no effect and returns immediately") },
	{ "trymalloc", (DeeObject *)&ctypes_trymalloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmalloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "trycalloc", (DeeObject *)&ctypes_trycalloc, MODSYM_FREADONLY,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gcalloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "tryrealloc", (DeeObject *)&ctypes_tryrealloc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grealloc, but return a NULL-pointer if allocation isn't "
	      /**/ "possible due to lack of memory, rather than throwing a :NoMemory error\n"
	      "In this event, the pre-existing heap-block passed for @ptr is not freed") },
	{ "strdup", (DeeObject *)&ctypes_strdup, MODSYM_FREADONLY,
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
	{ "trystrdup", (DeeObject *)&ctypes_trystrdup, MODSYM_FREADONLY,
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

	{ "memcpy", (DeeObject *)&ctypes_memcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Copies @size * @count bytes of memory from @src to @dst\n"
	      "Note that the source and destination ranges may not overlap") },
	{ "mempcpy", (DeeObject *)&ctypes_mempcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size * count} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but returns ${dst + size * count}") },
	{ "memccpy", (DeeObject *)&ctypes_memccpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{The last modified by in @dst}"
	      "Same as ?Gmemcpy, but stop after a byte equal to @needle is encountered in @src") },
	{ "memset", (DeeObject *)&ctypes_memset, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Set every byte in the range @dst+@size to equal @byte") },
	{ "mempset", (DeeObject *)&ctypes_mempset, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemset, but returns ${dst + size}") },
	{ "bzero", (DeeObject *)&ctypes_bzero, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,size:?Dint,count=!1)\n"
	      "Same as ?Gmemset, but always fills memory with ${0}s") },
	{ "memmove", (DeeObject *)&ctypes_memmove, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns @dst as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but the source and destination ranges are allowed to overlap") },
	{ "mempmove", (DeeObject *)&ctypes_mempmove, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "#r{Always re-returns ${dst + size} as a ?Aptr?Gvoid}"
	      "Same as ?Gmemcpy, but returns ${dst + size}") },
	{ "memchr", (DeeObject *)&ctypes_memchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in the specified @haystack+@haystack_size "
	      /**/ "that equals @needle, or return a NULL-pointer if not found") },
	{ "memrchr", (DeeObject *)&ctypes_memrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memchr, but if @needle appears multiple times, return a pointer to the last instance") },
	{ "memlen", (DeeObject *)&ctypes_memlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Return the offset from @haystack of the first byte equal to @needle, or @haystack_size if now found") },
	{ "memrlen", (DeeObject *)&ctypes_memrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memlen, but if @needle appears multiple times, return the offset of the last instance") },
	{ "memend", (DeeObject *)&ctypes_memend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in @haystack that is equal to @needle, or @haystack+@haystack_size if now found") },
	{ "memrend", (DeeObject *)&ctypes_memrend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemend, but if @needle appears multiple times, return a pointer to the last instance\n"
	      "If @needle doesn't appear at all, return a pointer to @haystack") },
	{ "rawmemchr", (DeeObject *)&ctypes_rawmemchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Search for the byte equal to @needle, starting at @haystack and only "
	      /**/ "stopping once one is found, or unmapped memory is reached and "
	      /**/ "the host provides support for a MMU") },
	{ "rawmemlen", (DeeObject *)&ctypes_rawmemlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemchr, but return the offset from @haystack") },
	{ "rawmemrchr", (DeeObject *)&ctypes_rawmemrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemchr, but search in reverse, starting with ${haystack[-1]}\n"
	      "You can think of this function as a variant of ?Gmemrend that operates "
	      /**/ "on a buffer that spans the entirety of the available address space") },
	{ "rawmemrlen", (DeeObject *)&ctypes_rawmemrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrchr, but return the positive (unsigned) offset of the "
	      /**/ "matching byte, such that ${haystack + return} points to the byte in question") },
	{ "memxchr", (DeeObject *)&ctypes_memxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxlen", (DeeObject *)&ctypes_memxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxend", (DeeObject *)&ctypes_memxend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrchr", (DeeObject *)&ctypes_memxrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrlen", (DeeObject *)&ctypes_memxrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrend", (DeeObject *)&ctypes_memxrend, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxchr", (DeeObject *)&ctypes_rawmemxchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxlen", (DeeObject *)&ctypes_rawmemxlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrchr", (DeeObject *)&ctypes_rawmemxrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrlen", (DeeObject *)&ctypes_rawmemxrlen, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "bcmp", (DeeObject *)&ctypes_bcmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but only returns $0 when buffer are equal, and non-$0 otherwise") },
	{ "memcmp", (DeeObject *)&ctypes_memcmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Compare bytes from 2 buffers in @lhs and @rhs of equal @size, and "
	      /**/ "search for the first non-matching byte, returning ${< 0} if that byte "
	      /**/ "in @lhs is smaller than its counterpart in @rhs, ${> 0} if the opposite "
	      /**/ "is true, and ${== 0} no such byte exists") },
	{ "memcasecmp", (DeeObject *)&ctypes_memcasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but bytes are casted as ASCII characters "
	      /**/ "into a common casing prior to comparison") },
	{ "memmem", (DeeObject *)&ctypes_memmem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Search for the first memory block in @haystack+@haystack_size that is equal to @needle+@needle_size "
	      /**/ "such that ${memcmp(candidate, needle, needle_size) == 0} and return a pointer to its starting "
	      /**/ "location in @haystack\n"
	      "If no such memory block exists, or @needle_size is $0, return a NULL-pointer instead") },
	{ "memcasemem", (DeeObject *)&ctypes_memcasemem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but perform case-insensitive comparisons, using ?Gmemcasecmp instead of ?Gmemcmp") },
	{ "memrmem", (DeeObject *)&ctypes_memrmem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memcasermem", (DeeObject *)&ctypes_memcasermem, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemcasemem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memrev", (DeeObject *)&ctypes_memrev, MODSYM_FREADONLY,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Reverse the order of bytes in the given @buf+@size, such that upon return its first "
	      /**/ "byte contains the old value of the last byte, and the last byte the value of the first, "
	      /**/ "and so on.") },
	{ "memfrob", (DeeObject *)&ctypes_memfrob, MODSYM_FREADONLY,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "xor all bytes within @buf with $42, implementing _very_ simplistic encryption") },

	{ "strlen", (DeeObject *)&ctypes_strlen, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Dint\n"
	      "Returns the length of a given @str in characters") },
	{ "strend", (DeeObject *)&ctypes_strend, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the end of a given @str") },
	{ "strnlen", (DeeObject *)&ctypes_strnlen, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrlen, but limit the max number of scanned characters to @maxlen") },
	{ "strnend", (DeeObject *)&ctypes_strnend, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrend, but limit the max number of scanned characters to @maxlen") },
	{ "strchr", (DeeObject *)&ctypes_strchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the first character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strrchr", (DeeObject *)&ctypes_strrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the last character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strnchr", (DeeObject *)&ctypes_strnchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrchr, but limit the max number of scanned characters to @maxlen") },
	{ "strnrchr", (DeeObject *)&ctypes_strnrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but limit the max number of scanned characters to @maxlen") },
	{ "stroff", (DeeObject *)&ctypes_stroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${strlen(haystack)} if @needle wasn't found") },
	{ "strroff", (DeeObject *)&ctypes_strroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${size_t.max} if @needle wasn't found") },
	{ "strnoff", (DeeObject *)&ctypes_strnoff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${strnlen(haystack, maxlen)} if @needle wasn't found") },
	{ "strnroff", (DeeObject *)&ctypes_strnroff, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnrchr, but return the offset of the found character from @haystack, "
	      /**/ "or ${size_t.max} if @needle wasn't found") },
	{ "strchrnul", (DeeObject *)&ctypes_strchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrchr, but return ${strend(haystack)} if @needle wasn't found") },
	{ "strrchrnul", (DeeObject *)&ctypes_strrchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strnchrnul", (DeeObject *)&ctypes_strnchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnchr, but return ${strnend(haystack, maxlen)} if @needle wasn't found") },
	{ "strnrchrnul", (DeeObject *)&ctypes_strnrchrnul, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strcmp", (DeeObject *)&ctypes_strcmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Compare the given strings @lhs and @rhs, returning ${<0}, "
	      "${==0} or ${>0} indicative of their relation to one-another") },
	{ "strncmp", (DeeObject *)&ctypes_strncmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrcmp, but limit the max number of compared characters to @maxlen") },
	{ "strcasecmp", (DeeObject *)&ctypes_strcasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but ignore casing") },
	{ "strncasecmp", (DeeObject *)&ctypes_strncasecmp, MODSYM_FREADONLY,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrncmp, but ignore casing") },
	{ "strcpy", (DeeObject *)&ctypes_strcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)memcpy(dst, src, (strlen(src) + 1) * sizeof(char))}") },
	{ "strcat", (DeeObject *)&ctypes_strcat, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${({ local r = dst; strcpy(strend(dst), src); (char.ptr)r; })}") },
	{ "strncpy", (DeeObject *)&ctypes_strncpy, MODSYM_FREADONLY,
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
	{ "strncat", (DeeObject *)&ctypes_strncat, MODSYM_FREADONLY,
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
	{ "stpcpy", (DeeObject *)&ctypes_stpcpy, MODSYM_FREADONLY,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)mempcpy(dst, src, (strlen(src) + 1) * sizeof(char)) - 1}") },
	{ "stpncpy", (DeeObject *)&ctypes_stpncpy, MODSYM_FREADONLY,
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
	{ "strstr", (DeeObject *)&ctypes_strstr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strcasestr", (DeeObject *)&ctypes_strcasestr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ?Gstrstr, but ignore casing") },
	{ "strnstr", (DeeObject *)&ctypes_strnstr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strncasestr", (DeeObject *)&ctypes_strncasestr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnstr, but ignore casing") },
	{ "strverscmp", (DeeObject *)&ctypes_strverscmp, MODSYM_FREADONLY,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but do special handling for version strings (s.a. ?Avercompare?Dstring)") },
	{ "index", (DeeObject *)&ctypes_strchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrchr") },
	{ "rindex", (DeeObject *)&ctypes_strrchr, MODSYM_FREADONLY,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrrchr") },
	{ "strspn", (DeeObject *)&ctypes_strspn, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,accept:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that is also "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strcspn", (DeeObject *)&ctypes_strcspn, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,reject:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that isn't "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strpbrk", (DeeObject *)&ctypes_strpbrk, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,accept:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the first character in @str, that is also apart of @accept\n"
	      "If no such character exists, a NULL-pointer is returned") },
	{ "strrev", (DeeObject *)&ctypes_strrev, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Reverse the order of characters in @str and re-return the given @str") },
	{ "strnrev", (DeeObject *)&ctypes_strnrev, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Reverse the order of up to @maxlen characters in @str and re-return the given @str") },
	{ "strlwr", (DeeObject *)&ctypes_strlwr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strupr", (DeeObject *)&ctypes_strupr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strnlwr", (DeeObject *)&ctypes_strnlwr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strnupr", (DeeObject *)&ctypes_strnupr, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strset", (DeeObject *)&ctypes_strset, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,chr:?Dint)->?Aptr?Gchar\n"
	      "Set all characters in @str to @chr and re-return the given @str") },
	{ "strnset", (DeeObject *)&ctypes_strnset, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,chr:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrset, but limit the operator to up to @maxlen characters") },
	{ "strfry", (DeeObject *)&ctypes_strfry, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Randomly shuffle the order of characters in @str, creating an anagram") },
	{ "strsep", (DeeObject *)&ctypes_strsep, MODSYM_FREADONLY,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar") },
	{ "stresep", (DeeObject *)&ctypes_stresep, MODSYM_FREADONLY,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar,escape:?Dint)->?Aptr?Gchar") },
	{ "strtok", (DeeObject *)&ctypes_strtok, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Split @str at each occurrence of @delim and return the resulting strings individually") },
	{ "strtok_r", (DeeObject *)&ctypes_strtok_r, MODSYM_FREADONLY,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar,save_ptr:?Aptr?Aptr?Gchar)->?Aptr?Gchar") },
	{ "basename", (DeeObject *)&ctypes_basename, MODSYM_FREADONLY,
	  DOC("(filename:?Aptr?Gchar)->?Aptr?Gchar") },

	/* Atomic functions */
	{ "atomic_cmpxch", (DeeObject *)&ctypes_atomic_cmpxch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind],weak=!f)->?Dbool\n"
	      "Do an atomic compare-and-exchange of memory at @ptr from @oldval to @newval\n"
	      /**/ "When @weak is true, the operation is allowed to fail sporadically, even when "
	      /**/ "memory at @ptr and @oldval are identical\n"
	      "This is a type-generic operation, with the address-width of the atomic operation "
	      /**/ "depending on the typing of @ptr. Supported widths are $1, $2, $4 and $8 bytes") },
	{ "atomic_cmpxch_val", (DeeObject *)&ctypes_atomic_cmpxch_val, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Same as ?Gatomic_cmpxch, except that rather than returning ?t or ?f indicative of "
	      /**/ "the success of the exchange, the #Ireal old value read from @ptr is returned. If "
	      /**/ "this is equal to @oldval, the operation was successful. If not, memory pointed-to "
	      /**/ "by @ptr remains unchanged") },
	{ "atomic_fetchadd", (DeeObject *)&ctypes_atomic_fetchadd, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind += addend; r; })}") },
	{ "atomic_fetchsub", (DeeObject *)&ctypes_atomic_fetchsub, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind -= addend; r; })}") },
	{ "atomic_fetchand", (DeeObject *)&ctypes_atomic_fetchand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind &= mask; r; })}") },
	{ "atomic_fetchor", (DeeObject *)&ctypes_atomic_fetchor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind |= mask; r; })}") },
	{ "atomic_fetchxor", (DeeObject *)&ctypes_atomic_fetchxor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind ^= mask; r; })}") },
	{ "atomic_fetchnand", (DeeObject *)&ctypes_atomic_fetchnand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind = ~(ptr.ind & mask); r; })}") },
	{ "atomic_addfetch", (DeeObject *)&ctypes_atomic_addfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind += addend; copy ptr.ind; })}") },
	{ "atomic_subfetch", (DeeObject *)&ctypes_atomic_subfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind -= addend; copy ptr.ind; })}") },
	{ "atomic_andfetch", (DeeObject *)&ctypes_atomic_andfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind &= mask; copy ptr.ind; })}") },
	{ "atomic_orfetch", (DeeObject *)&ctypes_atomic_orfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind |= mask; copy ptr.ind; })}") },
	{ "atomic_xorfetch", (DeeObject *)&ctypes_atomic_xorfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind ^= mask; copy ptr.ind; })}") },
	{ "atomic_nandfetch", (DeeObject *)&ctypes_atomic_nandfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind = ~(ptr.ind & mask); copy ptr.ind; })}") },
	{ "atomic_add", (DeeObject *)&ctypes_atomic_add, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind += addend}") },
	{ "atomic_sub", (DeeObject *)&ctypes_atomic_sub, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind -= addend}") },
	{ "atomic_and", (DeeObject *)&ctypes_atomic_and, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind &= mask}") },
	{ "atomic_or", (DeeObject *)&ctypes_atomic_or, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind |= mask}") },
	{ "atomic_xor", (DeeObject *)&ctypes_atomic_xor, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind ^= mask}") },
	{ "atomic_nand", (DeeObject *)&ctypes_atomic_nand, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = ~(ptr.ind & mask)}") },
	{ "atomic_write", (DeeObject *)&ctypes_atomic_write, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,value:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = value}") },
	{ "atomic_fetchinc", (DeeObject *)&ctypes_atomic_fetchinc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind++}") },
	{ "atomic_fetchdec", (DeeObject *)&ctypes_atomic_fetchdec, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind--}") },
	{ "atomic_incfetch", (DeeObject *)&ctypes_atomic_incfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_decfetch", (DeeObject *)&ctypes_atomic_decfetch, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${--ptr.ind}") },
	{ "atomic_read", (DeeObject *)&ctypes_atomic_read, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${copy ptr.ind}") },
	{ "atomic_inc", (DeeObject *)&ctypes_atomic_inc, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_dec", (DeeObject *)&ctypes_atomic_dec, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${--ptr.ind}") },

	/* Futex functions */
	{ "futex_wakeone", (DeeObject *)&ctypes_futex_wakeone, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake at most 1 thread that is waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wakeall", (DeeObject *)&ctypes_futex_wakeall, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake all threads that are waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wait", (DeeObject *)&ctypes_futex_wait, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind])\n"
	      "Atomically check if ${ptr.ind == expected}. If this is isn't the case, return immediately. "
	      /**/ "Otherwise, wait until another thread makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, "
	      /**/ "or until the #I{stars align} (by which I mean that this function might also return sporadically)\n"
	      "This function can be used to form the basis for any other, arbitrary synchronization "
	      /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)") },
	{ "futex_timedwait", (DeeObject *)&ctypes_futex_timedwait, MODSYM_FREADONLY,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind],timeout_nanoseconds:?Dint)->?Dbool\n"
	      "#r{true You were woken up, either sporadically, because the value of ${ptr.ind} differs "
	      /*        */ "from @expected, or because another thread called ?Gfutex_wakeone or ?Gfutex_wakeall}"
	      "#r{false The given @timeout_nanoseconds has expired}"
	      "Atomically check if ${ptr.ind == expected}. If this is isn't the case, immediately return ?t. "
	      /**/ "Otherwise, wait until either @timeout_nanoseconds have elapsed, or another thread "
	      /**/ "makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, or until the #I{stars align} "
	      /**/ "(by which I mean that this function might also return sporadically)\n"
	      "This function can be used to form the basis for any other, arbitrary synchronization "
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
	/* .d_imports = */ { NULL },
	/* .d_clear   = */ &libctypes_clear
};

DECL_END


#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_C */
