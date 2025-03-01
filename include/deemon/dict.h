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
/**/

#include "object.h"
#include "util/lock.h"
#ifndef __INTELLISENSE__
#include "util/atomic.h"
#endif /* !__INTELLISENSE__ */
/**/

#include <hybrid/host.h> /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC */
#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_dict_item   dict_item
#define Dee_dict_object dict_object
#endif /* DEE_SOURCE */

typedef struct Dee_dict_object DeeDictObject;


/* Dict can remember insertion order:
 *
 * How:
 * - "d_vtab" keep track of insertion order
 * - Following "d_vtab" (iow: allocated within the same heap-block),
 *   there is a uint[8|16|32|64]_t vector "d_htab" of indices into
 *   "d_vtab", that is the actual hash-table (iow: _DeeDict_HashIdxAdv()
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
 * Also implement this for HashSet
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
#define di_key   _di_skv.di_key
#define di_value _di_skv.di_value
#endif /* __COMPILER_HAVE_TRANSPARENT_UNION */
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
		;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_di_ukv
#define di_key_and_value _di_ukv.di_key_and_value
#ifdef __COMPILER_HAVE_TRANSPARENT_STRUCT
#define di_key   _di_ukv.di_key
#define di_value _di_ukv.di_value
#else /* __COMPILER_HAVE_TRANSPARENT_STRUCT */
#define di_key   _di_ukv._di_skv.di_key
#define di_value _di_ukv._di_skv.di_value
#endif /* !__COMPILER_HAVE_TRANSPARENT_STRUCT */
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

/* Static initializer for `struct Dee_dict_item' */
#define Dee_DICT_ITEM_INIT(hash, key, value) { hash, { { key, value } } }

/* Index for "d_vtab" (real and virt version) */
typedef size_t Dee_dict_vidx_t;

typedef WUNUSED_T NONNULL_T((1)) /*virt*/Dee_dict_vidx_t (DCALL *Dee_dict_gethidx_t)(void const *__restrict htab, size_t index);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_sethidx_t)(void *__restrict htab, size_t index, /*virt*/Dee_dict_vidx_t value);

DFUNDEF WUNUSED NONNULL((1)) Dee_dict_vidx_t DCALL Dee_dict_gethidx8(void const *__restrict htab, size_t index);
DFUNDEF NONNULL((1)) void DCALL Dee_dict_sethidx8(void *__restrict htab, size_t index, Dee_dict_vidx_t value);

struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	/*real*/Dee_dict_vidx_t d_valloc;  /* [lock(d_lock)][<= d_hmask] Allocated size of "d_vtab" (should be ~2/3rd of `d_hmask + 1') */
	/*real*/Dee_dict_vidx_t d_vsize;   /* [lock(d_lock)][<= d_valloc] 1+ the greatest index in "d_vtab" that was ever initialized (and also the index of the next item in "d_vtab" to-be populated). */
	size_t                  d_vused;   /* [lock(d_lock)][<= d_vsize] # of non-NULL keys in "d_vtab". */
	struct Dee_dict_item   *d_vtab;    /* [lock(d_lock)][0..d_vsize][owned_if(!= INTERNAL(DeeDict_EmptyTab))]
	                                    * [OWNED_AT(. + 1)] Value-table (offset by 1 to account for special meaning of index==Dee_DICT_HTAB_EOF) */
	Dee_hash_t              d_hmask;   /* [lock(d_lock)] Hash mask (allocated hash-map size, minus 1). */
	Dee_dict_gethidx_t      d_hidxget; /* [lock(d_lock)][1..1] Getter for "d_htab" (always depends on "d_valloc") */
	Dee_dict_sethidx_t      d_hidxset; /* [lock(d_lock)][1..1] Setter for "d_htab" (always depends on "d_valloc") */
	void                   *d_htab;    /* [lock(d_lock)][== (byte_t *)(_DeeDict_GetRealVTab(this) + d_valloc)] Hash-table (contains indices into "d_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t     d_lock;    /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

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



#ifdef DEE_SOURCE

/* Helper macros for converting between "virt" and "real" dict VIDX indices. */
#define Dee_dict_vidx_virt2real(p_self) (void)(--*(p_self))
#define Dee_dict_vidx_real2virt(p_self) (void)(++*(p_self))
#define Dee_dict_vidx_toreal(self)      ((self) - 1)
#define Dee_dict_vidx_tovirt(self)      ((self) + 1)
#define Dee_dict_vidx_virt_lt_real(virt_self, real_count) ((virt_self) <= (real_count))
/*#define Dee_dict_vidx_virt_lt_real(virt_self, real_count) (Dee_dict_vidx_toreal(virt_self) < (real_count))*/

typedef NONNULL_T((1)) void (DCALL *Dee_dict_movhidx_t)(void *dst, void const *src, size_t n_words);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_uprhidx_t)(void *dst, void const *src, size_t n_words);
typedef NONNULL_T((1)) void (DCALL *Dee_dict_dwnhidx_t)(void *dst, void const *src, size_t n_words);
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

/* Dynamic dict I/O functions:
 * >> vtab = &Dee_dict_hidxio[DEE_DICT_HIDXIO_FROMALLOC(dict->d_valloc)];  */
DDATDEF struct Dee_dict_hidxio_struct Dee_tpconst Dee_dict_hidxio[DEE_DICT_HIDXIO_COUNT];

/* Index value found in "d_htab" when end-of-chain is encountered. */
#define Dee_DICT_HTAB_EOF 0

/* Get/set "d_vtab" in both its:
 * - virt[ual] (index starts at 1), and
 * - real (index starts at 0) form
 *
 * VIRT:
 * - Accepts indices in range "[Dee_dict_vidx_tovirt(0),Dee_dict_vidx_tovirt(d_vsize)-1)"  (aka: "[1,d_vsize]")
 * - These sort of indices are what is stored in `d_htab'. Indices
 *   start at 1, because an index=0 appearing in `d_htab' has the
 *   special meaning of `Dee_DICT_HTAB_EOF'
 *
 * REAL:
 * - Accepts indices in range "[0,d_vsize)"
 * - Actual, regular, 0-based indices
 * - _DeeDict_GetRealVTab() also represents the actual base of the
 *   heap-block holding the dict's tables. */
#define _DeeDict_GetVirtVTab(self)    ((self)->d_vtab)
#define _DeeDict_SetVirtVTab(self, v) (void)((self)->d_vtab = (v))
#define _DeeDict_GetRealVTab(self)    ((self)->d_vtab + 1)
#define _DeeDict_SetRealVTab(self, v) (void)((self)->d_vtab = (v) - 1)

/* Advance hash-index */
#define _DeeDict_HashIdxInit(self, p_hs, p_perturb, hash) \
	__DeeDict_HashIdxInitEx(p_hs, p_perturb, hash, (self)->d_hmask)
#define _DeeDict_HashIdxAdv(self, p_hs, p_perturb) \
	__DeeDict_HashIdxAdvEx(p_hs, p_perturb)

#define __DeeDict_HashIdxInitEx(p_hs, p_perturb, hash, hmask) \
	(void)(*(p_hs) = (*(p_perturb) = (hash)) & (hmask))
#define __DeeDict_HashIdxAdvEx(p_hs, p_perturb) \
	(void)(*(p_hs) = (*(p_hs) << 2) + *(p_hs) + *(p_perturb) + 1, *(p_perturb) >>= 5)

/* Get/set vtab-index "i" of htab at a given "hs" */
#define /*virt*/_DeeDict_HTabGet(self, hs)    (*(self)->d_hidxget)((self)->d_htab, (hs) & (self)->d_hmask)
#define _DeeDict_HTabSet(self, hs, /*virt*/i) (*(self)->d_hidxset)((self)->d_htab, (hs) & (self)->d_hmask, i)
#endif /* DEE_SOURCE */

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


#define DeeDict_GetItemString(self, key)                              DeeDict_GetItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_GetItemStringLen(self, key, keylen)                   DeeDict_GetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_GetItemStringHash(self, key, hash)                    (*DeeDict_Type.tp_seq->tp_getitem_string_hash)(self, key, hash)
#define DeeDict_GetItemStringLenHash(self, key, keylen, hash)         (*DeeDict_Type.tp_seq->tp_getitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_DelItemString(self, key)                              DeeDict_DelItemStringHash(self, key, Dee_HashStr(key))
#define DeeDict_DelItemStringLen(self, key, keylen)                   DeeDict_DelItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen))
#define DeeDict_DelItemStringHash(self, key, hash)                    (*DeeDict_Type.tp_seq->tp_delitem_string_hash)(self, key, hash)
#define DeeDict_DelItemStringLenHash(self, key, keylen, hash)         (*DeeDict_Type.tp_seq->tp_delitem_string_len_hash)(self, key, keylen, hash)
#define DeeDict_SetItemString(self, key, value)                       DeeDict_SetItemStringHash(self, key, Dee_HashStr(key), value)
#define DeeDict_SetItemStringLen(self, key, keylen, value)            DeeDict_SetItemStringLenHash(self, key, keylen, Dee_HashPtr(key, keylen), value)
#define DeeDict_SetItemStringHash(self, key, hash, value)             (*DeeDict_Type.tp_seq->tp_setitem_string_hash)(self, key, hash, value)
#define DeeDict_SetItemStringLenHash(self, key, keylen, hash, value)  (*DeeDict_Type.tp_seq->tp_setitem_string_len_hash)(self, key, keylen, hash, value)

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
#ifdef __INTELLISENSE__
#define DeeDict_SIZE_ATOMIC(self) DeeDict_SIZE(self)
#elif defined(__ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC) || defined(CONFIG_NO_THREADS)
#define DeeDict_SIZE_ATOMIC(self) Dee_atomic_read(&(self)->d_vused)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC || CONFIG_NO_THREADS */
LOCAL ATTR_PURE WUNUSED NONNULL((1)) size_t
(DeeDict_SIZE_ATOMIC)(DeeDictObject *__restrict self) {
	size_t result;
	DeeDict_LockRead(self);
	result = self->d_vused;
	DeeDict_LockEndRead(self);
	return result;
}
#endif /* !__ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC && !CONFIG_NO_THREADS */



DDATDEF DeeObject DeeDict_Dummy; /* DEPRECATED (no longer used by the dict impl) */

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
