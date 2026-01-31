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
/*!export **/
/*!export DeeDeepCopy_**/
/*!export Dee_deepcopy_heap_**/
#ifndef GUARD_DEEMON_DEEPCOPY_H
#define GUARD_DEEMON_DEEPCOPY_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "serial.h" /* Dee_SERIAL_HEAD */
#include "types.h"  /* DREF, DeeObject */

#include <stddef.h> /* size_t */

#ifndef __INTELLISENSE__
#include "alloc.h" /* Dee_*alloc*, Dee_Free */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* "DeeSerial" can also be used to implement "deepcopy" (in a way that
 * solves the problem of non-GC objects being copied whilst some nested
 * GC object holding another reference to the original object; in the
 * current deepcopy impl, this causes the original object to be copied
 * twice).
 *
 * Even beyond this, "DeeSerial" can also be used to implement "copy",
 * too. For that, simply do what "deepcopy" does, but rather than recurse
 * on each nested object, simply re-encode a reference to the source
 * object instead! (though in this regard, this can only be a fallback
 * "copy", since certain types of objects; specifically certain Iterator
 * types; still need to do certain nested copy operations to ensure the
 * copied iterator is detached from the original iterator).
 */


struct Dee_deepcopy_mapitem {
	__BYTE_TYPE__ const *dcmi_old_minaddr; /* [1..1] Old pointer min address */
	__BYTE_TYPE__ const *dcmi_old_maxaddr; /* [1..1] Old pointer max address */
	__BYTE_TYPE__       *dcmi_new;         /* [1..1] New pointer */
};

struct Dee_deepcopy_uheap {
	struct Dee_deepcopy_uheap       *ddcuh_next; /* [0..1][owned] Next user-heap chunk */
	DeeObject                       *ddcuh_base; /* [1..1][owned(ddcuh_free)] Heap item base address */
	NONNULL_T((1)) void      (DCALL *ddcuh_free)(void *__restrict ob); /* [1..1][const] Free function for `ddcuh_base' */
};
#ifdef __INTELLISENSE__
#define Dee_deepcopy_uheap_alloc()    ((struct Dee_deepcopy_uheap *)sizeof(struct Dee_deepcopy_uheap))
#define Dee_deepcopy_uheap_free(self) (void)Dee_REQUIRES_TYPE(struct Dee_deepcopy_uheap *, self)
#define Dee_deepcopy_uheap_destroy(self)        \
	((*(self)->ddcuh_free)((self)->ddcuh_base), \
	 Dee_deepcopy_uheap_free(self))
#else /* __INTELLISENSE__ */
#define Dee_deepcopy_uheap_alloc()    DeeObject_MALLOC(struct Dee_deepcopy_uheap)
#define Dee_deepcopy_uheap_free(self) DeeObject_FREE(Dee_REQUIRES_TYPE(struct Dee_deepcopy_uheap *, self))
#define Dee_deepcopy_uheap_destroy(self)               \
	(Dee_Decref_unlikely((self)->ddcuh_base->ob_type), \
	 (*(self)->ddcuh_free)((self)->ddcuh_base),        \
	 Dee_deepcopy_uheap_free(self))
#endif /* !__INTELLISENSE__ */


#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
typedef void Dee_deepcopy_heap_t;
#define Dee_deepcopy_heap_getbase(self)    (self)
#ifdef __INTELLISENSE__
#define Dee_deepcopy_heap_getnext(self)    (*(void *const *)((__BYTE_TYPE__ *)(self) + (size_t)(self) - sizeof(void *)))
#define Dee_deepcopy_heap_setnext(self, v) (*(void **)((__BYTE_TYPE__ *)(self) + (size_t)(self) - sizeof(void *)) = (v))
#define Dee_deepcopy_heap_destroy(self)    (void)(self)
#else /* __INTELLISENSE__ */
#define Dee_deepcopy_heap_getnext(self)    (*(void *const *)((__BYTE_TYPE__ *)(self) + Dee_MallocUsableSize(self) - sizeof(void *)))
#define Dee_deepcopy_heap_setnext(self, v) (*(void **)((__BYTE_TYPE__ *)(self) + Dee_MallocUsableSize(self) - sizeof(void *)) = (v))
#define Dee_deepcopy_heap_destroy(self)    Dee_Free(self)
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
typedef struct Dee_deepcopy_heap Dee_deepcopy_heap_t;
struct Dee_deepcopy_heap {
	Dee_deepcopy_heap_t *ddch_next; /* [0..1][owned] Next heap chunk */
	void                *ddch_base; /* [1..1][owned] Heap item base address */
};
#ifdef __INTELLISENSE__
#define Dee_deepcopy_heap_tryallocchunk() ((Dee_deepcopy_heap_t *)(sizeof(Dee_deepcopy_heap_t)))
#define Dee_deepcopy_heap_allocchunk()    ((Dee_deepcopy_heap_t *)(sizeof(Dee_deepcopy_heap_t)))
#define Dee_deepcopy_heap_freechunk(p)    (void)(p)
#else /* __INTELLISENSE__ */
#define Dee_deepcopy_heap_tryallocchunk() ((Dee_deepcopy_heap_t *)Dee_TryMalloc(sizeof(Dee_deepcopy_heap_t)))
#define Dee_deepcopy_heap_allocchunk()    ((Dee_deepcopy_heap_t *)Dee_Malloc(sizeof(Dee_deepcopy_heap_t)))
#define Dee_deepcopy_heap_freechunk(p)    Dee_Free(p)
#endif /* !__INTELLISENSE__ */

#define Dee_deepcopy_heap_getbase(self)    (self)->ddch_base
#define Dee_deepcopy_heap_getnext(self)    (self)->ddch_next
#define Dee_deepcopy_heap_setnext(self, v) (void)((self)->ddch_next = (v))
#ifdef __INTELLISENSE__
#define Dee_deepcopy_heap_destroy(self)    (void)(self)->ddch_base
#else /* __INTELLISENSE__ */
#define Dee_deepcopy_heap_destroy(self)    (Dee_Free((self)->ddch_base), Dee_Free(self))
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

struct Dee_gc_head;
struct Dee_weakref;

/*
 * Context controller for creating deep copies of objects by use of "tp_serialize":
 * >>    DeeObject *copy1;
 * >>    DeeObject *copy2;
 * >>    DeeDeepCopyContext ctx;
 * >>    DeeDeepCopy_Init(&ctx);
 * >>    copy1 = DeeDeepCopy_CopyObject(&ctx, ob1);
 * >>    if unlikely(!copy1)
 * >>        goto err;
 * >>    copy2 = DeeDeepCopy_CopyObject(&ctx, ob2);
 * >>    if unlikely(!copy2)
 * >>        goto err;
 * >>    DeeDeepCopy_Pack(&ctx);
 * >>    return DeeTuple_Newf("OO", copy1, copy2);
 * >>err:
 * >>    DeeDeepCopy_Fini(&ctx);
 * >>    return -1;
 *
 * In the above example, cross-references between "ob1" and "ob2" are also properly
 * copied, such that a reference to "ob2" from "ob1" will appear as a reference to
 * "copy2" in "copy1", with the same also applying in the reverse order, as well as
 * regarding self-, circular, or any other kind of reference between/within "ob1"
 * and "ob2". */
typedef struct {
	Dee_SERIAL_HEAD
	Dee_deepcopy_heap_t         *dcc_heap;       /* [0..N][owned] Deepcopy heap */
	Dee_deepcopy_heap_t         *dcc_obheap;     /* [0..N][owned] Deepcopy heap for objects */
	Dee_deepcopy_heap_t         *dcc_gcheap;     /* [0..N][owned] Deepcopy heap for gc objects */
	struct Dee_deepcopy_uheap   *dcc_uheap;      /* [0..N][owned] Deepcopy heap for objects with custom allocators */
	struct Dee_deepcopy_mapitem *dcc_ptrmapv;    /* [0..dcc_ptrmapc][owned][SORT(dcmi_old_minaddr ASC)] mapping of source range to target ranges. */
	size_t                       dcc_ptrmapc;    /* # of used elements in `dcc_ptrmapv' */
	size_t                       dcc_ptrmapa;    /* # of allocated elements in `dcc_ptrmapv' */
	struct Dee_gc_head          *dcc_gc_head;    /* [0..1][(!= NULL) == (dcc_gc_tail != NULL)] First GC object to track in `DeeDeepCopy_Pack()' */
	struct Dee_gc_head          *dcc_gc_tail;    /* [0..1][(!= NULL) == (dcc_gc_tail != NULL)] Last GC object to track in `DeeDeepCopy_Pack()' */
	union {
		DREF DeeObject         **dcc_immutablev; /* [1..1][0..dcc_immutablec][valid_if(dcc_immutablec != 1)][owned]
		                                          * Vector of references to immutable objects that were returned by
		                                          * `DeeDeepCopy_CopyObject()' or embedded within copied objects, and
		                                          * must be inherited by `DeeDeepCopy_Pack()', but decref'd by `DeeDeepCopy_Fini' */
		DREF DeeObject          *dcc_immutable1; /* [1..1][valid_if(dcc_immutablec == 1)] Singular immutable object */
	};
	size_t                       dcc_immutablec; /* # of elements in `dcc_immutablev' */
	struct Dee_weakref         **dcc_weakrefv;   /* [1..1][0..dcc_weakrefc][owned]
	                                              * List of duplicated weakrefs that must be re-initialized during context packing.
	                                              * During copying, these weakrefs are initialized to store references to the source
	                                              * objects, which are then replaced with their serialized equivalents as necessary. */
	size_t                       dcc_weakrefc;   /* # of elements in `dcc_weakrefv' */
} DeeDeepCopyContext;

/* Initialize/finalize a deepcopy context. */
DFUNDEF NONNULL((1)) void DCALL
DeeDeepCopy_Init(DeeDeepCopyContext *__restrict self);
DFUNDEF NONNULL((1)) void DCALL
DeeDeepCopy_Fini(DeeDeepCopyContext *__restrict self);

/* TODO: I really want to have a function:
 * >> DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
 * >> DeeDeepCopy_MapObject(DeeDeepCopyContext *__restrict self,
 * >>                       DeeObject *old_obj, DeeObject *new_obj);
 *
 * This could then be exposed to user-code to allow for custom copy
 * operations that replace certain objects with others in a larger
 * tree of objects referencing each other.
 *
 * However, currently this can't be done because `DeeSerial_PutObject'
 * needs a flag or sibling function to indicate when the typing of the
 * serialized object can/can't be changed (e.g. many iterator types are
 * (rightfully so) assuming the typing of their struct's 'DREF T *'
 * members, and the above API would allow user-code to break that
 * assumption) */



/* Serialize "ob" into "self" and return a pointer for where "ob" will
 * eventually be initialized once `DeeDeepCopy_Pack()' is called. But
 * until that has been done, this pointer must **NOT** be exposed to
 * user-code, and must similarly not be interacted with, either!
 *
 * When "ob" has already been copied by "self", remember that an extra
 * incref needs to happen and return that pre-existing copy (any distinct
 * object (as per `DeeObject_Id()') will always be copied at most once)
 *
 * @return: * :   Location where the deep-copy of "ob" will appear
 *                When "ob" is immutable, this function may just re-return "ob"
 *                after storing a special reference to "ob" that is dropped if
 *                `DeeDeepCopy_Fini' is called instead of `DeeDeepCopy_Pack'.
 * @return: NULL: Error */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*after(DeeDeepCopy_Pack)*/ DeeObject *DCALL
DeeDeepCopy_CopyObject(DeeDeepCopyContext *__restrict self,
                       DeeObject *__restrict ob);

/* Finalize "self" in the sense of doing a COMMIT.
 *
 * This function behaves similar to `DeeDeepCopy_Fini()', but will also
 * (atomically) initialize the return values of `DeeDeepCopy_CopyObject'
 * to resemble valid references to deemon objects (which must then be
 * inherited by the caller(s) of that function)
 *
 * IMPORTANT: Do **NOT** call `DeeDeepCopy_Fini(self)' after this function.
 *            If you want to re-use "self" as a different context, you must
 *            first re-initialize it using `DeeDeepCopy_Init(self)' */
DFUNDEF NONNULL((1)) void DCALL
DeeDeepCopy_Pack(/*inherit(always)*/DeeDeepCopyContext *__restrict self);


DECL_END

#endif /* !GUARD_DEEMON_DEEPCOPY_H */
