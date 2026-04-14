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
#ifdef __INTELLISENSE__
#define DEFINE_dict_init_fromcopy
//#define DEFINE_dict_init_fromcopy_keysonly
//#define DEFINE_dict_init_fromhashset_keysonly
//#define DEFINE_hashset_init_fromcopy
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/dict.h>            /* DeeDict_LockEndRead, Dee_dict_item, _DeeDict_GetRealVTab, _DeeDict_SetRealVTab */
#include <deemon/hashset.h>         /* DeeHashSetObject, DeeHashSet_LockEndRead, Dee_hashset_item, _DeeHashSet_GetRealVTab, _DeeHashSet_SetRealVTab */
#include <deemon/object.h>          /* Dee_Incref, Dee_hash_t */
#include <deemon/system-features.h> /* memcpy* */
#include <deemon/util/hash-io.h>    /* Dee_HASH_HIDXIO_FROM_VALLOC, Dee_hash_* */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_init */

#include "dict-utils.h"

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* size_t */

#if (defined(DEFINE_dict_init_fromcopy) +             \
     defined(DEFINE_dict_init_fromcopy_keysonly) +    \
     defined(DEFINE_dict_init_fromhashset_keysonly) + \
     defined(DEFINE_hashset_init_fromcopy)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_dict_init_fromcopy
#define LOCAL_dict_init_fromcopy dict_init_fromcopy
#elif defined(DEFINE_dict_init_fromcopy_keysonly)
#define LOCAL_dict_init_fromcopy dict_init_fromcopy_keysonly
#define LOCAL_IS_KEYSONLY
#elif defined(DEFINE_dict_init_fromhashset_keysonly)
#define LOCAL_dict_init_fromcopy dict_init_fromhashset_keysonly
#define LOCAL_IS_KEYSONLY
#define LOCAL_IS_OTHER_HASHSET
#elif defined(DEFINE_hashset_init_fromcopy)
#define LOCAL_dict_init_fromcopy hashset_init_fromcopy
#define LOCAL_IS_KEYSONLY
#define LOCAL_IS_OTHER_HASHSET
#define LOCAL_IS_SELF_HASHSET
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

#ifdef LOCAL_IS_SELF_HASHSET
#ifdef __INTELLISENSE__
#include "hashset.c"
#endif /* __INTELLISENSE__ */
#define LOCAL_SelfType                                              DeeHashSetObject
#define LOCAL_Self_d_vsize                                          hs_vsize
#define LOCAL_Self_d_valloc                                         hs_valloc
#define LOCAL_Self_d_vused                                          hs_vused
#define LOCAL_Self_d_hmask                                          hs_hmask
#define LOCAL_Self_d_htab                                           hs_htab
#define LOCAL_Self_d_hidxops                                        hs_hidxops
#define LOCAL_Self_dict_item                                        struct Dee_hashset_item
#define LOCAL_Self_di_hash                                          hsi_hash
#define LOCAL_Self_di_key                                           hsi_key
#define LOCAL_Self_dict_hmask_from_count                            hashset_hmask_from_count
#define LOCAL_Self_dict_valloc_from_hmask_and_count                 hashset_valloc_from_hmask_and_count
#define LOCAL_Self_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio hashset_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_Self__DeeDict_TabsTryMalloc                           _DeeHashSet_TabsTryMalloc
#define LOCAL_Self__DeeDict_TabsMalloc                              _DeeHashSet_TabsMalloc
#define LOCAL_Self__DeeDict_TabsFree                                _DeeHashSet_TabsFree
#define LOCAL_Self__DeeDict_SetRealVTab                             _DeeHashSet_SetRealVTab
#define LOCAL_Self_dict_htab_rebuild_after_optimize                 hashset_htab_rebuild_after_optimize
#define LOCAL_Self_dict_verify                                      hashset_verify
#else /* LOCAL_IS_SELF_HASHSET */
#ifdef __INTELLISENSE__
#include "dict.c"
#endif /* __INTELLISENSE__ */
#define LOCAL_SelfType                                              Dict
#define LOCAL_Self_d_vsize                                          d_vsize
#define LOCAL_Self_d_valloc                                         d_valloc
#define LOCAL_Self_d_vused                                          d_vused
#define LOCAL_Self_d_hmask                                          d_hmask
#define LOCAL_Self_d_htab                                           d_htab
#define LOCAL_Self_d_hidxops                                        d_hidxops
#define LOCAL_Self_dict_item                                        struct Dee_dict_item
#define LOCAL_Self_di_hash                                          di_hash
#define LOCAL_Self_di_key                                           di_key
#define LOCAL_Self_dict_hmask_from_count                            dict_hmask_from_count
#define LOCAL_Self_dict_valloc_from_hmask_and_count                 dict_valloc_from_hmask_and_count
#define LOCAL_Self_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio dict_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_Self__DeeDict_TabsTryMalloc                           _DeeDict_TabsTryMalloc
#define LOCAL_Self__DeeDict_TabsMalloc                              _DeeDict_TabsMalloc
#define LOCAL_Self__DeeDict_TabsFree                                _DeeDict_TabsFree
#define LOCAL_Self__DeeDict_SetRealVTab                             _DeeDict_SetRealVTab
#define LOCAL_Self_dict_htab_rebuild_after_optimize                 dict_htab_rebuild_after_optimize
#define LOCAL_Self_dict_verify                                      dict_verify
#endif /* !LOCAL_IS_SELF_HASHSET */

#ifdef LOCAL_IS_OTHER_HASHSET
#define LOCAL_OtherType                 DeeHashSetObject
#define LOCAL_Other_LockReadAndOptimize DeeHashSet_LockReadAndOptimize
#define LOCAL_Other_LockEndRead         DeeHashSet_LockEndRead
#define LOCAL_Other__GetRealVTab        _DeeHashSet_GetRealVTab
#define LOCAL_Other_d_vsize             hs_vsize
#define LOCAL_Other_d_valloc            hs_valloc
#define LOCAL_Other_d_vused             hs_vused
#define LOCAL_Other_d_hmask             hs_hmask
#define LOCAL_Other_d_htab              hs_htab
#define LOCAL_Other_d_hidxops           hs_hidxops
#define LOCAL_Other_dict_item           struct Dee_hashset_item
#define LOCAL_Other_di_hash             hsi_hash
#define LOCAL_Other_di_key              hsi_key
#else /* LOCAL_IS_OTHER_HASHSET */
#define LOCAL_OtherType                 Dict
#define LOCAL_Other_LockReadAndOptimize DeeDict_LockReadAndOptimize
#define LOCAL_Other_LockEndRead         DeeDict_LockEndRead
#define LOCAL_Other__GetRealVTab        _DeeDict_GetRealVTab
#define LOCAL_Other_d_vsize             d_vsize
#define LOCAL_Other_d_valloc            d_valloc
#define LOCAL_Other_d_vused             d_vused
#define LOCAL_Other_d_hmask             d_hmask
#define LOCAL_Other_d_htab              d_htab
#define LOCAL_Other_d_hidxops           d_hidxops
#define LOCAL_Other_dict_item           struct Dee_dict_item
#define LOCAL_Other_di_hash             di_hash
#define LOCAL_Other_di_key              di_key
#endif /* !LOCAL_IS_OTHER_HASHSET */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromcopy(LOCAL_SelfType *__restrict self,
                         LOCAL_OtherType *__restrict other) {
	Dee_hash_t copy_hmask;
	Dee_hash_vidx_t copy_valloc;
	Dee_hash_vidx_t copy_vsize;
	Dee_hash_hidxio_t copy_hidxio;
	void *copy_tabs;
	size_t copy_tabssz;
	LOCAL_Self_dict_item *copy_vtab;
	union Dee_hash_htab *copy_htab;
again:
	LOCAL_Other_LockReadAndOptimize(other);

	/* Figure out size of the copied tables. */
	copy_vsize  = other->LOCAL_Other_d_vsize;
	copy_hmask  = LOCAL_Self_dict_hmask_from_count(copy_vsize);
	copy_valloc = LOCAL_Self_dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, true);
	if (copy_valloc >= other->LOCAL_Other_d_valloc) {
		copy_valloc = other->LOCAL_Other_d_valloc;
	} else {
		copy_valloc = LOCAL_Self_dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, false);
		if unlikely(copy_valloc > other->LOCAL_Other_d_valloc)
			copy_valloc = other->LOCAL_Other_d_valloc;
	}
	copy_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(copy_valloc);
	copy_tabssz = LOCAL_Self_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(copy_hmask, copy_valloc, copy_hidxio);
	copy_tabs   = LOCAL_Self__DeeDict_TabsTryMalloc(copy_tabssz);

	/* Try to allocate smaller tables if the first allocation failed. */
	if unlikely(!copy_tabs) {
		copy_valloc = LOCAL_Self_dict_valloc_from_hmask_and_count(copy_hmask, copy_vsize, false);
		if unlikely(copy_valloc > other->LOCAL_Other_d_valloc)
			copy_valloc = other->LOCAL_Other_d_valloc;
		copy_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(copy_valloc);
		copy_tabssz = LOCAL_Self_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(copy_hmask, copy_valloc, copy_hidxio);
		copy_tabs   = LOCAL_Self__DeeDict_TabsTryMalloc(copy_tabssz);
		if unlikely(!copy_tabs) {
			LOCAL_Other_LockEndRead(other);
			copy_tabs = LOCAL_Self__DeeDict_TabsMalloc(copy_tabssz);
			if unlikely(!copy_tabs)
				goto err;
			LOCAL_Other_LockReadAndOptimize(other);
			if unlikely(copy_vsize < other->LOCAL_Other_d_vsize) {
				LOCAL_Other_LockEndRead(other);
				LOCAL_Self__DeeDict_TabsFree(copy_tabs);
				goto again;
			}
		}
	}

	/* Copy data into copy's tables. */
	copy_vtab = (LOCAL_Self_dict_item *)copy_tabs;
	copy_htab = (union Dee_hash_htab *)(copy_vtab + copy_valloc);
#ifdef LOCAL_IS_KEYSONLY
	/* Create reference to copied objects. */
	{
		Dee_hash_vidx_t i, vsize = other->LOCAL_Other_d_vsize;
		LOCAL_Other_dict_item *orig_vtab;
		orig_vtab = LOCAL_Other__GetRealVTab(other);
		for (i = 0; i < vsize; ++i) {
			LOCAL_Self_dict_item *dst_item = &copy_vtab[i];
			LOCAL_Other_dict_item *src_item = &orig_vtab[i];
			dst_item->LOCAL_Self_di_hash = src_item->LOCAL_Other_di_hash;
			dst_item->LOCAL_Self_di_key  = src_item->LOCAL_Other_di_key;
			Dee_Incref(dst_item->LOCAL_Self_di_key);
#ifndef LOCAL_IS_SELF_HASHSET
			/* Value will be initialized by the caller! */
			DBG_memset(&dst_item->di_value, 0xcc, sizeof(dst_item->di_value));
#endif /* !LOCAL_IS_SELF_HASHSET */
		}
	}
#else /* LOCAL_IS_KEYSONLY */
	copy_vtab = (LOCAL_Self_dict_item *)memcpyc(copy_vtab, _DeeDict_GetRealVTab(other),
	                                            other->LOCAL_Other_d_vsize,
	                                            sizeof(LOCAL_Self_dict_item));
	/* Create reference to copied objects. */
	{
		Dee_hash_vidx_t i;
		for (i = 0; i < copy_vsize; ++i) {
			LOCAL_Self_dict_item *item;
			item = &copy_vtab[i];
			ASSERTF(item->di_key, "Table was optimized above, so there should be no deleted keys!");
			Dee_Incref(item->di_key);
			Dee_Incref(item->di_value);
		}
	}
#endif /* !LOCAL_IS_KEYSONLY */

	/* Fill in control structures. */
	self->LOCAL_Self_d_valloc  = copy_valloc;
	self->LOCAL_Self_d_vsize   = copy_vsize;
	self->LOCAL_Self_d_vused   = other->LOCAL_Other_d_vused;
	LOCAL_Self__DeeDict_SetRealVTab(self, copy_vtab);
	self->LOCAL_Self_d_hmask   = copy_hmask;
	self->LOCAL_Self_d_hidxops = &Dee_hash_hidxio[copy_hidxio];
	self->LOCAL_Self_d_htab    = copy_htab;

	if (copy_hmask == other->LOCAL_Other_d_hmask) {
		/* No need to re-build the hash table. Can instead copy is verbatim. */
		Dee_hash_hidxio_t orig_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(other->LOCAL_Other_d_valloc);
		Dee_hash_hidx_t htab_words = copy_hmask + 1;
		ASSERT(copy_hidxio <= orig_hidxio || (copy_hidxio + 1) == orig_hidxio);
		if (orig_hidxio == copy_hidxio) {
			size_t sizeof_htab = htab_words << copy_hidxio;
			(void)memcpy(copy_htab, other->LOCAL_Other_d_htab, sizeof_htab);
		} else if (copy_hidxio > orig_hidxio) {
			(*self->LOCAL_Self_d_hidxops->hxio_upr)(copy_htab, other->LOCAL_Other_d_htab, htab_words);
		} else if (copy_hidxio == orig_hidxio - 1) {
			(*self->LOCAL_Self_d_hidxops->hxio_lwr)(copy_htab, other->LOCAL_Other_d_htab, htab_words);
		} else {
			Dee_hash_hidx_t i;
			Dee_hash_sethidx_t setter = self->LOCAL_Self_d_hidxops->hxio_set;
			Dee_hash_gethidx_t getter = other->LOCAL_Other_d_hidxops->hxio_get;
			ASSERT(copy_hidxio < orig_hidxio);
			for (i = 0; i < htab_words; ++i)
				(*setter)(copy_htab, i, (*getter)(other->LOCAL_Other_d_htab, i));
		}
	} else {
		/* Copy has a different hash-mask -> hash table cannot be copied and needs to be re-build! */
		LOCAL_Self_dict_htab_rebuild_after_optimize(self);
	}
	LOCAL_Other_LockEndRead(other);
#if defined(HASHSET_INITFROM_NEEDSLOCK) && defined(LOCAL_IS_SELF_HASHSET)
	Dee_atomic_rwlock_init(&self->hs_lock);
#elif defined(DICT_INITFROM_NEEDSLOCK) && !defined(LOCAL_IS_SELF_HASHSET)
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* DICT_INITFROM_NEEDSLOCK */
	LOCAL_Self_dict_verify(self);
	return 0;
err:
	return -1;
}

#undef LOCAL_SelfType
#undef LOCAL_Self_d_vsize
#undef LOCAL_Self_d_valloc
#undef LOCAL_Self_d_vused
#undef LOCAL_Self_d_hmask
#undef LOCAL_Self_d_htab
#undef LOCAL_Self_d_hidxops
#undef LOCAL_Self_dict_item
#undef LOCAL_Self_di_hash
#undef LOCAL_Self_di_key
#undef LOCAL_Self_dict_hmask_from_count
#undef LOCAL_Self_dict_valloc_from_hmask_and_count
#undef LOCAL_Self_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio
#undef LOCAL_Self__DeeDict_TabsTryMalloc
#undef LOCAL_Self__DeeDict_TabsMalloc
#undef LOCAL_Self__DeeDict_TabsFree
#undef LOCAL_Self__DeeDict_SetRealVTab
#undef LOCAL_Self_dict_htab_rebuild_after_optimize
#undef LOCAL_Self_dict_verify

#undef LOCAL_OtherType
#undef LOCAL_Other_LockReadAndOptimize
#undef LOCAL_Other_LockEndRead
#undef LOCAL_Other__GetRealVTab
#undef LOCAL_Other_d_vsize
#undef LOCAL_Other_d_valloc
#undef LOCAL_Other_d_vused
#undef LOCAL_Other_d_hmask
#undef LOCAL_Other_d_htab
#undef LOCAL_Other_d_hidxops
#undef LOCAL_Other_dict_item
#undef LOCAL_Other_di_hash
#undef LOCAL_Other_di_key

#undef LOCAL_IS_KEYSONLY
#undef LOCAL_IS_OTHER_HASHSET
#undef LOCAL_IS_SELF_HASHSET
#undef LOCAL_dict_init_fromcopy

DECL_END

#undef DEFINE_dict_init_fromcopy
#undef DEFINE_dict_init_fromcopy_keysonly
#undef DEFINE_dict_init_fromhashset_keysonly
#undef DEFINE_hashset_init_fromcopy
