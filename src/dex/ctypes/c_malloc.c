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
#ifndef GUARD_DEX_CTYPES_C_MALLOC_C
#define GUARD_DEX_CTYPES_C_MALLOC_C 1
#define DEE_SOURCE
#define _GNU_SOURCE 1 /* strnlen() */

#include "libctypes.h"
/**/


#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* strnlen() */

#include <hybrid/overflow.h>
/**/

#include "c_api.h" /* Prototypes... */
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */


INTERN WUNUSED DREF DeeObject *DCALL
capi_free(size_t argc, DeeObject *const *argv) {
	union pointer ptr;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("free", params: "ptr");]]]*/
	struct {
		DeeObject *ptr;
	} args;
	DeeArg_Unpack1(err, argc, argv, "free", &args.ptr);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.ptr, &DeeCVoid_Type, &ptr))
		goto err;
	CTYPES_FAULTPROTECT(Dee_Free(ptr.ptr),
	                    goto err);
	return_none;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_malloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("malloc", params: "size_t num_bytes");]]]*/
	struct {
		size_t num_bytes;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "malloc", &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	ptr = Dee_Malloc(args.num_bytes);
	if unlikely(!ptr)
		goto err;
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_realloc(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	union pointer ptr;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("realloc", params: "ptr, size_t new_size");]]]*/
	struct {
		DeeObject *ptr;
		size_t new_size;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "realloc", &args, &args.ptr, "o", _DeeArg_AsObject, &args.new_size, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.ptr, &DeeCVoid_Type, &ptr))
		goto err;

	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_FAULTPROTECT(ptr.ptr = Dee_Realloc(ptr.ptr, args.new_size),
	                    goto err_r);
	if unlikely(!ptr.ptr)
		goto err_r;

	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
	return result;
err_r:
	Dee_DecrefDokill(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_calloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("calloc", params: "size_t count, size_t num_bytes = 1;");]]]*/
	struct {
		size_t count;
		size_t num_bytes;
	} args;
	args.num_bytes = 1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "calloc", &args, &args.count, UNPuSIZ, DeeObject_AsSize, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	/* Check for allocation overflow. */
	if (OVERFLOW_UMUL(args.count, args.num_bytes, &total)) {
		DeeRT_ErrIntegerOverflowUMul(args.count, args.num_bytes);
		goto err;
	}
	ptr = Dee_Calloc(total);
	if unlikely(!ptr)
		goto err;
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
err:
	return NULL;
}



INTERN WUNUSED DREF DeeObject *DCALL
capi_trymalloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("trymalloc", params: "size_t num_bytes");]]]*/
	struct {
		size_t num_bytes;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "trymalloc", &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	ptr    = Dee_TryMalloc(args.num_bytes);
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_tryrealloc(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	union pointer ptr;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryrealloc", params: "ptr, size_t new_size");]]]*/
	struct {
		DeeObject *ptr;
		size_t new_size;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "tryrealloc", &args, &args.ptr, "o", _DeeArg_AsObject, &args.new_size, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.ptr, &DeeCVoid_Type, &ptr))
		goto err;

	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_FAULTPROTECT(ptr.ptr = Dee_TryRealloc(ptr.ptr, args.new_size),
	                    goto err_r);

	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
	return result;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err_r:
	Dee_DecrefDokill(result);
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_trycalloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("trycalloc", params: "size_t count, size_t num_bytes = 1;");]]]*/
	struct {
		size_t count;
		size_t num_bytes;
	} args;
	args.num_bytes = 1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trycalloc", &args, &args.count, UNPuSIZ, DeeObject_AsSize, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (OVERFLOW_UMUL(args.count, args.num_bytes, &total)) {
		ptr = NULL;
	} else {
		ptr = Dee_TryCalloc(total);
	}
	if unlikely(!ptr)
		goto err;
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
err:
	return NULL;
}



INTERN WUNUSED DREF DeeObject *DCALL
capi_strdup(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t len;
	union pointer str;
	void *resptr;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("trycalloc", params: """
	DeeObject *string;
	size_t maxlen = (size_t)-1;
""");]]]*/
	struct {
		DeeObject *string;
		size_t maxlen;
	} args;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trycalloc", &args, &args.string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.string, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(len = strnlen(str.pchar, args.maxlen),
	                    goto err);
	resptr = Dee_Mallocc(len + 1, sizeof(char));
	if unlikely(!resptr)
		goto err;
	CTYPES_FAULTPROTECT(memcpyc(resptr, str.pchar, len, sizeof(char)),
	                    goto err_r);
	((char *)resptr)[len] = '\0';
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	Dee_Free(resptr);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_trystrdup(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t len;
	union pointer str;
	void *resptr;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("trystrdup", params: """
	DeeObject *string;
	size_t maxlen = (size_t)-1;
""");]]]*/
	struct {
		DeeObject *string;
		size_t maxlen;
	} args;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trystrdup", &args, &args.string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.string, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(len = strnlen(str.pchar, args.maxlen),
	                    goto err);
	resptr = Dee_TryMallocc(len + 1, sizeof(char));
	if likely(resptr) {
		CTYPES_FAULTPROTECT({
			*(char *)mempcpyc(resptr, str.pchar, len, sizeof(char)) = '\0';
		}, goto err_r);
	}
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	Dee_Free(resptr);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_MALLOC_C */
