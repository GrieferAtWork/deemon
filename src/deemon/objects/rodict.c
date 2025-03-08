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
#ifndef GUARD_DEEMON_OBJECTS_RODICT_C
#define GUARD_DEEMON_OBJECTS_RODICT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/dict.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>
#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/default-compare.h"
#include "seq/default-map-proxy.h"
/**/

#include "dict.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

#ifdef __OPTIMIZE_SIZE__
#define NULL_IF_Os(v) NULL
#else /* __OPTIMIZE_SIZE__ */
#define NULL_IF_Os(v) v
#endif /* !__OPTIMIZE_SIZE__ */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeRoDictObject RoDict;

/************************************************************************/
/* RODICT HELPERS                                                       */
/************************************************************************/


#ifdef DICT_NDEBUG
#define rodict_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
rodict_verify(RoDict *__restrict self) {
	size_t i, real_vused;
	shift_t hidxio;
	ASSERT(self->rd_vsize <= self->rd_hmask);
	ASSERT(IS_POWER_OF_TWO(self->rd_hmask + 1));
	ASSERT(self->rd_htab == _DeeRoDict_GetRealVTab(self) + self->rd_vsize);
	hidxio = DEE_DICT_HIDXIO_FROMALLOC(self->rd_vsize);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < DEE_DICT_HIDXIO_COUNT);
	ASSERT(self->rd_hidxget == Dee_dict_hidxio[hidxio].dhxio_get);
	for (i = Dee_dict_vidx_tovirt(0), real_vused = 0;
	     Dee_dict_vidx_virt_lt_real(i, self->rd_vsize); ++i) {
		struct Dee_dict_item *item = &_DeeRoDict_GetVirtVTab(self)[i];
		if (item->di_key) {
			ASSERT_OBJECT(item->di_key);
			ASSERT_OBJECT(item->di_value);
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->rd_vsize,
	        "RODICT: vtab key count=%" PRFuSIZ " differs from rd_vsize=%" PRFuSIZ,
	        real_vused, self->rd_vsize);
	for (i = 0; i <= self->rd_hmask; ++i) {
		Dee_dict_vidx_t vidx;
		vidx = (*self->rd_hidxget)(self->rd_htab, i);
		if (vidx == Dee_DICT_HTAB_EOF)
			continue;
		Dee_dict_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->rd_vsize,
		        "RODICT: htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->rd_vsize);
	}
	for (i = 0;; ++i) {
		Dee_dict_vidx_t vidx;
		ASSERTF(i <= self->rd_hmask, "RODICT: htab contains no EOF pointers (infinite loop would occur on non-present item lookup)");
		vidx = (*self->rd_hidxget)(self->rd_htab, i);
		if (vidx == Dee_DICT_HTAB_EOF)
			break;
	}
	for (i = Dee_dict_vidx_tovirt(0), real_vused = 0;
	     Dee_dict_vidx_virt_lt_real(i, self->rd_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item = &_DeeRoDict_GetVirtVTab(self)[i];
		if (!item->di_key)
			continue;
		_DeeRoDict_HashIdxInit(self, &hs, &perturb, item->di_hash);
		for (;; _DeeRoDict_HashIdxAdv(self, &hs, &perturb)) {
			Dee_dict_vidx_t vidx;
			struct Dee_dict_item *hitem;
			vidx = _DeeRoDict_HTabGet(self, hs);
			ASSERTF(vidx != Dee_DICT_HTAB_EOF,
			        "RODICT: End-of-hash-chain[hash:%#" PRFxSIZ "] before item idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r:%r> was found",
			        item->di_hash, Dee_dict_vidx_toreal(i), self->rd_vsize,
			        item->di_key, item->di_value);
			hitem = &_DeeRoDict_GetVirtVTab(self)[vidx];
			if (hitem == item)
				break;
		}
	}
}
#endif /* !DICT_NDEBUG */


PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL
rodict_htab_decafter(RoDict *__restrict me, Dee_dict_sethidx_t hidxset,
                     /*virt*/Dee_dict_vidx_t vtab_threshold) {
	Dee_hash_t i;
	Dee_dict_gethidx_t hidxget = me->rd_hidxget;
	for (i = 0; i <= me->rd_hmask; ++i) {
		/*virt*/Dee_dict_vidx_t vtab_idx;
		vtab_idx = (*hidxget)(me->rd_htab, i);
		if (vtab_idx >= vtab_threshold) {
			--vtab_idx;
			(*hidxset)(me->rd_htab, i, vtab_idx);
		}
	}
}

/* Re-build the htab (the caller must first bzero it) */
PRIVATE NONNULL((1)) void DCALL
rodict_htab_rebuild(RoDict *__restrict me, Dee_dict_sethidx_t hidxset) {
	/*real*/Dee_dict_vidx_t i;
	Dee_dict_gethidx_t hidxget = me->rd_hidxget;
	for (i = 0; i < me->rd_vsize; ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item = &_DeeRoDict_GetRealVTab(me)[i];
		_DeeRoDict_HashIdxInit(me, &hs, &perturb, item->di_hash);
		for (;; _DeeRoDict_HashIdxAdv(me, &hs, &perturb)) {
			/*virt*/Dee_dict_vidx_t vtab_idx;
			size_t htab_idx = hs & me->rd_hmask;
			vtab_idx = (*hidxget)(me->rd_htab, htab_idx);
			if unlikely(vtab_idx != Dee_DICT_HTAB_EOF)
				continue;
			(*hidxset)(me->rd_htab, htab_idx, Dee_dict_vidx_tovirt(i));
			break;
		}
	}
}








/************************************************************************/
/* RODICT BUILDER API                                                   */
/************************************************************************/
PUBLIC NONNULL((1)) void DCALL
Dee_rodict_builder_fini(struct Dee_rodict_builder *__restrict self) {
	size_t i;
	RoDict *dict = self->rdb_dict;
	if unlikely(!dict)
		return;
	for (i = 0; i < dict->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(dict)[i];
		Dee_Decref(item->di_value);
		Dee_Decref(item->di_key);
	}
	_RoDict_Free(dict);
}

PUBLIC NONNULL((1)) void DCALL
Dee_rodict_builder_init_with_hint(struct Dee_rodict_builder *__restrict self,
                                  size_t num_items) {
	RoDict *dict;
	size_t sizeof_dict;
	size_t hmask;
	shift_t hidxio;
	hidxio      = DEE_DICT_HIDXIO_FROMALLOC(num_items);
	hmask       = dict_hmask_from_count(num_items);
	sizeof_dict = _RoDict_SafeSizeOf3(num_items, hmask, hidxio);
	dict        = _RoDict_TryMalloc(sizeof_dict);
	if unlikely(!dict) {
		hmask       = dict_tiny_hmask_from_count(num_items);
		sizeof_dict = _RoDict_SafeSizeOf3(num_items, hmask, hidxio);
		dict        = _RoDict_TryMalloc(sizeof_dict);
		if unlikely(!dict) {
			Dee_rodict_builder_init(self);
			return;
		}
	}
	dict->rd_vsize   = 0;
	dict->rd_hmask   = hmask;
	dict->rd_hidxget = Dee_dict_hidxio[hidxio].dhxio_get;
	dict->rd_htab    = _DeeRoDict_GetRealVTab(dict) + num_items;
	bzero(dict->rd_htab, (hmask + 1) << hidxio);
	self->rdb_dict    = dict;
	self->rdb_valloc  = num_items;
	self->rdb_hidxset = Dee_dict_hidxio[hidxio].dhxio_set;
}

/* Truncate the vtab of "self" to "new_vsize" elements.
 *
 * Dropped elements are NOT finalized (if they were ever
 * initialized to begin with), and if the htab changes
 * word size as a result, it will be truncated accordingly. */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) RoDict *DCALL
rodict_trunc_vtab(RoDict *__restrict self, size_t old_valloc, size_t new_valloc) {
	RoDict *result;
	void *old_htab, *new_htab;
	shift_t old_hidxio, new_hidxio;
	size_t new_sizeof;
	ASSERT(new_valloc < old_valloc);
	old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(old_valloc);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	old_htab   = self->rd_htab;
	new_htab   = _DeeRoDict_GetRealVTab(self) + new_valloc;
	ASSERT(old_htab >= (void *)(_DeeRoDict_GetRealVTab(self) + self->rd_vsize)); /* ">=" because of over-allocation */
	hmask_memmovedown_and_maybe_downcast(new_htab, new_hidxio,
	                                     old_htab, old_hidxio,
	                                     self->rd_hmask + 1);
	self->rd_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
	new_sizeof = _RoDict_SizeOf3(new_valloc, self->rd_hmask, new_hidxio);
	result = _RoDict_TryRealloc(self, new_sizeof);
	if unlikely(!result)
		result = self;
	result->rd_htab = _DeeRoDict_GetRealVTab(result) + new_valloc;
	return result;
}

/* Pack the result of the builder and return it.
 * This function never fails, but "self" becomes invalid as a result. */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeRoDictObject *DCALL
Dee_rodict_builder_pack(struct Dee_rodict_builder *__restrict self) {
	RoDict *result = self->rdb_dict;
	if unlikely(!result) {
		ASSERT(self->rdb_valloc == 0);
		result = (DREF DeeRoDictObject *)Dee_EmptyRoDict;
		Dee_Incref(result);
	} else {
		ASSERT(result->rd_vsize <= self->rdb_valloc);
		/* Free unused memory if there are unused buffers. */
		if unlikely(result->rd_vsize < self->rdb_valloc)
			result = rodict_trunc_vtab(result, self->rdb_valloc, result->rd_vsize);
		DeeObject_Init(result, &DeeRoDict_Type);
	}
	DBG_memset(self, 0xcc, sizeof(struct Dee_rodict_builder));
	rodict_verify(result);
	return result;
}

PRIVATE ATTR_NOINLINE NONNULL((1, 2, 3)) Dee_ssize_t DCALL
rodict_builder_first_setitem(struct Dee_rodict_builder *me,
                             /*inherit(always)*/ DREF DeeObject *key,
                             /*inherit(always)*/ DREF DeeObject *value) {
	RoDict *dict;
	size_t valloc, hmask;
	ASSERT(!me->rdb_dict);
	valloc = DICT_FROMSEQ_DEFAULT_HINT;
	hmask  = dict_hmask_from_count(valloc);
	ASSERT(DEE_DICT_HIDXIO_FROMALLOC(valloc) == 0);
	dict = _RoDict_TryMalloc(_RoDict_SizeOf3(valloc, hmask, 0));
	if unlikely(!dict) {
		valloc = 1;
		hmask  = dict_tiny_hmask_from_count(valloc);
		ASSERT(DEE_DICT_HIDXIO_FROMALLOC(valloc) == 0);
		dict = _RoDict_Malloc(_RoDict_SizeOf3(valloc, hmask, 0));
		if unlikely(!dict)
			goto err;
	}
	me->rdb_valloc   = valloc;
	me->rdb_dict     = dict;
	me->rdb_hidxset  = &Dee_dict_sethidx8;
	dict->rd_vsize   = 1;
	dict->rd_hmask   = hmask;
	dict->rd_hidxget = &Dee_dict_gethidx8;
	dict->rd_htab    = _DeeRoDict_GetRealVTab(dict) + valloc;
	bzero(dict->rd_htab, hmask + 1);
	dict->rd_vtab[0].di_hash  = DeeObject_Hash(key);
	dict->rd_vtab[0].di_key   = key;   /* Inherit reference */
	dict->rd_vtab[0].di_value = value; /* Inherit reference */
	((uint8_t *)dict->rd_htab)[dict->rd_vtab[0].di_hash & hmask] = Dee_dict_vidx_tovirt(0);
	return 0;
err:
	Dee_Decref_unlikely(value);
	Dee_Decref_unlikely(key);
	return -1;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) int DCALL
rodict_builder_grow(struct Dee_rodict_builder *__restrict me) {
	size_t min_valloc, new_sizeof;
	RoDict *old_dict, *new_dict;
	size_t old_hmask, new_hmask;
	size_t old_valloc, new_valloc;
	shift_t old_hidxio, new_hidxio;

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
	old_hidxio = DEE_DICT_HIDXIO_FROMALLOC(old_valloc);
	new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);

	/* Do the allocation. */
	new_sizeof = _RoDict_SafeSizeOf3(new_valloc, new_hmask, new_hidxio);
	new_dict   = _RoDict_TryRealloc(old_dict, new_sizeof);
	if unlikely(!new_dict) {
		/* Try again with a smaller buffer size */
		new_hmask  = dict_tiny_hmask_from_count(min_valloc);
		new_valloc = dict_valloc_from_hmask_and_count(new_hmask, min_valloc, false);
		new_hidxio = DEE_DICT_HIDXIO_FROMALLOC(new_valloc);
		ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
		new_sizeof = _RoDict_SafeSizeOf3(new_valloc, new_hmask, new_hidxio);
		new_dict   = _RoDict_Realloc(old_dict, new_sizeof);
		if unlikely(!new_dict)
			goto err;
	}

	/* Write-back results. */
	me->rdb_dict         = new_dict;
	me->rdb_valloc       = new_valloc;
	me->rdb_hidxset      = Dee_dict_hidxio[new_hidxio].dhxio_set;
	new_dict->rd_hidxget = Dee_dict_hidxio[new_hidxio].dhxio_get;
	new_dict->rd_hmask   = new_hmask;
	new_dict->rd_htab    = _DeeRoDict_GetRealVTab(new_dict) + new_valloc;

	ASSERT(new_hmask >= old_hmask);
	ASSERT(old_hidxio == new_hidxio || (old_hidxio + 1) == new_hidxio);
	if (new_hmask == old_hmask) {
		/* Move/scale the htab so it appears in the correct position. */
		void *old_htab = _DeeRoDict_GetRealVTab(new_dict) + old_valloc;
		void *new_htab = new_dict->rd_htab;
		if likely(old_hidxio == new_hidxio) {
			(*Dee_dict_hidxio[old_hidxio].dhxio_movup)(new_htab, old_htab, new_hmask + 1);
		} else {
			(*Dee_dict_hidxio[old_hidxio].dhxio_upr)(new_htab, old_htab, new_hmask + 1);
		}
	} else {
		/* Rebuild the htab */
		bzero(new_dict->rd_htab, (new_hmask + 1) << new_hidxio);
		rodict_htab_rebuild(new_dict, me->rdb_hidxset);
	}
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem_inherited(/*struct Dee_rodict_builder*/ void *__restrict self,
                                     /*inherit(always)*/ DREF DeeObject *key,
                                     /*inherit(always)*/ DREF DeeObject *value) {
	RoDict *dict;
	size_t result_htab_idx;
	struct Dee_dict_item *item;
	Dee_hash_t hash, hs, perturb;
	struct Dee_rodict_builder *me;
	me   = (struct Dee_rodict_builder *)self;
	dict = me->rdb_dict;
	if unlikely(!dict)
		return rodict_builder_first_setitem(me, key, value);
	ASSERT(dict->rd_vsize <= me->rdb_valloc);

	/* Check if the key already exists (and if so: override it) */
	hash = DeeObject_Hash(key);
	_DeeRoDict_HashIdxInit(dict, &hs, &perturb, hash);
	for (;; _DeeRoDict_HashIdxAdv(dict, &hs, &perturb)) {
		int key_cmp;
		/*virt*/Dee_dict_vidx_t vtab_idx;
		result_htab_idx = hs & dict->rd_hmask;
		vtab_idx = (*dict->rd_hidxget)(dict->rd_htab, result_htab_idx);
		if (vtab_idx == Dee_DICT_HTAB_EOF)
			break; /* EOF */
		item = &_DeeRoDict_GetVirtVTab(dict)[vtab_idx];
		if (item->di_hash != hash)
			continue;
		key_cmp = DeeObject_TryCompareEq(key, item->di_key);
		if unlikely(key_cmp == Dee_COMPARE_ERR)
			goto err;
		if (key_cmp == 0) {
			/* Found the key -> override it */
			ASSERT(Dee_dict_vidx_toreal(vtab_idx) <= dict->rd_vsize - 1);
			Dee_Decref(item->di_key);
			Dee_Decref(item->di_value);
			if (Dee_dict_vidx_toreal(vtab_idx) != dict->rd_vsize - 1) {
				/* Must down-shift the old location. */
				size_t n_after = (dict->rd_vsize - 1) - Dee_dict_vidx_toreal(vtab_idx);
				ASSERT(n_after > 0);
				memmovedownc(item, item + 1, n_after, sizeof(struct Dee_dict_item));
				rodict_htab_decafter(dict, me->rdb_hidxset, vtab_idx /*+ 1*/); /* +1 doesn't matter here */
				item = &_DeeRoDict_GetRealVTab(dict)[dict->rd_vsize - 1];
				item->di_hash = hash;
				(*me->rdb_hidxset)(dict->rd_htab, result_htab_idx, Dee_dict_vidx_tovirt(dict->rd_vsize - 1));
			}
			ASSERT(item->di_hash == hash);
			item->di_key   = key;   /* Inherit reference */
			item->di_value = value; /* Inherit reference */
			return 0;
		}
	}

	/* Insert the key/value pair into the dict. */
	if unlikely(dict->rd_vsize >= me->rdb_valloc) {
		size_t old_hmask = dict->rd_hmask;
		if unlikely(rodict_builder_grow(me))
			goto err;
		dict = me->rdb_dict;
		if (dict->rd_hmask != old_hmask) {
			/* Must re-discover the end of the relevant hash-chain */
			_DeeRoDict_HashIdxInit(dict, &hs, &perturb, hash);
			for (;; _DeeRoDict_HashIdxAdv(dict, &hs, &perturb)) {
				result_htab_idx = hs & dict->rd_hmask;
				if ((*dict->rd_hidxget)(dict->rd_htab, result_htab_idx) == Dee_DICT_HTAB_EOF)
					break; /* EOF */
			}
		}
	}
	ASSERT(dict->rd_vsize < me->rdb_valloc);

	/* Append new item at the end of the vtab (and link it in the htab) */
	item = &_DeeRoDict_GetRealVTab(dict)[dict->rd_vsize];
	item->di_hash  = hash;
	item->di_key   = key;   /* Inherit reference */
	item->di_value = value; /* Inherit reference */
	(*me->rdb_hidxset)(dict->rd_htab, result_htab_idx, Dee_dict_vidx_tovirt(dict->rd_vsize));
	++dict->rd_vsize;
	return 0;
err:
	Dee_Decref_unlikely(value);
	Dee_Decref_unlikely(key);
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL /* binary-compatible with "Dee_foreach_pair_t" */
Dee_rodict_builder_setitem(/*struct Dee_rodict_builder*/ void *__restrict self,
                           DeeObject *key, DeeObject *value) {
	Dee_Incref(key);
	Dee_Incref(value);
	return Dee_rodict_builder_setitem_inherited(self, key, value);
}


PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_from_generic_sequence(DeeObject *__restrict self) {
	size_t hint;
	struct Dee_rodict_builder builder;
	hint = DeeObject_SizeFast(self);
	if (hint != (size_t)-1) {
		Dee_rodict_builder_init_with_hint(&builder, hint);
	} else {
		Dee_rodict_builder_init(&builder);
	}
	if unlikely(Dee_rodict_builder_update(&builder, self))
		goto err;
	return Dee_rodict_builder_pack(&builder);
err:
	Dee_rodict_builder_fini(&builder);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self) {
	if (DeeRoDict_Check(self))
		return_reference_(self);
	if (DeeDict_CheckExact(self))
		return DeeRoDict_FromDict(self);
	return (DREF DeeObject *)rodict_from_generic_sequence(self);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromDict(/*Dict*/ DeeObject *__restrict self) {
	DREF RoDict *result;
	size_t sizeof_result;
	size_t vsize;
	size_t i, hmask;
	shift_t src_hidxio;
	shift_t dst_hidxio;
	DeeDictObject *me = (DeeDictObject *)self;
	ASSERT_OBJECT_TYPE(me, &DeeDict_Type);
again:
	DeeDict_LockReadAndOptimize(me);
	vsize         = me->d_vused;
	hmask         = me->d_hmask;
	src_hidxio    = DEE_DICT_HIDXIO_FROMALLOC(me->d_valloc);
	dst_hidxio    = DEE_DICT_HIDXIO_FROMALLOC(vsize);
	sizeof_result = _RoDict_SizeOf3(vsize, hmask, dst_hidxio);
	result = _RoDict_TryMalloc(sizeof_result);
	if unlikely(!result) {
		DeeDict_LockEndRead(me);
		result = _RoDict_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		DeeDict_LockReadAndOptimize(me);
		if unlikely(vsize != me->d_vused ||
		            hmask != me->d_hmask) {
			DeeDict_LockEndRead(me);
			_RoDict_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rd_htab = mempcpyc(_DeeRoDict_GetRealVTab(result),
	                           _DeeDict_GetRealVTab(me), vsize,
	                           sizeof(struct Dee_dict_item));
	hmask_memcpy_and_maybe_downcast(result->rd_htab, dst_hidxio,
	                                me->d_htab, src_hidxio,
	                                hmask + 1);
	for (i = 0; i < vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(result)[i];
		ASSERT(item->di_key);
		Dee_Incref(item->di_key);
		Dee_Incref(item->di_value);
	}
	DeeDict_LockEndRead(me);
	result->rd_vsize   = vsize;
	result->rd_hmask   = hmask;
	result->rd_hidxget = Dee_dict_hidxio[dst_hidxio].dhxio_get;
	DeeObject_Init(result, &DeeRoDict_Type);
	rodict_verify(result);
	return (DREF DeeObject *)result;
err:
	return NULL;
}









/************************************************************************/
/* RODICT ITERATOR                                                      */
/************************************************************************/

typedef struct {
	PROXY_OBJECT_HEAD_EX(RoDict, rodi_dict) /* [1..1][const] The RoDict being iterated. */
	/*real*/Dee_dict_vidx_t      rodi_vidx; /* [lock(ATOMIC)] Index of next item to yield. */
} RoDictIterator;

INTDEF DeeTypeObject RoDictIterator_Type;

STATIC_ASSERT(offsetof(RoDictIterator, rodi_dict) == offsetof(ProxyObject, po_obj));
#define rditer_fini  generic_proxy__fini
#define rditer_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
rditer_ctor(RoDictIterator *__restrict self) {
	self->rodi_dict = (DREF RoDict *)Dee_EmptyRoDict;
	Dee_Incref(Dee_EmptyRoDict);
	self->rodi_vidx = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rditer_copy(RoDictIterator *__restrict self,
            RoDictIterator *__restrict other) {
	self->rodi_dict = other->rodi_dict;
	Dee_Incref(self->rodi_dict);
	self->rodi_vidx = atomic_read(&other->rodi_vidx);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rditer_deep(RoDictIterator *__restrict self,
            RoDictIterator *__restrict other) {
	self->rodi_dict = (DREF RoDict *)DeeObject_DeepCopy((DeeObject *)other->rodi_dict);
	if unlikely(!self->rodi_dict)
		goto err;
	self->rodi_vidx = atomic_read(&other->rodi_vidx);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rditer_init(RoDictIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	self->rodi_vidx = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_RoDictIterator",
	                  &self->rodi_dict, &self->rodi_vidx))
		goto err;
	if (DeeObject_AssertTypeExact(self->rodi_dict, &DeeRoDict_Type))
		goto err;
	Dee_Incref(self->rodi_dict);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rditer_bool(RoDictIterator *__restrict self) {
	return atomic_read(&self->rodi_vidx) < self->rodi_dict->rd_vsize ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rditer_compare(RoDictIterator *__restrict lhs, RoDictIterator *__restrict rhs) {
	if (DeeObject_AssertTypeExact(rhs, &RoDictIterator_Type))
		goto err;
	Dee_return_compare_if_neT(RoDict *, lhs->rodi_dict, rhs->rodi_dict);
	Dee_return_compareT(/*real*/Dee_dict_vidx_t,
	                    atomic_read(&lhs->rodi_vidx),
	                    atomic_read(&rhs->rodi_vidx));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rditer_nextpair(RoDictIterator *__restrict self, DREF DeeObject *key_and_value[2]) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_dict_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return 1;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	key_and_value[0] = item->di_key;
	key_and_value[1] = item->di_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rditer_nextkey(RoDictIterator *__restrict self) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_dict_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return ITER_DONE;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	return_reference_(item->di_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rditer_nextvalue(RoDictIterator *__restrict self) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_dict_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return ITER_DONE;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	return_reference_(item->di_value);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rditer_advance(RoDictIterator *__restrict self, size_t step) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_dict_vidx_t old_vidx;
	/*real*/Dee_dict_vidx_t new_vidx;
	do {
		old_vidx = atomic_read(&self->rodi_vidx);
		if (OVERFLOW_UADD(old_vidx, step, &new_vidx))
			new_vidx = (Dee_dict_vidx_t)-1;
		if (new_vidx > dict->rd_vsize)
			new_vidx = dict->rd_vsize;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, old_vidx, new_vidx));
	return (size_t)(new_vidx - old_vidx);
}

PRIVATE struct type_cmp rditer_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rditer_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&rditer_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_iterator rditer_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&rditer_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rditer_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rditer_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&rditer_advance,
};

PRIVATE struct type_member tpconst rditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RoDictIterator, rodi_dict), "->?DDict"),
	TYPE_MEMBER_FIELD_DOC("__index__", STRUCT_SIZE_T | STRUCT_ATOMIC, offsetof(RoDictIterator, rodi_vidx), "->?Dint"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(dict:?Ert:RoDict,index=!0)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rditer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rditer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rditer_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&rditer_init,
				TYPE_FIXED_ALLOCATOR(RoDictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rditer_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &rditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &rditer_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};








/************************************************************************/
/* RODICT                                                               */
/************************************************************************/

PUBLIC struct Dee_empty_rodict_object DeeRoDict_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeRoDict_Type),
	/* .rd_vsize     = */ 0,
	/* .rd_hmask     = */ 0,
	/* .rd_hidxget   = */ &Dee_dict_gethidx8,
	/* .rd_htab      = */ DeeRoDict_EmptyInstance.rd_htab_data,
	/* .rd_vtab      = */ /*{},*/
	/* .rd_htab_data = */ { 0 },
};


PRIVATE WUNUSED DREF RoDict *DCALL rodict_ctor(void) {
	return_reference_((DREF RoDict *)Dee_EmptyRoDict);
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_deepcopy(RoDict *__restrict self) {
	size_t sizeof_result, i;
	DREF RoDict *result;
	shift_t hidxio;
	hidxio        = DEE_DICT_HIDXIO_FROMALLOC(self->rd_vsize);
	sizeof_result = _RoDict_SizeOf3(self->rd_vsize, self->rd_hmask, hidxio);
	result        = _RoDict_Malloc(sizeof_result);
	if unlikely(!result)
		goto err;
	sizeof_result -= offsetof(DeeRoDictObject, rd_vtab);
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *dst = &_DeeRoDict_GetRealVTab(result)[i];
		struct Dee_dict_item *src = &_DeeRoDict_GetRealVTab(self)[i];
		dst->di_key = DeeObject_DeepCopy(src->di_key);
		if unlikely(!dst->di_key)
			goto err_r_items_before_i;
		dst->di_value = DeeObject_DeepCopy(src->di_value);
		if unlikely(!dst->di_value)
			goto err_r_items_before_i_and_key_at_i;
		dst->di_hash = DeeObject_Hash(dst->di_key);
	}
	result->rd_vsize   = self->rd_vsize;
	result->rd_hmask   = self->rd_hmask;
	result->rd_hidxget = self->rd_hidxget;
	result->rd_htab    = _DeeRoDict_GetRealVTab(result) + result->rd_vsize;
	bzero(result->rd_htab, (result->rd_hmask + 1) << hidxio);
	/* Must rebuild the hash-table since deep-copied keys may have different hashs. */
	rodict_htab_rebuild(result, Dee_dict_hidxio[hidxio].dhxio_set);
	DeeObject_Init(result, &DeeRoDict_Type);
	rodict_verify(result);
	return result;
err_r_items_before_i_and_key_at_i:
	Dee_Decref(_DeeRoDict_GetRealVTab(result)[i].di_key);
err_r_items_before_i:
	while (i) {
		struct Dee_dict_item *item;
		--i;
		item = &_DeeRoDict_GetRealVTab(result)[i];
		Dee_Decref(item->di_value);
		Dee_Decref(item->di_key);
	}
/*err_r:*/
	_RoDict_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF RoDict *DCALL
rodict_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_RoDict", &seq))
		goto err;
	return (DREF RoDict *)DeeRoDict_FromSequence(seq);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rodict_fini(RoDict *__restrict self) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		Dee_Decref(item->di_key);
		Dee_Decref(item->di_value);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
rodict_visit(RoDict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		Dee_Visit(item->di_key);
		Dee_Visit(item->di_value);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_bool(RoDict *__restrict self) {
	return self->rd_vsize != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_printrepr(RoDict *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
#define RODICT_REPR_EMPTY "Dict.Frozen()"
#define RODICT_REPR_HEAD  "{ "
#define RODICT_REPR_TAIL  " }.frozen"
	/*real*/Dee_dict_vidx_t i;
	Dee_ssize_t temp, result;
	if unlikely(!self->rd_vsize)
		return DeeFormat_PRINT(printer, arg, RODICT_REPR_EMPTY);
	result = DeeFormat_PRINT(printer, arg, RODICT_REPR_HEAD);
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		if (i) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		item = &_DeeRoDict_GetRealVTab(self)[i];
		temp = DeeFormat_Printf(printer, arg, "%r: %r", item->di_key, item->di_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	temp = DeeFormat_PRINT(printer, arg, RODICT_REPR_TAIL);
	if unlikely(temp < 0)
		goto err_temp;
	result += temp;
done:
	return result;
err_temp:
	return temp;
#undef RODICT_REPR_EMPTY
#undef RODICT_REPR_HEAD
#undef RODICT_REPR_TAIL
}



PRIVATE WUNUSED NONNULL((1)) DREF RoDictIterator *DCALL
rodict_iter(RoDict *__restrict self) {
	DREF RoDictIterator *result;
	result = DeeObject_MALLOC(RoDictIterator);
	if unlikely(!result)
		goto err;
	result->rodi_dict = self;
	Dee_Incref(self);
	result->rodi_vidx = 0;
	DeeObject_Init(result, &RoDictIterator_Type);
	return result;
err:
	return NULL;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_sizeob(RoDict *__restrict self) {
	return DeeInt_NewSize(self->rd_vsize);
}
#endif /* !__OPTIMIZE_SIZE__ */

#define rodict_size_fast rodict_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rodict_size(RoDict *__restrict self) {
	return self->rd_vsize;
}

/* Item lookup operators. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_contains(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_getitemnr(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem(RoDict *self, DeeObject *key);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL rodict_getitem_index(RoDict *self, size_t key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL rodict_trygetitem_index(RoDict *self, size_t key);
PRIVATE WUNUSED NONNULL((1)) int DCALL rodict_bounditem_index(RoDict *self, size_t key);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_getitemnr_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_getitemnr_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);

#ifdef Dee_BOUND_MAYALIAS_HAS
#define rodict_hasitem                 rodict_bounditem
#define rodict_hasitem_index           rodict_bounditem_index
#define rodict_hasitem_string_hash     rodict_bounditem_string_hash
#define rodict_hasitem_string_len_hash rodict_bounditem_string_len_hash
#else /* Dee_BOUND_MAYALIAS_HAS */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_hasitem(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) int DCALL rodict_hasitem_index(RoDict *self, size_t key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_hasitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_hasitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
#endif /* !Dee_BOUND_MAYALIAS_HAS */

#ifdef __OPTIMIZE_SIZE__
#define SUBSTITUDE_rodict_getitemnr
#define SUBSTITUDE_rodict_getitemnr_string_hash
#define SUBSTITUDE_rodict_getitemnr_string_len_hash
#define SUBSTITUDE_rodict_contains
#define SUBSTITUDE_rodict_getitem
#define SUBSTITUDE_rodict_trygetitem
#define SUBSTITUDE_rodict_bounditem
#define SUBSTITUDE_rodict_hasitem
#define SUBSTITUDE_rodict_getitem_index
#define SUBSTITUDE_rodict_bounditem_index
#define SUBSTITUDE_rodict_hasitem_index
#define SUBSTITUDE_rodict_getitem_string_hash
#define SUBSTITUDE_rodict_trygetitem_string_hash
#define SUBSTITUDE_rodict_bounditem_string_hash
#define SUBSTITUDE_rodict_hasitem_string_hash
#define SUBSTITUDE_rodict_getitem_string_len_hash
#define SUBSTITUDE_rodict_trygetitem_string_len_hash
#define SUBSTITUDE_rodict_bounditem_string_len_hash
#define SUBSTITUDE_rodict_hasitem_string_len_hash
#else /* __OPTIMIZE_SIZE__ */
/* Event without -Os, substitute these operators, just because
 * the dedicated variant wouldn't really be noticeably faster. */
#define SUBSTITUDE_rodict_contains
#define SUBSTITUDE_rodict_bounditem
#define SUBSTITUDE_rodict_hasitem
/*#define SUBSTITUDE_rodict_bounditem_index*/ /* Would need unnecessary incref() */
/*#define SUBSTITUDE_rodict_hasitem_index*/   /* Would need unnecessary incref() */
#define SUBSTITUDE_rodict_bounditem_string_hash
#define SUBSTITUDE_rodict_hasitem_string_hash
#define SUBSTITUDE_rodict_bounditem_string_len_hash
#define SUBSTITUDE_rodict_hasitem_string_len_hash
#endif /* !__OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_rodict_trygetitemnr
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitemnr_string_hash
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitemnr_string_len_hash
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitem_index
#include "rodict-getitem.c.inl"

#ifndef SUBSTITUDE_rodict_getitemnr
#define DEFINE_rodict_getitemnr
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitemnr */

#ifndef SUBSTITUDE_rodict_getitemnr_string_hash
#define DEFINE_rodict_getitemnr_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitemnr_string_hash */

#ifndef SUBSTITUDE_rodict_getitemnr_string_len_hash
#define DEFINE_rodict_getitemnr_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitemnr_string_len_hash */

#ifndef SUBSTITUDE_rodict_contains
#define DEFINE_rodict_contains
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_contains */

#ifndef SUBSTITUDE_rodict_getitem
#define DEFINE_rodict_getitem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem */

#ifndef SUBSTITUDE_rodict_trygetitem
#define DEFINE_rodict_trygetitem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem */

#ifndef SUBSTITUDE_rodict_bounditem
#define DEFINE_rodict_bounditem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem */

#ifndef SUBSTITUDE_rodict_hasitem
#define DEFINE_rodict_hasitem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_hasitem */

#ifndef SUBSTITUDE_rodict_getitem_index
#define DEFINE_rodict_getitem_index
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_index */

#ifndef SUBSTITUDE_rodict_bounditem_index
#define DEFINE_rodict_bounditem_index
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_index */

#ifndef SUBSTITUDE_rodict_hasitem_index
#define DEFINE_rodict_hasitem_index
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_hasitem_index */

#ifndef SUBSTITUDE_rodict_getitem_string_hash
#define DEFINE_rodict_getitem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_string_hash */

#ifndef SUBSTITUDE_rodict_trygetitem_string_hash
#define DEFINE_rodict_trygetitem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem_string_hash */

#ifndef SUBSTITUDE_rodict_bounditem_string_hash
#define DEFINE_rodict_bounditem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_string_hash */

#ifndef SUBSTITUDE_rodict_hasitem_string_hash
#define DEFINE_rodict_hasitem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_hasitem_string_hash */

#ifndef SUBSTITUDE_rodict_getitem_string_len_hash
#define DEFINE_rodict_getitem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_string_len_hash */

#ifndef SUBSTITUDE_rodict_trygetitem_string_len_hash
#define DEFINE_rodict_trygetitem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem_string_len_hash */

#ifndef SUBSTITUDE_rodict_bounditem_string_len_hash
#define DEFINE_rodict_bounditem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_string_len_hash */

#ifndef SUBSTITUDE_rodict_hasitem_string_len_hash
#define DEFINE_rodict_hasitem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_hasitem_string_len_hash */

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef SUBSTITUDE_rodict_getitemnr
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr(RoDict *self, DeeObject *key) {
	DeeObject *result = rodict_trygetitemnr(self, key);
	if unlikely(result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitemnr */

#ifdef SUBSTITUDE_rodict_getitemnr_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_hash(self, key, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitemnr_string_hash */

#ifdef SUBSTITUDE_rodict_getitemnr_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_str_len((DeeObject *)self, key, keylen);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitemnr_string_len_hash */

#ifdef SUBSTITUDE_rodict_contains
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_contains(RoDict *self, DeeObject *key) {
	DeeObject *value = rodict_trygetitemnr(self, key);
	if unlikely(!value)
		goto err;
	return_bool_(value != ITER_DONE);
err:
	return NULL;
}
#endif /* SUBSTITUDE_rodict_contains */

#ifdef SUBSTITUDE_rodict_getitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem(RoDict *self, DeeObject *key) {
	DeeObject *result = rodict_getitemnr(self, key);
	Dee_XIncref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem */

#ifdef SUBSTITUDE_rodict_trygetitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem(RoDict *self, DeeObject *key) {
	DeeObject *result = rodict_trygetitemnr(self, key);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem */

#ifdef SUBSTITUDE_rodict_bounditem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem(RoDict *self, DeeObject *key) {
	DeeObject *value = rodict_trygetitemnr(self, key);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem */

#ifdef SUBSTITUDE_rodict_hasitem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem(RoDict *self, DeeObject *key) {
	int bound = rodict_bounditem(self, key);
	return Dee_BOUND_ASHAS(bound);
}
#endif /* SUBSTITUDE_rodict_hasitem */

#ifdef SUBSTITUDE_rodict_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getitem_index(RoDict *self, size_t key) {
	DREF DeeObject *result = rodict_trygetitem_index(self, key);
	if unlikely(result == ITER_DONE) {
		err_unknown_key_int((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_index */

#ifdef SUBSTITUDE_rodict_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_bounditem_index(RoDict *self, size_t key) {
	DREF DeeObject *value = rodict_trygetitem_index(self, key);
	if unlikely(!value)
		return Dee_BOUND_ERR;
	if (value == ITER_DONE)
		return Dee_BOUND_MISSING;
	Dee_DecrefNokill(value);
	return Dee_BOUND_YES;
}
#endif /* SUBSTITUDE_rodict_bounditem_index */

#ifdef SUBSTITUDE_rodict_hasitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_hasitem_index(RoDict *self, size_t key) {
	int bound = rodict_bounditem_index(self, key);
	return Dee_BOUND_ASHAS(bound);
}
#endif /* SUBSTITUDE_rodict_hasitem_index */

#ifdef SUBSTITUDE_rodict_getitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result = rodict_getitemnr_string_hash(self, key, hash);
	Dee_XIncref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_string_hash */

#ifdef SUBSTITUDE_rodict_trygetitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_hash(self, key, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem_string_hash */

#ifdef SUBSTITUDE_rodict_bounditem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *value = rodict_trygetitemnr_string_hash(self, key, hash);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem_string_hash */

#ifdef SUBSTITUDE_rodict_hasitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	int bound = rodict_bounditem_string_hash(self, key, hash);
	return Dee_BOUND_ASHAS(bound);
}
#endif /* SUBSTITUDE_rodict_hasitem_string_hash */

#ifdef SUBSTITUDE_rodict_getitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result = rodict_getitemnr_string_len_hash(self, key, keylen, hash);
	Dee_XIncref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_string_len_hash */

#ifdef SUBSTITUDE_rodict_trygetitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem_string_len_hash */

#ifdef SUBSTITUDE_rodict_bounditem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *value = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem_string_len_hash */

#ifdef SUBSTITUDE_rodict_hasitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int bound = rodict_bounditem_string_len_hash(self, key, keylen, hash);
	return Dee_BOUND_ASHAS(bound);
}
#endif /* SUBSTITUDE_rodict_hasitem_string_len_hash */


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_foreach_pair(RoDict *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	/*real*/Dee_dict_vidx_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		temp = (*cb)(arg, item->di_key, item->di_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_nonempty_as_bound(RoDict *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->rd_vsize != 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_getfirst(RoDict *__restrict self) {
	DREF DeeTupleObject *result;
	struct Dee_dict_item *item;
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	item = _DeeRoDict_GetRealVTab(self);
	result->t_elem[0] = item->di_key;
	Dee_Incref(result->t_elem[0]);
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[1]);
	return result;
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_first);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_getlast(RoDict *__restrict self) {
	DREF DeeTupleObject *result;
	struct Dee_dict_item *item;
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	item = _DeeRoDict_GetRealVTab(self);
	item += self->rd_vsize - 1;
	result->t_elem[0] = item->di_key;
	Dee_Incref(result->t_elem[0]);
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[1]);
	return result;
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_last);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_trygetfirst(RoDict *__restrict self) {
	if (!self->rd_vsize)
		return (DREF DeeTupleObject *)ITER_DONE;
	return rodict_getfirst(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_trygetlast(RoDict *__restrict self) {
	if (!self->rd_vsize)
		return (DREF DeeTupleObject *)ITER_DONE;
	return rodict_getlast(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getfirstkey(RoDict *__restrict self) {
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[0].di_key);
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_first);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getlastkey(RoDict *__restrict self) {
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[self->rd_vsize - 1].di_key);
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_first);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getfirstvalue(RoDict *__restrict self) {
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[0].di_value);
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_first);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getlastvalue(RoDict *__restrict self) {
	if unlikely(!self->rd_vsize)
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[self->rd_vsize - 1].di_value);
err_unbound:
	err_unbound_attribute_string(&DeeRoDict_Type, STR_first);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_sizeof(RoDict *__restrict self) {
	size_t result = _RoDict_SizeOf(self->rd_vsize, self->rd_hmask);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict___hidxio__(RoDict *__restrict self) {
	shift_t result = DEE_DICT_HIDXIO_FROMALLOC(self->rd_vsize);
	return DeeInt_NEWU(result);
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_foreach(RoDict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeTupleObject *key_and_value;
	Dee_dict_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeTuple_NewUninitializedPair();
	if unlikely(!key_and_value)
		goto err;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		key_and_value->t_elem[0] = item->di_key;   /* Inherit reference (shared) */
		key_and_value->t_elem[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(key_and_value->t_elem[0] == item->di_key);
		ASSERT(key_and_value->t_elem[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Incref(key_and_value->t_elem[1]);
			Dee_Incref(key_and_value->t_elem[0]);
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeTuple_NewUninitializedPair();
			if unlikely(!key_and_value)
				goto err;
		} else {
			DBG_memset(key_and_value->t_elem, 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeTuple_FreeUninitializedPair(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Incref(key_and_value->t_elem[1]);
	Dee_Incref(key_and_value->t_elem[0]);
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_foreach_reverse(RoDict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeTupleObject *key_and_value;
	Dee_dict_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeTuple_NewUninitializedPair();
	if unlikely(!key_and_value)
		goto err;
	i = self->rd_vsize;
	while (i) {
		struct Dee_dict_item *item;
		--i;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		key_and_value->t_elem[0] = item->di_key;   /* Inherit reference (shared) */
		key_and_value->t_elem[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(key_and_value->t_elem[0] == item->di_key);
		ASSERT(key_and_value->t_elem[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Incref(key_and_value->t_elem[1]);
			Dee_Incref(key_and_value->t_elem[0]);
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeTuple_NewUninitializedPair();
			if unlikely(!key_and_value)
				goto err;
		} else {
			DBG_memset(key_and_value->t_elem, 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeTuple_FreeUninitializedPair(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Incref(key_and_value->t_elem[1]);
	Dee_Incref(key_and_value->t_elem[0]);
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_enumerate_index(RoDict *__restrict self, Dee_seq_enumerate_index_t cb,
                              void *arg, size_t start, size_t end) {
	DREF DeeTupleObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeTuple_NewUninitializedPair();
	if unlikely(!key_and_value)
		goto err;
	if (end > self->rd_vsize)
		end = self->rd_vsize;
	while (start < end) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[start];
		key_and_value->t_elem[0] = item->di_key;   /* Inherit reference (shared) */
		key_and_value->t_elem[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, start, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(key_and_value->t_elem[0] == item->di_key);
		ASSERT(key_and_value->t_elem[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Incref(key_and_value->t_elem[1]);
			Dee_Incref(key_and_value->t_elem[0]);
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeTuple_NewUninitializedPair();
			if unlikely(!key_and_value)
				goto err;
		} else {
			DBG_memset(key_and_value->t_elem, 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		++start;
	}
	DeeTuple_FreeUninitializedPair(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Incref(key_and_value->t_elem[1]);
	Dee_Incref(key_and_value->t_elem[0]);
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_enumerate_index_reverse(RoDict *__restrict self, Dee_seq_enumerate_index_t cb,
                                      void *arg, size_t start, size_t end) {
	DREF DeeTupleObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeTuple_NewUninitializedPair();
	if unlikely(!key_and_value)
		goto err;
	if (end > self->rd_vsize)
		end = self->rd_vsize;
	while (start < end) {
		struct Dee_dict_item *item;
		--end;
		item = &_DeeRoDict_GetRealVTab(self)[end];
		key_and_value->t_elem[0] = item->di_key;   /* Inherit reference (shared) */
		key_and_value->t_elem[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, end, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(key_and_value->t_elem[0] == item->di_key);
		ASSERT(key_and_value->t_elem[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Incref(key_and_value->t_elem[1]);
			Dee_Incref(key_and_value->t_elem[0]);
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeTuple_NewUninitializedPair();
			if unlikely(!key_and_value)
				goto err;
		} else {
			DBG_memset(key_and_value->t_elem, 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeTuple_FreeUninitializedPair(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Incref(key_and_value->t_elem[1]);
	Dee_Incref(key_and_value->t_elem[0]);
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_mh_seq_getitem_index(RoDict *__restrict self, size_t index) {
	DREF DeeTupleObject *result;
	struct Dee_dict_item *item;
	if unlikely(index >= self->rd_vsize)
		goto err_oob;
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	item = &_DeeRoDict_GetRealVTab(self)[index];
	result->t_elem[0] = item->di_key;
	Dee_Incref(result->t_elem[0]);
	result->t_elem[1] = item->di_value;
	Dee_Incref(result->t_elem[1]);
	return result;
err_oob:
	err_index_out_of_bounds((DeeObject *)self, index, self->rd_vsize);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rodict_mh_seq_trygetitem_index(RoDict *__restrict self, size_t index) {
	if (index >= self->rd_vsize)
		return (DREF DeeTupleObject *)ITER_DONE;
	return rodict_mh_seq_getitem_index(self, index);
}


struct rodict_compare_seq_foreach_data {
	RoDict                 *rdcsfd_lhs;   /* [1..1] lhs-dict. */
	/*real*/Dee_dict_vidx_t rdcsfd_index; /* Next index into "rdcsfd_lhs" to compare against. */
};
#define RODICT_COMPARE_SEQ_FOREACH_ERROR    (-1)
#define RODICT_COMPARE_SEQ_FOREACH_EQUAL    (0)
#define RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL (-2)
#define RODICT_COMPARE_SEQ_FOREACH_LESS     (-2)
#define RODICT_COMPARE_SEQ_FOREACH_GREATER  (-3)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_compare_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoDict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	struct rodict_compare_seq_foreach_data *data;
	data = (struct rodict_compare_seq_foreach_data *)arg;
	dict = data->rdcsfd_lhs;
	if unlikely(data->rdcsfd_index >= dict->rd_vsize)
		return RODICT_COMPARE_SEQ_FOREACH_LESS;
	lhs_item = &_DeeRoDict_GetRealVTab(dict)[data->rdcsfd_index];
	cmp_result = seq_docompare__lhs_vector(lhs_item->di_key_and_value, 2, rhs_item);
	if unlikely(cmp_result == Dee_COMPARE_ERR)
		goto err;
	++data->rdcsfd_index;
	if (cmp_result < 0)
		return RODICT_COMPARE_SEQ_FOREACH_LESS;
	if (cmp_result > 0)
		return RODICT_COMPARE_SEQ_FOREACH_GREATER;
	return RODICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return RODICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_compare_eq_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoDict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	struct rodict_compare_seq_foreach_data *data;
	if (!DeeType_RequireSupportedNativeOperator(Dee_TYPE(rhs_item), foreach))
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	data = (struct rodict_compare_seq_foreach_data *)arg;
	dict = data->rdcsfd_lhs;
	if unlikely(data->rdcsfd_index >= dict->rd_vsize)
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	lhs_item = &_DeeRoDict_GetRealVTab(dict)[data->rdcsfd_index];
	cmp_result = seq_docompareeq__lhs_vector(lhs_item->di_key_and_value, 2, rhs_item);
	if unlikely(cmp_result == Dee_COMPARE_ERR)
		goto err;
	++data->rdcsfd_index;
	if (cmp_result != 0)
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	return RODICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return RODICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_compare(RoDict *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct rodict_compare_seq_foreach_data data;
	data.rdcsfd_index = 0;
	data.rdcsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &rodict_compare_seq_foreach, &data);
	if unlikely(foreach_status == RODICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_LESS)
		return -1;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_GREATER)
		return 1;
	if (data.rdcsfd_index < lhs->rd_vsize)
		return 1;
	return 0;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_compare_eq(RoDict *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct rodict_compare_seq_foreach_data data;
	data.rdcsfd_index = 0;
	data.rdcsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &rodict_compare_eq_seq_foreach, &data);
	if unlikely(foreach_status == RODICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL)
		return 1;
	if (data.rdcsfd_index < lhs->rd_vsize)
		return 1;
	return 0;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_trycompare_eq(RoDict *lhs, DeeObject *rhs) {
	if (!DeeType_RequireSupportedNativeOperator(Dee_TYPE(rhs), foreach))
		return 1;
	return rodict_mh_seq_compare_eq(lhs, rhs);
}


struct rodict_fromkeys_data {
	struct Dee_rodict_builder rdfkd_builder; /* Dict builder. */
	DeeObject                *rdfkd_value; /* [1..1] Value to insert, or callback to generate value. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_fromkeys_with_value(void *arg, DeeObject *key) {
	struct rodict_fromkeys_data *data = (struct rodict_fromkeys_data *)arg;
	return Dee_rodict_builder_setitem(&data->rdfkd_builder, key, data->rdfkd_value);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_fromkeys_with_valuefor(void *arg, DeeObject *key) {
	struct rodict_fromkeys_data *data = (struct rodict_fromkeys_data *)arg;
	DREF DeeObject *used_value;
	data = (struct rodict_fromkeys_data *)arg;
	used_value = DeeObject_Call(data->rdfkd_value, 1, &key);
	if unlikely(!used_value)
		goto err;
	Dee_Incref(key);
	return Dee_rodict_builder_setitem_inherited(&data->rdfkd_builder, key, used_value);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_fromkeys_slow(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	Dee_ssize_t foreach_status;
	struct rodict_fromkeys_data data;
	size_t hint;
	hint = DeeObject_SizeFast(keys);
	if unlikely(hint == (size_t)-1)
		hint = DICT_FROMSEQ_DEFAULT_HINT;
	Dee_rodict_builder_init_with_hint(&data.rdfkd_builder, hint);
	if unlikely(valuefor) {
		data.rdfkd_value = valuefor;
		foreach_status = DeeObject_Foreach(keys, &rodict_fromkeys_with_valuefor, &data);
	} else {
		data.rdfkd_value = value;
		foreach_status = DeeObject_Foreach(keys, &rodict_fromkeys_with_value, &data);
	}
	if unlikely(foreach_status < 0)
		goto err_r;
	return Dee_rodict_builder_pack(&data.rdfkd_builder);
err_r:
	Dee_rodict_builder_fini(&data.rdfkd_builder);
/*err:*/
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rodict_destroy_keysonly(RoDict *__restrict self) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		Dee_Decref(item->di_key);
		/*Dee_Decref(item->di_value);*/
	}
	_RoDict_Free(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_init_values_after_keys(RoDict *__restrict self, DeeObject *value, DeeObject *valuefor) {
	size_t i;
	struct Dee_dict_item *vtab = _DeeRoDict_GetRealVTab(self);
	if (valuefor) {
		for (i = 0; i < self->rd_vsize; ++i) {
			DREF DeeObject *computed_value;
			struct Dee_dict_item *item = &vtab[i];
			computed_value = DeeObject_Call(valuefor, 1, &item->di_key);
			if unlikely(!computed_value)
				goto err_values_before_i;
			item->di_value = computed_value; /* Inherit reference */
		}
	} else {
		for (i = 0; i < self->rd_vsize; ++i)
			vtab[i].di_value = value; /* Inherit reference */
		Dee_Incref_n(value, self->rd_vsize);
	}
	return 0;
err_values_before_i:
	while (i) {
		--i;
		Dee_Decref(vtab[i].di_value);
	}
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_dict_keys(DeeDictObject *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
	size_t sizeof_result;
	size_t vsize;
	size_t i, hmask;
	shift_t src_hidxio;
	shift_t dst_hidxio;
again:
	DeeDict_LockReadAndOptimize(dict_keys);
	vsize         = dict_keys->d_vused;
	hmask         = dict_keys->d_hmask;
	src_hidxio    = DEE_DICT_HIDXIO_FROMALLOC(dict_keys->d_valloc);
	dst_hidxio    = DEE_DICT_HIDXIO_FROMALLOC(vsize);
	sizeof_result = _RoDict_SizeOf3(vsize, hmask, dst_hidxio);
	result = _RoDict_TryMalloc(sizeof_result);
	if unlikely(!result) {
		DeeDict_LockEndRead(dict_keys);
		result = _RoDict_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		DeeDict_LockReadAndOptimize(dict_keys);
		if unlikely(vsize != dict_keys->d_vused ||
		            hmask != dict_keys->d_hmask) {
			DeeDict_LockEndRead(dict_keys);
			_RoDict_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rd_htab = _DeeRoDict_GetRealVTab(result) + vsize;
	for (i = 0; i < vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeDict_GetRealVTab(dict_keys)[i];
		dst->di_hash = src->di_hash;
		dst->di_key  = src->di_key;
		Dee_Incref(dst->di_key);
	}
	hmask_memcpy_and_maybe_downcast(result->rd_htab, dst_hidxio,
	                                dict_keys->d_htab, src_hidxio,
	                                hmask + 1);
	DeeDict_LockEndRead(dict_keys);
	result->rd_vsize   = vsize;
	result->rd_hmask   = hmask;
	result->rd_hidxget = Dee_dict_hidxio[dst_hidxio].dhxio_get;

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_Init(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_rodict_keys(DeeRoDictObject *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
	size_t i, sizeof_result;
	shift_t hidxio;
	hidxio        = DEE_DICT_HIDXIO_FROMALLOC(dict_keys->rd_vsize);
	sizeof_result = _RoDict_SizeOf3(dict_keys->rd_vsize, dict_keys->rd_hmask, hidxio);
	result        = _RoDict_Malloc(sizeof_result);
	if unlikely(!result)
		goto err;
	result->rd_vsize   = dict_keys->rd_vsize;
	result->rd_hmask   = dict_keys->rd_hmask;
	result->rd_hidxget = dict_keys->rd_hidxget;

	/* Copy htab */
	result->rd_htab = memcpy(_DeeRoDict_GetRealVTab(result) + result->rd_vsize,
	                         dict_keys->rd_htab, (result->rd_hmask + 1) << hidxio);

	/* Copy items (keys-only) */
	for (i = 0; i < result->rd_vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeRoDict_GetRealVTab(dict_keys)[i];
		dst->di_hash = src->di_hash;
		dst->di_key  = src->di_key;
		Dee_Incref(dst->di_key);
	}

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_Init(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_fromkeys(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	DeeTypeObject *tp_keys = Dee_TYPE(keys);

	/* Optimizations for special, known keys types. */
	if (tp_keys == &DefaultSequence_MapKeys_Type) {
		/* Special optimization when "keys" are the keys of another Dict/RoDict */
		DefaultSequence_MapProxy *proxy = (DefaultSequence_MapProxy *)keys;
		DeeObject *mapping_of_keys = proxy->dsmp_map;
		DeeTypeObject *tp_mapping_of_keys = Dee_TYPE(mapping_of_keys);
		if (tp_mapping_of_keys == &DeeDict_Type)
			return rodict_from_dict_keys((DeeDictObject *)mapping_of_keys, value, valuefor);
		if (tp_mapping_of_keys == &DeeRoDict_Type)
			return rodict_from_rodict_keys((DeeRoDictObject *)mapping_of_keys, value, valuefor);
	} else if (tp_keys == &DeeHashSet_Type) {
		/* Special optimization when "keys" is a HashSet: Duplicate its control structures */
		/* TODO: do this once "HashSet" uses the same data structure as Dict. */
	} else if (tp_keys == &DeeRoSet_Type) {
		/* Special optimization when "keys" is a RoSet: Duplicate its control structures */
		/* TODO: do this once "RoSet" uses the same data structure as Dict. */
	}

	return rodict_fromkeys_slow(keys, value, valuefor);
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_fromkeys_f(DeeTypeObject *__restrict UNUSED(dict),
                  size_t argc, DeeObject *const *argv,
                  DeeObject *kw) {
	DeeObject *keys, *value = Dee_None;
	DeeObject *valuefor = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__keys_value_valuefor,
	                    "o|oo:fromkeys", &keys, &value, &valuefor))
		goto err;
	return rodict_fromkeys(keys, value, valuefor);
err:
	return NULL;
}

PRIVATE struct type_seq rodict_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_iter,
	/* .tp_sizeob                       = */ NULL_IF_Os((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_sizeob),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ NULL,
	/* .tp_delrange                     = */ NULL,
	/* .tp_setrange                     = */ NULL,
	/* .tp_foreach                      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&rodict_mh_seq_foreach,
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_enumerate                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_enumerate_index              = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))NULL,
	/* .tp_iterkeys                     = */ NULL,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_bounditem,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size_fast,
	/* .tp_getitem_index                = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rodict_getitem_index,
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rodict_bounditem_index,
	/* .tp_hasitem_index                = */ (int (DCALL *)(DeeObject *, size_t))&rodict_hasitem_index,
	/* .tp_getrange_index               = */ NULL,
	/* .tp_delrange_index               = */ NULL,
	/* .tp_setrange_index               = */ NULL,
	/* .tp_getrange_index_n             = */ NULL,
	/* .tp_delrange_index_n             = */ NULL,
	/* .tp_setrange_index_n             = */ NULL,
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_trygetitem,
	/* .tp_trygetitem_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rodict_trygetitem_index,
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_bounditem_string_hash,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_hasitem_string_len_hash,
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_unpack                       = */ NULL,
	/* .tp_unpack_ex                    = */ NULL,
	/* .tp_unpack_ub                    = */ NULL,
	/* .tp_getitemnr                    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&rodict_getitemnr,
	/* .tp_getitemnr_string_hash        = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&rodict_getitemnr_string_hash,
	/* .tp_getitemnr_string_len_hash    = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&rodict_getitemnr_string_len_hash,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&rodict_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&rodict_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&rodict_trygetitemnr_string_len_hash,
};


#define D_TKey   "?O"
#define D_TValue "?O"
#define D_TItem  "?T2" D_TKey D_TValue

PRIVATE struct type_getset tpconst rodict_getsets[] = {
	TYPE_GETTER_BOUND(STR_first, &rodict_getfirst, &rodict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETTER_BOUND(STR_last, &rodict_getlast, &rodict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETTER_BOUND("firstkey", &rodict_getfirstkey, &rodict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETTER_BOUND("lastkey", &rodict_getlastkey, &rodict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETTER_BOUND("firstvalue", &rodict_getfirstvalue, &rodict_nonempty_as_bound, "->" D_TValue),
	TYPE_GETTER_BOUND("lastvalue", &rodict_getlastvalue, &rodict_nonempty_as_bound, "->" D_TValue),

	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &rodict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__hidxio__", &rodict___hidxio__, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Size shift-multipler for htab words (word size is ${1 << __hidxio__})\n"
	                 "#T{?#__hidxio__|htab word type~"
	                 /**/ "$0|?Ectypes:uint8_t"
	                 /**/ IF_DEE_DICT_HIDXIO_COUNT_GE_2("&" "$1|?Ectypes:uint16_t")
	                 /**/ IF_DEE_DICT_HIDXIO_COUNT_GE_3("&" "$2|?Ectypes:uint32_t")
	                 /**/ IF_DEE_DICT_HIDXIO_COUNT_GE_4("&" "$3|?Ectypes:uint64_t")
	                 "}"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst rodict_members[] = {
	TYPE_MEMBER_FIELD_DOC("__vsize__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(RoDict, rd_vsize), "## of used vtab slots (excluding deleted slots; same as ?#{op:size})"),
	TYPE_MEMBER_FIELD_DOC("__hmask__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(RoDict, rd_hmask), "Currently active hash-mask"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rodict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst rodict_methods[] = {
//	TYPE_KWMETHOD("byhash", &rodict_byhash, DOC_GET(map_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_size__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_compare__),
	TYPE_METHOD_HINTREF(__seq_compare_eq__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rodict_method_hints[] = {
	/* Operators for "Dict as Sequence" (specifically defined because dicts are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &rodict_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rodict_mh_seq_enumerate_index_reverse, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &rodict_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_trygetlast, &rodict_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_iter, &rodict_iter, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &rodict_mh_seq_foreach, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach_pair, &rodict_foreach_pair, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_enumerate_index, &rodict_mh_seq_enumerate_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_size, &rodict_size, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &rodict_mh_seq_getitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &rodict_mh_seq_trygetitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &rodict_mh_seq_compare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &rodict_mh_seq_compare, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &rodict_mh_seq_trycompare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst rodict_class_methods[] = {
	TYPE_KWMETHOD_F("fromkeys", &rodict_fromkeys_f, METHOD_FNOREFESCAPE,
	                "(keys:?DSet,value=!N,valuefor?:?DCallable)->?.\n"
	                "Construct a new ?. from @keys, and @value (or ${valuefor(key)}) as value."),
	TYPE_METHOD_END
};

PRIVATE struct type_operator const rodict_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeRoDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDict",
	/* .tp_doc      = */ DOC("Read-only / frozen variant of ?DDict\n"
	                         "\n"

	                         "()\n"
	                         "Returns an empty ?.\n"
	                         "\n"

	                         "(dict:?DDict)\n"
	                         "(map:?M?O?O)\n"
	                         "(seq:?ST2?O?O)\n"
	                         "Convert the given dict/map/seq into an ?. and return it"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW, /* RoDict offers KW-support (because items cannot be removed, values can be loaded as NOREF) */
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rodict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&rodict_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rodict_init,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rodict_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__667432E5904B49F8),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__40D3D60A1F18CAE2), /* TODO: &rodict_cmp */
	/* .tp_seq           = */ &rodict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rodict_methods,
	/* .tp_getsets       = */ rodict_getsets,
	/* .tp_members       = */ rodict_members,
	/* .tp_class_methods = */ rodict_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rodict_class_members,
	/* .tp_method_hints  = */ rodict_method_hints,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ rodict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(rodict_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */
