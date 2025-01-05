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
//#define DEFINE_dict_trygetitem
//#define DEFINE_dict_trygetitem_string_hash
//#define DEFINE_dict_trygetitem_string_len_hash
#define DEFINE_dict_trygetitem_index
//#define DEFINE_dict_getitem
//#define DEFINE_dict_getitem_string_hash
//#define DEFINE_dict_getitem_string_len_hash
//#define DEFINE_dict_getitem_index
//#define DEFINE_dict_hasitem
//#define DEFINE_dict_hasitem_string_hash
//#define DEFINE_dict_hasitem_string_len_hash
//#define DEFINE_dict_hasitem_index
//#define DEFINE_dict_bounditem
//#define DEFINE_dict_bounditem_string_hash
//#define DEFINE_dict_bounditem_string_len_hash
//#define DEFINE_dict_bounditem_index
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_dict_trygetitem) +                 \
     defined(DEFINE_dict_trygetitem_string_hash) +     \
     defined(DEFINE_dict_trygetitem_string_len_hash) + \
     defined(DEFINE_dict_trygetitem_index) +           \
     defined(DEFINE_dict_getitem) +                    \
     defined(DEFINE_dict_getitem_string_hash) +        \
     defined(DEFINE_dict_getitem_string_len_hash) +    \
     defined(DEFINE_dict_getitem_index) +              \
     defined(DEFINE_dict_hasitem) +                    \
     defined(DEFINE_dict_hasitem_string_hash) +        \
     defined(DEFINE_dict_hasitem_string_len_hash) +    \
     defined(DEFINE_dict_hasitem_index) +              \
     defined(DEFINE_dict_bounditem) +                  \
     defined(DEFINE_dict_bounditem_string_hash) +      \
     defined(DEFINE_dict_bounditem_string_len_hash) +  \
     defined(DEFINE_dict_bounditem_index)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_dict_getitem
#define LOCAL_dict_getitem dict_getitem
#elif defined(DEFINE_dict_getitem_string_hash)
#define LOCAL_dict_getitem dict_getitem_string_hash
#define LOCAL_HAS_KEY_IS_STRING_HASH
#elif defined(DEFINE_dict_getitem_string_len_hash)
#define LOCAL_dict_getitem dict_getitem_string_len_hash
#define LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#elif defined(DEFINE_dict_getitem_index)
#define LOCAL_dict_getitem dict_getitem_index
#define LOCAL_HAS_KEY_IS_INDEX
#elif defined(DEFINE_dict_trygetitem)
#define LOCAL_dict_getitem dict_trygetitem
#define LOCAL_IS_TRY
#elif defined(DEFINE_dict_trygetitem_string_hash)
#define LOCAL_dict_getitem dict_trygetitem_string_hash
#define LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_IS_TRY
#elif defined(DEFINE_dict_trygetitem_string_len_hash)
#define LOCAL_dict_getitem dict_trygetitem_string_len_hash
#define LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#define LOCAL_IS_TRY
#elif defined(DEFINE_dict_trygetitem_index)
#define LOCAL_dict_getitem dict_trygetitem_index
#define LOCAL_HAS_KEY_IS_INDEX
#define LOCAL_IS_TRY
#elif defined(DEFINE_dict_hasitem)
#define LOCAL_dict_getitem dict_hasitem
#define LOCAL_IS_HAS
#elif defined(DEFINE_dict_hasitem_string_hash)
#define LOCAL_dict_getitem dict_hasitem_string_hash
#define LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_IS_HAS
#elif defined(DEFINE_dict_hasitem_string_len_hash)
#define LOCAL_dict_getitem dict_hasitem_string_len_hash
#define LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#define LOCAL_IS_HAS
#elif defined(DEFINE_dict_hasitem_index)
#define LOCAL_dict_getitem dict_hasitem_index
#define LOCAL_HAS_KEY_IS_INDEX
#define LOCAL_IS_HAS
#elif defined(DEFINE_dict_bounditem)
#define LOCAL_dict_getitem dict_bounditem
#define LOCAL_IS_BOUND
#elif defined(DEFINE_dict_bounditem_string_hash)
#define LOCAL_dict_getitem dict_bounditem_string_hash
#define LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_IS_BOUND
#elif defined(DEFINE_dict_bounditem_string_len_hash)
#define LOCAL_dict_getitem dict_bounditem_string_len_hash
#define LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#define LOCAL_IS_BOUND
#elif defined(DEFINE_dict_bounditem_index)
#define LOCAL_dict_getitem dict_bounditem_index
#define LOCAL_HAS_KEY_IS_INDEX
#define LOCAL_IS_BOUND
#else /* ... */
#error "Invalid configuration"
#endif /* ... */

#ifdef LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAS_KEY_IS_STRING
#elif defined(LOCAL_HAS_KEY_IS_STRING_LEN_HASH)
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_HAS_KEY_IS_STRING
#elif defined(LOCAL_HAS_KEY_IS_INDEX)
#define LOCAL_NONNULL    NONNULL((1))
#define LOCAL_KEY_PARAMS size_t index
#else /* ... */
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS DeeObject *key
#endif /* !... */


#ifdef LOCAL_IS_TRY
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_MISSING     ITER_DONE
#define LOCAL_ERR         NULL
#elif defined(LOCAL_IS_HAS)
#define LOCAL_return_type int
#define LOCAL_PRESENT     1
#define LOCAL_MISSING     0
#define LOCAL_ERR         (-1)
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_return_type int
#define LOCAL_PRESENT     Dee_BOUND_YES
#define LOCAL_MISSING     Dee_BOUND_MISSING
#define LOCAL_ERR         Dee_BOUND_ERR
#else /* LOCAL_IS_SETNEW || LOCAL_HAVE_SETOLD */
#define LOCAL_return_type DREF DeeObject *
#define LOCAL_ERR         NULL
#endif /* !LOCAL_IS_SETNEW && !LOCAL_HAVE_SETOLD */



#undef LOCAL_IS_UNLOCKED_OR_NOTHREADS
#if defined(LOCAL_IS_UNLOCKED) || defined(CONFIG_NO_THREADS)
#define LOCAL_IS_UNLOCKED_OR_NOTHREADS
#endif /* LOCAL_IS_UNLOCKED || CONFIG_NO_THREADS */

#ifndef LOCAL_IS_UNLOCKED_OR_NOTHREADS
#define LOCAL_IF_NOT_UNLOCKED_AND_THREADS(...) __VA_ARGS__
#else /* !LOCAL_IS_UNLOCKED_OR_NOTHREADS */
#define LOCAL_IF_NOT_UNLOCKED_AND_THREADS(...) /* nothing */
#endif /* LOCAL_IS_UNLOCKED_OR_NOTHREADS */

#ifndef LOCAL_IS_UNLOCKED
#define LOCAL_IF_NOT_UNLOCKED(...) __VA_ARGS__
#else /* !LOCAL_IS_UNLOCKED */
#define LOCAL_IF_NOT_UNLOCKED(...) /* nothing */
#endif /* LOCAL_IS_UNLOCKED */

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
LOCAL_dict_getitem(Dict *self, LOCAL_KEY_PARAMS) {
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
	Dee_hash_t hs, perturb;

/*again:*/
	LOCAL_DeeDict_LockRead(self);
/*again_locked:*/
	_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
#ifndef LOCAL_HAS_KEY_IS_STRING
		int cmp;
		DREF DeeObject *item_key;
#ifndef LOCAL_PRESENT
		DREF DeeObject *item_value;
#endif /* !LOCAL_PRESENT */
#endif /* !LOCAL_HAS_KEY_IS_STRING */
		struct Dee_dict_item *item;
		size_t vtab_idx = _DeeDict_HTabGet(self, hs);
		if (vtab_idx == Dee_DICT_HTAB_EOF)
			break; /* End-of-chain */
		ASSERT(Dee_dict_vidx_virt_lt_real(vtab_idx, self->d_vsize));
		item = &_DeeDict_GetVirtVTab(self)[vtab_idx];
		if (item->di_hash != LOCAL_hash)
			continue; /* Different hash */
#ifdef LOCAL_HAS_KEY_IS_STRING
		if (!DeeString_Check(item->di_key))
			continue; /* Not a string */
#endif /* LOCAL_HAS_KEY_IS_STRING */
		if (item->di_key == NULL)
			continue; /* Deleted item */

#ifdef LOCAL_HAS_KEY_IS_STRING
#ifdef LOCAL_HAS_KEY_IS_STRING_LEN_HASH
		if likely(DeeString_EqualsBuf(item->di_key, key, keylen))
#else /* LOCAL_HAS_KEY_IS_STRING_LEN_HASH */
		if likely(strcmp(DeeString_STR(item->di_key), key) == 0)
#endif /* !LOCAL_HAS_KEY_IS_STRING_LEN_HASH */
#else /* LOCAL_HAS_KEY_IS_STRING */
		/* This might be it! */
		item_key = item->di_key;
		LOCAL_IF_NOT_UNLOCKED(Dee_Incref(item_key));
#ifndef LOCAL_PRESENT
		item_value = item->di_value;
		LOCAL_IF_NOT_UNLOCKED(Dee_Incref(item_value));
#endif /* !LOCAL_PRESENT */
		LOCAL_DeeDict_LockEndRead(self);
#ifdef LOCAL_HAS_KEY_IS_INDEX
		cmp = DeeInt_Size_TryCompareEq(index, item_key);
#else /* LOCAL_HAS_KEY_IS_INDEX */
		cmp = DeeObject_TryCompareEq(key, item_key);
#endif /* !LOCAL_HAS_KEY_IS_INDEX */
		LOCAL_IF_NOT_UNLOCKED(Dee_Decref_unlikely(item_key));
		if likely(cmp == 0)
#endif /* !LOCAL_HAS_KEY_IS_STRING */
		{
#ifdef LOCAL_PRESENT
#ifdef LOCAL_HAS_KEY_IS_STRING
			LOCAL_DeeDict_LockEndRead(self);
#endif /* LOCAL_HAS_KEY_IS_STRING */
			return LOCAL_PRESENT;
#else /* !LOCAL_PRESENT */
#ifdef LOCAL_HAS_KEY_IS_STRING
			DREF DeeObject *item_value;
			item_value = item->di_value;
			Dee_Incref(item_value);
			LOCAL_DeeDict_LockEndRead(self);
#elif defined(LOCAL_IS_UNLOCKED)
			Dee_Incref(item_value);
#endif /* ... */
			return item_value;
#endif /* !LOCAL_PRESENT */
		}

#ifndef LOCAL_PRESENT
#ifndef LOCAL_HAS_KEY_IS_STRING
		LOCAL_IF_NOT_UNLOCKED(Dee_Decref_unlikely(item_value));
#endif /* !LOCAL_HAS_KEY_IS_STRING */
#endif /* !LOCAL_PRESENT */

#ifndef LOCAL_HAS_KEY_IS_STRING
		if unlikely(cmp == Dee_COMPARE_ERR)
			goto err;
#define NEED_err
		LOCAL_DeeDict_LockRead(self);
#endif /* !LOCAL_HAS_KEY_IS_STRING */
	}

	LOCAL_DeeDict_LockEndRead(self);
#ifdef LOCAL_MISSING
	return LOCAL_MISSING;
#elif defined(LOCAL_HAS_KEY_IS_STRING_LEN_HASH)
	err_unknown_key_str_len((DeeObject *)self, key, keylen);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_KEY_IS_STRING_HASH)
	err_unknown_key_str((DeeObject *)self, key);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_KEY_IS_INDEX)
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
#undef LOCAL_hash
}


#undef LOCAL_return_type
#undef LOCAL_PRESENT
#undef LOCAL_MISSING
#undef LOCAL_ERR
#undef LOCAL_IS_UNLOCKED_OR_NOTHREADS
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
#undef LOCAL_HAS_KEY_IS_STRING
#undef LOCAL_HAS_KEY_IS_STRING_HASH
#undef LOCAL_HAS_KEY_IS_STRING_LEN_HASH
#undef LOCAL_HAS_KEY_IS_INDEX
#undef LOCAL_IS_UNLOCKED
#undef LOCAL_IS_TRY
#undef LOCAL_IS_HAS
#undef LOCAL_IS_BOUND
#undef LOCAL_dict_getitem

DECL_END

#undef DEFINE_dict_trygetitem
#undef DEFINE_dict_trygetitem_string_hash
#undef DEFINE_dict_trygetitem_string_len_hash
#undef DEFINE_dict_trygetitem_index
#undef DEFINE_dict_getitem
#undef DEFINE_dict_getitem_string_hash
#undef DEFINE_dict_getitem_string_len_hash
#undef DEFINE_dict_getitem_index
#undef DEFINE_dict_hasitem
#undef DEFINE_dict_hasitem_string_hash
#undef DEFINE_dict_hasitem_string_len_hash
#undef DEFINE_dict_hasitem_index
#undef DEFINE_dict_bounditem
#undef DEFINE_dict_bounditem_string_hash
#undef DEFINE_dict_bounditem_string_len_hash
#undef DEFINE_dict_bounditem_index
