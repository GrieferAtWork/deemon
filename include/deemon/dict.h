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
#ifndef GUARD_DEEMON_DICT_H
#define GUARD_DEEMON_DICT_H 1

#include "api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "object.h"
#include "util/lock.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_dict_item   dict_item
#define Dee_dict_object dict_object
#endif /* DEE_SOURCE */

typedef struct Dee_dict_object DeeDictObject;


/* TODO: Make Dict remember insertion order:
 * - "d_elem" keep track of insertion order
 * - Following "d_elem" (iow: allocated within the same heap-block),
 *   there is a uint[8|16|32|64]_t vector of indices into "d_elem",
 *   that is the actual hash-table (iow: DeeDict_HashNx() enumerates
 *   indices of the uintX_t-vector, which in turn holds indices to
 *   the actual "d_elem" table)
 *
 * Advantages:
 * - To increase the size of a dict, it is now possible to use realloc()
 *   on the combined heap-block, and simply re-build the uintX_t-vector,
 *   whereas currently, one has to allocate an entirely new heap-block
 *   and copy "d_elem" into it.
 * - The length of "d_elem" and uintX_t-vector can be linked to ensure
 *   that there are always enough free hash-table entries to prevent
 *   hash collisions. But there can be less free entries in "d_elem",
 *   meaning less wasted memory (since "d_elem" takes much more space)
 * - Dict housekeeping (as needed after repeatedly inserting/deleting
 *   items) can also be done in-place, since once can just trim dummy
 *   entries from "d_elem" (by shifting successor with memovedownc),
 *   and then re-build the uintX_t-vector.
 * - Insertion order of elements is retained by the dict, allowing for
 *   more user-code use cases, including a better syntax for declaring
 *   structs via ctypes:
 *       >> import * from ctypes;
 *       >> struct point {
 *       >>     .x = int,
 *       >>     .y = int,
 *       >> };
 *   Currently, this way of writing code can't guaranty that the order
 *   of struct fields is retained, forcing the user to instead write:
 *       >> import * from ctypes;
 *       >> struct point {
 *       >>     ("x", int),
 *       >>     ("y", int),
 *       >> };
 *
 * Also implement this for HashSet
 */
#ifdef CONFIG_EXPERIMENTAL_ORDERED_DICTS
struct Dee_dict_item {
	Dee_hash_t      di_hash;  /* [valid_if(di_key)] Hash of `di_key' (undefined, but readable when "di_key == NULL") */
	DREF DeeObject *di_key;   /* [0..1] Dict item key, or "NULL" if deleted and not cleaned up (yet) */
	DREF DeeObject *di_value; /* [1..1][valid_if(di_key)] Dict item value. */
};

/* Index for "d_vtab" (real and virt version) */
typedef size_t Dee_dict_vidx_t;
#define Dee_dict_vidx_virt2real(p_self) (void)(--*(p_self))
#define Dee_dict_vidx_real2virt(p_self) (void)(++*(p_self))
#define Dee_dict_vidx_toreal(self)      ((self) - 1)
#define Dee_dict_vidx_tovirt(self)      ((self) + 1)
#define Dee_dict_vidx_virt_lt_real(virt_self, real_count) ((virt_self) <= (real_count))
/*#define Dee_dict_vidx_virt_lt_real(virt_self, real_count) (Dee_dict_vidx_toreal(virt_self) < (real_count))*/

typedef WUNUSED_T NONNULL_T((1)) /*virt*/Dee_dict_vidx_t (DCALL *Dee_dict_gethidx_t)(void *__restrict htab, size_t index);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_sethidx_t)(void *__restrict htab, size_t index, /*virt*/Dee_dict_vidx_t value);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_movhidx_t)(void *__restrict dst, void const *__restrict src, size_t n_words);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_uprhidx_t)(void *__restrict dst, void const *__restrict src, size_t n_words);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_dwnhidx_t)(void *__restrict dst, void const *__restrict src, size_t n_words);
struct Dee_dict_hidxio_struct {
	Dee_dict_gethidx_t dhxio_get; /* Getter */
	Dee_dict_sethidx_t dhxio_set; /* Setter */
	Dee_dict_movhidx_t dhxio_mov; /* memmove */
#define dhxio_movup    dhxio_mov  /* memmoveup */
#define dhxio_movdown  dhxio_mov  /* memmovedown */
	Dee_dict_uprhidx_t dhxio_upr; /* Upsize ("dst" is DEE_DICT_HIDXIO+1; assume that "dst >= src") */
	Dee_dict_dwnhidx_t dhxio_dwn; /* Downsize ("dst" is DEE_DICT_HIDXIO-1; assume that "dst <= src") */
};

/* NOTE: HIDXIO indices can also used as <<shifts to multiply some value by the size of an index:
 * >> size_t htab_size = (d_hmask + 1) << DEE_DICT_HIDXIO_FROMALLOC(...); */
#if __SIZEOF_SIZE_T__ >= 8
#define DEE_DICT_HIDXIO_COUNT 4
#define DEE_DICT_HIDXIO_IS8(valloc)  likely((valloc) <= __UINT8_C(0xff))
#define DEE_DICT_HIDXIO_IS16(valloc) likely((valloc) <= __UINT16_C(0xffff))
#define DEE_DICT_HIDXIO_IS32(valloc) likely((valloc) <= __UINT32_C(0xffffffff))
#define DEE_DICT_HIDXIO_IS64(valloc) 1
#define DEE_DICT_HIDXIO_FROMALLOC(valloc) \
	(DEE_DICT_HIDXIO_IS8(valloc) ? 0 : DEE_DICT_HIDXIO_IS16(valloc) ? 1 : DEE_DICT_HIDXIO_IS32(valloc) ? 2 : 3)
#elif __SIZEOF_SIZE_T__ >= 4
#define DEE_DICT_HIDXIO_COUNT 3
#define DEE_DICT_HIDXIO_IS8(valloc)  likely((valloc) <= __UINT8_C(0xff))
#define DEE_DICT_HIDXIO_IS16(valloc) likely((valloc) <= __UINT16_C(0xffff))
#define DEE_DICT_HIDXIO_IS32(valloc) 1
#define DEE_DICT_HIDXIO_FROMALLOC(valloc) \
	(DEE_DICT_HIDXIO_IS8(valloc) ? 0 : DEE_DICT_HIDXIO_IS16(valloc) ? 1 : 2)
#elif __SIZEOF_SIZE_T__ >= 2
#define DEE_DICT_HIDXIO_COUNT 2
#define DEE_DICT_HIDXIO_IS8(valloc)  likely((valloc) <= __UINT8_C(0xff))
#define DEE_DICT_HIDXIO_IS16(valloc) 1
#define DEE_DICT_HIDXIO_FROMALLOC(valloc) \
	(DEE_DICT_HIDXIO_IS8(valloc) ? 0 : 1)
#else /* __SIZEOF_SIZE_T__ >= 1 */
#define DEE_DICT_HIDXIO_COUNT 1
#define DEE_DICT_HIDXIO_IS8(valloc) 1
#define DEE_DICT_HIDXIO_FROMALLOC(valloc) 0
#endif /* __SIZEOF_SIZE_T__ < 1 */

DDATDEF struct Dee_dict_hidxio_struct Dee_tpconst Dee_dict_hidxio[DEE_DICT_HIDXIO_COUNT];
DFUNDEF WUNUSED NONNULL((1)) Dee_dict_vidx_t DCALL Dee_dict_gethidx8(void *__restrict htab, size_t index);
DFUNDEF NONNULL((1)) void DCALL Dee_dict_sethidx8(void *__restrict htab, size_t index, Dee_dict_vidx_t value);

/* Index value found in "d_htab" when end-of-chain is encountered. */
#define Dee_DICT_HTAB_EOF 0

struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                  d_valloc;  /* [lock(d_lock)][<= d_hmask] Allocated size of "d_vtab" (should be ~2/3rd of `d_hmask + 1') */
	/*real*/Dee_dict_vidx_t d_vsize;   /* [lock(d_lock)][<= d_valloc] 1+ the greatest index in "d_vtab" that was ever initialized (and also the index of the next item in "d_vtab" to-be populated). */
	size_t                  d_vused;   /* [lock(d_lock)][<= d_vsize] # of non-NULL keys in "d_vtab". */
	struct Dee_dict_item   *d_vtab;    /* [lock(d_lock)][0..d_vsize][owned_if(!= INTERNAL(DeeDict_EmptyTab))]
	                                    * [OWNED_AT(. + 1)] Value-table (offset by 1 to account for special meaning of index==Dee_DICT_HTAB_EOF) */
	size_t                  d_hmask;   /* [lock(d_lock)] Hash mask (allocated hash-map size, minus 1). */
	Dee_dict_gethidx_t      d_hidxget; /* [lock(d_lock)] Getter for "d_htab" (always depends on "d_valloc") */
	Dee_dict_sethidx_t      d_hidxset; /* [lock(d_lock)] Setter for "d_htab" (always depends on "d_valloc") */
	void                   *d_htab;    /* [lock(d_lock)][== (byte_t *)(d_vtab + 1) + d_valloc] Hash-table (contains indices into "d_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t     d_lock;    /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

/* Return the # of bound key within "self". */
#define DeeDict_SIZE(self)        ((self)->d_vused)
#define DeeDict_SIZE_ATOMIC(self) Dee_atomic_read(&(self)->d_vused)

#define _DeeDict_GetVirtVTab(self)    ((self)->d_vtab)
#define _DeeDict_SetVirtVTab(self, v) (void)((self)->d_vtab = (v))
#define _DeeDict_GetRealVTab(self)    ((self)->d_vtab + 1)
#define _DeeDict_SetRealVTab(self, v) (void)((self)->d_vtab = (v) - 1)

DDATDEF __BYTE_TYPE__ const _DeeDict_EmptyTab[];
#define DeeDict_EmptyVTab /*virt*/ ((struct Dee_dict_item *)_DeeDict_EmptyTab - 1)
#define DeeDict_EmptyHTab ((void *)_DeeDict_EmptyTab)

#ifdef CONFIG_NO_THREADS
#define _Dee_DICT_INIT_LOCK /* nothing */
#else /* CONFIG_NO_THREADS */
#define _Dee_DICT_INIT_LOCK , DEE_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
#define Dee_DICT_INIT                          \
	{                                          \
		Dee_OBJECT_HEAD_INIT(&DeeDict_Type),   \
		/* .d_valloc  = */ 0,                  \
		/* .d_vsize   = */ 0,                  \
		/* .d_vused   = */ 0,                  \
		/* .d_vtab    = */ DeeDict_EmptyVTab,  \
		/* .d_hmask   = */ 0,                  \
		/* .d_hidxget = */ &Dee_dict_gethidx8, \
		/* .d_hidxset = */ &Dee_dict_sethidx8, \
		/* .d_htab    = */ DeeDict_EmptyHTab   \
		_Dee_DICT_INIT_LOCK,                   \
		Dee_WEAKREF_SUPPORT_INIT               \
	}

/* Advance hash-index */
#define _DeeDict_HashIdxInit(self, p_hs, p_perturb, hash) \
	(void)(*(p_hs) = *(p_perturb) = (hash) & (self)->d_hmask)
#define _DeeDict_HashIdxAdv(self, p_hs, p_perturb) \
	(void)(*(p_hs) = (*(p_hs) << 2) + *(p_hs) + *(p_perturb) + 1, *(p_perturb) >>= 5)

/* Get/set vtab-index "i" of htab at a given "hs" */
#define /*virt*/_DeeDict_HTabGet(self, hs)    (*(self)->d_hidxget)((self)->d_htab, (hs) & (self)->d_hmask)
#define _DeeDict_HTabSet(self, hs, /*virt*/i) (*(self)->d_hidxset)((self)->d_htab, (hs) & (self)->d_hmask, i)


DDATDEF DeeObject DeeDict_Dummy; /* DEPRECATED */

#else /* CONFIG_EXPERIMENTAL_ORDERED_DICTS */
struct Dee_dict_item {
	DREF DeeObject *di_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
	Dee_hash_t      di_hash;  /* [valid_if(di_key)][lock(:d_lock)] Hash of `di_key' (with a starting value of `0').
	                           * NOTE: Some random value when `di_key' is the dummy key. */
};

struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                d_mask; /* [lock(d_lock)][> d_size || d_mask == 0] Allocated dictionary size. */
	size_t                d_size; /* [lock(d_lock)][< d_mask || d_mask == 0] Amount of non-NULL key-item pairs. */
	size_t                d_used; /* [lock(d_lock)][<= d_size] Amount of key-item pairs actually in use.
	                               * HINT: The difference to `d_size' is the number of dummy keys currently in use. */
	struct Dee_dict_item *d_elem; /* [1..d_size|ALLOC(d_mask+1)][lock(d_lock)]
	                               * [owned_if(!= INTERNAL(DeeDict_EmptyItems))] Dict key-item pairs (items). */
	/* TODO: Make Dict remember insertion order:
	 * - "d_elem" keep track of insertion order
	 * - Following "d_elem" (iow: allocated within the same heap-block),
	 *   there is a uint[8|16|32|64]_t vector of indices into "d_elem",
	 *   that is the actual hash-table (iow: DeeDict_HashNx() enumerates
	 *   indices of the uintX_t-vector, which in turn holds indices to
	 *   the actual "d_elem" table)
	 *
	 * Advantages:
	 * - To increase the size of a dict, it is now possible to use realloc()
	 *   on the combined heap-block, and simply re-build the uintX_t-vector,
	 *   whereas currently, one has to allocate an entirely new heap-block
	 *   and copy "d_elem" into it.
	 * - The length of "d_elem" and uintX_t-vector can be linked to ensure
	 *   that there are always enough free hash-table entries to prevent
	 *   hash collisions. But there can be less free entries in "d_elem",
	 *   meaning less wasted memory (since "d_elem" takes much more space)
	 * - Dict housekeeping (as needed after repeatedly inserting/deleting
	 *   items) can also be done in-place, since once can just trim dummy
	 *   entries from "d_elem" (by shifting successor with memovedownc),
	 *   and then re-build the uintX_t-vector.
	 * - Insertion order of elements is retained by the dict, allowing for
	 *   more user-code use cases, including a better syntax for declaring
	 *   structs via ctypes:
	 *       >> import * from ctypes;
	 *       >> struct point {
	 *       >>     .x = int,
	 *       >>     .y = int,
	 *       >> };
	 *   Currently, this way of writing code can't guaranty that the order
	 *   of struct fields is retained, forcing the user to instead write:
	 *       >> import * from ctypes;
	 *       >> struct point {
	 *       >>     ("x", int),
	 *       >>     ("y", int),
	 *       >> };
	 *
	 * Also implement this for HashSet
	 */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t   d_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

/* Return the # of bound key within "self". */
#define DeeDict_SIZE(self)        ((self)->d_used)
#define DeeDict_SIZE_ATOMIC(self) atomic_read(&(self)->d_used)

#ifdef CONFIG_NO_THREADS
#define Dee_DICT_INIT \
	{ Dee_OBJECT_HEAD_INIT(&DeeDict_Type), 0, 0, 0, (struct Dee_dict_item *)DeeDict_EmptyItems, Dee_WEAKREF_SUPPORT_INIT }
#else /* CONFIG_NO_THREADS */
#define Dee_DICT_INIT \
	{ Dee_OBJECT_HEAD_INIT(&DeeDict_Type), 0, 0, 0, (struct Dee_dict_item *)DeeDict_EmptyItems, DEE_ATOMIC_RWLOCK_INIT, Dee_WEAKREF_SUPPORT_INIT }
#endif /* !CONFIG_NO_THREADS */

/* A dummy object used by Dict and HashSet to refer to
 * deleted keys that are still apart of the hash chain.
 * Only here to allow dex modules to correct work
 * DON'T USE THIS OBJECT AS KEY FOR DICTS OR HASHSETS!
 * DO NOT EXPOSE THIS OBJECT TO USER-CODE! (not even in `rt'!) */
DDATDEF DeeObject DeeDict_Dummy;
DDATDEF struct Dee_dict_item const DeeDict_EmptyItems[1];

/* The basic dictionary item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     Dee_hash_t i, perturb;
 * >>     Dee_hash_t hash = DeeObject_Hash(key);
 * >>     perturb = i = DeeDict_HashSt(self, hash);
 * >>     for (;; i = DeeDict_HashNx(i, perturb), DeeDict_HashPt(perturb)) {
 * >>          struct Dee_dict_item *item = DeeDict_HashIt(self, i);
 * >>          if (!item->di_key)
 * >>              break; // Not found
 * >>          if (item->di_hash != hash)
 * >>              continue; // Non-matching hash
 * >>          if (DeeObject_TryCompareEq(key, item->di_key) == 0)
 * >>              return item->di_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 * - `DeeDict_HashNx()' must be able to eventually enumerate all integers `<= d_mask'
 * - `d_size' must always be lower than `d_mask', ensuring that at least one(1)
 *   entry exists that no value has been assigned to (Acting as a sentinel to
 *   terminate a search for an existing element).
 * - `d_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but noone can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DeeDict_HashSt(self, hash)  ((hash) & (self)->d_mask)
#define DeeDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5)
#define DeeDict_HashIt(self, i)     ((self)->d_elem + ((i) & (self)->d_mask))

#endif /* !CONFIG_EXPERIMENTAL_ORDERED_DICTS */


/* The main `Dict' container class */
DDATDEF DeeTypeObject DeeDict_Type;
#define DeeDict_Check(ob)         DeeObject_InstanceOf(ob, &DeeDict_Type)
#define DeeDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeDict_Type)

#define DeeDict_New() DeeObject_NewDefault(&DeeDict_Type)
/* "weak" hint:
 *    hint represents a lower bound for the # of items.
 *    returned dict will have space for at least this many
 *    items, but probably quite a bit more, depending on how
 *    extra items can be addressed by the required hash-mask
 * non-"weak" hint:
 *    hint represents an exact # of items that the dict has
 *    space for. No extra space is pre-allocated, even if the
 *    required hash-mask could address many more items.
 * Note that in the end, it doesn't matter which function is
 * used. Correct use of these functions merely means that the
 * returned dict won't have to be resized too often.
 */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeDict_TryNewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeDict_TryNewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeDict_NewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeDict_NewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromSequenceInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromRoDict(DeeObject *__restrict self);


#if defined(CONFIG_BUILDING_DEEMON) && !defined(CONFIG_EXPERIMENTAL_ORDERED_DICTS)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemStringHash(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemStringLenHash(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemStringHash(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemStringLenHash(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeDict_SetItemStringHash(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeDict_SetItemStringLenHash(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *value);
#else /* CONFIG_BUILDING_DEEMON && !CONFIG_EXPERIMENTAL_ORDERED_DICTS */
#define DeeDict_GetItemStringHash(self, key, hash)                    (*DeeDict_Type.tp_seq->tp_getitem_string_hash)(self, key, hash)
#define DeeDict_GetItemStringLenHash(self, key, keylen, hash)         (*DeeDict_Type.tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_DelItemStringHash(self, key, hash)                    (*DeeDict_Type.tp_seq->tp_delitem_string_hash)(self, key, hash)
#define DeeDict_DelItemStringLenHash(self, key, keylen, hash)         (*DeeDict_Type.tp_seq->tp_delitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_SetItemStringHash(self, key, hash, value)             (*DeeDict_Type.tp_seq->tp_setitem_string_hash)(self, key, hash, value)
#define DeeDict_SetItemStringLenHash(self, key, keylen, hash, value)  (*DeeDict_Type.tp_seq->tp_setitem_string_len_hash)(self, key, keylen, hash, value)
#endif /* !CONFIG_BUILDING_DEEMON || CONFIG_EXPERIMENTAL_ORDERED_DICTS */
#define DeeDict_TryGetItem(self, key)                       (*DeeDict_Type.tp_seq->tp_trygetitem)(self, key)
#define DeeDict_GetItemString(self, key)                    DeeDict_GetItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_GetItemStringLen(self, key, keylen)         DeeDict_GetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_DelItemString(self, key)                    DeeDict_DelItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_DelItemStringLen(self, key, keylen)         DeeDict_DelItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_SetItemString(self, key, value)             DeeDict_SetItemStringHash(self, key, Dee_HashStr(key), value)
#define DeeDict_SetItemStringLen(self, key, keylen, value)  DeeDict_SetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen), value)

#define DeeDict_GetItem(self, key)        (*DeeDict_Type.tp_seq->tp_getitem)(self, key)
#define DeeDict_DelItem(self, key)        (*DeeDict_Type.tp_seq->tp_delitem)(self, key)
#define DeeDict_SetItem(self, key, value) (*DeeDict_Type.tp_seq->tp_setitem)(self, key, value)

#define DeeDict_Clear(self) ((*DeeDict_Type.tp_gc->tp_clear)(self))

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_values: A vector containing `num_items*2' elements,
 *                     even ones being keys and odd ones being items.
 * @param: num_items:  The number of key-value pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyValuesInherited(size_t num_items,
                              /*inhert(on_success)*/ DREF DeeObject **key_values);

/* Locking helpers for `DeeDictObject' */
#define DeeDict_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->d_lock)
#define DeeDict_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->d_lock)
#define DeeDict_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->d_lock)
#define DeeDict_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->d_lock)
#define DeeDict_LockRead(self)       Dee_atomic_rwlock_read(&(self)->d_lock)
#define DeeDict_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->d_lock)
#define DeeDict_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->d_lock)
#define DeeDict_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->d_lock)
#define DeeDict_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->d_lock)
#define DeeDict_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->d_lock)
#define DeeDict_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->d_lock)
#define DeeDict_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->d_lock)

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
