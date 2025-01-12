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

#include <deemon/api.h>

#include <hybrid/typecore.h>
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
#include "seq/hashfilter.h"

/**/
#include "dict.h"

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

#ifdef CONFIG_EXPERIMENTAL_ORDERED_RODICTS

/************************************************************************/
/* RODICT HELPERS                                                       */
/************************************************************************/
#define _RoDict_TryMalloc(sizeof)     ((DREF RoDict *)DeeObject_TryMalloc(sizeof))
#define _RoDict_TryCalloc(sizeof)     ((DREF RoDict *)DeeObject_TryCalloc(sizeof))
#define _RoDict_TryRealloc(p, sizeof) ((DREF RoDict *)DeeObject_TryRealloc(p, sizeof))
#define _RoDict_Malloc(sizeof)        ((DREF RoDict *)DeeObject_Malloc(sizeof))
#define _RoDict_Calloc(sizeof)        ((DREF RoDict *)DeeObject_Calloc(sizeof))
#define _RoDict_Realloc(p, sizeof)    ((DREF RoDict *)DeeObject_Realloc(p, sizeof))
#define _RoDict_Free(p)               DeeObject_Free(p)

#define _RoDict_SizeOf(valloc, hmask) \
	_RoDict_SizeOf3(valloc, hmask, DEE_DICT_HIDXIO_FROMALLOC(valloc))
#define _RoDict_SafeSizeOf(valloc, hmask) \
	_RoDict_SafeSizeOf3(valloc, hmask, DEE_DICT_HIDXIO_FROMALLOC(valloc))
#define _RoDict_SizeOf3(valloc, hmask, hidxio)   \
	(offsetof(DeeRoDictObject, rd_vtab) +               \
	 ((size_t)(valloc) * sizeof(struct Dee_dict_item)) + \
	 (((size_t)(hmask) + 1) << hidxio))
LOCAL ATTR_CONST size_t DCALL
_RoDict_SafeSizeOf3(size_t valloc, size_t hmask, shift_t hidxio) {
	size_t result, hmask_size;
	if (OVERFLOW_UMUL(valloc, sizeof(struct Dee_dict_item), &result))
		goto toobig;
	if (OVERFLOW_UADD(hmask, 1, &hmask_size))
		goto toobig;
	if unlikely(((hmask_size << hidxio) >> hidxio) != hmask_size)
		goto toobig;
	hmask_size <<= hidxio;
	if (OVERFLOW_UADD(result, hmask_size, &result))
		goto toobig;
	if (OVERFLOW_UADD(result, offsetof(DeeRoDictObject, rd_vtab), &result))
		goto toobig;
	return result;
toobig:
	return (size_t)-1; /* Force down-stream allocation failure */
}

PRIVATE NONNULL((1, 3)) void DCALL
hmask_memcpy_and_maybe_downcast(void *__restrict dst, shift_t dst_hidxio,
                                void const *__restrict src, shift_t src_hidxio,
                                size_t n_words) {
	ASSERT(dst_hidxio <= src_hidxio);
	if likely(dst_hidxio == src_hidxio) {
		memcpy(dst, src, n_words << dst_hidxio);
	} else {
		size_t i;
		Dee_dict_gethidx_t src_get = Dee_dict_hidxio[src_hidxio].dhxio_get;
		Dee_dict_sethidx_t dst_set = Dee_dict_hidxio[dst_hidxio].dhxio_set;
		for (i = 0; i < n_words; ++i) {
			/*virt*/Dee_dict_vidx_t word;
			word = (*src_get)(src, i);
			(*dst_set)(dst, i, word);
		}
	}
}

PRIVATE NONNULL((1, 3)) void DCALL
hmask_memmovedown_and_maybe_downcast(void *dst, shift_t dst_hidxio,
                                     void const *src, shift_t src_hidxio,
                                     size_t n_words) {
	ASSERT(dst_hidxio <= src_hidxio);
	if likely(dst_hidxio == src_hidxio) {
		memmovedown(dst, src, n_words << dst_hidxio);
	} else {
		Dee_dict_gethidx_t src_get = Dee_dict_hidxio[src_hidxio].dhxio_get;
		Dee_dict_sethidx_t dst_set = Dee_dict_hidxio[dst_hidxio].dhxio_set;
		while (n_words) {
			/*virt*/Dee_dict_vidx_t word;
			--n_words;
			word = (*src_get)(src, n_words);
			(*dst_set)(dst, n_words, word);
		}
	}
}

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
	ASSERT(old_htab == _DeeRoDict_GetRealVTab(self) + self->rd_vsize);
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
				rodict_htab_decafter(dict, me->rdb_hidxset, vtab_idx);
				item = &_DeeRoDict_GetRealVTab(dict)[dict->rd_vsize - 1];
				item->di_hash = hash;
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
#define rditer_fini  generic_proxy_fini
#define rditer_visit generic_proxy_visit

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
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rditer_compare,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rditer_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &rditer_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_enumerate_index(RoDict *__restrict self, Dee_enumerate_index_t cb,
                       void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	Dee_hash_t hash;
	for (hash = start; hash < end; ++hash) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item;
		/* Search for an integer-key matching "hash" */
		_DeeRoDict_HashIdxInit(self, &hs, &perturb, hash);
		for (;; _DeeRoDict_HashIdxAdv(self, &hs, &perturb)) {
			size_t key_value;
			/*virt*/ Dee_dict_vidx_t vtab_idx = _DeeRoDict_HTabGet(self, hs);
			if (vtab_idx == Dee_DICT_HTAB_EOF)
				goto continue_with_next_hash;
			item = &_DeeRoDict_GetVirtVTab(self)[vtab_idx];
			if unlikely(item->di_hash != hash)
				continue;
			if likely(DeeInt_Check(item->di_key)) {
				if unlikely(!DeeInt_TryAsSize(item->di_key, &key_value))
					continue;
				if unlikely(key_value != hash)
					continue;
			} else {
				bool matches;
				DREF DeeObject *int_key;
#ifndef __OPTIMIZE_SIZE__
				/* Check: can "key" be converted to an integer; if not, then it can't be correct.
				 * NOTE: This same check is semantically repeated by the
				 *       `DeeError_Catch(&DeeError_NotImplemented)' below. */
				DeeTypeObject *tp_key = Dee_TYPE(item->di_key);
				if ((!tp_key->tp_math || !tp_key->tp_math->tp_int) && !DeeType_InheritInt(tp_key))
					continue;
#endif /* !__OPTIMIZE_SIZE__ */
				int_key = DeeObject_Int(item->di_key);
				if unlikely(!int_key) {
					if (DeeError_Catch(&DeeError_NotImplemented))
						continue;
					goto err;
				}
				matches = DeeInt_TryAsSize(int_key, &key_value) && key_value == hash;
				Dee_Decref(int_key);
				if unlikely(!matches)
					continue;
			}
			break;
		}
		ASSERT(item->di_hash == hash);
		temp = (*cb)(arg, hash, item->di_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
continue_with_next_hash:;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}



PRIVATE struct type_seq rodict_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_iter,
	/* .tp_sizeob                       = */ NULL_IF_Os((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_sizeob),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_getitem,
	/* .tp_delitem                      = */ NULL,
	/* .tp_setitem                      = */ NULL,
	/* .tp_getrange                     = */ NULL,
	/* .tp_delrange                     = */ NULL,
	/* .tp_setrange                     = */ NULL,
	/* .tp_foreach                      = */ NULL,
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_enumerate                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_enumerate_index              = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&rodict_enumerate_index,
	/* .tp_iterkeys                     = */ &DeeMap_DefaultIterKeysWithIter,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_bounditem,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size_fast,
	/* .tp_getitem_index                = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rodict_getitem_index,
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ NULL,
	/* .tp_setitem_index                = */ NULL,
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
	/* .tp_delitem_string_hash          = */ NULL,
	/* .tp_setitem_string_hash          = */ NULL,
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_bounditem_string_hash,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ NULL,
	/* .tp_setitem_string_len_hash      = */ NULL,
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

	TYPE_GETTER(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_F("__sizeof__", &rodict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_F("__hidxio__", &rodict___hidxio__, METHOD_FNOREFESCAPE,
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
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rodict_method_hints[] = {
	/* Operators for "Dict as Sequence" (specifically defined because dicts are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &rodict_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rodict_mh_seq_enumerate_index_reverse, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &rodict_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_trygetlast, &rodict_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &rodict_mh_seq_foreach, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach_pair, &rodict_foreach_pair, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_enumerate_index, &rodict_mh_seq_enumerate_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_size, &rodict_size, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_size_fast, &rodict_size_fast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
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
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rodict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO: &rodict_cmp */
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
	/* .tp_operators_size= */ COMPILER_LENOF(rodict_operators)
};

#else /* CONFIG_EXPERIMENTAL_ORDERED_RODICTS */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

typedef struct {
	PROXY_OBJECT_HEAD_EX(RoDict, rodi_dict) /* [1..1][const] The RoDict being iterated. */
	struct rodict_item          *rodi_next; /* [?..1][in(rodi_dict->rd_elem)][atomic]
	                                         * The first candidate for the next item. */
} RoDictIterator;
#define READ_ITEM(x) atomic_read(&(x)->rodi_next)

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodictiterator_ctor(RoDictIterator *__restrict self) {
	self->rodi_dict = DeeRoDict_New();
	if unlikely(!self->rodi_dict)
		goto err;
	self->rodi_next = self->rodi_dict->rd_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodictiterator_init(RoDictIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	RoDict *RoDict;
	if (DeeArg_Unpack(argc, argv, "o:_RoDictIterator", &RoDict))
		goto err;
	if (DeeObject_AssertTypeExact(RoDict, &DeeRoDict_Type))
		goto err;
	self->rodi_dict = RoDict;
	Dee_Incref(RoDict);
	self->rodi_next = RoDict->rd_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodictiterator_copy(RoDictIterator *__restrict self,
                    RoDictIterator *__restrict other) {
	self->rodi_dict = other->rodi_dict;
	Dee_Incref(self->rodi_dict);
	self->rodi_next = READ_ITEM(other);
	return 0;
}

STATIC_ASSERT(offsetof(RoDictIterator, rodi_dict) == offsetof(ProxyObject, po_obj));
#define rodictiterator_fini  generic_proxy_fini
#define rodictiterator_visit generic_proxy_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodictiterator_bool(RoDictIterator *__restrict self) {
	struct rodict_item *item = READ_ITEM(self);
	RoDict *dict = self->rodi_dict;
	for (;; ++item) {
		/* Check if the iterator is in-bounds. */
		if (item > dict->rd_elem + dict->rd_mask)
			return 0;
		if (item->rdi_key)
			break;
	}
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_next(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	RoDict *dict = self->rodi_dict;
	end = dict->rd_elem + dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return DeeTuple_Pack(2, item->rdi_key, item->rdi_value);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodictiterator_nextpair(RoDictIterator *__restrict self,
                        DREF DeeObject *key_and_value[2]) {
	struct rodict_item *item, *end;
	RoDict *dict = self->rodi_dict;
	end = dict->rd_elem + dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	key_and_value[0] = item->rdi_key;
	key_and_value[1] = item->rdi_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	return 0;
iter_exhausted:
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_nextkey(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	RoDict *dict = self->rodi_dict;
	end = dict->rd_elem + dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->rdi_key);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_nextvalue(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	RoDict *dict = self->rodi_dict;
	end = dict->rd_elem + dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->rdi_value);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE struct type_iterator rodictiterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&rodictiterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodictiterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodictiterator_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&rodictiterator_advance,
};

INTDEF DeeTypeObject RoDictIterator_Type;

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
rodictiterator_hash(RoDictIterator *self) {
	return Dee_HashPointer(READ_ITEM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodictiterator_compare(RoDictIterator *self, RoDictIterator *other) {
	if (DeeObject_AssertType(other, &RoDictIterator_Type))
		goto err;
	Dee_return_compareT(struct rodict_item *, READ_ITEM(self),
	                    /*                 */ READ_ITEM(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp rodictiterator_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&rodictiterator_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodictiterator_compare,
};


PRIVATE struct type_member tpconst rodict_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RoDictIterator, rodi_dict), "->?Ert:RoDict"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject RoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(dict:?Ert:RoDict)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rodictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rodictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&rodictiterator_init,
				TYPE_FIXED_ALLOCATOR(RoDictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rodictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rodictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodictiterator_next,
	/* .tp_iterator      = */ &rodictiterator_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rodict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






#define RODICT_ALLOC(mask)  ((DREF RoDict *)DeeObject_Calloc(SIZEOF_RODICT(mask)))
#define SIZEOF_RODICT(mask) _Dee_MallococBufsize(offsetof(RoDict, rd_elem), (mask) + 1, sizeof(struct rodict_item))
#define RODICT_INITIAL_MASK 0x03

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
DeeRoDict_Rehash(/*inherit(on_success)*/ DREF RoDict *__restrict self, size_t new_mask) {
	DREF RoDict *result;
	size_t i;
	result = RODICT_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	ASSERT(self->ob_refcnt == 1);
	ASSERT(self->ob_type == &DeeRoDict_Type);
	result->ob_refcnt = 1;
	result->ob_type = &DeeRoDict_Type;
	result->rd_size = self->rd_size;
	result->rd_mask = new_mask;
	for (i = 0; i <= self->rd_mask; ++i) {
		size_t j, perturb;
		struct rodict_item *item;
		if (!self->rd_elem[i].rdi_key)
			continue;
		perturb = j = self->rd_elem[i].rdi_hash & new_mask;
		for (;; DeeRoDict_HashNx(j, perturb)) {
			item = &result->rd_elem[j & new_mask];
			if (!item->rdi_key)
				break;
		}
		/* Copy the old item into the new slot. */
		memcpy(item, &self->rd_elem[i], sizeof(struct rodict_item));
	}
	DeeObject_Free(self);
done:
	return result;
}


PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeRoDict_Insert(/*in|out*/ DREF RoDict **__restrict p_self,
                 DeeObject *key, DeeObject *value) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	DREF RoDict *me = *p_self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeRoDict_Type);
	ASSERT(!DeeObject_IsShared(me));
	ASSERT(key != (DeeObject *)me);
	ASSERT(value != (DeeObject *)me);
	if unlikely(me->rd_size * 2 > me->rd_mask) {
		size_t new_mask = (me->rd_mask << 1) | 1;
		me = DeeRoDict_Rehash(me, new_mask);
		if unlikely(!me)
			goto err;
		*p_self = me;
	}

	/* Insert the new key/value-pair into the RoDict. */
	hash    = DeeObject_Hash(key);
	perturb = i = hash & me->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;

		/* Same hash. -> Check if it's also the same key. */
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Not the same key. */

		/* It _is_ the same key! (override it...) */
		--me->rd_size;
		Dee_Decref(item->rdi_key);
		Dee_Decref(item->rdi_value);
		break;
	}

	/* Fill in the item. */
	++me->rd_size;
	item->rdi_hash  = hash;
	item->rdi_key   = key;
	item->rdi_value = value;
	Dee_Incref(key);
	Dee_Incref(value);
	return 0;
err:
	return -1;
}


#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define DeeRoDict_InsertSequence_foreach_PTR ((Dee_foreach_pair_t)&DeeRoDict_Insert)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#define DeeRoDict_InsertSequence_foreach_PTR &DeeRoDict_InsertSequence_foreach
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
DeeRoDict_InsertSequence_foreach(void *arg, DeeObject *key, DeeObject *value) {
	return DeeRoDict_Insert((RoDict **)arg, key, value);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict sequence) {
	DREF RoDict *result;
	size_t length_hint, mask;

	/* Optimization: Since rodicts are immutable, re-return if the
	 *               given sequence already is a read-only RoDict. */
	if (DeeRoDict_CheckExact(sequence))
		return_reference_(sequence);

	/* TODO: Optimization: if (DeeDict_CheckExact(sequence)) ...
	 * (fix the dict's hash-vector to not contain dummies,
	 * then copy as-is) */

	/* Construct a read-only RoDict from a generic sequence. */
	mask        = RODICT_INITIAL_MASK;
	length_hint = DeeObject_SizeFast(sequence);
	if (length_hint != (size_t)-1) {
		while (mask <= length_hint)
			mask = (mask << 1) | 1;
		mask = (mask << 1) | 1;
	}
	result = RODICT_ALLOC(mask);
	if likely(result) {
		/*result->rd_size = 0;*/
		result->rd_mask = mask;
		DeeObject_Init(result, &DeeRoDict_Type);
		if unlikely(DeeObject_ForeachPair(sequence, DeeRoDict_InsertSequence_foreach_PTR, &result))
			goto err_r;
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_DecrefDokill(result);
/*err:*/
	return NULL;
}

PUBLIC WUNUSED DREF RoDict *DCALL
DeeRoDict_New(void) {
	DREF RoDict *result;
	result = RODICT_ALLOC(RODICT_INITIAL_MASK);
	if unlikely(!result)
		goto done;
	result->rd_mask = RODICT_INITIAL_MASK;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PUBLIC WUNUSED DREF RoDict *DCALL
DeeRoDict_NewWithHint(size_t num_items) {
	DREF RoDict *result;
	size_t mask = RODICT_INITIAL_MASK;
	while (mask < num_items)
		mask = (mask << 1) | 1;
	mask   = (mask << 1) | 1;
	result = RODICT_ALLOC(mask);
	if unlikely(!result)
		goto done;
	result->rd_mask = mask;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDictIterator *DCALL
rodict_iter(RoDict *__restrict self) {
	DREF RoDictIterator *result;
	result = DeeObject_MALLOC(RoDictIterator);
	if unlikely(!result)
		goto done;
	result->rodi_dict = self;
	result->rodi_next = self->rd_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &RoDictIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_contains(RoDict *self, DeeObject *key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_true;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr(RoDict *__restrict self, DeeObject *__restrict key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		/* Found it! */
		return item->rdi_value;
	}
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr_string_hash(DeeRoDictObject *__restrict self,
                             char const *__restrict key,
                             Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return item->rdi_value; /* Found it! */
	}
	err_unknown_key_str((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_getitemnr_string_len_hash(DeeRoDictObject *__restrict self,
                                 char const *__restrict key,
                                 size_t keylen, Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return item->rdi_value; /* Found it! */
	}
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_trygetitemnr(DeeRoDictObject *__restrict self,
                    /*string*/ DeeObject *__restrict key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash     = DeeObject_Hash(key);
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		/* Found it! */
		return item->rdi_value;
	}
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_trygetitemnr_string_hash(DeeRoDictObject *__restrict self,
                                char const *__restrict key,
                                Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return item->rdi_value; /* Found it! */
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
rodict_trygetitemnr_string_len_hash(DeeRoDictObject *__restrict self,
                                    char const *__restrict key,
                                    size_t keylen, Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return item->rdi_value; /* Found it! */
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_ByHash(DeeRoDictObject *__restrict self, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeTupleObject *match;
	Dee_hash_t i, perturb;
	match = NULL;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		struct rodict_item *item;
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break; /* Not found */
		if (item->rdi_hash != hash)
			continue; /* Non-matching hash */
		if unlikely(match) {
			/* There are multiple matches for `hash'. */
			Dee_Decref(match);
			/* XXX: RoDict-specific optimizations? */
			return DeeMap_HashFilter((DeeObject *)self, hash);
		}
		match = DeeTuple_NewUninitialized(2);
		if unlikely(!match)
			goto err;
		DeeTuple_SET(match, 0, item->rdi_key);
		DeeTuple_SET(match, 1, item->rdi_value);
		Dee_Incref(item->rdi_key);
		Dee_Incref(item->rdi_value);
	}
	if (!match)
		return_empty_tuple;
	result = (DREF DeeObject *)DeeTuple_NewUninitialized(1);
	if unlikely(!result)
		goto err_match;
	DeeTuple_SET(result, 0, match); /* Inherit reference */
	return result;
err_match:
	Dee_Decref(match);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem(RoDict *self, DeeObject *key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_reference_(item->rdi_value);
	}
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_hash(DeeRoDictObject *__restrict self,
                           char const *__restrict key,
                           Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return_reference_(item->rdi_value);
	}
	err_unknown_key_str((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_len_hash(DeeRoDictObject *__restrict self,
                               char const *__restrict key,
                               size_t keylen, Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return_reference_(item->rdi_value);
	}
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rodict_fini(RoDict *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].rdi_key)
			continue;
		Dee_Decref(self->rd_elem[i].rdi_key);
		Dee_Decref(self->rd_elem[i].rdi_value);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
rodict_visit(RoDict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].rdi_key)
			continue;
		Dee_Visit(self->rd_elem[i].rdi_key);
		Dee_Visit(self->rd_elem[i].rdi_value);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_bool(RoDict *__restrict self) {
	return self->rd_size != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_printrepr(RoDict *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	bool is_first = true;
	size_t i;
	result = DeeFormat_PRINT(printer, arg, "Dict.Frozen({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].rdi_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "%r: %r",
		                         self->rd_elem[i].rdi_key,
		                         self->rd_elem[i].rdi_value));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rodict_size(RoDict *__restrict self) {
	ASSERT(self->rd_size != (size_t)-1);
	return self->rd_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_foreach_pair(RoDict *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		DeeObject *key;
		key = self->rd_elem[i].rdi_key;
		if (!key)
			continue;
		temp = (*proc)(arg, key, self->rd_elem[i].rdi_value);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem(RoDict *self, DeeObject *key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		return 1;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem_string_hash(DeeRoDictObject *__restrict self,
                           char const *__restrict key,
                           Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return 1; /* Found it! */
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_hasitem_string_len_hash(DeeRoDictObject *__restrict self,
                               char const *__restrict key,
                               size_t keylen, Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return 1; /* Found it! */
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem(RoDict *self, DeeObject *key) {
	Dee_hash_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; DeeRoDict_HashNx(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_TryCompareEq(key, item->rdi_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err;
		if (error != 0)
			continue; /* Non-equal keys. */
		return_reference_(item->rdi_value);
	}
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_hash(DeeRoDictObject *__restrict self,
                              char const *__restrict key,
                              Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return_reference_(item->rdi_value);
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_len_hash(DeeRoDictObject *__restrict self,
                                  char const *__restrict key,
                                  size_t keylen, Dee_hash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		item = DeeRoDict_HashIt(self, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return_reference_(item->rdi_value);
	}
	return ITER_DONE;
}

PRIVATE struct type_seq rodict_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_iter,
	/* .tp_sizeob                       = */ NULL,
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_getitem,
	/* .tp_delitem                      = */ NULL,
	/* .tp_setitem                      = */ NULL,
	/* .tp_getrange                     = */ NULL,
	/* .tp_delrange                     = */ NULL,
	/* .tp_setrange                     = */ NULL,
	/* .tp_foreach                      = */ NULL,
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_enumerate                    = */ NULL,
	/* .tp_enumerate_index              = */ NULL,
	/* .tp_iterkeys                     = */ NULL,
	/* .tp_bounditem                    = */ NULL,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_getitem_index                = */ NULL,
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ NULL,
	/* .tp_setitem_index                = */ NULL,
	/* .tp_bounditem_index              = */ NULL,
	/* .tp_hasitem_index                = */ NULL,
	/* .tp_getrange_index               = */ NULL,
	/* .tp_delrange_index               = */ NULL,
	/* .tp_setrange_index               = */ NULL,
	/* .tp_getrange_index_n             = */ NULL,
	/* .tp_delrange_index_n             = */ NULL,
	/* .tp_setrange_index_n             = */ NULL,
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_trygetitem,
	/* .tp_trygetitem_index             = */ NULL,
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ NULL,
	/* .tp_setitem_string_hash          = */ NULL,
	/* .tp_bounditem_string_hash        = */ NULL,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ NULL,
	/* .tp_setitem_string_len_hash      = */ NULL,
	/* .tp_bounditem_string_len_hash    = */ NULL,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_sizeof(RoDict *self) {
	size_t result;
	result = _Dee_MallococBufsize(offsetof(RoDict, rd_elem),
	                              self->rd_mask + 1,
	                              sizeof(struct rodict_item));
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_byhash(RoDict *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
		goto err;
	return DeeRoDict_ByHash(self, DeeObject_Hash(template_));
err:
	return NULL;
}


DOC_REF(map_byhash_doc);

PRIVATE struct type_method tpconst rodict_methods[] = {
	TYPE_KWMETHOD("byhash", &rodict_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rodict_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_F("__sizeof__", &rodict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst rodict_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoDict, rd_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoDict, rd_size)),
	TYPE_MEMBER_END
};


PRIVATE struct type_member tpconst rodict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF RoDict *DCALL rodict_ctor(void) {
	return_reference_((DREF RoDict *)Dee_EmptyRoDict);
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_deepcopy(RoDict *__restrict self) {
	DREF RoDict *result;
	size_t i;
	int temp;
	result = (DREF RoDict *)DeeRoDict_NewWithHint(self->rd_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->rd_mask; ++i) {
		DREF DeeObject *key_copy, *value_copy;
		/* Deep-copy the key & value */
		if (!self->rd_elem[i].rdi_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->rd_elem[i].rdi_key);
		if unlikely(!key_copy)
			goto err;
		value_copy = DeeObject_DeepCopy(self->rd_elem[i].rdi_value);
		if unlikely(!value_copy) {
			Dee_Decref(key_copy);
			goto err;
		}
		/* Insert the copied key & value into the new RoDict. */
		temp = DeeRoDict_Insert(&result, key_copy, value_copy);
		Dee_Decref(value_copy);
		Dee_Decref(key_copy);
		if unlikely(temp)
			goto err;
	}
done:
	return result;
err:
	Dee_Clear(result);
	goto done;
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

/* The main `_RoDict' container class. */
PUBLIC DeeTypeObject DeeRoDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDict",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
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
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rodict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO: &rodict_cmp */
	/* .tp_seq           = */ &rodict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rodict_methods,
	/* .tp_getsets       = */ rodict_getsets,
	/* .tp_members       = */ rodict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rodict_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ rodict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(rodict_operators)
};

/* Special empty instance of `DeeRoDict_Type' */
PUBLIC struct Dee_empty_rodict_object DeeRoDict_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeRoDict_Type),
	/* .rd_mask = */ 0,
	/* .rd_size = */ 0,
	/* .rd_elem = */ {{ NULL, NULL, 0 }}
};

#endif /* !CONFIG_EXPERIMENTAL_ORDERED_RODICTS */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */
