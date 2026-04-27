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
#define DEFINE_DeeRoDict
//#define DEFINE_DeeRoSet
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_Unpack1 */
#include <deemon/dict.h>            /* DeeDictObject, DeeDict_*, Dee_dict_item, _DeeDict_GetRealVTab */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/hashset.h>         /* DeeHashSetObject, DeeHashSet_*, Dee_hashset_item */
#include <deemon/object.h>          /* ASSERT_OBJECT_TYPE, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ISEQ, Dee_COMPARE_ISERR, Dee_Decref, Dee_Decref_unlikely, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/rodict.h>          /* DeeRoDict*, Dee_EmptyRoDict, Dee_RODICT_BUILDER_INIT, Dee_empty_rodict_object, Dee_rodict_builder*, Dee_rodict_object, _DeeRoDict_* */
#include <deemon/roset.h>           /* DeeRoSet*, Dee_EmptyRoSet, Dee_ROSET_BUILDER_INIT, Dee_empty_roset_object, Dee_roset_builder, Dee_roset_builder_cinit, Dee_roset_builder_fini, Dee_roset_builder_init, Dee_roset_builder_init_with_hint, Dee_roset_builder_insert, Dee_roset_builder_insert_inherited, Dee_roset_builder_insertall, Dee_roset_builder_pack, Dee_roset_object, _DeeRoSet_GetRealVTab, _DeeRoSet_GetVirtVTab, _DeeRoSet_HashIdxInit, _DeeRoSet_HashIdxNext */
#include <deemon/serial.h>          /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/system-features.h> /* bzero*, memcpy, memmovedownc, mempcpyc */
#include <deemon/type.h>            /* DeeObject_InitStatic, Dee_Visit, Dee_visit_t */
#include <deemon/util/hash-io.h>    /* Dee_HASH_HIDXIO_FROM_VALLOC, Dee_HASH_HTAB_EOF, Dee_hash_* */

#include "seq/default-map-proxy.h"

#include <stdbool.h> /* false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t */

#if (defined(DEFINE_DeeRoDict) + \
     defined(DEFINE_DeeRoSet)) != 1
#error "Must #define exactly one of these"
#endif /* ... */


#ifdef DEFINE_DeeRoDict
#ifdef __INTELLISENSE__
#include "rodict.c"
#endif /* __INTELLISENSE__ */

#define LOCAL_RoDict                               RoDict
#define LOCAL_Dee_rodict_object                    Dee_rodict_object
#define LOCAL_Dee_dict_item                        Dee_dict_item
#define LOCAL_DeeRoDict_IsEmpty                    DeeRoDict_IsEmpty
#define LOCAL_DeeRoDict_Type                       DeeRoDict_Type
#define LOCAL_DeeRoDict_Check                      DeeRoDict_Check
#define LOCAL_DeeRoDict_CheckExact                 DeeRoDict_CheckExact
#define LOCAL_DeeRoDict_FromSequence               DeeRoDict_FromSequence
#define LOCAL_DeeRoDict_FromDict                   DeeRoDict_FromDict
#define LOCAL_DeeRoDict_EmptyInstance              DeeRoDict_EmptyInstance
#define LOCAL_DeeDict_Check                        DeeDict_Check
#define LOCAL_DeeDict_CheckExact                   DeeDict_CheckExact
#define LOCAL_DeeDictObject                        DeeDictObject
#define LOCAL_DeeDict_Type                         DeeDict_Type
#define LOCAL_DeeDict_LockReadAndOptimize          DeeDict_LockReadAndOptimize
#define LOCAL_DeeDict_LockEndRead                  DeeDict_LockEndRead
#define LOCAL_Dee_rodict_builder                   Dee_rodict_builder
#define LOCAL_Dee_RODICT_BUILDER_INIT              Dee_RODICT_BUILDER_INIT
#define LOCAL_Dee_rodict_builder_init              Dee_rodict_builder_init
#define LOCAL_Dee_rodict_builder_cinit             Dee_rodict_builder_cinit
#define LOCAL_Dee_rodict_builder_fini              Dee_rodict_builder_fini
#define LOCAL_Dee_rodict_builder_init_with_hint    Dee_rodict_builder_init_with_hint
#define LOCAL_Dee_rodict_builder_pack              Dee_rodict_builder_pack
#define LOCAL_Dee_rodict_builder_setitem           Dee_rodict_builder_setitem
#define LOCAL_Dee_rodict_builder_setitem_inherited Dee_rodict_builder_setitem_inherited
#define LOCAL_Dee_rodict_builder_update            Dee_rodict_builder_update
#define LOCAL__DeeRoDict_HashIdxInit               _DeeRoDict_HashIdxInit
#define LOCAL__DeeRoDict_HashIdxNext               _DeeRoDict_HashIdxNext
#define LOCAL__DeeRoDict_GetVirtVTab               _DeeRoDict_GetVirtVTab
#define LOCAL__DeeRoDict_GetRealVTab               _DeeRoDict_GetRealVTab
#define LOCAL__RoDict_TryMalloc                    _RoDict_TryMalloc
#define LOCAL__RoDict_TryCalloc                    _RoDict_TryCalloc
#define LOCAL__RoDict_TryRealloc                   _RoDict_TryRealloc
#define LOCAL__RoDict_Malloc                       _RoDict_Malloc
#define LOCAL__RoDict_Calloc                       _RoDict_Calloc
#define LOCAL__RoDict_Realloc                      _RoDict_Realloc
#define LOCAL__RoDict_Free                         _RoDict_Free
#define LOCAL__RoDict_SizeOf                       _RoDict_SizeOf
#define LOCAL__RoDict_SafeSizeOf                   _RoDict_SafeSizeOf
#define LOCAL__RoDict_SizeOf3                      _RoDict_SizeOf3
#define LOCAL__RoDict_SafeSizeOf3                  _RoDict_SafeSizeOf3
#define LOCAL_Dee_empty_rodict_object              Dee_empty_rodict_object
#define LOCAL_DeeRoDict_EmptyInstance              DeeRoDict_EmptyInstance
#define LOCAL_Dee_EmptyRoDict                      Dee_EmptyRoDict
#define LOCAL_rodict_verify                        rodict_verify
/*...................................................................................................*/
#define LOCAL_rodict_htab_decafter                 rodict_htab_decafter
#define LOCAL_rodict_htab_rebuild                  rodict_htab_rebuild
#define LOCAL_rodict_trunc_vtab                    rodict_trunc_vtab
#define LOCAL_rodict_builder_first_setitem         rodict_builder_first_setitem
#define LOCAL_rodict_builder_grow                  rodict_builder_grow
#define LOCAL_rodict_from_generic_sequence         rodict_from_generic_sequence
/*...................................................................................................*/
#define LOCAL_rodict_ctor                          rodict_ctor
#define LOCAL_rodict_init                          rodict_init
#define LOCAL_rodict_fini                          rodict_fini
#define LOCAL_rodict_visit                         rodict_visit
#define LOCAL_rodict_serialize                     rodict_serialize
#define LOCAL_rodict_printrepr                     rodict_printrepr
#else /* DEFINE_DeeRoDict */
#ifdef __INTELLISENSE__
#include "roset.c"
#endif /* __INTELLISENSE__ */

#define LOCAL_RoDict                               RoSet
#define LOCAL_Dee_rodict_object                    Dee_roset_object
#define LOCAL_Dee_dict_item                        Dee_hashset_item
#define LOCAL_DeeRoDict_IsEmpty                    DeeRoSet_IsEmpty
#define LOCAL_DeeRoDict_Type                       DeeRoSet_Type
#define LOCAL_DeeRoDict_Check                      DeeRoSet_Check
#define LOCAL_DeeRoDict_CheckExact                 DeeRoSet_CheckExact
#define LOCAL_DeeRoDict_FromSequence               DeeRoSet_FromSequence
#define LOCAL_DeeRoDict_FromDict                   DeeRoSet_FromHashSet
#define LOCAL_DeeRoDict_EmptyInstance              DeeRoSet_EmptyInstance
#define LOCAL_DeeDict_Check                        DeeHashSet_Check
#define LOCAL_DeeDict_CheckExact                   DeeHashSet_CheckExact
#define LOCAL_DeeDictObject                        DeeHashSetObject
#define LOCAL_DeeDict_Type                         DeeHashSet_Type
#define LOCAL_DeeDict_LockReadAndOptimize          DeeHashSet_LockReadAndOptimize
#define LOCAL_DeeDict_LockEndRead                  DeeHashSet_LockEndRead
#define LOCAL_Dee_rodict_builder                   Dee_roset_builder
#define LOCAL_Dee_RODICT_BUILDER_INIT              Dee_ROSET_BUILDER_INIT
#define LOCAL_Dee_rodict_builder_init              Dee_roset_builder_init
#define LOCAL_Dee_rodict_builder_cinit             Dee_roset_builder_cinit
#define LOCAL_Dee_rodict_builder_fini              Dee_roset_builder_fini
#define LOCAL_Dee_rodict_builder_init_with_hint    Dee_roset_builder_init_with_hint
#define LOCAL_Dee_rodict_builder_pack              Dee_roset_builder_pack
#define LOCAL_Dee_rodict_builder_setitem           Dee_roset_builder_insert
#define LOCAL_Dee_rodict_builder_setitem_inherited Dee_roset_builder_insert_inherited
#define LOCAL_Dee_rodict_builder_update            Dee_roset_builder_insertall
#define LOCAL__DeeRoDict_HashIdxInit               _DeeRoSet_HashIdxInit
#define LOCAL__DeeRoDict_HashIdxNext               _DeeRoSet_HashIdxNext
#define LOCAL__DeeRoDict_GetVirtVTab               _DeeRoSet_GetVirtVTab
#define LOCAL__DeeRoDict_GetRealVTab               _DeeRoSet_GetRealVTab
#define LOCAL__RoDict_TryMalloc                    _RoSet_TryMalloc
#define LOCAL__RoDict_TryCalloc                    _RoSet_TryCalloc
#define LOCAL__RoDict_TryRealloc                   _RoSet_TryRealloc
#define LOCAL__RoDict_Malloc                       _RoSet_Malloc
#define LOCAL__RoDict_Calloc                       _RoSet_Calloc
#define LOCAL__RoDict_Realloc                      _RoSet_Realloc
#define LOCAL__RoDict_Free                         _RoSet_Free
#define LOCAL__RoDict_SizeOf                       _RoSet_SizeOf
#define LOCAL__RoDict_SafeSizeOf                   _RoSet_SafeSizeOf
#define LOCAL__RoDict_SizeOf3                      _RoSet_SizeOf3
#define LOCAL__RoDict_SafeSizeOf3                  _RoSet_SafeSizeOf3
#define LOCAL_Dee_empty_rodict_object              Dee_empty_roset_object
#define LOCAL_DeeRoDict_EmptyInstance              DeeRoSet_EmptyInstance
#define LOCAL_Dee_EmptyRoDict                      Dee_EmptyRoSet
#define LOCAL_rodict_verify                        roset_verify
/*...................................................................................................*/
#define LOCAL_rodict_htab_decafter                 roset_htab_decafter
#define LOCAL_rodict_htab_rebuild                  roset_htab_rebuild
#define LOCAL_rodict_trunc_vtab                    roset_trunc_vtab
#define LOCAL_rodict_builder_first_setitem         roset_builder_first_setitem
#define LOCAL_rodict_builder_grow                  roset_builder_grow
#define LOCAL_rodict_from_generic_sequence         roset_from_generic_sequence
/*...................................................................................................*/
#define LOCAL_rodict_ctor                          roset_ctor
#define LOCAL_rodict_init                          roset_init
#define LOCAL_rodict_fini                          roset_fini
#define LOCAL_rodict_visit                         roset_visit
#define LOCAL_rodict_serialize                     roset_serialize
#define LOCAL_rodict_printrepr                     roset_printrepr
/*...................................................................................................*/
#define rd_vsize                                   rs_vsize
#define rd_hmask                                   rs_hmask
#define rd_hidxget                                 rs_hidxget
#define rd_htab                                    rs_htab
#define rd_vtab                                    rs_vtab
#define rd_htab_data                               rs_htab_data
#define di_hash                                    hsi_hash
#define di_key                                     hsi_key
#define rdb_dict                                   rsb_set
#define rdb_valloc                                 rsb_valloc
#define rdb_hidxset                                rsb_hidxset
#define d_valloc                                   hs_valloc
#define d_vsize                                    hs_vsize
#define d_vused                                    hs_vused
#define d_vtab                                     hs_vtab
#define d_hmask                                    hs_hmask
#define d_hidxops                                  hs_hidxops
#define d_htab                                     hs_htab
#endif /* !DEFINE_DeeRoDict */

DECL_BEGIN

PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL
LOCAL_rodict_htab_decafter(LOCAL_RoDict *__restrict me, Dee_hash_sethidx_t hidxset,
                           /*virt*/ Dee_hash_vidx_t vtab_threshold) {
	Dee_hash_t i;
	Dee_hash_gethidx_t hidxget = me->rd_hidxget;
	for (i = 0; i <= me->rd_hmask; ++i) {
		/*virt*/Dee_hash_vidx_t vtab_idx;
		vtab_idx = (*hidxget)(me->rd_htab, i);
		if (vtab_idx >= vtab_threshold) {
			--vtab_idx;
			(*hidxset)(me->rd_htab, i, vtab_idx);
		}
	}
}

/* Re-build the htab (the caller must first bzero it) */
PRIVATE NONNULL((1)) void DCALL
LOCAL_rodict_htab_rebuild(LOCAL_RoDict *__restrict me, Dee_hash_sethidx_t hidxset) {
	/*real*/Dee_hash_vidx_t i;
	Dee_hash_gethidx_t hidxget = me->rd_hidxget;
	for (i = 0; i < me->rd_vsize; ++i) {
		Dee_hash_t hs, perturb;
		struct LOCAL_Dee_dict_item *item = &LOCAL__DeeRoDict_GetRealVTab(me)[i];
		for (LOCAL__DeeRoDict_HashIdxInit(me, &hs, &perturb, item->di_hash);;
		     LOCAL__DeeRoDict_HashIdxNext(me, &hs, &perturb, item->di_hash)) {
			Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, me->rd_hmask);
			Dee_hash_vidx_t vtab_idx = (*hidxget)(me->rd_htab, htab_idx); /*virt*/
			if unlikely(vtab_idx != Dee_HASH_HTAB_EOF)
				continue;
			(*hidxset)(me->rd_htab, htab_idx, Dee_hash_vidx_tovirt(i));
			break;
		}
	}
}








/************************************************************************/
/* RODICT BUILDER API                                                   */
/************************************************************************/
Dee_HIDDEN_IMPL(PUBLIC NONNULL((1)) void DCALL Dee_rodict_builder_fini(struct Dee_rodict_builder *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC NONNULL((1)) void DCALL Dee_roset_builder_fini(struct Dee_roset_builder *__restrict self));
PUBLIC NONNULL((1)) void DCALL
LOCAL_Dee_rodict_builder_fini(struct LOCAL_Dee_rodict_builder *__restrict self) {
	size_t i;
	LOCAL_RoDict *dict = self->rdb_dict;
	if unlikely(!dict)
		return;
	for (i = 0; i < dict->rd_vsize; ++i) {
		struct LOCAL_Dee_dict_item *item;
		item = &LOCAL__DeeRoDict_GetRealVTab(dict)[i];
#ifndef DEFINE_DeeRoSet
		Dee_Decref(item->di_value);
#endif /* !DEFINE_DeeRoSet */
		Dee_Decref(item->di_key);
	}
	LOCAL__RoDict_Free(dict);
}

Dee_HIDDEN_IMPL(PUBLIC NONNULL((1)) void DCALL Dee_rodict_builder_init_with_hint(struct Dee_rodict_builder *__restrict self, size_t num_items));
Dee_HIDDEN_IMPL(PUBLIC NONNULL((1)) void DCALL Dee_roset_builder_init_with_hint(struct Dee_roset_builder *__restrict self, size_t num_items));
PUBLIC NONNULL((1)) void DCALL
LOCAL_Dee_rodict_builder_init_with_hint(struct LOCAL_Dee_rodict_builder *__restrict self,
                                        size_t num_items) {
	LOCAL_RoDict *dict;
	size_t sizeof_dict;
	size_t hmask;
	Dee_hash_hidxio_t hidxio;
	hidxio      = Dee_HASH_HIDXIO_FROM_VALLOC(num_items);
	hmask       = dict_hmask_from_count(num_items);
	sizeof_dict = LOCAL__RoDict_SafeSizeOf3(num_items, hmask, hidxio);
	dict        = LOCAL__RoDict_TryMalloc(sizeof_dict);
	if unlikely(!dict) {
		hmask       = dict_tiny_hmask_from_count(num_items);
		sizeof_dict = LOCAL__RoDict_SafeSizeOf3(num_items, hmask, hidxio);
		dict        = LOCAL__RoDict_TryMalloc(sizeof_dict);
		if unlikely(!dict) {
			LOCAL_Dee_rodict_builder_init(self);
			return;
		}
	}
	dict->rd_vsize   = 0;
	dict->rd_hmask   = hmask;
	dict->rd_hidxget = Dee_hash_hidxio[hidxio].hxio_get;
	dict->rd_htab    = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(dict) + num_items);
	bzero(dict->rd_htab, (hmask + 1) << hidxio);
	self->rdb_dict    = dict;
	self->rdb_valloc  = num_items;
	self->rdb_hidxset = Dee_hash_hidxio[hidxio].hxio_set;
}

/* Truncate the vtab of "self" to "new_vsize" elements.
 *
 * Dropped elements are NOT finalized (if they were ever
 * initialized to begin with), and if the htab changes
 * word size as a result, it will be truncated accordingly. */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_RoDict *DCALL
LOCAL_rodict_trunc_vtab(LOCAL_RoDict *__restrict self, size_t old_valloc, size_t new_valloc) {
	LOCAL_RoDict *result;
	union Dee_hash_htab *old_htab, *new_htab;
	Dee_hash_hidxio_t old_hidxio, new_hidxio;
	size_t new_sizeof;
	ASSERT(new_valloc < old_valloc);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(old_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	old_htab   = self->rd_htab;
	new_htab   = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(self) + new_valloc);
	ASSERT(old_htab >= (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(self) + self->rd_vsize)); /* ">=" because of over-allocation */
	hmask_memmovedown_and_maybe_downcast(new_htab, new_hidxio,
	                                     old_htab, old_hidxio,
	                                     self->rd_hmask + 1);
	self->rd_hidxget = Dee_hash_hidxio[new_hidxio].hxio_get;
	new_sizeof = LOCAL__RoDict_SizeOf3(new_valloc, self->rd_hmask, new_hidxio);
	result = LOCAL__RoDict_TryRealloc(self, new_sizeof);
	if unlikely(!result)
		result = self;
	result->rd_htab = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(result) + new_valloc);
	return result;
}

/* Pack the result of the builder and return it.
 * This function never fails, but "self" becomes invalid as a result. */
Dee_HIDDEN_IMPL(PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeRoDictObject *DCALL Dee_rodict_builder_pack(struct Dee_rodict_builder *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeRoSetObject *DCALL Dee_roset_builder_pack(struct Dee_roset_builder *__restrict self));
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF LOCAL_RoDict *DCALL
LOCAL_Dee_rodict_builder_pack(struct LOCAL_Dee_rodict_builder *__restrict self) {
	LOCAL_RoDict *result = self->rdb_dict;
	if unlikely(!result) {
		ASSERT(self->rdb_valloc == 0);
		result = (DREF LOCAL_RoDict *)LOCAL_Dee_EmptyRoDict;
		Dee_Incref(result);
	} else {
		ASSERT(result->rd_vsize <= self->rdb_valloc);
		/* Free unused memory if there are unused buffers. */
		if unlikely(result->rd_vsize < self->rdb_valloc)
			result = LOCAL_rodict_trunc_vtab(result, self->rdb_valloc, result->rd_vsize);
		DeeObject_InitStatic(result, &LOCAL_DeeRoDict_Type);
	}
	DBG_memset(self, 0xcc, sizeof(struct LOCAL_Dee_rodict_builder));
	LOCAL_rodict_verify(result);
	return result;
}

#ifdef DEFINE_DeeRoSet
PRIVATE ATTR_NOINLINE NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_rodict_builder_first_setitem(struct LOCAL_Dee_rodict_builder *me,
                                   /*inherit(always)*/ DREF DeeObject *key)
#else /* DEFINE_DeeRoSet */
PRIVATE ATTR_NOINLINE NONNULL((1, 2, 3)) Dee_ssize_t DCALL
LOCAL_rodict_builder_first_setitem(struct LOCAL_Dee_rodict_builder *me,
                                   /*inherit(always)*/ DREF DeeObject *key,
                                   /*inherit(always)*/ DREF DeeObject *value)
#endif /* !DEFINE_DeeRoSet */
{
	LOCAL_RoDict *dict;
	size_t valloc, hmask;
	ASSERT(!me->rdb_dict);
	valloc = DICT_FROMSEQ_DEFAULT_HINT;
	hmask  = dict_hmask_from_count(valloc);
	ASSERT(Dee_HASH_HIDXIO_FROM_VALLOC(valloc) == 0);
	dict = LOCAL__RoDict_TryMalloc(LOCAL__RoDict_SizeOf3(valloc, hmask, 0));
	if unlikely(!dict) {
		valloc = 1;
		hmask  = dict_tiny_hmask_from_count(valloc);
		ASSERT(Dee_HASH_HIDXIO_FROM_VALLOC(valloc) == 0);
		dict = LOCAL__RoDict_Malloc(LOCAL__RoDict_SizeOf3(valloc, hmask, 0));
		if unlikely(!dict)
			goto err;
	}
	me->rdb_valloc   = valloc;
	me->rdb_dict     = dict;
	me->rdb_hidxset  = &Dee_hash_sethidx8;
	dict->rd_vsize   = 1;
	dict->rd_hmask   = hmask;
	dict->rd_hidxget = &Dee_hash_gethidx8;
	dict->rd_htab    = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(dict) + valloc);
	bzeroc(dict->rd_htab, hmask + 1, sizeof(uint8_t));
	dict->rd_vtab[0].di_hash  = DeeObject_Hash(key);
	dict->rd_vtab[0].di_key   = key;   /* Inherit reference */
#ifndef DEFINE_DeeRoSet
	dict->rd_vtab[0].di_value = value; /* Inherit reference */
#endif /* !DEFINE_DeeRoSet */
	((uint8_t *)dict->rd_htab)[dict->rd_vtab[0].di_hash & hmask] = Dee_hash_vidx_tovirt(0);
	return 0;
err:
#ifndef DEFINE_DeeRoSet
	Dee_Decref_unlikely(value);
#endif /* !DEFINE_DeeRoSet */
	Dee_Decref_unlikely(key);
	return -1;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) int DCALL
LOCAL_rodict_builder_grow(struct LOCAL_Dee_rodict_builder *__restrict me) {
	size_t min_valloc, new_sizeof;
	LOCAL_RoDict *old_dict, *new_dict;
	size_t old_hmask, new_hmask;
	size_t old_valloc, new_valloc;
	Dee_hash_hidxio_t old_hidxio, new_hidxio;

	/* Calculate desired change in buffer size (s.a. `dict_trygrow_vtab_and_htab_with()') */
	old_dict = me->rdb_dict;
	ASSERT(old_dict);
	min_valloc = old_dict->rd_vsize + 1;
	old_hmask  = old_dict->rd_hmask;
	new_hmask  = dict_hmask_from_count(min_valloc);
	if unlikely(new_hmask < old_hmask)
		new_hmask = old_hmask;
	if (new_hmask <= old_hmask && _DeeDict_ShouldGrowHTab2(old_dict->rd_vsize, old_dict->rd_hmask))
		new_hmask = (new_hmask << 1) | 1;
	old_valloc = me->rdb_valloc;
	new_valloc = dict_valloc_from_hmask_and_count(new_hmask, min_valloc, true);
	old_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(old_valloc);
	new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);

	/* Do the allocation. */
	new_sizeof = LOCAL__RoDict_SafeSizeOf3(new_valloc, new_hmask, new_hidxio);
	new_dict   = LOCAL__RoDict_TryRealloc(old_dict, new_sizeof);
	if unlikely(!new_dict) {
		/* Try again with a smaller buffer size */
		new_hmask  = dict_tiny_hmask_from_count(min_valloc);
		new_valloc = dict_valloc_from_hmask_and_count(new_hmask, min_valloc, false);
		new_hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_sizeof = LOCAL__RoDict_SafeSizeOf3(new_valloc, new_hmask, new_hidxio);
		new_dict   = LOCAL__RoDict_Realloc(old_dict, new_sizeof);
		if unlikely(!new_dict)
			goto err;
	}

	/* Write-back results. */
	me->rdb_dict         = new_dict;
	me->rdb_valloc       = new_valloc;
	me->rdb_hidxset      = Dee_hash_hidxio[new_hidxio].hxio_set;
	new_dict->rd_hidxget = Dee_hash_hidxio[new_hidxio].hxio_get;
	new_dict->rd_hmask   = new_hmask;
	new_dict->rd_htab    = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(new_dict) + new_valloc);

	ASSERT(new_hmask >= old_hmask);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	if (new_hmask == old_hmask) {
		/* Move/scale the htab so it appears in the correct position. */
		union Dee_hash_htab *old_htab = (union Dee_hash_htab *)(LOCAL__DeeRoDict_GetRealVTab(new_dict) + old_valloc);
		union Dee_hash_htab *new_htab = new_dict->rd_htab;
		if likely(old_hidxio == new_hidxio) {
			(*Dee_hash_hidxio[old_hidxio].hxio_movup)(new_htab, old_htab, new_hmask + 1);
		} else {
			(*Dee_hash_hidxio[old_hidxio].hxio_upr)(new_htab, old_htab, new_hmask + 1);
		}
	} else {
		/* Rebuild the htab */
		bzero(new_dict->rd_htab, (new_hmask + 1) << new_hidxio);
		LOCAL_rodict_htab_rebuild(new_dict, me->rdb_hidxset);
	}
	return 0;
err:
	return -1;
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL Dee_rodict_builder_setitem_inherited(/*struct Dee_rodict_builder*/ void *__restrict self, /*inherit(always)*/ DREF DeeObject *key, /*inherit(always)*/ DREF DeeObject *value));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL Dee_roset_builder_insert_inherited(/*struct Dee_roset_builder*/ void *__restrict self, /*inherit(always)*/ DREF DeeObject *key));
#ifdef DEFINE_DeeRoSet
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_t" */
LOCAL_Dee_rodict_builder_setitem_inherited(/*struct LOCAL_Dee_rodict_builder*/ void *__restrict self,
                                           /*inherit(always)*/ DREF DeeObject *key)
#else /* DEFINE_DeeRoSet */
PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
LOCAL_Dee_rodict_builder_setitem_inherited(/*struct LOCAL_Dee_rodict_builder*/ void *__restrict self,
                                           /*inherit(always)*/ DREF DeeObject *key,
                                           /*inherit(always)*/ DREF DeeObject *value)
#endif /* !DEFINE_DeeRoSet */
{
	LOCAL_RoDict *dict;
	Dee_hash_hidx_t result_htab_idx;
	struct LOCAL_Dee_dict_item *item;
	Dee_hash_t hash, hs, perturb;
	struct LOCAL_Dee_rodict_builder *me;
	me   = (struct LOCAL_Dee_rodict_builder *)self;
	dict = me->rdb_dict;
	if unlikely(!dict) {
#ifdef DEFINE_DeeRoSet
		return LOCAL_rodict_builder_first_setitem(me, key);
#else /* DEFINE_DeeRoSet */
		return LOCAL_rodict_builder_first_setitem(me, key, value);
#endif /* !DEFINE_DeeRoSet */
	}
	ASSERT(dict->rd_vsize <= me->rdb_valloc);

	/* Check if the key already exists (and if so: override it) */
	hash = DeeObject_Hash(key);
	for (LOCAL__DeeRoDict_HashIdxInit(dict, &hs, &perturb, hash);;
	     LOCAL__DeeRoDict_HashIdxNext(dict, &hs, &perturb, hash)) {
		int key_cmp;
		Dee_hash_vidx_t vtab_idx; /*virt*/
		result_htab_idx = Dee_hash_hidx_ofhash(hs, dict->rd_hmask);
		vtab_idx = (*dict->rd_hidxget)(dict->rd_htab, result_htab_idx);
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break; /* EOF */
		item = &LOCAL__DeeRoDict_GetVirtVTab(dict)[vtab_idx];
		if (item->di_hash != hash)
			continue;
		key_cmp = DeeObject_TryCompareEq(key, item->di_key);
		if (Dee_COMPARE_ISERR(key_cmp))
			goto err;
		if (Dee_COMPARE_ISEQ(key_cmp)) {
			/* Found the key -> override it */
			ASSERT(Dee_hash_vidx_toreal(vtab_idx) <= dict->rd_vsize - 1);
			Dee_Decref(item->di_key);
#ifndef DEFINE_DeeRoSet
			Dee_Decref(item->di_value);
#endif /* !DEFINE_DeeRoSet */
			if (Dee_hash_vidx_toreal(vtab_idx) != dict->rd_vsize - 1) {
				/* Must down-shift the old location. */
				size_t n_after = (dict->rd_vsize - 1) - Dee_hash_vidx_toreal(vtab_idx);
				ASSERT(n_after > 0);
				memmovedownc(item, item + 1, n_after, sizeof(struct LOCAL_Dee_dict_item));
				LOCAL_rodict_htab_decafter(dict, me->rdb_hidxset, vtab_idx /*+ 1*/); /* +1 doesn't matter here */
				item = &LOCAL__DeeRoDict_GetRealVTab(dict)[dict->rd_vsize - 1];
				item->di_hash = hash;
				(*me->rdb_hidxset)(dict->rd_htab, result_htab_idx, Dee_hash_vidx_tovirt(dict->rd_vsize - 1));
			}
			ASSERT(item->di_hash == hash);
			item->di_key   = key;   /* Inherit reference */
#ifndef DEFINE_DeeRoSet
			item->di_value = value; /* Inherit reference */
#endif /* !DEFINE_DeeRoSet */
			return 0;
		}
	}

	/* Insert the key/value pair into the dict. */
	if unlikely(dict->rd_vsize >= me->rdb_valloc) {
		size_t old_hmask = dict->rd_hmask;
		if unlikely(LOCAL_rodict_builder_grow(me))
			goto err;
		dict = me->rdb_dict;
		if (dict->rd_hmask != old_hmask) {
			/* Must re-discover the end of the relevant hash-chain */
			for (LOCAL__DeeRoDict_HashIdxInit(dict, &hs, &perturb, hash);;
			     LOCAL__DeeRoDict_HashIdxNext(dict, &hs, &perturb, hash)) {
				result_htab_idx = Dee_hash_hidx_ofhash(hs, dict->rd_hmask);
				if ((*dict->rd_hidxget)(dict->rd_htab, result_htab_idx) == Dee_HASH_HTAB_EOF)
					break; /* EOF */
			}
		}
	}
	ASSERT(dict->rd_vsize < me->rdb_valloc);

	/* Append new item at the end of the vtab (and link it in the htab) */
	item = &LOCAL__DeeRoDict_GetRealVTab(dict)[dict->rd_vsize];
	item->di_hash  = hash;
	item->di_key   = key;   /* Inherit reference */
#ifndef DEFINE_DeeRoSet
	item->di_value = value; /* Inherit reference */
#endif /* !DEFINE_DeeRoSet */
	(*me->rdb_hidxset)(dict->rd_htab, result_htab_idx, Dee_hash_vidx_tovirt(dict->rd_vsize));
	++dict->rd_vsize;
	return 0;
err:
#ifndef DEFINE_DeeRoSet
	Dee_Decref_unlikely(value);
#endif /* !DEFINE_DeeRoSet */
	Dee_Decref_unlikely(key);
	return -1;
}


Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL Dee_rodict_builder_setitem(/*struct Dee_rodict_builder*/ void *__restrict self, DeeObject *key, DeeObject *value));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL Dee_roset_builder_insert(/*struct Dee_roset_builder*/ void *__restrict self, DeeObject *key));
#ifdef DEFINE_DeeRoSet
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_t" */
LOCAL_Dee_rodict_builder_setitem(/*struct LOCAL_Dee_rodict_builder*/ void *__restrict self,
                                 DeeObject *key) {
	Dee_Incref(key);
	return LOCAL_Dee_rodict_builder_setitem_inherited(self, key);
}
#else /* DEFINE_DeeRoSet */
PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
LOCAL_Dee_rodict_builder_setitem(/*struct LOCAL_Dee_rodict_builder*/ void *__restrict self,
                                 DeeObject *key, DeeObject *value) {
	Dee_Incref(key);
	Dee_Incref(value);
	return LOCAL_Dee_rodict_builder_setitem_inherited(self, key, value);
}
#endif /* !DEFINE_DeeRoSet */


PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_RoDict *DCALL
LOCAL_rodict_from_generic_sequence(DeeObject *__restrict self) {
	size_t hint;
	struct LOCAL_Dee_rodict_builder builder;
	hint = DeeObject_SizeFast(self);
	if (hint != (size_t)-1) {
		LOCAL_Dee_rodict_builder_init_with_hint(&builder, hint);
	} else {
		LOCAL_Dee_rodict_builder_init(&builder);
	}
	if unlikely(LOCAL_Dee_rodict_builder_update(&builder, self))
		goto err;
	return LOCAL_Dee_rodict_builder_pack(&builder);
err:
	LOCAL_Dee_rodict_builder_fini(&builder);
	return NULL;
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*RoDict*/ DeeObject *DCALL DeeRoDict_FromSequence(DeeObject *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*RoSet*/ DeeObject *DCALL DeeRoSet_FromSequence(DeeObject *__restrict self));
PUBLIC WUNUSED NONNULL((1)) DREF /*RoDict*/ DeeObject *DCALL
LOCAL_DeeRoDict_FromSequence(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self == &LOCAL_DeeRoDict_Type)
		return_reference_(self);
	if (tp_self == &LOCAL_DeeDict_Type)
		return LOCAL_DeeRoDict_FromDict(self);
#ifdef DEFINE_DeeRoSet
	if (tp_self == &DefaultSequence_MapKeys_Type) {
		DefaultSequence_MapProxy *proxy = (DefaultSequence_MapProxy *)self;
		DeeObject *mapping_of_keys = proxy->dsmp_map;
		DeeTypeObject *tp_mapping_of_keys = Dee_TYPE(mapping_of_keys);
		if (tp_mapping_of_keys == &DeeDict_Type)
			return Dee_AsObject(roset_from_dict_keys((DeeDictObject *)mapping_of_keys));
		if (tp_mapping_of_keys == &DeeRoDict_Type)
			return Dee_AsObject(roset_from_rodict_keys((DeeRoDictObject *)mapping_of_keys));
	}
#endif /* DEFINE_DeeRoSet */
	return Dee_AsObject(LOCAL_rodict_from_generic_sequence(self));
}

Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*RoDict*/ DeeObject *DCALL DeeRoDict_FromDict(/*Dict*/ DeeObject *__restrict self));
Dee_HIDDEN_IMPL(PUBLIC WUNUSED NONNULL((1)) DREF /*RoSet*/ DeeObject *DCALL DeeRoSet_FromHashSet(/*HashSet*/ DeeObject *__restrict self));
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_DeeRoDict_FromDict(/*Dict*/ DeeObject *__restrict self) {
	DREF LOCAL_RoDict *result;
	size_t sizeof_result;
	size_t vsize;
	size_t i, hmask;
	Dee_hash_hidxio_t src_hidxio;
	Dee_hash_hidxio_t dst_hidxio;
	LOCAL_DeeDictObject *me = (LOCAL_DeeDictObject *)self;
	ASSERT_OBJECT_TYPE(me, &LOCAL_DeeDict_Type);
again:
	LOCAL_DeeDict_LockReadAndOptimize(me);
	vsize         = me->d_vused;
	hmask         = me->d_hmask;
	src_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(me->d_valloc);
	dst_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(vsize);
	sizeof_result = LOCAL__RoDict_SizeOf3(vsize, hmask, dst_hidxio);
	result = LOCAL__RoDict_TryMalloc(sizeof_result);
	if unlikely(!result) {
		LOCAL_DeeDict_LockEndRead(me);
		result = LOCAL__RoDict_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		LOCAL_DeeDict_LockReadAndOptimize(me);
		if unlikely(vsize != me->d_vused ||
		            hmask != me->d_hmask) {
			LOCAL_DeeDict_LockEndRead(me);
			LOCAL__RoDict_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rd_htab = (union Dee_hash_htab *)mempcpyc(LOCAL__DeeRoDict_GetRealVTab(result),
	                                                  _DeeDict_GetRealVTab(me), vsize,
	                                                  sizeof(struct LOCAL_Dee_dict_item));
	hmask_memcpy_and_maybe_downcast(result->rd_htab, dst_hidxio,
	                                me->d_htab, src_hidxio,
	                                hmask + 1);
	for (i = 0; i < vsize; ++i) {
		struct LOCAL_Dee_dict_item *item;
		item = &LOCAL__DeeRoDict_GetRealVTab(result)[i];
		ASSERT(item->di_key);
		Dee_Incref(item->di_key);
#ifndef DEFINE_DeeRoSet
		Dee_Incref(item->di_value);
#endif /* !DEFINE_DeeRoSet */
	}
	LOCAL_DeeDict_LockEndRead(me);
	result->rd_vsize   = vsize;
	result->rd_hmask   = hmask;
	result->rd_hidxget = Dee_hash_hidxio[dst_hidxio].hxio_get;
	DeeObject_InitStatic(result, &LOCAL_DeeRoDict_Type);
	LOCAL_rodict_verify(result);
	return Dee_AsObject(result);
err:
	return NULL;
}




Dee_HIDDEN_IMPL(PUBLIC struct Dee_empty_rodict_object DeeRoDict_EmptyInstance =);
Dee_HIDDEN_IMPL(PUBLIC struct Dee_empty_roset_object DeeRoSet_EmptyInstance =);
PUBLIC struct LOCAL_Dee_empty_rodict_object LOCAL_DeeRoDict_EmptyInstance = {
	OBJECT_HEAD_INIT(&LOCAL_DeeRoDict_Type),
	/* .rd_vsize     = */ 0,
	/* .rd_hmask     = */ 0,
	/* .rd_hidxget   = */ &Dee_hash_gethidx8,
	/* .rd_htab      = */ (union Dee_hash_htab *)LOCAL_DeeRoDict_EmptyInstance.rd_htab_data,
	/* .rd_vtab      = */ /*{},*/
	/* .rd_htab_data = */ { 0 },
};



PRIVATE WUNUSED DREF LOCAL_RoDict *DCALL LOCAL_rodict_ctor(void) {
	return_reference_((DREF LOCAL_RoDict *)LOCAL_Dee_EmptyRoDict);
}

PRIVATE WUNUSED DREF LOCAL_RoDict *DCALL
LOCAL_rodict_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "_" PP_STR(LOCAL_RoDict), &seq);
	return (DREF LOCAL_RoDict *)LOCAL_DeeRoDict_FromSequence(seq);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
LOCAL_rodict_fini(LOCAL_RoDict *__restrict self) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct LOCAL_Dee_dict_item *item;
		item = &LOCAL__DeeRoDict_GetRealVTab(self)[i];
		Dee_Decref(item->di_key);
#ifndef DEFINE_DeeRoSet
		Dee_Decref(item->di_value);
#endif /* !DEFINE_DeeRoSet */
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
LOCAL_rodict_visit(LOCAL_RoDict *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct LOCAL_Dee_dict_item *item;
		item = &LOCAL__DeeRoDict_GetRealVTab(self)[i];
		Dee_Visit(item->di_key);
#ifndef DEFINE_DeeRoSet
		Dee_Visit(item->di_value);
#endif /* !DEFINE_DeeRoSet */
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
LOCAL_rodict_serialize(LOCAL_RoDict *__restrict self, DeeSerial *__restrict writer) {
	LOCAL_RoDict *out;
	size_t i, sizeof_dict;
	size_t sizeof__rd_htab;
	Dee_seraddr_t addrof_out;
	byte_t *out__rd_htab;
	Dee_hash_hidxio_t hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->rd_vsize);
	sizeof__rd_htab = (self->rd_hmask + 1) << hidxio;
	sizeof_dict = LOCAL__RoDict_SizeOf3(self->rd_vsize, self->rd_hmask, hidxio);
	addrof_out  = DeeSerial_Object_Malloc(writer, sizeof_dict, self);
	if (!Dee_SERADDR_ISOK(addrof_out))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addrof_out, LOCAL_RoDict);
	out->rd_vsize = self->rd_vsize;
	out->rd_hmask = self->rd_hmask;
	out__rd_htab = (byte_t *)(out->rd_vtab + self->rd_vsize);
	memcpy(out__rd_htab, self->rd_htab, sizeof__rd_htab);
	for (i = 0; i < self->rd_vsize; ++i) {
		struct LOCAL_Dee_dict_item *out_item;
		struct LOCAL_Dee_dict_item *in_item;
		Dee_seraddr_t addrof_item;
		addrof_item = addrof_out + offsetof(LOCAL_RoDict, rd_vtab) +
		              i * sizeof(struct LOCAL_Dee_dict_item);
		out_item = DeeSerial_Addr2Mem(writer, addrof_item, struct LOCAL_Dee_dict_item);
		in_item  = &self->rd_vtab[i];
		if (in_item->di_key) {
			out_item->di_hash = in_item->di_hash;
			if (DeeSerial_PutObject(writer,
			                        addrof_item + offsetof(struct LOCAL_Dee_dict_item, di_key),
			                        in_item->di_key))
				goto err;
#ifndef DEFINE_DeeRoSet
			if (DeeSerial_PutObject(writer,
			                        addrof_item + offsetof(struct LOCAL_Dee_dict_item, di_value),
			                        in_item->di_value))
				goto err;
#endif /* !DEFINE_DeeRoSet */
		} else {
			out_item->di_hash  = 0;
			out_item->di_key   = NULL;
#ifndef DEFINE_DeeRoSet
			out_item->di_value = NULL;
#endif /* !DEFINE_DeeRoSet */
		}
	}
	if (DeeSerial_PutStaticDeemon(writer, addrof_out + offsetof(LOCAL_RoDict, rd_hidxget),
	                              (void *)self->rd_hidxget))
		goto err;
	if (DeeSerial_PutAddr(writer, addrof_out + offsetof(LOCAL_RoDict, rd_htab),
	                      addrof_out + offsetof(LOCAL_RoDict, rd_vtab) +
	                      self->rd_vsize * sizeof(struct LOCAL_Dee_dict_item)))
		goto err;
	return addrof_out;
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_rodict_printrepr(LOCAL_RoDict *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
#ifdef DEFINE_DeeRoSet
#define LOCAL_RODICT_REPR_EMPTY "HashSet.Frozen()"
#else /* DEFINE_DeeRoSet */
#define LOCAL_RODICT_REPR_EMPTY "Dict.Frozen()"
#endif /* !DEFINE_DeeRoSet */
#define LOCAL_RODICT_REPR_HEAD  "{ "
#define LOCAL_RODICT_REPR_TAIL  " }.frozen"
	/*real*/Dee_hash_vidx_t i;
	Dee_ssize_t temp, result;
	if unlikely(!self->rd_vsize)
		return DeeFormat_PRINT(printer, arg, LOCAL_RODICT_REPR_EMPTY);
	result = DeeFormat_PRINT(printer, arg, LOCAL_RODICT_REPR_HEAD);
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct LOCAL_Dee_dict_item *item;
		if (i) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		item = &LOCAL__DeeRoDict_GetRealVTab(self)[i];
#ifdef DEFINE_DeeRoSet
		temp = DeeObject_PrintRepr(item->di_key, printer, arg);
#else /* DEFINE_DeeRoSet */
		temp = DeeFormat_Printf(printer, arg, "%r: %r", item->di_key, item->di_value);
#endif /* !DEFINE_DeeRoSet */
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	temp = DeeFormat_PRINT(printer, arg, LOCAL_RODICT_REPR_TAIL);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
#undef LOCAL_RODICT_REPR_EMPTY
#undef LOCAL_RODICT_REPR_HEAD
#undef LOCAL_RODICT_REPR_TAIL
}





DECL_END

#undef LOCAL_RoDict
#undef LOCAL_Dee_rodict_object
#undef LOCAL_Dee_dict_item
#undef LOCAL_DeeRoDict_IsEmpty
#undef LOCAL_DeeRoDict_Type
#undef LOCAL_DeeRoDict_Check
#undef LOCAL_DeeRoDict_CheckExact
#undef LOCAL_DeeRoDict_FromSequence
#undef LOCAL_DeeRoDict_FromDict
#undef LOCAL_DeeRoDict_EmptyInstance
#undef LOCAL_DeeDict_Check
#undef LOCAL_DeeDict_CheckExact
#undef LOCAL_DeeDictObject
#undef LOCAL_DeeDict_Type
#undef LOCAL_DeeDict_LockReadAndOptimize
#undef LOCAL_DeeDict_LockEndRead
#undef LOCAL_Dee_rodict_builder
#undef LOCAL_Dee_RODICT_BUILDER_INIT
#undef LOCAL_Dee_rodict_builder_init
#undef LOCAL_Dee_rodict_builder_cinit
#undef LOCAL_Dee_rodict_builder_fini
#undef LOCAL_Dee_rodict_builder_init_with_hint
#undef LOCAL_Dee_rodict_builder_pack
#undef LOCAL_Dee_rodict_builder_setitem
#undef LOCAL_Dee_rodict_builder_setitem_inherited
#undef LOCAL_Dee_rodict_builder_update
#undef LOCAL__DeeRoDict_HashIdxInit
#undef LOCAL__DeeRoDict_HashIdxNext
#undef LOCAL__DeeRoDict_GetVirtVTab
#undef LOCAL__DeeRoDict_GetRealVTab
#undef LOCAL__RoDict_TryMalloc
#undef LOCAL__RoDict_TryCalloc
#undef LOCAL__RoDict_TryRealloc
#undef LOCAL__RoDict_Malloc
#undef LOCAL__RoDict_Calloc
#undef LOCAL__RoDict_Realloc
#undef LOCAL__RoDict_Free
#undef LOCAL__RoDict_SizeOf
#undef LOCAL__RoDict_SafeSizeOf
#undef LOCAL__RoDict_SizeOf3
#undef LOCAL__RoDict_SafeSizeOf3
#undef LOCAL_Dee_empty_rodict_object
#undef LOCAL_DeeRoDict_EmptyInstance
#undef LOCAL_Dee_EmptyRoDict
#undef LOCAL_rodict_verify
/*...................................................................................................*/
#undef LOCAL_rodict_htab_decafter
#undef LOCAL_rodict_htab_rebuild
#undef LOCAL_rodict_trunc_vtab
#undef LOCAL_rodict_builder_first_setitem
#undef LOCAL_rodict_builder_grow
#undef LOCAL_rodict_from_generic_sequence
/*...................................................................................................*/
#undef LOCAL_rodict_ctor
#undef LOCAL_rodict_init
#undef LOCAL_rodict_fini
#undef LOCAL_rodict_visit
#undef LOCAL_rodict_serialize
#undef LOCAL_rodict_printrepr
#ifdef DEFINE_DeeRoSet
#undef rd_vsize
#undef rd_hmask
#undef rd_hidxget
#undef rd_htab
#undef rd_vtab
#undef rd_htab_data
#undef di_hash
#undef di_key
#undef rdb_dict
#undef rdb_valloc
#undef rdb_hidxset
#undef d_valloc
#undef d_vsize
#undef d_vused
#undef d_vtab
#undef d_hmask
#undef d_hidxops
#undef d_htab
#endif /* DEFINE_DeeRoSet */

#undef DEFINE_DeeRoDict
#undef DEFINE_DeeRoSet
