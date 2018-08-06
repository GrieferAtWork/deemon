/* Copyright (c) 2018 Griefer@Work                                            *
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
    DREF DeeObject  *sk_key;    /* [1..1][const] The key of this shared-key. */
    DREF DeeObject  *sk_value;  /* [1..1][const] The item of this shared-key. */
} SharedKey;

typedef struct {
    DeeObject       *sk_key;    /* [1..1][const] The key of this shared-key. */
    DeeObject       *sk_value;  /* [1..1][const] The item of this shared-key. */
    dhash_t          sk_hash;   /* Hash of this key. */
} SharedKeyEx;

typedef struct {
    OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
    rwlock_t         sm_lock;   /* Lock for this shared-vector. */
#endif
    size_t           sm_length; /* [lock(skv_lock)] The number of items in this vector. */
    SharedKey       *sm_vector; /* [1..1][const][0..sv_length][lock(skv_lock)][owned]
                                 * The vector of objects that is being referenced.
                                 * NOTE: Elements of this vector must not be changed. */
    size_t           sm_mask;   /* [const][> sm_length][!0] Hash-vector mask for `skv_map'. */
    SharedKeyEx      sm_map[1]; /* [lock(WRITE_ONCE)] Hash-vector of cached keys.
                                 * This hash-vector is populated lazily as objects are queried by key. */
} SharedMap;

#define SHAREDMAP_SIZEOF(mask) (offsetof(SharedMap,sm_map)+((mask)+1)*sizeof(SharedKeyEx))
INTDEF DeeTypeObject SharedMap_Type;



/* Create a new shared vector that will inherit elements
 * from the given vector once `SharedMap_Decref()' is called.
 * NOTE: This function implicitly inherits a reference to each item
 *       of the given vector, though does not actually inherit the
 *       vector itself! */
INTDEF DREF SharedMap *DCALL
SharedMap_NewShared(size_t length, DREF SharedKey *__restrict vector);

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `skv_vector',
 * but still decref() all contained object.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sskv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_MAP' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
INTDEF void DCALL SharedMap_Decref(DREF SharedMap *__restrict self);


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
