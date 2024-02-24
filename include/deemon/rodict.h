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
#ifndef GUARD_DEEMON_RODICT_H
#define GUARD_DEEMON_RODICT_H 1

#include "api.h"

#include "object.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_rodict_item   rodict_item
#define Dee_rodict_object rodict_object
#define RODICT_HASHST     DeeRoDict_HashSt
#define RODICT_HASHNX     DeeRoDict_HashNx
#define RODICT_HASHIT     DeeRoDict_HashIt
#endif /* DEE_SOURCE */

/* A read-only variant of a Dict object, who's main purpose is to be used by
 * compiler optimizations in order to optimize generic mapping expressions
 * in various locations, when mapping arguments are constant.
 *
 * As the name implies, this type of mapping is read-only, in a sense being
 * for `Dict' what `Tuple' is for `List'.
 *
 * The read-only Dict type inherits from `mapping', meaning that mapping
 * proxy types are automatically provided for by abstract base classes.
 * Secondly, because read-only dicts cannot be modified, this allows them
 * to operate using an inline hash-vector, as well as not requiring the
 * use of any sort of lock.
 *
 * NOTE: `_RoDict' is exported as `deemon.Dict.Frozen' */
typedef struct Dee_rodict_object DeeRoDictObject;

struct Dee_rodict_item {
	DREF DeeObject *rdi_key;   /* [0..1][const] Dictionary item key. */
	DREF DeeObject *rdi_value; /* [1..1][valid_if(rdi_key)][const] Dictionary item value. */
	Dee_hash_t      rdi_hash;  /* [valid_if(rdi_key)][const] Hash of `rdi_key' (with a starting value of `0'). */
};

struct Dee_rodict_object {
	Dee_OBJECT_HEAD
	size_t                                          rd_mask;  /* [const][!0] Allocated dictionary mask. */
	size_t                                          rd_size;  /* [const][< rd_mask] Amount of non-NULL key-item pairs. */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_rodict_item, rd_elem); /* [rd_mask+1] Dict key-item pairs. */
};

/* The main `_RoDict' container class. */
DDATDEF DeeTypeObject DeeRoDict_Type;
#define DeeRoDict_Check(ob)         DeeObject_InstanceOfExact(ob, &DeeRoDict_Type) /* `_RoDict' is final */
#define DeeRoDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeRoDict_Type)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self);

/* Internal functions for constructing a read-only Dict object. */
DFUNDEF WUNUSED DREF DeeRoDictObject *DCALL DeeRoDict_New(void);
DFUNDEF WUNUSED DREF DeeRoDictObject *DCALL DeeRoDict_NewWithHint(size_t num_items);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeRoDict_Insert(/*in|out*/ DREF DeeRoDictObject **__restrict p_self,
                 DeeObject *key, DeeObject *value);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeRoDict_HasItemStringHash(DeeRoDictObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeRoDict_HasItemStringLenHash(DeeRoDictObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeRoDict_GetItemNR(DeeRoDictObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeRoDict_GetItemNRStringHash(DeeRoDictObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DeeObject *DCALL DeeRoDict_GetItemNRStringLenHash(DeeRoDictObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL DeeRoDict_GetItemNRDef(DeeRoDictObject *self, DeeObject *key, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 4)) DeeObject *DCALL DeeRoDict_GetItemNRStringHashDef(DeeRoDictObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2, 5)) DeeObject *DCALL DeeRoDict_GetItemNRStringLenHashDef(DeeRoDictObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *def);
#define DeeRoDict_HasItemString(self, key)                    DeeRoDict_HasItemStringHash(self, key, Dee_HashStr(key))
#define DeeRoDict_HasItemStringLen(self, key, keylen)         DeeRoDict_HasItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeRoDict_GetItemNRString(self, key)                    DeeRoDict_GetItemNRStringHash(self, key, Dee_HashStr(key))
#define DeeRoDict_GetItemNRStringLen(self, key, keylen)         DeeRoDict_GetItemNRStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeRoDict_GetItemNRStringDef(self, key, def)            DeeRoDict_GetItemNRStringHashDef(self, key, Dee_HashStr(key), def)
#define DeeRoDict_GetItemNRStringLenDef(self, key, keylen, def) DeeRoDict_GetItemNRStringLenHashDef(self, key, keylen, Dee_HashPtr(key, keylen), def)
#endif /* !CONFIG_BUILDING_DEEMON */

/* Hash-iteration control. */
#define DeeRoDict_HashSt(self, hash)  ((hash) & (self)->rd_mask)
#define DeeRoDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeRoDict_HashIt(self, i)     ((self)->rd_elem+((i) & (self)->rd_mask))

DECL_END

#endif /* !GUARD_DEEMON_RODICT_H */
