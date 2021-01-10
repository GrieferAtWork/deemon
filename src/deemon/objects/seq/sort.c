/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SORT_C
#define GUARD_DEEMON_OBJECTS_SEQ_SORT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>

DECL_BEGIN

PRIVATE WUNUSED int DCALL
mergesort_impl(DREF DeeObject **__restrict dst,
               DeeObject **__restrict temp,
               DREF DeeObject *const *__restrict src,
               size_t objc) {
	int error;
	switch (objc) {

	case 0:
		break;
	case 1:
		dst[0] = src[0];
		break;

	case 2:
		error = DeeObject_CompareLo(src[0], src[1]);
		if unlikely(error < 0 &&
			         !DeeError_Catch(&DeeError_TypeError) &&
			         !DeeError_Catch(&DeeError_NotImplemented))
		goto err;
		if (error <= 0) {
			dst[0] = src[1];
			dst[1] = src[0];
		} else {
			dst[0] = src[0];
			dst[1] = src[1];
		}
		break;

	default: {
		size_t s1, s2;
		DeeObject **iter1;
		DeeObject **iter2;
		s1    = objc / 2;
		s2    = objc - s1;
		error = mergesort_impl(temp, dst, src, s1);
		if unlikely(error < 0)
			goto err;
		error = mergesort_impl(temp + s1, dst + s1, src + s1, s2);
		if unlikely(error < 0)
			goto err;
		iter1 = temp;
		iter2 = temp + s1;
		while (s1 && s2) {
			error = DeeObject_CompareLo(*iter1, *iter2);
			if unlikely(error < 0) {
				if (!DeeError_Catch(&DeeError_TypeError) &&
				    !DeeError_Catch(&DeeError_NotImplemented))
					goto err;
			}
			if (error <= 0) {
				*dst++ = *iter2++;
				--s2;
			} else {
				*dst++ = *iter1++;
				--s1;
			}
		}
		if (s1) {
			ASSERT(!s2);
			memcpyc(dst, iter1, s1,
			        sizeof(DREF DeeObject *));
		} else if (s2) {
			memcpyc(dst, iter2, s2,
			        sizeof(DREF DeeObject *));
		}
	}	break;
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
compare_lo(DeeObject *lhs, DeeObject *rhs, DeeObject *key) {
	int result;
	lhs = DeeObject_Call(key, 1, (DeeObject **)&lhs);
	if unlikely(!lhs)
		goto err;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err_lhs;
	result = DeeObject_CompareLo(lhs, rhs);
	Dee_Decref(rhs);
	Dee_Decref(lhs);
	return result;
err_lhs:
	Dee_Decref(lhs);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((5)) int DCALL
mergesort_impl_p(DREF DeeObject **__restrict dst,
                 DeeObject **__restrict temp,
                 DREF DeeObject *const *__restrict src,
                 size_t objc,
                 DeeObject *key) {
	int error;
	switch (objc) {

	case 0:
		break;

	case 1:
		dst[0] = src[0];
		break;

	case 2:
		error = compare_lo(src[0], src[1], key);
		if (error <= 0) {
			if unlikely(error < 0 &&
				         !DeeError_Catch(&DeeError_TypeError) &&
				         !DeeError_Catch(&DeeError_NotImplemented))
			goto err;
			dst[0] = src[1];
			dst[1] = src[0];
		} else {
			dst[0] = src[0];
			dst[1] = src[1];
		}
		break;

	default: {
		size_t s1, s2;
		DeeObject *const *iter1;
		DeeObject *const *iter2;
		s1    = objc / 2;
		s2    = objc - s1;
		error = mergesort_impl_p(temp, dst, src, s1, key);
		if unlikely(error < 0)
			goto err;
		error = mergesort_impl_p(temp + s1, dst + s1, src + s1, s2, key);
		if unlikely(error < 0)
			goto err;
		iter1 = temp;
		iter2 = temp + s1;
		while (s1 && s2) {
			error = compare_lo(*iter1, *iter2, key);
			if (error <= 0) {
				if unlikely(error < 0 &&
					         !DeeError_Catch(&DeeError_TypeError) &&
					         !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
				*dst++ = *iter2++;
				--s2;
			} else {
				*dst++ = *iter1++;
				--s1;
			}
		}
		if (s1) {
			ASSERT(!s2);
			memcpyc(dst, iter1, s1,
			        sizeof(DREF DeeObject *));
		} else if (s2) {
			memcpyc(dst, iter2, s2,
			        sizeof(DREF DeeObject *));
		}
	}	break;
	}
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED int DCALL
insertsort_impl(DREF DeeObject **__restrict dst,
                DREF DeeObject *const *__restrict src,
                size_t objc) {
	int temp;
	size_t i, j;
	for (i = 0; i < objc; ++i) {
		DeeObject *ob = src[i];
		for (j = 0; j < i; ++j) {
			/* Check if we need to insert the object in this location. */
			temp = DeeObject_CompareLo(ob, dst[j]);
			if unlikely(temp < 0 &&
				         !DeeError_Catch(&DeeError_TypeError) &&
				         !DeeError_Catch(&DeeError_NotImplemented))
			goto err;
			if (temp > 0)
				break;
		}
		memmoveupc(&dst[j + 1], &dst[j],
		           i - j, sizeof(DREF DeeObject *));
		dst[j] = ob;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((4)) int DCALL
insertsort_impl_p(DREF DeeObject **__restrict dst,
                  DREF DeeObject *const *__restrict src,
                  size_t objc, DeeObject *key) {
	int temp;
	size_t i, j;
	for (i = 0; i < objc; ++i) {
		DeeObject *src_ob = src[i];
		for (j = 0; j < i; ++j) {
			/* Check if we need to insert the object in this location. */
			temp = compare_lo(src_ob, dst[j], key);
			if unlikely(temp < 0 &&
				         !DeeError_Catch(&DeeError_TypeError) &&
				         !DeeError_Catch(&DeeError_NotImplemented))
			goto err;
			if (temp > 0)
				break;
		}
		memmoveupc(&dst[j + 1], &dst[j],
		           i - j, sizeof(DREF DeeObject *));
		dst[j] = src_ob;
	}
	return 0;
err:
	return -1;
}



INTERN WUNUSED int DCALL
DeeSeq_MergeSort(DREF DeeObject **__restrict dst,
                 DREF DeeObject *const *__restrict src,
                 size_t objc, DeeObject *key) {
	int result = 0;
	ASSERT(dst != src);
	switch (objc) {

	case 0:
		break;

	case 1:
		dst[0] = src[0];
		break;

	case 2: {
		int temp;
		/* Optimization for sorting 2 objects. */
		temp = key
		       ? compare_lo(src[0], src[1], key)
		       : DeeObject_CompareLo(src[0], src[1]);
		if (temp <= 0) {
			if unlikely(temp < 0 &&
			            !DeeError_Catch(&DeeError_TypeError) &&
			            !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			dst[0] = src[1];
			dst[1] = src[0];
		} else {
			dst[0] = src[0];
			dst[1] = src[1];
		}
	}	break;

	default: {
		DeeObject **temp;
		/* Default case: Do an actual merge-sort. */
		temp = (DeeObject **)Dee_TryMalloc(objc * sizeof(DeeObject *));
		if unlikely(!temp) {
			/* Use a fallback sorting function */
			result = key
			         ? insertsort_impl_p(dst, src, objc, key)
			         : insertsort_impl(dst, src, objc);
		} else {
			result = key
			         ? mergesort_impl_p(dst, temp, src, objc, key)
			         : mergesort_impl(dst, temp, src, objc);
			Dee_Free(temp);
		}
	}	break;
	}
	return result;
err:
	return -1;
}



INTERN WUNUSED int DCALL
DeeSeq_InsertionSort(DREF DeeObject **__restrict dst,
                     DREF DeeObject *const *__restrict src,
                     size_t objc, DeeObject *key) {
	int result = 0;
	ASSERT(dst != src);
	switch (objc) {

	case 0:
		break;

	case 1:
		dst[0] = src[0];
		break;

	case 2: {
		int temp;
		/* Optimization for sorting 2 objects. */
		temp = key
		       ? compare_lo(src[0], src[1], key)
		       : DeeObject_CompareLo(src[0], src[1]);
		if unlikely(temp < 0 &&
			         !DeeError_Catch(&DeeError_TypeError) &&
			         !DeeError_Catch(&DeeError_NotImplemented))
		goto err;
		if (temp <= 0) {
			dst[0] = src[0];
			dst[1] = src[1];
		} else {
			dst[0] = src[1];
			dst[1] = src[0];
		}
	}	break;

	default:
		result = key
		         ? insertsort_impl_p(dst, src, objc, key)
		         : insertsort_impl(dst, src, objc);
		break;
	}
	return result;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SORT_C */
