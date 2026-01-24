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
#include "dict.c"
//#define DEFINE_dict_trygetitem
//#define DEFINE_dict_trygetitem_string_hash
//#define DEFINE_dict_trygetitem_string_len_hash
//#define DEFINE_dict_trygetitem_index
#define DEFINE_dict_getitem
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

#include <deemon/api.h>

#include <stddef.h> /* NULL, size_t */

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


#ifdef LOCAL_HAS_KEY_IS_STRING_HASH
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_boolcmp(rhs) boolcmp_string(key, rhs)
#elif defined(LOCAL_HAS_KEY_IS_STRING_LEN_HASH)
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_boolcmp(rhs) boolcmp_string_len(key, keylen, rhs)
#elif defined(LOCAL_HAS_KEY_IS_INDEX)
#define LOCAL_NONNULL    NONNULL((1))
#define LOCAL_KEY_PARAMS size_t index
#define LOCAL_fastcmp(rhs) fastcmp_index(index, rhs)
#define LOCAL_slowcmp(rhs) slowcmp_index(index, rhs)
#else /* ... */
#define LOCAL_NONNULL    NONNULL((1, 2))
#define LOCAL_KEY_PARAMS DeeObject *key
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

	LOCAL_DeeDict_LockRead(self);
#ifndef LOCAL_boolcmp
LOCAL_IF_NOT_UNLOCKED(again_with_lock:)
#endif /* !LOCAL_boolcmp */
	_DeeDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);
	for (;; _DeeDict_HashIdxAdv(self, &hs, &perturb)) {
		DREF DeeObject *item_key;
#ifndef LOCAL_boolcmp
		int item_key_cmp_caller_key;
#endif /* !LOCAL_boolcmp */
#ifndef LOCAL_PRESENT
		DREF DeeObject *item_value;
#endif /* LOCAL_PRESENT */
		struct Dee_dict_item *item;
		size_t htab_idx;          /* hash-index in "d_htab" */
		Dee_dict_vidx_t vtab_idx; /* hash-index in "d_vtab" */

		/* Load hash indices */
		htab_idx = hs & self->d_hmask;
		vtab_idx = (*self->d_hidxget)(self->d_htab, htab_idx);

		/* Check for end-of-hash-chain */
		if (vtab_idx == Dee_DICT_HTAB_EOF)
			break;

		/* Load referenced item in "d_vtab" */
		ASSERT(Dee_dict_vidx_virt_lt_real(vtab_idx, self->d_vsize));
		item = &_DeeDict_GetVirtVTab(self)[vtab_idx];

		/* Check for deleted items... */
		item_key = item->di_key;
		if (item_key == NULL)
			continue;

		/* Check if hash really matches. */
		if (item->di_hash != LOCAL_hash)
			continue; /* Different hash */

		/* Helper macro used to verify that the dict didn't change. */
#ifdef LOCAL_IS_UNLOCKED
#define LOCAL_verify_unchanged_after_unlock(goto_if_changed) (void)0
#else /* LOCAL_IS_UNLOCKED */
#define LOCAL_verify_unchanged_after_unlock(goto_if_changed)                \
	do {                                                                    \
		if unlikely(htab_idx != (hs & self->d_hmask))                       \
			goto goto_if_changed;                                           \
		if unlikely(vtab_idx != (*self->d_hidxget)(self->d_htab, htab_idx)) \
			goto goto_if_changed;                                           \
		if unlikely(item != &_DeeDict_GetVirtVTab(self)[vtab_idx])          \
			goto goto_if_changed;                                           \
		if unlikely(item->di_key != item_key)                               \
			goto goto_if_changed;                                           \
		if unlikely(item->di_hash != LOCAL_hash)                            \
			goto goto_if_changed;                                           \
	}	__WHILE0
#endif /* !LOCAL_IS_UNLOCKED */


#ifdef LOCAL_boolcmp
		if likely(LOCAL_boolcmp(item_key)) {
#ifdef LOCAL_PRESENT
			LOCAL_DeeDict_LockEndRead(self);
			return LOCAL_PRESENT;
#else /* LOCAL_PRESENT */
			item_value = item->di_value;
			Dee_Incref(item_value);
			LOCAL_DeeDict_LockEndRead(self);
			return item_value;
#endif /* !LOCAL_PRESENT */
		}
#else /* LOCAL_boolcmp */

		/* Special optimizations when the caller-given "key" is a special value. */
#ifdef LOCAL_fastcmp
		{
			int fastcmp_result = LOCAL_fastcmp(item_key);
			if likely(fastcmp_result == 0) {
				/* Found the item! */
#ifdef LOCAL_PRESENT
				LOCAL_DeeDict_LockEndRead(self);
				return LOCAL_PRESENT;
#else /* LOCAL_PRESENT */
				item_value = item->di_value;
				Dee_Incref(item_value);
				LOCAL_DeeDict_LockEndRead(self);
				return item_value;
#endif /* !LOCAL_PRESENT */
			}
			if (fastcmp_result > 0)
				continue; /* Wrong key (hash just matched by random) */
		}
#endif /* LOCAL_fastcmp */

		/* Load item key and release dict lock (so we can do compare) */
		Dee_Incref(item_key);
#ifndef LOCAL_PRESENT
		item_value = item->di_value;
		Dee_Incref(item_value);
#endif /* LOCAL_PRESENT */
		LOCAL_DeeDict_LockEndRead(self);

		/* Regular caller-given key vs. dict key compare. */
#ifdef LOCAL_slowcmp
		item_key_cmp_caller_key = LOCAL_slowcmp(item_key);
#else /* LOCAL_slowcmp */
		item_key_cmp_caller_key = DeeObject_TryCompareEq(key, item_key);
#endif /* !LOCAL_slowcmp */
		Dee_Decref_unlikely(item_key);

		/* Case: keys are equal, meaning we must override this item! */
		if likely(Dee_COMPARE_ISEQ_NO_ERR(item_key_cmp_caller_key)) {
#ifdef LOCAL_PRESENT
			return LOCAL_PRESENT;
#else /* LOCAL_PRESENT */
			return item_value;
#endif /* !LOCAL_PRESENT */
		}

#ifndef LOCAL_PRESENT
		Dee_Decref_unlikely(item_value);
#endif /* LOCAL_PRESENT */

		/* Check for error during compare. */
		if (Dee_COMPARE_ISERR(item_key_cmp_caller_key))
			goto err;
#define NEED_err

		/* Re-acquire lock */
		LOCAL_DeeDict_LockRead(self);

		/* Check if the dict changed. */
		LOCAL_verify_unchanged_after_unlock(again_with_lock);
#endif /* !LOCAL_boolcmp */
#undef LOCAL_verify_unchanged_after_unlock
	}
	LOCAL_DeeDict_LockEndRead(self);

	/* Indicate that the given key is missing... */
#ifdef LOCAL_MISSING
	return LOCAL_MISSING;
#elif defined(DEFINE_dict_mh_pop_with_default)
	Dee_Incref(def);
	return def;
#elif defined(LOCAL_HAS_KEY_IS_STRING_LEN_HASH)
	DeeRT_ErrUnknownKeyStrLen(self, key, keylen);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_KEY_IS_STRING_HASH)
	DeeRT_ErrUnboundKeyStr(self, key);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_KEY_IS_INDEX)
	DeeRT_ErrUnknownKeyInt(self, index);
#define NEED_err_fallthru
#else /* ... */
	DeeRT_ErrUnknownKey(self, key);
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

#undef LOCAL_boolcmp
#undef LOCAL_fastcmp
#undef LOCAL_slowcmp

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
#undef LOCAL_PRESENT
#undef LOCAL_MISSING
#undef LOCAL_ERR
#undef LOCAL_HAS_KEY_IS_INDEX
#undef LOCAL_HAS_KEY_IS_STRING_HASH
#undef LOCAL_HAS_KEY_IS_STRING_LEN_HASH
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
