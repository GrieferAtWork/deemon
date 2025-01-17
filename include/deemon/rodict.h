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
#ifndef GUARD_DEEMON_RODICT_H
#define GUARD_DEEMON_RODICT_H 1

#include "api.h"

#ifdef CONFIG_EXPERIMENTAL_ORDERED_RODICTS
#include "dict.h"
#endif /* CONFIG_EXPERIMENTAL_ORDERED_RODICTS */
#include "object.h"

#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_rodict_object rodict_object
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

#ifdef CONFIG_EXPERIMENTAL_ORDERED_RODICTS
#ifdef DEE_SOURCE
#define Dee_rodict_builder rodict_builder
#endif /* DEE_SOURCE */

struct Dee_rodict_object {
	Dee_OBJECT_HEAD /* All of the below fields are [const] */
	/*real*/Dee_dict_vidx_t                       rd_vsize;      /* # of key-value pairs in the dict. */
	Dee_hash_t                                    rd_hmask;      /* [>= rd_vsize] Hash-mask */
	Dee_dict_gethidx_t                            rd_hidxget;    /* [1..1] Getter for "rd_htab" */
	void                                         *rd_htab;       /* [== (byte_t *)(_DeeRoDict_GetRealVTab(this) + rd_vsize)] Hash-table (contains indices into "rd_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_dict_item, rd_vtab);      /* [rd_vsize] Dict key-item pairs (never contains deleted keys). */
//	COMPILER_FLEXIBLE_ARRAY(byte_t,               rd_htab_data); /* Dict hash-table. */
};

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromDict(/*Dict*/ DeeObject *__restrict self);


/* Special empty instance of `DeeRoDict_Type'
 * NOTE: This is _NOT_ a singleton! */
#ifdef GUARD_DEEMON_OBJECTS_RODICT_C
struct Dee_empty_rodict_object {
	Dee_OBJECT_HEAD
	size_t                                        rd_vsize;        /* # of key-value pairs in the dict. */
	size_t                                        rd_hmask;        /* [>= rd_vsize] Hash-mask */
	Dee_dict_gethidx_t                            rd_hidxget;      /* [1..1] Getter for "rd_htab" */
	void                                         *rd_htab;         /* [== (byte_t *)(_DeeRoDict_GetRealVTab(this) + rd_vsize)] Hash-table (contains indices into "rd_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
//	struct Dee_dict_item                          rd_vtab[0];      /* [rd_vsize] Dict key-item pairs. */
	__BYTE_TYPE__                                 rd_htab_data[1]; /* Dict key-item pairs. */
};
DDATDEF struct Dee_empty_rodict_object DeeRoDict_EmptyInstance;
#define Dee_EmptyRoDict ((DeeObject *)&DeeRoDict_EmptyInstance)
#else /* GUARD_DEEMON_OBJECTS_RODICT_C */
DDATDEF DeeObject DeeRoDict_EmptyInstance;
#define Dee_EmptyRoDict (&DeeRoDict_EmptyInstance)
#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */



/************************************************************************/
/* RODICT BUILDER API                                                   */
/************************************************************************/
struct Dee_rodict_builder {
	/* In an rodict builder, the following are true:
	 * - rdb_dict->ob_refcnt:  [UNDEFINED]
	 * - rdb_dict->ob_type:    [UNDEFINED]
	 * - rdb_dict->rd_vtab:    [0..rdb_dict->rd_vsize|ALLOC(rdb_valloc)] (out-of-bound keys are undefined)
	 * - rdb_dict->rd_htab:    [== _DeeRoDict_GetRealVTab(rdb_dict) + rdb_valloc]
	 * - rdb_dict->rd_hidxget: [== Dee_dict_hidxio[DEE_DICT_HIDXIO_FROMALLOC(rdb_valloc)].dhxio_get] */
	DeeRoDictObject   *rdb_dict;    /* [0..1][owned] The dict being built. */
	size_t             rdb_valloc;  /* Allocated size of `rdb_dict' (or 0 when `rdb_dict' is `NULL') */
	Dee_dict_sethidx_t rdb_hidxset; /* [?..1][valid_if(rdb_dict)] Setter for `rdb_dict->rd_htab' */
};

#define Dee_RODICT_BUILDER_INIT { NULL, 0, NULL }
#define Dee_rodict_builder_init(self) \
	(void)((self)->rdb_dict   = NULL, \
	       (self)->rdb_valloc = 0)
#define Dee_rodict_builder_cinit(self)           \
	(void)(Dee_ASSERT((self)->rdb_dict == NULL), \
	       Dee_ASSERT((self)->rdb_valloc == 0))
DFUNDEF NONNULL((1)) void DCALL
Dee_rodict_builder_fini(struct Dee_rodict_builder *__restrict self);

DFUNDEF NONNULL((1)) void DCALL
Dee_rodict_builder_init_with_hint(struct Dee_rodict_builder *__restrict self,
                                  size_t num_items);

/* Pack the result of the builder and return it.
 * This function never fails, but "self" becomes invalid as a result. */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeRoDictObject *DCALL
Dee_rodict_builder_pack(struct Dee_rodict_builder *__restrict self);

DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem(/*struct Dee_rodict_builder*/ void *__restrict self,
                           DeeObject *key, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem_inherited(/*struct Dee_rodict_builder*/ void *__restrict self,
                                     /*inherit(always)*/ DREF DeeObject *key,
                                     /*inherit(always)*/ DREF DeeObject *value);
#define Dee_rodict_builder_update(self, mapping) \
	DeeObject_ForeachPair(mapping, &Dee_rodict_builder_setitem, self)




#ifdef DEE_SOURCE

/* Advance hash-index */
#define _DeeRoDict_HashIdxInit(self, p_hs, p_perturb, hash) \
	__DeeDict_HashIdxInitEx(p_hs, p_perturb, hash, (self)->rd_hmask)
#define _DeeRoDict_HashIdxAdv(self, p_hs, p_perturb) \
	__DeeDict_HashIdxAdvEx(p_hs, p_perturb)

/* Get/set vtab-index "i" of htab at a given "hs" */
#define /*virt*/_DeeRoDict_HTabGet(self, hs) (*(self)->rd_hidxget)((self)->rd_htab, (hs) & (self)->rd_hmask)

/* Get "rd_vtab" in both its:
 * - virt[ual] (index starts at 1), and
 * - real (index starts at 0) form */
#define _DeeRoDict_GetVirtVTab(self) ((self)->rd_vtab - 1)
#define _DeeRoDict_GetRealVTab(self) ((self)->rd_vtab)
#endif /* DEE_SOURCE */

#else /* CONFIG_EXPERIMENTAL_ORDERED_RODICTS */

#ifdef DEE_SOURCE
#define Dee_rodict_item rodict_item
#endif /* DEE_SOURCE */

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

/* Special empty instance of `DeeRoDict_Type'
 * NOTE: This is _NOT_ a singleton! */
#ifdef GUARD_DEEMON_OBJECTS_RODICT_C
struct Dee_empty_rodict_object {
	Dee_OBJECT_HEAD
	size_t                 rd_mask;    /* [== 0] */
	size_t                 rd_size;    /* [== 0] */
	struct Dee_rodict_item rd_elem[1]; /* [== {{NULL,NULL,0}}] */
};
DDATDEF struct Dee_empty_rodict_object DeeRoDict_EmptyInstance;
#define Dee_EmptyRoDict ((DeeObject *)&DeeRoDict_EmptyInstance)
#else /* GUARD_DEEMON_OBJECTS_RODICT_C */
DDATDEF DeeObject DeeRoDict_EmptyInstance;
#define Dee_EmptyRoDict (&DeeRoDict_EmptyInstance)
#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */

/* Hash-iteration control. */
#define DeeRoDict_HashSt(self, hash)  ((hash) & (self)->rd_mask)
#define DeeRoDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeRoDict_HashIt(self, i)     ((self)->rd_elem+((i) & (self)->rd_mask))

/* Internal functions for constructing a read-only Dict object. */
DFUNDEF WUNUSED DREF DeeRoDictObject *DCALL DeeRoDict_New(void);
DFUNDEF WUNUSED DREF DeeRoDictObject *DCALL DeeRoDict_NewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeRoDictObject *DCALL DeeRoDict_TryNewWithHint(size_t num_items);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeRoDict_Insert(/*in|out*/ DREF DeeRoDictObject **__restrict p_self,
                 DeeObject *key, DeeObject *value);

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self);





/************************************************************************/
/* FORWARD-COMPATIBLE RODICT BUILDER                                    */
/************************************************************************/
struct Dee_rodict_builder {
	DeeRoDictObject *rdb_dict; /* [0..1][owned] The dict being built. */
};

#define Dee_RODICT_BUILDER_INIT        { NULL }
#define Dee_rodict_builder_init(self)  (void)((self)->rdb_dict = NULL)
#define Dee_rodict_builder_cinit(self) (void)(Dee_ASSERT((self)->rdb_dict == NULL))
#define Dee_rodict_builder_fini(self)  (void)Dee_XDecref_likely((self)->rdb_dict)
#define Dee_rodict_builder_init_with_hint(self, num_items) \
	(void)((self)->rdb_dict = DeeRoDict_TryNewWithHint(num_items))
#define Dee_rodict_builder_pack(self) \
	((self)->rdb_dict ? (self)->rdb_dict : (Dee_Incref(Dee_EmptyRoDict), (DREF DeeRoDictObject *)Dee_EmptyRoDict))
DFUNDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem(/*struct Dee_rodict_builder*/ void *__restrict self,
                           DeeObject *key, DeeObject *value);

LOCAL WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem_inherited(/*struct Dee_rodict_builder*/ void *__restrict self,
                                     /*inherit(always)*/ DREF DeeObject *key,
                                     /*inherit(always)*/ DREF DeeObject *value) {
	int result = Dee_rodict_builder_setitem(self, key, value);
	Dee_Decref_unlikely(key);
	Dee_Decref_unlikely(value);
	return result;
}
#define Dee_rodict_builder_update(self, mapping) \
	DeeObject_ForeachPair(mapping, &Dee_rodict_builder_setitem, self)

#endif /* !CONFIG_EXPERIMENTAL_ORDERED_RODICTS */

/* The main `_RoDict' container class. */
DDATDEF DeeTypeObject DeeRoDict_Type;
#define DeeRoDict_Check(ob)         DeeObject_InstanceOfExact(ob, &DeeRoDict_Type) /* `_RoDict' is final */
#define DeeRoDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeRoDict_Type)

DECL_END

#endif /* !GUARD_DEEMON_RODICT_H */
