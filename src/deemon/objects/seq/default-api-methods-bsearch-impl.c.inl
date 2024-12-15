/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "default-api-methods.c"
//#define DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
//#define DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex
//#define DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex
//#define DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex
//#define DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex
#define DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex) +            \
     defined(DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex) +     \
     defined(DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex) +        \
     defined(DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex) + \
     defined(DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex) +           \
     defined(DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeSeq_DefaultB... */


DECL_BEGIN

#ifdef DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
#define LOCAL_IS_FIND
#elif defined(DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex)
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex
#define LOCAL_IS_FIND
#define LOCAL_HAS_KEY
#elif defined(DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex)
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex
#define LOCAL_IS_POSITION
#elif defined(DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex)
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex
#define LOCAL_IS_POSITION
#define LOCAL_HAS_KEY
#elif defined(DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex
#define LOCAL_IS_RANGE
#elif defined(DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
#define LOCAL_DeeSeq_DefaultBFind DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex
#define LOCAL_IS_RANGE
#define LOCAL_HAS_KEY
#else /* DEFINE_DeeSeq_DefaultB... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeSeq_DefaultB... */

#if defined(LOCAL_IS_FIND) || defined(LOCAL_IS_POSITION)
#define LOCAL_return_t   size_t
#define LOCAL_return_ERR (size_t)Dee_COMPARE_ERR
#else /* LOCAL_IS_FIND || LOCAL_IS_POSITION */
#define LOCAL_return_t   int
#define LOCAL_return_ERR (-1)
#endif /* !LOCAL_IS_FIND && !LOCAL_IS_POSITION */


#ifndef overflowsafe_mid_DEFINED
#define overflowsafe_mid_DEFINED
LOCAL ATTR_CONST size_t overflowsafe_mid(size_t a, size_t b) {
	size_t result;
	if unlikely(OVERFLOW_UADD(a, b, &result)) {
		size_t a_div2 = a >> 1;
		size_t b_div2 = b >> 1;
		result = (a_div2 + b_div2);
		if ((a & 1) && (b & 1))
			++result;
		return result;
	}
	return result >> 1;
}
#endif /* !overflowsafe_mid_DEFINED */



INTERN WUNUSED LOCAL_return_t DCALL
LOCAL_DeeSeq_DefaultBFind(DeeObject *self, DeeObject *item,
                          size_t start, size_t end
#ifdef LOCAL_HAS_KEY
                          , DeeObject *key
#endif /* LOCAL_HAS_KEY */
#ifdef LOCAL_IS_RANGE
                          , size_t result_range[2]
#endif /* LOCAL_IS_RANGE */
                          ) {
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t selfsize = (*seq->tp_size)(self);
	if unlikely(selfsize == (size_t)-1)
		goto err_item;
	if (end > selfsize)
		end = selfsize;
	if likely(start < end) {
#ifdef LOCAL_HAS_KEY
		item = DeeObject_Call(key, 1, &item);
		if unlikely(!item)
			goto err;
#define WANT_err
#endif /* LOCAL_HAS_KEY */
		do {
			int cmp_result;
			size_t mid = overflowsafe_mid(start, end);
			DREF DeeObject *seq_item = (*seq->tp_trygetitem_index)(self, mid);
			if unlikely(!ITER_ISOK(seq_item)) {
				if unlikely(!seq_item)
					goto err_item;
				cmp_result = 1; /* item > <unbound> */
			} else {
#ifdef LOCAL_HAS_KEY
				cmp_result = DeeObject_CompareKey(item, seq_item, key);
#else /* LOCAL_HAS_KEY */
				cmp_result = DeeObject_Compare(item, seq_item);
#endif /* !LOCAL_HAS_KEY */

				Dee_Decref(seq_item);
				if unlikely(cmp_result == Dee_COMPARE_ERR)
					goto err_item;
			}
			if (cmp_result < 0) {
				end = mid;
			} else if (cmp_result > 0) {
				start = mid + 1;
			} else {
				/* Found it! (at "mid") */

#if defined(LOCAL_IS_FIND) || defined(LOCAL_IS_POSITION)
				ASSERTF(mid != (size_t)-1, "Impossible, because `mid < end', it can't be SIZE_MAX!");
				if unlikely(mid == (size_t)Dee_COMPARE_ERR)
					goto err_item_overflow;
#define WANT_err_item_overflow
#ifdef LOCAL_HAS_KEY
				Dee_Decref(item);
#endif /* LOCAL_HAS_KEY */
				return mid;
#elif defined(LOCAL_IS_RANGE)
				size_t result_range_start = mid;
				size_t result_range_end   = mid + 1;

				/* Widen the result range's lower bound */
				while (result_range_start > start) {
					mid = overflowsafe_mid(start, result_range_start);
					seq_item = (*seq->tp_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
#ifdef LOCAL_HAS_KEY
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
#else /* LOCAL_HAS_KEY */
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
#endif /* !LOCAL_HAS_KEY */
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_start = mid;
					} else {
						/* No longer part of returned range! */
						start = mid + 1;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Widen the result range's upper bound */
				while (result_range_end < end) {
					mid = overflowsafe_mid(result_range_end, end);
					seq_item = (*seq->tp_trygetitem_index)(self, mid);
					if unlikely(!ITER_ISOK(seq_item)) {
						if unlikely(!seq_item)
							goto err_item;
						cmp_result = 1; /* item > <unbound> */
					} else {
#ifdef LOCAL_HAS_KEY
						cmp_result = DeeObject_TryCompareKeyEq(item, seq_item, key);
#else /* LOCAL_HAS_KEY */
						cmp_result = DeeObject_TryCompareEq(item, seq_item);
#endif /* !LOCAL_HAS_KEY */
						Dee_Decref(seq_item);
						if unlikely(cmp_result == Dee_COMPARE_ERR)
							goto err_item;
					}
					if (cmp_result == 0) {
						/* Still part of returned range! */
						result_range_end = mid + 1;
					} else {
						/* No longer part of returned range! */
						end = mid;
					}
					/* Since this runs in O(log(N)), there's no need to check for interrupts! */
				}

				/* Write-back the result range bounds */
				result_range[0] = result_range_start;
				result_range[1] = result_range_end;
#ifdef LOCAL_HAS_KEY
				Dee_Decref(item);
#endif /* LOCAL_HAS_KEY */
				return 0;
#else /* LOCAL_IS_... */
#error "Invalid configuration"
#endif /* !LOCAL_IS_... */
			}

			/* Since this runs in O(log(N)), there's no need to check for interrupts! */
		} while (start < end);

#ifdef LOCAL_HAS_KEY
		Dee_Decref(item);
#endif /* LOCAL_HAS_KEY */
	}
#ifdef LOCAL_IS_FIND
	return (size_t)-1; /* Not found */
#elif defined(LOCAL_IS_POSITION)
	ASSERT(start == end);
	if unlikely(start == (size_t)-1 || start == (size_t)Dee_COMPARE_ERR)
		goto err_item_overflow;
	return start;
#elif defined(LOCAL_IS_RANGE)
	result_range[0] = start;
	result_range[1] = end;
	return 0;
#else /* LOCAL_IS_... */
#error "Invalid configuration"
#endif /* !LOCAL_IS_... */
#ifdef WANT_err_item_overflow
#undef WANT_err_item_overflow
err_item_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
#endif /* WANT_err_item_overflow */
err_item:
#ifdef LOCAL_HAS_KEY
	Dee_Decref(item);
#endif /* LOCAL_HAS_KEY */
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
	return LOCAL_return_ERR;
}

#undef LOCAL_return_t
#undef LOCAL_return_ERR

#undef LOCAL_DeeSeq_DefaultBFind
#undef LOCAL_HAS_KEY
#undef LOCAL_IS_FIND
#undef LOCAL_IS_POSITION
#undef LOCAL_IS_RANGE

DECL_END

#undef DEFINE_DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex
#undef DEFINE_DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex
#undef DEFINE_DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex
#undef DEFINE_DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex
#undef DEFINE_DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex
#undef DEFINE_DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex
