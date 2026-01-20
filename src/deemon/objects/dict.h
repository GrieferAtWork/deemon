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
#ifndef GUARD_DEEMON_OBJECTS_DICT_H
#define GUARD_DEEMON_OBJECTS_DICT_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/bytes.h>
#include <deemon/dict.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* CONFIG_HAVE_strcmp */

#include <hybrid/bit.h>      /* CLZ */
#include <hybrid/overflow.h> /* OVERFLOW_UADD, OVERFLOW_UMUL */
#include <hybrid/typecore.h> /* __CHAR_BIT__, __SHIFT_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef shift_t
#define shift_t __SHIFT_TYPE__

#ifndef SIZE_MAX
#include <hybrid/limitcore.h> /* __SIZE_MAX__ */
#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif /* !SIZE_MAX */
#endif /* !SIZE_MAX */

#undef DICT_NDEBUG
#if defined(NDEBUG) || 1
#define DICT_NDEBUG
#endif /* NDEBUG */

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */



/*DFUNDEF WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx8(void *__restrict htab, size_t index);*/
/*DFUNDEF NONNULL((1)) void DCALL Dee_dict_sethidx8(void *__restrict htab, size_t index, size_t value);*/
INTDEF NONNULL((1)) void DCALL Dee_dict_movhidx8(void *__restrict dst, void const *__restrict src, size_t n_words);

#if DEE_DICT_HIDXIO_COUNT >= 2
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx16(void const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_dict_sethidx16(void *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_dict_movhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx8_PTR &Dee_dict_uprhidx8
INTDEF NONNULL((1)) void DCALL Dee_dict_uprhidx8(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_dict_dwnhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx32(void const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_dict_sethidx32(void *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_dict_movhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx16_PTR &Dee_dict_uprhidx16
INTDEF NONNULL((1)) void DCALL Dee_dict_uprhidx16(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_dict_dwnhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_dict_gethidx64(void const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_dict_sethidx64(void *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_dict_movhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_dict_uprhidx32_PTR &Dee_dict_uprhidx32
INTDEF NONNULL((1)) void DCALL Dee_dict_uprhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_dict_dwnhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */

#if DEE_DICT_HIDXIO_COUNT >= 2
#define IF_DEE_DICT_HIDXIO_COUNT_GE_2(...) __VA_ARGS__
#else /* DEE_DICT_HIDXIO_COUNT >= 2 */
#define IF_DEE_DICT_HIDXIO_COUNT_GE_2(...) /* nothing */
#endif /* DEE_DICT_HIDXIO_COUNT < 2 */
#if DEE_DICT_HIDXIO_COUNT >= 3
#define IF_DEE_DICT_HIDXIO_COUNT_GE_3(...) __VA_ARGS__
#else /* DEE_DICT_HIDXIO_COUNT >= 3 */
#define IF_DEE_DICT_HIDXIO_COUNT_GE_3(...) /* nothing */
#endif /* DEE_DICT_HIDXIO_COUNT < 3 */
#if DEE_DICT_HIDXIO_COUNT >= 4
#define IF_DEE_DICT_HIDXIO_COUNT_GE_4(...) __VA_ARGS__
#else /* DEE_DICT_HIDXIO_COUNT >= 4 */
#define IF_DEE_DICT_HIDXIO_COUNT_GE_4(...) /* nothing */
#endif /* DEE_DICT_HIDXIO_COUNT < 4 */


/* This right here encodes our target ratio of used vs. unused hash indices.
 * Currently, we aim for a ratio of:
 * >> d_valloc * 2   ~=   d_hmask * 3 */
#define DICT_VTAB_HTAB_RATIO_V 2
#define DICT_VTAB_HTAB_RATIO_H 3

STATIC_ASSERT(DICT_VTAB_HTAB_RATIO_H > DICT_VTAB_HTAB_RATIO_V);

/* Returns true if "d_htab" *should* be grown to its next larger multiple */
#define _DeeDict_ShouldGrowHTab(self) \
	_DeeDict_ShouldGrowHTab2((self)->d_vused, (self)->d_hmask)
#define _DeeDict_ShouldGrowHTab2(vused, hmask)                                       \
	((dict_suggested_max_valloc_from_count((vused) + 1) * DICT_VTAB_HTAB_RATIO_V) >= \
	 ((hmask) * DICT_VTAB_HTAB_RATIO_H))

/* Returns true if "d_htab" *MUST* be grown to its next larger
 * multiple (before a new item can be added to the dict) */
#define _DeeDict_MustGrowHTab(self) \
	_DeeDict_MustGrowHTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_MustGrowHTab2(valloc, hmask) \
	((valloc) >= (hmask))


/* Evaluates to true if the dict's "d_vtab" should be optimized.
 * s.a. `dict_optimize_vtab()' */
#define _DeeDict_ShouldOptimizeVTab(self) \
	_DeeDict_ShouldOptimizeVTab2((self)->d_vsize, (self)->d_vused)
#define _DeeDict_ShouldOptimizeVTab2(vsize, vused) \
	(((vsize) >> 1) > (vused))
#define _DeeDict_CanOptimizeVTab(self) \
	_DeeDict_CanOptimizeVTab2((self)->d_vsize, (self)->d_vused)
#define _DeeDict_CanOptimizeVTab2(vsize, vused) \
	((vsize) > (vused))

#define _DeeDict_MustGrowVTab(self) \
	_DeeDict_MustGrowVTab2((self)->d_vsize, (self)->d_valloc)
#define _DeeDict_MustGrowVTab2(vsize, valloc) \
	((vsize) >= (valloc))
#define _DeeDict_CanGrowVTab(self) \
	_DeeDict_CanGrowVTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_CanGrowVTab2(valloc, hmask) \
	((valloc) < (hmask))


/* Returns true if the vtab should shrink following an item being deleted. */
#define _DeeDict_ShouldShrinkVTab(self) \
	_DeeDict_ShouldShrinkVTab2((self)->d_vused, (self)->d_valloc)
#define _DeeDict_ShouldShrinkVTab_NEWALLOC(self) \
	_DeeDict_ShouldShrinkVTab_NEWALLOC2((self)->d_vused)
#define _DeeDict_ShouldShrinkVTab2(vused, valloc) \
	(_DeeDict_ShouldShrinkVTab_NEWALLOC2(vused) < (valloc))
#define _DeeDict_ShouldShrinkVTab_NEWALLOC2(vused) \
	((vused) << 1) /* Shrink the vtab when more than half of it is unused */

/* Returns true if the vtab can be shrunk. */
#define _DeeDict_CanShrinkVTab(self) \
	_DeeDict_CanShrinkVTab2((self)->d_vused, (self)->d_valloc)
#define _DeeDict_CanShrinkVTab2(vused, valloc) \
	((vused) < (valloc))

/* Returns true if the htab should shrink following the vtab having been shrunk */
#define _DeeDict_ShouldShrinkHTab(self) \
	_DeeDict_ShouldShrinkHTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_ShouldShrinkHTab2(valloc, hmask) \
	(((valloc)*DICT_VTAB_HTAB_RATIO_H) <=         \
	 (((hmask) >> 1) * DICT_VTAB_HTAB_RATIO_V) && (hmask))

/* Returns true if the htab can be shrunk. */
#define _DeeDict_CanShrinkHTab(self) \
	_DeeDict_CanShrinkHTab2((self)->d_valloc, (self)->d_hmask)
#define _DeeDict_CanShrinkHTab2(valloc, hmask) \
	((valloc) <= ((hmask) >> 1) && (hmask))


/* Default hint in `DeeDict_FromSequence' when the sequence doesn't provide one itself. */
#ifndef DICT_FROMSEQ_DEFAULT_HINT
#define DICT_FROMSEQ_DEFAULT_HINT 64
#endif /* !DICT_FROMSEQ_DEFAULT_HINT */


LOCAL ATTR_CONST size_t DCALL
dict_suggested_max_valloc_from_count(size_t num_item) {
	shift_t items_nbits;
	size_t result;
	ASSERT(num_item > 0);
	items_nbits = CLZ(num_item);
	if unlikely(!items_nbits)
		return (size_t)-1;
	items_nbits = (sizeof(size_t) * __CHAR_BIT__) - items_nbits;
	result = ((size_t)1 << (items_nbits)) |
	         ((size_t)1 << (items_nbits + 1));
	ASSERT(result >= num_item);
	return result;
}

/* Calculate what would be a good "d_valloc" for a given "d_hmask" and "d_vsize" */
LOCAL ATTR_CONST size_t DCALL
dict_valloc_from_hmask_and_count(size_t hmask, size_t num_item, bool allow_overalloc) {
	size_t result, max_valloc;
	ASSERT(num_item <= hmask);
	if (!allow_overalloc)
		return num_item;
	result = (hmask / DICT_VTAB_HTAB_RATIO_H) * DICT_VTAB_HTAB_RATIO_V;
	ASSERT(result <= hmask);
	if unlikely(result < num_item)
		return num_item;
	if unlikely(!num_item)
		return 0;
	max_valloc = dict_suggested_max_valloc_from_count(num_item);
	if (result > max_valloc)
		result = max_valloc;
	ASSERT(result >= num_item);
	ASSERT(result <= hmask);
	return result;
}


LOCAL ATTR_CONST size_t DCALL
dict_hmask_from_count(size_t num_item) {
	size_t result;
	shift_t mask_nbits;
	if unlikely(!num_item)
		return 0;
	if (OVERFLOW_UMUL(num_item, DICT_VTAB_HTAB_RATIO_H, &result))
		goto fallback;
	if (OVERFLOW_UADD(result, DICT_VTAB_HTAB_RATIO_V - 1, &result))
		goto fallback;
#if DICT_VTAB_HTAB_RATIO_V == 2
	result >>= 1;
#else /* DICT_VTAB_HTAB_RATIO_V == 2 */
	result /= DICT_VTAB_HTAB_RATIO_V;
#endif /* DICT_VTAB_HTAB_RATIO_V != 2 */
	ASSERT(result);
	mask_nbits = CLZ(result);
	mask_nbits = (sizeof(size_t) * __CHAR_BIT__) - mask_nbits;
	result = 1;
	result <<= mask_nbits;
	--result;
	ASSERT(result >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, true) >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, false) >= num_item);
	return result;
fallback:
	return SIZE_MAX;
}

LOCAL ATTR_CONST size_t DCALL
dict_tiny_hmask_from_count(size_t num_item) {
	size_t result;
	shift_t mask_nbits;
	if unlikely(!num_item)
		return 0;
	mask_nbits = CLZ(num_item);
	mask_nbits = (sizeof(size_t) * __CHAR_BIT__) - mask_nbits;
	result = 1;
	result <<= mask_nbits;
	--result;
	ASSERT(result >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, true) >= num_item);
	ASSERT(dict_valloc_from_hmask_and_count(result, num_item, false) >= num_item);
	return result;
}

#define dict_sizeoftabs_from_hmask_and_valloc(hmask, valloc) \
	dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(hmask, valloc, DEE_DICT_HIDXIO_FROMALLOC(valloc))
LOCAL ATTR_CONST size_t DCALL
dict_sizeoftabs_from_hmask_and_valloc_and_hidxio(size_t hmask, size_t valloc, shift_t hidxio) {
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
	return result;
toobig:
	return (size_t)-1; /* Force down-stream allocation failure */
}






/* Fast-compare functions that never throw or invoke user-code:
 * - Can be called while holding locks.
 * @return:  1: It is guarantied that "index != rhs"
 * @return:  0: It is guarantied that "index == rhs"
 * @return: -1: Comparison cannot be done "fast" - load "rhs" properly, release locks, and try again. */
LOCAL WUNUSED NONNULL((2)) int DCALL
fastcmp_index(size_t lhs, DeeObject *rhs) {
	if (DeeInt_Check(rhs)) {
		size_t rhs_value;
		if (!DeeInt_TryAsSize(rhs, &rhs_value))
			return 1;
		return lhs == rhs_value ? 0 : 1;
	}
	return -1;
}

LOCAL WUNUSED NONNULL((1, 2)) bool DCALL
boolcmp_string(char const *lhs, DeeObject *rhs) {
	if (DeeString_Check(rhs))
		return strcmp(lhs, DeeString_STR(rhs)) == 0;
	if (DeeBytes_Check(rhs)) {
		size_t lhslen = strlen(lhs);
		if (lhslen != DeeBytes_SIZE(rhs))
			return false;
		return memcmp(lhs, DeeBytes_DATA(rhs), lhslen) == 0;
	}
	return false;
}

LOCAL WUNUSED NONNULL((1, 3)) bool DCALL
boolcmp_string_len(char const *lhs, size_t lhslen, DeeObject *rhs) {
	if (DeeString_Check(rhs)) {
		if (lhslen != DeeString_SIZE(rhs))
			return false;
		return memcmp(lhs, DeeString_STR(rhs), lhslen) == 0;
	}
	if (DeeBytes_Check(rhs)) {
		if (lhslen != DeeBytes_SIZE(rhs))
			return false;
		return memcmp(lhs, DeeBytes_DATA(rhs), lhslen) == 0;
	}
	return false;
}

/* Slow equivalents to the above -- only use these when the above returned "-1"
 * Return value is like `DeeObject_TryCompareEq()' (iow: Dee_COMPARE_ERR on error) */
LOCAL WUNUSED NONNULL((2)) int DCALL
slowcmp_index(size_t lhs, DeeObject *rhs) {
	bool ok;
	size_t rhs_value;
	DREF DeeObject *rhs_as_int;
	rhs_as_int = DeeObject_Int(rhs);
	if unlikely(!rhs_as_int)
		goto err;
	ok = DeeInt_TryAsSize(rhs_as_int, &rhs_value);
	Dee_Decref(rhs_as_int);
	if (!ok)
		return 1;
	return lhs == rhs_value ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}
//LOCAL WUNUSED NONNULL((1, 2)) int DCALL slowcmp_string(char const *lhs, DeeObject *rhs);
//LOCAL WUNUSED NONNULL((1, 3)) int DCALL slowcmp_string_len(char const *lhs, size_t lhslen, DeeObject *rhs);


LOCAL WUNUSED NONNULL((2)) DREF DeeObject *DCALL
matched_keyob_tryfrom_index(size_t key, DeeObject *matched_key) {
	if (DeeInt_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeInt_TryNewSize
	return DeeInt_TryNewSize(key);
#else /* DeeInt_TryNewSize */
	(void)key;
	return NULL;
#endif /* !DeeInt_TryNewSize */
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
matched_keyob_tryfrom_string(char const *key, DeeObject *matched_key) {
	if (DeeString_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeString_TryNew
	return DeeString_TryNew(key);
#else /* DeeString_TryNew */
	(void)key;
	return NULL;
#endif /* !DeeString_TryNew */
}

LOCAL WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
matched_keyob_tryfrom_string_len(char const *key, size_t keylen, DeeObject *matched_key) {
	if (DeeString_Check(matched_key))
		return_reference_(matched_key);
#ifdef DeeString_TryNewSized
	return DeeString_TryNewSized(key, keylen);
#else /* DeeString_TryNewSized */
	(void)key;
	(void)keylen;
	return NULL;
#endif /* !DeeString_TryNewSized */
}



/* Dict-specific, internal APIs */
INTDEF NONNULL((1)) void DCALL
dict_optimize_vtab(DeeDictObject *__restrict self);

LOCAL NONNULL((1)) void DCALL
DeeDict_LockReadAndOptimize(DeeDictObject *__restrict self) {
	DeeDict_LockRead(self);
	if (_DeeDict_CanOptimizeVTab(self)) {
		if (DeeDict_LockUpgrade(self) || _DeeDict_CanOptimizeVTab(self))
			dict_optimize_vtab(self);
		DeeDict_LockDowngrade(self);
	}
}




/************************************************************************/
/* RODICT HELPERS                                                       */
/************************************************************************/
#define _RoDict_TryMalloc(sizeof)     ((DREF DeeRoDictObject *)DeeObject_TryMalloc(sizeof))
#define _RoDict_TryCalloc(sizeof)     ((DREF DeeRoDictObject *)DeeObject_TryCalloc(sizeof))
#define _RoDict_TryRealloc(p, sizeof) ((DREF DeeRoDictObject *)DeeObject_TryRealloc(p, sizeof))
#define _RoDict_Malloc(sizeof)        ((DREF DeeRoDictObject *)DeeObject_Malloc(sizeof))
#define _RoDict_Calloc(sizeof)        ((DREF DeeRoDictObject *)DeeObject_Calloc(sizeof))
#define _RoDict_Realloc(p, sizeof)    ((DREF DeeRoDictObject *)DeeObject_Realloc(p, sizeof))
#define _RoDict_Free(p)               DeeObject_Free(p)

#define _RoDict_SizeOf(valloc, hmask) \
	_RoDict_SizeOf3(valloc, hmask, DEE_DICT_HIDXIO_FROMALLOC(valloc))
#define _RoDict_SafeSizeOf(valloc, hmask) \
	_RoDict_SafeSizeOf3(valloc, hmask, DEE_DICT_HIDXIO_FROMALLOC(valloc))
#define _RoDict_SizeOf3(valloc, hmask, hidxio)           \
	(offsetof(DeeRoDictObject, rd_vtab) +                \
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

LOCAL NONNULL((1, 3)) void DCALL
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

LOCAL NONNULL((1, 3)) void DCALL
hmask_memmovedown_and_maybe_downcast(void *dst, shift_t dst_hidxio,
                                     void const *src, shift_t src_hidxio,
                                     size_t n_words) {
	ASSERT(dst_hidxio <= src_hidxio);
	if likely(dst_hidxio == src_hidxio) {
		memmovedown(dst, src, n_words << dst_hidxio);
	} else {
		Dee_dict_gethidx_t src_get = Dee_dict_hidxio[src_hidxio].dhxio_get;
		Dee_dict_sethidx_t dst_set = Dee_dict_hidxio[dst_hidxio].dhxio_set;
		size_t i;
		for (i = 0; i < n_words; ++i) {
			/*virt*/Dee_dict_vidx_t word;
			word = (*src_get)(src, i);
			(*dst_set)(dst, i, word);
		}
	}
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICT_H */
