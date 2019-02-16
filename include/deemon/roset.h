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
#ifndef GUARD_DEEMON_ROSET_H
#define GUARD_DEEMON_ROSET_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */
#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_roset_item   roset_item
#define Dee_roset_object roset_object
#define ROSET_HASHST     Dee_ROSET_HASHST
#define ROSET_HASHNX     Dee_ROSET_HASHNX
#define ROSET_HASHPT     Dee_ROSET_HASHPT
#define ROSET_HASHIT     Dee_ROSET_HASHIT
#endif /* DEE_SOURCE */

/* A read-only variant of a set object, who's main purpose is to be used
 * by compiler optimizations in order to optimize generic set expressions
 * in various locations, when sequence arguments are constant.
 * The main usage case is to optimize code such as:
 * >> if (get_item() in [10,20,17,19,11,3]) {
 * >>     print "Well... It's one of 'em!";
 * >> }
 * ASM:
 * >>     push  @_roset { 10, 20, 17, 19, 11, 3 }
 * >>     push  call global @get_item, #0
 * >>     contains top, pop
 * >>     jf    1f
 * >>     print @"Well... It's one of 'em!", nl
 * >> 1:
 * As the name implies, this type of set is read-only, in a sense being
 * for `hashset' what `tuple' is for `list'.
 * Because read-only sets cannot be modified, this allows them to operate
 * using an inline hash-vector, as well as not requiring the use of any
 * sort of lock.
 * NOTE: `_roset' is not exported by deemon, as it is an implementation-specific
 *        sequence type that is not required to implement the deemon standard
 *        programming interface.
 *        However GriferAtWork's implementation uses it to solve the problem
 *        caused by having hashset objects appear as constant variables. */
typedef struct Dee_roset_object DeeRoSetObject;

struct Dee_roset_item {
#ifdef __INTELLISENSE__
    DeeObject      *si_key;   /* [0..1][const] Set item key. */
#else
    DREF DeeObject *si_key;   /* [0..1][const] Set item key. */
#endif
    Dee_hash_t      si_hash;  /* [valis_if(si_key)][const]
                               * Hash of `si_key' (with a starting value of `0'). */
};

struct Dee_roset_object {
    Dee_OBJECT_HEAD
    size_t                rs_mask;    /* [> rs_size] Allocated set size. */
    size_t                rs_size;    /* [< rs_mask] Amount of non-NULL keys. */
    struct Dee_roset_item rs_elem[1]; /* [1..rs_mask+1] Set key hash-vector. */
};

/* The main `_roset' container class. */
DDATDEF DeeTypeObject DeeRoSet_Type;
#define DeeRoSet_Check(ob)       DeeObject_InstanceOfExact(ob,&DeeRoSet_Type) /* `_roset' is final */
#define DeeRoSet_CheckExact(ob)  DeeObject_InstanceOfExact(ob,&DeeRoSet_Type)

DFUNDEF DREF DeeObject *DCALL DeeRoSet_FromSequence(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeRoSet_FromIterator(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeRoSet_FromIteratorWithHint(DeeObject *__restrict self, size_t num_items);

/* Internal functions for constructing a read-only set object. */
DFUNDEF DREF DeeObject *DCALL DeeRoSet_New(void);
DFUNDEF DREF DeeObject *DCALL DeeRoSet_NewWithHint(size_t num_items);
DFUNDEF int DCALL DeeRoSet_Insert(DeeObject **__restrict pself, DeeObject *__restrict key);

/* @return:  1/true:  The object exists.
 * @return:  0/false: No such object exists.
 * @return: -1:       An error occurred. */
DFUNDEF int DCALL DeeRoSet_Contains(DeeObject *__restrict self, DeeObject *__restrict key);
DFUNDEF bool DCALL DeeRoSet_ContainsString(DeeObject *__restrict self, char const *__restrict key, size_t key_length);

/* Hash-iteration control. */
#define Dee_ROSET_HASHST(self,ro)      ((ro) & ((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_mask)
#define Dee_ROSET_HASHNX(hs,perturb)  (((hs) << 2) + (hs) + (perturb) + 1)
#define Dee_ROSET_HASHPT(perturb)      ((perturb) >>= 5) /* This `5' is tunable. */
#define Dee_ROSET_HASHIT(self,i)      (((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_elem+((i) & ((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_mask))

DECL_END

#endif /* !GUARD_DEEMON_ROSET_H */
