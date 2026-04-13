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
#define DEFINE_DeeDict
//#define DEFINE_DeeHashSet
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>

#include "dict-utils.h"
#include "../runtime/kwlist.h"

#if (defined(DEFINE_DeeDict) + \
     defined(DEFINE_DeeHashSet)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

#ifdef __INTELLISENSE__
#ifdef DEFINE_DeeDict
#include "dict.c"
#endif /* DEFINE_DeeDict */
#ifdef DEFINE_DeeHashSet
#include "hashset.c"
#endif /* DEFINE_DeeHashSet */
#endif /* __INTELLISENSE__ */


#ifdef DEFINE_DeeDict
#define LOCAL_dict_new_with_hint                               dict_new_with_hint
#define LOCAL_DeeDict_TryNewWithHint                           DeeDict_TryNewWithHint
#define LOCAL_DeeDict_TryNewWithWeakHint                       DeeDict_TryNewWithWeakHint
#define LOCAL_DeeDict_NewWithHint                              DeeDict_NewWithHint
#define LOCAL_DeeDict_NewWithWeakHint                          DeeDict_NewWithWeakHint
#define LOCAL_dict_htab_rebuild_after_optimize                 dict_htab_rebuild_after_optimize
#define LOCAL_dict_do_optimize_vtab_without_rebuild            dict_do_optimize_vtab_without_rebuild
#define LOCAL_dict_optimize_vtab                               dict_optimize_vtab
#define LOCAL_dict_htab_rebuild                                dict_htab_rebuild
#define LOCAL_dict_trygrow_vtab                                dict_trygrow_vtab
#define LOCAL_dict_trygrow_vtab_and_htab_with                  dict_trygrow_vtab_and_htab_with
#define LOCAL_dict_trygrow_vtab_and_htab                       dict_trygrow_vtab_and_htab
#define LOCAL_dict_trygrow_htab_and_maybe_vtab                 dict_trygrow_htab_and_maybe_vtab
#define LOCAL_dict_grow_vtab_and_htab_and_relock               dict_grow_vtab_and_htab_and_relock
#define LOCAL_dict_grow_vtab_and_htab_and_relock_impl          dict_grow_vtab_and_htab_and_relock_impl
#define LOCAL_dict_grow_htab_and_relock                        dict_grow_htab_and_relock
#define LOCAL_dict_shrink_htab                                 dict_shrink_htab
#define LOCAL_dict_shrink_vtab_and_htab                        dict_shrink_vtab_and_htab
#define LOCAL_dict_autoshrink                                  dict_autoshrink
#define LOCAL_dict_makespace_at_impl                           dict_makespace_at_impl
#define LOCAL_dict_makespace_at                                dict_makespace_at
/*...................................................................................................*/
#define LOCAL_dict_seq_erase_data                              dict_seq_erase_data
#define LOCAL_dict_seq_erase_data_init                         dict_seq_erase_data_init
#define LOCAL_dict_mh_seq_erase                                dict_mh_seq_erase
#define LOCAL_dict_items_reverse                               dict_items_reverse
#define LOCAL_dict_mh_seq_reverse                              dict_mh_seq_reverse
#define LOCAL_dict_shrink_impl                                 dict_shrink_impl
#define LOCAL_dict_shrink                                      dict_shrink
#define LOCAL_dict_reserve                                     dict_reserve
#define LOCAL_dict_cc                                          dict_cc
#define LOCAL_dict_sizeof                                      dict_sizeof
#define LOCAL_dict_compare_seq_foreach_data                    dict_compare_seq_foreach_data
#define LOCAL_dict_compare_seq_foreach                         dict_compare_seq_foreach
#define LOCAL_dict_compare_eq_seq_foreach                      dict_compare_eq_seq_foreach
#define LOCAL_dict_mh_seq_compare                              dict_mh_seq_compare
#define LOCAL_dict_mh_seq_compare_eq                           dict_mh_seq_compare_eq
#define LOCAL_dict_mh_seq_trycompare_eq                        dict_mh_seq_trycompare_eq
/*...................................................................................................*/
#define LOCAL_Dict                                             Dict
#define LOCAL__DeeDict_TabsMalloc                              _DeeDict_TabsMalloc
#define LOCAL__DeeDict_TabsCalloc                              _DeeDict_TabsCalloc
#define LOCAL__DeeDict_TabsRealloc                             _DeeDict_TabsRealloc
#define LOCAL__DeeDict_TabsTryMalloc                           _DeeDict_TabsTryMalloc
#define LOCAL__DeeDict_TabsTryCalloc                           _DeeDict_TabsTryCalloc
#define LOCAL__DeeDict_TabsTryRealloc                          _DeeDict_TabsTryRealloc
#define LOCAL__DeeDict_TabsFree                                _DeeDict_TabsFree
#define LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL                  NULL_IF__DeeDict_EmptyVTab_REAL
#define LOCAL_Dee_dict_item                                    Dee_dict_item
#define LOCAL_DeeDict_Type                                     DeeDict_Type
#define LOCAL__DeeDict_ShouldGrowHTab                          _DeeDict_ShouldGrowHTab
#define LOCAL__DeeDict_MustGrowHTab                            _DeeDict_MustGrowHTab
#define LOCAL__DeeDict_ShouldOptimizeVTab                      _DeeDict_ShouldOptimizeVTab
#define LOCAL__DeeDict_CanOptimizeVTab                         _DeeDict_CanOptimizeVTab
#define LOCAL__DeeDict_MustGrowVTab                            _DeeDict_MustGrowVTab
#define LOCAL__DeeDict_CanGrowVTab                             _DeeDict_CanGrowVTab
#define LOCAL__DeeDict_ShouldShrinkVTab                        _DeeDict_ShouldShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC               _DeeDict_ShouldShrinkVTab_NEWALLOC
#define LOCAL__DeeDict_CanShrinkVTab                           _DeeDict_CanShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkHTab                        _DeeDict_ShouldShrinkHTab
#define LOCAL__DeeDict_CanShrinkHTab                           _DeeDict_CanShrinkHTab
#define LOCAL_DICT_FROMSEQ_DEFAULT_HINT                        DICT_FROMSEQ_DEFAULT_HINT
#define LOCAL_dict_suggested_max_valloc_from_count             dict_suggested_max_valloc_from_count
#define LOCAL_dict_valloc_from_hmask_and_count                 dict_valloc_from_hmask_and_count
#define LOCAL_dict_hmask_from_count                            dict_hmask_from_count
#define LOCAL_dict_tiny_hmask_from_count                       dict_tiny_hmask_from_count
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc            dict_sizeoftabs_from_hmask_and_valloc
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio dict_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_DeeDict_LockReadAndOptimize                      DeeDict_LockReadAndOptimize
#define LOCAL__DeeDict_GetVirtVTab                             _DeeDict_GetVirtVTab
#define LOCAL__DeeDict_SetVirtVTab                             _DeeDict_SetVirtVTab
#define LOCAL__DeeDict_GetRealVTab                             _DeeDict_GetRealVTab
#define LOCAL__DeeDict_SetRealVTab                             _DeeDict_SetRealVTab
#define LOCAL__DeeDict_HashIdxInit                             _DeeDict_HashIdxInit
#define LOCAL__DeeDict_HashIdxNext                             _DeeDict_HashIdxNext
#define LOCAL__DeeDict_HTabGet                                 _DeeDict_HTabGet
#define LOCAL__DeeDict_HTabSet                                 _DeeDict_HTabSet
#define LOCAL_DeeDict_LockReading                              DeeDict_LockReading
#define LOCAL_DeeDict_LockWriting                              DeeDict_LockWriting
#define LOCAL_DeeDict_LockTryRead                              DeeDict_LockTryRead
#define LOCAL_DeeDict_LockTryWrite                             DeeDict_LockTryWrite
#define LOCAL_DeeDict_LockRead                                 DeeDict_LockRead
#define LOCAL_DeeDict_LockWrite                                DeeDict_LockWrite
#define LOCAL_DeeDict_LockTryUpgrade                           DeeDict_LockTryUpgrade
#define LOCAL_DeeDict_LockUpgrade                              DeeDict_LockUpgrade
#define LOCAL_DeeDict_LockDowngrade                            DeeDict_LockDowngrade
#define LOCAL_DeeDict_LockEndWrite                             DeeDict_LockEndWrite
#define LOCAL_DeeDict_LockEndRead                              DeeDict_LockEndRead
#define LOCAL_DeeDict_LockEnd                                  DeeDict_LockEnd
#define LOCAL_DeeDict_SIZE                                     DeeDict_SIZE
#define LOCAL_DeeDict_SIZE_ATOMIC                              DeeDict_SIZE_ATOMIC
#define LOCAL_DeeDict_EmptyVTab                                DeeDict_EmptyVTab
#define LOCAL_DeeDict_EmptyHTab                                DeeDict_EmptyHTab
#define LOCAL_dict_htab_decafter                               dict_htab_decafter
#define LOCAL_dict_htab_incafter                               dict_htab_incafter
#define LOCAL_dict_htab_decrange                               dict_htab_decrange
#define LOCAL_dict_htab_incrange                               dict_htab_incrange
#define LOCAL_dict_htab_reverse                                dict_htab_reverse
#elif defined(DEFINE_DeeHashSet)
#define LOCAL_dict_new_with_hint                               hashset_new_with_hint
#define LOCAL_DeeDict_TryNewWithHint                           DeeHashSet_TryNewWithHint
#define LOCAL_DeeDict_TryNewWithWeakHint                       DeeHashSet_TryNewWithWeakHint
#define LOCAL_DeeDict_NewWithHint                              DeeHashSet_NewWithHint
#define LOCAL_DeeDict_NewWithWeakHint                          DeeHashSet_NewWithWeakHint
#define LOCAL_dict_htab_rebuild_after_optimize                 hashset_htab_rebuild_after_optimize
#define LOCAL_dict_do_optimize_vtab_without_rebuild            hashset_do_optimize_vtab_without_rebuild
#define LOCAL_dict_optimize_vtab                               hashset_optimize_vtab
#define LOCAL_dict_htab_rebuild                                hashset_htab_rebuild
#define LOCAL_dict_trygrow_vtab                                hashset_trygrow_vtab
#define LOCAL_dict_trygrow_vtab_and_htab_with                  hashset_trygrow_vtab_and_htab_with
#define LOCAL_dict_trygrow_vtab_and_htab                       hashset_trygrow_vtab_and_htab
#define LOCAL_dict_trygrow_htab_and_maybe_vtab                 hashset_trygrow_htab_and_maybe_vtab
#define LOCAL_dict_grow_vtab_and_htab_and_relock               hashset_grow_vtab_and_htab_and_relock
#define LOCAL_dict_grow_vtab_and_htab_and_relock_impl          hashset_grow_vtab_and_htab_and_relock_impl
#define LOCAL_dict_grow_htab_and_relock                        hashset_grow_htab_and_relock
#define LOCAL_dict_shrink_htab                                 hashset_shrink_htab
#define LOCAL_dict_shrink_vtab_and_htab                        hashset_shrink_vtab_and_htab
#define LOCAL_dict_autoshrink                                  hashset_autoshrink
#define LOCAL_dict_makespace_at_impl                           hashset_makespace_at_impl
#define LOCAL_dict_makespace_at                                hashset_makespace_at
/*...................................................................................................*/
#define LOCAL_dict_seq_erase_data                              hashset_seq_erase_data
#define LOCAL_dict_seq_erase_data_init                         hashset_seq_erase_data_init
#define LOCAL_dict_mh_seq_erase                                hashset_mh_seq_erase
#define LOCAL_dict_items_reverse                               hashset_items_reverse
#define LOCAL_dict_mh_seq_reverse                              hashset_mh_seq_reverse
#define LOCAL_dict_shrink_impl                                 hashset_shrink_impl
#define LOCAL_dict_shrink                                      hashset_shrink
#define LOCAL_dict_reserve                                     hashset_reserve
#define LOCAL_dict_cc                                          hashset_cc
#define LOCAL_dict_sizeof                                      hashset_sizeof
#define LOCAL_dict_compare_seq_foreach_data                    hashset_compare_seq_foreach_data
#define LOCAL_dict_compare_seq_foreach                         hashset_compare_seq_foreach
#define LOCAL_dict_compare_eq_seq_foreach                      hashset_compare_eq_seq_foreach
#define LOCAL_dict_mh_seq_compare                              hashset_mh_seq_compare
#define LOCAL_dict_mh_seq_compare_eq                           hashset_mh_seq_compare_eq
#define LOCAL_dict_mh_seq_trycompare_eq                        hashset_mh_seq_trycompare_eq
/*...................................................................................................*/
#define LOCAL_Dict                                             HashSet
#define LOCAL__DeeDict_TabsMalloc                              _DeeHashSet_TabsMalloc
#define LOCAL__DeeDict_TabsCalloc                              _DeeHashSet_TabsCalloc
#define LOCAL__DeeDict_TabsRealloc                             _DeeHashSet_TabsRealloc
#define LOCAL__DeeDict_TabsTryMalloc                           _DeeHashSet_TabsTryMalloc
#define LOCAL__DeeDict_TabsTryCalloc                           _DeeHashSet_TabsTryCalloc
#define LOCAL__DeeDict_TabsTryRealloc                          _DeeHashSet_TabsTryRealloc
#define LOCAL__DeeDict_TabsFree                                _DeeHashSet_TabsFree
#define LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL                  NULL_IF__DeeHashSet_EmptyVTab_REAL
#define LOCAL_Dee_dict_item                                    Dee_hashset_item
#define LOCAL_DeeDict_Type                                     DeeHashSet_Type
#define LOCAL__DeeDict_ShouldGrowHTab                          _DeeHashSet_ShouldGrowHTab
#define LOCAL__DeeDict_MustGrowHTab                            _DeeHashSet_MustGrowHTab
#define LOCAL__DeeDict_ShouldOptimizeVTab                      _DeeHashSet_ShouldOptimizeVTab
#define LOCAL__DeeDict_CanOptimizeVTab                         _DeeHashSet_CanOptimizeVTab
#define LOCAL__DeeDict_MustGrowVTab                            _DeeHashSet_MustGrowVTab
#define LOCAL__DeeDict_CanGrowVTab                             _DeeHashSet_CanGrowVTab
#define LOCAL__DeeDict_ShouldShrinkVTab                        _DeeHashSet_ShouldShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC               _DeeHashSet_ShouldShrinkVTab_NEWALLOC
#define LOCAL__DeeDict_CanShrinkVTab                           _DeeHashSet_CanShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkHTab                        _DeeHashSet_ShouldShrinkHTab
#define LOCAL__DeeDict_CanShrinkHTab                           _DeeHashSet_CanShrinkHTab
#define LOCAL_DICT_FROMSEQ_DEFAULT_HINT                        DICT_FROMSEQ_DEFAULT_HINT
#define LOCAL_dict_suggested_max_valloc_from_count             hashset_suggested_max_valloc_from_count
#define LOCAL_dict_valloc_from_hmask_and_count                 hashset_valloc_from_hmask_and_count
#define LOCAL_dict_hmask_from_count                            hashset_hmask_from_count
#define LOCAL_dict_tiny_hmask_from_count                       hashset_tiny_hmask_from_count
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc            hashset_sizeoftabs_from_hmask_and_valloc
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio hashset_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_DeeDict_LockReadAndOptimize                      DeeHashSet_LockReadAndOptimize
#define LOCAL__DeeDict_GetVirtVTab                             _DeeHashSet_GetVirtVTab
#define LOCAL__DeeDict_SetVirtVTab                             _DeeHashSet_SetVirtVTab
#define LOCAL__DeeDict_GetRealVTab                             _DeeHashSet_GetRealVTab
#define LOCAL__DeeDict_SetRealVTab                             _DeeHashSet_SetRealVTab
#define LOCAL__DeeDict_HashIdxInit                             _DeeHashSet_HashIdxInit
#define LOCAL__DeeDict_HashIdxNext                             _DeeHashSet_HashIdxNext
#define LOCAL__DeeDict_HTabGet                                 _DeeHashSet_HTabGet
#define LOCAL__DeeDict_HTabSet                                 _DeeHashSet_HTabSet
#define LOCAL_DeeDict_LockReading                              DeeHashSet_LockReading
#define LOCAL_DeeDict_LockWriting                              DeeHashSet_LockWriting
#define LOCAL_DeeDict_LockTryRead                              DeeHashSet_LockTryRead
#define LOCAL_DeeDict_LockTryWrite                             DeeHashSet_LockTryWrite
#define LOCAL_DeeDict_LockRead                                 DeeHashSet_LockRead
#define LOCAL_DeeDict_LockWrite                                DeeHashSet_LockWrite
#define LOCAL_DeeDict_LockTryUpgrade                           DeeHashSet_LockTryUpgrade
#define LOCAL_DeeDict_LockUpgrade                              DeeHashSet_LockUpgrade
#define LOCAL_DeeDict_LockDowngrade                            DeeHashSet_LockDowngrade
#define LOCAL_DeeDict_LockEndWrite                             DeeHashSet_LockEndWrite
#define LOCAL_DeeDict_LockEndRead                              DeeHashSet_LockEndRead
#define LOCAL_DeeDict_LockEnd                                  DeeHashSet_LockEnd
#define LOCAL_DeeDict_SIZE                                     DeeHashSet_SIZE
#define LOCAL_DeeDict_SIZE_ATOMIC                              DeeHashSet_SIZE_ATOMIC
#define LOCAL_DeeDict_EmptyVTab                                DeeHashSet_EmptyVTab
#define LOCAL_DeeDict_EmptyHTab                                DeeHashSet_EmptyHTab
#define LOCAL_dict_htab_decafter                               hashset_htab_decafter
#define LOCAL_dict_htab_incafter                               hashset_htab_incafter
#define LOCAL_dict_htab_decrange                               hashset_htab_decrange
#define LOCAL_dict_htab_incrange                               hashset_htab_incrange
#define LOCAL_dict_htab_reverse                                hashset_htab_reverse


/* Map dict fields to hashsets */
#define d_valloc  hs_valloc
#define d_vsize   hs_vsize
#define d_vused   hs_vused
#define d_vtab    hs_vtab
#define d_hmask   hs_hmask
#define d_hidxops hs_hidxops
#define d_htab    hs_htab
#define d_lock    hs_lock
#define di_hash   hsi_hash
#define di_key    hsi_key

#ifdef __INTELLISENSE__
#define Dee_dict_item ::BAD_DONT_USE_DIRECTLY
#endif /* __INTELLISENSE__ */
#endif /* DEFINE_DeeHashSet */

DECL_BEGIN

#ifdef DEFINE_DeeHashSet
#define LOCAL_OBJECTS_PER_ITEM 1
#else /* DEFINE_DeeHashSet */
#define LOCAL_OBJECTS_PER_ITEM 2
#endif /* !DEFINE_DeeHashSet */

/************************************************************************/
/************************************************************************/
/* LOW-LEVEL API                                                        */
/************************************************************************/
/************************************************************************/

PRIVATE WUNUSED DREF LOCAL_Dict *DCALL
LOCAL_dict_new_with_hint(size_t num_items, bool tryalloc, bool allow_overalloc) {
	DREF LOCAL_Dict *result;
	Dee_hash_t hmask;
	Dee_hash_vidx_t valloc;
	Dee_hash_hidxio_t hidxio;
	void *tabs;
	if unlikely(!num_items) {
		hmask  = 0;
		valloc = 0;
		hidxio = 0;
		tabs   = (void *)_DeeHash_EmptyTab;
	} else {
		size_t tabssz;
		hmask  = LOCAL_dict_hmask_from_count(num_items);
		valloc = LOCAL_dict_valloc_from_hmask_and_count(hmask, num_items, allow_overalloc);
		hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(valloc);
		tabssz = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(hmask, valloc, hidxio);
		tabs   = tryalloc ? LOCAL__DeeDict_TabsTryCalloc(tabssz)
		                  : LOCAL__DeeDict_TabsCalloc(tabssz);
		if unlikely(!tabs)
			goto err;
	}
	result = tryalloc ? DeeGCObject_TRYMALLOC(LOCAL_Dict)
	                  : DeeGCObject_MALLOC(LOCAL_Dict);
	if unlikely(!result)
		goto err_tabs;
	result->d_valloc  = valloc;
	result->d_vsize   = 0;
	result->d_vused   = 0;
	_DeeDict_SetRealVTab(result, (struct LOCAL_Dee_dict_item *)tabs);
	result->d_hmask   = hmask;
	result->d_hidxops = &Dee_hash_hidxio[hidxio];
	result->d_htab    = (union Dee_hash_htab *)((struct LOCAL_Dee_dict_item *)tabs + valloc);
	Dee_atomic_rwlock_init(&result->d_lock);
	Dee_weakref_support_init(result);
	DeeObject_InitStatic(result, &LOCAL_DeeDict_Type);
	result = DeeGC_TRACK(LOCAL_Dict, result);
	return result;
err_tabs:
	if likely(num_items)
		LOCAL__DeeDict_TabsFree(tabs);
err:
	return NULL;
}

PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_TryNewWithHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, true, false));
}

PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_TryNewWithWeakHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, true, true));
}

PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_NewWithHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, false, false));
}

PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_NewWithWeakHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, false, true));
}



/* Re-build the dict's "d_htab" (allowed to assume that "d_vtab" does not contain deleted keys) */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_rebuild_after_optimize(LOCAL_Dict *__restrict self) {
	struct LOCAL_Dee_dict_item *vtab = LOCAL__DeeDict_GetVirtVTab(self);
	union Dee_hash_htab *htab  = self->d_htab;
	Dee_hash_vidx_t i, vsize = self->d_vsize;
	Dee_hash_t hmask = self->d_hmask;
	struct Dee_hash_hidxio_ops const *ops = self->d_hidxops;
	ASSERT(vsize == self->d_vused);
	(*ops->hxio_zro)(htab, hmask + 1);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, vsize); ++i) {
		(*ops->hxio_insert)(htab, hmask, vtab[i].di_hash, i);
	}
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_do_optimize_vtab_without_rebuild(LOCAL_Dict *__restrict self) {
	/*real*/ Dee_hash_vidx_t i;
	struct LOCAL_Dee_dict_item *vtab;
	vtab = LOCAL__DeeDict_GetRealVTab(self);
	for (i = 0; i < self->d_vsize;) {
		/*real*/ Dee_hash_vidx_t j;
		Dee_hash_vidx_t delta;
		if (vtab[i].di_key) {
			++i;
			continue;
		}
		delta = 1;
		for (j = i;;) {
			++j;
			if (j >= self->d_vsize)
				break;
			if (vtab[j].di_key)
				break;
			++delta;
		}
		memmovedownc(&vtab[i], &vtab[j], self->d_vsize - j,
		             sizeof(struct LOCAL_Dee_dict_item));
		self->d_vsize -= delta;
#ifndef __OPTIMIZE_SIZE__
		if (self->d_vsize <= self->d_vused)
			break; /* Fully optimized -> can stop early */
#endif /* !__OPTIMIZE_SIZE__ */
	}
	ASSERT(self->d_vsize == self->d_vused);
	ASSERT(self->d_vsize < self->d_valloc);
}

INTERN NONNULL((1)) void DCALL
LOCAL_dict_optimize_vtab(LOCAL_Dict *__restrict self) {
	ASSERT(_DeeDict_CanOptimizeVTab(self));
	LOCAL_dict_do_optimize_vtab_without_rebuild(self);
	LOCAL_dict_htab_rebuild_after_optimize(self);
}

PRIVATE NONNULL((1)) void DCALL
LOCAL_dict_htab_rebuild(LOCAL_Dict *__restrict self) {
	if (LOCAL__DeeDict_CanOptimizeVTab(self))
		LOCAL_dict_do_optimize_vtab_without_rebuild(self);
	LOCAL_dict_htab_rebuild_after_optimize(self);
}

/* Try to make it so "d_vsize < d_valloc" by enlarging the vector.
 * Do this while the caller is holding a write-lock to "self", and
 * do so without ever releasing that lock.
 * NOTES:
 * - This function will NEVER rehash the dict or change the contents of d_htab!
 * - The caller must ensure that `_DeeDict_CanGrowVTab(self)' is true
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
LOCAL_dict_trygrow_vtab(LOCAL_Dict *__restrict self) {
	Dee_hash_vidx_t new_valloc;
	size_t new_tabsize;
	Dee_hash_hidxio_t old_hidxio;
	Dee_hash_hidxio_t new_hidxio;
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	union Dee_hash_htab *old_htab;
	union Dee_hash_htab *new_htab;

	/* Must truly allocate a new, larger v-table */
	ASSERT(_DeeDict_CanGrowVTab(self));
	new_valloc = dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, true);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	ASSERT(self->d_hidxops == &Dee_hash_hidxio[old_hidxio]);
	new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
	old_vtab = _DeeDict_GetRealVTab(self);
	old_vtab = LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL(old_vtab);
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		new_valloc = dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, false);
		old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
		new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_tabsize = dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
		new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			return false;
	}
	old_htab = (union Dee_hash_htab *)(new_vtab + self->d_valloc);
	new_htab = (union Dee_hash_htab *)(new_vtab + new_valloc);
	if likely(old_hidxio == new_hidxio) {
		(*self->d_hidxops->hxio_movup)(new_htab, old_htab, self->d_hmask + 1);
	} else {
		(*self->d_hidxops->hxio_upr)(new_htab, old_htab, self->d_hmask + 1);
		self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
	}
	bzeroc(new_vtab + self->d_valloc,
	       new_valloc - self->d_valloc,
	       sizeof(struct LOCAL_Dee_dict_item));
	_DeeDict_SetRealVTab(self, new_vtab);
	self->d_valloc = new_valloc;
	self->d_htab   = new_htab;
	return true;
}

/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeDict_CanGrowVTab(self)"
 * Tries to make it so "d_valloc >= min_valloc"
 * @return: true:  Success: "d_valloc >= min_valloc"
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE NONNULL((1)) bool DCALL
LOCAL_dict_trygrow_vtab_and_htab_with(LOCAL_Dict *__restrict self,
                                      Dee_hash_vidx_t min_valloc,
                                      bool allow_overalloc) {
	Dee_hash_t old_hmask;
	Dee_hash_t new_hmask;
#ifndef NDEBUG
	Dee_hash_vidx_t old_valloc;
#endif /* !NDEBUG */
	Dee_hash_vidx_t new_valloc;
	size_t new_tabsize;
	Dee_hash_hidxio_t old_hidxio;
	Dee_hash_hidxio_t new_hidxio;
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	union Dee_hash_htab *old_htab;
	union Dee_hash_htab *new_htab;
	ASSERT(min_valloc > self->d_valloc);

	/* Must truly allocate a new, larger v-table */
	old_hmask = self->d_hmask;
	new_hmask = LOCAL_dict_hmask_from_count(min_valloc);
	if unlikely(new_hmask < old_hmask)
		new_hmask = old_hmask;
	if (new_hmask <= old_hmask && LOCAL__DeeDict_ShouldGrowHTab(self))
		new_hmask = (new_hmask << 1) | 1;
	new_valloc = LOCAL_dict_valloc_from_hmask_and_count(new_hmask, min_valloc, allow_overalloc);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	ASSERT(self->d_hidxops == &Dee_hash_hidxio[old_hidxio]);
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	old_vtab = LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL(old_vtab);
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		new_hmask  = LOCAL_dict_tiny_hmask_from_count(min_valloc);
		new_valloc = LOCAL_dict_valloc_from_hmask_and_count(new_hmask, min_valloc, false);
		new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
		new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			return false;
	}
	ASSERT(new_valloc >= min_valloc);
	old_htab = (union Dee_hash_htab *)(new_vtab + self->d_valloc);
	new_htab = (union Dee_hash_htab *)(new_vtab + new_valloc);
	_DeeDict_SetRealVTab(self, new_vtab);
#ifndef NDEBUG
	old_valloc = self->d_valloc;
#endif /* !NDEBUG */
	self->d_valloc = new_valloc;
	self->d_htab   = new_htab;
	if (old_hmask == new_hmask) {
		if likely(old_hidxio == new_hidxio) {
			(*self->d_hidxops->hxio_movup)(new_htab, old_htab, old_hmask + 1);
		} else {
			(*self->d_hidxops->hxio_upr)(new_htab, old_htab, old_hmask + 1);
			self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
		}
	} else {
		/* Must rebuild d_htab */
		self->d_hmask   = new_hmask;
		self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
		LOCAL_dict_htab_rebuild(self);
	}
#ifndef NDEBUG
	DBG_memset(new_vtab + old_valloc, 0xcc,
	           (new_valloc - old_valloc) *
	           sizeof(struct LOCAL_Dee_dict_item));
#endif /* !NDEBUG */
	return true;
}


/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeDict_CanGrowVTab(self)"
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
LOCAL_dict_trygrow_vtab_and_htab(LOCAL_Dict *__restrict self) {
	return LOCAL_dict_trygrow_vtab_and_htab_with(self, self->d_vsize + 1, true);
}

#if 0
/* Try to change "d_hmask = (d_hmask << 1) | 1",
 * and (if we want to), also increase "d_valloc"
 * @return: true:  Success
 * @return: false: Failure (allocation failed) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
LOCAL_dict_trygrow_htab_and_maybe_vtab(LOCAL_Dict *__restrict self) {
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	size_t new_tabsize;
	Dee_hash_t new_hmask;
	Dee_hash_vidx_t old_valloc = self->d_valloc;
	Dee_hash_vidx_t new_valloc;
	Dee_hash_hidxio_t hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(old_valloc);
	ASSERTF(self->d_hmask != (Dee_hash_t)-1, "How? This should have been an OOM");
	new_hmask  = (self->d_hmask << 1) | 1;
	new_valloc = (new_hmask / DICT_VTAB_HTAB_RATIO_H) * DICT_VTAB_HTAB_RATIO_V;
	if (new_valloc < old_valloc)
		new_valloc = old_valloc;
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, hidxio);

	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	old_vtab = LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL(old_vtab);
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		/* Fine... We won't grow the vtab if you wanna be that way... */
		new_valloc  = old_valloc;
		new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, hidxio);
		new_vtab    = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
		if unlikely(!new_vtab)
			goto err;
	}
	self->d_valloc = new_valloc;
	self->d_hmask  = new_hmask;
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab = (union Dee_hash_htab *)(new_vtab + new_valloc);
	LOCAL_dict_htab_rebuild(self);
	return true;
err:
	return false;
}
#endif

#undef LOCAL_HAVE_dict_setitem_unlocked
#if ((defined(DEFINE_DeeDict) && defined(HAVE_dict_setitem_unlocked)) || \
     (defined(DEFINE_DeeHashSet) && defined(HAVE_hashset_insert_unlocked)))
#define LOCAL_HAVE_dict_setitem_unlocked
#endif /* ... */

/* Make it so "!_DeeDict_MustGrowVTab(self)"
 * (aka: " d_vsize < d_valloc && d_valloc <= d_hmask")
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
#ifdef LOCAL_HAVE_dict_setitem_unlocked
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_grow_vtab_and_htab_and_relock(LOCAL_Dict *__restrict self, bool without_locks)
#else /* LOCAL_HAVE_dict_setitem_unlocked */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_grow_vtab_and_htab_and_relock_impl(LOCAL_Dict *__restrict self)
#ifdef DEFINE_DeeDict
#define dict_grow_vtab_and_htab_and_relock(self, without_locks) dict_grow_vtab_and_htab_and_relock_impl(self)
#endif /* DEFINE_DeeDict */
#ifdef DEFINE_DeeHashSet
#define hashset_grow_vtab_and_htab_and_relock(self, without_locks) hashset_grow_vtab_and_htab_and_relock_impl(self)
#endif /* DEFINE_DeeHashSet */
#endif /* !LOCAL_HAVE_dict_setitem_unlocked */
{
#ifdef LOCAL_HAVE_dict_setitem_unlocked
#define IF_with_locks(x) if (!without_locks) x
#else /* LOCAL_HAVE_dict_setitem_unlocked */
#define IF_with_locks(x) x
#endif /* !LOCAL_HAVE_dict_setitem_unlocked */
	Dee_hash_vidx_t new_valloc;
	Dee_hash_t old_hmask;
	Dee_hash_t new_hmask;
	size_t new_tabsize;
	Dee_hash_hidxio_t new_hidxio;
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
again:
#ifdef LOCAL_HAVE_dict_setitem_unlocked
	ASSERT(without_locks || LOCAL_DeeDict_LockWriting(self));
#else /* LOCAL_HAVE_dict_setitem_unlocked */
	ASSERT(LOCAL_DeeDict_LockWriting(self));
#endif /* !LOCAL_HAVE_dict_setitem_unlocked */
	ASSERT(_DeeDict_MustGrowVTab(self));

	/* Figure out allocation sizes (never overallocate here; we only get here when memory is low!) */
	old_hmask = self->d_hmask;
	new_hmask = old_hmask;
	if (LOCAL__DeeDict_MustGrowHTab(self)) {
		new_hmask = LOCAL_dict_tiny_hmask_from_count(self->d_vsize + 1);
		ASSERT(new_hmask > old_hmask);
	}
	new_valloc = LOCAL_dict_valloc_from_hmask_and_count(new_hmask, self->d_vsize + 1, false);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);

	/* Release dict lock */
	IF_with_locks(LOCAL_DeeDict_LockEndWrite(self));

	/* Allocate new dict tables. */
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsMalloc(new_tabsize);
	if unlikely(!new_vtab)
		goto err;

	/* Re-acquire lock to the dict. */
	IF_with_locks(LOCAL_DeeDict_LockWrite(self));

	/* Check if the dict still needs the new buffer */
	if unlikely(!LOCAL__DeeDict_MustGrowVTab(self)) {
free_buffer_and_try_again:
		IF_with_locks(LOCAL_DeeDict_LockEndWrite(self));
		LOCAL__DeeDict_TabsFree(new_vtab);
		IF_with_locks(LOCAL_DeeDict_LockWrite(self));
		if likely(!LOCAL__DeeDict_MustGrowVTab(self))
			return 0;
		goto again;
	}

	/* Check that the buffer we just allocated is actually large enough */
	if unlikely(new_valloc <= self->d_valloc)
		goto free_buffer_and_try_again;
	if unlikely(old_hmask != self->d_hmask)
		goto free_buffer_and_try_again;
	ASSERT(new_hidxio >= Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc));

	/* Everything checks out: the buffer can be installed like this! */

	/* Copy over the contents of the old vtab */
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	if likely(old_hmask == new_hmask) {
		Dee_hash_hidxio_t old_hidxio;
		union Dee_hash_htab *new_htab = (union Dee_hash_htab *)(new_vtab + new_valloc);
		new_vtab = (struct LOCAL_Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
		                                                 sizeof(struct LOCAL_Dee_dict_item));
		old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		ASSERT(self->d_hidxops == &Dee_hash_hidxio[old_hidxio]);
		if likely(old_hidxio == new_hidxio) {
			(*self->d_hidxops->hxio_movup)(new_htab, self->d_htab, new_hmask + 1);
		} else {
			(*self->d_hidxops->hxio_upr)(new_htab, self->d_htab, new_hmask + 1);
			self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
		}

		/* Update dict control elements to use the new tables. */
		self->d_valloc = new_valloc;
		LOCAL__DeeDict_SetRealVTab(self, new_vtab);
		self->d_htab = new_htab;
	} else {
		/* Must rebuild htab */
		if (_DeeDict_CanOptimizeVTab(self))
			LOCAL_dict_do_optimize_vtab_without_rebuild(self);
		new_vtab = (struct LOCAL_Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
		                                                 sizeof(struct LOCAL_Dee_dict_item));
		self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
		self->d_valloc  = new_valloc;
		_DeeDict_SetRealVTab(self, new_vtab);
		self->d_htab = (union Dee_hash_htab *)(new_vtab + new_valloc);
		LOCAL_dict_htab_rebuild_after_optimize(self);
	}

	/* Free the old tables. */
	IF_with_locks(LOCAL_DeeDict_LockEndWrite(self));
	LOCAL__DeeDict_TabsFree(old_vtab);
	IF_with_locks(LOCAL_DeeDict_LockWrite(self));
	if unlikely(LOCAL__DeeDict_MustGrowVTab(self))
		goto again;
	return 0;
err:
	return -1;
#undef IF_with_locks
}
#undef LOCAL_HAVE_dict_setitem_unlocked

#if 0
/* Make it so "!_DeeDict_MustGrowHTab(self)".
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_grow_htab_and_relock(LOCAL_Dict *__restrict self) {
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	size_t new_tabsize;
	Dee_hash_t new_hmask;
	Dee_hash_vidx_t valloc;
	Dee_hash_hidxio_t hidxio;
again:
	ASSERT(LOCAL_DeeDict_LockWriting(self));
	ASSERT(LOCAL__DeeDict_MustGrowHTab(self));
	ASSERTF(self->d_hmask != (Dee_hash_t)-1, "How? This should have been an OOM");
	valloc = self->d_valloc;
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(valloc);

	/* Figure out the size for the new table. */
	new_hmask   = (self->d_hmask << 1) | 1;
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, valloc, hidxio);
	LOCAL_DeeDict_LockEndWrite(self);

	/* Allocate the new table. */
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsMalloc(new_tabsize);
	if unlikely(!new_vtab)
		goto err;

	/* Re-acquire lock to the dict. */
	LOCAL_DeeDict_LockWrite(self);

	/* Verify that the dict still needs to have it's htab grow. */
	if unlikely(!LOCAL__DeeDict_MustGrowHTab(self)) {
free_buffer_and_try_again:
		LOCAL_DeeDict_LockEndWrite(self);
		LOCAL__DeeDict_TabsFree(new_vtab);
		LOCAL_DeeDict_LockWrite(self);
		if likely(!LOCAL__DeeDict_MustGrowHTab(self))
			return 0;
		goto again;
	}
	if unlikely(new_hmask <= self->d_hmask)
		goto free_buffer_and_try_again;
	if unlikely(valloc <= self->d_valloc)
		goto free_buffer_and_try_again;

	/* Everything checks out: the buffer can be installed like this! */
	if (LOCAL__DeeDict_CanOptimizeVTab(self))
		LOCAL_dict_do_optimize_vtab_without_rebuild(self); /* Optimize first so the memcpy needs to copy less stuff! */
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	new_vtab = (struct LOCAL_Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
	                                                 sizeof(struct LOCAL_Dee_dict_item));
	self->d_hmask = new_hmask;
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab = (union Dee_hash_htab *)(new_vtab + valloc);
	LOCAL_dict_htab_rebuild_after_optimize(self);

	/* Free the old tables. */
	LOCAL_DeeDict_LockEndWrite(self);
	LOCAL__DeeDict_TabsFree(old_vtab);
	LOCAL_DeeDict_LockWrite(self);
	if unlikely(LOCAL__DeeDict_MustGrowHTab(self))
		goto again;
	return 0;
err:
	return -1;
}
#endif

/* Shrink the vtab and release a lock to "self". Must be called when:
 * - holding a write-lock
 * - _DeeDict_CanShrinkHTab(self) is true
 * - _DeeDict_ShouldShrinkHTab(self) is true (or `fully_shrink=true')
 * NOTE: After a call to this function, the caller must always rebuild the htab! */
PRIVATE NONNULL((1)) void DCALL
LOCAL_dict_shrink_htab(LOCAL_Dict *__restrict self, bool fully_shrink) {
	Dee_hash_t new_hmask;
	size_t new_tabsize;
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	ASSERT(LOCAL_DeeDict_LockWriting(self));
	ASSERT(LOCAL__DeeDict_CanShrinkHTab(self));
	ASSERT(LOCAL__DeeDict_ShouldShrinkHTab(self) || fully_shrink);
	/* Figure out the new hmask */
	new_hmask = self->d_hmask;
	if (fully_shrink) {
		do {
			new_hmask >>= 1;
		} while (_DeeDict_CanShrinkHTab2(self->d_valloc, new_hmask));
	} else {
		do {
			new_hmask >>= 1;
		} while (_DeeDict_ShouldShrinkHTab2(self->d_valloc, new_hmask));
	}
	ASSERT(new_hmask >= self->d_valloc);
	ASSERT(new_hmask < self->d_hmask);
	ASSERT(self->d_valloc > 0);
	ASSERT(new_hmask > 0);
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc(new_hmask, self->d_valloc);
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	ASSERT(old_vtab != (struct LOCAL_Dee_dict_item *)_DeeHash_EmptyTab);
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab)
		new_vtab = old_vtab;
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
	self->d_hmask = new_hmask;

	LOCAL_dict_htab_rebuild(self);
}


/* Shrink the vtab+htab. Must be called while:
 * - holding a write-lock
 * - _DeeDict_CanShrinkVTab(self) is true
 * - _DeeDict_ShouldShrinkVTab(self) is true (or `fully_shrink=true') */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_shrink_vtab_and_htab(LOCAL_Dict *__restrict self, bool fully_shrink) {
	bool must_rebuild_htab = false;
	struct LOCAL_Dee_dict_item *old_vtab;
	struct LOCAL_Dee_dict_item *new_vtab;
	Dee_hash_vidx_t old_valloc;
	Dee_hash_vidx_t new_valloc;
	Dee_hash_t old_hmask;
	Dee_hash_t new_hmask;
	size_t new_tabsize;
	Dee_hash_hidxio_t old_hidxio;
	Dee_hash_hidxio_t new_hidxio;
	ASSERT(LOCAL_DeeDict_LockWriting(self));
	ASSERT(LOCAL__DeeDict_CanShrinkVTab(self));
	ASSERT(LOCAL__DeeDict_ShouldShrinkVTab(self) || fully_shrink);
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	if unlikely(!self->d_vused) {
		/* Special case: dict is now empty -> clear all data. */
		self->d_valloc  = 0;
		self->d_vsize   = 0;
		LOCAL__DeeDict_SetVirtVTab(self, LOCAL_DeeDict_EmptyVTab);
		self->d_hmask   = 0;
		self->d_hidxops = &Dee_hash_hidxio[0];
		self->d_htab    = DeeDict_EmptyHTab;
		if (old_vtab != (struct LOCAL_Dee_dict_item *)_DeeHash_EmptyTab)
			LOCAL__DeeDict_TabsFree(old_vtab);
		return;
	}

	/* Optimize the dict so we don't have to transfer deleted
	 * items, and can shrink even more when `fully_shrink=true' */
	if (LOCAL__DeeDict_CanOptimizeVTab(self)) {
		LOCAL_dict_do_optimize_vtab_without_rebuild(self);
		must_rebuild_htab = true;
	}

	/* Calculate changes in buffer size */
	ASSERT(self->d_vsize < self->d_valloc);
	old_valloc = self->d_valloc;
	new_valloc = fully_shrink ? self->d_vsize : LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC(self);
	ASSERT(new_valloc >= self->d_vsize);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(old_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	ASSERT(new_hidxio <= old_hidxio);
	ASSERT(self->d_hidxops == &Dee_hash_hidxio[old_hidxio]);
	old_hmask = self->d_hmask;
	new_hmask = old_hmask;
	if (fully_shrink) {
		while (_DeeDict_CanShrinkHTab2(new_valloc, new_hmask))
			new_hmask >>= 1;
	} else {
		while (_DeeDict_ShouldShrinkHTab2(new_valloc, new_hmask))
			new_hmask >>= 1;
	}
	ASSERT(new_hmask <= old_hmask);
	ASSERT(new_hmask >= new_valloc);

	/* Shrink dict hash-table in-place (so we can then inplace-realloc-trunc the table heap area) */
	if (old_hmask == new_hmask) {
		if (!must_rebuild_htab) {
			union Dee_hash_htab *old_htab = (union Dee_hash_htab *)(old_vtab + old_valloc);
			union Dee_hash_htab *new_htab = (union Dee_hash_htab *)(old_vtab + new_valloc);
			if (new_hidxio == old_hidxio) {
				(*self->d_hidxops->hxio_movdown)(new_htab, old_htab, self->d_hmask + 1);
			} else {
				Dee_hash_hidxio_t current_hidxio;
				(*self->d_hidxops->hxio_lwr)(new_htab, old_htab, self->d_hmask + 1);
				current_hidxio = old_hidxio - 1;
				while (current_hidxio > new_hidxio) {
					(*Dee_hash_hidxio[current_hidxio].hxio_lwr)(new_htab, new_htab, self->d_hmask + 1);
					--current_hidxio;
				}
				ASSERT(current_hidxio == new_hidxio);
			}
		}
	} else {
		must_rebuild_htab = true;
	}

	/* Actually realloc the dict tables. */
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(new_hmask, new_valloc, new_hidxio);
	ASSERT(new_valloc < old_valloc);
	ASSERT(new_valloc >= self->d_vsize);
	ASSERTF(old_vtab != (struct LOCAL_Dee_dict_item *)_DeeHash_EmptyTab,
	        "The empty table should have been handled by '!self->d_vused'");
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab)
		new_vtab = old_vtab; /* Shouldn't get here because the new table is always smaller! */

	/* Assign new tables / control values. */
	self->d_valloc  = new_valloc;
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
	self->d_htab    = (union Dee_hash_htab *)(new_vtab + new_valloc);
	self->d_hmask   = new_hmask;
	self->d_hidxops = &Dee_hash_hidxio[new_hidxio];

	/* Re-build the hash-table if necessary. */
	if (must_rebuild_htab)
		LOCAL_dict_htab_rebuild_after_optimize(self);
}


/* Automatically shrink allocated tables of "self" if appropriate.
 * Call this function are removing elements from "self"
 * Same as the API function "dict.shrink(fully: false)" */
LOCAL NONNULL((1)) void DCALL
LOCAL_dict_autoshrink(LOCAL_Dict *__restrict self) {
	if (LOCAL__DeeDict_ShouldShrinkVTab(self))
		LOCAL_dict_shrink_vtab_and_htab(self, false);
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_makespace_at_impl(LOCAL_Dict *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx) {
	struct LOCAL_Dee_dict_item *vtab = LOCAL__DeeDict_GetRealVTab(self);
	ASSERT(vtab_idx < self->d_vsize);
	memmoveupc(&vtab[vtab_idx + 1], &vtab[vtab_idx],
	           self->d_vsize - vtab_idx,
	           sizeof(struct LOCAL_Dee_dict_item));
	LOCAL_dict_htab_incafter(self, Dee_hash_vidx_tovirt(vtab_idx));
}

LOCAL NONNULL((1)) void DCALL
LOCAL_dict_makespace_at(LOCAL_Dict *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx) {
	ASSERT(vtab_idx <= self->d_vsize);
	if (vtab_idx < self->d_vsize)
		LOCAL_dict_makespace_at_impl(self, vtab_idx);
}






/************************************************************************/
/************************************************************************/
/* HIGH-LEVEL API                                                       */
/************************************************************************/
/************************************************************************/

struct LOCAL_dict_seq_erase_data {
	size_t dse_start;
	size_t dse_end;
	size_t dse_count;
};

PRIVATE NONNULL((1, 2)) bool DCALL
LOCAL_dict_seq_erase_data_init(struct LOCAL_dict_seq_erase_data *__restrict self,
                               LOCAL_Dict *__restrict dict, size_t start, size_t count) {
	self->dse_start = start;
	if (self->dse_start > dict->d_vused)
		self->dse_start = dict->d_vused;
	if (OVERFLOW_UADD(self->dse_start, count, &self->dse_end))
		self->dse_end = (size_t)-1;
	if (self->dse_end > dict->d_vused)
		self->dse_end = dict->d_vused;
	return OVERFLOW_USUB(self->dse_end, self->dse_start, &self->dse_count);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_mh_seq_erase(LOCAL_Dict *__restrict self, size_t start, size_t count) {
	size_t i;
	DREF DeeObject **old_objv;
	struct LOCAL_dict_seq_erase_data used;
	struct LOCAL_Dee_dict_item *vtab;
again:
	LOCAL_DeeDict_LockWrite(self);
	if unlikely(LOCAL_dict_seq_erase_data_init(&used, self, start, count)) {
		LOCAL_DeeDict_LockEndWrite(self);
		return 0;
	}
	old_objv = (DREF DeeObject **)Dee_TryMallocac(used.dse_count,
	                                              LOCAL_OBJECTS_PER_ITEM *
	                                              sizeof(DREF DeeObject *));
	if unlikely(!old_objv) {
		struct LOCAL_dict_seq_erase_data new_used;
		LOCAL_DeeDict_LockEndWrite(self);
		old_objv = (DREF DeeObject **)Dee_Mallocac(used.dse_count,
		                                           LOCAL_OBJECTS_PER_ITEM *
		                                           sizeof(DREF DeeObject *));
		if unlikely(!old_objv)
			goto err;
		LOCAL_DeeDict_LockWrite(self);
		if unlikely(LOCAL_dict_seq_erase_data_init(&new_used, self, start, count)) {
			LOCAL_DeeDict_LockEndWrite(self);
			Dee_Freea(old_objv);
			return 0;
		}
		if unlikely(new_used.dse_count != used.dse_count) {
			LOCAL_DeeDict_LockEndWrite(self);
			Dee_Freea(old_objv);
			goto again;
		}
		used.dse_start = new_used.dse_start;
		used.dse_end   = new_used.dse_end;
	}

	/* Optimize dict to ensure linear ordering. */
	if (LOCAL__DeeDict_CanOptimizeVTab(self))
		LOCAL_dict_optimize_vtab(self);

	/* Copy references and delete items. */
	vtab = LOCAL__DeeDict_GetRealVTab(self);
	ASSERT(self->d_vused >= used.dse_count);
	for (i = used.dse_start; i < used.dse_end; ++i) {
		struct LOCAL_Dee_dict_item *item = &vtab[i];
		ASSERTF(item->di_key, "Nothing should be deleted because we're optimized!");
		old_objv[(i * LOCAL_OBJECTS_PER_ITEM) + 0] = item->di_key;   /* Inherit reference */
#ifndef DEFINE_DeeHashSet
		old_objv[(i * LOCAL_OBJECTS_PER_ITEM) + 1] = item->di_value; /* Inherit reference */
		DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !DEFINE_DeeHashSet */
		item->di_key = NULL;
	}
	self->d_vused -= used.dse_count;
	LOCAL_dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decrefv(old_objv, used.dse_count * LOCAL_OBJECTS_PER_ITEM);
	Dee_Freea(old_objv);
	return 0;
err:
	return -1;
}





PRIVATE NONNULL((1)) void DCALL
LOCAL_dict_items_reverse(struct LOCAL_Dee_dict_item *items, size_t count) {
	struct LOCAL_Dee_dict_item *lo = items;
	struct LOCAL_Dee_dict_item *hi = items + count;
	while (lo < hi) {
		struct LOCAL_Dee_dict_item temp;
		--hi;
		memcpy(&temp, lo, sizeof(struct LOCAL_Dee_dict_item));
		memcpy(lo, hi, sizeof(struct LOCAL_Dee_dict_item));
		memcpy(hi, &temp, sizeof(struct LOCAL_Dee_dict_item));
		++lo;
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_mh_seq_reverse(LOCAL_Dict *self, size_t start, size_t end) {
	LOCAL_DeeDict_LockWrite(self);
	if (end > self->d_vused)
		end = self->d_vused;
	if (start < end) {
		/*virt*/Dee_hash_vidx_t vstart;
		/*virt*/Dee_hash_vidx_t vend;
		if (LOCAL__DeeDict_CanOptimizeVTab(self))
			LOCAL_dict_optimize_vtab(self);
		vstart = Dee_hash_vidx_tovirt(start);
		vend   = Dee_hash_vidx_tovirt(end);
		LOCAL_dict_items_reverse(LOCAL__DeeDict_GetVirtVTab(self) + vstart, vend - vstart);
		/* Also reverse all index references in "htab" */
		ASSERT(vstart != Dee_HASH_HTAB_EOF);
		LOCAL_dict_htab_reverse(self, vstart, vend - 1);
	}
	DeeDict_LockEndWrite(self);
	return 0;
}

PRIVATE NONNULL((1)) bool DCALL
LOCAL_dict_shrink_impl(LOCAL_Dict *__restrict self, bool fully_shrink) {
	bool result = false;
	LOCAL_DeeDict_LockWrite(self);
	if (fully_shrink ? LOCAL__DeeDict_CanShrinkVTab(self)
	                 : LOCAL__DeeDict_ShouldShrinkVTab(self)) {
		LOCAL_dict_shrink_vtab_and_htab(self, fully_shrink);
		result = true;
	} else if (fully_shrink ? LOCAL__DeeDict_CanShrinkHTab(self)
	                        : LOCAL__DeeDict_ShouldShrinkHTab(self)) {
		LOCAL_dict_shrink_htab(self, fully_shrink);
		result = true;
	}
	LOCAL_DeeDict_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_dict_shrink(LOCAL_Dict *__restrict self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	bool result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("shrink", params: "
	bool fully = true;
", docStringPrefix: "LOCAL_dict");]]]*/
#define LOCAL_dict_shrink_params "fully=!t"
	struct {
		bool fully;
	} args;
	args.fully = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fully, "|b:shrink", &args))
		goto err;
/*[[[end]]]*/
	result = LOCAL_dict_shrink_impl(self, args.fully);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_dict_reserve(LOCAL_Dict *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	bool result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("reserve", params: "
	size_t total = 0;
	size_t more = 0;
	bool weak = false;
", docStringPrefix: "LOCAL_dict");]]]*/
#define LOCAL_dict_reserve_params "total=!0,more=!0,weak=!f"
	struct {
		size_t total;
		size_t more;
		bool weak;
	} args;
	args.total = 0;
	args.more = 0;
	args.weak = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__total_more_weak, "|" UNPuSIZ UNPuSIZ "b:reserve", &args))
		goto err;
/*[[[end]]]*/
	LOCAL_DeeDict_LockWrite(self);
	if (args.total < self->d_vused)
		args.total = self->d_vused;
	if (OVERFLOW_UADD(args.total, args.more, &args.total))
		args.total = (size_t)-1;
	/* Try to allocate more space if there isn't enough space already. */
	if (args.total <= self->d_valloc) {
		result = true;
	} else {
		result = LOCAL_dict_trygrow_vtab_and_htab_with(self, args.total, args.weak);
	}
	LOCAL_DeeDict_LockEndWrite(self);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
LOCAL_dict_cc(LOCAL_Dict *__restrict self) {
	return LOCAL_dict_shrink_impl(self, true);
}




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_dict_sizeof(LOCAL_Dict *__restrict self) {
	Dee_hash_t hmask;
	Dee_hash_vidx_t valloc;
	size_t result;
	LOCAL_DeeDict_LockRead(self);
	valloc = self->d_valloc;
	hmask  = self->d_hmask;
	LOCAL_DeeDict_LockEndRead(self);
	result = LOCAL_dict_sizeoftabs_from_hmask_and_valloc(hmask, valloc);
	return DeeInt_NewSize(sizeof(LOCAL_Dict) + result);
}


struct LOCAL_dict_compare_seq_foreach_data {
	LOCAL_Dict             *dcsfd_lhs;   /* [1..1] lhs-dict. */
	/*real*/Dee_hash_vidx_t dcsfd_index; /* Next index into "dcsfd_lhs" to compare against. */
};
#define DICT_COMPARE_SEQ_FOREACH_ERROR    (-1)
#define DICT_COMPARE_SEQ_FOREACH_EQUAL    (0)
#define DICT_COMPARE_SEQ_FOREACH_NOTEQUAL (-2)
#define DICT_COMPARE_SEQ_FOREACH_LESS     (-2)
#define DICT_COMPARE_SEQ_FOREACH_GREATER  (-3)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_dict_compare_seq_foreach(void *arg, DeeObject *rhs_item) {
	LOCAL_Dict *dict;
	int cmp_result;
	struct LOCAL_Dee_dict_item *lhs_item;
#ifdef DEFINE_DeeHashSet
	DREF DeeObject *lhs_key;
#else /* DEFINE_DeeHashSet */
	DREF DeeObject *lhs_key_and_value[2];
#endif /* !DEFINE_DeeHashSet */
	struct LOCAL_dict_compare_seq_foreach_data *data;
	data = (struct LOCAL_dict_compare_seq_foreach_data *)arg;
	dict = data->dcsfd_lhs;
	LOCAL_DeeDict_LockReadAndOptimize(dict);
	if unlikely(data->dcsfd_index >= dict->d_vused) {
		DeeDict_LockEndRead(dict);
		return DICT_COMPARE_SEQ_FOREACH_LESS;
	}
	lhs_item = &LOCAL__DeeDict_GetRealVTab(dict)[data->dcsfd_index];
	ASSERT(lhs_item->di_key);
#ifdef DEFINE_DeeHashSet
	lhs_key = lhs_item->di_key;
	Dee_Incref(lhs_key);
#else /* DEFINE_DeeHashSet */
	lhs_key_and_value[0] = lhs_item->di_key;
	lhs_key_and_value[1] = lhs_item->di_value;
	Dee_Incref(lhs_key_and_value[0]);
	Dee_Incref(lhs_key_and_value[1]);
#endif /* !DEFINE_DeeHashSet */
	LOCAL_DeeDict_LockEndRead(dict);
#ifdef DEFINE_DeeHashSet
	cmp_result = DeeObject_Compare(lhs_key, rhs_item);
	Dee_Decref_unlikely(lhs_key);
#else /* DEFINE_DeeHashSet */
	cmp_result = seq_docompare__lhs_vector(lhs_key_and_value, 2, rhs_item);
	Dee_Decref_unlikely(lhs_key_and_value[1]);
	Dee_Decref_unlikely(lhs_key_and_value[0]);
#endif /* !DEFINE_DeeHashSet */
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->dcsfd_index;
	if (Dee_COMPARE_ISLE(cmp_result))
		return DICT_COMPARE_SEQ_FOREACH_LESS;
	if (Dee_COMPARE_ISGR(cmp_result))
		return DICT_COMPARE_SEQ_FOREACH_GREATER;
	return DICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return DICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_dict_compare_eq_seq_foreach(void *arg, DeeObject *rhs_item) {
	LOCAL_Dict *dict;
	int cmp_result;
	struct LOCAL_Dee_dict_item *lhs_item;
	struct LOCAL_dict_compare_seq_foreach_data *data;
#ifdef DEFINE_DeeHashSet
	DREF DeeObject *lhs_key;
#else /* DEFINE_DeeHashSet */
	DREF DeeObject *lhs_key_and_value[2];
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs_item), foreach))
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
#endif /* !DEFINE_DeeHashSet */
	data = (struct LOCAL_dict_compare_seq_foreach_data *)arg;
	dict = data->dcsfd_lhs;
	LOCAL_DeeDict_LockReadAndOptimize(dict);
	if unlikely(data->dcsfd_index >= dict->d_vused) {
		LOCAL_DeeDict_LockEndRead(dict);
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	}
	lhs_item = &LOCAL__DeeDict_GetRealVTab(dict)[data->dcsfd_index];
	ASSERT(lhs_item->di_key);
#ifdef DEFINE_DeeHashSet
	lhs_key = lhs_item->di_key;
	Dee_Incref(lhs_key);
#else /* DEFINE_DeeHashSet */
	lhs_key_and_value[0] = lhs_item->di_key;
	lhs_key_and_value[1] = lhs_item->di_value;
	Dee_Incref(lhs_key_and_value[0]);
	Dee_Incref(lhs_key_and_value[1]);
#endif /* !DEFINE_DeeHashSet */
	LOCAL_DeeDict_LockEndRead(dict);
#ifdef DEFINE_DeeHashSet
	cmp_result = DeeObject_TryCompareEq(lhs_key, rhs_item);
	Dee_Decref_unlikely(lhs_key);
#else /* DEFINE_DeeHashSet */
	cmp_result = seq_docompareeq__lhs_vector(lhs_key_and_value, 2, rhs_item);
	Dee_Decref_unlikely(lhs_key_and_value[1]);
	Dee_Decref_unlikely(lhs_key_and_value[0]);
#endif /* !DEFINE_DeeHashSet */
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->dcsfd_index;
	if (Dee_COMPARE_ISNE(cmp_result))
		return DICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	return DICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return DICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_mh_seq_compare(LOCAL_Dict *lhs, DeeObject *rhs) {
	size_t lhs_size;
	Dee_ssize_t foreach_status;
	struct LOCAL_dict_compare_seq_foreach_data data;
	data.dcsfd_index = 0;
	data.dcsfd_lhs   = lhs;
	foreach_status   = DeeObject_Foreach(rhs, &LOCAL_dict_compare_seq_foreach, &data);
	if unlikely(foreach_status == DICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_LESS)
		return Dee_COMPARE_LO;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_GREATER)
		return Dee_COMPARE_GR;
	lhs_size = LOCAL_DeeDict_SIZE_ATOMIC(lhs);
	if (data.dcsfd_index < lhs_size)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_mh_seq_compare_eq(LOCAL_Dict *lhs, DeeObject *rhs) {
	size_t lhs_size;
	Dee_ssize_t foreach_status;
	struct LOCAL_dict_compare_seq_foreach_data data;
	data.dcsfd_index = 0;
	data.dcsfd_lhs   = lhs;
	foreach_status   = DeeObject_Foreach(rhs, &LOCAL_dict_compare_eq_seq_foreach, &data);
	if unlikely(foreach_status == DICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == DICT_COMPARE_SEQ_FOREACH_NOTEQUAL)
		return Dee_COMPARE_NE;
	lhs_size = LOCAL_DeeDict_SIZE_ATOMIC(lhs);
	if (data.dcsfd_index < lhs_size)
		return Dee_COMPARE_NE;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_mh_seq_trycompare_eq(LOCAL_Dict *lhs, DeeObject *rhs) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs), iter))
		return Dee_COMPARE_NE;
	return LOCAL_dict_mh_seq_compare_eq(lhs, rhs);
}

#undef LOCAL_OBJECTS_PER_ITEM

#undef LOCAL_dict_new_with_hint
#undef LOCAL_DeeDict_TryNewWithHint
#undef LOCAL_DeeDict_TryNewWithWeakHint
#undef LOCAL_DeeDict_NewWithHint
#undef LOCAL_DeeDict_NewWithWeakHint
#undef LOCAL_dict_htab_rebuild_after_optimize
#undef LOCAL_dict_do_optimize_vtab_without_rebuild
#undef LOCAL_dict_optimize_vtab
#undef LOCAL_dict_htab_rebuild
#undef LOCAL_dict_trygrow_vtab
#undef LOCAL_dict_trygrow_vtab_and_htab_with
#undef LOCAL_dict_trygrow_vtab_and_htab
#undef LOCAL_dict_trygrow_htab_and_maybe_vtab
#undef LOCAL_dict_grow_vtab_and_htab_and_relock
#undef LOCAL_dict_grow_vtab_and_htab_and_relock_impl
#undef LOCAL_dict_grow_htab_and_relock
#undef LOCAL_dict_shrink_htab
#undef LOCAL_dict_shrink_vtab_and_htab
#undef LOCAL_dict_autoshrink
#undef LOCAL_dict_makespace_at_impl
#undef LOCAL_dict_makespace_at
/*...................................................................................................*/
#undef LOCAL_dict_seq_erase_data
#undef LOCAL_dict_seq_erase_data_init
#undef LOCAL_dict_mh_seq_erase
#undef LOCAL_dict_items_reverse
#undef LOCAL_dict_mh_seq_reverse
#undef LOCAL_dict_shrink_impl
#undef LOCAL_dict_shrink
#undef LOCAL_dict_reserve
#undef LOCAL_dict_cc
#undef LOCAL_dict_sizeof
#undef LOCAL_dict_compare_seq_foreach_data
#undef LOCAL_dict_compare_seq_foreach
#undef LOCAL_dict_compare_eq_seq_foreach
#undef LOCAL_dict_mh_seq_compare
#undef LOCAL_dict_mh_seq_compare_eq
#undef LOCAL_dict_mh_seq_trycompare_eq
/*...................................................................................................*/
#undef LOCAL_Dict
#undef LOCAL__DeeDict_TabsMalloc
#undef LOCAL__DeeDict_TabsCalloc
#undef LOCAL__DeeDict_TabsRealloc
#undef LOCAL__DeeDict_TabsTryMalloc
#undef LOCAL__DeeDict_TabsTryCalloc
#undef LOCAL__DeeDict_TabsTryRealloc
#undef LOCAL__DeeDict_TabsFree
#undef LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL
#undef LOCAL_Dee_dict_item
#undef LOCAL_DeeDict_Type
#undef LOCAL__DeeDict_ShouldGrowHTab
#undef LOCAL__DeeDict_MustGrowHTab
#undef LOCAL__DeeDict_ShouldOptimizeVTab
#undef LOCAL__DeeDict_CanOptimizeVTab
#undef LOCAL__DeeDict_MustGrowVTab
#undef LOCAL__DeeDict_CanGrowVTab
#undef LOCAL__DeeDict_ShouldShrinkVTab
#undef LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC
#undef LOCAL__DeeDict_CanShrinkVTab
#undef LOCAL__DeeDict_ShouldShrinkHTab
#undef LOCAL__DeeDict_CanShrinkHTab
#undef LOCAL_DICT_FROMSEQ_DEFAULT_HINT
#undef LOCAL_dict_suggested_max_valloc_from_count
#undef LOCAL_dict_valloc_from_hmask_and_count
#undef LOCAL_dict_hmask_from_count
#undef LOCAL_dict_tiny_hmask_from_count
#undef LOCAL_dict_sizeoftabs_from_hmask_and_valloc
#undef LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio
#undef LOCAL_DeeDict_LockReadAndOptimize
#undef LOCAL__DeeDict_GetVirtVTab
#undef LOCAL__DeeDict_SetVirtVTab
#undef LOCAL__DeeDict_GetRealVTab
#undef LOCAL__DeeDict_SetRealVTab
#undef LOCAL__DeeDict_HashIdxInit
#undef LOCAL__DeeDict_HashIdxNext
#undef LOCAL__DeeDict_HTabGet
#undef LOCAL__DeeDict_HTabSet
#undef LOCAL_DeeDict_LockReading
#undef LOCAL_DeeDict_LockWriting
#undef LOCAL_DeeDict_LockTryRead
#undef LOCAL_DeeDict_LockTryWrite
#undef LOCAL_DeeDict_LockRead
#undef LOCAL_DeeDict_LockWrite
#undef LOCAL_DeeDict_LockTryUpgrade
#undef LOCAL_DeeDict_LockUpgrade
#undef LOCAL_DeeDict_LockDowngrade
#undef LOCAL_DeeDict_LockEndWrite
#undef LOCAL_DeeDict_LockEndRead
#undef LOCAL_DeeDict_LockEnd
#undef LOCAL_DeeDict_SIZE
#undef LOCAL_DeeDict_SIZE_ATOMIC
#undef LOCAL_DeeDict_EmptyVTab
#undef LOCAL_DeeDict_EmptyHTab
#undef LOCAL_dict_htab_decafter
#undef LOCAL_dict_htab_incafter
#undef LOCAL_dict_htab_decrange
#undef LOCAL_dict_htab_incrange
#ifdef DEFINE_DeeHashSet
#undef d_valloc
#undef d_vsize
#undef d_vused
#undef d_vtab
#undef d_hmask
#undef d_hidxops
#undef d_htab
#undef d_lock
#undef di_hash
#undef di_key
#ifdef __INTELLISENSE__
#undef Dee_dict_item
#endif /* __INTELLISENSE__ */
#endif /* DEFINE_DeeHashSet */

DECL_END

#undef DEFINE_DeeDict
#undef DEFINE_DeeHashSet
