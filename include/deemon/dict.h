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
/*!export Dee_HASH_HIDXIO_**/
/*!export DeeDict_**/
/*!export Dee_DICT_**/
/*!export Dee_dict_**/
/*!export _DeeDict_**/
/*!export __DeeDict_HashIdx**/
#ifndef GUARD_DEEMON_DICT_H
#define GUARD_DEEMON_DICT_H 1 /*!export-*/

#include "api.h"

#include "object.h"
#include "types.h"        /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_OBJECT_HEAD, Dee_OBJECT_HEAD_INIT, Dee_WEAKREF_SUPPORT, Dee_WEAKREF_SUPPORT_INIT, Dee_hash_t */
#include "util/hash-io.h" /* Dee_hash_*, _DeeHash_* */
#include "util/hash.h"    /* Dee_HashPtr, Dee_HashStr */
#include "util/lock.h"    /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_* */

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Dict can remember insertion order:
 *
 * How:
 * - "d_vtab" keep track of insertion order
 * - Following "d_vtab" (iow: allocated within the same heap-block),
 *   there is a uint[8|16|32|64]_t vector "d_htab" of indices into
 *   "d_vtab", that is the actual hash-table (iow: _DeeDict_HashIdxNext()
 *   enumerates indices of "d_htab", which in turn holds indices to
 *   the actual "d_vtab" table)
 *
 * Advantages:
 * - To increase the size of a dict, it is now possible to use realloc()
 *   on the combined heap-block, and simply re-build "d_htab".
 * - The length of "d_vtab" and "d_htab" can be linked to ensure
 *   that there are always enough free hash-table entries to prevent
 *   hash collisions. But there can be less free entries in "d_vtab",
 *   meaning less wasted memory (since "d_vtab" takes much more space)
 * - Dict housekeeping (as needed after repeatedly inserting/deleting
 *   items) can also be done in-place, since one can just trim deleted
 *   entries from "d_vtab" (by shifting successor with memovedownc),
 *   and then re-build "d_htab".
 * - Insertion order of elements is retained by the dict, allowing for
 *   more user-code use cases, including a better syntax for declaring
 *   structs via ctypes:
 *       >> import * from ctypes;
 *       >> struct point {
 *       >>     .x = int,
 *       >>     .y = int,
 *       >> };
 *   Previously, this way of writing code couldn't guaranty that the
 *   order of struct fields is retained, forcing the user to instead
 *   write:
 *       >> import * from ctypes;
 *       >> struct point {
 *       >>     ("x", int),
 *       >>     ("y", int),
 *       >> };
 *
 * #ifndef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
 * TODO: Also implement this for HashSet
 * #endif // !CONFIG_EXPERIMENTAL_ORDERED_HASHSET
 */
struct Dee_dict_item {
	Dee_hash_t      di_hash;  /* [valid_if(di_key)] Hash of `di_key' (undefined, but readable when "di_key == NULL") */
	union {
		DREF DeeObject *di_key_and_value[2]; /* [0..1] Inline vector of the key, followed by its value. */
		struct {
			DREF DeeObject *di_key;   /* [0..1] Dict item key, or "NULL" if deleted and not cleaned up (yet) */
			DREF DeeObject *di_value; /* [1..1][valid_if(di_key)] Dict item value. */
		}
#ifndef __COMPILER_HAVE_TRANSPARENT_STRUCT
		_di_skv
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
#define di_key   _di_skv.di_key   /*!export-*/
#define di_value _di_skv.di_value /*!export-*/
#endif /* __COMPILER_HAVE_TRANSPARENT_UNION */
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_di_ukv
#define di_key_and_value _di_ukv.di_key_and_value /*!export-*/
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define di_key   _di_ukv.di_key   /*!export-*/
#define di_value _di_ukv.di_value /*!export-*/
#else /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#define di_key   _di_ukv._di_skv.di_key   /*!export-*/
#define di_value _di_ukv._di_skv.di_value /*!export-*/
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

/* Static initializer for `struct Dee_dict_item' */
#define Dee_DICT_ITEM_INIT(hash, key, value) { hash, { { key, value } } }

typedef struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	/*real*/Dee_hash_vidx_t           d_valloc;  /* [lock(d_lock)][<= d_hmask] Allocated size of "d_vtab" (should be ~2/3rd of `d_hmask + 1') */
	/*real*/Dee_hash_vidx_t           d_vsize;   /* [lock(d_lock)][<= d_valloc] 1+ the greatest index in "d_vtab" that was ever initialized (and also the index of the next item in "d_vtab" to-be populated). */
	Dee_hash_vidx_t                   d_vused;   /* [lock(d_lock)][<= d_vsize] # of non-NULL keys in "d_vtab". */
	struct Dee_dict_item             *d_vtab;    /* [lock(d_lock)][0..d_vsize][owned_if(!= INTERNAL(DeeDict_EmptyTab))]
	                                              * [OWNED_AT(. + 1)] Value-table (offset by 1 to account for special meaning of index==Dee_HASH_HTAB_EOF) */
	Dee_hash_t                        d_hmask;   /* [lock(d_lock)] Hash mask (allocated hash-map size, minus 1). */
	struct Dee_hash_hidxio_ops const *d_hidxops; /* [lock(d_lock)][1..1] Operators for "d_htab" (always depends on "d_valloc") */
	union Dee_hash_htab              *d_htab;    /* [lock(d_lock)][== (byte_t *)(_DeeDict_GetRealVTab(this) + d_valloc)] Hash-table (contains indices into "d_vtab", index==Dee_HASH_HTAB_EOF means END-OF-CHAIN) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t               d_lock;    /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
} DeeDictObject;

#define DeeDict_EmptyVTab _DeeHash_VIRT_EmptyVTab(struct Dee_dict_item)
#define DeeDict_EmptyHTab _DeeHash_EmptyHTab

#ifdef CONFIG_NO_THREADS
#define _Dee_DICT_INIT_LOCK /* nothing */
#else /* CONFIG_NO_THREADS */
#define _Dee_DICT_INIT_LOCK , Dee_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
#define Dee_DICT_INIT                           \
	{                                           \
		Dee_OBJECT_HEAD_INIT(&DeeDict_Type),    \
		/* .d_valloc  = */ 0,                   \
		/* .d_vsize   = */ 0,                   \
		/* .d_vused   = */ 0,                   \
		/* .d_vtab    = */ DeeDict_EmptyVTab,   \
		/* .d_hmask   = */ 0,                   \
		/* .d_hidxops = */ &Dee_hash_hidxio[0], \
		/* .d_htab    = */ DeeDict_EmptyHTab    \
		_Dee_DICT_INIT_LOCK,                    \
		Dee_WEAKREF_SUPPORT_INIT                \
	}



#ifdef DEE_SOURCE
#define _DeeDict_GetVirtVTab(self)    _DeeHash_VIRT_GetVirtVTab((self)->d_vtab)
#define _DeeDict_SetVirtVTab(self, v) _DeeHash_VIRT_SetVirtVTab((self)->d_vtab, v)
#define _DeeDict_GetRealVTab(self)    _DeeHash_VIRT_GetRealVTab((self)->d_vtab)
#define _DeeDict_SetRealVTab(self, v) _DeeHash_VIRT_SetRealVTab((self)->d_vtab, v)

/* Advance hash-index */
#define _DeeDict_HashIdxInit(self, p_hs, p_perturb, hash) _DeeHash_HashIdxInit(p_hs, p_perturb, hash, (self)->d_hmask)
#define _DeeDict_HashIdxNext(self, p_hs, p_perturb, hash) _DeeHash_HashIdxNext(p_hs, p_perturb, hash, (self)->d_hmas)

#define _DeeDict_HTabGet(self, htab_index)    (*(self)->d_hidxops->hxio_get)((self)->d_htab, htab_index)
#define _DeeDict_HTabSet(self, htab_index, v) (*(self)->d_hidxops->hxio_set)((self)->d_htab, htab_index, v)
#endif /* DEE_SOURCE */

/* The main `Dict' container class */
DDATDEF DeeTypeObject DeeDict_Type;
#define DeeDict_Check(ob)         DeeObject_InstanceOf(ob, &DeeDict_Type)
#define DeeDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeDict_Type)

/* Create a new, empty Dict */
#define DeeDict_New() DeeObject_NewDefault(&DeeDict_Type)

/* "weak" hint:
 *    hint represents a lower bound for the # of items.
 *    returned Dict will have space for at least this many
 *    items, but probably quite a bit more, depending on how
 *    extra items can be addressed by the required hash-mask
 * non-"weak" hint:
 *    hint represents an exact # of items that the Dict has
 *    space for. No extra space is pre-allocated, even if the
 *    required hash-mask could address many more items.
 * Note that in the end, it doesn't matter which function is
 * used. Correct use of these functions merely means that the
 * returned Dict won't have to be resized too often. */
DFUNDEF WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_TryNewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_TryNewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_NewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_NewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromRoDict(/*RoDict*/ DeeObject *__restrict self);


#define DeeDict_GetItemString(self, key)                             DeeDict_GetItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_GetItemStringLen(self, key, keylen)                  DeeDict_GetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_GetItemStringHash(self, key, hash)                   (*DeeDict_Type.tp_seq->tp_getitem_string_hash)(self, key, hash)
#define DeeDict_GetItemStringLenHash(self, key, keylen, hash)        (*DeeDict_Type.tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_DelItemString(self, key)                             DeeDict_DelItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_DelItemStringLen(self, key, keylen)                  DeeDict_DelItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_DelItemStringHash(self, key, hash)                   (*DeeDict_Type.tp_seq->tp_delitem_string_hash)(self, key, hash)
#define DeeDict_DelItemStringLenHash(self, key, keylen, hash)        (*DeeDict_Type.tp_seq->tp_delitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_SetItemString(self, key, value)                      DeeDict_SetItemStringHash(self, key, Dee_HashStr(key), value)
#define DeeDict_SetItemStringLen(self, key, keylen, value)           DeeDict_SetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen), value)
#define DeeDict_SetItemStringHash(self, key, hash, value)            (*DeeDict_Type.tp_seq->tp_setitem_string_hash)(self, key, hash, value)
#define DeeDict_SetItemStringLenHash(self, key, keylen, hash, value) (*DeeDict_Type.tp_seq->tp_setitem_string_len_hash)(self, key, keylen, hash, value)

#define DeeDict_GetItem(self, key)        (*DeeDict_Type.tp_seq->tp_getitem)(self, key)
#define DeeDict_TryGetItem(self, key)     (*DeeDict_Type.tp_seq->tp_trygetitem)(self, key)
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
                              /*inherit(on_success)*/ DREF DeeObject **key_values);

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


/* Return the # of bound key within "self". */
#define DeeDict_SIZE(self) ((self)->d_vused)
#define DeeDict_SIZE_ATOMIC(self) \
	Dee_atomic_read_with_atomic_rwlock(&(self)->d_vused, &(self)->d_lock)

DDATDEF DeeObject DeeDict_Dummy; /* DEPRECATED (no longer used by the dict impl) */

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
