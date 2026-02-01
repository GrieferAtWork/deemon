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
#include "rodict.c"
//#define DEFINE_rodict_contains
//#define DEFINE_rodict_getitem
#define DEFINE_rodict_getitemnr
//#define DEFINE_rodict_bounditem
//#define DEFINE_rodict_hasitem
//#define DEFINE_rodict_trygetitem
//#define DEFINE_rodict_trygetitemnr
//#define DEFINE_rodict_getitem_index
//#define DEFINE_rodict_bounditem_index
//#define DEFINE_rodict_hasitem_index
//#define DEFINE_rodict_trygetitem_index
//#define DEFINE_rodict_trygetitem_string_hash
//#define DEFINE_rodict_trygetitemnr_string_hash
//#define DEFINE_rodict_getitem_string_hash
//#define DEFINE_rodict_bounditem_string_hash
//#define DEFINE_rodict_hasitem_string_hash
//#define DEFINE_rodict_trygetitem_string_len_hash
//#define DEFINE_rodict_trygetitemnr_string_len_hash
//#define DEFINE_rodict_getitem_string_len_hash
//#define DEFINE_rodict_bounditem_string_len_hash
//#define DEFINE_rodict_hasitem_string_len_hash
//#define DEFINE_rodict_getitemnr_string_hash
//#define DEFINE_rodict_getitemnr_string_len_hash
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_rodict_contains) +                     \
     defined(DEFINE_rodict_getitem) +                      \
     defined(DEFINE_rodict_getitemnr) +                    \
     defined(DEFINE_rodict_bounditem) +                    \
     defined(DEFINE_rodict_hasitem) +                      \
     defined(DEFINE_rodict_trygetitem) +                   \
     defined(DEFINE_rodict_trygetitemnr) +                 \
     defined(DEFINE_rodict_getitem_index) +                \
     defined(DEFINE_rodict_bounditem_index) +              \
     defined(DEFINE_rodict_hasitem_index) +                \
     defined(DEFINE_rodict_trygetitem_index) +             \
     defined(DEFINE_rodict_trygetitem_string_hash) +       \
     defined(DEFINE_rodict_trygetitemnr_string_hash) +     \
     defined(DEFINE_rodict_getitem_string_hash) +          \
     defined(DEFINE_rodict_bounditem_string_hash) +        \
     defined(DEFINE_rodict_hasitem_string_hash) +          \
     defined(DEFINE_rodict_trygetitem_string_len_hash) +   \
     defined(DEFINE_rodict_trygetitemnr_string_len_hash) + \
     defined(DEFINE_rodict_getitem_string_len_hash) +      \
     defined(DEFINE_rodict_bounditem_string_len_hash) +    \
     defined(DEFINE_rodict_hasitem_string_len_hash) +      \
     defined(DEFINE_rodict_getitemnr_string_hash) +        \
     defined(DEFINE_rodict_getitemnr_string_len_hash)) != 1
#error "Must #define exactly 1 of these macros"
#endif /* ... */

#ifdef DEFINE_rodict_contains
#define LOCAL_rodict_getitem rodict_contains
#define LOCAL_RETURNS_DEEMON_BOOL
#elif defined(DEFINE_rodict_getitem)
#define LOCAL_rodict_getitem rodict_getitem
#elif defined(DEFINE_rodict_getitemnr)
#define LOCAL_rodict_getitem rodict_getitemnr
#define LOCAL_IS_NOREF
#elif defined(DEFINE_rodict_bounditem)
#define LOCAL_rodict_getitem rodict_bounditem
#define LOCAL_IS_BOUND
#elif defined(DEFINE_rodict_hasitem)
#define LOCAL_rodict_getitem rodict_hasitem
#define LOCAL_IS_HAS
#elif defined(DEFINE_rodict_trygetitem)
#define LOCAL_rodict_getitem rodict_trygetitem
#define LOCAL_IS_TRY
#elif defined(DEFINE_rodict_trygetitemnr)
#define LOCAL_rodict_getitem rodict_trygetitemnr
#define LOCAL_IS_TRY
#define LOCAL_IS_NOREF
#elif defined(DEFINE_rodict_getitem_index)
#define LOCAL_rodict_getitem rodict_getitem_index
#define LOCAL_HAS_INDEX
#elif defined(DEFINE_rodict_bounditem_index)
#define LOCAL_rodict_getitem rodict_bounditem_index
#define LOCAL_IS_BOUND
#define LOCAL_HAS_INDEX
#elif defined(DEFINE_rodict_hasitem_index)
#define LOCAL_rodict_getitem rodict_hasitem_index
#define LOCAL_IS_HAS
#define LOCAL_HAS_INDEX
#elif defined(DEFINE_rodict_trygetitem_index)
#define LOCAL_rodict_getitem rodict_trygetitem_index
#define LOCAL_IS_TRY
#define LOCAL_HAS_INDEX
#elif defined(DEFINE_rodict_trygetitem_string_hash)
#define LOCAL_rodict_getitem rodict_trygetitem_string_hash
#define LOCAL_IS_TRY
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_trygetitemnr_string_hash)
#define LOCAL_rodict_getitem rodict_trygetitemnr_string_hash
#define LOCAL_IS_TRY
#define LOCAL_IS_NOREF
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_getitem_string_hash)
#define LOCAL_rodict_getitem rodict_getitem_string_hash
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_bounditem_string_hash)
#define LOCAL_rodict_getitem rodict_bounditem_string_hash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_hasitem_string_hash)
#define LOCAL_rodict_getitem rodict_hasitem_string_hash
#define LOCAL_IS_HAS
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_trygetitem_string_len_hash)
#define LOCAL_rodict_getitem rodict_trygetitem_string_len_hash
#define LOCAL_IS_TRY
#define LOCAL_HAS_STRING_LEN_HASH
#elif defined(DEFINE_rodict_trygetitemnr_string_len_hash)
#define LOCAL_rodict_getitem rodict_trygetitemnr_string_len_hash
#define LOCAL_IS_TRY
#define LOCAL_IS_NOREF
#define LOCAL_HAS_STRING_LEN_HASH
#elif defined(DEFINE_rodict_getitem_string_len_hash)
#define LOCAL_rodict_getitem rodict_getitem_string_len_hash
#define LOCAL_HAS_STRING_LEN_HASH
#elif defined(DEFINE_rodict_bounditem_string_len_hash)
#define LOCAL_rodict_getitem rodict_bounditem_string_len_hash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_STRING_LEN_HASH
#elif defined(DEFINE_rodict_hasitem_string_len_hash)
#define LOCAL_rodict_getitem rodict_hasitem_string_len_hash
#define LOCAL_IS_HAS
#define LOCAL_HAS_STRING_LEN_HASH
#elif defined(DEFINE_rodict_getitemnr_string_hash)
#define LOCAL_rodict_getitem rodict_getitemnr_string_hash
#define LOCAL_IS_NOREF
#define LOCAL_HAS_STRING_HASH
#elif defined(DEFINE_rodict_getitemnr_string_len_hash)
#define LOCAL_rodict_getitem rodict_getitemnr_string_len_hash
#define LOCAL_IS_NOREF
#define LOCAL_HAS_STRING_LEN_HASH
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#include <deemon/api.h>

#include <deemon/bool.h>         /* return_false, return_true */
#include <deemon/dict.h>         /* Dee_dict_item */
#include <deemon/error-rt.h>     /* DeeRT_Err* */
#include <deemon/int.h>          /* DeeInt_Size_TryCompareEq */
#include <deemon/object.h>
#include <deemon/rodict.h>       /* _DeeRoDict_* */
#include <deemon/util/hash-io.h> /* Dee_HASH_HTAB_EOF, Dee_hash_* */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#ifdef LOCAL_IS_BOUND
#define LOCAL_return_type       int
#define LOCAL_return_NOKEY      return Dee_BOUND_MISSING
#define LOCAL_return_ITEM(item) return Dee_BOUND_YES
#define LOCAL_return_ERR        return Dee_BOUND_ERR
#elif defined(LOCAL_RETURNS_DEEMON_BOOL)
#define LOCAL_return_type       DREF DeeObject *
#define LOCAL_return_NOKEY      return_false
#define LOCAL_return_ITEM(item) return_true
#define LOCAL_return_ERR        return NULL
#elif defined(LOCAL_IS_HAS)
#define LOCAL_return_type       int
#define LOCAL_return_NOKEY      return Dee_BOUND_MISSING
#define LOCAL_return_ITEM(item) return Dee_BOUND_YES
#define LOCAL_return_ERR        return Dee_BOUND_ERR
#elif defined(LOCAL_IS_NOREF)
#define LOCAL_return_type       DeeObject *
#ifdef LOCAL_IS_TRY
#define LOCAL_return_NOKEY      return ITER_DONE
#endif /* LOCAL_IS_TRY */
#define LOCAL_return_ITEM(item) return item->di_value
#define LOCAL_return_ERR        return NULL
#else /* ... */
#define LOCAL_return_type       DREF DeeObject *
#ifdef LOCAL_IS_TRY
#define LOCAL_return_NOKEY      return ITER_DONE
#endif /* LOCAL_IS_TRY */
#define LOCAL_return_ITEM(item) return_reference_(item->di_value)
#define LOCAL_return_ERR        return NULL
#endif /* !... */

#ifdef LOCAL_HAS_STRING_LEN_HASH
#define LOCAL_KEY_PARAMS char const *key, size_t keylen, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_boolcmp(rhs) boolcmp_string_len(key, keylen, rhs)
#elif defined(LOCAL_HAS_STRING_HASH)
#define LOCAL_KEY_PARAMS char const *key, Dee_hash_t hash
#define LOCAL_HAS_PARAM_HASH
#define LOCAL_boolcmp(rhs) boolcmp_string(key, rhs)
#elif defined(LOCAL_HAS_INDEX)
#define LOCAL_KEY_PARAMS   size_t key
#define LOCAL_compare(rhs) DeeInt_Size_TryCompareEq(key, rhs)
#else /* ... */
#define LOCAL_KEY_PARAMS   DeeObject *key
#define LOCAL_compare(rhs) DeeObject_TryCompareEq(key, rhs)
#endif /* !... */

PRIVATE WUNUSED NONNULL((1)) LOCAL_return_type DCALL
LOCAL_rodict_getitem(RoDict *self, LOCAL_KEY_PARAMS) {
#ifdef LOCAL_HAS_PARAM_HASH
#define LOCAL_hash hash
#elif defined(LOCAL_HAS_INDEX)
#define LOCAL_hash key
#else /* ... */
	Dee_hash_t hash = DeeObject_Hash(key);
#define LOCAL_hash hash
#endif /* !... */
	Dee_hash_t hs, perturb;

	for (_DeeRoDict_HashIdxInit(self, &hs, &perturb, LOCAL_hash);;
	     _DeeRoDict_HashIdxNext(self, &hs, &perturb, LOCAL_hash)) {
		struct Dee_dict_item *item;
		Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, self->rd_hmask);
		Dee_hash_vidx_t vtab_idx = (*self->rd_hidxget)(self->rd_htab, htab_idx); /*virt*/

		/* Check for end-of-hash-chain */
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break;

		/* Load referenced item in "rd_vtab" */
		ASSERT(Dee_hash_vidx_virt_lt_real(vtab_idx, self->rd_vsize));
		item = &_DeeRoDict_GetVirtVTab(self)[vtab_idx];

		/* Check if hash really matches. */
		if (item->di_hash != LOCAL_hash)
			continue; /* Different hash */

		/* Actually compare keys */
#ifdef LOCAL_boolcmp
		if unlikely(!LOCAL_boolcmp(item->di_key))
			continue;
#else /* LOCAL_boolcmp */
		{
			int status = LOCAL_compare(item->di_key);
			if unlikely(Dee_COMPARE_ISNE_OR_ERR(status)) {
				if (Dee_COMPARE_ISERR(status))
					goto err;
#define NEED_err
				continue;
			}
		}
#endif /* !LOCAL_boolcmp */

		/* Found it! */
		LOCAL_return_ITEM(item);
	}

	/* Key could not be found... */
#ifdef LOCAL_return_NOKEY
	LOCAL_return_NOKEY;
#elif defined(LOCAL_HAS_STRING_LEN_HASH)
	DeeRT_ErrUnknownKeyStrLen(self, key, keylen);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_STRING_HASH)
	DeeRT_ErrUnboundKeyStr(self, key);
#define NEED_err_fallthru
#elif defined(LOCAL_HAS_INDEX)
	DeeRT_ErrUnknownKeyInt(self, key);
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
	LOCAL_return_ERR;
#endif /* NEED_err_fallthru */
#undef LOCAL_hash
}

#undef LOCAL_return_type
#undef LOCAL_return_NOKEY
#undef LOCAL_return_ITEM
#undef LOCAL_return_ERR
#undef LOCAL_KEY_PARAMS
#undef LOCAL_HAS_PARAM_HASH
#undef LOCAL_boolcmp
#undef LOCAL_compare

DECL_END

#undef LOCAL_rodict_getitem
#undef LOCAL_RETURNS_DEEMON_BOOL
#undef LOCAL_IS_NOREF
#undef LOCAL_IS_BOUND
#undef LOCAL_IS_HAS
#undef LOCAL_IS_TRY
#undef LOCAL_HAS_INDEX
#undef LOCAL_HAS_STRING_HASH
#undef LOCAL_HAS_STRING_LEN_HASH

#undef DEFINE_rodict_contains
#undef DEFINE_rodict_getitem
#undef DEFINE_rodict_getitemnr
#undef DEFINE_rodict_bounditem
#undef DEFINE_rodict_hasitem
#undef DEFINE_rodict_trygetitem
#undef DEFINE_rodict_trygetitemnr
#undef DEFINE_rodict_getitem_index
#undef DEFINE_rodict_bounditem_index
#undef DEFINE_rodict_hasitem_index
#undef DEFINE_rodict_trygetitem_index
#undef DEFINE_rodict_trygetitem_string_hash
#undef DEFINE_rodict_trygetitemnr_string_hash
#undef DEFINE_rodict_getitem_string_hash
#undef DEFINE_rodict_bounditem_string_hash
#undef DEFINE_rodict_hasitem_string_hash
#undef DEFINE_rodict_trygetitem_string_len_hash
#undef DEFINE_rodict_trygetitemnr_string_len_hash
#undef DEFINE_rodict_getitem_string_len_hash
#undef DEFINE_rodict_bounditem_string_len_hash
#undef DEFINE_rodict_hasitem_string_len_hash
#undef DEFINE_rodict_getitemnr_string_hash
#undef DEFINE_rodict_getitemnr_string_len_hash
