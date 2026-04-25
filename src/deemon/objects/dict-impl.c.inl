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
#define DEFINE_LOW_LEVEL
#define DEFINE_HIGH_LEVEL
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Freea, Dee_Mallocac, Dee_TryMallocac */
#include <deemon/arg.h>             /* DeeArg_UnpackStructKw, UNPuSIZ */
#include <deemon/bool.h>            /* return_bool */
#include <deemon/dict.h>            /* DeeDict_*, _DeeDict_* */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/gc.h>              /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGCObject_TRYMALLOC, DeeGC_TRACK */
#include <deemon/hashset.h>         /* DeeHashSet_*, Dee_hashset_item, _DeeHashSet_* */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/object.h>          /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_*, Dee_AsObject, Dee_COMPARE_*, Dee_Decref*, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_weakref_support_init */
#include <deemon/operator-hints.h>  /* DeeType_HasNativeOperator */
#include <deemon/rodict.h>          /* DeeRoDict* */
#include <deemon/roset.h>           /* DeeRoSet* */
#include <deemon/serial.h>          /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/system-features.h> /* bzeroc, memcpy*, memmovedownc, memmoveupc */
#include <deemon/type.h>            /* DeeObject_InitStatic, DeeObject_IsShared, Dee_Visit, Dee_visit_t */
#include <deemon/util/hash-io.h>    /* Dee_HASH_HIDXIO_FROM_VALLOC, Dee_HASH_HTAB_EOF, Dee_hash_*, _DeeHash_EmptyTab */
#include <deemon/util/lock.h>       /* DeeLock_Acquire2, Dee_atomic_rwlock_init */
#include <deemon/util/objectlist.h> /* Dee_objectlist, Dee_objectlist_* */

#include <hybrid/align.h>    /* CEILDIV */
#include <hybrid/overflow.h> /* OVERFLOW_UADD, OVERFLOW_USUB */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/kwlist.h"
#include "dict-utils.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

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


#undef LOCAL_DICT_INITFROM_NEEDSLOCK
#ifdef DEFINE_DeeDict
#ifdef DICT_INITFROM_NEEDSLOCK
#define LOCAL_DICT_INITFROM_NEEDSLOCK
#endif /* DICT_INITFROM_NEEDSLOCK */
#define LOCAL_dict_new_with_hint                                dict_new_with_hint
#define LOCAL_DeeDict_TryNewWithHint                            DeeDict_TryNewWithHint
#define LOCAL_DeeDict_TryNewWithWeakHint                        DeeDict_TryNewWithWeakHint
#define LOCAL_DeeDict_NewWithHint                               DeeDict_NewWithHint
#define LOCAL_DeeDict_NewWithWeakHint                           DeeDict_NewWithWeakHint
#define LOCAL_dict_htab_rebuild_after_optimize                  dict_htab_rebuild_after_optimize
#define LOCAL_dict_do_optimize_vtab_without_rebuild             dict_do_optimize_vtab_without_rebuild
#define LOCAL_dict_optimize_vtab                                dict_optimize_vtab
#define LOCAL_dict_htab_rebuild                                 dict_htab_rebuild
#define LOCAL_dict_trygrow_vtab                                 dict_trygrow_vtab
#define LOCAL_dict_trygrow_vtab_and_htab_with                   dict_trygrow_vtab_and_htab_with
#define LOCAL_dict_trygrow_vtab_and_htab                        dict_trygrow_vtab_and_htab
#define LOCAL_dict_trygrow_htab_and_maybe_vtab                  dict_trygrow_htab_and_maybe_vtab
#define LOCAL_dict_grow_vtab_and_htab_and_relock                dict_grow_vtab_and_htab_and_relock
#define LOCAL_dict_grow_vtab_and_htab_and_relock_impl           dict_grow_vtab_and_htab_and_relock_impl
#define LOCAL_dict_grow_htab_and_relock                         dict_grow_htab_and_relock
#define LOCAL_dict_shrink_htab                                  dict_shrink_htab
#define LOCAL_dict_shrink_vtab_and_htab                         dict_shrink_vtab_and_htab
#define LOCAL_dict_autoshrink                                   dict_autoshrink
#define LOCAL_dict_makespace_at_impl                            dict_makespace_at_impl
#define LOCAL_dict_makespace_at                                 dict_makespace_at
/*...................................................................................................*/
#define LOCAL_dict_init_fromrodict_noincref                     dict_init_fromrodict_noincref
#define LOCAL_dict_init_fromrodict                              dict_init_fromrodict
#define LOCAL_dict_init_fromrodict_keysonly                     dict_init_fromrodict_keysonly
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#define LOCAL_dict_init_fromroset_noincref                      dict_init_fromroset_noincref
#define LOCAL_dict_init_fromroset_keysonly                      dict_init_fromroset_keysonly
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
#define LOCAL_dict_new_copy                                     dict_new_copy
#define LOCAL_DeeDict_FromSequence                              DeeDict_FromSequence
#define LOCAL_DeeDict_FromSequenceInheritedOnSuccess            DeeDict_FromSequenceInheritedOnSuccess
#define LOCAL_DeeDict_FromRoDict                                DeeDict_FromRoDict
#define LOCAL_dict_insert_items_inherited_after_first_duplicate dict_insert_items_inherited_after_first_duplicate
#define LOCAL_DeeDict_NewKeyValuesInherited                     DeeDict_NewKeyValuesInherited
#define LOCAL_dict_fini                                         dict_fini
#define LOCAL_dict_visit                                        dict_visit
#define LOCAL_dict_initfrom_empty                               dict_initfrom_empty
#define LOCAL_dict_initfrom_hint                                dict_initfrom_hint
#define LOCAL_dict_initfrom_seq                                 dict_initfrom_seq
#define LOCAL_DictBuffer                                        DictBuffer
#define LOCAL_dict_assign                                       dict_assign
#define LOCAL_dict_moveassign                                   dict_moveassign
#define LOCAL_dict_ctor                                         dict_ctor
#define LOCAL_dict_copy                                         dict_copy
#define LOCAL_dict_init                                         dict_init
#define LOCAL_dict_serialize                                    dict_serialize
#define LOCAL_dict_printrepr                                    dict_printrepr
#define LOCAL_dict_mh_clear                                     dict_mh_clear
#define LOCAL_dict_seq_erase_data                               dict_seq_erase_data
#define LOCAL_dict_seq_erase_data_init                          dict_seq_erase_data_init
#define LOCAL_dict_mh_seq_erase                                 dict_mh_seq_erase
#define LOCAL_dict_items_reverse                                dict_items_reverse
#define LOCAL_dict_mh_seq_reverse                               dict_mh_seq_reverse
#define LOCAL_dict_shrink_impl                                  dict_shrink_impl
#define LOCAL_dict_shrink                                       dict_shrink
#define LOCAL_dict_reserve                                      dict_reserve
#define LOCAL_dict_cc                                           dict_cc
#define LOCAL_dict_sizeof                                       dict_sizeof
#define LOCAL_dict_compare_seq_foreach_data                     dict_compare_seq_foreach_data
#define LOCAL_dict_compare_seq_foreach                          dict_compare_seq_foreach
#define LOCAL_dict_compare_eq_seq_foreach                       dict_compare_eq_seq_foreach
#define LOCAL_dict_mh_seq_compare                               dict_mh_seq_compare
#define LOCAL_dict_mh_seq_compare_eq                            dict_mh_seq_compare_eq
#define LOCAL_dict_mh_seq_trycompare_eq                         dict_mh_seq_trycompare_eq
/*...................................................................................................*/
#define LOCAL_Dict                                              Dict
#define LOCAL__DeeDict_TabsMalloc                               _DeeDict_TabsMalloc
#define LOCAL__DeeDict_TabsCalloc                               _DeeDict_TabsCalloc
#define LOCAL__DeeDict_TabsRealloc                              _DeeDict_TabsRealloc
#define LOCAL__DeeDict_TabsTryMalloc                            _DeeDict_TabsTryMalloc
#define LOCAL__DeeDict_TabsTryCalloc                            _DeeDict_TabsTryCalloc
#define LOCAL__DeeDict_TabsTryRealloc                           _DeeDict_TabsTryRealloc
#define LOCAL__DeeDict_TabsFree                                 _DeeDict_TabsFree
#define LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL                   NULL_IF__DeeDict_EmptyVTab_REAL
#define LOCAL_Dee_dict_item                                     Dee_dict_item
#define LOCAL_DeeDict_Type                                      DeeDict_Type
#define LOCAL__DeeDict_ShouldGrowHTab                           _DeeDict_ShouldGrowHTab
#define LOCAL__DeeDict_MustGrowHTab                             _DeeDict_MustGrowHTab
#define LOCAL__DeeDict_ShouldOptimizeVTab                       _DeeDict_ShouldOptimizeVTab
#define LOCAL__DeeDict_CanOptimizeVTab                          _DeeDict_CanOptimizeVTab
#define LOCAL__DeeDict_MustGrowVTab                             _DeeDict_MustGrowVTab
#define LOCAL__DeeDict_CanGrowVTab                              _DeeDict_CanGrowVTab
#define LOCAL__DeeDict_ShouldShrinkVTab                         _DeeDict_ShouldShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC                _DeeDict_ShouldShrinkVTab_NEWALLOC
#define LOCAL__DeeDict_CanShrinkVTab                            _DeeDict_CanShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkHTab                         _DeeDict_ShouldShrinkHTab
#define LOCAL__DeeDict_CanShrinkHTab                            _DeeDict_CanShrinkHTab
#define LOCAL_DICT_FROMSEQ_DEFAULT_HINT                         DICT_FROMSEQ_DEFAULT_HINT
#define LOCAL_dict_suggested_max_valloc_from_count              dict_suggested_max_valloc_from_count
#define LOCAL_dict_valloc_from_hmask_and_count                  dict_valloc_from_hmask_and_count
#define LOCAL_dict_hmask_from_count                             dict_hmask_from_count
#define LOCAL_dict_tiny_hmask_from_count                        dict_tiny_hmask_from_count
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc             dict_sizeoftabs_from_hmask_and_valloc
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio  dict_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_DeeDict_LockReadAndOptimize                       DeeDict_LockReadAndOptimize
#define LOCAL__DeeDict_GetVirtVTab                              _DeeDict_GetVirtVTab
#define LOCAL__DeeDict_SetVirtVTab                              _DeeDict_SetVirtVTab
#define LOCAL__DeeDict_GetRealVTab                              _DeeDict_GetRealVTab
#define LOCAL__DeeDict_SetRealVTab                              _DeeDict_SetRealVTab
#define LOCAL__DeeDict_HashIdxInit                              _DeeDict_HashIdxInit
#define LOCAL__DeeDict_HashIdxNext                              _DeeDict_HashIdxNext
#define LOCAL__DeeDict_HTabGet                                  _DeeDict_HTabGet
#define LOCAL__DeeDict_HTabSet                                  _DeeDict_HTabSet
#define LOCAL_DeeDict_LockReading                               DeeDict_LockReading
#define LOCAL_DeeDict_LockWriting                               DeeDict_LockWriting
#define LOCAL_DeeDict_LockTryRead                               DeeDict_LockTryRead
#define LOCAL_DeeDict_LockTryWrite                              DeeDict_LockTryWrite
#define LOCAL_DeeDict_LockRead                                  DeeDict_LockRead
#define LOCAL_DeeDict_LockWrite                                 DeeDict_LockWrite
#define LOCAL_DeeDict_LockTryUpgrade                            DeeDict_LockTryUpgrade
#define LOCAL_DeeDict_LockUpgrade                               DeeDict_LockUpgrade
#define LOCAL_DeeDict_LockDowngrade                             DeeDict_LockDowngrade
#define LOCAL_DeeDict_LockEndWrite                              DeeDict_LockEndWrite
#define LOCAL_DeeDict_LockEndRead                               DeeDict_LockEndRead
#define LOCAL_DeeDict_LockEnd                                   DeeDict_LockEnd
#define LOCAL_DeeDict_SIZE                                      DeeDict_SIZE
#define LOCAL_DeeDict_SIZE_ATOMIC                               DeeDict_SIZE_ATOMIC
#define LOCAL_DeeDict_EmptyVTab                                 DeeDict_EmptyVTab
#define LOCAL_DeeDict_EmptyHTab                                 DeeDict_EmptyHTab
#define LOCAL_dict_htab_decafter                                dict_htab_decafter
#define LOCAL_dict_htab_incafter                                dict_htab_incafter
#define LOCAL_dict_htab_decrange                                dict_htab_decrange
#define LOCAL_dict_htab_incrange                                dict_htab_incrange
#define LOCAL_dict_htab_reverse                                 dict_htab_reverse
#define LOCAL_DeeRoDictObject                                   DeeRoDictObject
#define LOCAL_dict_init_fromcopy                                dict_init_fromcopy
#define LOCAL_DeeDict_CheckExact                                DeeDict_CheckExact
#define LOCAL_DeeRoDict_Check                                   DeeRoDict_Check
#define LOCAL_DeeDict_New                                       DeeDict_New
#define LOCAL_DeeRoDict_Type                                    DeeRoDict_Type
#define LOCAL_dict_verify                                       dict_verify
#define LOCAL_STR_Dict                                          STR_Dict
#elif defined(DEFINE_DeeHashSet)
#ifdef HASHSET_INITFROM_NEEDSLOCK
#define LOCAL_DICT_INITFROM_NEEDSLOCK
#endif /* HASHSET_INITFROM_NEEDSLOCK */
#define LOCAL_dict_new_with_hint                                hashset_new_with_hint
#define LOCAL_DeeDict_TryNewWithHint                            DeeHashSet_TryNewWithHint
#define LOCAL_DeeDict_TryNewWithWeakHint                        DeeHashSet_TryNewWithWeakHint
#define LOCAL_DeeDict_NewWithHint                               DeeHashSet_NewWithHint
#define LOCAL_DeeDict_NewWithWeakHint                           DeeHashSet_NewWithWeakHint
#define LOCAL_dict_htab_rebuild_after_optimize                  hashset_htab_rebuild_after_optimize
#define LOCAL_dict_do_optimize_vtab_without_rebuild             hashset_do_optimize_vtab_without_rebuild
#define LOCAL_dict_optimize_vtab                                hashset_optimize_vtab
#define LOCAL_dict_htab_rebuild                                 hashset_htab_rebuild
#define LOCAL_dict_trygrow_vtab                                 hashset_trygrow_vtab
#define LOCAL_dict_trygrow_vtab_and_htab_with                   hashset_trygrow_vtab_and_htab_with
#define LOCAL_dict_trygrow_vtab_and_htab                        hashset_trygrow_vtab_and_htab
#define LOCAL_dict_trygrow_htab_and_maybe_vtab                  hashset_trygrow_htab_and_maybe_vtab
#define LOCAL_dict_grow_vtab_and_htab_and_relock                hashset_grow_vtab_and_htab_and_relock
#define LOCAL_dict_grow_vtab_and_htab_and_relock_impl           hashset_grow_vtab_and_htab_and_relock_impl
#define LOCAL_dict_grow_htab_and_relock                         hashset_grow_htab_and_relock
#define LOCAL_dict_shrink_htab                                  hashset_shrink_htab
#define LOCAL_dict_shrink_vtab_and_htab                         hashset_shrink_vtab_and_htab
#define LOCAL_dict_autoshrink                                   hashset_autoshrink
#define LOCAL_dict_makespace_at_impl                            hashset_makespace_at_impl
#define LOCAL_dict_makespace_at                                 hashset_makespace_at
/*...................................................................................................*/
#define LOCAL_dict_init_fromrodict_noincref                     hashset_init_fromroset_noincref
#define LOCAL_dict_init_fromrodict                              hashset_init_fromroset
#define LOCAL_dict_new_copy                                     hashset_new_copy
#define LOCAL_DeeDict_FromSequence                              DeeHashSet_FromSequence
#define LOCAL_DeeDict_FromSequenceInheritedOnSuccess            DeeHashSet_FromSequenceInheritedOnSuccess
#define LOCAL_DeeDict_FromRoDict                                DeeHashSet_FromRoSet
#define LOCAL_dict_insert_items_inherited_after_first_duplicate hashset_insert_items_inherited_after_first_duplicate
#define LOCAL_DeeDict_NewKeyValuesInherited                     DeeHashSet_NewItemsInherited
#define LOCAL_dict_fini                                         hashset_fini
#define LOCAL_dict_visit                                        hashset_visit
#define LOCAL_dict_initfrom_empty                               hashset_initfrom_empty
#define LOCAL_dict_initfrom_hint                                hashset_initfrom_hint
#define LOCAL_dict_initfrom_seq                                 hashset_initfrom_seq
#define LOCAL_DictBuffer                                        HashSetBuffer
#define LOCAL_dict_assign                                       hashset_assign
#define LOCAL_dict_moveassign                                   hashset_moveassign
#define LOCAL_dict_ctor                                         hashset_ctor
#define LOCAL_dict_copy                                         hashset_copy
#define LOCAL_dict_init                                         hashset_init
#define LOCAL_dict_serialize                                    hashset_serialize
#define LOCAL_dict_printrepr                                    hashset_printrepr
#define LOCAL_dict_mh_clear                                     hashset_mh_clear
#define LOCAL_dict_seq_erase_data                               hashset_seq_erase_data
#define LOCAL_dict_seq_erase_data_init                          hashset_seq_erase_data_init
#define LOCAL_dict_mh_seq_erase                                 hashset_mh_seq_erase
#define LOCAL_dict_items_reverse                                hashset_items_reverse
#define LOCAL_dict_mh_seq_reverse                               hashset_mh_seq_reverse
#define LOCAL_dict_shrink_impl                                  hashset_shrink_impl
#define LOCAL_dict_shrink                                       hashset_shrink
#define LOCAL_dict_reserve                                      hashset_reserve
#define LOCAL_dict_cc                                           hashset_cc
#define LOCAL_dict_sizeof                                       hashset_sizeof
#define LOCAL_dict_compare_seq_foreach_data                     hashset_compare_seq_foreach_data
#define LOCAL_dict_compare_seq_foreach                          hashset_compare_seq_foreach
#define LOCAL_dict_compare_eq_seq_foreach                       hashset_compare_eq_seq_foreach
#define LOCAL_dict_mh_seq_compare                               hashset_mh_seq_compare
#define LOCAL_dict_mh_seq_compare_eq                            hashset_mh_seq_compare_eq
#define LOCAL_dict_mh_seq_trycompare_eq                         hashset_mh_seq_trycompare_eq
/*...................................................................................................*/
#define LOCAL_Dict                                              HashSet
#define LOCAL__DeeDict_TabsMalloc                               _DeeHashSet_TabsMalloc
#define LOCAL__DeeDict_TabsCalloc                               _DeeHashSet_TabsCalloc
#define LOCAL__DeeDict_TabsRealloc                              _DeeHashSet_TabsRealloc
#define LOCAL__DeeDict_TabsTryMalloc                            _DeeHashSet_TabsTryMalloc
#define LOCAL__DeeDict_TabsTryCalloc                            _DeeHashSet_TabsTryCalloc
#define LOCAL__DeeDict_TabsTryRealloc                           _DeeHashSet_TabsTryRealloc
#define LOCAL__DeeDict_TabsFree                                 _DeeHashSet_TabsFree
#define LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL                   NULL_IF__DeeHashSet_EmptyVTab_REAL
#define LOCAL_Dee_dict_item                                     Dee_hashset_item
#define LOCAL_DeeDict_Type                                      DeeHashSet_Type
#define LOCAL__DeeDict_ShouldGrowHTab                           _DeeHashSet_ShouldGrowHTab
#define LOCAL__DeeDict_MustGrowHTab                             _DeeHashSet_MustGrowHTab
#define LOCAL__DeeDict_ShouldOptimizeVTab                       _DeeHashSet_ShouldOptimizeVTab
#define LOCAL__DeeDict_CanOptimizeVTab                          _DeeHashSet_CanOptimizeVTab
#define LOCAL__DeeDict_MustGrowVTab                             _DeeHashSet_MustGrowVTab
#define LOCAL__DeeDict_CanGrowVTab                              _DeeHashSet_CanGrowVTab
#define LOCAL__DeeDict_ShouldShrinkVTab                         _DeeHashSet_ShouldShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkVTab_NEWALLOC                _DeeHashSet_ShouldShrinkVTab_NEWALLOC
#define LOCAL__DeeDict_CanShrinkVTab                            _DeeHashSet_CanShrinkVTab
#define LOCAL__DeeDict_ShouldShrinkHTab                         _DeeHashSet_ShouldShrinkHTab
#define LOCAL__DeeDict_CanShrinkHTab                            _DeeHashSet_CanShrinkHTab
#define LOCAL_DICT_FROMSEQ_DEFAULT_HINT                         HASHSET_FROMSEQ_DEFAULT_HINT
#define LOCAL_dict_suggested_max_valloc_from_count              hashset_suggested_max_valloc_from_count
#define LOCAL_dict_valloc_from_hmask_and_count                  hashset_valloc_from_hmask_and_count
#define LOCAL_dict_hmask_from_count                             hashset_hmask_from_count
#define LOCAL_dict_tiny_hmask_from_count                        hashset_tiny_hmask_from_count
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc             hashset_sizeoftabs_from_hmask_and_valloc
#define LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio  hashset_sizeoftabs_from_hmask_and_valloc_and_hidxio
#define LOCAL_DeeDict_LockReadAndOptimize                       DeeHashSet_LockReadAndOptimize
#define LOCAL__DeeDict_GetVirtVTab                              _DeeHashSet_GetVirtVTab
#define LOCAL__DeeDict_SetVirtVTab                              _DeeHashSet_SetVirtVTab
#define LOCAL__DeeDict_GetRealVTab                              _DeeHashSet_GetRealVTab
#define LOCAL__DeeDict_SetRealVTab                              _DeeHashSet_SetRealVTab
#define LOCAL__DeeDict_HashIdxInit                              _DeeHashSet_HashIdxInit
#define LOCAL__DeeDict_HashIdxNext                              _DeeHashSet_HashIdxNext
#define LOCAL__DeeDict_HTabGet                                  _DeeHashSet_HTabGet
#define LOCAL__DeeDict_HTabSet                                  _DeeHashSet_HTabSet
#define LOCAL_DeeDict_LockReading                               DeeHashSet_LockReading
#define LOCAL_DeeDict_LockWriting                               DeeHashSet_LockWriting
#define LOCAL_DeeDict_LockTryRead                               DeeHashSet_LockTryRead
#define LOCAL_DeeDict_LockTryWrite                              DeeHashSet_LockTryWrite
#define LOCAL_DeeDict_LockRead                                  DeeHashSet_LockRead
#define LOCAL_DeeDict_LockWrite                                 DeeHashSet_LockWrite
#define LOCAL_DeeDict_LockTryUpgrade                            DeeHashSet_LockTryUpgrade
#define LOCAL_DeeDict_LockUpgrade                               DeeHashSet_LockUpgrade
#define LOCAL_DeeDict_LockDowngrade                             DeeHashSet_LockDowngrade
#define LOCAL_DeeDict_LockEndWrite                              DeeHashSet_LockEndWrite
#define LOCAL_DeeDict_LockEndRead                               DeeHashSet_LockEndRead
#define LOCAL_DeeDict_LockEnd                                   DeeHashSet_LockEnd
#define LOCAL_DeeDict_SIZE                                      DeeHashSet_SIZE
#define LOCAL_DeeDict_SIZE_ATOMIC                               DeeHashSet_SIZE_ATOMIC
#define LOCAL_DeeDict_EmptyVTab                                 DeeHashSet_EmptyVTab
#define LOCAL_DeeDict_EmptyHTab                                 DeeHashSet_EmptyHTab
#define LOCAL_dict_htab_decafter                                hashset_htab_decafter
#define LOCAL_dict_htab_incafter                                hashset_htab_incafter
#define LOCAL_dict_htab_decrange                                hashset_htab_decrange
#define LOCAL_dict_htab_incrange                                hashset_htab_incrange
#define LOCAL_dict_htab_reverse                                 hashset_htab_reverse
#define LOCAL_DeeRoDictObject                                   DeeRoSetObject
#define LOCAL_dict_init_fromcopy                                hashset_init_fromcopy
#define LOCAL_DeeDict_CheckExact                                DeeHashSet_CheckExact
#define LOCAL_DeeRoDict_Check                                   DeeRoSet_Check
#define LOCAL_DeeDict_New                                       DeeHashSet_New
#define LOCAL_DeeRoDict_Type                                    DeeRoSet_Type
#define LOCAL_dict_verify                                       hashset_verify
#define LOCAL_STR_Dict                                          STR_HashSet

/* Map dict fields to hashsets */
#define d_valloc   hs_valloc
#define d_vsize    hs_vsize
#define d_vused    hs_vused
#define d_vtab     hs_vtab
#define d_hmask    hs_hmask
#define d_hidxops  hs_hidxops
#define d_htab     hs_htab
#define d_lock     hs_lock
#define di_hash    hsi_hash
#define di_key     hsi_key
#define rd_vsize   rs_vsize
#define rd_hmask   rs_hmask
#define rd_hidxget rs_hidxget
#define rd_htab    rs_htab
#define rd_vtab    rs_vtab
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
#ifdef DEFINE_LOW_LEVEL
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
	LOCAL__DeeDict_SetRealVTab(result, (struct LOCAL_Dee_dict_item *)tabs);
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


Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_TryNewWithHint(size_t num_items));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_TryNewWithHint(size_t num_items));
PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_TryNewWithHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, true, false));
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_TryNewWithWeakHint(size_t num_items));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_TryNewWithWeakHint(size_t num_items));
PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_TryNewWithWeakHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, true, true));
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_NewWithHint(size_t num_items));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_NewWithHint(size_t num_items));
PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_NewWithHint(size_t num_items) {
	return Dee_AsObject(LOCAL_dict_new_with_hint(num_items, false, false));
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_NewWithWeakHint(size_t num_items));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_NewWithWeakHint(size_t num_items));
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
	ASSERT(LOCAL__DeeDict_CanOptimizeVTab(self));
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
 * - The caller must ensure that `LOCAL__DeeDict_CanGrowVTab(self)' is true
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
	ASSERT(LOCAL__DeeDict_CanGrowVTab(self));
	new_valloc = LOCAL_dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, true);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	ASSERT(self->d_hidxops == &Dee_hash_hidxio[old_hidxio]);
	new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
	old_vtab = LOCAL__DeeDict_GetRealVTab(self);
	old_vtab = LOCAL_NULL_IF__DeeDict_EmptyVTab_REAL(old_vtab);
	new_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsTryRealloc(old_vtab, new_tabsize);
	if unlikely(!new_vtab) {
		new_valloc = LOCAL_dict_valloc_from_hmask_and_count(self->d_hmask, self->d_vsize + 1, false);
		old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
		new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_tabsize = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, new_valloc, new_hidxio);
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
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
	self->d_valloc = new_valloc;
	self->d_htab   = new_htab;
	return true;
}

/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!LOCAL__DeeDict_CanGrowVTab(self)"
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
	LOCAL__DeeDict_SetRealVTab(self, new_vtab);
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
 * also, and can be used even when "!LOCAL__DeeDict_CanGrowVTab(self)"
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

/* Make it so "!LOCAL__DeeDict_MustGrowVTab(self)"
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
	ASSERT(LOCAL__DeeDict_MustGrowVTab(self));

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
		if (LOCAL__DeeDict_CanOptimizeVTab(self))
			LOCAL_dict_do_optimize_vtab_without_rebuild(self);
		new_vtab = (struct LOCAL_Dee_dict_item *)memcpyc(new_vtab, old_vtab, self->d_vsize,
		                                                 sizeof(struct LOCAL_Dee_dict_item));
		self->d_hidxops = &Dee_hash_hidxio[new_hidxio];
		self->d_valloc  = new_valloc;
		LOCAL__DeeDict_SetRealVTab(self, new_vtab);
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
/* Make it so "!LOCAL__DeeDict_MustGrowHTab(self)".
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
 * - LOCAL__DeeDict_CanShrinkHTab(self) is true
 * - LOCAL__DeeDict_ShouldShrinkHTab(self) is true (or `fully_shrink=true')
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
 * - LOCAL__DeeDict_CanShrinkVTab(self) is true
 * - LOCAL__DeeDict_ShouldShrinkVTab(self) is true (or `fully_shrink=true') */
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
		self->d_htab    = LOCAL_DeeDict_EmptyHTab;
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
#endif /* DEFINE_LOW_LEVEL */






/************************************************************************/
/************************************************************************/
/* HIGH-LEVEL API                                                       */
/************************************************************************/
/************************************************************************/
#ifdef DEFINE_HIGH_LEVEL

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromrodict_noincref(LOCAL_Dict *__restrict self,
                                    LOCAL_DeeRoDictObject *__restrict other) {
	size_t tabssz;
	Dee_hash_hidxio_t hidxio;
	struct LOCAL_Dee_dict_item *vtab;
	self->d_valloc  = other->rd_vsize;
	self->d_vsize   = other->rd_vsize;
	self->d_vused   = other->rd_vsize;
	self->d_hmask   = other->rd_hmask;
	ASSERT(self->d_valloc <= self->d_hmask);
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	ASSERT(other->rd_hidxget == Dee_hash_hidxio[hidxio].hxio_get);
	self->d_hidxops = &Dee_hash_hidxio[hidxio];
	tabssz = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, self->d_valloc, hidxio);
	vtab   = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsMalloc(tabssz);
	if unlikely(!vtab)
		goto err;

	/* Because they're binary-compatible, we can just copy data from "other" as-is.
	 * This will duplicate both the vtab, as well as the htab! */
	vtab = (struct LOCAL_Dee_dict_item *)memcpy(vtab, other->rd_vtab, tabssz);
	LOCAL__DeeDict_SetRealVTab(self, vtab);
	self->d_htab = (union Dee_hash_htab *)(vtab + self->d_valloc);
#ifdef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* LOCAL_DICT_INITFROM_NEEDSLOCK */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromrodict(LOCAL_Dict *__restrict self,
                           LOCAL_DeeRoDictObject *__restrict other) {
	int result = LOCAL_dict_init_fromrodict_noincref(self, other);
	if likely(result == 0) {
		Dee_hash_vidx_t i;
		struct LOCAL_Dee_dict_item *vtab;
		vtab = LOCAL__DeeDict_GetRealVTab(self);
		ASSERT(self->d_vused == self->d_vsize);
		for (i = 0; i < self->d_vused; ++i) {
			struct LOCAL_Dee_dict_item *item = &vtab[i];
			Dee_Incref(item->di_key);
#ifndef DEFINE_DeeHashSet
			Dee_Incref(item->di_value);
#endif /* !DEFINE_DeeHashSet */
		}
		LOCAL_dict_verify(self);
	}
	return result;
}

#ifdef DEFINE_DeeDict
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromrodict_keysonly(LOCAL_Dict *__restrict self,
                                    LOCAL_DeeRoDictObject *__restrict other) {
	int result = LOCAL_dict_init_fromrodict_noincref(self, other);
	if likely(result == 0) {
		Dee_hash_vidx_t i;
		struct LOCAL_Dee_dict_item *vtab;
		vtab = LOCAL__DeeDict_GetRealVTab(self);
		ASSERT(self->d_vused == self->d_vsize);
		for (i = 0; i < self->d_vused; ++i) {
			struct LOCAL_Dee_dict_item *item = &vtab[i];
			Dee_Incref(item->di_key);
#ifndef DEFINE_DeeHashSet
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !DEFINE_DeeHashSet */
		}
	}
	return result;
}

#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromroset_noincref(LOCAL_Dict *__restrict self,
                                   DeeRoSetObject *__restrict other) {
	size_t i, tabssz;
	Dee_hash_hidxio_t hidxio;
	struct LOCAL_Dee_dict_item *dst_vtab;
	struct Dee_hashset_item *src_vtab;
	self->d_valloc  = other->rs_vsize;
	self->d_vsize   = other->rs_vsize;
	self->d_vused   = other->rs_vsize;
	self->d_hmask   = other->rs_hmask;
	ASSERT(self->d_valloc <= self->d_hmask);
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	ASSERT(other->rs_hidxget == Dee_hash_hidxio[hidxio].hxio_get);
	self->d_hidxops = &Dee_hash_hidxio[hidxio];
	tabssz = LOCAL_dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(self->d_hmask, self->d_valloc, hidxio);
	dst_vtab = (struct LOCAL_Dee_dict_item *)LOCAL__DeeDict_TabsMalloc(tabssz);
	if unlikely(!dst_vtab)
		goto err;

	/* Because they're binary-compatible, we can just copy data from "other" as-is.
	 * This will duplicate both the vtab, as well as the htab! */
	src_vtab = other->rs_vtab;
	for (i = 0; i < tabssz; ++i) {
		dst_vtab[i].di_hash = src_vtab[i].hsi_hash;
		dst_vtab[i].di_key  = src_vtab[i].hsi_key;
#ifndef DEFINE_DeeHashSet
		DBG_memset(&dst_vtab[i].di_value, 0xcc, sizeof(dst_vtab[i].di_value));
#endif /* !DEFINE_DeeHashSet */
	}

	LOCAL__DeeDict_SetRealVTab(self, dst_vtab);
	self->d_htab = (union Dee_hash_htab *)(dst_vtab + self->d_valloc);
#ifdef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* LOCAL_DICT_INITFROM_NEEDSLOCK */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_init_fromroset_keysonly(LOCAL_Dict *__restrict self,
                                   DeeRoSetObject *__restrict other) {
	int result = LOCAL_dict_init_fromroset_noincref(self, other);
	if likely(result == 0) {
		Dee_hash_vidx_t i;
		struct LOCAL_Dee_dict_item *vtab;
		vtab = LOCAL__DeeDict_GetRealVTab(self);
		ASSERT(self->d_vused == self->d_vsize);
		for (i = 0; i < self->d_vused; ++i) {
			struct LOCAL_Dee_dict_item *item = &vtab[i];
			Dee_Incref(item->di_key);
#ifndef DEFINE_DeeHashSet
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !DEFINE_DeeHashSet */
		}
	}
	return result;
}
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
#endif /* DEFINE_DeeDict */

PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_Dict *DCALL
LOCAL_dict_new_copy(LOCAL_Dict *__restrict self) {
	DREF LOCAL_Dict *result = DeeGCObject_MALLOC(LOCAL_Dict);
	if unlikely(!result)
		goto err;
	if unlikely(LOCAL_dict_init_fromcopy(result, self))
		goto err_r;
#ifndef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	DeeObject_InitStatic(result, &LOCAL_DeeDict_Type);
	return DeeGC_TRACK(LOCAL_Dict, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequence(DeeObject *__restrict self));
PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_FromSequence(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	DREF LOCAL_Dict *result;
	size_t hint;
	if (LOCAL_DeeDict_CheckExact(self)) {
		/* Special optimization when "self" is another dict:
		 * Optimize "self" and then duplicate its control structures */
		return Dee_AsObject(LOCAL_dict_new_copy((LOCAL_Dict *)self));
	}
	if (LOCAL_DeeRoDict_Check(self)) {
		/* Special optimization when "self" is an RoDict:
		 * Duplicate its control structures */
		return LOCAL_DeeDict_FromRoDict(self);
	}
	hint = DeeObject_SizeFast(self);
	if likely(hint != (size_t)-1) {
		result = (DREF LOCAL_Dict *)LOCAL_DeeDict_TryNewWithHint(hint);
	} else {
		result = (DREF LOCAL_Dict *)LOCAL_DeeDict_TryNewWithWeakHint(LOCAL_DICT_FROMSEQ_DEFAULT_HINT);
	}
	if unlikely(!result) {
		result = (DREF LOCAL_Dict *)LOCAL_DeeDict_New();
		if unlikely(!result)
			goto err;
	}
#ifdef DEFINE_DeeHashSet
	foreach_status = DeeObject_Foreach(self, &hashset_fromsequence_foreach_cb, result);
#else /* DEFINE_DeeHashSet */
	foreach_status = DeeObject_ForeachPair(self, &dict_fromsequence_foreach_cb, result);
#endif /* !DEFINE_DeeHashSet */
	if unlikely(foreach_status < 0)
		goto err_r;
	return Dee_AsObject(result);
err_r:
	Dee_DecrefDokill(result);
err:
	return NULL;
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self));
PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (LOCAL_DeeDict_CheckExact(self) && !DeeObject_IsShared(self))
		return self; /* Can re-use existing Dict object. */
	result = LOCAL_DeeDict_FromSequence(self);
	if likely(result)
		Dee_Decref_unlikely(self);
	return result;
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromRoDict(/*RoDict*/ DeeObject *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromRoSet(/*RoSet*/ DeeObject *__restrict self));
PUBLIC WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_FromRoDict(/*RoDict*/ DeeObject *__restrict self) {
	DREF LOCAL_Dict *result;
	ASSERT_OBJECT_TYPE_EXACT(self, &LOCAL_DeeRoDict_Type);
	result = DeeGCObject_MALLOC(LOCAL_Dict);
	if unlikely(!result)
		goto err;
	if unlikely(LOCAL_dict_init_fromrodict(result, (LOCAL_DeeRoDictObject *)self))
		goto err_r;
#ifndef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	DeeObject_InitStatic(result, &LOCAL_DeeDict_Type);
	return Dee_AsObject(DeeGC_TRACK(LOCAL_Dict, result));
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_dict_insert_items_inherited_after_first_duplicate(LOCAL_Dict *self, struct LOCAL_Dee_dict_item *item,
                                                        Dee_hash_hidx_t htab_idx, size_t num_items,
                                                        /*inherit(on_success)*/ DREF DeeObject **key_values) {
	/*virt*/Dee_hash_vidx_t vtab_idx;
	DREF DeeObject **dups;
	Dee_hash_t hash;
	struct Dee_objectlist duplicates;
	ASSERT(num_items > 0);
	Dee_objectlist_init(&duplicates);
	dups = Dee_objectlist_alloc(&duplicates, LOCAL_OBJECTS_PER_ITEM);
	if unlikely(!dups)
		goto err_duplicates;
	dups[0] = item->di_key;   /* Inherit reference (on_success) */
	item->di_key = NULL;      /* Deleted */
#ifndef DEFINE_DeeHashSet
	dups[1] = item->di_value; /* Inherit reference (on_success) */
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !DEFINE_DeeHashSet */
	hash = item->di_hash;
	ASSERTF(self->d_vsize < self->d_valloc, "DeeDict_NewWithHint() should have allocated enough space!");
	vtab_idx = Dee_hash_vidx_tovirt(self->d_vsize);
	/*++self->d_vused;*/ /* We're replacing a key, so this isn't being incremented! */
	++self->d_vsize;
	item = &LOCAL__DeeDict_GetVirtVTab(self)[vtab_idx];
	LOCAL__DeeDict_HTabSet(self, htab_idx, vtab_idx);
	item->di_hash  = hash;
	item->di_key   = *key_values++; /* Inherit reference */
#ifndef DEFINE_DeeHashSet
	item->di_value = *key_values++; /* Inherit reference */
#endif /* !DEFINE_DeeHashSet */
	--num_items;

	/* Insert remaining items (and keep colling references to overwritten keys/values) */
	while (num_items--) {
		Dee_hash_t hs, perturb;
		DREF DeeObject *key = *key_values++;
#ifndef DEFINE_DeeHashSet
		DREF DeeObject *value = *key_values++;
#endif /* !DEFINE_DeeHashSet */
		hash = DeeObject_Hash(key);
		for (LOCAL__DeeDict_HashIdxInit(self, &hs, &perturb, hash);;
		     LOCAL__DeeDict_HashIdxNext(self, &hs, &perturb, hash)) {
			int key_cmp;
			htab_idx = Dee_hash_hidx_ofhash(hs, self->d_hmask);
			vtab_idx = LOCAL__DeeDict_HTabGet(self, htab_idx);
			if likely(vtab_idx == Dee_HASH_HTAB_EOF)
				break;
			item = &LOCAL__DeeDict_GetVirtVTab(self)[vtab_idx];
			if likely(item->di_hash != hash)
				continue; /* Not what we're looking for... */
			if unlikely(!item->di_key)
				continue; /* Deleted key (highly unlikely, but *could* happen) */
			key_cmp = DeeObject_TryCompareEq(key, item->di_key);
			if (Dee_COMPARE_ISERR(key_cmp))
				goto err_duplicates;
			if likely(Dee_COMPARE_ISNE(key_cmp))
				continue;

			/* Another duplicate -> override it like before... */
			dups = Dee_objectlist_alloc(&duplicates, LOCAL_OBJECTS_PER_ITEM);
			if unlikely(!dups)
				goto err_duplicates;
			dups[0] = item->di_key;   /* Inherit reference */
			item->di_key = NULL;      /* Deleted */
#ifndef DEFINE_DeeHashSet
			dups[1] = item->di_value; /* Inherit reference */
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
#endif /* !DEFINE_DeeHashSet */

			ASSERTF(self->d_vsize < self->d_valloc, "DeeDict_NewWithHint() should have allocated enough space!");
			vtab_idx = Dee_hash_vidx_tovirt(self->d_vsize);
			/*++self->d_vused;*/ /* We're replacing a key, so this isn't being incremented! */
			goto account_size_and_append_item;
		}

		/* Append key/value-pair at the end of the vtab. */
		ASSERTF(self->d_vsize < self->d_valloc, "DeeDict_NewWithHint() should have allocated enough space!");
		vtab_idx = Dee_hash_vidx_tovirt(self->d_vsize);
		++self->d_vused;
account_size_and_append_item:
		++self->d_vsize;
		item = &LOCAL__DeeDict_GetVirtVTab(self)[vtab_idx];
		LOCAL__DeeDict_HTabSet(self, htab_idx, vtab_idx);
		item->di_hash = hash;
		item->di_key  = key;   /* Inherit reference */
#ifndef DEFINE_DeeHashSet
		item->di_value = value; /* Inherit reference */
#endif /* !DEFINE_DeeHashSet */
	}

	/* Drop all of the collected duplicate references */
	Dee_objectlist_fini(&duplicates);
	return 0;
err_duplicates:
	Dee_objectlist_elemv_free(duplicates.ol_elemv);
	return -1;
}

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_values: A vector containing `num_items*LOCAL_OBJECTS_PER_ITEM' objects,
 *                     even ones being keys and odd ones being items.
 * @param: num_items:  The number of key-value pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ references to its elements! */
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeDict_NewKeyValuesInherited(size_t num_items, /*inherit(on_success)*/ DREF DeeObject **key_values));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL DeeHashSet_NewItemsInherited(size_t num_keys, /*inherit(on_success)*/ DREF DeeObject **items));
PUBLIC WUNUSED DREF /*Dict*/ DeeObject *DCALL
LOCAL_DeeDict_NewKeyValuesInherited(size_t num_items,
                                    /*inherit(on_success)*/ DREF DeeObject **key_values) {
	size_t i;
	DREF LOCAL_Dict *result;
	result = (DREF LOCAL_Dict *)LOCAL_DeeDict_NewWithHint(num_items);
	if unlikely(!result)
		goto err;
	for (i = 0; i < num_items; ++i) {
		struct LOCAL_Dee_dict_item *item;
		DREF DeeObject *key = key_values[(i * LOCAL_OBJECTS_PER_ITEM) + 0];
#ifndef DEFINE_DeeHashSet
		DREF DeeObject *value = key_values[(i * LOCAL_OBJECTS_PER_ITEM) + 1];
#endif /* !DEFINE_DeeHashSet */
		Dee_hash_t hs, perturb, hash = DeeObject_Hash(key);
		Dee_hash_hidx_t htab_idx;
		Dee_hash_vidx_t vtab_idx; /*virt*/

		for (LOCAL__DeeDict_HashIdxInit(result, &hs, &perturb, hash);;
		     LOCAL__DeeDict_HashIdxNext(result, &hs, &perturb, hash)) {
			int key_cmp;
			htab_idx = Dee_hash_hidx_ofhash(hs, result->d_hmask);
			vtab_idx = LOCAL__DeeDict_HTabGet(result, htab_idx);
			if likely(vtab_idx == Dee_HASH_HTAB_EOF)
				break;
			item = &LOCAL__DeeDict_GetVirtVTab(result)[vtab_idx];
			if likely(item->di_hash != hash)
				continue; /* Not what we're looking for... */
			if unlikely(!item->di_key)
				continue; /* Deleted key (highly unlikely, but *could* happen) */
			key_cmp = DeeObject_TryCompareEq(key, item->di_key);
			if (Dee_COMPARE_ISERR(key_cmp))
				goto err_r;
			if likely(Dee_COMPARE_ISNE(key_cmp))
				continue;

			/* Damn it! there are duplicate keys in the caller-given items.
			 * -> Must keep track of those duplicates as we go... */
			key_values += i * LOCAL_OBJECTS_PER_ITEM;
			num_items -= i;
			if unlikely(LOCAL_dict_insert_items_inherited_after_first_duplicate(result, item, htab_idx,
			                                                                    num_items, key_values))
				goto err_r;
			LOCAL_dict_verify(result);
			goto done;
		}

		/* Append key/value-pair at the end of the vtab. */
		ASSERTF(result->d_vsize < result->d_valloc,
		        "" PP_STR(LOCAL_DeeDict_NewWithHint) "() "
		        "should have allocated enough space!");
		ASSERT(result->d_vused == result->d_vsize);
		vtab_idx = Dee_hash_vidx_tovirt(result->d_vsize);
		++result->d_vused;
		++result->d_vsize;
		item = &LOCAL__DeeDict_GetVirtVTab(result)[vtab_idx];
		LOCAL__DeeDict_HTabSet(result, htab_idx, vtab_idx);
		item->di_hash = hash;
		item->di_key  = key;   /* Inherit reference */
#ifndef DEFINE_DeeHashSet
		item->di_value = value; /* Inherit reference */
#endif /* !DEFINE_DeeHashSet */
	}
	LOCAL_dict_verify(result);
done:
	return Dee_AsObject(result);
err_r:
	/* Free "result" without inheriting references to already inserted key/value pairs */
	ASSERTF(LOCAL__DeeDict_GetVirtVTab(result) != LOCAL_DeeDict_EmptyVTab,
	        "That would mean 'num_items == 0', which it can't "
	        "be because then we wouldn't have gotten here");
	LOCAL__DeeDict_TabsFree(LOCAL__DeeDict_GetRealVTab(result));
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_fini(LOCAL_Dict *__restrict self) {
	if (LOCAL__DeeDict_GetVirtVTab(self) != LOCAL_DeeDict_EmptyVTab) {
		Dee_hash_vidx_t i;
		struct LOCAL_Dee_dict_item *vtab = LOCAL__DeeDict_GetRealVTab(self);
		for (i = 0; i < self->d_vsize; ++i) {
			if (vtab[i].di_key) {
				Dee_Decref(vtab[i].di_key);
#ifndef DEFINE_DeeHashSet
				Dee_Decref(vtab[i].di_value);
#endif /* !DEFINE_DeeHashSet */
			}
		}
		LOCAL__DeeDict_TabsFree(vtab);
	}
}

PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL
LOCAL_dict_visit(LOCAL_Dict *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_hash_vidx_t i;
	struct LOCAL_Dee_dict_item *vtab;
	LOCAL_DeeDict_LockRead(self);
	vtab = LOCAL__DeeDict_GetVirtVTab(self);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		if (vtab[i].di_key) {
			Dee_Visit(vtab[i].di_key);
#ifndef DEFINE_DeeHashSet
			Dee_Visit(vtab[i].di_value);
#endif /* !DEFINE_DeeHashSet */
		}
	}
	LOCAL_DeeDict_LockEndRead(self);
}


LOCAL NONNULL((1)) void DCALL
LOCAL_dict_initfrom_empty(LOCAL_Dict *__restrict self) {
	self->d_valloc  = 0;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	LOCAL__DeeDict_SetVirtVTab(self, LOCAL_DeeDict_EmptyVTab);
	self->d_hmask   = 0;
	self->d_hidxops = &Dee_hash_hidxio[0];
	self->d_htab    = LOCAL_DeeDict_EmptyHTab;
#ifdef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* LOCAL_DICT_INITFROM_NEEDSLOCK */
}

LOCAL NONNULL((1)) void DCALL
LOCAL_dict_initfrom_hint(LOCAL_Dict *__restrict self,
                         size_t num_items, bool allow_overalloc) {
	Dee_hash_t hmask;
	Dee_hash_vidx_t valloc;
	Dee_hash_hidxio_t hidxio;
	void *tabs;
	if unlikely(!num_items) {
init_empty:
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
		tabs   = LOCAL__DeeDict_TabsTryCalloc(tabssz);
		if unlikely(!tabs)
			goto init_empty;
	}
	self->d_valloc  = valloc;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	LOCAL__DeeDict_SetRealVTab(self, (struct LOCAL_Dee_dict_item *)tabs);
	self->d_hmask   = hmask;
	self->d_hidxops = &Dee_hash_hidxio[hidxio];
	self->d_htab    = (union Dee_hash_htab *)((struct LOCAL_Dee_dict_item *)tabs + valloc);
#ifdef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* LOCAL_DICT_INITFROM_NEEDSLOCK */
}

LOCAL WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_initfrom_seq(LOCAL_Dict *__restrict self, DeeObject *seq) {
	Dee_ssize_t foreach_status;
	size_t hint;

	/* Special optimization when "seq" is a Dict: Duplicate its control structures */
	if (LOCAL_DeeDict_CheckExact(seq))
		return LOCAL_dict_init_fromcopy(self, (LOCAL_Dict *)seq);

	/* Special optimization when "seq" is an RoDict: Duplicate its control structures */
	if (LOCAL_DeeRoDict_Check(seq))
		return LOCAL_dict_init_fromrodict(self, (LOCAL_DeeRoDictObject *)seq);

	hint = DeeObject_SizeFast(seq);
	if likely(hint != (size_t)-1) {
		LOCAL_dict_initfrom_hint(self, hint, false);
	} else {
		LOCAL_dict_initfrom_hint(self, LOCAL_DICT_FROMSEQ_DEFAULT_HINT, true);
	}
#ifdef DEFINE_DeeHashSet
	foreach_status = DeeObject_Foreach(seq, &hashset_fromsequence_foreach_cb, self);
#else /* DEFINE_DeeHashSet */
	foreach_status = DeeObject_ForeachPair(seq, &dict_fromsequence_foreach_cb, self);
#endif /* !DEFINE_DeeHashSet */
	if unlikely(foreach_status < 0)
		goto err_self;
	LOCAL_dict_verify(self);
	return 0;
err_self:
	LOCAL_dict_fini(self);
	return -1;
}



/* Minimal (mostly) Dict-compatible buffer for constructing temporary dict objects. */
#define LOCAL__DictBuffer_startoff offsetof(LOCAL_Dict, d_valloc)
#ifndef CONFIG_NO_THREADS
#define LOCAL__DictBuffer_endoff_WITHOUT_LOCK offsetof(LOCAL_Dict, d_lock)
#else /* !CONFIG_NO_THREADS */
#define LOCAL__DictBuffer_endoff_WITHOUT_LOCK offsetof(LOCAL_Dict, ob_weakrefs)
#endif /* CONFIG_NO_THREADS */
#ifdef LOCAL_DICT_INITFROM_NEEDSLOCK
#define LOCAL__DictBuffer_endoff_WITH_LOCK offsetof(LOCAL_Dict, ob_weakrefs)
#else /* LOCAL_DICT_INITFROM_NEEDSLOCK */
#define LOCAL__DictBuffer_endoff_WITH_LOCK LOCAL__DictBuffer_endoff_WITHOUT_LOCK
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
#define LOCAL__DictBuffer_NWORDS_WITH_LOCK    CEILDIV(LOCAL__DictBuffer_endoff_WITH_LOCK - LOCAL__DictBuffer_startoff, sizeof(uintptr_t))
#define LOCAL__DictBuffer_NWORDS_WITHOUT_LOCK CEILDIV(LOCAL__DictBuffer_endoff_WITHOUT_LOCK - LOCAL__DictBuffer_startoff, sizeof(uintptr_t))

typedef struct {
	uintptr_t db_data[LOCAL__DictBuffer_NWORDS_WITH_LOCK];
} LOCAL_DictBuffer;
#define LOCAL_DictBuffer_AsDict(self) ((LOCAL_Dict *)((byte_t *)((self)->db_data) - LOCAL__DictBuffer_startoff))
#define LOCAL_Dict_AsDictBuffer(self) ((LOCAL_DictBuffer *)((byte_t *)(self) + LOCAL__DictBuffer_startoff))
#define LOCAL_DictBuffer_Fini(self) \
	LOCAL_dict_fini(LOCAL_DictBuffer_AsDict(self))
#define LOCAL_DictBuffer_InitEmpty(self) \
	LOCAL_dict_initfrom_empty(LOCAL_DictBuffer_AsDict(self))
#define LOCAL_DictBuffer_InitWithHint(self, num_items, allow_overalloc) \
	LOCAL_dict_initfrom_hint(LOCAL_DictBuffer_AsDict(self), num_items, allow_overalloc)
#define LOCAL_DictBuffer_InitFromSequence(self, seq) \
	LOCAL_dict_initfrom_seq(LOCAL_DictBuffer_AsDict(self), seq)

/* Init "self" by stealing data from "dict"
 * CAUTION: WILL NOT INITIALIZE THE BUFFER'S LOCK WORD!!! */
#define LOCAL_DictBuffer_FromDict(self, dict) \
	memcpyp((self)->db_data, LOCAL_Dict_AsDictBuffer(dict), LOCAL__DictBuffer_NWORDS_WITHOUT_LOCK)

/* Transfer the contents of the buffer into "dict"
 * CAUTION: WILL OVERRIDE WHATEVER WAS IN "dict" BEFORE!!! */
#define LOCAL_DictBuffer_IntoDict(self, dict) \
	memcpyp(LOCAL_Dict_AsDictBuffer(dict), (self)->db_data, LOCAL__DictBuffer_NWORDS_WITHOUT_LOCK)


PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_assign(LOCAL_Dict *self, DeeObject *seq) {
	LOCAL_DictBuffer new_buffer;
	LOCAL_DictBuffer old_buffer;
	if unlikely(self == (LOCAL_Dict *)seq)
		return 0;
	if unlikely(LOCAL_DictBuffer_InitFromSequence(&new_buffer, seq))
		goto err;
	LOCAL_DeeDict_LockWrite(self);
	LOCAL_DictBuffer_FromDict(&old_buffer, self);
	LOCAL_DictBuffer_IntoDict(&new_buffer, self);
	LOCAL_DeeDict_LockEndWrite(self);
	LOCAL_DictBuffer_Fini(&old_buffer);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_moveassign(LOCAL_Dict *self, LOCAL_Dict *other) {
	if likely(self != other) {
		LOCAL_DictBuffer old_buffer;
		DeeLock_Acquire2(LOCAL_DeeDict_LockWrite(self), LOCAL_DeeDict_LockTryWrite(self), LOCAL_DeeDict_LockEndWrite(self),
		                 LOCAL_DeeDict_LockWrite(other), LOCAL_DeeDict_LockTryWrite(other), LOCAL_DeeDict_LockEndWrite(other));
		LOCAL_DictBuffer_FromDict(&old_buffer, self);
		LOCAL_DictBuffer_IntoDict(LOCAL_Dict_AsDictBuffer(other), self);
		LOCAL_DictBuffer_InitEmpty(LOCAL_Dict_AsDictBuffer(other));
		LOCAL_DeeDict_LockEndWrite(other);
		LOCAL_DeeDict_LockEndWrite(self);
		LOCAL_DictBuffer_Fini(&old_buffer);
	}
	return 0;
}

#undef LOCAL__DictBuffer_startoff
#undef LOCAL__DictBuffer_endoff_WITHOUT_LOCK
#undef LOCAL__DictBuffer_endoff_WITH_LOCK
#undef LOCAL__DictBuffer_NWORDS_WITH_LOCK
#undef LOCAL__DictBuffer_NWORDS_WITHOUT_LOCK
#undef LOCAL_DictBuffer_AsDict
#undef LOCAL_Dict_AsDictBuffer
#undef LOCAL_DictBuffer_Fini
#undef LOCAL_DictBuffer_InitEmpty
#undef LOCAL_DictBuffer_InitWithHint
#undef LOCAL_DictBuffer_InitFromSequence
#undef LOCAL_DictBuffer_FromDict
#undef LOCAL_DictBuffer_IntoDict


PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_ctor(LOCAL_Dict *__restrict self) {
#ifndef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(self);
	LOCAL_dict_initfrom_empty(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_copy(LOCAL_Dict *__restrict self,
                LOCAL_Dict *__restrict other) {
#ifndef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(self);
	return LOCAL_dict_init_fromcopy(self, other);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_init(LOCAL_Dict *__restrict self,
                size_t argc, DeeObject *const *argv) {
#ifndef LOCAL_DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&self->d_lock);
#endif /* !LOCAL_DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(self);
	switch (argc) {

	case 1: {
		DeeObject *arg = argv[0];
		if (DeeInt_Check(arg)) {
			size_t hint;
			if unlikely(DeeInt_AsSize(arg, &hint))
				goto err;
			LOCAL_dict_initfrom_hint(self, hint, false);
			return 0;
		}
		return LOCAL_dict_initfrom_seq(self, arg);
	}	break;

	case 2: {
		int weak;
		size_t hint;
		if unlikely(DeeObject_AsSize(argv[0], &hint))
			goto err;
		weak = DeeObject_Bool(argv[1]);
		if unlikely(weak < 0)
			goto err;
		LOCAL_dict_initfrom_hint(self, hint, !!weak);
		return 0;
	}	break;

	default: break;
	}
	return err_invalid_argc(LOCAL_STR_Dict, argc, 1, 2);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_dict_serialize(LOCAL_Dict *__restrict self,
                     DeeSerial *__restrict writer,
                     Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(LOCAL_Dict, field))
	LOCAL_Dict *out;
	struct Dee_hash_hidxio_ops const *out__d_hidxops;
	Dee_hash_vidx_t self__d_valloc;
again:
	out = DeeSerial_Addr2Mem(writer, addr, LOCAL_Dict);
	LOCAL_DeeDict_LockRead(self);
	out->d_valloc = self__d_valloc = self->d_valloc;
	out->d_vsize  = self->d_vsize;
	out->d_vused  = self->d_vused;
	out->d_hmask  = self->d_hmask;
	Dee_weakref_support_init(out);
	out__d_hidxops = self->d_hidxops; /* Relocated later... */
	Dee_atomic_rwlock_init(&out->d_lock);
	if (self->d_vtab == LOCAL_DeeDict_EmptyVTab) {
		ASSERT(self->d_htab == LOCAL_DeeDict_EmptyHTab);
		LOCAL_DeeDict_LockEndRead(self);
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(d_vtab), LOCAL_DeeDict_EmptyVTab))
			goto err;
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(d_htab), LOCAL_DeeDict_EmptyHTab))
			goto err;
	} else {
		Dee_hash_vidx_t i, out__d_vsize;
		Dee_seraddr_t addrof_out__d_vtab;
		size_t sizeof_out__d_vtab;
		struct LOCAL_Dee_dict_item *out__d_vtab;
		struct LOCAL_Dee_dict_item *in__d_vtab;
		Dee_hash_hidxio_t hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self__d_valloc);
		sizeof_out__d_vtab = self__d_valloc * sizeof(struct LOCAL_Dee_dict_item);
		sizeof_out__d_vtab += (self->d_hmask + 1) << hidxio;
		addrof_out__d_vtab = DeeSerial_TryMalloc(writer, sizeof_out__d_vtab, NULL);
		if (!Dee_SERADDR_ISOK(addrof_out__d_vtab)) {
			LOCAL_DeeDict_LockEndRead(self);
			addrof_out__d_vtab = DeeSerial_Malloc(writer, sizeof_out__d_vtab, NULL);
			if (!Dee_SERADDR_ISOK(addrof_out__d_vtab))
				goto err;
			LOCAL_DeeDict_LockRead(self);
			if unlikely(self__d_valloc != self->d_valloc) {
free_out__d_vtab__and__again:
				LOCAL_DeeDict_LockEndRead(self);
				DeeSerial_Free(writer, addrof_out__d_vtab, NULL);
				goto again;
			}
			out = DeeSerial_Addr2Mem(writer, addr, LOCAL_Dict);
			if unlikely(out->d_vsize != self->d_vsize)
				goto free_out__d_vtab__and__again;
			if unlikely(out->d_vused != self->d_vused)
				goto free_out__d_vtab__and__again;
			if unlikely(out->d_hmask != self->d_hmask)
				goto free_out__d_vtab__and__again;
			if unlikely(out__d_hidxops != self->d_hidxops)
				goto free_out__d_vtab__and__again;
			if unlikely(self->d_vtab == LOCAL_DeeDict_EmptyVTab)
				goto free_out__d_vtab__and__again;
		}
		out          = DeeSerial_Addr2Mem(writer, addr, LOCAL_Dict);
		out__d_vsize = out->d_vsize;
		out__d_vtab  = DeeSerial_Addr2Mem(writer, addrof_out__d_vtab, struct LOCAL_Dee_dict_item);
		in__d_vtab   = LOCAL__DeeDict_GetRealVTab(self);
		memcpy(out__d_vtab, in__d_vtab, sizeof_out__d_vtab);
		for (i = 0; i < out__d_vsize; ++i) {
			if (out__d_vtab[i].di_key) {
				Dee_Incref(out__d_vtab[i].di_key);
#ifndef DEFINE_DeeHashSet
				Dee_Incref(out__d_vtab[i].di_value);
#endif /* !DEFINE_DeeHashSet */
			}
		}
		LOCAL_DeeDict_LockEndRead(self);
		for (i = 0; i < out__d_vsize; ++i) {
			if (out__d_vtab[i].di_key) {
				int error;
				Dee_seraddr_t addrof_out__d_vtab_i = addrof_out__d_vtab + i * sizeof(struct LOCAL_Dee_dict_item);
				DREF DeeObject *key = out__d_vtab[i].di_key;
#ifndef DEFINE_DeeHashSet
				DREF DeeObject *value = out__d_vtab[i].di_value;
#endif /* !DEFINE_DeeHashSet */
				error = DeeSerial_PutObjectInherited(writer, addrof_out__d_vtab_i + offsetof(struct LOCAL_Dee_dict_item, di_key), key);
#ifndef DEFINE_DeeHashSet
				if likely(error == 0)
					error = DeeSerial_PutObject(writer, addrof_out__d_vtab_i + offsetof(struct LOCAL_Dee_dict_item, di_value), value);
				Dee_Decref_unlikely(value);
#endif /* !DEFINE_DeeHashSet */
				out__d_vtab = DeeSerial_Addr2Mem(writer, addrof_out__d_vtab, struct LOCAL_Dee_dict_item);
				if unlikely(error) {
					for (; i < out__d_vsize; ++i) {
						if (out__d_vtab[i].di_key) {
							Dee_Decref_unlikely(out__d_vtab[i].di_key);
#ifndef DEFINE_DeeHashSet
							Dee_Decref_unlikely(out__d_vtab[i].di_value);
#endif /* !DEFINE_DeeHashSet */
						}
					}
					goto err;
				}
			}
		}
		if (DeeSerial_PutAddr(writer, ADDROF(d_vtab),
		                      addrof_out__d_vtab - sizeof(struct LOCAL_Dee_dict_item)))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(d_htab),
		                      addrof_out__d_vtab +
		                      (self__d_valloc * sizeof(struct LOCAL_Dee_dict_item))))
			goto err;
	}
	return DeeSerial_PutStaticDeemon(writer, ADDROF(d_hidxops), (void *)out__d_hidxops);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_dict_printrepr(LOCAL_Dict *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	/*virt*/ struct LOCAL_Dee_dict_item *vtab;
	/*virt*/ Dee_hash_vidx_t i;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, PP_STR(LOCAL_Dict) "({ ");
	if unlikely(result < 0)
		return result;
	is_first = true;
	LOCAL_DeeDict_LockRead(self);
	vtab = LOCAL__DeeDict_GetVirtVTab(self);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *key;
#ifndef DEFINE_DeeHashSet
		DREF DeeObject *value;
#endif /* !DEFINE_DeeHashSet */
		key = vtab[i].di_key;
		if (key == NULL)
			continue;
		Dee_Incref(key);
#ifndef DEFINE_DeeHashSet
		value = vtab[i].di_value;
		Dee_Incref(value);
#endif /* !DEFINE_DeeHashSet */
		LOCAL_DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
#ifdef DEFINE_DeeHashSet
		temp = DeeFormat_Printf(printer, arg, "%s%R", is_first ? "" : ", ", key);
#else /* DEFINE_DeeHashSet */
		temp = DeeFormat_Printf(printer, arg, "%s%R: %R", is_first ? "" : ", ", key, value);
#endif /* !DEFINE_DeeHashSet */
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		is_first = false;
		LOCAL_DeeDict_LockRead(self);
	}
	LOCAL_DeeDict_LockEndRead(self);
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
	return result;
err_temp:
	return temp;
}

PRIVATE NONNULL((1)) int DCALL
LOCAL_dict_mh_clear(LOCAL_Dict *__restrict self) {
	Dee_hash_vidx_t i, vsize;
	struct LOCAL_Dee_dict_item *vtab;
	LOCAL_DeeDict_LockWrite(self);
	if (!self->d_vsize) {
		LOCAL_DeeDict_LockEndWrite(self);
		return 0;
	}
	vtab  = LOCAL__DeeDict_GetRealVTab(self);
	vsize = self->d_vsize;
	self->d_valloc  = 0;
	self->d_vsize   = 0;
	self->d_vused   = 0;
	LOCAL__DeeDict_SetVirtVTab(self, LOCAL_DeeDict_EmptyVTab);
	self->d_hmask   = 0;
	self->d_hidxops = &Dee_hash_hidxio[0];
	self->d_htab    = LOCAL_DeeDict_EmptyHTab;
	LOCAL_DeeDict_LockEndWrite(self);
	ASSERTF(vtab != (struct LOCAL_Dee_dict_item *)_DeeHash_EmptyTab,
	        "Should have been handled by '!self->" PP_STR(d_vsize) "' above");
	for (i = 0; i < vsize; ++i) {
		if (vtab[i].di_key) {
			Dee_Decref(vtab[i].di_key);
#ifndef DEFINE_DeeHashSet
			Dee_Decref(vtab[i].di_value);
#endif /* !DEFINE_DeeHashSet */
		}
	}
	LOCAL__DeeDict_TabsFree(vtab);
	return 0;
}



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
	LOCAL_DeeDict_LockEndWrite(self);
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
	LOCAL_DeeDict_LockEndWrite(self);
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
		LOCAL_DeeDict_LockEndRead(dict);
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
#endif /* DEFINE_HIGH_LEVEL */

#undef LOCAL_OBJECTS_PER_ITEM

#undef LOCAL_DICT_INITFROM_NEEDSLOCK
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
#undef LOCAL_dict_init_fromrodict_noincref
#undef LOCAL_dict_init_fromrodict
#undef LOCAL_dict_init_fromrodict_keysonly
#undef LOCAL_dict_init_fromroset_noincref
#undef LOCAL_dict_init_fromroset_keysonly
#undef LOCAL_dict_new_copy
#undef LOCAL_DeeDict_FromSequence
#undef LOCAL_DeeDict_FromSequenceInheritedOnSuccess
#undef LOCAL_DeeDict_FromRoDict
#undef LOCAL_dict_insert_items_inherited_after_first_duplicate
#undef LOCAL_DeeDict_NewKeyValuesInherited
#undef LOCAL_dict_fini
#undef LOCAL_dict_visit
#undef LOCAL_dict_initfrom_empty
#undef LOCAL_dict_initfrom_hint
#undef LOCAL_dict_initfrom_seq
#undef LOCAL_DictBuffer
#undef LOCAL_dict_assign
#undef LOCAL_dict_moveassign
#undef LOCAL_dict_ctor
#undef LOCAL_dict_copy
#undef LOCAL_dict_init
#undef LOCAL_dict_serialize
#undef LOCAL_dict_printrepr
#undef LOCAL_dict_mh_clear
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
#undef LOCAL_dict_htab_reverse
#undef LOCAL_DeeRoDictObject
#undef LOCAL_dict_init_fromcopy
#undef LOCAL_DeeDict_CheckExact
#undef LOCAL_DeeRoDict_Check
#undef LOCAL_DeeDict_New
#undef LOCAL_DeeRoDict_Type
#undef LOCAL_dict_verify
#undef LOCAL_STR_Dict
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
#undef rd_vsize
#undef rd_hmask
#undef rd_hidxget
#undef rd_htab
#undef rd_vtab
#ifdef __INTELLISENSE__
#undef Dee_dict_item
#endif /* __INTELLISENSE__ */
#endif /* DEFINE_DeeHashSet */

DECL_END

#undef DEFINE_DeeDict
#undef DEFINE_DeeHashSet
#undef DEFINE_LOW_LEVEL
#undef DEFINE_HIGH_LEVEL
