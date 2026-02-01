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
#ifndef GUARD_DEX_CTYPES_C_MALLOC_C
#define GUARD_DEX_CTYPES_C_MALLOC_C 1
#define DEE_SOURCE
#define _GNU_SOURCE 1 /* strnlen() */

#include "libctypes.h"
/**/


#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_Free */
#include <deemon/arg.h>             /* DeeArg_UnpackStruct1XOr2X, DeeArg_UnpackStruct2X, UNPuSIZ, UNPxSIZ, _DeeArg_AsObject */
#include <deemon/error-rt.h>        /* DeeRT_ErrIntegerOverflowUMul */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_AsSize, DeeObject_AsSizeM1, Dee_DecrefDokill */
#include <deemon/objmethod.h>       /*  */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_strnlen, memcpyc */
#include <deemon/type.h>            /* METHOD_FNORMAL */

#include <hybrid/overflow.h> /* OVERFLOW_UMUL */

#include "c_api.h" /* Prototypes... */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */


/*[[[deemon (print_CMethod from rt.gen.unpack)("free", """
	ptr:ctypes:void*
""", visi: "INTERN");]]]*/
#define c_malloc_free_params "ptr:?Aptr?Gvoid"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_free_f_impl(void *ptr);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_malloc_free_f(DeeObject *__restrict arg0) {
	union pointer ptr;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCVoid_Type, &ptr))
		goto err;
	return c_malloc_free_f_impl(ptr.pvoid);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_malloc_free, &c_malloc_free_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_free_f_impl(void *ptr)
/*[[[end]]]*/
{
	/* TODO: This (and the other APIs) should use the system's native
	 *       malloc()+free() (if available) for the sake of compatibility
	 *       with other dynamically loaded shared, native libraries. */
	CTYPES_FAULTPROTECT(Dee_Free(ptr), return NULL);
	return_none;
}



/*[[[deemon (print_CMethod from rt.gen.unpack)("malloc", """
	size_t num_bytes
""", visi: "INTERN");]]]*/
#define c_malloc_malloc_params "num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_malloc_f_impl(size_t num_bytes);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_malloc_malloc_f(DeeObject *__restrict arg0) {
	size_t num_bytes;
	if (DeeObject_AsSize(arg0, &num_bytes))
		goto err;
	return c_malloc_malloc_f_impl(num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_malloc_malloc, &c_malloc_malloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_malloc_f_impl(size_t num_bytes)
/*[[[end]]]*/
{
	void *ptr;
	DREF DeeObject *result;
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


/*[[[deemon (print_CMethod from rt.gen.unpack)("realloc", """
	ptr:ctypes:void*, size_t new_size
""", visi: "INTERN");]]]*/
#define c_malloc_realloc_params "ptr:?Aptr?Gvoid,new_size:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_realloc_f_impl(void *ptr, size_t new_size);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_realloc_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_ptr;
		size_t new_size;
	} args;
	union pointer ptr;
	DeeArg_UnpackStruct2X(err, argc, argv, "realloc", &args, &args.raw_ptr, "o", _DeeArg_AsObject, &args.new_size, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_ptr, &DeeCVoid_Type, &ptr))
		goto err;
	return c_malloc_realloc_f_impl(ptr.pvoid, args.new_size);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_realloc, &c_malloc_realloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_realloc_f_impl(void *ptr, size_t new_size)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	void *result_ptr;

	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_FAULTPROTECT(result_ptr = Dee_Realloc(ptr, new_size),
	                    goto err_r);
	if unlikely(!result_ptr)
		goto err_r;

	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = result_ptr;
	return result;
err_r:
	Dee_DecrefDokill(result);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("calloc", """
	size_t count, size_t num_bytes = 1;
""", visi: "INTERN");]]]*/
#define c_malloc_calloc_params "count:?Dint,num_bytes=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_calloc_f_impl(size_t count, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_calloc_f(size_t argc, DeeObject *const *argv) {
	struct {
		size_t count;
		size_t num_bytes;
	} args;
	args.num_bytes = 1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "calloc", &args, &args.count, UNPuSIZ, DeeObject_AsSize, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	return c_malloc_calloc_f_impl(args.count, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_calloc, &c_malloc_calloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_calloc_f_impl(size_t count, size_t num_bytes)
/*[[[end]]]*/
{
	void *ptr;
	DREF DeeObject *result;
	size_t total;
	/* Check for allocation overflow. */
	if (OVERFLOW_UMUL(count, num_bytes, &total)) {
		DeeRT_ErrIntegerOverflowUMul(count, num_bytes);
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



/*[[[deemon (print_CMethod from rt.gen.unpack)("trymalloc", """
	size_t num_bytes
""", visi: "INTERN");]]]*/
#define c_malloc_trymalloc_params "num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trymalloc_f_impl(size_t num_bytes);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_malloc_trymalloc_f(DeeObject *__restrict arg0) {
	size_t num_bytes;
	if (DeeObject_AsSize(arg0, &num_bytes))
		goto err;
	return c_malloc_trymalloc_f_impl(num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_malloc_trymalloc, &c_malloc_trymalloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trymalloc_f_impl(size_t num_bytes)
/*[[[end]]]*/
{
	void *ptr;
	DREF DeeObject *result;
	ptr    = Dee_TryMalloc(num_bytes);
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		Dee_Free(ptr);
	return result;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("tryrealloc", """
	ptr:ctypes:void*, size_t new_size
""", visi: "INTERN");]]]*/
#define c_malloc_tryrealloc_params "ptr:?Aptr?Gvoid,new_size:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_tryrealloc_f_impl(void *ptr, size_t new_size);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_tryrealloc_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_ptr;
		size_t new_size;
	} args;
	union pointer ptr;
	DeeArg_UnpackStruct2X(err, argc, argv, "tryrealloc", &args, &args.raw_ptr, "o", _DeeArg_AsObject, &args.new_size, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_ptr, &DeeCVoid_Type, &ptr))
		goto err;
	return c_malloc_tryrealloc_f_impl(ptr.pvoid, args.new_size);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_tryrealloc, &c_malloc_tryrealloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_tryrealloc_f_impl(void *ptr, size_t new_size)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	void *result_ptr;

	/* Allocate the resulting pointer _before_ doing the realloc().
	 * This way, we don't run the chance to cause an exception
	 * after we've already successfully reallocated the user-pointer. */
	result = DeePointer_NewVoid(0);
	if unlikely(!result)
		goto err;
	CTYPES_FAULTPROTECT(result_ptr = Dee_TryRealloc(ptr, new_size),
	                    goto err_r);

	/* Update the resulting pointer. */
	((struct pointer_object *)result)->p_ptr.ptr = result_ptr;
	return result;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
err_r:
	Dee_DecrefDokill(result);
#endif /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
err:
	return NULL;
}

/*[[[deemon (print_CMethod from rt.gen.unpack)("trycalloc", """
	size_t count;
	size_t num_bytes = 1;
""", visi: "INTERN");]]]*/
#define c_malloc_trycalloc_params "count:?Dint,num_bytes=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trycalloc_f_impl(size_t count, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_trycalloc_f(size_t argc, DeeObject *const *argv) {
	struct {
		size_t count;
		size_t num_bytes;
	} args;
	args.num_bytes = 1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trycalloc", &args, &args.count, UNPuSIZ, DeeObject_AsSize, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	return c_malloc_trycalloc_f_impl(args.count, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_trycalloc, &c_malloc_trycalloc_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trycalloc_f_impl(size_t count, size_t num_bytes)
/*[[[end]]]*/
{
	void *ptr;
	DREF DeeObject *result;
	size_t total;
	if (OVERFLOW_UMUL(count, num_bytes, &total)) {
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



/*[[[deemon (print_CMethod from rt.gen.unpack)("strdup", """
	string:ctypes:char_const*, size_t maxlen = (size_t)-1;
""", visi: "INTERN");]]]*/
#define c_malloc_strdup_params "string:?Aptr?Gvoid,maxlen=!-1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_strdup_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_strdup_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "strdup", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
	if unlikely(DeeObject_AsPointer(args.raw_string, &DeeCChar_Type, &string))
		goto err;
	return c_malloc_strdup_f_impl(string.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_strdup, &c_malloc_strdup_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_strdup_f_impl(char const *string, size_t maxlen)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	size_t len;
	char *resptr;
	CTYPES_FAULTPROTECT(len = strnlen(string, maxlen), goto err);
	resptr = (char *)Dee_Mallocc(len + 1, sizeof(char));
	if unlikely(!resptr)
		goto err;
	CTYPES_FAULTPROTECT(memcpyc(resptr, string, len, sizeof(char)), goto err_r);
	resptr[len] = '\0';
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	Dee_Free(resptr);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("trystrdup", """
	string:ctypes:char_const*, size_t maxlen = (size_t)-1;
""", visi: "INTERN");]]]*/
#define c_malloc_trystrdup_params "string:?Aptr?Gvoid,maxlen=!-1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trystrdup_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_trystrdup_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trystrdup", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
	if unlikely(DeeObject_AsPointer(args.raw_string, &DeeCChar_Type, &string))
		goto err;
	return c_malloc_trystrdup_f_impl(string.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_malloc_trystrdup, &c_malloc_trystrdup_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trystrdup_f_impl(char const *string, size_t maxlen)
/*[[[end]]]*/
{
	DREF DeeObject *result;
	size_t len;
	char *resptr;
	CTYPES_FAULTPROTECT(len = strnlen(string, maxlen), return NULL);
	resptr = (char *)Dee_TryMallocc(len + 1, sizeof(char));
	if likely(resptr) {
		CTYPES_FAULTPROTECT(memcpyc(resptr, string, len, sizeof(char)), goto err_r);
		resptr[len] = '\0';
	}
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	Dee_Free(resptr);
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_MALLOC_C */
