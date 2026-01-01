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
#include "sort.c"
//#define DEFINE_DeeSeq_SortVector
#define DEFINE_DeeSeq_SortVectorWithKey
//#define DEFINE_DeeSeq_SortGetItemIndexFast
//#define DEFINE_DeeSeq_SortGetItemIndexFastWithKey
//#define DEFINE_DeeSeq_SortTryGetItemIndex
//#define DEFINE_DeeSeq_SortTryGetItemIndexWithKey
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeSeq_SortVector) +                  \
     defined(DEFINE_DeeSeq_SortVectorWithKey) +           \
     defined(DEFINE_DeeSeq_SortGetItemIndexFast) +        \
     defined(DEFINE_DeeSeq_SortGetItemIndexFastWithKey) + \
     defined(DEFINE_DeeSeq_SortTryGetItemIndex) +         \
     defined(DEFINE_DeeSeq_SortTryGetItemIndexWithKey)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeSeq_Sort... */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memcpyc */
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifdef DEFINE_DeeSeq_SortVector
#define LOCAL_DeeSeq_Sort DeeSeq_SortVector
#define LOCAL_HAS_VECTOR
#elif defined(DEFINE_DeeSeq_SortVectorWithKey)
#define LOCAL_DeeSeq_Sort DeeSeq_SortVectorWithKey
#define LOCAL_HAS_VECTOR
#define LOCAL_HAS_KEY
#elif defined(DEFINE_DeeSeq_SortGetItemIndexFast)
#define LOCAL_DeeSeq_Sort DeeSeq_SortGetItemIndexFast
#define LOCAL_HAS_GETITEM
#define LOCAL_HAS_GETITEM_FAST
#elif defined(DEFINE_DeeSeq_SortGetItemIndexFastWithKey)
#define LOCAL_DeeSeq_Sort DeeSeq_SortGetItemIndexFastWithKey
#define LOCAL_HAS_GETITEM
#define LOCAL_HAS_GETITEM_FAST
#define LOCAL_HAS_KEY
#elif defined(DEFINE_DeeSeq_SortTryGetItemIndex)
#define LOCAL_DeeSeq_Sort DeeSeq_SortTryGetItemIndex
#define LOCAL_HAS_GETITEM
#elif defined(DEFINE_DeeSeq_SortTryGetItemIndexWithKey)
#define LOCAL_DeeSeq_Sort DeeSeq_SortTryGetItemIndexWithKey
#define LOCAL_HAS_GETITEM
#define LOCAL_HAS_KEY
#else /* DEFINE_DeeSeq_Sort... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_Sort... */

#define LOCAL_mergesort_impl                PP_CAT2(LOCAL_DeeSeq_Sort, _impl__mergesort_impl)
#define LOCAL_mergesort_impl_with_key_cache PP_CAT2(LOCAL_DeeSeq_Sort, _impl__mergesort_impl_with_key_cache)
#define LOCAL_insertsort_impl               PP_CAT2(LOCAL_DeeSeq_Sort, _impl__insertsort_impl)

#ifdef LOCAL_HAS_KEY
#define LOCAL__PARAM_KEY , DeeObject *key
#define LOCAL__ARG_KEY   , key
#define LOCAL_DeeObject_CmpLoAsBool(lhs, rhs) \
	DeeObject_CmpLoAsBoolWithKey(lhs, rhs, key)
#ifndef DeeObject_CmpLoAsBoolWithKey_DEFINED
#define DeeObject_CmpLoAsBoolWithKey_DEFINED
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_CmpLoAsBoolWithKey(DeeObject *lhs, DeeObject *rhs, DeeObject *key) {
	int result;
	lhs = DeeObject_Call(key, 1, (DeeObject **)&lhs);
	if unlikely(!lhs)
		goto err;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err_lhs;
	result = DeeObject_CmpLoAsBool(lhs, rhs);
	Dee_Decref(rhs);
	Dee_Decref(lhs);
	return result;
err_lhs:
	Dee_Decref(lhs);
err:
	return -1;
}
#endif /* !DeeObject_CmpLoAsBoolWithKey_DEFINED */
#else /* LOCAL_HAS_KEY */
#define LOCAL__PARAM_KEY /* nothing */
#define LOCAL__ARG_KEY   /* nothing */
#define LOCAL_DeeObject_CmpLoAsBool(lhs, rhs) \
	DeeObject_CmpLoAsBool(lhs, rhs)
#endif /* !LOCAL_HAS_KEY */


#ifdef LOCAL_HAS_GETITEM_FAST
#define LOCAL_PARAM_SRC       DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_getitem_index_fast)(DeeObject *__restrict self, size_t index)
#define LOCAL_ARG_SRC(skip)   src, src_start + (skip), src_getitem_index_fast
#define LOCAL_GETITEM(i)      (*src_getitem_index_fast)(src, src_start + (i))
#define LOCAL_GETITEM_UNBOUND NULL
#define LOCAL_GETITEM_ISREF
#elif defined(LOCAL_HAS_GETITEM)
#define LOCAL_PARAM_SRC       DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_trygetitem_index)(DeeObject *__restrict self, size_t index)
#define LOCAL_ARG_SRC(skip)   src, src_start + (skip), src_trygetitem_index
#define LOCAL_GETITEM(i)      (*src_trygetitem_index)(src, src_start + (i))
#define LOCAL_GETITEM_UNBOUND ITER_DONE
#define LOCAL_GETITEM_ERROR   NULL
#define LOCAL_GETITEM_ISREF
#elif defined(LOCAL_HAS_VECTOR)
#define LOCAL_PARAM_SRC     /*inherit(on_success)*/ DREF DeeObject *const *__restrict src
#define LOCAL_ARG_SRC(skip) src + (skip)
#define LOCAL_GETITEM(i)    src[i]
#else /* ... */
#error "Invalid configuration"
#endif /* !... */


#ifdef LOCAL_GETITEM_ISREF
#define LOCAL_IF_GETITEM_ISREF(...) __VA_ARGS__
#else /* LOCAL_GETITEM_ISREF */
#define LOCAL_IF_GETITEM_ISREF(...) /* nothing */
#endif /* !LOCAL_GETITEM_ISREF */

#ifdef LOCAL_GETITEM_UNBOUND
#define LOCAL_DecrefIfNotUnbound(x)       \
	do {                                  \
		if ((x) != LOCAL_GETITEM_UNBOUND) \
			Dee_Decref(x);                \
	}	__WHILE0
#define LOCAL_DecrefvIfNotUnbound(v, s)      \
	do {                                     \
		size_t _i;                           \
		for (_i = 0; _i < (s); ++_i) {       \
			DeeObject *_o = (v)[_i];         \
			if (_o != LOCAL_GETITEM_UNBOUND) \
				Dee_Decref(_o);              \
		}                                    \
	}	__WHILE0
#else /* LOCAL_GETITEM_UNBOUND */
#define LOCAL_DecrefIfNotUnbound(x)     Dee_Decref(x)
#define LOCAL_DecrefvIfNotUnbound(v, s) Dee_Decrefv(v, s)
#endif /* !LOCAL_GETITEM_UNBOUND */


PRIVATE WUNUSED ATTR_OUTS(2, 1) int DCALL
LOCAL_mergesort_impl(size_t objc, DREF DeeObject **__restrict dst,
                     DeeObject **__restrict scratch, /* Scratch buffer area */
                     LOCAL_PARAM_SRC LOCAL__PARAM_KEY) {
	int error;
	switch (objc) {

	case 1: {
		DREF DeeObject *src0;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		dst[0] = src0;
	}	break;

	case 2: { /* Optimization for sorting 2 objects. */
		int src0_lo_src1;
		DREF DeeObject *src0, *src1;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		src1 = LOCAL_GETITEM(1);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src1 == LOCAL_GETITEM_ERROR) {
			LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefIfNotUnbound(src0));
			goto err;
		}
#endif /* LOCAL_GETITEM_ERROR */
#ifdef LOCAL_GETITEM_UNBOUND
		if (src0 == LOCAL_GETITEM_UNBOUND) {
			src0_lo_src1 = src1 == LOCAL_GETITEM_UNBOUND ? 0 : 1;
		} else if (src1 == LOCAL_GETITEM_UNBOUND) {
			src0_lo_src1 = 0;
		} else
#endif /* LOCAL_GETITEM_UNBOUND */
		{
			src0_lo_src1 = LOCAL_DeeObject_CmpLoAsBool(src0, src1);
		}
		if (src0_lo_src1 <= 0) {
			if unlikely(src0_lo_src1 < 0)
				goto err;
			dst[0] = src1; /* Inherit reference */
			dst[1] = src0; /* Inherit reference */
		} else {
			dst[0] = src0; /* Inherit reference */
			dst[1] = src1; /* Inherit reference */
		}
	}	break;

	default: {
		size_t s1, s2;
		DREF DeeObject **iter1;
		DREF DeeObject **iter2;
		s1 = objc / 2;
		s2 = objc - s1;
		error = LOCAL_mergesort_impl(s1, scratch, dst, LOCAL_ARG_SRC(0) LOCAL__ARG_KEY);
		if unlikely(error < 0)
			goto err;
		error = LOCAL_mergesort_impl(s2, scratch + s1, dst + s1, LOCAL_ARG_SRC(s1) LOCAL__ARG_KEY);
		if unlikely(error < 0) {
			LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefvIfNotUnbound(scratch, s1));
			goto err;
		}
		iter1 = scratch;
		iter2 = scratch + s1;
		while (s1 && s2) {
			DREF DeeObject *iter1_value = *iter1;
			DREF DeeObject *iter2_value = *iter2;
#ifdef LOCAL_GETITEM_UNBOUND
			if (iter1_value == LOCAL_GETITEM_UNBOUND) {
				error = iter2_value == LOCAL_GETITEM_UNBOUND ? 0 : 1;
			} else if (iter2_value == LOCAL_GETITEM_UNBOUND) {
				error = 0;
			} else
#endif /* LOCAL_GETITEM_UNBOUND */
			{
				error = LOCAL_DeeObject_CmpLoAsBool(iter1_value, iter2_value);
			}
			if (error <= 0) {
				if unlikely(error < 0) {
					LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefvIfNotUnbound(scratch, objc));
					goto err;
				}
				*dst = iter2_value;
				++iter2;
				--s2;
			} else {
				*dst = iter1_value;
				++iter1;
				--s1;
			}
			++dst;
		}
		if (s1) {
			ASSERT(!s2);
			memcpyc(dst, iter1, s1, sizeof(DREF DeeObject *));
		} else if (s2) {
			memcpyc(dst, iter2, s2, sizeof(DREF DeeObject *));
		}
	}	break;

	}
	return 0;
err:
	return -1;
}

#ifdef LOCAL_HAS_KEY
PRIVATE WUNUSED ATTR_OUTS(2, 1) ATTR_OUTS(3, 1) int DCALL
LOCAL_mergesort_impl_with_key_cache(size_t objc,
                                    DREF DeeObject **__restrict dst,
                                    DREF DeeObject **__restrict dst_keyed,
                                    DeeObject **__restrict scratch,       /* Scratch buffer area */
                                    DeeObject **__restrict scratch_keyed, /* Scratch buffer area */
                                    LOCAL_PARAM_SRC LOCAL__PARAM_KEY) {
	int error;
	switch (objc) {

	case 1: {
		DREF DeeObject *src0;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		dst[0] = src0;
#ifdef LOCAL_GETITEM_UNBOUND
		if (src0 == LOCAL_GETITEM_UNBOUND) {
			dst_keyed[0] = LOCAL_GETITEM_UNBOUND;
		} else
#endif /* LOCAL_GETITEM_UNBOUND */
		{
			dst_keyed[0] = DeeObject_Call(key, 1, &dst[0]);
			if unlikely(!dst_keyed[0]) {
				LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefIfNotUnbound(dst[0]));
				goto err;
			}
		}
	}	break;

	case 2: { /* Optimization for sorting 2 objects. */
		int src0_lo_src1;
		DREF DeeObject *src0, *src1;
		DREF DeeObject *src0_keyed, *src1_keyed;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		src1 = LOCAL_GETITEM(1);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src1 == LOCAL_GETITEM_ERROR)
			goto err_src0;
#endif /* LOCAL_GETITEM_ERROR */
#ifdef LOCAL_GETITEM_UNBOUND
		if (src0 == LOCAL_GETITEM_UNBOUND) {
			src0_keyed = LOCAL_GETITEM_UNBOUND;
			if (src1 == LOCAL_GETITEM_UNBOUND) {
				src0_lo_src1 = 0;
				src1_keyed   = LOCAL_GETITEM_UNBOUND;
			} else {
				src0_lo_src1 = 1;
				src1_keyed = DeeObject_Call(key, 1, &src1);
				if unlikely(!src1_keyed)
					goto err_src0_src1;
			}
		} else if (src1 == LOCAL_GETITEM_UNBOUND) {
			src0_lo_src1 = 0;
			src0_keyed = DeeObject_Call(key, 1, &src0);
			if unlikely(!src0_keyed)
				goto err_src0_src1;
			src1_keyed = LOCAL_GETITEM_UNBOUND;
		} else
#endif /* LOCAL_GETITEM_UNBOUND */
		{
			src0_keyed = DeeObject_Call(key, 1, &src0);
			if unlikely(!src0_keyed)
				goto err_src0_src1;
			src1_keyed = DeeObject_Call(key, 1, &src1);
			if unlikely(!src1_keyed) {
				Dee_Decref(src0_keyed);
err_src0_src1:
				LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefIfNotUnbound(src1));
#ifdef LOCAL_GETITEM_ERROR
err_src0:
#endif /* LOCAL_GETITEM_ERROR */
				LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefIfNotUnbound(src0));
				goto err;
			}
			src0_lo_src1 = DeeObject_CmpLoAsBool(src0_keyed, src1_keyed);
		}
		if (src0_lo_src1 <= 0) {
			if unlikely(src0_lo_src1 < 0)
				goto err;
			dst[0]       = src1;       /* Inherit reference */
			dst[1]       = src0;       /* Inherit reference */
			dst_keyed[0] = src1_keyed; /* Inherit reference */
			dst_keyed[1] = src0_keyed; /* Inherit reference */
		} else {
			dst[0]       = src0;       /* Inherit reference */
			dst[1]       = src1;       /* Inherit reference */
			dst_keyed[0] = src0_keyed; /* Inherit reference */
			dst_keyed[1] = src1_keyed; /* Inherit reference */
		}
	}	break;

	default: {
		size_t s1, s2;
		DREF DeeObject **iter1, **iter1_keyed;
		DREF DeeObject **iter2, **iter2_keyed;
		s1 = objc / 2;
		s2 = objc - s1;
		error = LOCAL_mergesort_impl_with_key_cache(s1,
		                                            scratch, scratch_keyed,
		                                            dst, dst_keyed,
		                                            LOCAL_ARG_SRC(0) LOCAL__ARG_KEY);
		if unlikely(error < 0)
			goto err;
		error = LOCAL_mergesort_impl_with_key_cache(s2,
		                                            scratch + s1, scratch_keyed + s1,
		                                            dst + s1, dst_keyed + s1,
		                                            LOCAL_ARG_SRC(s1) LOCAL__ARG_KEY);
		if unlikely(error < 0) {
			LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefvIfNotUnbound(scratch, s1));
			LOCAL_DecrefvIfNotUnbound(scratch_keyed, s1);
			goto err;
		}
		iter1       = scratch;
		iter1_keyed = scratch_keyed;
		iter2       = scratch + s1;
		iter2_keyed = scratch_keyed + s1;
		while (s1 && s2) {
#ifdef LOCAL_GETITEM_UNBOUND
			if (*iter1_keyed == LOCAL_GETITEM_UNBOUND) {
				error = *iter2_keyed == LOCAL_GETITEM_UNBOUND ? 0 : 1;
			} else if (*iter2_keyed == LOCAL_GETITEM_UNBOUND) {
				error = 0;
			} else
#endif /* LOCAL_GETITEM_UNBOUND */
			{
				error = DeeObject_CmpLoAsBool(*iter1_keyed, *iter2_keyed);
			}
			if (error <= 0) {
				if unlikely(error < 0) {
					LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefvIfNotUnbound(scratch, objc));
					LOCAL_DecrefvIfNotUnbound(scratch_keyed, objc);
					goto err;
				}
				*dst++       = *iter2++;
				*dst_keyed++ = *iter2_keyed++;
				--s2;
			} else {
				*dst++       = *iter1++;
				*dst_keyed++ = *iter1_keyed++;
				--s1;
			}
		}
		if (s1) {
			ASSERT(!s2);
			memcpyc(dst, iter1, s1, sizeof(DREF DeeObject *));
			memcpyc(dst_keyed, iter1_keyed, s1, sizeof(DREF DeeObject *));
		} else if (s2) {
			memcpyc(dst, iter2, s2, sizeof(DREF DeeObject *));
			memcpyc(dst_keyed, iter2_keyed, s2, sizeof(DREF DeeObject *));
		}
	}	break;

	}
	return 0;
err:
	return -1;
}
#endif /* LOCAL_HAS_KEY */


PRIVATE WUNUSED ATTR_OUTS(1, 2) int DCALL
LOCAL_insertsort_impl(DREF DeeObject **__restrict dst, size_t objc,
                      LOCAL_PARAM_SRC LOCAL__PARAM_KEY) {
	int temp;
	size_t i, j;
	for (i = 0; i < objc; ++i) {
		DREF DeeObject *ob = LOCAL_GETITEM(i);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(ob == LOCAL_GETITEM_ERROR)
			goto err_dst_i;
#endif /* LOCAL_GETITEM_ERROR */
		j = 0;
#ifdef LOCAL_GETITEM_UNBOUND
		if (ob != LOCAL_GETITEM_UNBOUND)
#endif /* LOCAL_GETITEM_UNBOUND */
		{
#ifdef LOCAL_HAS_KEY
			DREF DeeObject *keyed_ob = DeeObject_Call(key, 1, &ob);
			if unlikely(!keyed_ob) {
				LOCAL_IF_GETITEM_ISREF(Dee_Decref(ob));
				goto err_dst_i;
			}
#endif /* LOCAL_HAS_KEY */
			for (; j < i; ++j) {
				/* Check if we need to insert the object in this location. */
#ifdef LOCAL_HAS_KEY
				DREF DeeObject *keyed_dst_j;
				keyed_dst_j = DeeObject_Call(key, 1, &dst[j]);
				if unlikely(!keyed_dst_j) {
err_dst_i_keyed_ob:
					Dee_Decref(keyed_ob);
					goto err_dst_i;
				}
				temp = DeeObject_CmpLoAsBool(ob, keyed_dst_j);
				Dee_Decref(keyed_dst_j);
				if unlikely(temp < 0)
					goto err_dst_i_keyed_ob;
#else /* LOCAL_HAS_KEY */
				temp = DeeObject_CmpLoAsBool(ob, dst[j]);
				if unlikely(temp < 0)
					goto err_dst_i;
#endif /* !LOCAL_HAS_KEY */
				if (temp > 0)
					break;
			}
		}
		memmoveupc(&dst[j + 1], &dst[j], i - j, sizeof(DREF DeeObject *));
		dst[j] = ob;
	}
	return 0;
err_dst_i:
	LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefvIfNotUnbound(dst, i));
	return -1;
}

INTERN WUNUSED ATTR_OUTS(2, 1) int DCALL
LOCAL_DeeSeq_Sort(size_t objc, DREF DeeObject **__restrict dst,
                  LOCAL_PARAM_SRC LOCAL__PARAM_KEY) {
	int result = 0;
	switch (objc) {

	case 0:
		break;

	case 1: {
		DREF DeeObject *src0;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		dst[0] = src0;
	}	break;

	case 2: { /* Optimization for sorting 2 objects. */
		int src0_lo_src1;
		DREF DeeObject *src0, *src1;
		src0 = LOCAL_GETITEM(0);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src0 == LOCAL_GETITEM_ERROR)
			goto err;
#endif /* LOCAL_GETITEM_ERROR */
		src1 = LOCAL_GETITEM(1);
#ifdef LOCAL_GETITEM_ERROR
		if unlikely(src1 == LOCAL_GETITEM_ERROR) {
			LOCAL_IF_GETITEM_ISREF(LOCAL_DecrefIfNotUnbound(src0));
			goto err;
		}
#endif /* LOCAL_GETITEM_ERROR */
		src0_lo_src1 = LOCAL_DeeObject_CmpLoAsBool(src0, src1);
		if (src0_lo_src1 <= 0) {
			if unlikely(src0_lo_src1 < 0)
				goto err;
			dst[0] = src1; /* Inherit reference */
			dst[1] = src0; /* Inherit reference */
		} else {
			dst[0] = src0; /* Inherit reference */
			dst[1] = src1; /* Inherit reference */
		}
	}	break;

	default: {
		DREF DeeObject **scratch;
#ifdef LOCAL_HAS_KEY
		scratch = (DeeObject **)Dee_TryMallocc(objc * 3, sizeof(DeeObject *));
		if likely(scratch) {
			result = LOCAL_mergesort_impl_with_key_cache(objc, dst, scratch, scratch + objc, scratch + objc * 2,
			                                             LOCAL_ARG_SRC(0) LOCAL__ARG_KEY);
			if likely(result == 0) /* Drop references still held on cached keys */
				LOCAL_DecrefvIfNotUnbound(scratch, objc);
			Dee_Free(scratch);
		} else
#endif /* LOCAL_HAS_KEY */
		{
			scratch = (DeeObject **)Dee_TryMallocc(objc, sizeof(DeeObject *));
			if likely(scratch) {
				result = LOCAL_mergesort_impl(objc, dst, scratch, LOCAL_ARG_SRC(0) LOCAL__ARG_KEY);
				Dee_Free(scratch);
			} else {
				result = LOCAL_insertsort_impl(dst, objc, LOCAL_ARG_SRC(0) LOCAL__ARG_KEY);
			}
		}
	}	break;
	}
	return result;
err:
	return -1;
}

#undef LOCAL_DecrefIfNotUnbound
#undef LOCAL_DecrefvIfNotUnbound
#undef LOCAL_IF_GETITEM_ISREF
#undef LOCAL__PARAM_KEY
#undef LOCAL__ARG_KEY
#undef LOCAL_DeeObject_CmpLoAsBool
#undef LOCAL_PARAM_SRC
#undef LOCAL_ARG_SRC
#undef LOCAL_GETITEM
#undef LOCAL_GETITEM_UNBOUND
#undef LOCAL_GETITEM_ERROR
#undef LOCAL_GETITEM_ISREF
#undef LOCAL_mergesort_impl
#undef LOCAL_insertsort_impl
#undef LOCAL_DeeSeq_Sort
#undef LOCAL_HAS_VECTOR
#undef LOCAL_HAS_KEY
#undef LOCAL_HAS_GETITEM
#undef LOCAL_HAS_GETITEM_FAST

DECL_END

#undef DEFINE_DeeSeq_SortVector
#undef DEFINE_DeeSeq_SortVectorWithKey
#undef DEFINE_DeeSeq_SortGetItemIndexFast
#undef DEFINE_DeeSeq_SortGetItemIndexFastWithKey
#undef DEFINE_DeeSeq_SortTryGetItemIndex
#undef DEFINE_DeeSeq_SortTryGetItemIndexWithKey
