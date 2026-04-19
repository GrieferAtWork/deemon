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
/*!export **/
/*!export DeeRoSet**/
#ifndef GUARD_DEEMON_ROSET_H
#define GUARD_DEEMON_ROSET_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "hashset.h"      /* Dee_hashset_item */
#include "types.h"        /* DREF, DeeObject, DeeObject_InstanceOfExact, DeeTypeObject, Dee_OBJECT_HEAD, Dee_REQUIRES_OBJECT, Dee_hash_t, Dee_ssize_t */
#include "util/hash-io.h" /* Dee_hash_*, _DeeHash_* */

#include <stddef.h> /* NULL, size_t */

#ifndef __INTELLISENSE__
#include "object.h" /* DeeObject_Foreach, Dee_Incref */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* A read-only variant of a set object, who's main purpose is to be used
 * by compiler optimizations in order to optimize generic set expressions
 * in various locations, when sequence arguments are constant.
 * The main usage case is to optimize code such as:
 *
 * >> if (get_item() in [10, 20, 17, 19, 11, 3]) {
 * >>     print "Well... It's one of 'em!";
 * >> }
 * ASM:
 * >>     push  call global @get_item, #0
 * >>     push  contains const @_RoSet { 10, 20, 17, 19, 11, 3 }, pop
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

#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
typedef struct Dee_roset_object {
	Dee_OBJECT_HEAD /* All of the below fields are [const] */
	/*real*/Dee_hash_vidx_t                          rs_vsize;      /* # of keys in the set. */
	Dee_hash_t                                       rs_hmask;      /* [>= rs_vsize] Hash-mask */
	Dee_hash_gethidx_t                               rs_hidxget;    /* [1..1] Getter for "rs_htab" */
	union Dee_hash_htab                             *rs_htab;       /* [== (byte_t *)(_DeeRoSet_GetRealVTab(this) + rs_vsize)] Hash-table (contains indices into "rs_vtab", index==Dee_HASH_HTAB_EOF means END-OF-CHAIN) */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_hashset_item, rs_vtab);      /* [rs_vsize] Hashset keys (never contains deleted keys). */
//	COMPILER_FLEXIBLE_ARRAY(byte_t,                  rs_htab_data); /* Hashset hash-table. */
} DeeRoSetObject;

#define DeeRoSet_IsEmpty(self) (Dee_REQUIRES_OBJECT(DeeRoSetObject, self)->rs_vsize == 0)

/* The main `_RoSet' container class. */
DDATDEF DeeTypeObject DeeRoSet_Type;
#define DeeRoSet_Check(ob)       DeeObject_InstanceOfExact(ob, &DeeRoSet_Type) /* `_RoSet' is final */
#define DeeRoSet_CheckExact(ob)  DeeObject_InstanceOfExact(ob, &DeeRoSet_Type)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromHashSet(/*HashSet*/ DeeObject *__restrict self);


/* Special empty instance of `DeeRoSet_Type'
 * NOTE: This is _NOT_ a singleton! */
struct Dee_empty_roset_object {
	Dee_OBJECT_HEAD
	size_t               rs_vsize;        /* # of keys in the dict. */
	size_t               rs_hmask;        /* [>= rs_vsize] Hash-mask */
	Dee_hash_gethidx_t   rs_hidxget;      /* [1..1] Getter for "rs_htab" */
	union Dee_hash_htab *rs_htab;         /* [== (byte_t *)(_DeeRoSet_GetRealVTab(this) + rs_vsize)] Hash-table (contains indices into "rs_vtab", index==Dee_HASH_HTAB_EOF means END-OF-CHAIN) */
//	struct Dee_dict_item rs_vtab[0];      /* [rs_vsize] Set keys. */
	__BYTE_TYPE__        rs_htab_data[1]; /* Hashset hash-table. */
};
DDATDEF struct Dee_empty_roset_object DeeRoSet_EmptyInstance;
#define Dee_EmptyRoSet ((DeeObject *)&DeeRoSet_EmptyInstance)
#ifdef __INTELLISENSE__
#define DeeRoSet_NewEmpty() ((DeeObject *)&DeeRoSet_EmptyInstance)
#else /* __INTELLISENSE__ */
#define DeeRoSet_NewEmpty() (Dee_Incref(&DeeRoSet_EmptyInstance), (DeeObject *)&DeeRoSet_EmptyInstance)
#endif /* !__INTELLISENSE__ */


/************************************************************************/
/* ROSET BUILDER API                                                    */
/************************************************************************/

struct Dee_roset_builder {
	/* In an roset builder, the following are true:
	 * - rsb_set->ob_refcnt:  [UNDEFINED]
	 * - rsb_set->ob_type:    [UNDEFINED]
	 * - rsb_set->rs_vtab:    [0..rsb_set->rs_vsize|ALLOC(rsb_valloc)] (out-of-bound keys are undefined)
	 * - rsb_set->rs_htab:    [== _DeeRoSet_GetRealVTab(rsb_set) + rsb_valloc]
	 * - rsb_set->rs_hidxget: [== Dee_hash_hidxio[Dee_HASH_HIDXIO_FROM_VALLOC(rsb_valloc)].hxio_get] */
	DeeRoSetObject    *rsb_set;    /* [0..1][owned] The set being built. */
	size_t             rsb_valloc;  /* Allocated size of `rsb_set' (or 0 when `rsb_set' is `NULL') */
	Dee_hash_sethidx_t rsb_hidxset; /* [?..1][valid_if(rsb_set)] Setter for `rsb_set->rs_htab' */
};

#define Dee_RODICT_BUILDER_INIT { NULL, 0, NULL }
#define Dee_roset_builder_init(self) \
	(void)((self)->rsb_set   = NULL, \
	       (self)->rsb_valloc = 0)
#define Dee_roset_builder_cinit(self)           \
	(void)(Dee_ASSERT((self)->rsb_set == NULL), \
	       Dee_ASSERT((self)->rsb_valloc == 0))
DFUNDEF NONNULL((1)) void DCALL
Dee_roset_builder_fini(struct Dee_roset_builder *__restrict self);

DFUNDEF NONNULL((1)) void DCALL
Dee_roset_builder_init_with_hint(struct Dee_roset_builder *__restrict self,
                                 size_t num_items);

/* Pack the result of the builder and return it.
 * This function never fails, but "self" becomes invalid as a result. */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeRoSetObject *DCALL
Dee_roset_builder_pack(struct Dee_roset_builder *__restrict self);

/* Insert an item into the builder.
 * @return: 1 : Item not inserted because already present
 * @return: 0 : Item inserted successfully
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_t" */
Dee_roset_builder_insert(/*struct Dee_roset_builder*/ void *__restrict self, DeeObject *key);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_t" */
Dee_roset_builder_insert_inherited(/*struct Dee_roset_builder*/ void *__restrict self,
                                   /*inherit(always)*/ DREF DeeObject *key);
/* Insert all elements from "keys" into "self"
 * @return: * : The # of keys that weren't inserted because they were already present.
 * @return: -1: An error was thrown. */
#define Dee_roset_builder_insertall(self, keys) \
	DeeObject_Foreach(keys, &Dee_roset_builder_insert, self)




#ifdef DEE_SOURCE

/* Advance hash-index */
#define _DeeRoSet_HashIdxInit(self, p_hs, p_perturb, hash) _DeeHash_HashIdxInit(p_hs, p_perturb, hash, (self)->rs_hmask)
#define _DeeRoSet_HashIdxNext(self, p_hs, p_perturb, hash) _DeeHash_HashIdxNext(p_hs, p_perturb, hash, (self)->rs_hmask)

/* Get "rs_vtab" in both its:
 * - virt[ual] (index starts at 1), and
 * - real (index starts at 0) form */
#define _DeeRoSet_GetVirtVTab(self) _DeeHash_REAL_GetVirtVTab((self)->rs_vtab)
#define _DeeRoSet_GetRealVTab(self) _DeeHash_REAL_GetRealVTab((self)->rs_vtab)
#endif /* DEE_SOURCE */


#else /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
struct Dee_roset_item {
	DREF DeeObject *rsi_key;  /* [0..1][const] Set item key. */
	Dee_hash_t      rsi_hash; /* [valis_if(rsi_key)][const] Hash of `rsi_key'. */
};

typedef struct Dee_roset_object {
	Dee_OBJECT_HEAD
	size_t                                         rs_mask;  /* [>= rs_size] Allocated set size. */
	size_t                                         rs_size;  /* [<= rs_mask] Amount of non-NULL keys. */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_roset_item, rs_elem); /* [1..rs_mask+1] Set key hash-vector. */
} DeeRoSetObject;

#define DeeRoSet_IsEmpty(self) (Dee_REQUIRES_OBJECT(DeeRoSetObject, self)->rs_size == 0)

/* The main `_RoSet' container class. */
DDATDEF DeeTypeObject DeeRoSet_Type;
#define DeeRoSet_Check(ob)       DeeObject_InstanceOfExact(ob, &DeeRoSet_Type) /* `_RoSet' is final */
#define DeeRoSet_CheckExact(ob)  DeeObject_InstanceOfExact(ob, &DeeRoSet_Type)

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoSet_FromSequence(DeeObject *__restrict self);

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
#define DeeRoSet_HashSt(self, ro)    ((ro) & (self)->rs_mask)
#define DeeRoSet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeRoSet_HashIt(self, i)     ((self)->rs_elem + ((i) & (self)->rs_mask))
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

DECL_END

#endif /* !GUARD_DEEMON_ROSET_H */
