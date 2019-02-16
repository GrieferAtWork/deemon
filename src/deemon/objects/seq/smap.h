/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SMAP_H
#define GUARD_DEEMON_OBJECTS_SEQ_SMAP_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

DECL_BEGIN

typedef struct {
    DeeObject       *si_key;    /* [1..1][const] The key of this shared item. */
    DeeObject       *si_value;  /* [1..1][const] The value of this shared item. */
    dhash_t          si_hash;   /* Hash of this key. */
} SharedItemEx;

typedef struct {
    OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
    rwlock_t         sm_lock;   /* Lock for this shared-vector. */
#endif
    size_t           sm_length; /* [lock(skv_lock)] The number of items in this vector. */
    DeeSharedItem   *sm_vector; /* [1..1][const][0..sv_length][lock(skv_lock)][owned]
                                 * The vector of objects that is being referenced.
                                 * NOTE: Elements of this vector must not be changed. */
    size_t           sm_mask;   /* [const][> sm_length][!0] Hash-vector mask for `skv_map'. */
    SharedItemEx     sm_map[1]; /* [lock(WRITE_ONCE)] Hash-vector of cached keys.
                                 * This hash-vector is populated lazily as objects are queried by key. */
} SharedMap;

#define SHAREDMAP_SIZEOF(mask) (offsetof(SharedMap,sm_map)+((mask)+1)*sizeof(SharedItemEx))
INTDEF DeeTypeObject SharedMap_Type;



/* Hash-iteration control. */
#define SMAP_HASHST(self,hash)  ((hash) & ((SharedMap *)(self))->sm_mask)
#define SMAP_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define SMAP_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define SMAP_HASHIT(self,i)     (((SharedMap *)(self))->sm_map+((i) & ((SharedMap *)(self))->sm_mask))


typedef struct {
    OBJECT_HEAD
    DREF SharedMap *sm_seq;   /* [1..1][const] The shared-vector that is being iterated. */
    size_t          sm_index; /* [atomic] The current sequence index.
                               * Should this value be `>= si_seq->sv_length',
                               * then the iterator has been exhausted. */
} SharedMapIterator;

INTDEF DeeTypeObject SharedMapIterator_Type;



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SMAP_H */
