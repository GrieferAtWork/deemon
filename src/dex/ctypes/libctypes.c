/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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
		type = (DeeSTypeObject *)Dee_TYPE(arg);
	} else {
		if (DeeBytes_Check(arg))
			return DeeInt_NewSize(DeeBytes_SIZE(arg));
		type = DeeSType_Get(arg);
		if unlikely (!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = ((DeeLValueTypeObject *)type)->lt_orig;
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
		type = (DeeSTypeObject *)Dee_TYPE(arg);
	} else {
		type = DeeSType_Get(arg);
		if unlikely (!type)
			goto err;
	}
	if (DeeLValueType_Check(type))
		type = ((DeeLValueTypeObject *)type)->lt_orig;
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
		return_reference_((DeeObject *)result);
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


PRIVATE DEFINE_CMETHOD(ctypes_sizeof, f_ctypes_sizeof);
PRIVATE DEFINE_CMETHOD(ctypes_alignof, f_ctypes_alignof);
PRIVATE DEFINE_CMETHOD(ctypes_intfor, f_ctypes_intfor);
PRIVATE DEFINE_CMETHOD(ctypes_union, f_ctypes_union);
#define ctypes_struct DeeStructType_Type

PRIVATE DEFINE_CMETHOD(ctypes_bswap16, f_ctypes_bswap16);
PRIVATE DEFINE_CMETHOD(ctypes_bswap32, f_ctypes_bswap32);
PRIVATE DEFINE_CMETHOD(ctypes_bswap64, f_ctypes_bswap64);
PRIVATE DEFINE_CMETHOD(ctypes_bswap128, f_ctypes_bswap128);

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
PRIVATE DEFINE_CMETHOD(ctypes_bzero, capi_bzero);
PRIVATE DEFINE_CMETHOD(ctypes_memmove, capi_memmove);
PRIVATE DEFINE_CMETHOD(ctypes_mempmove, capi_mempmove);
PRIVATE DEFINE_CMETHOD(ctypes_memchr, capi_memchr);
PRIVATE DEFINE_CMETHOD(ctypes_memrchr, capi_memrchr);
PRIVATE DEFINE_CMETHOD(ctypes_memlen, capi_memlen);
PRIVATE DEFINE_CMETHOD(ctypes_memrlen, capi_memrlen);
PRIVATE DEFINE_CMETHOD(ctypes_memend, capi_memend);
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
PRIVATE DEFINE_CMETHOD(ctypes_bcmp, capi_bcmp);
PRIVATE DEFINE_CMETHOD(ctypes_memcmp, capi_memcmp);
PRIVATE DEFINE_CMETHOD(ctypes_memcasecmp, capi_memcasecmp);
PRIVATE DEFINE_CMETHOD(ctypes_memmem, capi_memmem);
PRIVATE DEFINE_CMETHOD(ctypes_memcasemem, capi_memcasemem);
PRIVATE DEFINE_CMETHOD(ctypes_memrmem, capi_memrmem);
PRIVATE DEFINE_CMETHOD(ctypes_memcasermem, capi_memcasermem);
PRIVATE DEFINE_CMETHOD(ctypes_memrev, capi_memrev);
PRIVATE DEFINE_CMETHOD(ctypes_memfrob, capi_memfrob);

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
PRIVATE DEFINE_CMETHOD(ctypes_strnstr, capi_strnstr);
PRIVATE DEFINE_CMETHOD(ctypes_strncasestr, capi_strncasestr);
PRIVATE DEFINE_CMETHOD(ctypes_strverscmp, capi_strverscmp);
PRIVATE DEFINE_CMETHOD(ctypes_strspn, capi_strspn);
PRIVATE DEFINE_CMETHOD(ctypes_strcspn, capi_strcspn);
PRIVATE DEFINE_CMETHOD(ctypes_strpbrk, capi_strpbrk);
PRIVATE DEFINE_CMETHOD(ctypes_strrev, capi_strrev);
PRIVATE DEFINE_CMETHOD(ctypes_strnrev, capi_strnrev);
PRIVATE DEFINE_CMETHOD(ctypes_strlwr, capi_strlwr);
PRIVATE DEFINE_CMETHOD(ctypes_strupr, capi_strupr);
PRIVATE DEFINE_CMETHOD(ctypes_strnlwr, capi_strnlwr);
PRIVATE DEFINE_CMETHOD(ctypes_strnupr, capi_strnupr);
PRIVATE DEFINE_CMETHOD(ctypes_strset, capi_strset);
PRIVATE DEFINE_CMETHOD(ctypes_strnset, capi_strnset);
PRIVATE DEFINE_CMETHOD(ctypes_strfry, capi_strfry);
PRIVATE DEFINE_CMETHOD(ctypes_strsep, capi_strsep);
PRIVATE DEFINE_CMETHOD(ctypes_stresep, capi_stresep);
PRIVATE DEFINE_CMETHOD(ctypes_strtok, capi_strtok);
PRIVATE DEFINE_CMETHOD(ctypes_strtok_r, capi_strtok_r);
PRIVATE DEFINE_CMETHOD(ctypes_basename, capi_basename);

/* Atomic functions */
PRIVATE DEFINE_CMETHOD(ctypes_atomic_cmpxch, capi_atomic_cmpxch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_cmpxch_val, capi_atomic_cmpxch_val);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchadd, capi_atomic_fetchadd);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchsub, capi_atomic_fetchsub);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchand, capi_atomic_fetchand);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchor, capi_atomic_fetchor);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchxor, capi_atomic_fetchxor);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchnand, capi_atomic_fetchnand);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_addfetch, capi_atomic_addfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_subfetch, capi_atomic_subfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_andfetch, capi_atomic_andfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_orfetch, capi_atomic_orfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_xorfetch, capi_atomic_xorfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_nandfetch, capi_atomic_nandfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_add, capi_atomic_add);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_sub, capi_atomic_sub);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_and, capi_atomic_and);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_or, capi_atomic_or);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_xor, capi_atomic_xor);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_nand, capi_atomic_nand);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_write, capi_atomic_write);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchinc, capi_atomic_fetchinc);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_fetchdec, capi_atomic_fetchdec);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_incfetch, capi_atomic_incfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_decfetch, capi_atomic_decfetch);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_read, capi_atomic_read);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_inc, capi_atomic_inc);
PRIVATE DEFINE_CMETHOD(ctypes_atomic_dec, capi_atomic_dec);

/* Futex functions */
PRIVATE DEFINE_CMETHOD(ctypes_futex_wakeone, capi_futex_wakeone);
PRIVATE DEFINE_CMETHOD(ctypes_futex_wakeall, capi_futex_wakeall);
PRIVATE DEFINE_CMETHOD(ctypes_futex_wait, capi_futex_wait);
PRIVATE DEFINE_CMETHOD(ctypes_futex_timedwait, capi_futex_timedwait);


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
	{ "struct", (DeeObject *)&ctypes_struct },
	{ "union", (DeeObject *)&ctypes_union },
	/* TODO: There needs to be a way of deemon types to list additional base types.
	 *       All of these extra types must be non-variable, and have an instance
	 *       size equal to that of `Object' (iow: not add any of their own fields).
	 *       They also must implement a no-args constructor (that is called when
	 *       instances of relevant sub-class are created)
	 * -> Once that is possible, ctypes integer types must derive frmo `Numeric from deemon'
	 *
	 * User-code must be able to specify multiple bases for a class (just implement this
	 * as passing a tuple of types of the make-class opcode), with the following syntax:
	 * >> // Order doesn't matter; the runtime will figure out which type needs to be
	 * >> // the *true* base-type of `MyList', and if you try to sub-class from more
	 * >> // than 1 type with an instance size greater than `Object', an exception will
	 * >> // be thrown a runtime.
	 * >> class MyList: List, Number {
	 * >>     this = super; // This inherts the constructor from `List'
	 * >>     foo() {
	 * >>         // In user-types with a single base, this is `this as ActualBaseType'
	 * >>         // In user-types with multiple bases, this is `this as MyList.__base__',
	 * >>         // such that the actual typing of `super' will be left up to the runtime
	 * >>         super.append(42);
	 * >>     }
	 * >> };
	 *
	 * This new feature will integrate into the MRO (method-resolution-order) such that
	 * sub-classes inherit attributes in a depth-first order (i.e. the *true* base goes
	 * first, followed by each of the inner-most secondary bases, as well as their bases
	 * recursively, and so on, going up all the way to the top):
	 * - type->ATTRIBUTES
	 * - type->true_base->ATTRIBUTES      -- Recursive lookup here doesn't search extra bases
	 * - type->extra_bases[0]->ATTRIBUTES
	 * - type->extra_bases[1]->ATTRIBUTES
	 * - type->extra_bases[2]->ATTRIBUTES
	 * iow: Compared to the current implementation, attribute lookup in extra bases simply
	 *      takes place _AFTER_ an attribute was searched for like they are already being
	 *      search at the moment.
	 *
	 * Also note that extra bases are inherited recursively during type creation:
	 * >> function createType(bases: {Type...}): Type {
	 * >>     local trueBase = List(for (local base: bases) if (IS_TRUE_BASE(base)) base);
	 * >>     if (!trueBase)
	 * >>         trueBase = [bases.first];
	 * >>     if (#trueBase > 1)
	 * >>         throw ...;
	 * >>     trueBase = trueBase.first;
	 * >>     local extraBases = [];
	 * >>     for (local extraBase: bases) {
	 * >>         if (extraBase == trueBase)
	 * >>             continue;
	 * >>         if (extraBase in extraBases)
	 * >>             continue;
	 * >>         if (IS_TRUE_BASE(extraBase))
	 * >>             throw ...; // More than 1 true base type
	 * >>         extraBases.append(extraBase);
	 * >>     }
	 * >>     for (local base: bases) {
	 * >>         // HINT: `base.__bases__' also includes extra bases
	 * >>         //       that were linked to `base' when it was created
	 * >>         for (local extraBase: base.__bases__) {
	 * >>             if (extraBase !in extraBases)
	 * >>                 extraBases.append(extraBase);
	 * >>         }
	 * >>     }
	 * >>     // Note how the order of `extraBases' still depends
	 * >>     // on the order of types in the caller-given `bases',
	 * >>     // as well as the order of extra bases that were
	 * >>     // specified when each of the caller-given types were
	 * >>     // originally created
	 * >>     // Additionally, one can specify a base that had already
	 * >>     // been given earlier once again in order to increase
	 * >>     // its visibility within the MRO-order.
	 * >>     // Finally, `extraBases' contains _ALL_ extra bases,
	 * >>     // such that attributes/operators defined by them are
	 * >>     // always less important than those of the true base.
	 * >>     return DO_CREATE_TYPE(trueBase: trueBase, extraBases: extraBases);
	 * >> }
	 *
	 * XXX: Wouldn't it be better NOT to leave the decision of the true
	 *      base up to the runtime? This seems like something that the
	 *      user should specify explicitly, rather than be something
	 *      that the runtime should determine implicitly.
	 * >> class MyClass extends TrueBase implements ExtraBase1, ExtraBase2, ExtraBase3 {
	 * >>     ...
	 * >> }
	 * I know, I know: java... But this sort of thing would make the most sense here.
	 * ... Anyways: the core idea that types must be able to specify extra bases holds,
	 *              and is something that needs to be implemented independently of how
	 *              user-defined classes might interact with such a feature, meaning
	 *              that the runtime part can (and must be) be implemented long before
	 *              the user-code part can be.
	 */

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
	 * >> local p2 = (foo){ .u = 0x40000000 }.s.ptr;
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
	{ "ShLib", (DeeObject *)&DeeShlib_Type },
	{ "dlopen", (DeeObject *)&DeeShlib_Type }, /* Convenience alias for `ShLib' */

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
	{ "int128_t", (DeeObject *)&DeeCInt128_Type },
	{ "uint8_t", (DeeObject *)&DeeCUInt8_Type },
	{ "uint16_t", (DeeObject *)&DeeCUInt16_Type },
	{ "uint32_t", (DeeObject *)&DeeCUInt32_Type },
	{ "uint64_t", (DeeObject *)&DeeCUInt64_Type },
	{ "uint128_t", (DeeObject *)&DeeCUInt128_Type },
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
	      "Returns the size of the given :Bytes ob, which is the same as ${##ob}") },
	{ "alignof", (DeeObject *)&ctypes_alignof, MODSYM_FNORMAL,
	  DOC("(ob:?GStructuredType)->?Dint\n"
	      "(ob:?GStructured)->?Dint\n"
	      "@throw TypeError The given @tp or @ob are not recognized c-types, nor aliases\n"
	      "Returns the alignment of a given structured type or object in bytes") },
	{ "intfor", (DeeObject *)&ctypes_intfor, MODSYM_FNORMAL,
	  DOC("(size:?Dint,signed=!t)->?GStructuredType\n"
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

	{ "htole16", (DeeObject *)&ctypes_htole16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to little-endian") },
	{ "letoh16", (DeeObject *)&ctypes_letoh16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from little-endian to host-endian") },
	{ "htole32", (DeeObject *)&ctypes_htole32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to little-endian") },
	{ "letoh32", (DeeObject *)&ctypes_letoh32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from little-endian to host-endian") },
	{ "htole64", (DeeObject *)&ctypes_htole64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to little-endian") },
	{ "letoh64", (DeeObject *)&ctypes_letoh64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from little-endian to host-endian") },
	{ "htole128", (DeeObject *)&ctypes_htole128, MODSYM_FNORMAL,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to little-endian") },
	{ "letoh128", (DeeObject *)&ctypes_letoh128, MODSYM_FNORMAL,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from little-endian to host-endian") },

	{ "htobe16", (DeeObject *)&ctypes_htobe16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from host-endian to big-endian") },
	{ "betoh16", (DeeObject *)&ctypes_betoh16, MODSYM_FNORMAL,
	  DOC("(x:?Guint16_t)->?Guint16_t\n"
	      "Convert a 16-bit integer from big-endian to host-endian") },
	{ "htobe32", (DeeObject *)&ctypes_htobe32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from host-endian to big-endian") },
	{ "betoh32", (DeeObject *)&ctypes_betoh32, MODSYM_FNORMAL,
	  DOC("(x:?Guint32_t)->?Guint32_t\n"
	      "Convert a 32-bit integer from big-endian to host-endian") },
	{ "htobe64", (DeeObject *)&ctypes_htobe64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from host-endian to big-endian") },
	{ "betoh64", (DeeObject *)&ctypes_betoh64, MODSYM_FNORMAL,
	  DOC("(x:?Guint64_t)->?Guint64_t\n"
	      "Convert a 64-bit integer from big-endian to host-endian") },
	{ "htobe128", (DeeObject *)&ctypes_htobe128, MODSYM_FNORMAL,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from host-endian to big-endian") },
	{ "betoh128", (DeeObject *)&ctypes_betoh128, MODSYM_FNORMAL,
	  DOC("(x:?Guint128_t)->?Guint128_t\n"
	      "Convert a 128-bit integer from big-endian to host-endian") },

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
	      "@throw IntegerOverflow The given @count and @size overflow ?Gsize_t when multiplied\n"
	      "Same as ?Gmalloc, but rather than leaving the newly allocated memory uninitialized, "
	      "fill it with all zeroes, the same way ${memset(malloc(size), 0, size)} would\n"
	      "If the product of @count and @size equals ${0}, a minimal-sized heap-block is allocated, "
	      "however the caller must still assume that the memory range they are allowed to access "
	      "is non-existant") },
	{ "realloc", (DeeObject *)&ctypes_realloc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@throw NoMemory Insufficient memory to allocate @size bytes\n"
	      "Given a heap-pointer previously allocated using either ?Gmalloc, ?Gcalloc or "
	      "a prior call to ?Grealloc, change its size to @size, either releasing then "
	      "unused trailing memory resulting from the difference between its old size "
	      "and a smaller, newer size, or try to extend it if its new size is larger "
	      "than its own, in which case a collision with another block located at the "
	      "location where the extension attempt was made will result in an entirely "
	      "new block being allocated, with all pre-existing data being copied inside.\n"
	      "In all cases, a pointer to the new heap block is returned, which may be "
	      "identical to the old block\n"
	      "If a NULL-pointer is passed for @ptr, the function behaves the same as "
	      "a call to ?Gmalloc with the given @{size}. Alternatively, if a valid heap "
	      "pointer is passed for @ptr, and @size is passed as ${0}, the heap block is "
	      "truncated to a minimal size and a heap block is returned that is semantically "
	      "equivalent to one returned by ${malloc(0)}\n"
	      "In the event of failure, the pre-existing heap-block passed for @ptr will remain unchanged") },
	{ "free", (DeeObject *)&ctypes_free, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid)\n"
	      "Release a previously allocated heap-block returned by one of ?Gmalloc, ?Gcalloc or ?Grealloc\n"
	      "If a NULL-pointer is passed for @ptr, this function has no effect and returns immediately") },
	{ "trymalloc", (DeeObject *)&ctypes_trymalloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmalloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "trycalloc", (DeeObject *)&ctypes_trycalloc, MODSYM_FNORMAL,
	  DOC("(size:?Dint)->?Aptr?Gvoid\n"
	      "(count:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gcalloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error") },
	{ "tryrealloc", (DeeObject *)&ctypes_tryrealloc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grealloc, but return a NULL-pointer if allocation isn't "
	      "possible due to lack of memory, rather than throwing a :NoMemory error\n"
	      "In this event, the pre-existing heap-block passed for @ptr is not freed") },
	{ "strdup", (DeeObject *)&ctypes_strdup, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint=!A!Pmax!Gsize_t)->?Aptr?Gchar\n"
	      "@throw NoMemory Insufficient memory\n"
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
	{ "trystrdup", (DeeObject *)&ctypes_trystrdup, MODSYM_FNORMAL,
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

	{ "memcpy", (DeeObject *)&ctypes_memcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a ?Aptr?Gvoid\n"
	      "Copies @size * @count bytes of memory from @src to @dst\n"
	      "Note that the source and destination ranges may not overlap") },
	{ "mempcpy", (DeeObject *)&ctypes_mempcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint,count=!1)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size * count} as a ?Aptr?Gvoid\n"
	      "Same as ?Gmemcpy, but returns ${dst + size * count}") },
	{ "memccpy", (DeeObject *)&ctypes_memccpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return The last modified by in @dst\n"
	      "Same as ?Gmemcpy, but stop after a byte equal to @needle is encountered in @src") },
	{ "memset", (DeeObject *)&ctypes_memset, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a ?Aptr?Gvoid\n"
	      "Set every byte in the range @dst+@size to equal @byte") },
	{ "mempset", (DeeObject *)&ctypes_mempset, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,byte:?Dint,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size} as a ?Aptr?Gvoid\n"
	      "Same as ?Gmemset, but returns ${dst + size}") },
	{ "bzero", (DeeObject *)&ctypes_bzero, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,size:?Dint,count=!1)\n"
	      "Same as ?Gmemset, but always fills memory with ${0}s") },
	{ "memmove", (DeeObject *)&ctypes_memmove, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns @dst as a ?Aptr?Gvoid\n"
	      "Same as ?Gmemcpy, but the source and destination ranges are allowed to overlap") },
	{ "mempmove", (DeeObject *)&ctypes_mempmove, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gvoid,src:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "@return Always re-returns ${dst + size} as a ?Aptr?Gvoid\n"
	      "Same as ?Gmemcpy, but returns ${dst + size}") },
	{ "memchr", (DeeObject *)&ctypes_memchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in the specified @haystack+@haystack_size "
	      "that equals @needle, or return a NULL-pointer if not found") },
	{ "memrchr", (DeeObject *)&ctypes_memrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as :memchr, but if @needle appears multiple times, return a pointer to the last instance") },
	{ "memlen", (DeeObject *)&ctypes_memlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Return the offset from @haystack of the first byte equal to @needle, or @haystack_size if now found") },
	{ "memrlen", (DeeObject *)&ctypes_memrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as :memlen, but if @needle appears multiple times, return the offset of the last instance") },
	{ "memend", (DeeObject *)&ctypes_memend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Return a pointer to the first byte in @haystack that is equal to @needle, or @haystack+@haystack_size if now found") },
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
	      "Same as ?Grawmemchr, but return the offset from @haystack") },
	{ "rawmemrchr", (DeeObject *)&ctypes_rawmemrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as :rawmemchr, but search in reverse, starting with ${haystack[-1]}\n"
	      "You can think of this function as a variant of ?Gmemrend that operates "
	      "on a buffer that spans the entirety of the available address space") },
	{ "rawmemrlen", (DeeObject *)&ctypes_rawmemrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrchr, but return the positive (unsigned) offset of the "
	      "matching byte, such that ${haystack + return} points to the byte in question") },
	{ "memxchr", (DeeObject *)&ctypes_memxchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxlen", (DeeObject *)&ctypes_memxlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxend", (DeeObject *)&ctypes_memxend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrchr", (DeeObject *)&ctypes_memxrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrlen", (DeeObject *)&ctypes_memxrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Dint\n"
	      "Same as ?Gmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "memxrend", (DeeObject *)&ctypes_memxrend, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint,haystack_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemrend, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxchr", (DeeObject *)&ctypes_rawmemxchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxlen", (DeeObject *)&ctypes_rawmemxlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrchr", (DeeObject *)&ctypes_rawmemxrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Grawmemrchr, but instead of comparing bytes for being equal, compare them for being different") },
	{ "rawmemxrlen", (DeeObject *)&ctypes_rawmemxrlen, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,needle:?Dint)->?Dint\n"
	      "Same as ?Grawmemrlen, but instead of comparing bytes for being equal, compare them for being different") },
	{ "bcmp", (DeeObject *)&ctypes_bcmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but only returns $0 when buffer are equal, and non-$0 otherwise") },
	{ "memcmp", (DeeObject *)&ctypes_memcmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Compare bytes from 2 buffers in @lhs and @rhs of equal @size, and "
	      "search for the first non-matching byte, returning ${< 0} if that byte "
	      "in @lhs is smaller than its counterpart in @rhs, ${> 0} if the opposite "
	      "is true, and ${== 0} no such byte exists") },
	{ "memcasecmp", (DeeObject *)&ctypes_memcasecmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,size:?Dint)->?Dint\n"
	      "Same as ?Gmemcmp, but bytes are casted as ASCII characters "
	      "into a common casing prior to comparison") },
	{ "memmem", (DeeObject *)&ctypes_memmem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Search for the first memory block in @haystack+@haystack_size that is equal to @needle+@needle_size "
	      "such that ${memcmp(candidate, needle, needle_size) == 0} and return a pointer to its starting "
	      "location in @haystack\n"
	      "If no such memory block exists, or @needle_size is $0, return a NULL-pointer instead") },
	{ "memcasemem", (DeeObject *)&ctypes_memcasemem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but perform case-insensitive comparisons, using ?Gmemcasecmp instead of ?Gmemcmp") },
	{ "memrmem", (DeeObject *)&ctypes_memrmem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemmem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memcasermem", (DeeObject *)&ctypes_memcasermem, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gvoid,haystack_size:?Dint,needle:?Aptr?Gvoid,needle_size:?Dint)->?Aptr?Gvoid\n"
	      "Same as ?Gmemcasemem, but in case more than 1 match exists, return a pointer to the last, rather than the first\n"
	      "When @needle_size is $0, always return a NULL-pointer") },
	{ "memrev", (DeeObject *)&ctypes_memrev, MODSYM_FNORMAL,
	  DOC("(buf:?Aptr?Gvoid,size:?Dint)->?Aptr?Gvoid\n"
	      "Reverse the order of bytes in the given @buf+@size, such that upon return its first "
	      "byte contains the old value of the last byte, and the last byte the value of the first, "
	      "and so on.") },
	{ "memfrob", (DeeObject *)&ctypes_memfrob, MODSYM_FNORMAL,
	  DOC("(buf:?Aptr?Gchar,size:?Dint)->?Aptr?Gvoid\n"
	      "xor all bytse within @buf with $42, implementing _very_ simplistic encryption") },

	{ "strlen", (DeeObject *)&ctypes_strlen, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Dint\n"
	      "Returns the length of a given @str in characters") },
	{ "strend", (DeeObject *)&ctypes_strend, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Return a pointer to the end of a given @str") },
	{ "strnlen", (DeeObject *)&ctypes_strnlen, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrlen, but limit the max number of scanned characters to @maxlen") },
	{ "strnend", (DeeObject *)&ctypes_strnend, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrend, but limit the max number of scanned characters to @maxlen") },
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
	      "Same as ?Gstrchr, but limit the max number of scanned characters to @maxlen") },
	{ "strnrchr", (DeeObject *)&ctypes_strnrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but limit the max number of scanned characters to @maxlen") },
	{ "stroff", (DeeObject *)&ctypes_stroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrchr, but return the offset of the found character from @haystack, or ${strlen(haystack)} if @needle wasn't found") },
	{ "strroff", (DeeObject *)&ctypes_strroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Dint\n"
	      "Same ?Gstrrchr, but return the offset of the found character from @haystack, or ${size_t.max} if @needle wasn't found") },
	{ "strnoff", (DeeObject *)&ctypes_strnoff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnchr, but return the offset of the found character from @haystack, or ${strnlen(haystack, maxlen)} if @needle wasn't found") },
	{ "strnroff", (DeeObject *)&ctypes_strnroff, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Dint\n"
	      "Same ?Gstrnrchr, but return the offset of the found character from @haystack, or ${size_t.max} if @needle wasn't found") },
	{ "strchrnul", (DeeObject *)&ctypes_strchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrchr, but return ${strend(haystack)} if @needle wasn't found") },
	{ "strrchrnul", (DeeObject *)&ctypes_strrchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strnchrnul", (DeeObject *)&ctypes_strnchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnchr, but return ${strnend(haystack, maxlen)} if @needle wasn't found") },
	{ "strnrchrnul", (DeeObject *)&ctypes_strnrchrnul, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnrchr, but return ${haystack - 1} if @needle wasn't found") },
	{ "strcmp", (DeeObject *)&ctypes_strcmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Compare the given strings @lhs and @rhs, returning ${<0}, "
	      "${==0} or ${>0} indicative of their relation to one-another") },
	{ "strncmp", (DeeObject *)&ctypes_strncmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrcmp, but limit the max number of compared characters to @maxlen") },
	{ "strcasecmp", (DeeObject *)&ctypes_strcasecmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but ignore casing") },
	{ "strncasecmp", (DeeObject *)&ctypes_strncasecmp, MODSYM_FNORMAL,
	  DOC("(lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint)->?Dint\n"
	      "Same as ?Gstrncmp, but ignore casing") },
	{ "strcpy", (DeeObject *)&ctypes_strcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)memcpy(dst, src, (strlen(src) + 1) * sizeof(char))}") },
	{ "strcat", (DeeObject *)&ctypes_strcat, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${({ local r = dst; strcpy(strend(dst), src); (char.ptr)r; })}") },
	{ "strncpy", (DeeObject *)&ctypes_strncpy, MODSYM_FNORMAL,
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
	{ "strncat", (DeeObject *)&ctypes_strncat, MODSYM_FNORMAL,
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
	{ "stpcpy", (DeeObject *)&ctypes_stpcpy, MODSYM_FNORMAL,
	  DOC("(dst:?Aptr?Gchar,src:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ${(char.ptr)mempcpy(dst, src, (strlen(src) + 1) * sizeof(char)) - 1}") },
	{ "stpncpy", (DeeObject *)&ctypes_stpncpy, MODSYM_FNORMAL,
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
	{ "strstr", (DeeObject *)&ctypes_strstr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strcasestr", (DeeObject *)&ctypes_strcasestr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Same as ?Gstrstr, but ignore casing") },
	{ "strnstr", (DeeObject *)&ctypes_strnstr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Find the first instance of @needle contained within @haystack, or return a NULL-pointer if none exists") },
	{ "strncasestr", (DeeObject *)&ctypes_strncasestr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrnstr, but ignore casing") },
	{ "strverscmp", (DeeObject *)&ctypes_strverscmp, MODSYM_FNORMAL,
	  DOC("(a:?Aptr?Gchar,b:?Aptr?Gchar)->?Dint\n"
	      "Same as ?Gstrcmp, but do special handling for version strings (s.a. ?Avercompare?Dstring)") },
	{ "index", (DeeObject *)&ctypes_strchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrchr") },
	{ "rindex", (DeeObject *)&ctypes_strrchr, MODSYM_FNORMAL,
	  DOC("(haystack:?Aptr?Gchar,needle:?Dint)->?Aptr?Gchar\n"
	      "Alias for ?Gstrrchr") },
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
	{ "strnlwr", (DeeObject *)&ctypes_strnlwr, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to lower-case and re-return the given @str") },
	{ "strnupr", (DeeObject *)&ctypes_strnupr, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Convert all characters in @str to upper-case and re-return the given @str") },
	{ "strset", (DeeObject *)&ctypes_strset, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,chr:?Dint)->?Aptr?Gchar\n"
	      "Set all characters in @str to @chr and re-return the given @str") },
	{ "strnset", (DeeObject *)&ctypes_strnset, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,chr:?Dint,maxlen:?Dint)->?Aptr?Gchar\n"
	      "Same as ?Gstrset, but limit the operator to up to @maxlen characters") },
	{ "strfry", (DeeObject *)&ctypes_strfry, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Randomly shuffle the order of characters in @str, creating an anagram") },
	{ "strsep", (DeeObject *)&ctypes_strsep, MODSYM_FNORMAL,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar") },
	{ "stresep", (DeeObject *)&ctypes_stresep, MODSYM_FNORMAL,
	  DOC("(stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar,escape:?Dint)->?Aptr?Gchar") },
	{ "strtok", (DeeObject *)&ctypes_strtok, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar)->?Aptr?Gchar\n"
	      "Split @str at each occurrence of @delim and return the resulting strings individually") },
	{ "strtok_r", (DeeObject *)&ctypes_strtok_r, MODSYM_FNORMAL,
	  DOC("(str:?Aptr?Gchar,delim:?Aptr?Gchar,save_ptr:?Aptr?Aptr?Gchar)->?Aptr?Gchar") },
	{ "basename", (DeeObject *)&ctypes_basename, MODSYM_FNORMAL,
	  DOC("(filename:?Aptr?Gchar)->?Aptr?Gchar") },

	/* Atomic functions */
	{ "atomic_cmpxch", (DeeObject *)&ctypes_atomic_cmpxch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind],weak=!f)->?Dbool\n"
	      "Do an atomic compare-and-exchange of memory at @ptr from @oldval to @newval\n"
	      "When @weak is true, the operation is allowed to fail sporadically, even when "
	      "memory at @ptr and @oldval are identical\n"
	      "This is a type-generic operation, with the address-width of the atomic operation "
	      "depending on the typing of @ptr. Supported widths are $1, $2, $4 and $8 bytes") },
	{ "atomic_cmpxch_val", (DeeObject *)&ctypes_atomic_cmpxch_val, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,oldval:?Q!A!Aptr!Pind],newval:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Same as ?Gatomic_cmpxch, except that rather than returning !t or !f indicative of "
	      "the success of the exchange, the #Ireal old value read from @ptr is returned. If "
	      "this is equal to @oldval, the operation was successful. If not, memory pointed-to "
	      "by @ptr remains unchanged") },
	{ "atomic_fetchadd", (DeeObject *)&ctypes_atomic_fetchadd, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind += addend; r; })}") },
	{ "atomic_fetchsub", (DeeObject *)&ctypes_atomic_fetchsub, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind -= addend; r; })}") },
	{ "atomic_fetchand", (DeeObject *)&ctypes_atomic_fetchand, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind &= mask; r; })}") },
	{ "atomic_fetchor", (DeeObject *)&ctypes_atomic_fetchor, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind |= mask; r; })}") },
	{ "atomic_fetchxor", (DeeObject *)&ctypes_atomic_fetchxor, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind ^= mask; r; })}") },
	{ "atomic_fetchnand", (DeeObject *)&ctypes_atomic_fetchnand, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ local r = copy ptr.ind; ptr.ind = ~(ptr.ind & mask); r; })}") },
	{ "atomic_addfetch", (DeeObject *)&ctypes_atomic_addfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind += addend; copy ptr.ind; })}") },
	{ "atomic_subfetch", (DeeObject *)&ctypes_atomic_subfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind -= addend; copy ptr.ind; })}") },
	{ "atomic_andfetch", (DeeObject *)&ctypes_atomic_andfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind &= mask; copy ptr.ind; })}") },
	{ "atomic_orfetch", (DeeObject *)&ctypes_atomic_orfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind |= mask; copy ptr.ind; })}") },
	{ "atomic_xorfetch", (DeeObject *)&ctypes_atomic_xorfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind ^= mask; copy ptr.ind; })}") },
	{ "atomic_nandfetch", (DeeObject *)&ctypes_atomic_nandfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${({ ptr.ind = ~(ptr.ind & mask); copy ptr.ind; })}") },
	{ "atomic_add", (DeeObject *)&ctypes_atomic_add, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind += addend}") },
	{ "atomic_sub", (DeeObject *)&ctypes_atomic_sub, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,addend:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind -= addend}") },
	{ "atomic_and", (DeeObject *)&ctypes_atomic_and, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind &= mask}") },
	{ "atomic_or", (DeeObject *)&ctypes_atomic_or, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind |= mask}") },
	{ "atomic_xor", (DeeObject *)&ctypes_atomic_xor, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind ^= mask}") },
	{ "atomic_nand", (DeeObject *)&ctypes_atomic_nand, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,mask:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = ~(ptr.ind & mask)}") },
	{ "atomic_write", (DeeObject *)&ctypes_atomic_write, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,value:?Q!A!Aptr!Pind])\n"
	      "Atomic operation for ${ptr.ind = value}") },
	{ "atomic_fetchinc", (DeeObject *)&ctypes_atomic_fetchinc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind++}") },
	{ "atomic_fetchdec", (DeeObject *)&ctypes_atomic_fetchdec, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${ptr.ind--}") },
	{ "atomic_incfetch", (DeeObject *)&ctypes_atomic_incfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_decfetch", (DeeObject *)&ctypes_atomic_decfetch, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${--ptr.ind}") },
	{ "atomic_read", (DeeObject *)&ctypes_atomic_read, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)->?Q!A!Aptr!Pind]\n"
	      "Atomic operation for ${copy ptr.ind}") },
	{ "atomic_inc", (DeeObject *)&ctypes_atomic_inc, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${++ptr.ind}") },
	{ "atomic_dec", (DeeObject *)&ctypes_atomic_dec, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Atomic operation for ${--ptr.ind}") },

	/* Futex functions */
	{ "futex_wakeone", (DeeObject *)&ctypes_futex_wakeone, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake at most 1 thread that is waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wakeall", (DeeObject *)&ctypes_futex_wakeall, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured)\n"
	      "Wake all threads that are waiting for @ptr to change (s.a. ?Gfutex_wait)") },
	{ "futex_wait", (DeeObject *)&ctypes_futex_wait, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind])\n"
	      "Atomically check if ${ptr.ind == expected}. If this is isn't the case, return immediatly. "
	      /**/ "Otherwise, wait until another thread makes a call to ?Gfutex_wakeone or ?Gfutex_wakeall, "
	      /**/ "or until the #I{stars align} (by which I mean that this function might also return sporadically)\n"
	      "This function can be used to form the basis for any other, arbitrary synchronization "
	      /**/ "primitive (mutex, semaphore, condition-variable, events, #Ianything)") },
	{ "futex_timedwait", (DeeObject *)&ctypes_futex_timedwait, MODSYM_FNORMAL,
	  DOC("(ptr:?Aptr?GStructured,expected:?Q!A!Aptr!Pind],timeout_nanoseconds:?Dint)->?Dbool\n"
	      "@return true You were woken up, either sporadically, because the value of ${ptr.ind} differs "
	      /*        */ "from @expected, or because another thread called ?Gfutex_wakeone or ?Gfutex_wakeall\n"
	      "@return false The given @timeout_nanoseconds has expired\n"
	      "Atomically check if ${ptr.ind == expected}. If this is isn't the case, immediatly return !t. "
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
	libctypes_fini_type((DeeSTypeObject *)&DeeStructured_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeePointer_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeLValue_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeArray_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCFunction_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCVoid_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCChar_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCWChar_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCChar16_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCChar32_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCBool_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCInt8_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCInt16_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCInt32_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCInt64_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUInt8_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUInt16_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUInt32_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUInt64_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCFloat_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCDouble_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCLDouble_Type);

#ifdef CONFIG_SUCHAR_NEEDS_OWN_TYPE
	libctypes_fini_type((DeeSTypeObject *)&DeeCSChar_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUChar_Type);
#endif /* CONFIG_SUCHAR_NEEDS_OWN_TYPE */

#ifdef CONFIG_SHORT_NEEDS_OWN_TYPE
	libctypes_fini_type((DeeSTypeObject *)&DeeCShort_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUShort_Type);
#endif /* CONFIG_SHORT_NEEDS_OWN_TYPE */

#ifdef CONFIG_INT_NEEDS_OWN_TYPE
	libctypes_fini_type((DeeSTypeObject *)&DeeCInt_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCUInt_Type);
#endif /* CONFIG_INT_NEEDS_OWN_TYPE */

#ifdef CONFIG_LONG_NEEDS_OWN_TYPE
	libctypes_fini_type((DeeSTypeObject *)&DeeCLong_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCULong_Type);
#endif /* CONFIG_LONG_NEEDS_OWN_TYPE */

#ifdef CONFIG_LLONG_NEEDS_OWN_TYPE
	libctypes_fini_type((DeeSTypeObject *)&DeeCLLong_Type);
	libctypes_fini_type((DeeSTypeObject *)&DeeCULLong_Type);
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
