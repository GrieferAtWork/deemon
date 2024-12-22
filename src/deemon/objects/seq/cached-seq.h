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
#include <deemon/dict.h>
#include <deemon/gc.h>
#include <deemon/int.h>
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
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t cswi_lock;  /* The lock used to synchronize the cache below. */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject   *cswi_iter;  /* [0..1][lock(cswi_lock)] The iterator whose results are being cached (or NULL once exhausted). */
	struct objectlist cswi_cache; /* [lock(cswi_lock)] Cache of results returned by `cswi_iter' */
} CachedSeq_WithIter;

/* Uses an auto-growing vector for elements, that is fed by an iterator. */
INTDEF DeeTypeObject CachedSeq_WithIter_Type;

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
	DeeObject_Init(result, &CachedSeq_WithIter_Type);
	return DeeGC_TRACK(CachedSeq_WithIter, result);
err:
	Dee_Decref_likely(iter);
	return NULL;
}







/************************************************************************/
/* INDEX-BASED CACHE                                                    */
/************************************************************************/

#undef HAVE_CachedSeq_WithGetItem
#if 0 /* TODO */
#define HAVE_CachedSeq_WithGetItem
#endif

#ifdef HAVE_CachedSeq_WithGetItem
/* Threshold used to determine if a `cswgi_vector' should be used (as opposed to `cswgi_btab') */
#define CACHEDSEQ_WITHGETITEM_SMALLINDEX_THRESHOLD 0xffff
#define CACHEDSEQ_WITHGETITEM_ISSMALLINDEX(index) \
	((index) <= CACHEDSEQ_WITHGETITEM_SMALLINDEX_THRESHOLD)

struct indexbtab_item {
	DREF DeeIntObject *ibti_index; /* [1..1] Integer index. */
	DREF DeeObject    *ibti_value; /* [0..1] Value of this index (NULL means `err_unbound_key') */
};

struct indexbtab {
	struct indexbtab_item *ibt_elem; /* [0..ibt_size][owned] Sorted (for bsearch) vector of index/value pairs. */
	size_t                 ibt_size; /* Size of `ibt_elem' */
};

#define indexbtab_init(self) \
	(void)((self)->ibt_size = 0, (self)->ibt_elem = NULL)

struct cachedseq_index {
	DREF DeeIntObject *csi_indexob; /* [0..1] Object-based index, or `NULL' if `csi_index' should be used. */
	size_t             csi_index;   /* [valid_if(csi_indexob == NULL)] Native index. */
};
#define cachedseq_index_init_index(self, index) \
	(void)((self)->csi_indexob = NULL, (self)->csi_index = (index))
#define cachedseq_index_fini(self) Dee_XDecref((self)->csi_indexob)
#define cachedseq_index_copy(dst, src)              \
	(void)((dst)->csi_indexob = (src)->csi_indexob, \
	       Dee_XIncref((dst)->csi_indexob),         \
	       (dst)->csi_index = (src)->csi_index)
#define cachedseq_index_move(dst, src)              \
	(void)((dst)->csi_indexob = (src)->csi_indexob, \
	       (dst)->csi_index   = (src)->csi_index)
INTDEF WUNUSED NONNULL((1)) int DCALL
cachedseq_index_inc(struct cachedseq_index *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
cachedseq_index_compare(struct cachedseq_index const *__restrict lhs,
                        struct cachedseq_index const *__restrict rhs);

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t      cswgi_lock;    /* The lock used to synchronize the cache below. */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject        *cswgi_seq;     /* [0..1][lock(cswgi_lock)] The sequence being cached (or NULL if fully cached). */
	struct cachedseq_index cswgi_size;    /* [lock(cswgi_lock)] Size of `cswgi_seq' as an object (or {NULL,(size_t)-1} if not yet calculated). */
	struct cachedseq_index cswgi_maxsize; /* [lock(cswgi_lock)] First index known to be out-of-bounds (or {NULL,(size_t)-1} if not yet calculated). */
	struct cachedseq_index cswgi_loaded;  /* [lock(cswgi_lock)] # of leading, consecutive sequence elements that have been loaded (can assume that absence in the cache means UNBOUND) */
	/* XXX: Remove "cswgi_vector"? */
	struct objectlist      cswgi_vector;  /* [lock(cswgi_lock)] Cache for "small" indices (may contain NULL-elements) (s.a. `CACHEDSEQ_WITHGETITEM_ISSMALLINDEX') */
	struct indexbtab       cswgi_btab;    /* [lock(cswgi_lock)] Binary-table for "large" indices */
} CachedSeq_WithGetItem;

#define CachedSeq_WithGetItem_LockAvailable(self)  Dee_atomic_lock_available(&(self)->cswgi_lock)
#define CachedSeq_WithGetItem_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->cswgi_lock)
#define CachedSeq_WithGetItem_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->cswgi_lock)
#define CachedSeq_WithGetItem_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->cswgi_lock)
#define CachedSeq_WithGetItem_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->cswgi_lock)
#define CachedSeq_WithGetItem_LockRelease(self)    Dee_atomic_lock_release(&(self)->cswgi_lock)

INTDEF DeeTypeObject CachedSeq_WithGetItem_Type;          /* Uses a lazily-allocated vector for small integers, and a mapping for large ones */
INTDEF DeeTypeObject CachedSeq_WithSizeObAndGetItem_Type; /* Like `CachedSeq_WithGetItem_Type', but also uses+caches `DeeSeq_OperatorSizeOb' */
INTDEF DeeTypeObject CachedSeq_WithSizeAndGetItem_Type;   /* Like `CachedSeq_WithSizeObAndGetItem_Type', but uses `DeeSeq_OperatorSize' instead of `DeeSeq_OperatorSizeOb' */


typedef struct {
	PROXY_OBJECT_HEAD_EX(CachedSeq_WithGetItem, cswgii_cache);    /* [1..1][const] Underlying cache */
	struct cachedseq_index                      cswgii_nextindex; /* [lock(cswgii_lock)] Lower bound for next index to enumerate */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t                           cswgii_lock;      /* The lock used to synchronize the cache below. */
#endif /* !CONFIG_NO_THREADS */
} CachedSeq_WithGetItem_Iterator;

INTDEF DeeTypeObject CachedSeq_WithGetItem_Iterator_Type;
#endif /* HAVE_CachedSeq_WithGetItem */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_SEQ_H */
