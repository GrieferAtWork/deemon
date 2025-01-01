/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CACHED_DICT_H
#define GUARD_DEEMON_CACHED_DICT_H 1

#include "api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "object.h"
#include "util/lock.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_cached_dict_item   cached_dict_item
#define Dee_cached_dict_object cached_dict_object
#endif /* DEE_SOURCE */

typedef struct Dee_cached_dict_object DeeCachedDictObject;

struct Dee_cached_dict_item {
	DREF DeeObject *cdi_key;   /* [0..1][lock(:cd_lock)] Dictionary item key. */
	DREF DeeObject *cdi_value; /* [1..1][valid_if(cdi_key)][lock(:cd_lock)] Dictionary item value. */
	Dee_hash_t      cdi_hash;  /* [valid_if(cdi_key)][lock(:cd_lock)] Hash of `cdi_key' */
};

struct Dee_cached_dict_object {
	Dee_OBJECT_HEAD
	size_t                       cd_mask; /* [lock(cd_lock)][> cd_size || cd_mask == 0] Allocated dictionary size. */
	size_t                       cd_size; /* [lock(cd_lock)][< cd_mask || cd_mask == 0] Amount of non-NULL key-item pairs. */
	DREF DeeObject              *cd_map;  /* [1..1][const] Underlying mapping used to populate `cd_elem', as they are accessed. */
	struct Dee_cached_dict_item *cd_elem; /* [1..cd_size|ALLOC(cd_mask+1)][lock(cd_lock)]
	                                       * [owned_if(!= INTERNAL(DeeCachedDict_EmptyItems))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t          cd_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
};

/* Locking helpers for `DeeDictObject' */
#define DeeCachedDict_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->cd_lock)
#define DeeCachedDict_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->cd_lock)
#define DeeCachedDict_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->cd_lock)
#define DeeCachedDict_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->cd_lock)
#define DeeCachedDict_LockRead(self)       Dee_atomic_rwlock_read(&(self)->cd_lock)
#define DeeCachedDict_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->cd_lock)
#define DeeCachedDict_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->cd_lock)
#define DeeCachedDict_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->cd_lock)
#define DeeCachedDict_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->cd_lock)
#define DeeCachedDict_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->cd_lock)
#define DeeCachedDict_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->cd_lock)
#define DeeCachedDict_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->cd_lock)

DDATDEF DeeTypeObject DeeCachedDict_Type;
#define DeeCachedDict_Check(ob)      DeeObject_InstanceOf(ob, &DeeCachedDict_Type)
#define DeeCachedDict_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeCachedDict_Type)

#define DeeCachedDict_MAP(ob) ((DeeCachedDictObject const *)Dee_REQUIRES_OBJECT(ob))->cd_map

#define DeeCachedDict_EmptyItems ((struct Dee_cached_dict_item const *)DeeDict_EmptyItems)

/* Construct a cached dict wrapper around "mapping" */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCachedDict_New(DeeObject *__restrict mapping);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCachedDict_NewInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict mapping);

/* Hash iteration helpers. */
#define DeeCachedDict_HashSt(self, hash)  ((hash) & (self)->cd_mask)
#define DeeCachedDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeCachedDict_HashIt(self, i)     ((self)->cd_elem + ((i) & (self)->cd_mask))

DECL_END

#endif /* !GUARD_DEEMON_CACHED_DICT_H */
