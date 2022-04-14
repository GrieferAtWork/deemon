/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_C_MALLOC_C
#define GUARD_DEX_CTYPES_C_MALLOC_C 1
#define DEE_SOURCE 1
#define _GNU_SOURCE 1 /* strnlen() */

#include "libctypes.h"
/**/

#include "c_api.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* strnlen() */

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */


INTERN WUNUSED DREF DeeObject *DCALL
capi_free(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *uptr;
	union pointer ptr;
	if (DeeArg_Unpack(argc, argv, "o:free", &uptr))
		goto err;
	if (DeeObject_AsPointer(uptr, &DeeCVoid_Type, &ptr))
		goto err;
	CTYPES_RECURSIVE_FAULTPROTECT(Dee_Free(ptr.ptr), goto err);
	return_none;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_malloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":malloc", &num_bytes))
		goto err;
	ptr = Dee_Malloc(num_bytes);
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
	DREF DeeObject *uptr;
	DREF DeeObject *result;
	union pointer ptr;
	size_t new_size;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":realloc", &uptr, &new_size))
		goto err;
	if (DeeObject_AsPointer(uptr, &DeeCVoid_Type, &ptr))
		goto err;
	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_RECURSIVE_FAULTPROTECT(
	ptr.ptr = Dee_Realloc(ptr.ptr, new_size),
	goto err_r);
	if unlikely(!ptr.ptr)
		goto err_r;
	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_calloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
	size_t count, num_bytes = 1, total;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|" UNPuSIZ ":calloc", &count, &num_bytes))
		goto err;
	total = count * num_bytes;
	/* Check for allocation overflow. */
	if unlikely((total < count || total < num_bytes) &&
	            count && num_bytes) {
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "calloc allocation total of `%Iu * %Iu' is overflowing",
		                count, num_bytes);
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
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":trymalloc", &num_bytes))
		goto err;
	ptr    = Dee_TryMalloc(num_bytes);
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_tryrealloc(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *uptr;
	DREF DeeObject *result;
	union pointer ptr;
	size_t new_size;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":tryrealloc", &uptr, &new_size))
		goto err;
	if (DeeObject_AsPointer(uptr, &DeeCVoid_Type, &ptr))
		goto err;
	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_RECURSIVE_FAULTPROTECT(
	ptr.ptr = Dee_TryRealloc(ptr.ptr, new_size),
	goto err_r);
	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = ptr.ptr;
	return result;
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
err_r:
	Dee_Decref(result);
#endif /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_trycalloc(size_t argc, DeeObject *const *argv) {
	void *ptr;
	DREF DeeObject *result;
	size_t count, num_bytes = 1, total;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|" UNPuSIZ ":trycalloc", &count, &num_bytes))
		goto err;
	total = count * num_bytes;
	/* Check for allocation overflow. */
	if unlikely((total < count || total < num_bytes) &&
	            count && num_bytes) {
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
	size_t len, maxlen = (size_t)-1;
	DeeObject *str_ob;
	union pointer str;
	void *resptr;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":strdup", &str_ob, &maxlen))
		goto err;
	if (DeeObject_AsPointer(str_ob, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED_STRNLEN(len, str.pchar, maxlen, goto err);
	resptr = Dee_Malloc((len + 1) * sizeof(char));
	if unlikely(!resptr)
		goto err;
	CTYPES_PROTECTED_MEMCPY(resptr, str.pchar, len * sizeof(char), goto err_r);
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
	size_t len, maxlen = (size_t)-1;
	DeeObject *str_ob;
	union pointer str;
	void *resptr;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":trystrdup", &str_ob, &maxlen))
		goto err;
	if (DeeObject_AsPointer(str_ob, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED_STRNLEN(len, str.pchar, maxlen, goto err);
	resptr = Dee_TryMalloc((len + 1) * sizeof(char));
	if likely(resptr) {
		CTYPES_PROTECTED_MEMCPY(resptr, str.pchar, len * sizeof(char), goto err_r);
		((char *)resptr)[len] = '\0';
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
