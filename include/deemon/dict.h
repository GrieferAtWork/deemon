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
#ifndef GUARD_DEEMON_DICT_H
#define GUARD_DEEMON_DICT_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

DECL_BEGIN

typedef struct dict_object DeeDictObject;

struct dict_item {
    DREF DeeObject *di_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
    DREF DeeObject *di_value; /* [1..1|if(di_key == dummy,0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
    dhash_t         di_hash;  /* [valid_if(di_key)][lock(:d_lock)] Hash of `di_key' (with a starting value of `0').
                               *   NOTE: Some random value when `di_key' is the dummy key. */
};

struct dict_object {
    OBJECT_HEAD /* GC Object */
    size_t            d_mask; /* [lock(d_lock)][> d_size || d_mask == 0] Allocated dictionary size. */
    size_t            d_size; /* [lock(d_lock)][< d_mask || d_mask == 0] Amount of non-NULL key-item pairs. */
    size_t            d_used; /* [lock(d_lock)][<= d_size] Amount of key-item pairs actually in use.
                               *  HINT: The difference to `d_size' is the number of dummy keys currently in use. */
    struct dict_item *d_elem; /* [1..d_size|ALLOC(d_mask+1)][lock(d_lock)]
                               * [owned_if(!= INTERNAL(empty_dict_items))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
    rwlock_t          d_lock; /* Lock used for accessing this dict. */
#endif /* !CONFIG_NO_THREADS */
    WEAKREF_SUPPORT
};

/* The main `dict' container class (and all related types):
 * >> class dict: mapping { ... };
 * >> class dict.proxy: sequence { ... };
 * >> class dict.keys: dict.proxy { ... };
 * >> class dict.values: dict.proxy { ... };
 * >> class dict.items: dict.proxy { ... };
 * >> class dict.iterator: iterator { ... };
 * >> class dict.proxy.iterator: dict.iterator { ... };
 * >> class dict.keys.iterator: dict.proxy.iterator { ... };
 * >> class dict.values.iterator: dict.proxy.iterator { ... };
 * >> class dict.items.iterator: dict.proxy.iterator { ... };
 */
DDATDEF DeeTypeObject DeeDict_Type;
#define DeeDict_Check(ob)         DeeObject_InstanceOf(ob,&DeeDict_Type)
#define DeeDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob,&DeeDict_Type)

/* Dict proxy types (implemented in `/src/deemon/objects/dictproxy.c') */
DDATDEF DeeTypeObject DeeDictProxy_Type;
DDATDEF DeeTypeObject DeeDictKeys_Type;
DDATDEF DeeTypeObject DeeDictItems_Type;
DDATDEF DeeTypeObject DeeDictValues_Type;


#define DeeDict_New()  DeeObject_NewDefault(&DeeDict_Type)
DFUNDEF DREF DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeDict_FromIterator(DeeObject *__restrict self);


DFUNDEF DREF DeeObject *DCALL DeeDict_GetItemString(DeeObject *__restrict self, char const *__restrict key, dhash_t hash);
DFUNDEF DREF DeeObject *DCALL DeeDict_GetItemDef(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict def);
DFUNDEF DREF DeeObject *DCALL DeeDict_GetItemStringDef(DeeObject *__restrict self, char const *__restrict key, dhash_t hash, DeeObject *__restrict def);
DFUNDEF int DCALL DeeDict_DelItemString(DeeObject *__restrict self, char const *__restrict key, dhash_t hash);
DFUNDEF int DCALL DeeDict_SetItemString(DeeObject *__restrict self, char const *__restrict key, dhash_t hash, DeeObject *__restrict value);
DFUNDEF bool DCALL DeeDict_HasItemString(DeeObject *__restrict self, char const *__restrict key, dhash_t hash);

/* Create a new dict by inheriting a set of passed key-item pairs.
 * @param: key_items:    A vector containing `num_keyitems*2' elements,
 *                       even ones being keys and odd ones being items.
 * @param: num_keyitems: The number of key-item pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF DREF DeeObject *DCALL DeeDict_NewKeyItemsInherited(size_t num_keyitems, DREF DeeObject **__restrict key_items);

/* The basic dictionary item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     dhash_t i,perturb;
 * >>     dhash_t hash = DeeObject_Hash(0,key);
 * >>     perturb = i = DICT_HASHST(self,hash);
 * >>     for (;; i = DICT_HASHNX(i,perturb),DICT_HASHPT(perturb)) {
 * >>          struct dict_item *item = DICT_HASHIT(self,i);
 * >>          if (!item->di_key) break; // Not found
 * >>          if (item->di_hash != hash) continue; // Non-matching hash
 * >>          if (DeeObject_CompareEq(key,item->di_key))
 * >>              return item->di_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 *   - `DICT_HASHNX()' must be able to eventually enumerate all integers `<= d_mask'
 *   - `d_size' must always be lower than `d_mask', ensuring that at least one(1)
 *      entry exists that no value has been assigned to (Acting as a sentinel to
 *      terminate a search for an existing element).
 *   - `d_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but noone can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DICT_HASHST(self,hash)  ((hash) & ((DeeDictObject *)REQUIRES_OBJECT(self))->d_mask)
#define DICT_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define DICT_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define DICT_HASHIT(self,i)     (((DeeDictObject *)REQUIRES_OBJECT(self))->d_elem+((i) & ((DeeDictObject *)REQUIRES_OBJECT(self))->d_mask))



#ifndef CONFIG_NO_THREADS
#define DeeDict_LockReading(x)    rwlock_reading(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockWriting(x)    rwlock_writing(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTryread(x)    rwlock_tryread(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTrywrite(x)   rwlock_trywrite(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockRead(x)       rwlock_read(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockWrite(x)      rwlock_write(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTryUpgrade(x) rwlock_tryupgrade(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockUpgrade(x)    rwlock_upgrade(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockDowngrade(x)  rwlock_downgrade(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEndWrite(x)   rwlock_endwrite(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEndRead(x)    rwlock_endread(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEnd(x)        rwlock_end(&((DeeDictObject *)REQUIRES_OBJECT(x))->d_lock)
#else
#define DeeDict_LockReading(x)          1
#define DeeDict_LockWriting(x)          1
#define DeeDict_LockTryread(x)          1
#define DeeDict_LockTrywrite(x)         1
#define DeeDict_LockRead(x)       (void)0
#define DeeDict_LockWrite(x)      (void)0
#define DeeDict_LockTryUpgrade(x)       1
#define DeeDict_LockUpgrade(x)          1
#define DeeDict_LockDowngrade(x)  (void)0
#define DeeDict_LockEndWrite(x)   (void)0
#define DeeDict_LockEndRead(x)    (void)0
#define DeeDict_LockEnd(x)        (void)0
#endif

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
