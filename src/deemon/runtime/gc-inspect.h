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
#ifndef GUARD_DEEMON_RUNTIME_GC_INSPECT_H
#define GUARD_DEEMON_RUNTIME_GC_INSPECT_H 1
#define DEE_SOURCE

#include <deemon/api.h>

#include <deemon/types.h>     /* DREF, DeeObject, DeeTypeObject, Dee_hash_t, OBJECT_HEAD */
#include <deemon/util/lock.h> /* Dee_atomic_lock_* */
#include <deemon/util/once.h> /* Dee_once_t */

#include "../objects/generic-proxy.h"

#include <stddef.h> /* size_t */

DECL_BEGIN

struct gcset_item {
	DREF DeeObject *gsi_item; /* [0..1] Referenced object (or "NULL" if unused slot) */
};

typedef struct {
	struct gcset_item  *gs_map; /* [0..gs_msk+1][owned] Map of referenced objects, or "NULL" if not-yet-allocated */
	Dee_hash_t          gs_msk; /* Hash-mask for `gs_map' */
	size_t              gs_siz; /* # of non-NULL items in `gs_map', or `(size_t)-1' after an error */
} GCSet;

#define GCSet_HashSt(self, hash)  ((hash) & (self)->gs_msk)
#define GCSet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define GCSet_HashIt(self, i)     ((self)->gs_map + ((i) & (self)->gs_msk))



typedef struct gc_collection GCCollection;
typedef void (DCALL *gcset_populate_cb_t)(GCCollection *self);

struct gc_collection {
	PROXY_OBJECT_HEAD  (gcc_obj);     /* [1..1][const] The object whose reachable set is being described. */
	gcset_populate_cb_t gcc_populate; /* [1..1][const] Cache population function */
	GCSet               gcc_cache;    /* [const_if(Dee_once_hasrun(&gcc_loaded))] Cache of reachable objects (loaded lazily if needed for sequence operations) */
	Dee_once_t          gcc_loaded;   /* Is-loaded controller for `gcc_cache' */
	/* NOTE: `gcc_populate' should be one of:
	 * - GCCollection_PopulateReachableDirect
	 * - GCCollection_PopulateReachableTransitive
	 * - GCCollection_PopulateReferringDirect
	 * - GCCollection_PopulateReferringTransitive */
};

INTDEF DeeTypeObject GCCollection_Type;



typedef struct {
	PROXY_OBJECT_HEAD_EX(GCCollection, gcci_coll); /* [1..1][const] Underlying collection (cache of this has already been loaded) */
	struct gcset_item                 *gcci_iter;  /* [1..1][const] Next-pointer (in `gcci_coll->gcc_cache.gs_map') */
	struct gcset_item                 *gcci_end;   /* [1..1][const][== gcci_coll->gcc_cache.gs_map+(.gs_msk+1)] End-pointer */
} GCCollectionIterator;

INTDEF DeeTypeObject GCCollectionIterator_Type;




/* Iterator for enumerating the set of active GC objects. */
typedef struct {
	OBJECT_HEAD
	DREF struct gc_generation *gci_gen; /* [0..1] Current GC generation, or "NULL" if enumeration has finished. */
	DREF DeeObject            *gci_obj; /* [1..1][valid_if(gci_gen)] Next object to enumerate (object exists within `gci_gen') */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t          gci_lck; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
} GCIter;

#define gc_iter_available(self)  Dee_atomic_lock_available(&(self)->gci_lck)
#define gc_iter_acquired(self)   Dee_atomic_lock_acquired(&(self)->gci_lck)
#define gc_iter_tryacquire(self) Dee_atomic_lock_tryacquire(&(self)->gci_lck)
#define gc_iter_acquire(self)    Dee_atomic_lock_acquire(&(self)->gci_lck)
#define gc_iter_waitfor(self)    Dee_atomic_lock_waitfor(&(self)->gci_lck)
#define gc_iter_release(self)    Dee_atomic_lock_release(&(self)->gci_lck)

INTDEF DeeTypeObject GCIter_Type;

INTDEF DeeTypeObject GCEnum_Type;

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_GC_INSPECT_H */
