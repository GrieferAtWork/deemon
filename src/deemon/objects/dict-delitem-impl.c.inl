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
//#define DEFINE_dict_delitem
//#define DEFINE_dict_delitem_string_hash
//#define DEFINE_dict_delitem_string_len_hash
//#define DEFINE_dict_delitem_index
#define DEFINE_dict_popvalue
//#define DEFINE_dict_mh_remove
//#define DEFINE_dict_mh_pop
//#define DEFINE_dict_mh_pop_with_default
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_dict_delitem) +                 \
     defined(DEFINE_dict_delitem_string_hash) +     \
     defined(DEFINE_dict_delitem_string_len_hash) + \
     defined(DEFINE_dict_delitem_index) +           \
     defined(DEFINE_dict_popvalue) +                \
     defined(DEFINE_dict_mh_remove) +               \
     defined(DEFINE_dict_mh_pop) +                  \
     defined(DEFINE_dict_mh_pop_with_default)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_dict_delitem
#define LOCAL_dict_delitem dict_delitem
#elif defined(DEFINE_dict_delitem_string_hash)
#define LOCAL_dict_delitem dict_delitem_string_hash
#define LOCAL_HAVE_KEY_IS_STRING_HASH
#elif defined(DEFINE_dict_delitem_string_len_hash)
#define LOCAL_dict_delitem dict_delitem_string_len_hash
#define LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
#elif defined(DEFINE_dict_delitem_index)
#define LOCAL_dict_delitem dict_delitem_index
#define LOCAL_HAVE_KEY_IS_INDEX
#elif defined(DEFINE_dict_popvalue)
#define LOCAL_dict_delitem dict_popvalue
#elif defined(DEFINE_dict_mh_remove)
#define LOCAL_dict_delitem dict_mh_remove
#elif defined(DEFINE_dict_mh_pop)
#define LOCAL_dict_delitem dict_mh_pop
#elif defined(DEFINE_dict_mh_pop_with_default)
#define LOCAL_dict_delitem dict_mh_pop_with_default
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

#ifdef LOCAL_HAVE_KEY_IS_STRING_HASH
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAVE_KEY_IS_STRING
#elif defined(LOCAL_HAVE_KEY_IS_STRING_LEN_HASH)
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAVE_KEY_IS_STRING
#elif defined(LOCAL_HAVE_KEY_IS_INDEX)
#define LOCAL_NONNULL    NONNULL((1))
#define LOCAL_KEY_PARAMS size_t index
#elif defined(DEFINE_dict_mh_pop_with_default)
#define LOCAL_NONNULL    NONNULL((1, 2, 3))
#define LOCAL_KEY_PARAMS DeeObject *key, DeeObject *def
#else /* ... */
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS DeeObject *key
#endif /* !... */

#ifdef DEFINE_dict_popvalue
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_MISSING     ITER_DONE
#define LOCAL_ERR         NULL
#elif defined(DEFINE_dict_mh_pop)
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_ERR         NULL
#elif defined(DEFINE_dict_mh_pop_with_default)
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_MISSING     def
#define LOCAL_ERR         NULL
#elif defined(DEFINE_dict_mh_remove)
#define LOCAL_return_type int
#define LOCAL_PRESENT     1
#define LOCAL_MISSING     0
#define LOCAL_ERR         (-1)
#else /* ... */
#define LOCAL_return_type int
#define LOCAL_PRESENT     0
#define LOCAL_MISSING     0
#define LOCAL_ERR         (-1)
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
LOCAL_dict_delitem(Dict *self, LOCAL_KEY_PARAMS) {
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
	Dee_hash_t hs, perturb;

/*again:*/
	LOCAL_DeeDict_LockRead(self);
again_locked:
	_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
#ifndef LOCAL_HAVE_KEY_IS_STRING
		int cmp;
		DREF DeeObject *item_key;
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
		struct Dee_dict_item *item;
		size_t vtab_idx = _DeeDict_HTabGet(self, hs);
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
		if (item->di_key == NULL)
			continue; /* Deleted item */

#ifdef LOCAL_HAVE_KEY_IS_STRING
#ifdef LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
		if likely(DeeString_EqualsBuf(item->di_key, key, keylen))
#else /* LOCAL_HAVE_KEY_IS_STRING_LEN_HASH */
		if likely(strcmp(DeeString_STR(item->di_key), key) == 0)
#endif /* !LOCAL_HAVE_KEY_IS_STRING_LEN_HASH */
#else /* LOCAL_HAVE_KEY_IS_STRING */
		/* This might be it! */
		item_key = item->di_key;
		LOCAL_IF_NOT_UNLOCKED(Dee_Incref(item_key));
		LOCAL_DeeDict_LockEndRead(self);
#ifdef LOCAL_HAVE_KEY_IS_INDEX
		cmp = DeeInt_Size_TryCompareEq(index, item_key);
#else /* LOCAL_HAVE_KEY_IS_INDEX */
		cmp = DeeObject_TryCompareEq(key, item_key);
#endif /* !LOCAL_HAVE_KEY_IS_INDEX */
		LOCAL_IF_NOT_UNLOCKED(Dee_Decref_unlikely(item_key));
		if likely(cmp == 0)
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
		{
			/* Override existing item. */
#ifdef LOCAL_HAVE_KEY_IS_STRING
			DREF DeeObject *item_key = item->di_key;
#endif /* LOCAL_HAVE_KEY_IS_STRING */
			DREF DeeObject *old_value;
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
					goto downgrade_and_again_locked;
				if unlikely(item >= (_DeeDict_GetRealVTab(self) + self->d_vsize))
					goto downgrade_and_again_locked;
				if unlikely(item->di_key != item_key)
					goto downgrade_and_again_locked;
#endif /* LOCAL_HAVE_KEY_IS_STRING ? !LOCAL_HAVE_UNLOCKED_OR_NOTHREADS : !LOCAL_HAVE_UNLOCKED */
			}

			old_value = item->di_value; /* Inherit reference */
			item->di_key = NULL;        /* Inherit reference (mark as deleted) */
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
			--self->d_vused;            /* Account for deleted item */

			/* Check if the dict should shrink now. */
			dict_autoshrink(self);
			LOCAL_DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
#ifdef LOCAL_PRESENT
			Dee_Decref(old_value);
			return LOCAL_PRESENT;
#else /* LOCAL_PRESENT */
			return old_value;
#endif /* !LOCAL_PRESENT */
		}
#ifndef LOCAL_HAVE_KEY_IS_STRING
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
#define NEED_err
		LOCAL_DeeDict_LockRead(self);
#endif /* !LOCAL_HAVE_KEY_IS_STRING */
	}

	LOCAL_DeeDict_LockEndRead(self);
#ifdef LOCAL_MISSING
	return LOCAL_MISSING;
#elif defined(LOCAL_HAVE_KEY_IS_STRING_LEN_HASH)
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
#define NEED_err_fallthru
#elif defined(LOCAL_HAVE_KEY_IS_STRING_HASH)
	err_unknown_key_str((DeeObject *)self, key);
#define NEED_err_fallthru
#elif defined(LOCAL_HAVE_KEY_IS_INDEX)
	err_unknown_key_int((DeeObject *)self, index);
#define NEED_err_fallthru
#else /* ... */
	err_unknown_key((DeeObject *)self, key);
#define NEED_err_fallthru
#endif /* !... */

#ifdef NEED_err
#undef NEED_err
err:
#define NEED_err_fallthru
#endif /* NEED_err */
#ifdef NEED_err_fallthru
#undef NEED_err_fallthru
	return LOCAL_ERR;
#endif /* NEED_err_fallthru */

#if defined(LOCAL_HAVE_KEY_IS_STRING) ? !defined(LOCAL_HAVE_UNLOCKED_OR_NOTHREADS) : !defined(LOCAL_HAVE_UNLOCKED)
downgrade_and_again_locked:
	DeeDict_LockDowngrade(self);
	goto again_locked;
#endif /* LOCAL_HAVE_KEY_IS_STRING ? !LOCAL_HAVE_UNLOCKED_OR_NOTHREADS : !LOCAL_HAVE_UNLOCKED */
#undef LOCAL_hash
}


#undef LOCAL_return_type
#undef LOCAL_PRESENT
#undef LOCAL_MISSING
#undef LOCAL_ERR
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
#undef LOCAL_HAVE_KEY_IS_STRING
#undef LOCAL_HAVE_KEY_IS_STRING_HASH
#undef LOCAL_HAVE_KEY_IS_STRING_LEN_HASH
#undef LOCAL_HAVE_KEY_IS_INDEX
#undef LOCAL_HAVE_UNLOCKED
#undef LOCAL_dict_delitem

DECL_END

#undef DEFINE_dict_delitem
#undef DEFINE_dict_delitem_string_hash
#undef DEFINE_dict_delitem_string_len_hash
#undef DEFINE_dict_delitem_index
#undef DEFINE_dict_popvalue
#undef DEFINE_dict_mh_remove
#undef DEFINE_dict_mh_pop
#undef DEFINE_dict_mh_pop_with_default
