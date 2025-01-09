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
#define LOCAL_HAS_getindex
#elif defined(DEFINE_dict_setitem_string_hash)
#define LOCAL_dict_setitem dict_setitem_string_hash
#define LOCAL_HAS_KEY_IS_STRING_HASH
#elif defined(DEFINE_dict_setitem_index)
#define LOCAL_dict_setitem dict_setitem_index
#define LOCAL_HAS_KEY_IS_INDEX
#elif defined(DEFINE_dict_setitem_string_len_hash)
#define LOCAL_dict_setitem dict_setitem_string_len_hash
#define LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#elif defined(DEFINE_dict_setitem_unlocked)
#define LOCAL_dict_setitem dict_setitem_unlocked
#define LOCAL_IS_UNLOCKED
#elif defined(DEFINE_dict_setitem_unlocked_fast_inherited)
#define LOCAL_dict_setitem dict_setitem_unlocked_fast_inherited
#define LOCAL_IS_UNLOCKED
#define LOCAL_IS_FAST
#define LOCAL_IS_INHERITED
#elif defined(DEFINE_dict_mh_setold_ex)
#define LOCAL_dict_setitem dict_mh_setold_ex
#define LOCAL_IS_SETOLD
#elif defined(DEFINE_dict_mh_setnew_ex)
#define LOCAL_dict_setitem dict_mh_setnew_ex
#define LOCAL_IS_SETNEW
#elif defined(DEFINE_dict_mh_setdefault)
#define LOCAL_dict_setitem dict_mh_setdefault
#define LOCAL_IS_SETNEW
#define LOCAL_IS_SETDEFAULT
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

#if defined(LOCAL_IS_SETNEW) || defined(LOCAL_IS_SETOLD)
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_ERR         NULL
#else /* LOCAL_IS_SETNEW || LOCAL_IS_SETOLD */
#define LOCAL_return_type int
#define LOCAL_ERR         (-1)
#endif /* !LOCAL_IS_SETNEW && !LOCAL_IS_SETOLD */

#ifdef LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_NONNULL    NONNULL((1, 2, 4))
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAS_KEY_IS_STRING
#define LOCAL_HAS_keyob
#define LOCAL_new_keyob()                        DeeString_NewWithHash(key, hash)
//efine LOCAL_trynew_keyob()                     DeeString_TryNewWithHash(key, hash)
#define LOCAL_fastcmp(rhs)                       fastcmp_string(key, rhs)
//efine LOCAL_slowcmp(rhs)                       slowcmp_string(key, rhs)
#define LOCAL_matched_keyob_tryfrom(matched_key) matched_keyob_tryfrom_string(key, matched_key)
#elif defined(LOCAL_HAS_KEY_IS_STRING_LEN_HASH)
#define LOCAL_NONNULL    NONNULL((1, 2, 5))
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAS_KEY_IS_STRING
#define LOCAL_HAS_keyob
#define LOCAL_new_keyob()                        DeeString_NewSizedWithHash(key, keylen, hash)
//efine LOCAL_trynew_keyob()                     DeeString_TryNewSizedWithHash(key, keylen, hash)
#define LOCAL_fastcmp(rhs)                       fastcmp_string_len(key, keylen, rhs)
//efine LOCAL_slowcmp(rhs)                       slowcmp_string_len(key, keylen, rhs)
#define LOCAL_matched_keyob_tryfrom(matched_key) matched_keyob_tryfrom_string_len(key, keylen, matched_key)
#elif defined(LOCAL_HAS_KEY_IS_INDEX)
#define LOCAL_NONNULL    NONNULL((1, 3))
#define LOCAL_KEY_PARAMS size_t index
#define LOCAL_HAS_keyob
#define LOCAL_new_keyob()                        DeeInt_NewSize(index)
//efine LOCAL_trynew_keyob()                     DeeInt_TryNewSize(index)
#define LOCAL_fastcmp(rhs)                       fastcmp_index(index, rhs)
#define LOCAL_slowcmp(rhs)                       slowcmp_index(index, rhs)
#define LOCAL_matched_keyob_tryfrom(matched_key) matched_keyob_tryfrom_index(index, matched_key)
#else /* ... */
#define LOCAL_NONNULL    NONNULL((1, 2, 3))
#ifdef LOCAL_IS_INHERITED
#define LOCAL_KEY_PARAMS /*inherit(on_success)*/ DREF DeeObject *key
#else /* LOCAL_IS_INHERITED */
#define LOCAL_KEY_PARAMS DeeObject *key
#endif /* !LOCAL_IS_INHERITED */
#endif /* !... */

#ifndef LOCAL_IS_UNLOCKED
#define LOCAL_IF_NOT_UNLOCKED(...) __VA_ARGS__
#else /* !LOCAL_IS_UNLOCKED */
#define LOCAL_IF_NOT_UNLOCKED(...) /* nothing */
#endif /* LOCAL_IS_UNLOCKED */

#define LOCAL_DeeDict_LockTryRead(self)    LOCAL_IF_NOT_UNLOCKED(DeeDict_LockTryRead(self))
#define LOCAL_DeeDict_LockTryWrite(self)   LOCAL_IF_NOT_UNLOCKED(DeeDict_LockTryWrite(self))
#define LOCAL_DeeDict_LockRead(self)       LOCAL_IF_NOT_UNLOCKED(DeeDict_LockRead(self))
#define LOCAL_DeeDict_LockWrite(self)      LOCAL_IF_NOT_UNLOCKED(DeeDict_LockWrite(self))
#define LOCAL_DeeDict_LockTryUpgrade(self) LOCAL_IF_NOT_UNLOCKED(DeeDict_LockTryUpgrade(self))
#define LOCAL_DeeDict_LockUpgrade(self)    LOCAL_IF_NOT_UNLOCKED(DeeDict_LockUpgrade(self))
#define LOCAL_DeeDict_LockDowngrade(self)  LOCAL_IF_NOT_UNLOCKED(DeeDict_LockDowngrade(self))
#define LOCAL_DeeDict_LockEndWrite(self)   LOCAL_IF_NOT_UNLOCKED(DeeDict_LockEndWrite(self))
#define LOCAL_DeeDict_LockEndRead(self)    LOCAL_IF_NOT_UNLOCKED(DeeDict_LockEndRead(self))
#define LOCAL_DeeDict_LockEnd(self)        LOCAL_IF_NOT_UNLOCKED(DeeDict_LockEnd(self))


PRIVATE WUNUSED LOCAL_NONNULL LOCAL_return_type DCALL
LOCAL_dict_setitem(Dict *self, LOCAL_KEY_PARAMS
#ifdef LOCAL_IS_INHERITED
                   , /*inherit(on_success)*/ DREF DeeObject *value
#else /* LOCAL_IS_INHERITED */
                   , DeeObject *value
#endif /* !LOCAL_IS_INHERITED */
#ifdef LOCAL_HAS_getindex
                   , /*virt*/Dee_dict_vidx_t (DCALL *getindex)(void *cookie, Dict *self,
                                                               /*virt*/Dee_dict_vidx_t overwrite_index,
                                                               DREF DeeObject **p_value)
                   , void *getindex_cookie
#endif /* LOCAL_HAS_getindex */
                   ) {
#ifndef LOCAL_HAS_PARAM_HASH
#ifdef LOCAL_HAS_KEY_IS_INDEX
#define LOCAL_hash index
#else /* LOCAL_HAS_KEY_IS_INDEX */
	Dee_hash_t hash = DeeObject_Hash(key);
#define LOCAL_hash hash
#endif /* !LOCAL_HAS_KEY_IS_INDEX */
#else /* !LOCAL_HAS_PARAM_HASH */
#define LOCAL_hash hash
#endif /* LOCAL_HAS_PARAM_HASH */

#ifdef LOCAL_HAS_keyob
	DREF DeeObject *keyob = NULL;
#endif /* LOCAL_HAS_keyob */


	/* Helper macros for doing cleanup on no-op return paths. */
#if defined(LOCAL_HAS_keyob) && defined(LOCAL_IS_INHERITED)
#define LOCAL_cleanup_data_for_noop_return() (unlikely(keyob) ? Dee_Decref(keyob) : (void)0, Dee_Decref_unlikely(value))
#elif defined(LOCAL_HAS_keyob)
#define LOCAL_cleanup_data_for_noop_return() (unlikely(keyob) ? Dee_Decref(keyob) : (void)0)
#elif defined(LOCAL_IS_INHERITED)
#define LOCAL_cleanup_data_for_noop_return() (Dee_Decref_unlikely(key), Dee_Decref_unlikely(value))
#else /* ... */
#define LOCAL_cleanup_data_for_noop_return() (void)0
#endif /* !... */


	/* Helper macros for populating a given "item" */
#ifdef LOCAL_IS_INHERITED
#define _LOCAL_incref_key_for_populate_item_key_and_value()   (void)0
#define _LOCAL_incref_value_for_populate_item_key_and_value() (void)0
#else /* LOCAL_IS_INHERITED */
#define _LOCAL_incref_key_for_populate_item_key_and_value()   Dee_Incref(key)
#define _LOCAL_incref_value_for_populate_item_key_and_value() Dee_Incref(value)
#endif /* !LOCAL_IS_INHERITED */
#ifdef LOCAL_HAS_keyob
#define LOCAL_populate_item_key_and_value(item)                   \
	(void)((item)->di_key = keyob, /* Inherit reference */        \
	       _LOCAL_incref_value_for_populate_item_key_and_value(), \
	       (item)->di_value = value /* Inherit reference */)
#else /* LOCAL_HAS_keyob */
#define LOCAL_populate_item_key_and_value(item)                   \
	(void)(_LOCAL_incref_key_for_populate_item_key_and_value(),   \
	       (item)->di_key = key, /* Inherit reference */          \
	       _LOCAL_incref_value_for_populate_item_key_and_value(), \
	       (item)->di_value = value /* Inherit reference */)
#endif /* !LOCAL_HAS_keyob */


	Dee_hash_t hs, perturb;
	size_t result_htab_idx; /* index in "d_htab" of first deleted item ever encountered. */

#ifndef LOCAL_IS_SETOLD
	LOCAL_IF_NOT_UNLOCKED(bool use_write_lock = false);
#else /* !LOCAL_IS_SETOLD */
#undef LOCAL_DeeDict_LockEnd
#define LOCAL_DeeDict_LockEnd(self) LOCAL_IF_NOT_UNLOCKED(DeeDict_LockEndRead(self))
#endif /* LOCAL_IS_SETOLD */

	LOCAL_DeeDict_LockRead(self);
LOCAL_IF_NOT_UNLOCKED(again_with_lock:)
	result_htab_idx = (size_t)-1;
	_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		DREF DeeObject *item_key;
#ifdef LOCAL_IS_SETNEW
		DREF DeeObject *item_value;
#endif /* LOCAL_IS_SETNEW */
		int item_key_cmp_caller_key;
		struct Dee_dict_item *item;
		size_t htab_idx;          /* hash-index in "d_htab" */
		Dee_dict_vidx_t vtab_idx; /* hash-index in "d_vtab" */

		/* Load hash indices */
		htab_idx = hs & self->d_hmask;
		vtab_idx = (*self->d_hidxget)(self->d_htab, htab_idx);

		/* Check for end-of-hash-chain */
		if (vtab_idx == Dee_DICT_HTAB_EOF) {
#ifdef LOCAL_IS_SETOLD
			LOCAL_DeeDict_LockEnd(self);
			LOCAL_cleanup_data_for_noop_return();
			return ITER_DONE;
#else /* LOCAL_IS_SETOLD */
			if (result_htab_idx == (size_t)-1)
				result_htab_idx = htab_idx;
			break;
#endif /* !LOCAL_IS_SETOLD */
		}

		/* Load referenced item in "d_vtab" */
		ASSERT(Dee_dict_vidx_virt_lt_real(vtab_idx, self->d_vsize));
		item = &_DeeDict_GetVirtVTab(self)[vtab_idx];

		/* Check for deleted items... */
		item_key = item->di_key;
		if (item_key == NULL) {
			result_htab_idx = htab_idx;
			continue;
		}

		/* Check if hash really matches. */
		if (item->di_hash != LOCAL_hash)
			continue; /* Different hash */

		/* Special optimizations when the caller-given "key" is a special value. */
#ifdef LOCAL_fastcmp
		{
			int fastcmp_result = LOCAL_fastcmp(item->di_key);
			if likely(fastcmp_result == 0) {
				/* Found the item! */
#ifdef LOCAL_IS_SETNEW
				item_value = item->di_value;
				Dee_Incref(item_value);
				LOCAL_DeeDict_LockEnd(self);
				LOCAL_cleanup_data_for_noop_return();
				return item_value;
#else /* LOCAL_IS_SETNEW */
#ifdef LOCAL_matched_keyob_tryfrom
				if (keyob == NULL) {
					keyob = LOCAL_matched_keyob_tryfrom(item_key);
					if unlikely(!keyob) {
						Dee_Incref(item_key);
						LOCAL_DeeDict_LockEnd(self);
						goto do_allocate_keyob_for_override;
#define NEED_do_allocate_keyob_for_override
					}
				}
#endif /* LOCAL_matched_keyob_tryfrom */
#ifndef LOCAL_IS_UNLOCKED
#ifndef LOCAL_IS_SETOLD
				if (!use_write_lock)
#endif /* !LOCAL_IS_SETOLD */
				{
					if (LOCAL_DeeDict_LockTryUpgrade(self))
						goto override_item_after_consistency_check;
#define NEED_override_item_after_consistency_check
					LOCAL_DeeDict_LockEndRead(self);
					LOCAL_DeeDict_LockWrite(self);
				}
#endif /* !LOCAL_IS_UNLOCKED */
				goto override_item_before_consistency_check;
#define NEED_override_item_before_consistency_check
#endif /* !LOCAL_IS_SETNEW */
			}
			if (fastcmp_result > 0)
				continue; /* Wrong key (hash just matched by random) */
		}
#endif /* LOCAL_fastcmp */

		/* Load item key and release dict lock (so we can do compare) */
		Dee_Incref(item_key);
#ifdef LOCAL_IS_SETNEW
		item_value = item->di_value;
		Dee_Incref(item_value);
#endif /* LOCAL_IS_SETNEW */
		LOCAL_DeeDict_LockEnd(self);

		/* Special case used to verify that the dict didn't change. */
#ifdef LOCAL_IS_UNLOCKED
#define LOCAL_verify_unchanged_after_unlock(goto_if_changed) (void)0
#else /* LOCAL_IS_UNLOCKED */
#define LOCAL_verify_unchanged_after_unlock(goto_if_changed)                            \
	do {                                                                                \
		if unlikely(htab_idx != (hs & self->d_hmask))                                   \
			goto goto_if_changed;                                                       \
		if unlikely(vtab_idx != (*self->d_hidxget)(self->d_htab, htab_idx))             \
			goto goto_if_changed;                                                       \
		if unlikely(item != &_DeeDict_GetVirtVTab(self)[vtab_idx])                      \
			goto goto_if_changed;                                                       \
		if unlikely(item->di_key != item_key)                                           \
			goto goto_if_changed;                                                       \
		if unlikely(item->di_hash != LOCAL_hash)                                        \
			goto goto_if_changed;                                                       \
		if (result_htab_idx != (size_t)-1) {                                            \
			/*virt*/ Dee_dict_vidx_t first_deleted_vtab_idx;                            \
			first_deleted_vtab_idx = (*self->d_hidxget)(self->d_htab, result_htab_idx); \
			if unlikely(first_deleted_vtab_idx == Dee_DICT_HTAB_EOF ||                  \
			            _DeeDict_GetVirtVTab(self)[first_deleted_vtab_idx].di_key)      \
				result_htab_idx = (size_t)-1;                                           \
		}                                                                               \
	}	__WHILE0
#endif /* !LOCAL_IS_UNLOCKED */


		/* Regular caller-given key vs. dict key compare. */
#ifdef LOCAL_slowcmp
		item_key_cmp_caller_key = LOCAL_slowcmp(item_key);
#elif defined(LOCAL_HAS_keyob)
		if unlikely(!keyob) {
			keyob = LOCAL_new_keyob();
			if unlikely(!keyob) {
				Dee_Decref_unlikely(item_key);
#ifdef LOCAL_IS_SETNEW
				Dee_Decref_unlikely(item_value);
#endif /* LOCAL_IS_SETNEW */
				goto err_nokeyob;
			}
		}
		item_key_cmp_caller_key = DeeObject_TryCompareEq(keyob, item_key);
#else /* ... */
		item_key_cmp_caller_key = DeeObject_TryCompareEq(key, item_key);
#endif /* !... */

		/* Case: keys are equal, meaning we must override this item! */
		if likely(item_key_cmp_caller_key == 0) {
#ifdef LOCAL_IS_SETNEW
			Dee_Decref_unlikely(item_key);
			LOCAL_cleanup_data_for_noop_return();
			return item_value;
#else /* LOCAL_IS_SETNEW */
			DREF DeeObject *old_value;
			/*virt*/Dee_dict_vidx_t result_vidx;

			/* Allocate a key object if necessary */
#ifdef LOCAL_HAS_keyob
			if (keyob == NULL) {
#ifdef NEED_do_allocate_keyob_for_override
#undef NEED_do_allocate_keyob_for_override
do_allocate_keyob_for_override:
#endif /* NEED_do_allocate_keyob_for_override */
				keyob = LOCAL_new_keyob();
				if unlikely(!keyob) {
					Dee_Decref_unlikely(item_key);
					goto err_nokeyob;
				}
			}
#endif /* LOCAL_HAS_keyob */

			/* Drop the reference to the existing key, as used for the compare,  */
			Dee_Decref_unlikely(item_key);
			LOCAL_DeeDict_LockWrite(self);
#ifdef NEED_override_item_before_consistency_check
#undef NEED_override_item_before_consistency_check
override_item_before_consistency_check:
#endif /* NEED_override_item_before_consistency_check */
#ifndef LOCAL_IS_UNLOCKED
			LOCAL_verify_unchanged_after_unlock(downgrade_lock_and_try_again);
#define NEED_downgrade_lock_and_try_again
#endif /* !LOCAL_IS_UNLOCKED */
#ifdef NEED_override_item_after_consistency_check
#undef NEED_override_item_after_consistency_check
override_item_after_consistency_check:
#endif /* NEED_override_item_after_consistency_check */

			/************************************************************************/
			/* OVERRIDE EXISTING "item"                                             */
			/************************************************************************/

#ifdef LOCAL_HAS_getindex
			result_vidx = (*getindex)(getindex_cookie, self, vtab_idx, &value);
			if unlikely(result_vidx == Dee_DICT_HTAB_EOF)
				goto err;
#endif /* LOCAL_HAS_getindex */

			/* Steal the reference held by the item's old value. */
			ASSERT(item_key == item->di_key); /* Inherit reference */
			old_value = item->di_value;       /* Inherit reference */
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));

#ifdef LOCAL_HAS_getindex
			if (vtab_idx == result_vidx)
#else /* LOCAL_HAS_getindex */
			if (Dee_dict_vidx_toreal(vtab_idx) == self->d_vsize - 1)
#endif /* !LOCAL_HAS_getindex */
			{
				/* Simple case: the item being overwritten already *is* at the last position,
				 *              so there is absolutely no reason to shuffle anything around! */
#ifndef LOCAL_HAS_getindex
				result_vidx = vtab_idx;
#endif /* !LOCAL_HAS_getindex */
				ASSERT(item->di_hash == LOCAL_hash);
				LOCAL_populate_item_key_and_value(item);
			} else {
#ifndef LOCAL_HAS_getindex
				/* Append at the end */
				result_vidx = Dee_dict_vidx_tovirt(self->d_vsize);
				ASSERT(result_vidx != Dee_DICT_HTAB_EOF);
#endif /* !LOCAL_HAS_getindex */

#ifdef LOCAL_IS_FAST
				ASSERT(self->d_vsize < self->d_valloc);
#else /* LOCAL_IS_FAST */
				if (self->d_vsize < self->d_valloc)
#endif /* !LOCAL_IS_FAST */
				{
					/* Just append a new item at the end, and delete "item" */
					struct Dee_dict_item *new_item;
					item->di_key = NULL; /* Deleted! */
#ifdef LOCAL_HAS_getindex
					dict_makespace_at(self, Dee_dict_vidx_toreal(result_vidx));
#endif /* LOCAL_HAS_getindex */
					new_item = &_DeeDict_GetVirtVTab(self)[result_vidx];
					++self->d_vsize;
					new_item->di_hash = LOCAL_hash;
					LOCAL_populate_item_key_and_value(new_item);
					(*self->d_hidxset)(self->d_htab, htab_idx, result_vidx);
				}
#ifndef LOCAL_IS_FAST
				else {
					struct Dee_dict_item *new_item;
					ASSERT(self->d_vsize == self->d_valloc);
					/* Screw around with the vtab/htab to free up a slot for "new_item" */
#ifdef LOCAL_HAS_getindex
					if (result_vidx < vtab_idx) {
						new_item = &_DeeDict_GetVirtVTab(self)[result_vidx];
						memmoveupc(new_item + 1, new_item, /* HINT: This also deletes "item" */
						           (/*Dee_dict_vidx_toreal*/(vtab_idx) - 1) - /*Dee_dict_vidx_toreal*/(result_vidx),
						           sizeof(struct Dee_dict_item));
						/* What would also work: "dict_htab_incrange(self, result_vidx, vtab_idx + 1);" */
						dict_htab_incrange(self, result_vidx, vtab_idx);
					} else
#endif /* LOCAL_HAS_getindex */
					{
						ASSERTF(result_vidx > vtab_idx, "case 'result_vidx == vtab_idx' was already handled above");
						memmovedownc(item, item + 1, /* HINT: This also deletes "item" */
						             (/*Dee_dict_vidx_toreal*/(result_vidx) - 1) - /*Dee_dict_vidx_toreal*/(vtab_idx),
						             sizeof(struct Dee_dict_item));
#ifdef LOCAL_HAS_getindex
						/* What would also work: "dict_htab_decrange(self, vtab_idx + 1, result_vidx);" */
						dict_htab_decrange(self, vtab_idx, result_vidx);
#else /* LOCAL_HAS_getindex */
						dict_htab_decafter(self, vtab_idx); /* What would also work: "dict_htab_decafter(self, vtab_idx + 1);" */
#endif /* !LOCAL_HAS_getindex */
						--result_vidx;
						new_item = &_DeeDict_GetVirtVTab(self)[result_vidx];
					}
					new_item->di_hash = LOCAL_hash;
					LOCAL_populate_item_key_and_value(new_item);
					(*self->d_hidxset)(self->d_htab, htab_idx, result_vidx);

					/* Optimization: if we discovered another deleted key before "item",
					 *               then swap htab indices such that our key is found first. */
					if (result_htab_idx != (size_t)-1) {
						/*virt*/Dee_dict_vidx_t known_deleted_vidx;
						known_deleted_vidx = (*self->d_hidxget)(self->d_htab, result_htab_idx);
						ASSERT(known_deleted_vidx != Dee_DICT_HTAB_EOF);
						ASSERT(Dee_dict_vidx_virt_lt_real(known_deleted_vidx, self->d_vsize));
						ASSERT(_DeeDict_GetVirtVTab(self)[known_deleted_vidx].di_key == NULL);
						(*self->d_hidxset)(self->d_htab, htab_idx, known_deleted_vidx);
						(*self->d_hidxset)(self->d_htab, result_htab_idx, result_vidx);
					}

					if (_DeeDict_ShouldOptimizeVTab(self))
						goto done_overwrite_optimize_vtab;

					/* Dict says it shouldn't be optimized yet, but we have to resort
					 * to screwing around with table pointers in order to overwrite an
					 * item -> try to grow the dict (but it's OK if this fails) */
					if unlikely(!dict_trygrow_vtab_and_htab(self)) {
						if (_DeeDict_CanOptimizeVTab(self))
							goto done_overwrite_optimize_vtab;
					}
					goto done_overwrite_unlock_dict;
#define NEED_done_overwrite_unlock_dict
				}
#endif /* !LOCAL_IS_FAST */
			}


			/* Optimization: if we discovered another deleted key before "item",
			 *               then swap htab indices such that our key is found first. */
			if (result_htab_idx != (size_t)-1) {
				/*virt*/Dee_dict_vidx_t known_deleted_vidx;
				known_deleted_vidx = (*self->d_hidxget)(self->d_htab, result_htab_idx);
				ASSERT(known_deleted_vidx != Dee_DICT_HTAB_EOF);
				ASSERT(Dee_dict_vidx_virt_lt_real(known_deleted_vidx, self->d_vsize));
				ASSERT(_DeeDict_GetVirtVTab(self)[known_deleted_vidx].di_key == NULL);
				(*self->d_hidxset)(self->d_htab, htab_idx, known_deleted_vidx);
				(*self->d_hidxset)(self->d_htab, result_htab_idx, result_vidx);
			}

			/* If appropriate, optimize the dict's vtab. */
#ifndef LOCAL_IS_FAST
			if (_DeeDict_ShouldOptimizeVTab(self)) {
done_overwrite_optimize_vtab:
				dict_optimize_vtab(self);
			}
#endif /* !LOCAL_IS_FAST */

			/* Release the dict write-lock now that we're done. */
#ifdef NEED_done_overwrite_unlock_dict
#undef NEED_done_overwrite_unlock_dict
done_overwrite_unlock_dict:
#endif /* NEED_done_overwrite_unlock_dict */
			LOCAL_DeeDict_LockEndWrite(self);

			/* Drop the reference to the old key that we stole. */
			Dee_Decref(item_key);

			/* Indicate that we successfully overwrote an existing item. */
#ifdef LOCAL_IS_SETOLD
			return old_value;
#else /* LOCAL_IS_SETOLD */
			Dee_Decref(old_value);
			return 0;
#endif /* !LOCAL_IS_SETOLD */
#endif /* !LOCAL_IS_SETNEW */
		}

#ifdef LOCAL_IS_SETNEW
		Dee_Decref_unlikely(item_value);
#endif /* LOCAL_IS_SETNEW */

		/* Check for error during compare. */
		Dee_Decref_unlikely(item_key);
		if unlikely(item_key_cmp_caller_key == Dee_COMPARE_ERR)
			goto err;
#ifndef LOCAL_IS_SETOLD
		LOCAL_IF_NOT_UNLOCKED(if (use_write_lock) {
			LOCAL_DeeDict_LockWrite(self);
		} else)
#endif /* LOCAL_IS_SETOLD */
		{
			LOCAL_DeeDict_LockRead(self);
		}
		LOCAL_verify_unchanged_after_unlock(again_with_lock);
#undef LOCAL_verify_unchanged_after_unlock
	}
#ifdef LOCAL_IS_SETOLD
	__builtin_unreachable();
#else /* LOCAL_IS_SETOLD */

	/************************************************************************/
	/* KEY DOES NOT EXIST                                                   */
	/************************************************************************/

	/* key doesn't exit -> must append a new one!
	 *
	 * However, we must first:
	 * - ensure that we've allocated "keyob" (if necessary)
	 * - upgrade our read-lock into a write-lock */

	/* Allocate a key object if necessary */
#ifdef LOCAL_new_keyob
	if (keyob == NULL) {
#ifdef LOCAL_trynew_keyob
		keyob = LOCAL_trynew_keyob();
		if unlikely(!keyob)
#endif /* LOCAL_trynew_keyob */
		{
			LOCAL_DeeDict_LockEnd(self);
			keyob = LOCAL_new_keyob();
			if unlikely(!keyob)
				goto err_nokeyob;
			LOCAL_DeeDict_LockWrite(self);
			LOCAL_IF_NOT_UNLOCKED(use_write_lock = true);
			LOCAL_IF_NOT_UNLOCKED(goto again_with_lock);
		}
	}
#endif /* LOCAL_new_keyob */

	/* Upgrade read- into write-lock */
#ifndef LOCAL_IS_UNLOCKED
	if (!use_write_lock && !LOCAL_DeeDict_LockTryUpgrade(self)) {
		LOCAL_DeeDict_LockEndRead(self);
		LOCAL_DeeDict_LockWrite(self);
		use_write_lock = true;
		goto again_with_lock;
	}
#endif /* !LOCAL_IS_UNLOCKED */

	ASSERTF(result_htab_idx != (size_t)-1, "Should have been set by for-block above");

	/* Check if the size of the vtab needs to increase. */
	if unlikely(_DeeDict_MustGrowVTab(self)) {
		size_t old_hmask;
		if (_DeeDict_ShouldGrowHTab(self)) {
do_dict_trygrow_vtab_and_htab:
			old_hmask = self->d_hmask;
			if (!dict_trygrow_vtab_and_htab(self)) {
force_grow_without_locks:
#ifdef LOCAL_IS_UNLOCKED
				if unlikely(dict_grow_vtab_and_htab_and_relock(self, true))
#else /* LOCAL_IS_UNLOCKED */
				if unlikely(dict_grow_vtab_and_htab_and_relock(self, false))
#endif /* !LOCAL_IS_UNLOCKED */
				{
					goto err;
				}
				LOCAL_IF_NOT_UNLOCKED(goto again_with_lock);
			}
			ASSERT(old_hmask <= self->d_hmask);
			if (old_hmask != self->d_hmask) {
				/* Must re-discover "result_htab_idx" in d_htab (it'll be at the end of the hash-chain) */
				_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
				for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
					size_t htab_idx = hs & self->d_hmask;
					Dee_dict_vidx_t vtab_idx = (*self->d_hidxget)(self->d_htab, htab_idx);
					if (vtab_idx == Dee_DICT_HTAB_EOF) {
						result_htab_idx = htab_idx;
						break;
					}
				}
			}
		} else if (_DeeDict_CanGrowVTab(self)) {
			if (!dict_trygrow_vtab(self)) {
#ifdef LOCAL_IS_UNLOCKED
				old_hmask = self->d_hmask;
#endif /* LOCAL_IS_UNLOCKED */
				goto force_grow_without_locks;
			}
		} else {
			goto do_dict_trygrow_vtab_and_htab;
		}
	}

	/************************************************************************/
	/* APPEND NEW ITEM                                                      */
	/************************************************************************/
	ASSERT(!_DeeDict_MustGrowVTab(self));
	{
		/*virt*/Dee_dict_vidx_t result_vidx;
		struct Dee_dict_item *result_item;

		/* Figure out where the result item should be insert */
#ifdef LOCAL_HAS_getindex
		result_vidx = (*getindex)(getindex_cookie, self, Dee_DICT_HTAB_EOF, &value);
		if unlikely(result_vidx == Dee_DICT_HTAB_EOF)
			goto err;
		dict_makespace_at(self, Dee_dict_vidx_toreal(result_vidx));
#else /* LOCAL_HAS_getindex */
		result_vidx = Dee_dict_vidx_tovirt(self->d_vsize);
		ASSERT(result_vidx != Dee_DICT_HTAB_EOF);
#endif /* !LOCAL_HAS_getindex */

		/* Link in the result item. */
		result_item = &_DeeDict_GetVirtVTab(self)[result_vidx];
		(*self->d_hidxset)(self->d_htab, result_htab_idx, result_vidx);
		++self->d_vsize;
		++self->d_vused;

		/* Initialize the result item. */
		result_item->di_hash = LOCAL_hash;

		/* Fill in the result item key. */
#ifdef LOCAL_HAS_keyob
		result_item->di_key = keyob; /* Inherit reference */
#else /* LOCAL_HAS_keyob */
#ifndef LOCAL_IS_INHERITED
		Dee_Incref(key);
#endif /* !LOCAL_IS_INHERITED */
		result_item->di_key = key;
#endif /* !LOCAL_HAS_keyob */

		/* Fill in the result item value (and also allocate the return reference for "SETDEFAULT"). */
#if defined(LOCAL_IS_SETDEFAULT) && defined(LOCAL_IS_INHERITED)
		Dee_Incref(value);
#elif defined(LOCAL_IS_SETDEFAULT)
		Dee_Incref_n(value, 2);
#elif !defined(LOCAL_IS_INHERITED)
		Dee_Incref(value);
#endif /* ... */
		result_item->di_value = value;
	}
	LOCAL_DeeDict_LockEndWrite(self);
#ifdef LOCAL_IS_SETDEFAULT
	return value;
#elif defined(LOCAL_IS_SETNEW)
	return ITER_DONE;
#else /* LOCAL_IS_SETNEW */
	return 0;
#endif /* !LOCAL_IS_SETNEW */
#endif /* !LOCAL_IS_SETOLD */
err:
#ifdef LOCAL_HAS_keyob
	if unlikely(keyob)
		Dee_Decref(keyob);
err_nokeyob:
#endif /* LOCAL_HAS_keyob */
#if defined(LOCAL_IS_INHERITED) && 0 /* Only inherited on success! */
	Dee_Decref(key);
	Dee_Decref(value);
#endif /* LOCAL_IS_INHERITED */
	return LOCAL_ERR;

#ifdef NEED_downgrade_lock_and_try_again
#undef NEED_downgrade_lock_and_try_again
downgrade_lock_and_try_again:
#ifndef LOCAL_IS_SETOLD
	if (!use_write_lock)
#endif /* !LOCAL_IS_SETOLD */
	{
		LOCAL_DeeDict_LockDowngrade(self);
	}
	goto again_with_lock;
#endif /* NEED_downgrade_lock_and_try_again */

#undef LOCAL_hash
#undef _LOCAL_incref_key_for_populate_item_key_and_value
#undef _LOCAL_incref_value_for_populate_item_key_and_value
#undef LOCAL_cleanup_data_for_noop_return
#undef LOCAL_populate_item_key_and_value
}


#undef LOCAL_new_keyob
#undef LOCAL_trynew_keyob
#undef LOCAL_fastcmp
#undef LOCAL_slowcmp
#undef LOCAL_matched_keyob_tryfrom

#undef LOCAL_IS_INHERITED
#undef LOCAL_IS_UNLOCKED
#undef LOCAL_IF_NOT_UNLOCKED
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
#undef LOCAL_HAS_KEY_IS_INDEX
#undef LOCAL_HAS_KEY_IS_STRING
#undef LOCAL_HAS_KEY_IS_STRING_HASH
#undef LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#undef LOCAL_HAS_keyob
#undef LOCAL_IS_FAST
#undef LOCAL_IS_SETOLD
#undef LOCAL_IS_SETNEW
#undef LOCAL_IS_SETDEFAULT
#undef LOCAL_HAS_getindex
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
