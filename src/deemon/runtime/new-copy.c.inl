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
#ifdef __INTELLISENSE__
#include "new.c"
//#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC0
#define DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC1
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>  /* DeeObject_* */
#include <deemon/gc.h>     /* DeeGCObject_Free, DeeGCObject_Malloc, DeeGC_Track */
#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_DecrefNokill, Dee_Decref_unlikely, Dee_TYPE, OBJECT_HEAD */
#include <deemon/type.h>   /* DeeObject_InitHeap, DeeObject_InitStatic */

#include <stddef.h> /* NULL */

#if (defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC1)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

#ifdef DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC0
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free0_HeapType0_GC0
#elif defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free0_HeapType0_GC1
#define LOCAL_PARAM_GC              1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free0_HeapType1_GC0
#define LOCAL_PARAM_HeapType        1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free0_HeapType1_GC1
#define LOCAL_PARAM_HeapType        1
#define LOCAL_PARAM_GC              1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free1_HeapType0_GC0
#define LOCAL_PARAM_Free            1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free1_HeapType0_GC1
#define LOCAL_PARAM_Free            1
#define LOCAL_PARAM_GC              1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free1_HeapType1_GC0
#define LOCAL_PARAM_Free            1
#define LOCAL_PARAM_HeapType        1
#elif defined(DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultCopy DeeObject_DefaultCopy_Free1_HeapType1_GC1
#define LOCAL_PARAM_Free            1
#define LOCAL_PARAM_HeapType        1
#define LOCAL_PARAM_GC              1
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifndef LOCAL_PARAM_Free
#define LOCAL_PARAM_Free 0
#endif /* !LOCAL_PARAM_Free */
#ifndef LOCAL_PARAM_HeapType
#define LOCAL_PARAM_HeapType 0
#endif /* !LOCAL_PARAM_HeapType */
#ifndef LOCAL_PARAM_GC
#define LOCAL_PARAM_GC 0
#endif /* !LOCAL_PARAM_GC */

DECL_BEGIN

#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeObject_DefaultCopy(DeeTypeObject *tp_self, DeeObject *self) {
	int error;
	DREF DeeObject *result;

	/* Allocate storage for object copy */
#if LOCAL_PARAM_Free
	result = (DREF DeeObject *)((*tp_self->tp_init.tp_alloc.tp_alloc)());
#elif LOCAL_PARAM_GC
	result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
#else /* ... */
	result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
#endif /* !... */
	if unlikely(!result)
		goto err;

	/* Initialize common object header of copy */
#if LOCAL_PARAM_HeapType
	DeeObject_InitHeap((GenericObject *)result, tp_self);
#else /* LOCAL_PARAM_HeapType */
	DeeObject_InitStatic((GenericObject *)result, tp_self);
#endif /* LOCAL_PARAM_HeapType */

	/* Invoke user-defined copy operator */
	error = (*tp_self->tp_init.tp_alloc.tp_copy_ctor)(result, self);
	if unlikely(error)
		goto err_r;

	/* Begin tracking the newly created object. */
#if LOCAL_PARAM_GC
	result = DeeGC_Track(result);
#endif /* LOCAL_PARAM_GC */

	/* Return copy */
	return result;
err_r:

	/* Free optional debug data (CONFIG_TRACE_REFCHANGES) */
	DeeObject_FreeTracker(result);

	/* Free backing storage of would-have-been result */
	ASSERT(Dee_TYPE(result) == tp_self);
#if LOCAL_PARAM_Free
	((*tp_self->tp_init.tp_alloc.tp_free)(result));
#elif LOCAL_PARAM_GC
	DeeGCObject_Free(result);
#else /* ... */
	DeeObject_Free(result);
#endif /* !... */

	/* Drop reference to "tp_self" (if one was held) */
#if LOCAL_PARAM_HeapType
	Dee_Decref_unlikely(tp_self);
#endif /* LOCAL_PARAM_HeapType */
err:
	return NULL;
}

DECL_END

#undef LOCAL_PARAM_Free
#undef LOCAL_PARAM_HeapType
#undef LOCAL_PARAM_GC
#undef LOCAL_DeeObject_DefaultCopy


#undef DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultCopy_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultCopy_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultCopy_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultCopy_Free1_HeapType1_GC1
