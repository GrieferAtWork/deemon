/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ROSET_H
#define GUARD_DEEMON_ROSET_H 1

#include "api.h"

#include "object.h"

#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_roset_item   roset_item
#define Dee_roset_object roset_object
#define ROSET_HASHST     DeeRoSet_HASHST
#define ROSET_HASHNX     DeeRoSet_HASHNX
#define ROSET_HASHIT     DeeRoSet_HASHIT
#endif /* DEE_SOURCE */

/* A read-only variant of a set object, who's main purpose is to be used
 * by compiler optimizations in order to optimize generic set expressions
 * in various locations, when sequence arguments are constant.
 * The main usage case is to optimize code such as:
 *
 * >> if (get_item() in [10, 20, 17, 19, 11, 3]) {
 * >>     print "Well... It's one of 'em!";
 * >> }
 * ASM:
 * >>     push  @_RoSet { 10, 20, 17, 19, 11, 3 }
 * >>     push  call global @get_item, #0
 * >>     contains top, pop
 * >>     jf    pop, 1f
 * >>     print @"Well... It's one of 'em!", nl
 * >> 1:
 *
 * As the name implies, this type of set is read-only, in a sense being
 * for `HashSet' what `Tuple' is for `List'.
 *
 * Because read-only sets cannot be modified, this allows them to operate
 * using an inline hash-vector, as well as not requiring the use of any
 * sort of lock.
 *
 * NOTE: `_RoSet' is exported as `deemon.HashSet.Frozen'. */
typedef struct Dee_roset_object DeeRoSetObject;

struct Dee_roset_item {
	DREF DeeObject *rsi_key;  /* [0..1][const] Set item key. */
	Dee_hash_t      rsi_hash; /* [valis_if(rsi_key)][const]
	                           * Hash of `rsi_key' (with a starting value of `0'). */
};

struct Dee_roset_object {
	Dee_OBJECT_HEAD
	size_t                                         rs_mask;  /* [> rs_size] Allocated set size. */
	size_t                                         rs_size;  /* [< rs_mask] Amount of non-NULL keys. */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_roset_item, rs_elem); /* [1..rs_mask+1] Set key hash-vector. */
};

/* The main `_RoSet' container class. */
DDATDEF DeeTypeObject DeeRoSet_Type;
#define DeeRoSet_Check(ob)       DeeObject_InstanceOfExact(ob, &DeeRoSet_Type) /* `_RoSet' is final */
#define DeeRoSet_CheckExact(ob)  DeeObject_InstanceOfExact(ob, &DeeRoSet_Type)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoSet_FromIterator(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoSet_FromIteratorWithHint(DeeObject *__restrict self, size_t num_items);

/* Internal functions for constructing a read-only set object. */
DFUNDEF WUNUSED DREF DeeRoSetObject *DCALL DeeRoSet_New(void);
DFUNDEF WUNUSED DREF DeeRoSetObject *DCALL DeeRoSet_NewWithHint(size_t num_items);

/* @return: 0:  Successfully inserted.
 * @return: 1:  Already exists.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeRoSet_Insert(/*in|out*/ DREF DeeRoSetObject **__restrict p_self,
                DeeObject *__restrict key);

/* Hash-iteration control. */
#define DeeRoSet_HASHST(self, ro)    ((ro) & ((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_mask)
#define DeeRoSet_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeRoSet_HASHIT(self, i)                              \
	(((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_elem + \
	 ((i) & ((DeeRoSetObject *)Dee_REQUIRES_OBJECT(self))->rs_mask))

DECL_END

#endif /* !GUARD_DEEMON_ROSET_H */
