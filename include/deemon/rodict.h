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
#ifndef GUARD_DEEMON_RODICT_H
#define GUARD_DEEMON_RODICT_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

DECL_BEGIN

/* A read-only variant of a dict object, who's main purpose is to be used
 * by compiler optimizations in order to optimize generic mapping expressions
 * in various locations, when mapping arguments are constant.
 * As the name implies, this type of mapping is read-only, in a sense being
 * for `dict' what `tuple' is for `list'.
 * The read-only dict type inherits from `mapping', meaning that mapping
 * proxy types are automatically provided for by abstract base classes.
 * Secondly, because read-only dicts cannot be modified, this allows them
 * to operate using an inline hash-vector, as well as not requiring the
 * use of any sort of lock.
 * NOTE: `_rodict' is not exported by deemon, as it is an implementation-specific
 *        sequence type that is not required to implement the deemon standard
 *        programming interface.
 *        However GriferAtWork's implementation uses it to solve the problem
 *        caused by having dict objects appear as constant variables. */
typedef struct rodict_object DeeRoDictObject;

struct rodict_item {
#ifdef __INTELLISENSE__
    DeeObject      *di_key;   /* [0..1][const] Dictionary item key. */
    DeeObject      *di_value; /* [1..1][valid_if(di_key)][const] Dictionary item value. */
#else
    DREF DeeObject *di_key;   /* [0..1][const] Dictionary item key. */
    DREF DeeObject *di_value; /* [1..1][valid_if(di_key)][const] Dictionary item value. */
#endif
    dhash_t         di_hash;  /* [valid_if(di_key)][const] Hash of `di_key' (with a starting value of `0'). */
};

struct rodict_object {
    OBJECT_HEAD
    size_t             rd_mask;    /* [const][!0] Allocated dictionary mask. */
    size_t             rd_size;    /* [const][< rd_mask] Amount of non-NULL key-item pairs. */
    struct rodict_item rd_elem[1]; /* [rd_mask+1] Dict key-item pairs. */
};

DDATDEF DeeTypeObject DeeRoDict_Type;
#define DeeRoDict_Check(ob)         DeeObject_InstanceOfExact(ob,&DeeRoDict_Type) /* `_rodict' is final */
#define DeeRoDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob,&DeeRoDict_Type)

DFUNDEF DREF DeeObject *DCALL DeeRoDict_FromSequence(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_FromIterator(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_FromIteratorWithHint(DeeObject *__restrict self, size_t num_items);

/* Internal functions for constructing a read-only dict object. */
DFUNDEF DREF DeeObject *DCALL DeeRoDict_New(void);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_NewWithHint(size_t num_items);
DFUNDEF int DCALL DeeRoDict_Insert(DREF DeeObject **__restrict pself, DeeObject *__restrict key, DeeObject *__restrict value);


DFUNDEF DREF DeeObject *DCALL DeeRoDict_GetItemString(DeeObject *__restrict self, char const *__restrict key);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_GetItemDef(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict def);
DFUNDEF DREF DeeObject *DCALL DeeRoDict_GetItemStringDef(DeeObject *__restrict self, char const *__restrict key, DeeObject *__restrict def);
DFUNDEF bool DCALL DeeRoDict_HasItemString(DeeObject *__restrict self, char const *__restrict key);

/* Hash-iteration control. */
#define RODICT_HASHST(self,hash)  ((hash) & ((DeeRoDictObject *)REQUIRES_OBJECT(self))->rd_mask)
#define RODICT_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define RODICT_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define RODICT_HASHIT(self,i)     (((DeeRoDictObject *)REQUIRES_OBJECT(self))->rd_elem+((i) & ((DeeRoDictObject *)REQUIRES_OBJECT(self))->rd_mask))


DECL_END

#endif /* !GUARD_DEEMON_RODICT_H */
