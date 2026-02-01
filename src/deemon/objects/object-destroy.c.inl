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
#include "object.c"
#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1
//#define DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1) +      \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0) + \
     defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1) + \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1) +                 \
     defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1)) != 1
#error "Must #define exactly 1 of these macros"
#endif /* ... */

#include <deemon/api.h>

#include <deemon/alloc.h>       /* DeeObject_Free */
#include <deemon/gc.h>          /* DeeGCObject_Free, DeeGC_Track, DeeGC_Untrack */
#include <deemon/object.h>      /* DeeObject, DeeObject_Destroy, DeeTypeObject, Dee_Decref*, Dee_Incref, Dee_TYPE, Dee_refcnt_t, OBJECT_HEAD */
#include <deemon/type.h>        /* DeeType_IsFinal, DeeType_IsGC */
#include <deemon/util/atomic.h> /* atomic_fetchdec */

#include <stddef.h> /* NULL */

DECL_BEGIN


/* Configure feature options:
 * LOCAL_HAS_Dtor == 0: No type-specific destructors
 * LOCAL_HAS_Dtor == 1: Exactly 1 type-specific destructor, which is
 *                      stored in "Dee_TYPE(self)->tp_init.tp_dtor"
 * LOCAL_HAS_Dtor == 2: Destructors exist in base-classes
 * LOCAL_HAS_Free:      "Dee_TYPE(self)" defines a custom "tp_free"
 * LOCAL_HAS_HeapType:  "Dee_TYPE(self)" is a heap-type (TP_FHEAP)
 * LOCAL_HAS_GC:        "Dee_TYPE(self)" is a GC-type (TP_FGC)
 * LOCAL_HAS_Rev:       type-specific destructors may revive the object (TP_FMAYREVIVE) */
#ifdef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#define LOCAL_HAS_Dtor                 0
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 0
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             0
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1
#define LOCAL_HAS_Dtor                 1
#define LOCAL_HAS_Free                 1
#define LOCAL_HAS_HeapType             1
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  0
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_GC                   0
#define LOCAL_HAS_Rev                  1
#elif defined(DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1)
#define LOCAL_DeeObject_DefaultDestroy DeeObject_DefaultDestroy_DtorN_GC1_Rev1
#define LOCAL_HAS_Dtor                 2
#define LOCAL_HAS_GC                   1
#define LOCAL_HAS_Rev                  1
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifndef LOCAL_HAS_Dtor
#define LOCAL_HAS_Dtor 2
#endif /* !LOCAL_HAS_Dtor */
#ifndef LOCAL_HAS_HeapType
#define LOCAL_HAS_HeapType 1
#endif /* !LOCAL_HAS_HeapType */
#ifndef LOCAL_HAS_Rev
#define LOCAL_HAS_Rev 1
#endif /* !LOCAL_HAS_Rev */
#ifndef LOCAL_HAS_GC
#error "LOCAL_HAS_GC must be specified statically"
#endif /* !LOCAL_HAS_GC */

#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */

PRIVATE NONNULL((1)) void DCALL
LOCAL_DeeObject_DefaultDestroy(DeeObject *__restrict self) {
	DeeTypeObject *orig_type;

	/* Start by untracking the object in question. */
#if LOCAL_HAS_GC
	self = DeeGC_Untrack(self);
#endif /* LOCAL_HAS_GC */

	/* Load the object's type. */
	orig_type = Dee_TYPE((GenericObject *)self);

#if LOCAL_HAS_Dtor > 1
	{
		DeeTypeObject *type = orig_type;
		do {
			ASSERT(self->ob_refcnt == 0);
			ASSERTF(type == orig_type || !DeeType_IsFinal(type),
			        "Final type `%k' with sub-class `%k'",
			        type, orig_type);
			if (type->tp_init.tp_dtor) {
				/* Update the object's typing to mirror what is written here.
				 * NOTE: We're allowed to modify the type of `self' _ONLY_
				 *       because it's reference counter is ZERO (and because
				 *       implementors of `tp_free' are aware of its volatile
				 *       nature that may only be interpreted as a free-hint).
				 * NOTE: This even applies to the slab allocators used by `DeeObject_MALLOC'! */
				((GenericObject *)self)->ob_type = type;
				COMPILER_WRITE_BARRIER();
				(*type->tp_init.tp_dtor)(self);
				COMPILER_READ_BARRIER();
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
				Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */

				/* Special case: The destructor managed to revive the object. */
#if LOCAL_HAS_Rev
				if unlikely(self->ob_refcnt != 0) {
					/* Resume tracking of the object. */
#if LOCAL_HAS_GC
					ASSERTF(DeeType_IsGC(type),
					        "This runtime does not implementing reviving "
					        "GC-allocated objects as non-GC objects.");
					self = DeeGC_Track(self);
#endif /* LOCAL_HAS_GC */

					/* Incref() the new type that now describes this revived object.
					 * NOTE: The fact that this type may use a different (or none at all)
					 *       tp_free function, is the reason why no GC-able type from who's
					 *       destruction a user-callback that can somehow get a hold of the
					 *       instance being destroyed (which is also possible for any weakly
					 *       referenceable type), is allowed to assume that it will actually
					 *       be called, limiting its use to pre-allocated object caches that
					 *       allocate their instances using `DeeObject_Malloc'. */
					Dee_Incref(type);

					/* As part of the revival process, `tp_dtor' has us inherit a reference to `self'
					 * in order to prevent a race condition that could otherwise occur when another
					 * thread would have cleared the external reference after the destructor created
					 * it, but before we were able to read out the fact that `ob_refcnt' was now
					 * non-zero. - If that were to happen, the other thread may also attempt to destroy
					 * the object, causing it to be destroyed in multiple threads at the same time,
					 * which is something that's not allowed! */
					Dee_Decref(orig_type);

					/* Special case: in order for `tp_dtor' to revive the object, it has to gift us
					 * a secondary reference. This needs to be done to prevent a race condition in
					 * GC-enabled objects, where another thread might (once again) destroy the object
					 * before we were able to call `DeeGC_Track()' above.
					 *
					 * So to prevent other threads from destroying the object until then, there is
					 * this second, magic reference that only gets dropped here. */
#ifndef CONFIG_TRACE_REFCHANGES
					{
						Dee_refcnt_t oldref;
						oldref = atomic_fetchdec(&self->ob_refcnt);
						ASSERTF(oldref != 0,
						        "Upon revival, a destructor must let the caller inherit a "
						        "reference (which may appear like a leak, but actually isn't)");
						if unlikely(oldref == 1)
							DeeObject_Destroy(self);
					}
#else /* !CONFIG_TRACE_REFCHANGES */
					Dee_Decref_unlikely(self);
#endif /* CONFIG_TRACE_REFCHANGES */
					return;
				}
#endif /* LOCAL_HAS_Rev */
			}
		} while ((type = type->tp_base) != NULL);
	}
#elif LOCAL_HAS_Dtor == 1

	/* Invoke a custom destructor callback. */
	(*orig_type->tp_init.tp_dtor)(self);

#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */

	/* Special case: The destructor managed to revive the object. */
#if LOCAL_HAS_Rev
	if unlikely(self->ob_refcnt != 0) {
		/* Resume tracking of the object. */
#if LOCAL_HAS_GC
		self = DeeGC_Track(self);
#endif /* LOCAL_HAS_GC */

		/* See explanation above for what this decref is all about. */
#ifndef CONFIG_TRACE_REFCHANGES
		{
			Dee_refcnt_t oldref;
			oldref = atomic_fetchdec(&self->ob_refcnt);
			ASSERTF(oldref != 0,
			        "Upon revival, a destructor must let the caller inherit a "
			        "reference (which may appear like a leak, but actually isn't)");
			if unlikely(oldref == 1)
				DeeObject_Destroy(self);
		}
#else /* !CONFIG_TRACE_REFCHANGES */
		Dee_Decref_unlikely(self);
#endif /* CONFIG_TRACE_REFCHANGES */
		return;
	}
#endif /* LOCAL_HAS_Rev */
#endif /* LOCAL_HAS_Dtor == ... */

	/* Free the tracker for changes to reference counts. */
#ifdef CONFIG_TRACE_REFCHANGES
	free_reftracker(self->ob_trace);
#endif /* CONFIG_TRACE_REFCHANGES */

#if LOCAL_HAS_HeapType
#define LOCAL_decref_orig_type() Dee_Decref_unlikely(orig_type)
#else /* LOCAL_HAS_HeapType */
	/* Non-heap types can never be destroyed, so we can use the *Nokill version here! */
#define LOCAL_decref_orig_type() Dee_DecrefNokill(orig_type)
#endif /* !LOCAL_HAS_HeapType */

	/* Invoke `tp_free' of `orig_type', and decref `orig_type' */
#ifndef LOCAL_HAS_Free
	if (orig_type->tp_init.tp_alloc.tp_free) {
#if LOCAL_HAS_HeapType
		(*orig_type->tp_init.tp_alloc.tp_free)(self);
		LOCAL_decref_orig_type();
#else /* LOCAL_HAS_HeapType */
		LOCAL_decref_orig_type();
		(*orig_type->tp_init.tp_alloc.tp_free)(self);
#endif /* !LOCAL_HAS_HeapType */
	} else {
		LOCAL_decref_orig_type();
#if LOCAL_HAS_GC
		DeeGCObject_Free(self);
#else /* LOCAL_HAS_GC */
		DeeObject_Free(self);
#endif /* !LOCAL_HAS_GC */
	}
#elif LOCAL_HAS_Free && LOCAL_HAS_HeapType
	(*orig_type->tp_init.tp_alloc.tp_free)(self);
	/* It's a heap-type, so "tp_free" may become invalid if this ends up
	 * destroying the type (as such: this has to happen *after* "tp_free") */
	LOCAL_decref_orig_type();
#elif LOCAL_HAS_Free
	/* Static type, so allowed to decref it before calling "tp_free"
	 * (doing it this way allows compiler to optimize tail recursion) */
	LOCAL_decref_orig_type();
	(*orig_type->tp_init.tp_alloc.tp_free)(self);
#elif LOCAL_HAS_GC
	LOCAL_decref_orig_type();
	DeeGCObject_Free(self);
#else /* ... */
	LOCAL_decref_orig_type();
	DeeObject_Free(self);
#endif /* !... */
#undef LOCAL_decref_orig_type
}

#undef LOCAL_DeeObject_DefaultDestroy
#undef LOCAL_HAS_Dtor
#undef LOCAL_HAS_Free
#undef LOCAL_HAS_HeapType
#undef LOCAL_HAS_GC
#undef LOCAL_HAS_Rev

DECL_END

#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType0_GC1_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free0_HeapType1_GC1_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType0_GC1_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_Dtor1_Free1_HeapType1_GC1_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev0
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC0_Rev1
#undef DEFINE_DeeObject_DefaultDestroy_DtorN_GC1_Rev1
