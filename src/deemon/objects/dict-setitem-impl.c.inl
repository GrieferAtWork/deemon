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
#ifdef __INTELLISENSE__
#include "dict.c"
//#define DEFINE_dict_setitem
#define DEFINE_dict_setitem_at
//#define DEFINE_dict_setitem_string_hash
//#define DEFINE_dict_setitem_index
//#define DEFINE_dict_setitem_string_len_hash
//#define DEFINE_dict_setitem_unlocked
//#define DEFINE_dict_setitem_unlocked_fast_inherited
//#define DEFINE_dict_mh_setold_ex
//#define DEFINE_dict_mh_setnew_ex
//#define DEFINE_dict_mh_setdefault
#endif /* __INTELLISENSE__ */


#if (defined(DEFINE_dict_setitem) +                         \
     defined(DEFINE_dict_setitem_at) +                      \
     defined(DEFINE_dict_setitem_string_hash) +             \
     defined(DEFINE_dict_setitem_index) +                   \
     defined(DEFINE_dict_setitem_string_len_hash) +         \
     defined(DEFINE_dict_setitem_unlocked) +                \
     defined(DEFINE_dict_setitem_unlocked_fast_inherited) + \
     defined(DEFINE_dict_mh_setold_ex) +                    \
     defined(DEFINE_dict_mh_setnew_ex) +                    \
     defined(DEFINE_dict_mh_setdefault)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_dict_setitem
#define LOCAL_dict_setitem dict_setitem
#elif defined(DEFINE_dict_setitem_at)
#define LOCAL_dict_setitem dict_setitem_at
#define LOCAL_HAVE_CUSTOM_INDEX
#elif defined(DEFINE_dict_setitem_string_hash)
#define LOCAL_dict_setitem dict_setitem_string_hash
#define LOCAL_HAVE_KEY_IS_STRING_HASH
#elif defined(DEFINE_dict_setitem_index)
#define LOCAL_dict_setitem dict_setitem_index
#define LOCAL_HAVE_KEY_IS_INDEX
#elif defined(DEFINE_dict_setitem_string_len_hash)
#define LOCAL_dict_setitem dict_setitem_string_len_hash
#define LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
#elif defined(DEFINE_dict_setitem_unlocked)
#define LOCAL_dict_setitem dict_setitem_unlocked
#define LOCAL_HAVE_UNLOCKED
#elif defined(DEFINE_dict_setitem_unlocked_fast_inherited)
#define LOCAL_dict_setitem dict_setitem_unlocked_fast_inherited
#define LOCAL_HAVE_UNLOCKED
#define LOCAL_HAVE_FAST
#define LOCAL_HAVE_INHERITED
#elif defined(DEFINE_dict_mh_setold_ex)
#define LOCAL_dict_setitem dict_mh_setold_ex
#define LOCAL_HAVE_SETOLD
#elif defined(DEFINE_dict_mh_setnew_ex)
#define LOCAL_dict_setitem dict_mh_setnew_ex
#define LOCAL_HAVE_SETNEW
#elif defined(DEFINE_dict_mh_setdefault)
#define LOCAL_dict_setitem dict_mh_setdefault
#define LOCAL_HAVE_SETNEW
#define LOCAL_HAVE_SETDEFAULT
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

#if defined(LOCAL_HAVE_SETNEW) || defined(LOCAL_HAVE_SETOLD)
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_ERR         NULL
#else /* LOCAL_HAVE_SETNEW || LOCAL_HAVE_SETOLD */
#define LOCAL_return_type int
#define LOCAL_ERR         (-1)
#endif /* !LOCAL_HAVE_SETNEW && !LOCAL_HAVE_SETOLD */

#ifdef LOCAL_HAVE_KEY_IS_STRING_HASH
#define LOCAL_NONNULL    NONNULL((1, 2, 4))
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAVE_KEY_IS_STRING
#define LOCAL_HAVE_NON_OBJECT_KEY
#elif defined(LOCAL_HAVE_KEY_IS_STRING_LEN_HASH)
#define LOCAL_NONNULL    NONNULL((1, 2, 5))
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAVE_KEY_IS_STRING
#define LOCAL_HAVE_NON_OBJECT_KEY
#elif defined(LOCAL_HAVE_KEY_IS_INDEX)
#define LOCAL_NONNULL    NONNULL((1, 3))
#define LOCAL_KEY_PARAMS size_t index
#define LOCAL_HAVE_NON_OBJECT_KEY
#else /* ... */
#define LOCAL_NONNULL    NONNULL((1, 2, 3))
#ifdef LOCAL_HAVE_INHERITED
#define LOCAL_KEY_PARAMS /*inherit(on_success)*/ DREF DeeObject *key
#else /* LOCAL_HAVE_INHERITED */
#define LOCAL_KEY_PARAMS DeeObject *key
#endif /* !LOCAL_HAVE_INHERITED */
#endif /* !... */

#undef LOCAL_HAVE_UNLOCKED_OR_NOTHREADS
#if defined(LOCAL_HAVE_UNLOCKED) || defined(CONFIG_NO_THREADS)
#define LOCAL_HAVE_UNLOCKED_OR_NOTHREADS
#endif /* LOCAL_HAVE_UNLOCKED || CONFIG_NO_THREADS */

#ifndef LOCAL_HAVE_UNLOCKED_OR_NOTHREADS
#define LOCAL_IF_NOT_UNLOCKED_AND_THREADS(...) __VA_ARGS__
#else /* !LOCAL_HAVE_UNLOCKED_OR_NOTHREADS */
#define LOCAL_IF_NOT_UNLOCKED_AND_THREADS(...) /* nothing */
#endif /* LOCAL_HAVE_UNLOCKED_OR_NOTHREADS */

#ifndef LOCAL_HAVE_UNLOCKED
#define LOCAL_IF_NOT_UNLOCKED(...) __VA_ARGS__
#else /* !LOCAL_HAVE_UNLOCKED */
#define LOCAL_IF_NOT_UNLOCKED(...) /* nothing */
#endif /* LOCAL_HAVE_UNLOCKED */

#define LOCAL_DeeDict_LockTryRead(self)    LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockTryRead(self))
#define LOCAL_DeeDict_LockTryWrite(self)   LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockTryWrite(self))
#define LOCAL_DeeDict_LockRead(self)       LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockRead(self))
#define LOCAL_DeeDict_LockWrite(self)      LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockWrite(self))
#define LOCAL_DeeDict_LockTryUpgrade(self) LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockTryUpgrade(self))
#define LOCAL_DeeDict_LockUpgrade(self)    LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockUpgrade(self))
#define LOCAL_DeeDict_LockDowngrade(self)  LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockDowngrade(self))
#define LOCAL_DeeDict_LockEndWrite(self)   LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockEndWrite(self))
#define LOCAL_DeeDict_LockEndRead(self)    LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockEndRead(self))
#define LOCAL_DeeDict_LockEnd(self)        LOCAL_IF_NOT_UNLOCKED_AND_THREADS(DeeDict_LockEnd(self))


PRIVATE WUNUSED LOCAL_NONNULL LOCAL_return_type DCALL
LOCAL_dict_setitem(Dict *self, LOCAL_KEY_PARAMS
#ifdef LOCAL_HAVE_INHERITED
                   , /*inherit(on_success)*/ DREF DeeObject *value
#else /* LOCAL_HAVE_INHERITED */
                   , DeeObject *value
#endif /* !LOCAL_HAVE_INHERITED */
#ifdef LOCAL_HAVE_CUSTOM_INDEX
                   , size_t (DCALL *getindex)(void *cookie, Dict *self, /*real*/ Dee_dict_vidx_t overwrite_index)
                   , void *getindex_cookie
#endif /* LOCAL_HAVE_CUSTOM_INDEX */
                   ) {
#ifdef LOCAL_HAVE_NON_OBJECT_KEY
#ifndef LOCAL_HAVE_SETOLD
#ifdef LOCAL_HAVE_KEY_IS_STRING
	DREF DeeStringObject *keyob = NULL; /* Key string object (lazily allocated) */
#endif /* LOCAL_HAVE_KEY_IS_STRING */
#ifdef LOCAL_HAVE_KEY_IS_INDEX
	DREF DeeIntObject *keyob = NULL; /* Key index object (lazily allocated) */
#endif /* LOCAL_HAVE_KEY_IS_INDEX */
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* LOCAL_HAVE_NON_OBJECT_KEY */

#ifndef LOCAL_HAS_PARAM_HASH
#ifdef LOCAL_HAVE_KEY_IS_INDEX
#define LOCAL_hash index
#else /* LOCAL_HAVE_KEY_IS_INDEX */
	Dee_hash_t hash = DeeObject_Hash(key);
#define LOCAL_hash hash
#endif /* !LOCAL_HAVE_KEY_IS_INDEX */
#else /* !LOCAL_HAS_PARAM_HASH */
#define LOCAL_hash hash
#endif /* LOCAL_HAS_PARAM_HASH */

#ifndef LOCAL_HAVE_SETOLD
	size_t result_hidx;
#endif /* !LOCAL_HAVE_SETOLD */
	Dee_hash_t hs, perturb;
#if !defined(LOCAL_HAVE_UNLOCKED_OR_NOTHREADS) && !defined(LOCAL_HAVE_SETOLD)
	bool has_write_lock;
#define LOCAL_HAVE_has_write_lock
#endif /* ... */


/*again:*/
#ifdef LOCAL_HAVE_has_write_lock
	has_write_lock = false;
#endif /* LOCAL_HAVE_has_write_lock */
	LOCAL_DeeDict_LockRead(self);
#if !defined(LOCAL_HAVE_FAST)
again_locked:
#endif /* !LOCAL_HAVE_FAST */
#ifndef LOCAL_HAVE_SETOLD
	result_hidx = (size_t)-1;
#endif /* !LOCAL_HAVE_SETOLD */
	_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
#ifndef LOCAL_HAVE_KEY_IS_STRING
		int cmp;
#ifdef LOCAL_HAVE_SETNEW
		DREF DeeObject *item_value;
#endif /* LOCAL_HAVE_SETNEW */
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
		DREF DeeObject *item_key;
		struct Dee_dict_item *item;
		Dee_dict_vidx_t vtab_idx = _DeeDict_HTabGet(self, hs);
		if (vtab_idx == Dee_DICT_HTAB_EOF)
			break; /* End-of-chain */
		ASSERT(vtab_idx < self->d_vsize);
		item = &_DeeDict_GetVirtVTab(self)[vtab_idx];
		if (item->di_hash != LOCAL_hash)
			continue; /* Different hash */
#ifdef LOCAL_HAVE_KEY_IS_STRING
		if (!DeeString_Check(item->di_key))
			continue; /* Not a string */
#endif /* LOCAL_HAVE_KEY_IS_STRING */
		if (item->di_key == NULL) {
#ifndef LOCAL_HAVE_SETOLD
			result_hidx = hs & self->d_hmask;
#endif /* !LOCAL_HAVE_SETOLD */
			continue; /* Deleted item */
		}

		/* This might be it! */
		item_key = item->di_key;
#ifdef LOCAL_HAVE_KEY_IS_STRING
#ifdef LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
		if likely(DeeString_EqualsBuf(item->di_key, key, keylen))
#else /* LOCAL_HAVE_KEY_IS_STRING_LEN_HASH */
		if likely(strcmp(DeeString_STR(item->di_key), key) == 0)
#endif /* !LOCAL_HAVE_KEY_IS_STRING_LEN_HASH */
#else /* LOCAL_HAVE_KEY_IS_STRING */
		LOCAL_IF_NOT_UNLOCKED(Dee_Incref(item_key));
#ifdef LOCAL_HAVE_SETNEW
		item_value = item->di_key;
		LOCAL_IF_NOT_UNLOCKED(Dee_Incref(item_value));
#endif /* LOCAL_HAVE_SETNEW */

#ifdef LOCAL_HAVE_has_write_lock
		LOCAL_DeeDict_LockEnd(self);
		has_write_lock = false;
#else /* LOCAL_HAVE_has_write_lock */
		LOCAL_DeeDict_LockEndRead(self);
#endif /* !LOCAL_HAVE_has_write_lock */

#ifdef LOCAL_HAVE_KEY_IS_INDEX
		cmp = DeeInt_Size_TryCompareEq(index, item_key);
		if likely(cmp == 0 && keyob == NULL && DeeInt_Check(item_key)) {
			keyob = (DREF DeeIntObject *)item_key; /* Inherit reference */
#ifdef LOCAL_HAVE_UNLOCKED
			Dee_Incref(keyob);
#endif /* LOCAL_HAVE_UNLOCKED */
		} else
#else /* LOCAL_HAVE_KEY_IS_INDEX */
		cmp = DeeObject_TryCompareEq(key, item_key);
#endif /* !LOCAL_HAVE_KEY_IS_INDEX */
		{
			LOCAL_IF_NOT_UNLOCKED(Dee_Decref_unlikely(item_key));
		}
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
		if likely(cmp == 0)
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
		{
#ifdef LOCAL_HAVE_SETNEW
#ifdef LOCAL_HAVE_UNLOCKED
			Dee_Incref(item_value);
#endif /* LOCAL_HAVE_UNLOCKED */
#ifdef LOCAL_HAVE_NON_OBJECT_KEY
#ifndef LOCAL_HAVE_SETOLD
			if unlikely(keyob)
				Dee_DecrefDokill(keyob);
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* LOCAL_HAVE_NON_OBJECT_KEY */
#ifdef LOCAL_HAVE_INHERITED
			Dee_Decref(key);
			Dee_Decref(value);
#endif /* LOCAL_HAVE_INHERITED */
			return item_value;
#else /* LOCAL_HAVE_SETNEW */
			/* Override existing item. */
#ifdef LOCAL_HAVE_CUSTOM_INDEX
			size_t result_vtab_idx;
#endif /* LOCAL_HAVE_CUSTOM_INDEX */
			size_t htab_idx;
			DREF DeeObject *old_value;
#ifndef LOCAL_HAVE_INHERITED
#ifndef LOCAL_HAVE_KEY_IS_STRING
#ifdef LOCAL_HAVE_KEY_IS_INDEX
			if (keyob == NULL) {
				keyob = (DREF DeeIntObject *)DeeInt_NewSize(index);
				if unlikely(!keyob)
					goto err;
			}
#else /* LOCAL_HAVE_KEY_IS_INDEX */
			Dee_Incref(key);
#endif /* !LOCAL_HAVE_KEY_IS_INDEX */
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
			Dee_Incref(value);
#endif /* !LOCAL_HAVE_INHERITED */
#ifndef LOCAL_HAVE_KEY_IS_STRING
			LOCAL_DeeDict_LockWrite(self);
#else /* !LOCAL_HAVE_KEY_IS_STRING */
#ifndef LOCAL_HAVE_UNLOCKED_OR_NOTHREADS
			if (!LOCAL_DeeDict_LockUpgrade(self))
#endif /* !LOCAL_HAVE_UNLOCKED_OR_NOTHREADS */
#endif /* LOCAL_HAVE_KEY_IS_STRING */
			{
#if defined(LOCAL_HAVE_KEY_IS_STRING) ? !defined(LOCAL_HAVE_UNLOCKED_OR_NOTHREADS) : !defined(LOCAL_HAVE_UNLOCKED)
				if unlikely(item < _DeeDict_GetRealVTab(self))
					goto decref_kv_downgrade_and_again_locked;
				if unlikely(item >= (_DeeDict_GetRealVTab(self) + self->d_vsize))
					goto decref_kv_downgrade_and_again_locked;
				if unlikely(item->di_key != item_key)
					goto decref_kv_downgrade_and_again_locked;
#define NEED_decref_kv_downgrade_and_again_locked
#endif /* LOCAL_HAVE_KEY_IS_STRING ? !LOCAL_HAVE_UNLOCKED_OR_NOTHREADS : !LOCAL_HAVE_UNLOCKED */
			}

			/* Load custom item insertion index. */
			Dee_dict_vidx_virt2real(&vtab_idx);
#ifdef LOCAL_HAVE_CUSTOM_INDEX
			result_vtab_idx = (*getindex)(getindex_cookie, self, vtab_idx);
			if unlikely(result_vtab_idx == (size_t)-1)
				goto err;
			ASSERT(result_vtab_idx <= self->d_vsize);
#endif /* LOCAL_HAVE_CUSTOM_INDEX */
			old_value = item->di_value; /* Inherit reference */
			item->di_key = NULL;        /* Inherit reference (mark as deleted) */
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
			htab_idx = hs & self->d_hmask;

			if unlikely(_DeeDict_MustGrowVTab(self) && (vtab_idx < (self->d_vsize - 1))) {
				/* Must:
				 * - try to allocate more space in the vtab
				 * - or: shift the vtab (and adjust the htab) so that
				 *       a free slot appears at the end of the vtab. */
				size_t n_after;
				/* If the vtab isn't in need of optimization (yet), then try to increase its size. */
				if (!_DeeDict_ShouldOptimizeVTab(self)) {
					bool ok;
					--self->d_vused; /* Account for temporarily deleted key */
					ok = dict_tryalloc1_vtab(self);
					++self->d_vused; /* Account for key that's going to be re-added below */
					if (ok)
						goto append_at_end_of_vtab;
				}

				/* Shift the vtab to move items after the one being overwritten downwards,
				 * thus freeing up 1 extra space of memory at its far end (which we'll then
				 * use). */
				n_after = (self->d_vsize - 1) - vtab_idx;
				memmovedownc(item, item + 1, n_after, sizeof(struct Dee_dict_item));
				dict_htab_decafter(self, vtab_idx); /* "dict_htab_decafter(self, vtab_idx + 1)" would also work. */

#ifdef LOCAL_HAVE_CUSTOM_INDEX
				if (result_vtab_idx > vtab_idx)
					--result_vtab_idx;
				vtab_idx = result_vtab_idx;
				--self->d_vsize;
				dict_makespace_at(self, vtab_idx);
				++self->d_vsize;
#else /* LOCAL_HAVE_CUSTOM_INDEX */
				vtab_idx = self->d_vsize - 1;
#endif /* !LOCAL_HAVE_CUSTOM_INDEX */
				Dee_dict_vidx_real2virt(&vtab_idx);
				(*(self)->d_hidxset)((self)->d_htab, htab_idx, vtab_idx);
				item = &_DeeDict_GetVirtVTab(self)[vtab_idx];
				item->di_hash = LOCAL_hash;
#ifdef LOCAL_HAVE_KEY_IS_STRING
				item->di_key = item_key; /* Inherit reference */
#elif defined(LOCAL_HAVE_KEY_IS_INDEX)
				item->di_key = (DREF DeeObject *)keyob; /* Inherit reference */
#else /* ... */
				item->di_key = key; /* Inherit reference */
#endif /* !... */
				item->di_value = value; /* Inherit reference */

				/* If the dict should be optimized, do so now that it's reached a consistent state (again) */
				if (_DeeDict_ShouldOptimizeVTab(self))
					dict_optimize_vtab(self);
			} else {
append_at_end_of_vtab:
#ifdef LOCAL_HAVE_CUSTOM_INDEX
				dict_makespace_at(self, result_vtab_idx);
				(*(self)->d_hidxset)((self)->d_htab, htab_idx, result_vtab_idx);
				item = &_DeeDict_GetRealVTab(self)[result_vtab_idx];
#else /* LOCAL_HAVE_CUSTOM_INDEX */
				(*(self)->d_hidxset)((self)->d_htab, htab_idx, self->d_vsize);
				item = &_DeeDict_GetRealVTab(self)[self->d_vsize];
#endif /* !LOCAL_HAVE_CUSTOM_INDEX */
				++self->d_vsize;
				item->di_hash = LOCAL_hash;
#ifdef LOCAL_HAVE_KEY_IS_STRING
				item->di_key = item_key; /* Inherit reference */
#elif defined(LOCAL_HAVE_KEY_IS_INDEX)
				item->di_key = (DREF DeeObject *)keyob; /* Inherit reference */
#else /* ... */
				item->di_key = key; /* Inherit reference */
#endif /* !... */
				item->di_value = value; /* Inherit reference */
			}
			LOCAL_DeeDict_LockEndWrite(self);
#ifdef LOCAL_HAVE_KEY_IS_STRING
#ifndef LOCAL_HAVE_SETOLD
			if unlikely(keyob)
				Dee_DecrefDokill(keyob);
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* LOCAL_HAVE_KEY_IS_STRING */
#ifndef LOCAL_HAVE_KEY_IS_STRING
			Dee_Decref(item_key);
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
#ifdef LOCAL_HAVE_SETOLD
			return old_value;
#else /* LOCAL_HAVE_SETOLD */
			Dee_Decref(old_value);
			return 0;
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* !LOCAL_HAVE_SETNEW */
		}
#ifndef LOCAL_HAVE_KEY_IS_STRING
#ifdef LOCAL_HAVE_SETNEW
		LOCAL_IF_NOT_UNLOCKED(Dee_Decref_unlikely(item_value));
#endif /* LOCAL_HAVE_SETNEW */
		LOCAL_DeeDict_LockRead(self);
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
	}

#ifdef LOCAL_HAVE_SETOLD
	LOCAL_DeeDict_LockEndRead(self);
#ifdef LOCAL_HAVE_INHERITED
	Dee_Decref(key);
	Dee_Decref(value);
#endif /* LOCAL_HAVE_INHERITED */
	return ITER_DONE;
#else /* LOCAL_HAVE_SETOLD */

#ifdef LOCAL_HAVE_NON_OBJECT_KEY
	if likely(!keyob) {
#ifdef LOCAL_HAVE_UNLOCKED
#ifdef LOCAL_HAVE_KEY_IS_INDEX
		keyob = (DREF DeeIntObject *)DeeInt_NewSize(index);
#elif defined(LOCAL_HAVE_KEY_IS_STRING_LEN_HASH)
		keyob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(key, keylen, LOCAL_hash);
#else /* ... */
		keyob = (DREF DeeStringObject *)DeeString_NewWithHash(key, LOCAL_hash);
#endif /* !... */
		if unlikely(!keyob)
			goto err_nokeyob;
#else /* LOCAL_HAVE_UNLOCKED */
#ifdef LOCAL_HAVE_KEY_IS_INDEX
#if 0 /* Doesn't exist */
		keyob = (DREF DeeIntObject *)DeeInt_TryNewSize(index);
		if unlikely(!keyob)
#endif
		{
			LOCAL_DeeDict_LockEnd(self);
			keyob = (DREF DeeIntObject *)DeeInt_NewSize(index);
			if unlikely(!keyob)
				goto err_nokeyob;
			has_write_lock = false;
			goto again_locked;
		}
#else /* LOCAL_HAVE_KEY_IS_INDEX */
#ifndef LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
		size_t keylen = strlen(key);
#endif /* !LOCAL_HAVE_KEY_IS_STRING_LEN_HASH */
		keyob = (DREF DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                     keylen + 1, sizeof(char));
		if unlikely(!keyob) {
			LOCAL_DeeDict_LockEnd(self);
			keyob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(key, keylen, LOCAL_hash);
			if unlikely(!keyob)
				goto err_nokeyob;
			has_write_lock = false;
			goto again_locked;
		}
		DeeObject_Init(keyob, &DeeString_Type);
		keyob->s_data = NULL;
		keyob->s_hash = LOCAL_hash;
		keyob->s_len  = keylen;
		*(char *)mempcpyc(keyob->s_str, key, keylen, sizeof(char)) = '\0';
#endif /* !LOCAL_HAVE_KEY_IS_INDEX */
#endif /* !LOCAL_HAVE_UNLOCKED */
	}
#endif /* LOCAL_HAVE_NON_OBJECT_KEY */

	/* Need a write-lock for this next part. */
#ifndef LOCAL_HAVE_UNLOCKED
	if (!has_write_lock) {
		if unlikely(!LOCAL_DeeDict_LockUpgrade(self)) {
			has_write_lock = true;
			goto again_locked;
		}
	}
#endif /* !LOCAL_HAVE_UNLOCKED */

	/* Verify that the vtab can hold more values. */
#ifdef LOCAL_HAVE_FAST
	ASSERT(!_DeeDict_MustGrowVTab(self));
#else /* LOCAL_HAVE_FAST */
	/* Check if space is already available. */
	if (_DeeDict_MustGrowVTab(self)) {
		if (_DeeDict_ShouldOptimizeVTab(self)) {
			dict_optimize_vtab(self);
			ASSERT(!_DeeDict_MustGrowVTab(self));
		} else if likely(dict_tryalloc1_vtab(self)) {
			ASSERT(!_DeeDict_MustGrowVTab(self));
		} else if (_DeeDict_CanOptimizeVTab(self)) {
			dict_optimize_vtab(self);
			ASSERT(!_DeeDict_MustGrowVTab(self));
		} else {
#ifdef LOCAL_HAVE_UNLOCKED
			DeeDict_LockWrite(self);
#endif /* LOCAL_HAVE_UNLOCKED */
			if unlikely(dict_grow_vtab_and_relock(self))
				goto err;
#ifdef LOCAL_HAVE_UNLOCKED
			DeeDict_LockEndWrite(self);
#endif /* LOCAL_HAVE_UNLOCKED */
			goto again_with_wrlock;
		}
	}
#endif /* !LOCAL_HAVE_FAST */

	if (result_hidx != (size_t)-1) {
		/* If we found a previously deleted slot, re-use it. */
	} else {
		/* No deleted key found along hash-chain. Append at the end of the chain! */
#ifdef LOCAL_HAVE_FAST
		ASSERT(!_DeeDict_MustGrowHTab(self));
#else /* LOCAL_HAVE_FAST */
		if (_DeeDict_ShouldGrowHTab(self)) {
			if likely(dict_trygrow_htab(self))
				goto again_with_wrlock;
			if (_DeeDict_MustGrowHTab(self)) {
#ifdef LOCAL_HAVE_UNLOCKED
				DeeDict_LockWrite(self);
#endif /* LOCAL_HAVE_UNLOCKED */
				if unlikely(dict_grow_htab_and_relock(self))
					goto err;
#ifdef LOCAL_HAVE_UNLOCKED
				DeeDict_LockEndWrite(self);
#endif /* LOCAL_HAVE_UNLOCKED */
				goto again_with_wrlock;
			}
		}
#endif /* !LOCAL_HAVE_FAST */
		result_hidx = hs & self->d_hmask;
	}

	/* Actually append the new key/value pair to the hash-chain. */
	{
		size_t vtab_idx;
		struct Dee_dict_item *item;
#ifdef LOCAL_HAVE_CUSTOM_INDEX
		vtab_idx = (*getindex)(getindex_cookie, self, (size_t)-1);
		if unlikely(vtab_idx == (size_t)-1)
			goto err;
		dict_makespace_at(self, vtab_idx);
#else /* LOCAL_HAVE_CUSTOM_INDEX */
		vtab_idx = self->d_vsize;
#endif /* !LOCAL_HAVE_CUSTOM_INDEX */

		Dee_dict_vidx_real2virt(&vtab_idx);
		item = &_DeeDict_GetVirtVTab(self)[vtab_idx];
		(*self->d_hidxset)(self->d_htab, result_hidx, vtab_idx);
		item->di_hash = LOCAL_hash;
#ifdef LOCAL_HAVE_NON_OBJECT_KEY
		item->di_key = (DREF DeeObject *)keyob; /* Inherit reference */
#else /* LOCAL_HAVE_NON_OBJECT_KEY */
#ifndef LOCAL_HAVE_INHERITED
		Dee_Incref(key);
#endif /* !LOCAL_HAVE_INHERITED */
		item->di_key = key;
#endif /* !LOCAL_HAVE_NON_OBJECT_KEY */
#ifndef LOCAL_HAVE_INHERITED
#ifdef LOCAL_HAVE_SETDEFAULT
		Dee_Incref_n(value, 2); /* +1: item->di_value, +1: return */
#else /* LOCAL_HAVE_SETDEFAULT */
		Dee_Incref(value);
#endif /* !LOCAL_HAVE_SETDEFAULT */
#elif defined(LOCAL_HAVE_SETDEFAULT)
		Dee_Incref(value);      /* +1: return */
#endif /* ... */
		item->di_value = value;
		++self->d_vsize;
		++self->d_vused;
	}
	LOCAL_DeeDict_LockEndWrite(self);
#ifdef LOCAL_HAVE_SETDEFAULT
	return value;
#elif defined(LOCAL_HAVE_SETNEW)
	return ITER_DONE;
#else /* LOCAL_HAVE_SETNEW */
	return 0;
#endif /* !LOCAL_HAVE_SETNEW */

#ifndef LOCAL_HAVE_FAST
again_with_wrlock:
#ifdef LOCAL_HAVE_has_write_lock
	has_write_lock = true;
#endif /* LOCAL_HAVE_has_write_lock */
	goto again_locked;
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* !LOCAL_HAVE_FAST */

#ifdef NEED_decref_kv_downgrade_and_again_locked
#undef NEED_decref_kv_downgrade_and_again_locked
decref_kv_downgrade_and_again_locked:
	DeeDict_LockDowngrade(self);
#ifndef LOCAL_HAVE_NON_OBJECT_KEY
	Dee_DecrefNokill(key);
#endif /* !LOCAL_HAVE_NON_OBJECT_KEY */
	Dee_DecrefNokill(value);
#ifndef LOCAL_HAVE_SETOLD
	result_hidx = (size_t)-1;
#endif /* !LOCAL_HAVE_SETOLD */
	goto again_locked;
#endif /* NEED_decref_kv_downgrade_and_again_locked */

err:
#ifdef LOCAL_HAVE_NON_OBJECT_KEY
#ifndef LOCAL_HAVE_SETOLD
	if unlikely(keyob)
		Dee_DecrefDokill(keyob);
err_nokeyob:
#endif /* !LOCAL_HAVE_SETOLD */
#endif /* LOCAL_HAVE_NON_OBJECT_KEY */
#if defined(LOCAL_HAVE_INHERITED) && 0 /* Only inherited on success! */
	Dee_Decref(key);
	Dee_Decref(value);
#endif /* LOCAL_HAVE_INHERITED */
	return LOCAL_ERR;
#undef LOCAL_hash
#undef LOCAL_HAVE_has_write_lock
}


#undef LOCAL_HAVE_INHERITED
#undef LOCAL_HAVE_UNLOCKED_OR_NOTHREADS
#undef LOCAL_IF_NOT_UNLOCKED
#undef LOCAL_IF_NOT_UNLOCKED_AND_THREADS
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
#undef LOCAL_KEY_PARAMS
#undef LOCAL_NONNULL
#undef LOCAL_HAS_PARAM_HASH
#undef LOCAL_return_type
#undef LOCAL_ERR
#undef LOCAL_HAVE_KEY_IS_INDEX
#undef LOCAL_HAVE_KEY_IS_STRING
#undef LOCAL_HAVE_KEY_IS_STRING_HASH
#undef LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
#undef LOCAL_HAVE_NON_OBJECT_KEY
#undef LOCAL_HAVE_UNLOCKED
#undef LOCAL_HAVE_FAST
#undef LOCAL_HAVE_SETOLD
#undef LOCAL_HAVE_SETNEW
#undef LOCAL_HAVE_SETDEFAULT
#undef LOCAL_HAVE_CUSTOM_INDEX
#undef LOCAL_dict_setitem

DECL_END

#undef DEFINE_dict_setitem
#undef DEFINE_dict_setitem_at
#undef DEFINE_dict_setitem_string_hash
#undef DEFINE_dict_setitem_index
#undef DEFINE_dict_setitem_string_len_hash
#undef DEFINE_dict_setitem_unlocked
#undef DEFINE_dict_setitem_unlocked_fast_inherited
#undef DEFINE_dict_mh_setold_ex
#undef DEFINE_dict_mh_setnew_ex
#undef DEFINE_dict_mh_setdefault
