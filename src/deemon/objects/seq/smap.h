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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SMAP_H
#define GUARD_DEEMON_OBJECTS_SEQ_SMAP_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>     /* _Dee_MallococBufsize */
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include "../generic-proxy.h"

#include <stddef.h> /* offsetof, size_t */

DECL_BEGIN

#undef si_key
#undef si_value
#undef si_hash

typedef struct {
	DeeObject *si_key;   /* [0..1][lock(WRITE_ONCE)] The key of this shared item. */
	DeeObject *si_value; /* [?..1][valid_if(si_key)]
	                      * [lock(WRITE_ONCE)] The value of this shared item. */
	Dee_hash_t si_hash;  /* Hash of this key. */
} SharedItemEx;

typedef struct {
	OBJECT_HEAD
	size_t               sm_length; /* [lock(sm_lock)] The number of items in this vector. */
	DeeSharedItem const *sm_vector; /* [1..1][const][0..sm_length][lock(sm_lock)][owned]
	                                 * The vector of objects that is being referenced.
	                                 * NOTE: Elements of this vector must not be changed. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t  sm_lock;   /* Lock for this shared-vector. */
#endif /* !CONFIG_NO_THREADS */
	size_t               sm_loaded; /* [lock(sm_lock)] Set to non-zero once `sm_vector' has been fully loaded. */
	size_t               sm_mask;   /* [const][> sm_length][!0] Hash-vector mask for `skv_map'. */
	COMPILER_FLEXIBLE_ARRAY(SharedItemEx, sm_map); /* [lock(WRITE_ONCE)][sm_mask + 1] Hash-vector of cached keys.
	                                                * This hash-vector is populated lazily as objects are queried by key. */
} SharedMap;

#define SharedMap_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->sm_lock)
#define SharedMap_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->sm_lock)
#define SharedMap_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->sm_lock)
#define SharedMap_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->sm_lock)
#define SharedMap_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->sm_lock)
#define SharedMap_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->sm_lock)
#define SharedMap_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->sm_lock)
#define SharedMap_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->sm_lock)
#define SharedMap_LockRead(self)       Dee_atomic_rwlock_read(&(self)->sm_lock)
#define SharedMap_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->sm_lock)
#define SharedMap_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->sm_lock)
#define SharedMap_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->sm_lock)
#define SharedMap_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->sm_lock)
#define SharedMap_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->sm_lock)
#define SharedMap_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->sm_lock)
#define SharedMap_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->sm_lock)

#define SHAREDMAP_SIZEOF(mask) _Dee_MallococBufsize(offsetof(SharedMap, sm_map), (mask) + 1, sizeof(SharedItemEx))



/* Hash-iteration control. */
#define SMAP_HASHST(self, hash)  ((hash) & ((SharedMap *)(self))->sm_mask)
#define SMAP_HASHNX(hs, perturb) (hs = (((hs) << 2) + (hs) + (perturb) + 1), (perturb) >>= 5) /* This `5' is tunable. */
#define SMAP_HASHIT(self, i)     (((SharedMap *)(self))->sm_map + ((i) & ((SharedMap *)(self))->sm_mask))


typedef struct {
	PROXY_OBJECT_HEAD_EX(SharedMap, smi_seq);  /* [1..1][const] The shared-vector that is being iterated. */
	size_t                          smi_index; /* [atomic] The current sequence index.
	                                            * Should this value be `>= si_seq->sm_length',
	                                            * then the iterator has been exhausted. */
} SharedMapIterator;

INTDEF DeeTypeObject SharedMapIterator_Type;



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SMAP_H */
