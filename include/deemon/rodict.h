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
#ifndef GUARD_DEEMON_RODICT_H
#define GUARD_DEEMON_RODICT_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_rodict_item   rodict_item
#define Dee_rodict_object rodict_object
#define RODICT_HASHST     Dee_RODICT_HASHST
#define RODICT_HASHNX     Dee_RODICT_HASHNX
#define RODICT_HASHPT     Dee_RODICT_HASHPT
#define RODICT_HASHIT     Dee_RODICT_HASHIT
#endif /* DEE_SOURCE */

/* A read-only variant of a Dict object, who's main purpose is to be used
 * by compiler optimizations in order to optimize generic mapping expressions
 * in various locations, when mapping arguments are constant.
 * As the name implies, this type of mapping is read-only, in a sense being
 * for `Dict' what `tuple' is for `list'.
 * The read-only Dict type inherits from `mapping', meaning that mapping
 * proxy types are automatically provided for by abstract base classes.
 * Secondly, because read-only dicts cannot be modified, this allows them
 * to operate using an inline hash-vector, as well as not requiring the
 * use of any sort of lock.
 * NOTE: `_rodict' is not exported by deemon, as it is an implementation-specific
 *        sequence type that is not required to implement the deemon standard
 *        programming interface.
 *        However GriferAtWork's implementation uses it to solve the problem
 *        caused by having Dict objects appear as constant variables. */
typedef struct Dee_rodict_object DeeRoDictObject;

struct Dee_rodict_item {
#ifdef __INTELLISENSE__
	DeeObject      *di_key;   /* [0..1][const] Dictionary item key. */
	DeeObject      *di_value; /* [1..1][valid_if(di_key)][const] Dictionary item value. */
#else
	DREF DeeObject *di_key;   /* [0..1][const] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1][valid_if(di_key)][const] Dictionary item value. */
#endif
	Dee_hash_t      di_hash;  /* [valid_if(di_key)][const] Hash of `di_key' (with a starting value of `0'). */
};

struct Dee_rodict_object {
	Dee_OBJECT_HEAD
	size_t                 rd_mask;    /* [const][!0] Allocated dictionary mask. */
	size_t                 rd_size;    /* [const][< rd_mask] Amount of non-NULL key-item pairs. */
	struct Dee_rodict_item rd_elem[1]; /* [rd_mask+1] Dict key-item pairs. */
};

DDATDEF DeeTypeObject DeeRoDict_Type;
#define DeeRoDict_Check(ob)         DeeObject_InstanceOfExact(ob, &DeeRoDict_Type) /* `_rodict' is final */
#define DeeRoDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeRoDict_Type)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoDict_FromIterator(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeRoDict_FromIteratorWithHint(DeeObject *__restrict self, size_t num_items);

/* Internal functions for constructing a read-only Dict object. */
DFUNDEF DREF DeeObject *DCALL DeeRoDict_New(void);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_NewWithHint(size_t num_items);
DFUNDEF int DCALL DeeRoDict_Insert(DREF DeeObject **__restrict pself, DeeObject *__restrict key, DeeObject *__restrict value);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeRoDict_GetItemDef(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeRoDict_GetItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeRoDict_GetItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF DREF DeeObject *DCALL DeeRoDict_GetItemStringDef(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict def);
INTDEF DREF DeeObject *DCALL DeeRoDict_GetItemStringLenDef(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeRoDict_HasItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeRoDict_HasItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
#endif /* CONFIG_BUILDING_DEEMON */

/* Hash-iteration control. */
#define Dee_RODICT_HASHST(self, hash)  ((hash) & ((DeeRoDictObject *)Dee_REQUIRES_OBJECT(self))->rd_mask)
#define Dee_RODICT_HASHNX(hs, perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define Dee_RODICT_HASHPT(perturb)     ((perturb) >>= 5) /* This `5' is tunable. */
#define Dee_RODICT_HASHIT(self, i)     (((DeeRoDictObject *)Dee_REQUIRES_OBJECT(self))->rd_elem+((i) & ((DeeRoDictObject *)Dee_REQUIRES_OBJECT(self))->rd_mask))


DECL_END

#endif /* !GUARD_DEEMON_RODICT_H */
