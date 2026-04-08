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
#include <deemon/int.h>             /* DeeArg_UnpackStruct1XOr2X, DeeArg_UnpackStruct2X, UNPuSIZ, UNPxSIZ, _DeeArg_AsObject */
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

/* Whitelist of some C libraries where we know that `malloc(0)'
 * doesn't return `NULL' unless it's *actually* out-of-memory. */
#ifndef __MALLOC_ZERO_IS_NONNULL
#ifndef __KOS_SYSTEM_HEADERS__
#if defined(_MSC_VER)
#define __MALLOC_ZERO_IS_NONNULL
#undef __REALLOC_ZERO_IS_NONNULL /* Nope, `realloc(p, 0)' acts like `free(p)'... */
#elif defined(__GLIBC__) || defined(__GNU_LIBRARY__)
#define __MALLOC_ZERO_IS_NONNULL
#define __REALLOC_ZERO_IS_NONNULL
#endif /* ... */
#endif /* !__KOS_SYSTEM_HEADERS__ */
#endif /* !__MALLOC_ZERO_IS_NONNULL */


#undef C_MALLOC_USE_NATIVE
/* Check for minimal requirements to implement the full suite of heap functions */
#if (defined(CONFIG_HAVE_realloc) && \
     defined(CONFIG_HAVE_free) &&    \
     defined(CONFIG_HAVE_malloc_usable_size))
#define C_MALLOC_USE_NATIVE
#endif


#ifdef C_MALLOC_USE_NATIVE
#ifdef CONFIG_HAVE_malloc
#define c_trymalloc(n)          malloc(n)
#elif defined(CONFIG_HAVE_calloc)
#define c_trymalloc(n)          calloc(1, n)
#else /* ... */
#define c_trymalloc(n)          realloc(NULL, n)
#undef __MALLOC_ZERO_IS_NONNULL
#ifdef __REALLOC_ZERO_IS_NONNULL
#define __MALLOC_ZERO_IS_NONNULL
#endif /* __REALLOC_ZERO_IS_NONNULL */
#endif /* !... */
#ifdef CONFIG_HAVE_calloc
#define c_trycalloc(n)          calloc(1, n)
#endif /* !CONFIG_HAVE_calloc */
#define c_tryrealloc(p, n)      realloc(p, n)
#define c_free(p)               free(p)
#define c_malloc_usable_size(p) malloc_usable_size(p)
#else /* C_MALLOC_USE_NATIVE */
#define c_malloc(n)             (Dee_Malloc)(n)
#define c_calloc(n)             (Dee_Calloc)(n)
#define c_realloc(p, n)         (Dee_Realloc)(p, n)
#define c_trymalloc(n)          (Dee_TryMalloc)(n)
#define c_trycalloc(n)          (Dee_TryCalloc)(n)
#define c_tryrealloc(p, n)      (Dee_TryRealloc)(p, n)
#define c_free(p)               (Dee_Free)(p)
#define c_malloc_usable_size(p) (Dee_MallocUsableSize)(p)
#endif /* !C_MALLOC_USE_NATIVE */



/*[[[deemon (print_CMethod from rt.gen.unpack)("free", """
	ptr:ctypes:void*
""", visi: "INTERN");]]]*/
#define c_malloc_free_params "ptr:?Aptr?Gvoid"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_free_f_impl(void *ptr);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_malloc_free_f(DeeObject *__restrict arg0) {
	union pointer ptr;
	if unlikely(DeeObject_AsPointer(arg0, &CVoid_Type, &ptr))
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
	CTYPES_FAULTPROTECT(c_free(ptr), return NULL);
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!num_bytes)
		num_bytes = 1;
#endif /* !__MALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
#ifdef c_malloc
	ptr = c_malloc(num_bytes);
	if unlikely(!ptr)
		goto err;
#else /* c_malloc */
	for (;;) {
		ptr = c_trymalloc(num_bytes);
		if likely(ptr)
			break;
		if (!Dee_ReleaseSystemMemory())
			goto err;
	}
#endif /* !c_malloc */
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		c_free(ptr);
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
	if unlikely(DeeObject_AsPointer(args.raw_ptr, &CVoid_Type, &ptr))
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!new_size)
		new_size = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
#ifdef c_realloc
	CTYPES_FAULTPROTECT(result_ptr = c_realloc(ptr, new_size),
	                    goto err_r);
#else /* c_realloc */
	for (;;) {
		CTYPES_FAULTPROTECT(result_ptr = c_tryrealloc(ptr, new_size), goto err_r);
		if likely(result_ptr)
			break;
		if unlikely(!Dee_ReleaseSystemMemory())
			goto err_r;
	}
#endif /* !c_realloc */
	if unlikely(!result_ptr)
		goto err_r;

	/* Update the resulting pointer. */
	Object_AsCPointer(result)->cp_value.ptr = result_ptr;
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!total)
		total = 1;
#endif /* !__MALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
#if defined(c_calloc) || defined(c_malloc)
#ifdef c_calloc
	ptr = c_calloc(total);
#else /* c_calloc */
	ptr = c_malloc(total);
#endif /* !c_calloc */
	if unlikely(!ptr)
		goto err;
#ifndef c_calloc
	bzero(ptr, total);
#endif /* !c_calloc */
#else /* c_calloc || c_malloc */
	for (;;) {
#ifdef c_trycalloc
		ptr = c_trycalloc(total);
#else  /* c_trycalloc */
		ptr = c_trymalloc(total);
#endif /* !c_trycalloc */
		if likely(ptr)
			break;
		if unlikely(!Dee_ReleaseSystemMemory())
			goto err;
	}
#ifndef c_trycalloc
	bzero(ptr, total);
#endif /* !c_trycalloc */
#endif /* !c_calloc && !c_malloc */
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		c_free(ptr);
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!num_bytes)
		num_bytes = 1;
#endif /* !__MALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
	ptr    = c_trymalloc(num_bytes);
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		c_free(ptr);
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
	if unlikely(DeeObject_AsPointer(args.raw_ptr, &CVoid_Type, &ptr))
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!new_size)
		new_size = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
	CTYPES_FAULTPROTECT(result_ptr = c_tryrealloc(ptr, new_size),
	                    goto err_r);

	/* Update the resulting pointer. */
	Object_AsCPointer(result)->cp_value.ptr = result_ptr;
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
#ifdef C_MALLOC_USE_NATIVE
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!total)
		total = 1;
#endif /* !__MALLOC_ZERO_IS_NONNULL */
#endif /* C_MALLOC_USE_NATIVE */
#ifdef c_trycalloc
		ptr = c_trycalloc(total);
#else /* c_trycalloc */
		ptr = c_trymalloc(total);
		if (ptr)
			bzero(ptr, total);
#endif /* !c_trycalloc */
	}
	if unlikely(!ptr)
		goto err;
	result = DeePointer_NewVoid(ptr);
	if unlikely(!result)
		c_free(ptr);
	return result;
err:
	return NULL;
}



/*[[[deemon (print_CMethod from rt.gen.unpack)("strdup", """
	string:ctypes:char_const*, size_t maxlen = (size_t)-1;
""", visi: "INTERN");]]]*/
#define c_malloc_strdup_params "string:?Aptr?Gchar,maxlen=!-1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_strdup_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_strdup_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "strdup", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
	if unlikely(DeeObject_AsPointer(args.raw_string, &CChar_Type, &string))
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
#ifdef c_malloc
	resptr = (char *)c_malloc((len + 1) * sizeof(char));
	if unlikely(!resptr)
		goto err;
#else /* c_malloc */
	for (;;) {
		resptr = (char *)c_trymalloc((len + 1) * sizeof(char));
		if likely(resptr)
			break;
		if (!Dee_ReleaseSystemMemory())
			goto err;
	}
#endif /* !c_malloc */
	CTYPES_FAULTPROTECT(memcpyc(resptr, string, len, sizeof(char)), goto err_r);
	resptr[len] = '\0';
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	c_free(resptr);
err:
	return NULL;
}


/*[[[deemon (print_CMethod from rt.gen.unpack)("trystrdup", """
	string:ctypes:char_const*, size_t maxlen = (size_t)-1;
""", visi: "INTERN");]]]*/
#define c_malloc_trystrdup_params "string:?Aptr?Gchar,maxlen=!-1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_trystrdup_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_malloc_trystrdup_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	args.maxlen = (size_t)-1;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "trystrdup", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPxSIZ, DeeObject_AsSizeM1);
	if unlikely(DeeObject_AsPointer(args.raw_string, &CChar_Type, &string))
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
	resptr = (char *)c_trymalloc((len + 1) * sizeof(char));
	if likely(resptr) {
		CTYPES_FAULTPROTECT(memcpyc(resptr, string, len, sizeof(char)), goto err_r);
		resptr[len] = '\0';
	}
	result = DeePointer_NewChar(resptr);
	if unlikely(!result)
		goto err_r;
	return result;
err_r:
	c_free(resptr);
	return NULL;
}


#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
/*[[[deemon (print_CMethod from rt.gen.unpack)("malloc_usable_size", """
	ptr:ctypes:void*
""", visi: "INTERN");]]]*/
#define c_malloc_malloc_usable_size_params "ptr:?Aptr?Gvoid"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_malloc_usable_size_f_impl(void *ptr);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_malloc_malloc_usable_size_f(DeeObject *__restrict arg0) {
	union pointer ptr;
	if unlikely(DeeObject_AsPointer(arg0, &CVoid_Type, &ptr))
		goto err;
	return c_malloc_malloc_usable_size_f_impl(ptr.pvoid);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_malloc_malloc_usable_size, &c_malloc_malloc_usable_size_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_malloc_malloc_usable_size_f_impl(void *ptr)
/*[[[end]]]*/
{
	size_t result;
	CTYPES_FAULTPROTECT(result = c_malloc_usable_size(ptr), return NULL);
	return DeeInt_NewSize(result);
}
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_MALLOC_C */
