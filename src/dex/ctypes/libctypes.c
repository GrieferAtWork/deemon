/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_LIBCTYPES_C
#define GUARD_DEX_CTYPES_LIBCTYPES_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

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
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/objmethod.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/typecore.h>

#ifdef CONFIG_NO_ERRNO_H
#undef CONFIG_HAVE_ERRNO_H
#elif !defined(CONFIG_HAVE_ERRNO_H)
#ifdef __NO_has_include
#define CONFIG_HAVE_ERRNO_H 1
#elif __has_include(<errno.h>)
#define CONFIG_HAVE_ERRNO_H 1
#endif
#endif

#ifdef CONFIG_HAVE_ERRNO_H
#include <errno.h>
#endif /* CONFIG_HAVE_ERRNO_H */

#include <string.h>

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
	data = error_data();
	switch (data->e_class) {

#ifdef E_SEGFAULT
	case ERROR_CLASS(E_SEGFAULT):
		DeeError_Throwf(&DeeError_SegFault, "Segmentation fault when %s %p",
		                data->e_subclass == ERROR_SUBCLASS(E_SEGFAULT_NOTEXECUTABLE)
		                ? "executing"
		                : data->e_subclass == ERROR_SUBCLASS(E_SEGFAULT_READONLY)
		                  ? "writing to"
		                  : "reading from",
		                data->e_pointers[0]);
		break;
#endif /* E_SEGFAULT */

#ifdef E_DIVIDE_BY_ZERO
	case ERROR_CLASS(E_DIVIDE_BY_ZERO):
		DeeError_Throwf(&DeeError_DivideByZero, "Integer divide by zero");
		break;
#endif /* E_DIVIDE_BY_ZERO */

#ifdef E_OVERFLOW
	case ERROR_CLASS(E_OVERFLOW):
		DeeError_Throwf(&DeeError_IntegerOverflow, "Integer overflow");
		break;
#endif /* E_OVERFLOW */

#ifdef E_ILLEGAL_INSTRUCTION
	case ERROR_CLASS(E_ILLEGAL_INSTRUCTION):
#ifdef E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE
		if (data->e_subclass == ERROR_SUBCLASS(E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE)) {
			DeeError_Throwf(&DeeError_IllegalInstruction, "Privileged instruction");
			break;
		}
#endif /* E_ILLEGAL_INSTRUCTION_PRIVILEGED_OPCODE */
		DeeError_Throwf(&DeeError_IllegalInstruction, "Illegal instruction");
		break;
#endif /* E_ILLEGAL_INSTRUCTION */

#ifdef E_STACK_OVERFLOW
	case ERROR_CLASS(E_STACK_OVERFLOW):
		DeeError_Throwf(&DeeError_StackOverflow, "Stack overflow");
		break;
#endif /* E_STACK_OVERFLOW */

#ifdef E_INDEX_ERROR
	case ERROR_CLASS(E_INDEX_ERROR):
		DeeError_Throwf(&DeeError_IndexError, "Array bounds exceeded");
		break;
#endif /* E_INDEX_ERROR */

#ifdef E_INVALID_ALIGNMENT
	case ERROR_CLASS(E_INVALID_ALIGNMENT):
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



PRIVATE DREF DeeObject *DCALL
f_ctypes_sizeof(size_t argc, DeeObject **__restrict argv) {
	DeeSTypeObject *type;
	DeeObject *arg;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o:sizeof", &arg))
		goto err;
	if (DeeStruct_Check(arg))
		type = (DeeSTypeObject *)Dee_TYPE(arg);
	else {
		if (DeeBytes_Check(arg))
			return DeeInt_NewSize(DeeBytes_SIZE(arg));
		type = DeeSType_Get(arg);
		if unlikely(!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = ((DeeLValueTypeObject *)type)->lt_orig;
	result = DeeSType_Sizeof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_alignof(size_t argc, DeeObject **__restrict argv) {
	DeeSTypeObject *type;
	DeeObject *arg;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o:alignof", &arg))
		goto err;
	if (DeeStruct_Check(arg))
		type = (DeeSTypeObject *)Dee_TYPE(arg);
	else {
		type = DeeSType_Get(arg);
		if unlikely(!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = ((DeeLValueTypeObject *)type)->lt_orig;
	result = DeeSType_Alignof(type);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_intfor(size_t argc, DeeObject **__restrict argv) {
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

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_CHAR:
		result = return_signed ? &DeeCSChar_Type : &DeeCUChar_Type;
		break;
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_SHORT:
		result = return_signed ? &DeeCShort_Type : &DeeCUShort_Type;
		break;
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_INT:
		result = return_signed ? &DeeCInt_Type : &DeeCUInt_Type;
		break;
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
#if CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT
	case CONFIG_CTYPES_SIZEOF_LONG:
		result = return_signed ? &DeeCLong_Type : &DeeCULong_Type;
		break;
#endif /* CONFIG_CTYPES_SIZEOF_LONG != CONFIG_CTYPES_SIZEOF_INT */
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	case CONFIG_CTYPES_SIZEOF_LLONG:
		result = return_signed ? &DeeCLLong_Type : &DeeCULLong_Type;
		break;
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */

	default: break;
	}
	if (result)
		return_reference_((DeeObject *)result);
	DeeError_Throwf(&DeeError_ValueError,
	                "No C integer type with a width of `%Iu' bytes exists",
	                intsize);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_bswap16(size_t argc, DeeObject **__restrict argv) {
	uint16_t i;
	if (DeeArg_Unpack(argc, argv, "I16u:bswap16", &i))
		return NULL;
	return int_newu16(BSWAP16(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_bswap32(size_t argc, DeeObject **__restrict argv) {
	uint32_t i;
	if (DeeArg_Unpack(argc, argv, "I32u:bswap32", &i))
		return NULL;
	return int_newu32(BSWAP32(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_bswap64(size_t argc, DeeObject **__restrict argv) {
	uint64_t i;
	if (DeeArg_Unpack(argc, argv, "I64u:bswap64", &i))
		return NULL;
	return int_newu64(BSWAP64(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_leswap16(size_t argc, DeeObject **__restrict argv) {
	uint16_t i;
	if (DeeArg_Unpack(argc, argv, "I16u:leswap16", &i))
		return NULL;
	return int_newu16(LESWAP16(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_leswap32(size_t argc, DeeObject **__restrict argv) {
	uint32_t i;
	if (DeeArg_Unpack(argc, argv, "I32u:leswap32", &i))
		return NULL;
	return int_newu32(LESWAP32(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_leswap64(size_t argc, DeeObject **__restrict argv) {
	uint64_t i;
	if (DeeArg_Unpack(argc, argv, "I64u:leswap64", &i))
		return NULL;
	return int_newu64(LESWAP64(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_beswap16(size_t argc, DeeObject **__restrict argv) {
	uint16_t i;
	if (DeeArg_Unpack(argc, argv, "I16u:beswap16", &i))
		return NULL;
	return int_newu16(BESWAP16(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_beswap32(size_t argc, DeeObject **__restrict argv) {
	uint32_t i;
	if (DeeArg_Unpack(argc, argv, "I32u:beswap32", &i))
		return NULL;
	return int_newu32(BESWAP32(i));
}

PRIVATE DREF DeeObject *DCALL
f_ctypes_beswap64(size_t argc, DeeObject **__restrict argv) {
	uint64_t i;
	if (DeeArg_Unpack(argc, argv, "I64u:beswap64", &i))
		return NULL;
	return int_newu64(BESWAP64(i));
}

PRIVATE DEFINE_CMETHOD(ctypes_sizeof, f_ctypes_sizeof);
PRIVATE DEFINE_CMETHOD(ctypes_alignof, f_ctypes_alignof);
PRIVATE DEFINE_CMETHOD(ctypes_intfor, f_ctypes_intfor);

PRIVATE DEFINE_CMETHOD(ctypes_bswap16, f_ctypes_bswap16);
PRIVATE DEFINE_CMETHOD(ctypes_bswap32, f_ctypes_bswap32);
PRIVATE DEFINE_CMETHOD(ctypes_bswap64, f_ctypes_bswap64);

PRIVATE DEFINE_CMETHOD(ctypes_leswap16, f_ctypes_leswap16);
PRIVATE DEFINE_CMETHOD(ctypes_leswap32, f_ctypes_leswap32);
PRIVATE DEFINE_CMETHOD(ctypes_leswap64, f_ctypes_leswap64);
PRIVATE DEFINE_CMETHOD(ctypes_beswap16, f_ctypes_beswap16);
PRIVATE DEFINE_CMETHOD(ctypes_beswap32, f_ctypes_beswap32);
PRIVATE DEFINE_CMETHOD(ctypes_beswap64, f_ctypes_beswap64);

PRIVATE DEFINE_CMETHOD(ctypes_malloc, capi_malloc);
PRIVATE DEFINE_CMETHOD(ctypes_free, capi_free);
PRIVATE DEFINE_CMETHOD(ctypes_realloc, capi_realloc);
PRIVATE DEFINE_CMETHOD(ctypes_calloc, capi_calloc);
PRIVATE DEFINE_CMETHOD(ctypes_strdup, capi_strdup);
PRIVATE DEFINE_CMETHOD(ctypes_trymalloc, capi_trymalloc);
PRIVATE DEFINE_CMETHOD(ctypes_tryrealloc, capi_tryrealloc);
PRIVATE DEFINE_CMETHOD(ctypes_trycalloc, capi_trycalloc);
PRIVATE DEFINE_CMETHOD(ctypes_trystrdup, capi_trystrdup);

PRIVATE DEFINE_CMETHOD(ctypes_memcpy, capi_memcpy);
PRIVATE DEFINE_CMETHOD(ctypes_mempcpy, capi_mempcpy);
PRIVATE DEFINE_CMETHOD(ctypes_memccpy, capi_memccpy);
PRIVATE DEFINE_CMETHOD(ctypes_memset, capi_memset);
PRIVATE DEFINE_CMETHOD(ctypes_mempset, capi_mempset);
PRIVATE DEFINE_CMETHOD(ctypes_memmove, capi_memmove);
PRIVATE DEFINE_CMETHOD(ctypes_mempmove, capi_mempmove);
PRIVATE DEFINE_CMETHOD(ctypes_memchr, capi_memchr);
PRIVATE DEFINE_CMETHOD(ctypes_memlen, capi_memlen);
PRIVATE DEFINE_CMETHOD(ctypes_memend, capi_memend);
PRIVATE DEFINE_CMETHOD(ctypes_memrchr, capi_memrchr);
PRIVATE DEFINE_CMETHOD(ctypes_memrlen, capi_memrlen);
PRIVATE DEFINE_CMETHOD(ctypes_memrend, capi_memrend);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemchr, capi_rawmemchr);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemlen, capi_rawmemlen);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemrchr, capi_rawmemrchr);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemrlen, capi_rawmemrlen);
PRIVATE DEFINE_CMETHOD(ctypes_memxchr, capi_memxchr);
PRIVATE DEFINE_CMETHOD(ctypes_memxlen, capi_memxlen);
PRIVATE DEFINE_CMETHOD(ctypes_memxend, capi_memxend);
PRIVATE DEFINE_CMETHOD(ctypes_memxrchr, capi_memxrchr);
PRIVATE DEFINE_CMETHOD(ctypes_memxrlen, capi_memxrlen);
PRIVATE DEFINE_CMETHOD(ctypes_memxrend, capi_memxrend);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxchr, capi_rawmemxchr);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxlen, capi_rawmemxlen);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxrchr, capi_rawmemxrchr);
PRIVATE DEFINE_CMETHOD(ctypes_rawmemxrlen, capi_rawmemxrlen);
PRIVATE DEFINE_CMETHOD(ctypes_memcmp, capi_memcmp);
PRIVATE DEFINE_CMETHOD(ctypes_memcasecmp, capi_memcasecmp);
PRIVATE DEFINE_CMETHOD(ctypes_memmem, capi_memmem);
PRIVATE DEFINE_CMETHOD(ctypes_memcasemem, capi_memcasemem);
PRIVATE DEFINE_CMETHOD(ctypes_memrmem, capi_memrmem);
PRIVATE DEFINE_CMETHOD(ctypes_memcasermem, capi_memcasermem);
PRIVATE DEFINE_CMETHOD(ctypes_memrev, capi_memrev);

PRIVATE DEFINE_CMETHOD(ctypes_strlen, capi_strlen);
PRIVATE DEFINE_CMETHOD(ctypes_strend, capi_strend);
PRIVATE DEFINE_CMETHOD(ctypes_strnlen, capi_strnlen);
PRIVATE DEFINE_CMETHOD(ctypes_strnend, capi_strnend);
PRIVATE DEFINE_CMETHOD(ctypes_strchr, capi_strchr);
PRIVATE DEFINE_CMETHOD(ctypes_strrchr, capi_strrchr);
PRIVATE DEFINE_CMETHOD(ctypes_strnchr, capi_strnchr);
PRIVATE DEFINE_CMETHOD(ctypes_strnrchr, capi_strnrchr);
PRIVATE DEFINE_CMETHOD(ctypes_stroff, capi_stroff);
PRIVATE DEFINE_CMETHOD(ctypes_strroff, capi_strroff);
PRIVATE DEFINE_CMETHOD(ctypes_strnoff, capi_strnoff);
PRIVATE DEFINE_CMETHOD(ctypes_strnroff, capi_strnroff);
PRIVATE DEFINE_CMETHOD(ctypes_strchrnul, capi_strchrnul);
PRIVATE DEFINE_CMETHOD(ctypes_strrchrnul, capi_strrchrnul);
PRIVATE DEFINE_CMETHOD(ctypes_strnchrnul, capi_strnchrnul);
PRIVATE DEFINE_CMETHOD(ctypes_strnrchrnul, capi_strnrchrnul);
PRIVATE DEFINE_CMETHOD(ctypes_strcmp, capi_strcmp);
PRIVATE DEFINE_CMETHOD(ctypes_strncmp, capi_strncmp);
PRIVATE DEFINE_CMETHOD(ctypes_strcasecmp, capi_strcasecmp);
PRIVATE DEFINE_CMETHOD(ctypes_strncasecmp, capi_strncasecmp);
PRIVATE DEFINE_CMETHOD(ctypes_strcpy, capi_strcpy);
PRIVATE DEFINE_CMETHOD(ctypes_strcat, capi_strcat);
PRIVATE DEFINE_CMETHOD(ctypes_strncpy, capi_strncpy);
PRIVATE DEFINE_CMETHOD(ctypes_strncat, capi_strncat);
PRIVATE DEFINE_CMETHOD(ctypes_stpcpy, capi_stpcpy);
PRIVATE DEFINE_CMETHOD(ctypes_stpncpy, capi_stpncpy);
PRIVATE DEFINE_CMETHOD(ctypes_strstr, capi_strstr);
PRIVATE DEFINE_CMETHOD(ctypes_strcasestr, capi_strcasestr);
PRIVATE DEFINE_CMETHOD(ctypes_strverscmp, capi_strverscmp);
PRIVATE DEFINE_CMETHOD(ctypes_strtok, capi_strtok);
PRIVATE DEFINE_CMETHOD(ctypes_index, capi_index);
PRIVATE DEFINE_CMETHOD(ctypes_rindex, capi_rindex);
PRIVATE DEFINE_CMETHOD(ctypes_strspn, capi_strspn);
PRIVATE DEFINE_CMETHOD(ctypes_strcspn, capi_strcspn);
PRIVATE DEFINE_CMETHOD(ctypes_strpbrk, capi_strpbrk);
PRIVATE DEFINE_CMETHOD(ctypes_strcoll, capi_strcoll);
PRIVATE DEFINE_CMETHOD(ctypes_strncoll, capi_strncoll);
PRIVATE DEFINE_CMETHOD(ctypes_strcasecoll, capi_strcasecoll);
PRIVATE DEFINE_CMETHOD(ctypes_strncasecoll, capi_strncasecoll);
PRIVATE DEFINE_CMETHOD(ctypes_strxfrm, capi_strxfrm);
PRIVATE DEFINE_CMETHOD(ctypes_strrev, capi_strrev);
PRIVATE DEFINE_CMETHOD(ctypes_strnrev, capi_strnrev);
PRIVATE DEFINE_CMETHOD(ctypes_strlwr, capi_strlwr);
PRIVATE DEFINE_CMETHOD(ctypes_strupr, capi_strupr);
PRIVATE DEFINE_CMETHOD(ctypes_strset, capi_strset);
PRIVATE DEFINE_CMETHOD(ctypes_strnset, capi_strnset);
PRIVATE DEFINE_CMETHOD(ctypes_strfry, capi_strfry);
//PRIVATE DEFINE_CMETHOD(ctypes_strsep,capi_strsep);
//PRIVATE DEFINE_CMETHOD(ctypes_strtok_r,capi_strtok_r);

#ifdef CONFIG_HAVE_ERRNO_H
PRIVATE DREF DeeObject *DCALL
capi_errno_get(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
	return DeeInt_NewInt(errno);
}

PRIVATE DREF DeeObject *DCALL
capi_errno_del(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
	errno = 0;
	return_none;
}

PRIVATE DREF DeeObject *DCALL
capi_errno_set(size_t argc, DeeObject **__restrict argv) {
	int newval;
	if (DeeArg_Unpack(argc, argv, "d:errno.setter", &newval))
		goto err;
	errno = newval;
	return_none;
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(ctypes_errno_get, capi_errno_get);
PRIVATE DEFINE_CMETHOD(ctypes_errno_del, capi_errno_del);
PRIVATE DEFINE_CMETHOD(ctypes_errno_set, capi_errno_set);

#else /* CONFIG_HAVE_ERRNO_H */

PRIVATE DREF DeeObject *DCALL
capi_errno_access(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) {
	DeeError_Throwf(&DeeError_UnsupportedAPI, "No errno support");
	return NULL;
}

PRIVATE DEFINE_CMETHOD(ctypes_errno_access, capi_errno_access);
#define ctypes_errno_get ctypes_errno_access
#define ctypes_errno_del ctypes_errno_access
#define ctypes_errno_set ctypes_errno_access
#endif /* !CONFIG_HAVE_ERRNO_H */

PRIVATE DREF DeeObject *DCALL
capi_strerror(size_t argc, DeeObject **__restrict argv) {
#if !defined(CONFIG_HAVE_ERRNO_H) || defined(CONFIG_NO_STRERROR)
	int no;
	if (DeeArg_Unpack(argc, argv, "|d:strerror", &no))
		goto err;
	return_none;
#else
	int no = errno;
	char *name;
	if (DeeArg_Unpack(argc, argv, "|d:strerror", &no))
		goto err;
	name = strerror(no);
	if (name)
		return DeeString_New(name);
#endif
	return_none;
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(ctypes_strerror, capi_strerror);



PRIVATE struct dex_symbol symbols[] = {
	/* Export the underlying type-system used by ctypes. */
	{ "StructuredType", (DeeObject *)&DeeSType_Type },
	{ "PointerType", (DeeObject *)&DeePointerType_Type },
	{ "LValueType", (DeeObject *)&DeeLValueType_Type },
	{ "ArrayType", (DeeObject *)&DeeArrayType_Type },
	{ "StructType", (DeeObject *)&DeeStructType_Type },
	{ "FunctionType", (DeeObject *)&DeeCFunctionType_Type },
	{ "Structured", (DeeObject *)&DeeStructured_Type },
	{ "Pointer", (DeeObject *)&DeePointer_Type },
	{ "LValue", (DeeObject *)&DeeLValue_Type },
	{ "Array", (DeeObject *)&DeeArray_Type },
	{ "Struct", (DeeObject *)&DeeStruct_Type },
	{ "Function", (DeeObject *)&DeeCFunction_Type },

	/* A wrapper around the native shared-library loader. */
	{ "ShLib", (DeeObject *)&DeeShlib_Type },

	/* Export all the C-types. */
	{ "void", (DeeObject *)&DeeCVoid_Type },
	{ "char", (DeeObject *)&DeeCChar_Type },
	{ "wchar_t", (DeeObject *)&DeeCWChar_Type },
	{ "char16_t", (DeeObject *)&DeeCChar16_Type },
	{ "char32_t", (DeeObject *)&DeeCChar32_Type },
	{ "bool", (DeeObject *)&DeeCBool_Type },
	{ "int8_t", (DeeObject *)&DeeCInt8_Type },
	{ "int16_t", (DeeObject *)&DeeCInt16_Type },
	{ "int32_t", (DeeObject *)&DeeCInt32_Type },
	{ "int64_t", (DeeObject *)&DeeCInt64_Type },
	{ "uint8_t", (DeeObject *)&DeeCUInt8_Type },
	{ "uint16_t", (DeeObject *)&DeeCUInt16_Type },
	{ "uint32_t", (DeeObject *)&DeeCUInt32_Type },
	{ "uint64_t", (DeeObject *)&DeeCUInt64_Type },
	{ "float", (DeeObject *)&DeeCFloat_Type },
	{ "double", (DeeObject *)&DeeCDouble_Type },
	{ "ldouble", (DeeObject *)&DeeCLDouble_Type },
	{ "schar", (DeeObject *)&DeeCSChar_Type },
	{ "uchar", (DeeObject *)&DeeCUChar_Type },
	{ "short", (DeeObject *)&DeeCShort_Type },
	{ "ushort", (DeeObject *)&DeeCUShort_Type },
	{ "int", (DeeObject *)&DeeCInt_Type },
	{ "uint", (DeeObject *)&DeeCUInt_Type },
	{ "long", (DeeObject *)&DeeCLong_Type },
	{ "ulong", (DeeObject *)&DeeCULong_Type },
	{ "llong", (DeeObject *)&DeeCLLong_Type },
	{ "ullong", (DeeObject *)&DeeCULLong_Type },

	/* TODO: Support for atomics */

	/* Other, platform-specific C-types. */
	{ "size_t", (DeeObject *)&CUINT_SIZED(__SIZEOF_SIZE_T__) },
	{ "ssize_t", (DeeObject *)&CINT_SIZED(__SIZEOF_SIZE_T__) },
	{ "ptrdiff_t", (DeeObject *)&CUINT_SIZED(__SIZEOF_PTRDIFF_T__) },
	{ "intptr_t", (DeeObject *)&CINT_SIZED(__SIZEOF_POINTER__) },
	{ "uintptr_t", (DeeObject *)&CUINT_SIZED(__SIZEOF_POINTER__) },

	/* Export some helper functions */
	{ "sizeof", (DeeObject *)&ctypes_sizeof, MODSYM_FNORMAL,
	  DOC("(ob:?GStructuredType)->?Dint\n"
	      "(ob:?GStructured)->?Dint\n"
	      "@throw TypeError The given @tp or @ob are not recognized c-types, nor aliases\n"
	      "Returns the size of a given structured type or object in bytes\n"
	      "\n"
	      "(ob:?DBytes)->?Dint\n"
	      "Returns the size of the given :Bytes ob, which is the same as ${#ob}") },
	{ "alignof", (DeeObject *)&ctypes_alignof, MODSYM_FNORMAL,
	  DOC("(ob:?GStructuredType)->?Dint\n"
	      "(ob:?GStructured)->?Dint\n"
	      "@throw TypeError The given @tp or @ob are not recognized c-types, nor aliases\n"
	      "Returns the alignment of a given structured type or object in bytes") },
	{ "intfor", (DeeObject *)&ctypes_intfor, MODSYM_FNORMAL,
	  DOC("(size:?Dint,signed=!t)->structured_type\n"
	      "@throw ValueError No integer matching the requirements of @size is supported") },

	{ "bswap16", (DeeObject *)&ctypes_bswap16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Return @x with reversed endian") },
	{ "bswap32", (DeeObject *)&ctypes_bswap32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Return @x with reversed endian") },
	{ "bswap64", (DeeObject *)&ctypes_bswap64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Return @x with reversed endian") },

	{ "leswap16", (DeeObject *)&ctypes_leswap16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Return a little-endian @x in host-endian, or a host-endian @x in little-endian") },
	{ "leswap32", (DeeObject *)&ctypes_leswap32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Return a little-endian @x in host-endian, or a host-endian @x in little-endian") },
	{ "leswap64", (DeeObject *)&ctypes_leswap64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Return a little-endian @x in host-endian, or a host-endian @x in little-endian") },
	{ "beswap16", (DeeObject *)&ctypes_beswap16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Return a big-endian @x in host-endian, or a host-endian @x in big-endian") },
	{ "beswap32", (DeeObject *)&ctypes_beswap32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Return a big-endian @x in host-endian, or a host-endian @x in big-endian") },
	{ "beswap64", (DeeObject *)&ctypes_beswap64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Return a big-endian @x in host-endian, or a host-endian @x in big-endian") },

	/* <errno.h> - style accessors. */
	{ "errno", (DeeObject *)&ctypes_errno_get, MODSYM_FPROPERTY,
	  DOC("->?Dint\nAccess the system's errno runtime variable") },
	{ NULL, (DeeObject *)&ctypes_errno_del }, /* PROPERTY.DELETE */
	{ NULL, (DeeObject *)&ctypes_errno_set }, /* PROPERTY.SETTER */
	{ "strerror", (DeeObject *)&ctypes_strerror, MODSYM_FNORMAL,
	  DOC("(errno?:?Dint)->?X2?Dstring?N\n"
	      "Return the name of a given @errno (which defaults to #errno), "
	      "or return :none if the error doesn't have an associated name") },


	/* <string.h> & <stdlib.h> - style ctypes functions */
	{ "malloc", (DeeObject *)&ctypes_malloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "@throw NoMemory Insufficient memory to allocate @size bytes\n"
	      "Allocate a raw buffer of @size bytes from a heap and return a "
	      "pointer to its base\n"
	      "Passing a value of $0 for @size will allocate a minimal-sized heap-block") },
	{ "calloc", (DeeObject *)&ctypes_calloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@throw NoMemory Insufficient memory to allocate ${count * size} bytes\n"
	      "@throw IntegerOverflow The given @count and @size overflow :size_t when multiplied\n"
	      "Same as :malloc, but rather than leaving the newly allocated memory uninitialized, "
	      "fill it with all zeroes, the same way ${memset(malloc(size),0,size)} would\n"
	      "If the product of @count and @size equals ${0}, a minimal-sized heap-block is allocated, "
	      "however the caller must still assume that the memory range they are allowed to access "
	      "is non-existant") },
	{ "realloc", (DeeObject *)&ctypes_realloc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@throw NoMemory Insufficient memory to allocate @size bytes\n"
	      "Given a heap-pointer previously allocated using either :malloc, :calloc or "
	      "a prior call to :realloc, change its size to @size, either releasing then "
	      "unused trialing memory resulting from the difference between its old size "
	      "and a smaller, newer size, or try to extend it if its new size is larger "
	      "than its own, in which case a collision with another block located at the "
	      "location where the extension attempt was made will result in an entirely "
	      "new block being allocated, with all pre-existing data being copied inside.\n"
	      "In all cases, a pointer to the new heap block is returned, which may be "
	      "identical to the old block\n"
	      "If a NULL-pointer is passed for @ptr, the function behaves the same as "
	      "a call to :malloc with the given @{size}. Alternatively, if a valid heap "
	      "pointer is passed for @ptr, and @size is passed as ${0}, the heap block is "
	      "truncated to a minimal size and a heap block is returned that is semantically "
	      "equivalent to one returned by ${malloc(0)}\n"
	      "In the event of failure, the pre-existing heap-block passed for @ptr will remain unchanged") },
	{ "free", (DeeObject *)&ctypes_free, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid)\n"
	      "Release a previously allocated heap-block returned by one of :malloc, :calloc or :realloc\n"
	      "If a NULL-pointer is passed for @ptr, this function has no effect and returns immediately") },
	{ "trymalloc", (DeeObject *)&ctypes_trymalloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :malloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "trycalloc", (DeeObject *)&ctypes_trycalloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :calloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "tryrealloc", (DeeObject *)&ctypes_tryrealloc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :realloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error\n"
	      "In this event, the pre-existing heap-block passed for @ptr is not freed") },
	{ "strdup", (DeeObject *)&ctypes_strdup, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint=!Amax!Gsize_t)->?Aptr?Gchar\n"
	      "@throw NoMemory Insufficient memory\n"
	      "Duplicate the given @str into a heap-allocated memory block\n"
	      ">function strndup(str,maxlen?) {\n"
	      "> import * from ctypes;\n"
	      "> if (maxlen !is bound)\n"
	      ">  maxlen = size_t.max;\n"
	      "> local len = strnlen(str,maxlen) * sizeof(char);\n"
	      "> local res = (char.ptr)memcpy(malloc(len + sizeof(char)),str,len);\n"
	      "> res[len] = 0;\n"
	      "> return res;\n"
	      ">}") },
	{ "trystrdup", (DeeObject *)&ctypes_trystrdup, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint=!Amax!Gsize_t)->?Aptr?Gchar\n"
	      "Try to duplicate the given @str into a heap-allocated memory block\n"
	      ">function trystrndup(str,maxlen?) {\n"
	      "> import * from ctypes;\n"
	      "> if (maxlen !is bound)\n"
	      ">  maxlen = size_t.max;\n"
	      "> local len = strnlen(str,maxlen) * sizeof(char);\n"
	      "> local res = (char.ptr)trymalloc(len + sizeof(char));\n"
	      "> if (res) {\n"
	      ">  memcpy(res,str,len);\n"
	      ">  res[len] = 0;\n"
	      "> }\n"
	      "> return res;\n"
	      ">}") },



	{ "memcpy", (DeeObject *)&ctypes_memcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a :void.ptr\n"
	      "Copies @n bytes of memory from @src to @dst\n"
	      "Note that the source and destination ranges may not overlap") },
	{ "mempcpy", (DeeObject *)&ctypes_mempcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size} as a :void.ptr\n"
	      "Same as :memcpy, but returns ${dst + size}") },
	{ "memccpy", (DeeObject *)&ctypes_memccpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return The last modified by in @dst\n"
	      "Same as :memcpy, but stop after a byte equal to @needle is encountered in @src") },
	{ "memset", (DeeObject *)&ctypes_memset, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a :void.ptr\n"
	      "Set every byte in the range @dst+@size to equal @byte") },
	{ "mempset", (DeeObject *)&ctypes_mempset, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size} as a :void.ptr\n"
	      "Same as :memset, but returns ${dst + size}") },
	{ "memmove", (DeeObject *)&ctypes_memmove, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a :void.ptr\n"
	      "Same as :memcpy, but the source and destination ranges are allowed to overlap") },
	{ "mempmove", (DeeObject *)&ctypes_mempmove, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size} as a :void.ptr\n"
	      "Same as :memcpy, but returns ${dst + size}") },
	{ "memchr", (DeeObject *)&ctypes_memchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in the specified @haystack+@haystack_size "
	      "that equals @needle, or return a NULL-pointer if not found") },
	{ "memlen", (DeeObject *)&ctypes_memlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Return the offset from @haystack of the first byte equal to @needle, or @haystack_size if now found") },
	{ "memend", (DeeObject *)&ctypes_memend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in @haystack that is equal to @needle, or @haystack+@haystack_size if now found") },
	{ "memrchr", (DeeObject *)&ctypes_memrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memchr, but if @needle appears multiple times, return a pointer to the last instance") },
	{ "memrlen", (DeeObject *)&ctypes_memrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memlen, but if @needle appears multiple times, return the offset of the last instance") },
	{ "memrend", (DeeObject *)&ctypes_memrend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memend, but if @needle appears multiple times, return a pointer to the last instance\n"
	      "If @needle doesn't appear at all, return a pointer to @haystack") },
	{ "rawmemchr", (DeeObject *)&ctypes_rawmemchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Search for the byte equal to @needle, starting at @haystack and only "
	      "stopping once one is found, or unmapped memory is reached and "
	      "the host provides support for a MMU") },
	{ "rawmemlen", (DeeObject *)&ctypes_rawmemlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as :rawmemchr, but return the offset from @haystack") },
	{ "rawmemrchr", (DeeObject *)&ctypes_rawmemrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemchr, but search in reverse, starting with ${haystack[-1]}\n"
	      "You can think of this function as a variant of :memrend that operates "
	      "on a buffer that spans the entirety of the available address space") },
	{ "rawmemrlen", (DeeObject *)&ctypes_rawmemrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as :rawmemrchr, but return the positive (unsigned) offset of the "
	      "matching byte, such that ${haystack + return} points to the byte in question") },
	{ "memxchr", (DeeObject *)&ctypes_memxchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxlen", (DeeObject *)&ctypes_memxlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxend", (DeeObject *)&ctypes_memxend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrchr", (DeeObject *)&ctypes_memxrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrlen", (DeeObject *)&ctypes_memxrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrend", (DeeObject *)&ctypes_memxrend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memrend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxchr", (DeeObject *)&ctypes_rawmemxchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxlen", (DeeObject *)&ctypes_rawmemxlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as :rawmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrchr", (DeeObject *)&ctypes_rawmemxrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrlen", (DeeObject *)&ctypes_rawmemxrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as :rawmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memcmp", (DeeObject *)&ctypes_memcmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gvoid,b:?Aptr?Gvoid,haystack_size:?Dint)->?Dint\n"
	      "Compare bytes from 2 buffers in @a and @b of equal haystack_size @haystack_size, and "
	      "search for the first non-matching byte, returning ${< 0} if that byte "
	      "in @a is smaller than its counterpart in @b, ${> 0} if the opposite "
	      "is true, and ${== 0} no such byte exists") },
	{ "memcasecmp", (DeeObject *)&ctypes_memcasecmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gvoid,b:?Aptr?Gvoid,haystack_size:?Dint)->?Dint\n"
	      "Same as :memcmp, but bytes are casted as ASCII characters into a "
	      "common casing prior to comparison") },
	{ "memmem", (DeeObject *)&ctypes_memmem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Search for the first memory block in @haystack+@haystack_size that is equal to @needle+@needle_size "
	      "such that ${memcmp(candidate,needle,needle_size) == 0} and return a pointer to its starting "
	      "location in @haystack\n"
	      "If no such memory block exists, return a NULL-pointer instead") },
	{ "memcasemem", (DeeObject *)&ctypes_memcasemem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memmem, but perform case-insensitive comparisons, using :memcasecmp instead of :memcmp") },
	{ "memrmem", (DeeObject *)&ctypes_memrmem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memmem, but in case more than 1 match exists, return a pointer to the last, rather than the first") },
	{ "memcasermem", (DeeObject *)&ctypes_memcasermem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memcasemem, but in case more than 1 match exists, return a pointer to the last, rather than the first") },
	{ "memrev", (DeeObject *)&ctypes_memrev, MODSYM_FNORMAL,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Reverse the order of bytes in the given @buf+@size, such that upon return its first "
	      "byte contains the old value of the last byte, and the last byte the value of the first, "
	      "and so on.") },

	{ "strlen", (DeeObject *)&ctypes_strlen, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Dint\n"
	      "Returns the length of a given @str in characters") },
	{ "strend", (DeeObject *)&ctypes_strend, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the end of a given @str") },
	{ "strnlen", (DeeObject *)&ctypes_strnlen, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as #strlen, but limit the max number of scanned characters to @maxlen") },
	{ "strnend", (DeeObject *)&ctypes_strnend, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strend, but limit the max number of scanned characters to @maxlen") },
	{ "strchr", (DeeObject *)&ctypes_strchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the first character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strrchr", (DeeObject *)&ctypes_strrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Search for the last character equal to @needle within @haystack\n"
	      "If no such character exists, return a NULL-pointer instead") },
	{ "strnchr", (DeeObject *)&ctypes_strnchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strchr, but limit the max number of scanned characters to @maxlen") },
	{ "strnrchr", (DeeObject *)&ctypes_strnrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strrchr, but limit the max number of scanned characters to @maxlen") },
	{ "stroff", (DeeObject *)&ctypes_stroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same #strchr, but return the offset of the found character from @haystack, or ${strlen(haystack)} if @needle wasn't found") },
	{ "strroff", (DeeObject *)&ctypes_strroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same #strrchr, but return the offset of the found character from @haystack, or ${size_t.max} if @needle wasn't found") },
	{ "strnoff", (DeeObject *)&ctypes_strnoff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same #strnchr, but return the offset of the found character from @haystack, or ${strnlen(haystack,maxlen)} if @needle wasn't found") },
	{ "strnroff", (DeeObject *)&ctypes_strnroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same #strnrchr, but return the offset of the found character from @haystack, or ${size_t.max} if @needle wasn't found") },
	{ "strchrnul", (DeeObject *)&ctypes_strchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as #strchr, but return ${strend(haystack)} if @needle wasn't found") },
	{ "strrchrnul", (DeeObject *)&ctypes_strrchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as #strrchr, but return ${haystack-1} if @needle wasn't found") },
	{ "strnchrnul", (DeeObject *)&ctypes_strnchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strnchr, but return ${strnend(haystack,maxlen)} if @needle wasn't found") },
	{ "strnrchrnul", (DeeObject *)&ctypes_strnrchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strnrchr, but return ${haystack-1} if @needle wasn't found") },
	{ "strcmp", (DeeObject *)&ctypes_strcmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Compare the given strings @a and @b, returning ${<0}, "
	      "${==0} or ${>0} indicative of their relation to one-another") },
	{ "strncmp", (DeeObject *)&ctypes_strncmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as #strcmp, but limit the max number of compared characters to @maxlen") },
	{ "strcasecmp", (DeeObject *)&ctypes_strcasecmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as #strcmp, but ignore casing") },
	{ "strncasecmp", (DeeObject *)&ctypes_strncasecmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as #strncmp, but ignore casing") },
	{ "strcpy", (DeeObject *)&ctypes_strcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)memcpy(dst,src,(strlen(src)+1)*sizeof(char))}") },
	{ "strcat", (DeeObject *)&ctypes_strcat, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${({ local r = dst; strcpy(strend(dst),src); (char.ptr)r; })}") },
	{ "strncpy", (DeeObject *)&ctypes_strncpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      ">function strncpy(dst,src,count) {\n"
	      "> import * from ctypes;\n"
	      "> local srclen = strnlen(src,count);\n"
	      "> memcpy(dst,src,srclen * sizeof(char));\n"
	      "> memset(dst + strlen,0,(count - srclen) * sizeof(char));\n"
	      "> return dst;\n"
	      ">}") },
	{ "strncat", (DeeObject *)&ctypes_strncat, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,count:?Dint)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      ">function strncat(dst,src,count) {\n"
	      "> import * from ctypes;\n"
	      "> local srclen = strnlen(src,count);\n"
	      "> local buf = strend(dst);\n"
	      "> memcpy(buf,src,srclen * sizeof(char));\n"
	      "> buf[srclen] = 0;\n"
	      "> return dst;\n"
	      ">}") },
	{ "stpcpy", (DeeObject *)&ctypes_stpcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)mempcpy(dst,src,(strlen(src) + 1) * sizeof(char)) - 1}") },
	{ "stpncpy", (DeeObject *)&ctypes_stpncpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Implemented as:\n"
	      ">function stpncpy(dst,src,dstsize) {\n"
	      "> import * from ctypes;\n"
	      "> local srclen = strnlen(src,dstsize);\n"
	      "> memcpy(dst,src,srclen * sizeof(char));\n"
	      "> memset(dst + srclen,0,(dstsize - srclen) * sizeof(char));\n"
	      "> return dst + srclen;\n"
	      ">}") },
	{ "strstr", (DeeObject *)&ctypes_strstr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strcasestr", (DeeObject *)&ctypes_strcasestr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as #strstr, but ignore casing") },
	{ "strverscmp", (DeeObject *)&ctypes_strverscmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as #strcmp, but do special handling for version strings (s.a. :string.vercompare)") },
	{ "strtok", (DeeObject *)&ctypes_strtok, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Split @str at each occurance of @delim and return the resulting strings individually") },
	{ "index", (DeeObject *)&ctypes_index, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as #strchr, but return ${strend(haystack)} when @needle is $0") },
	{ "rindex", (DeeObject *)&ctypes_rindex, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as #strrchr, but return ${strend(haystack)} when @needle is $0") },
	{ "strspn", (DeeObject *)&ctypes_strspn, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,accept:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that is also "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strcspn", (DeeObject *)&ctypes_strcspn, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,reject:?Aptr?Gchar)->?Dint\n"
	      "Returns the offset to the first character in @str that isn't "
	      "apart of @accept, or ${strlen(str)} when no such character exists") },
	{ "strpbrk", (DeeObject *)&ctypes_strpbrk, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,accept:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the first character in @str, that is also apart of @accept\n"
	      "If no such character exists, a NULL-pointer is returned") },
	{ "strcoll", (DeeObject *)&ctypes_strcoll, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Compare @a and @b using the currently set locale") },
	{ "strncoll", (DeeObject *)&ctypes_strncoll, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as #strcoll, but limit the number of scanned characters to @maxlen") },
	{ "strcasecoll", (DeeObject *)&ctypes_strcasecoll, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as #strcoll, but ignore casing") },
	{ "strncasecoll", (DeeObject *)&ctypes_strncasecoll, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as #strcasecoll, but limit the number of scanned characters to @maxlen") },
	{ "strxfrm", (DeeObject *)&ctypes_strxfrm, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar,num:?Dint)->?Dint\n"
	      "Transform up to @num characters from @src, using the current locale, and "
	      "store them in @dst before returning the number of stored characters") },
	{ "strrev", (DeeObject *)&ctypes_strrev, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Reverse the order of characters in @str and re-return the given @str") },
	{ "strnrev", (DeeObject *)&ctypes_strnrev, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Reverse the order of up to @maxlen characters in @str and re-return the given @str") },
	{ "strlwr", (DeeObject *)&ctypes_strlwr, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strupr", (DeeObject *)&ctypes_strupr, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strset", (DeeObject *)&ctypes_strset, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,chr:?Dint)->?Aptr?Gchar\n"
	      "Set all characters in @str to @chr and re-return the given @str") },
	{ "strnset", (DeeObject *)&ctypes_strnset, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,chr:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as #strset, but limit the operator to up to @maxlen characters") },
	{ "strfry", (DeeObject *)&ctypes_strfry, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "xor all characters within @str with $42, implementing _very_ simplistic encryption") },
	//    { "strsep",      (DeeObject *)&ctypes_strsep, MODSYM_FNORMAL,
	//      DOC("TODO") },
	//    { "strtok_r",    (DeeObject *)&ctypes_strtok_r, MODSYM_FNORMAL,
	//      DOC("TODO") },
	{ NULL }
};

#ifndef NDEBUG
PRIVATE DeeSTypeObject *static_ctypes[] = {
	(DeeSTypeObject *)&DeeStructured_Type,
	(DeeSTypeObject *)&DeePointer_Type,
	(DeeSTypeObject *)&DeeLValue_Type,
	(DeeSTypeObject *)&DeeArray_Type,
	(DeeSTypeObject *)&DeeCFunction_Type,
	(DeeSTypeObject *)&DeeCVoid_Type,
	(DeeSTypeObject *)&DeeCChar_Type,
	(DeeSTypeObject *)&DeeCWChar_Type,
	(DeeSTypeObject *)&DeeCChar16_Type,
	(DeeSTypeObject *)&DeeCChar32_Type,
	(DeeSTypeObject *)&DeeCBool_Type,
	(DeeSTypeObject *)&DeeCInt8_Type,
	(DeeSTypeObject *)&DeeCInt16_Type,
	(DeeSTypeObject *)&DeeCInt32_Type,
	(DeeSTypeObject *)&DeeCInt64_Type,
	(DeeSTypeObject *)&DeeCUInt8_Type,
	(DeeSTypeObject *)&DeeCUInt16_Type,
	(DeeSTypeObject *)&DeeCUInt32_Type,
	(DeeSTypeObject *)&DeeCUInt64_Type,
	(DeeSTypeObject *)&DeeCFloat_Type,
	(DeeSTypeObject *)&DeeCDouble_Type,
	(DeeSTypeObject *)&DeeCLDouble_Type,
#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	(DeeSTypeObject *)&DeeCSChar_Type,
	(DeeSTypeObject *)&DeeCUChar_Type,
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	(DeeSTypeObject *)&DeeCShort_Type,
	(DeeSTypeObject *)&DeeCUShort_Type,
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	(DeeSTypeObject *)&DeeCInt_Type,
	(DeeSTypeObject *)&DeeCUInt_Type,
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
	(DeeSTypeObject *)&DeeCLong_Type,
	(DeeSTypeObject *)&DeeCULong_Type,
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	(DeeSTypeObject *)&DeeCLLong_Type,
	(DeeSTypeObject *)&DeeCULLong_Type,
#endif /* CONFIG_LLONG_NEEDS_OWN_TYPE */
};

PRIVATE void DCALL
libctypes_fini(DeeDexObject *__restrict UNUSED(self)) {
	size_t i;
	/* Create caches of static C-types, so they don't show up as leaks. */
	for (i = 0; i < COMPILER_LENOF(static_ctypes); ++i) {
		Dee_Free(static_ctypes[i]->st_array.sa_list);
#ifndef CONFIG_NO_CFUNCTION
		Dee_Free(static_ctypes[i]->st_cfunction.sf_list);
#endif /* !CONFIG_NO_CFUNCTION */
	}
}
#endif

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
#else /* !NDEBUG */
	/* .d_fini    = */ NULL,
#endif /* NDEBUG */
	/* .d_imports = */ { NULL },
	/* .d_clear   = */ &libctypes_clear
};

DECL_END


#endif /* !GUARD_DEX_CTYPES_LIBCTYPES_C */
