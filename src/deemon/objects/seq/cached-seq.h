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
#ifndef GUARD_DEEMON_OBJECTS_CACHED_SEQ_H
#define GUARD_DEEMON_OBJECTS_CACHED_SEQ_H 1

#include <deemon/api.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>
#include <deemon/util/objectlist.h>
/**/

#include "../generic-proxy.h"

DECL_BEGIN

/************************************************************************/
/* ITERATOR-BASED CACHE                                                 */
/************************************************************************/
typedef struct {
	PROXY_OBJECT_HEAD(cswi_iter)     /* [1..1][const] The iterator whose results are being cached. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t cswi_lock;     /* The lock used to synchronize the cache below. */
#endif /* !CONFIG_NO_THREADS */
	struct objectlist cswi_cache;    /* Cache of results returned by `cswi_iter' */
#ifndef DEE_OBJECTLIST_HAVE_ELEMA
#define CachedSeq_WithIter_HAVE_cswi_finished
	bool              cswi_finished; /* Set to true once `cswi_iter' has been exhausted */
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */
} CachedSeq_WithIter;

/* Uses an auto-growing vector for elements, that is fed by an iterator. */
INTDEF DeeTypeObject CachedSeq_WithIter_Type;

#ifdef CachedSeq_WithIter_HAVE_cswi_finished
#define CachedSeq_WithIter_InitFinished(self) (void)((self)->cswi_finished = false)
#define CachedSeq_WithIter_GetFinished(self)  ((self)->cswi_finished)
#define CachedSeq_WithIter_SetFinished(self)  (void)((self)->cswi_finished = true)
#else /* CachedSeq_WithIter_HAVE_cswi_finished */
#define CachedSeq_WithIter_InitFinished(self) (void)0
#define CachedSeq_WithIter_GetFinished(self)  ((self)->cswi_cache.ol_elema == (size_t)-1)
#define CachedSeq_WithIter_SetFinished(self)  (void)((self)->cswi_cache.ol_elema = (size_t)-1)
#endif /* !CachedSeq_WithIter_HAVE_cswi_finished */

#define CachedSeq_WithIter_LockAvailable(self)  Dee_atomic_lock_available(&(self)->cswi_lock)
#define CachedSeq_WithIter_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->cswi_lock)
#define CachedSeq_WithIter_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->cswi_lock)
#define CachedSeq_WithIter_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->cswi_lock)
#define CachedSeq_WithIter_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->cswi_lock)
#define CachedSeq_WithIter_LockRelease(self)    Dee_atomic_lock_release(&(self)->cswi_lock)



typedef struct {
	PROXY_OBJECT_HEAD_EX(CachedSeq_WithIter, cswii_cache) /* [1..1][const] The cache that is being iterated. */
	size_t                                   cswii_index; /* [lock(ATOMIC)] index of next element to yield. */
} CachedSeq_WithIter_Iterator;

INTDEF DeeTypeObject CachedSeq_WithIter_Iterator_Type;

LOCAL WUNUSED NONNULL((1)) DREF CachedSeq_WithIter *DCALL
CachedSeq_WithIter_New(/*inherit(always)*/ DREF DeeObject *iter) {
	DREF CachedSeq_WithIter *result;
	result = DeeGCObject_MALLOC(CachedSeq_WithIter);
	if unlikely(!result)
		goto err;
	result->cswi_iter = iter; /* Inherit reference */
	Dee_atomic_lock_init(&result->cswi_lock);
	objectlist_init(&result->cswi_cache);
	CachedSeq_WithIter_InitFinished(result);
	DeeObject_Init(result, &CachedSeq_WithIter_Type);
	return (DREF CachedSeq_WithIter *)DeeGC_Track((DREF DeeObject *)result);
err:
	Dee_Decref_likely(iter);
	return NULL;
}







/************************************************************************/
/* INDEX-BASED CACHE                                                    */
/************************************************************************/
typedef struct {
	PROXY_OBJECT_HEAD(cswgi_seq) /* [1..1][const] The sequence being cached. */
	/* TODO */
} CachedSeq_WithGetItem;

INTDEF DeeTypeObject CachedSeq_WithGetItem_Type;             /* Uses a lazily-allocated vector for small integers, and a dict for large ones, or non-integer keys */
INTDEF DeeTypeObject CachedSeq_WithSizeObAndGetItem_Type;    /* Like `CachedSeq_WithGetItem_Type', but also uses+caches `DeeSeq_OperatorSizeOb' */
INTDEF DeeTypeObject CachedSeq_WithSizeAndGetItemIndex_Type; /* Like `CachedSeq_WithSizeObAndGetItem_Type', but uses+caches `DeeSeq_OperatorSize'. */

INTDEF DeeTypeObject CachedSeq_WithGetItem_Iterator_Type;
INTDEF DeeTypeObject CachedSeq_WithSizeObAndGetItem_Iterator_Type;
INTDEF DeeTypeObject CachedSeq_WithSizeAndGetItemIndex_Iterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_SEQ_H */
