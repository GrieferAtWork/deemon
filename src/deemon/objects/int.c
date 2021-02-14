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
/* NOTE: Deemon's integer object implementation is
 *       heavily based on python's `long' data type.
 *       With that in mind, licensing of deemon's integer
 *       implementation must be GPL-compatible, GPL being
 *       the license that python is restricted by.
 *    >> So to simplify this whole deal: I make no claim of having invented the
 *       way that deemon's (phyton's) arbitrary-length integers are implemented,
 *       with all algorithms found in `int_logic.c' originating from phython
 *       before being adjusted to fit deemon's runtime. */
#ifndef GUARD_DEEMON_OBJECTS_INT_C
#define GUARD_DEEMON_OBJECTS_INT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>

#include <hybrid/atomic.h>
#include <hybrid/bit.h>
#include <hybrid/byteorder.h>
#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "int_logic.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

#if CONFIG_INT_CACHE_MAXCOUNT != 0
#include <deemon/util/rwlock.h>
#endif /* CONFIG_INT_CACHE_MAXCOUNT != 0 */


#ifndef CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
#if defined(__OPTIMIZE_SIZE__) && defined(CONFIG_HAVE_MATH_H)
#define CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS 0
#else /* __OPTIMIZE_SIZE__ && CONFIG_HAVE_MATH_H */
#define CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS 1
#endif /* !__OPTIMIZE_SIZE__ || !CONFIG_HAVE_MATH_H */
#elif (CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS + 0) == 0
#undef CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
#define CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS 0
#endif /* ... */


/* In order to calculate constants at runtime, we need `log()' from <math.h> */
#if !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
#ifdef CONFIG_HAVE_MATH_H
#include <math.h>
#else /* CONFIG_HAVE_MATH_H */
#ifdef __PREPROCESSOR_HAVE_WARNING
#warning "Unsupported feature: `CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS' requires a working <math.h> header"
#else /* __PREPROCESSOR_HAVE_WARNING */
#error "Unsupported feature: `CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS' requires a working <math.h> header"
#endif /* !__PREPROCESSOR_HAVE_WARNING */
#undef CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
#define CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS 0
#endif /* !CONFIG_HAVE_MATH_H */
#endif /* !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS */


DECL_BEGIN

#if CONFIG_INT_CACHE_MAXCOUNT != 0
struct free_int {
	struct free_int *fi_next; /* [0..1] Next free integer. */
};
struct free_int_set {
	struct free_int *fis_head; /* [0..1][lock(fis_lock)] First free integer object. */
	size_t           fis_size; /* [lock(fis_lock)][<= CONFIG_INT_CACHE_MAXSIZE]
	                            * Amount of free integer objects in this set. */
#ifndef CONFIG_NO_THREADS
	rwlock_t         fis_lock; /* Lock for this free integer set. */
#endif  /* !CONFIG_NO_THREADS */
};

PRIVATE struct free_int_set free_ints[CONFIG_INT_CACHE_MAXCOUNT];

INTERN size_t DCALL
intcache_clear(size_t max_clear) {
	size_t i, result = 0;
	struct free_int_set *set;
	for (i = 0; i < COMPILER_LENOF(free_ints); ++i) {
		struct free_int *chain;
		struct free_int *chain_end;
		size_t total_free;
		set = &free_ints[i];
#ifndef CONFIG_NO_THREADS
		while (!rwlock_trywrite(&set->fis_lock)) {
			if (!set->fis_size)
				goto next_set;
			SCHED_YIELD();
		}
#endif  /* !CONFIG_NO_THREADS */
		total_free = set->fis_size * (offsetof(DeeIntObject, ob_digit) +
		                              i * sizeof(digit));
		chain      = set->fis_head;
		if (max_clear >= result + total_free) {
			result += total_free;
			set->fis_size = 0;
			set->fis_head = NULL;
		} else {
			size_t single_item;
			single_item = offsetof(DeeIntObject, ob_digit) + i * sizeof(digit);
			total_free  = max_clear - result;
			total_free += single_item - 1;
			total_free /= single_item;
			ASSERT(total_free < set->fis_size);
			chain_end = chain;
			set->fis_size -= total_free;
			result += total_free * single_item;
			--total_free;
			while (total_free--)
				chain_end = chain_end->fi_next;
			set->fis_head      = chain_end->fi_next;
			chain_end->fi_next = NULL;
		}
		ASSERT((set->fis_head != NULL) == (set->fis_size != 0));
		rwlock_endwrite(&set->fis_lock);
		/* Free all of the extracted chain elements. */
		while (chain) {
			chain_end = chain->fi_next;
			DeeObject_Free(chain);
			chain = chain_end;
		}
		if (result >= max_clear)
			break;
	next_set:;
	}
	return result;
}

INTERN NONNULL((1)) void DCALL
DeeInt_Free(DeeIntObject *__restrict self) {
	size_t n_digits;
	ASSERT(self);
	n_digits = (size_t)self->ob_size;
	if (self->ob_size < 0)
		n_digits = (size_t)-self->ob_size;
	if (n_digits < CONFIG_INT_CACHE_MAXCOUNT) {
		struct free_int_set *set;
		set = &free_ints[n_digits];
#ifndef CONFIG_NO_THREADS
		while (!rwlock_trywrite(&set->fis_lock)) {
			if (ATOMIC_READ(set->fis_size) >= CONFIG_INT_CACHE_MAXSIZE)
				goto do_free;
			SCHED_YIELD();
		}
#endif  /* !CONFIG_NO_THREADS */
		COMPILER_READ_BARRIER();
		if (set->fis_size < CONFIG_INT_CACHE_MAXSIZE) {
			((struct free_int *)self)->fi_next = set->fis_head;
			set->fis_head                      = (struct free_int *)self;
			++set->fis_size;
			rwlock_endwrite(&set->fis_lock);
			return;
		}
		rwlock_endwrite(&set->fis_lock);
	}
do_free:
	DeeObject_Free(self);
}

INTERN WUNUSED DREF DeeIntObject *DCALL
#ifdef NDEBUG
DeeInt_Alloc(size_t n_digits)
#else /* NDEBUG */
DeeInt_Alloc_dbg(size_t n_digits, char const *file, int line)
#endif /* !NDEBUG */
{
	DREF DeeIntObject *result;
	/* Search the cache for free integers. */
	if (n_digits < CONFIG_INT_CACHE_MAXCOUNT) {
		struct free_int_set *set;
		set = &free_ints[n_digits];
#ifndef CONFIG_NO_THREADS
		while (!rwlock_trywrite(&set->fis_lock)) {
			if (!set->fis_size)
				goto do_alloc;
			SCHED_YIELD();
		}
#endif  /* !CONFIG_NO_THREADS */
		ASSERT((set->fis_size != 0) == (set->fis_head != NULL));
		if (set->fis_size) {
			result        = (DREF DeeIntObject *)set->fis_head;
			set->fis_head = ((struct free_int *)result)->fi_next;
			--set->fis_size;
			ASSERT((set->fis_size != 0) == (set->fis_head != NULL));
			rwlock_endwrite(&set->fis_lock);
			goto init_result;
		}
		rwlock_endwrite(&set->fis_lock);
	}
do_alloc:
#ifdef NDEBUG
	result = (DREF DeeIntObject *)DeeObject_Malloc(offsetof(DeeIntObject, ob_digit) +
	                                               n_digits * sizeof(digit));
#else /* NDEBUG */
	result = (DREF DeeIntObject *)DeeDbgObject_Malloc(offsetof(DeeIntObject, ob_digit) +
	                                                  n_digits * sizeof(digit),
	                                                  file,
	                                                  line);
#endif /* !NDEBUG */
	if unlikely(!result)
		goto done;
init_result:
	DeeObject_Init(result, &DeeInt_Type);
	result->ob_size = n_digits;
done:
	return result;
}

#else /* CONFIG_INT_CACHE_MAXCOUNT != 0 */

INTERN size_t DCALL
intcache_clear(size_t UNUSED(max_clear)) {
	return 0;
}


INTERN WUNUSED DREF DeeIntObject *DCALL
#ifdef NDEBUG
DeeInt_Alloc(size_t n_digits)
#else /* NDEBUG */
DeeInt_Alloc_dbg(size_t n_digits, char const *file, int line)
#endif /* !NDEBUG */
{
	DREF DeeIntObject *result;
#ifdef NDEBUG
	result = (DREF DeeIntObject *)DeeObject_Malloc(offsetof(DeeIntObject, ob_digit) +
	                                               n_digits * sizeof(digit));
#else /* NDEBUG */
	result = (DREF DeeIntObject *)DeeDbgObject_Malloc(offsetof(DeeIntObject, ob_digit) +
	                                                  n_digits * sizeof(digit),
	                                                  file,
	                                                  line);
#endif /* !NDEBUG */
	if (result) {
		DeeObject_Init(result, &DeeInt_Type);
		result->ob_size = n_digits;
	}
	return result;
}
#endif /* CONFIG_INT_CACHE_MAXCOUNT == 0 */

/* Create an integer from signed/unsigned LEB data. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_NewSleb(uint8_t **__restrict preader) {
	DREF DeeIntObject *result;
	digit *dst;
	twodigits temp;
	uint8_t num_bits;
	uint8_t *reader   = *preader;
	size_t num_digits = 1;
	/* Figure out a worst-case for how many digits we'll be needing. */
	while (*reader++ & 0x80)
		++num_digits;
	num_digits = ((num_digits * 7 + (DIGIT_BITS - 1)) / DIGIT_BITS);
	result     = DeeInt_Alloc(num_digits);
	if unlikely(!result)
		goto done;
	/* Read the integer. */
	reader   = *preader;
	dst      = result->ob_digit;
	num_bits = 6;
	temp     = *reader++ & 0x3f;
	for (;;) {
		while (num_bits < DIGIT_BITS &&
		       (reader[-1] & 0x80)) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			if (!(*reader++ & 0x80))
				break;
		}
		if (num_bits >= DIGIT_BITS) {
			*dst++ = temp & DIGIT_MASK;
			num_bits -= DIGIT_BITS;
			temp >>= DIGIT_BITS;
		} else {
			if (!num_bits) {
				if (dst == result->ob_digit) {
					/* Special case: INT(0) */
					DeeInt_Free(result);
					result = &DeeInt_Zero;
					Dee_Incref(result);
					goto done2;
				}
				/* Simple case: unused. */
				break;
			}
			/* Less than one digit. */
			*dst = (digit)temp;
			if (*dst)
				++dst;
			break;
		}
	}
	result->ob_size = (size_t)(dst - result->ob_digit);
	/* Check the sign bit. */
	if (**preader & 0x40)
		result->ob_size = -result->ob_size;
done2:
	/* Save the new read position. */
	*preader = reader;
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_NewUleb(uint8_t **__restrict preader) {
	DREF DeeIntObject *result;
	digit *dst;
	twodigits temp;
	uint8_t num_bits;
	uint8_t *reader   = *preader;
	size_t num_digits = 1;
	/* Figure out a worst-case for how many digits we'll be needing. */
	while (*reader++ & 0x80)
		++num_digits;
	num_digits = ((num_digits * 7 + (DIGIT_BITS - 1)) / DIGIT_BITS);
	result     = DeeInt_Alloc(num_digits);
	if unlikely(!result)
		goto done;
	/* Read the integer. */
	reader   = *preader;
	num_bits = 0, temp = 0;
	dst = result->ob_digit;
	for (;;) {
		while (num_bits < DIGIT_BITS &&
		       (reader == *preader || (reader[-1] & 0x80))) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			if (!(*reader++ & 0x80)) {
				while (num_bits &&
				       !((temp >> (num_bits - 1))&1))
					--num_bits;
				break;
			}
		}
		if (num_bits >= DIGIT_BITS) {
			*dst++ = temp & DIGIT_MASK;
			num_bits -= DIGIT_BITS;
			temp >>= DIGIT_BITS;
		} else {
			if (!num_bits) {
				if (dst == result->ob_digit) {
					/* Special case: INT(0) */
					DeeInt_Free(result);
					result = &DeeInt_Zero;
					Dee_Incref(result);
					goto done2;
				}
				/* Simple case: unused. */
				break;
			}
			/* Less than one digit. */
			*dst = (digit)temp;
			if (*dst)
				++dst;
			break;
		}
	}
	result->ob_size = (size_t)(dst - result->ob_digit);
done2:
	/* Save the new read position. */
	*preader = reader;
done:
	return (DREF DeeObject *)result;
}


PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
DeeInt_GetSleb(DeeObject *__restrict self,
               uint8_t *__restrict writer) {
	twodigits temp;
	uint8_t num_bits;
	uint8_t *dst = writer;
	digit *src, *end;
	size_t size;
	DeeIntObject *me = (DeeIntObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	size = (size_t)me->ob_size;
	src  = me->ob_digit;
	if ((dssize_t)size < 0) {
		/* Negative integer. */
		size = (size_t) - (dssize_t)size;
		/* Special handling for writing the first byte. */
		end      = src + size;
		temp     = *src++;
		num_bits = DIGIT_BITS;
		if (src == end) {
			/* Truncate zero-bits from the most significant digit. */
			while (num_bits &&
			       !((temp >> (num_bits - 1))&1))
				--num_bits;
			if (num_bits <= 6) {
				*dst++ = 0x40 | (uint8_t)temp;
				goto done;
			}
		}
		*dst++ = 0x80 | 0x40 | (temp & 0x3f);
		num_bits -= 6;
		temp >>= 6;
	} else {
		temp = 0, num_bits = 0;
		end = src + size;
	}
	for (;;) {
		if (src != end && num_bits < 7) {
			/* Read one more digit. */
			temp |= (twodigits)*src++ << num_bits;
			num_bits += DIGIT_BITS;
			if (src == end) {
				/* Truncate zero-bits from the most significant digit. */
				while (num_bits &&
				       !((temp >> (num_bits - 1))&1))
					--num_bits;
			}
		}
		if (num_bits >= 7) {
			/* Keep on writing digits into the buffer. */
			do {
				*dst++ = 0x80 | (temp & 0x7f);
				temp >>= 7;
				num_bits -= 7;
			} while (num_bits >= 7);
		} else {
			/* Last part. */
			if (!num_bits) {
				if (dst == writer)
					*dst++ = 0;
				/* Clear the continue-bit in the last LEB digit. */
				dst[-1] &= 0x7f;
				break;
			}
			ASSERT(!(temp & 0x80));
			*dst++ = (uint8_t)temp;
			break;
		}
	}
done:
	return dst;
}

PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
DeeInt_GetUleb(DeeObject *__restrict self,
               uint8_t *__restrict writer) {
	twodigits temp;
	uint8_t num_bits;
	uint8_t *dst = writer;
	digit *src, *end;
	DeeIntObject *me = (DeeIntObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	ASSERT(me->ob_size >= 0);
	temp = 0, num_bits = 0;
	src = me->ob_digit;
	end = src + (size_t)me->ob_size;
	for (;;) {
		if (src != end && num_bits < 7) {
			/* Read one more digit. */
			temp |= (twodigits)*src++ << num_bits;
			num_bits += DIGIT_BITS;
			if (src == end) {
				/* Truncate zero-bits from the most significant digit. */
				while (num_bits &&
				       !((temp >> (num_bits - 1))&1))
					--num_bits;
			}
		}
		if (num_bits >= 7) {
			/* Keep on writing digits into the buffer. */
			do {
				*dst++ = 0x80 | (temp & 0x7f);
				temp >>= 7;
				num_bits -= 7;
			} while (num_bits >= 7);
		} else {
			/* Last part. */
			if (!num_bits) {
				if (dst == writer)
					*dst++ = 0;
				/* Clear the continue-bit in the last LEB digit. */
				dst[-1] &= 0x7f;
				break;
			}
			ASSERT(!(temp & 0x80));
			*dst++ = (uint8_t)temp;
			break;
		}
	}
/*done:*/
	return dst;
}


#if DIGIT_BITS < 16
PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewS8(int8_t val) {
	DREF DeeIntObject *result;
	int sign        = 1;
	uint8_t abs_val = (uint8_t)val;
	if (val <= 0) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		sign    = -1;
		abs_val = (uint8_t)0 - (uint8_t)val;
	}
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = sign;
		result->ob_digit[0] = (digit)abs_val;
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewU8(uint8_t val) {
	DREF DeeIntObject *result;
	if (!val)
		return_reference_((DeeObject *)&DeeInt_Zero);
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = 1;
		result->ob_digit[0] = (digit)val;
	}
	return (DREF DeeObject *)result;
}
#endif /* DIGIT_BITS < 16 */


PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewU16(uint16_t val) {
	DREF DeeIntObject *result;
#if DIGIT_BITS >= 16
	if (!val)
		return_reference_((DeeObject *)&DeeInt_Zero);
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = 1;
		result->ob_digit[0] = (digit)val;
	}
#elif DIGIT_BITS >= 8
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = 1;
			result->ob_digit[0] = (digit)val;
		}
	} else {
		result = DeeInt_Alloc(2);
		if likely(result) {
			result->ob_size     = 2;
			result->ob_digit[0] = val & DIGIT_MASK;
			result->ob_digit[1] = val >> DIGIT_BITS;
		}
	}
#else
#error "Not implemented"
#endif
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewU32(uint32_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	uint32_t iter;
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = 1;
			result->ob_digit[0] = (digit)val;
		}
	} else {
		for (iter = val, req_digits = 0; iter;
		     iter >>= DIGIT_BITS, ++req_digits)
			;
		ASSERT(req_digits > 0);
		result = DeeInt_Alloc(req_digits);
		if likely(result) {
			result->ob_size = req_digits;
			for (req_digits = 0; val;
			     val >>= DIGIT_BITS, ++req_digits)
				result->ob_digit[req_digits] = val & DIGIT_MASK;
		}
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewU64(uint64_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	uint64_t iter;
#if __SIZEOF_POINTER__ < 8
	/* When the CPU wasn't designed for 64-bit
	 * integers, prefer using 32-bit path. */
	if (val <= UINT32_MAX)
		return DeeInt_NewU32((uint32_t)val);
#endif
		/* NOTE: 32 == Bits required to display everything in the range 0..UINT32_MAX */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = 1;
			result->ob_digit[0] = (digit)val;
		}
	} else
#endif
	{
		for (iter = val, req_digits = 0; iter;
		     iter >>= DIGIT_BITS, ++req_digits)
			;
		ASSERT(req_digits > 0);
		result = DeeInt_Alloc(req_digits);
		if likely(result) {
			result->ob_size = req_digits;
			for (req_digits = 0; val;
			     val >>= DIGIT_BITS, ++req_digits)
				result->ob_digit[req_digits] = val & DIGIT_MASK;
		}
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewS16(int16_t val) {
	DREF DeeIntObject *result;
	int sign         = 1;
	uint16_t abs_val = (uint16_t)val;
#if DIGIT_BITS >= 16
	if (val <= 0) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		sign    = -1;
		abs_val = (uint16_t)0 - (uint16_t)val;
	}
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = sign;
		result->ob_digit[0] = (digit)abs_val;
	}
#elif DIGIT_BITS >= 8
	if (val < 0) {
		sign    = -1;
		abs_val = (uint16_t)0 - (uint16_t)val;
	}
	if (!(abs_val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = sign;
			result->ob_digit[0] = (digit)abs_val;
		}
	} else {
		result = DeeInt_Alloc(2);
		if likely(result) {
			result->ob_size     = 2 * sign;
			result->ob_digit[0] = abs_val & DIGIT_MASK;
			result->ob_digit[1] = abs_val >> DIGIT_BITS;
		}
	}
#else
#error "Not implemented"
#endif
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewS32(int32_t val) {
	DREF DeeIntObject *result;
	int sign = 1;
	size_t req_digits;
	uint32_t iter, abs_val = (uint32_t)val;
	if (val < 0) {
		sign    = -1;
		abs_val = (uint32_t)0 - (uint32_t)val;
	}
	if (!(abs_val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = sign;
			result->ob_digit[0] = (digit)abs_val;
		}
	} else {
		for (iter = abs_val, req_digits = 0; iter;
		     iter >>= DIGIT_BITS, ++req_digits)
			;
		ASSERT(req_digits > 0);
		result = DeeInt_Alloc(req_digits);
		if likely(result) {
			result->ob_size = req_digits * sign;
			for (req_digits = 0; abs_val;
			     abs_val >>= DIGIT_BITS, ++req_digits)
				result->ob_digit[req_digits] = abs_val & DIGIT_MASK;
		}
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL DeeInt_NewS64(int64_t val) {
	DREF DeeIntObject *result;
	int sign;
	size_t req_digits;
	uint64_t iter, abs_val;
#if __SIZEOF_POINTER__ < 8
	/* When the CPU wasn't designed for 64-bit
	 * integers, prefer using 32-bit path. */
	if (val >= INT32_MIN && val <= INT32_MAX)
		return DeeInt_NewS32((int32_t)val);
#endif /* __SIZEOF_POINTER__ < 8 */
	sign = 1, abs_val = (uint64_t)val;
	if (val < 0) {
		sign    = -1;
		abs_val = (uint64_t)0 - (uint64_t)val;
	}
	/* NOTE: 32 == Bits required to display everything in the range 0..MAX(-INT32_MIN, INT32_MAX) */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
	if (!(abs_val >> DIGIT_BITS)) {
		if (!val)
			return_reference_((DeeObject *)&DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = sign;
			result->ob_digit[0] = (digit)abs_val;
		}
	} else
#endif /* __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32 */
	{
		for (iter = abs_val, req_digits = 0; iter;
		     iter >>= DIGIT_BITS, ++req_digits)
			;
		ASSERT(req_digits > 0);
		result = DeeInt_Alloc(req_digits);
		if likely(result) {
			result->ob_size = req_digits * sign;
			for (req_digits = 0; abs_val;
			     abs_val >>= DIGIT_BITS, ++req_digits)
				result->ob_digit[req_digits] = abs_val & DIGIT_MASK;
		}
	}
	return (DREF DeeObject *)result;
}


/* 128-bit integer creation. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewU128(duint128_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	duint128_t iter;
	/* Simplification: When it fits into a 64-bit integer, use that path! */
	if (DUINT128_IS64(val))
		return DeeInt_NewU64(DUINT128_GET64(val)[DEE_INT128_LS64]);
	/* The remainder is basically the same as any other creator, but
	 * using special macros implementing some basic 128-bit arithmetic. */
	for (iter = val, req_digits = 0; !DSINT128_ISNUL(iter);
	     DUINT128_SHR(iter, DIGIT_BITS), ++req_digits)
		;
	ASSERT(req_digits > 0);
	result = DeeInt_Alloc(req_digits);
	if likely(result) {
		result->ob_size = req_digits;
		for (req_digits = 0; !DSINT128_ISNUL(val);
		     DUINT128_SHR(val, DIGIT_BITS), ++req_digits) {
#if DIGIT_BITS == 30
			result->ob_digit[req_digits] = (digit)(DUINT128_GET32(val)[DEE_INT128_LS32] & DIGIT_MASK);
#else /* DIGIT_BITS == 30 */
			result->ob_digit[req_digits] = (digit)(DUINT128_GET16(val)[DEE_INT128_LS16] & DIGIT_MASK);
#endif /* DIGIT_BITS != 30 */
		}
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewS128(dint128_t val) {
	DREF DeeIntObject *result;
	int sign;
	size_t req_digits;
	union {
		dint128_t  s;
		duint128_t u;
	} iter, abs_val;
	if (DSINT128_IS64(val))
		return DeeInt_NewS64(DUINT128_GETS64(val)[DEE_INT128_LS64]);
	/* The remainder is basically the same as any other creator, but
	 * using special macros implementing some basic 128-bit arithmetic. */
	sign = 1;
	abs_val.s = val;
	if (DSINT128_ISNEG(val)) {
		sign = -1;
		DSINT128_TONEG(abs_val.s);
	}
	for (iter = abs_val, req_digits = 0; !DSINT128_ISNUL(iter.s);
	     DUINT128_SHR(iter.s, DIGIT_BITS), ++req_digits)
		;
	ASSERT(req_digits > 0);
	result = DeeInt_Alloc(req_digits);
	if likely(result) {
		result->ob_size = req_digits * sign;
		for (req_digits = 0; !DSINT128_ISNUL(abs_val.s);
		     DUINT128_SHR(abs_val.u, DIGIT_BITS), ++req_digits) {
#if DIGIT_BITS == 30
			result->ob_digit[req_digits] = (digit)(DUINT128_GET32(abs_val.u)[DEE_INT128_LS32] & DIGIT_MASK);
#else /* DIGIT_BITS == 30 */
			result->ob_digit[req_digits] = (digit)(DUINT128_GET16(abs_val.u)[DEE_INT128_LS16] & DIGIT_MASK);
#endif /* DIGIT_BITS != 30 */
		}
	}
	return (DREF DeeObject *)result;
}


#if DIGIT_BITS == 30
#define DeeInt_DECIMAL_SHIFT   9                 /* max(e such that 10**e fits in a digit) */
#define DeeInt_DECIMAL_BASE  ((digit)1000000000) /* 10 ** DECIMAL_SHIFT */
#else /* DIGIT_BITS == 30 */
#define DeeInt_DECIMAL_SHIFT   4            /* max(e such that 10**e fits in a digit) */
#define DeeInt_DECIMAL_BASE  ((digit)10000) /* 10 ** DECIMAL_SHIFT */
#endif /* DIGIT_BITS != 30 */



#define log_base_BASE    (log_base_BASE_ - 2)
#define convwidth_base   (convwidth_base_ - 2)
#define convmultmax_base (convmultmax_base_ - 2)
#if !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
PRIVATE double log_base_BASE_[35] = { 0.0e0 };
PRIVATE int convwidth_base_[35] = { 0 };
PRIVATE twodigits convmultmax_base_[35] = { 0 };
#else /* !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS */
PRIVATE double const log_base_BASE_[35] = {
#if DIGIT_BITS == 30
	0.033333333333333333, 0.052832083357371877,
	0.066666666666666666, 0.077397603162912082,
	0.086165416690705210, 0.093578497401920133,
	0.099999999999999992, 0.10566416671474375,
	0.11073093649624542, 0.11531438728790992,
	0.11949875002403855, 0.12334799060470308,
	0.12691183073525347, 0.13022968652028397,
	0.13333333333333333, 0.13624876137501132,
	0.13899750004807707, 0.14159758378145285,
	0.14406426982957873, 0.14641058075929203,
	0.14864772062124326, 0.15078539853523376,
	0.15283208335737189, 0.15479520632582416,
	0.15668132393803641, 0.15849625007211562,
	0.16024516406858680, 0.16193269983758574,
	0.16356301985361729, 0.16513987701289584,
	0.16666666666666669, 0.16814647064528179,
	0.16958209470834465, 0.17097610056483223,
	0.17233083338141042,
#elif DIGIT_BITS == 15
	0.066666666666666666, 0.10566416671474375,
	0.13333333333333333, 0.15479520632582416,
	0.17233083338141042, 0.18715699480384027,
	0.19999999999999998, 0.21132833342948751,
	0.22146187299249084, 0.23062877457581984,
	0.23899750004807710, 0.24669598120940617,
	0.25382366147050694, 0.26045937304056793,
	0.26666666666666666, 0.27249752275002265,
	0.27799500009615413, 0.28319516756290569,
	0.28812853965915747, 0.29282116151858406,
	0.29729544124248652, 0.30157079707046752,
	0.30566416671474378, 0.30959041265164833,
	0.31336264787607282, 0.31699250014423125,
	0.32049032813717360, 0.32386539967517147,
	0.32712603970723458, 0.33027975402579168,
	0.33333333333333337, 0.33629294129056359,
	0.33916418941668930, 0.34195220112966446,
	0.34466166676282084
#else
#error "Unsupported `DIGIT_BITS'"
#endif
};

PRIVATE int const convwidth_base_[35] = {
#if DIGIT_BITS == 30
	30, 18, 15, 12, 11, 10, 10, 9, 9, 8, 8, 8,
	7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 5, 5, 5, 5
#elif DIGIT_BITS == 15
	15, 9, 7, 6, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 2, 2, 2, 2
#else
#error "Unsupported `DIGIT_BITS'"
#endif
};

PRIVATE digit const convmultmax_base_[35] = {
#if DIGIT_BITS == 30
	0x40000000, 0x17179149, 0x40000000, 0x0e8d4a51, 0x159fd800,
	0x10d63af1, 0x40000000, 0x17179149, 0x3b9aca00, 0x0cc6db61,
	0x19a10000, 0x309f1021, 0x06487b80, 0x0a2f1b6f, 0x10000000,
	0x18754571, 0x247dbc80, 0x3547667b, 0x03d09000, 0x051cafe9,
	0x06c20a40, 0x08d2d931, 0x0b640000, 0x0e8d4a51, 0x1269ae40,
	0x17179149, 0x1cb91000, 0x23744899, 0x2b73a840, 0x34e63b41,
	0x40000000, 0x025528a1, 0x02b54a20, 0x03216b93, 0x039aa400
#elif DIGIT_BITS == 15
	0x8000, 0x4ce3, 0x4000, 0x3d09, 0x1e60,
	0x41a7, 0x8000, 0x19a1, 0x2710, 0x3931,
	0x5100, 0x6f91, 0x0ab8, 0x0d2f, 0x1000,
	0x1331, 0x16c8, 0x1acb, 0x1f40, 0x242d,
	0x2998, 0x2f87, 0x3600, 0x3d09, 0x44a8,
	0x4ce3, 0x55c0, 0x5f45, 0x6978, 0x745f,
	0x8000, 0x0441, 0x0484, 0x04c9, 0x0510
#else
#error "Unsupported `DIGIT_BITS'"
#endif
};
#endif /* CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS */

LOCAL WUNUSED DREF DeeIntObject *DCALL
int_from_nonbinary_string(char *__restrict begin,
                          char *__restrict end,
                          unsigned int radix,
                          uint32_t radix_and_flags) {
	/* !!!DISCLAIMER!!! This function was originally taken from python,
	 *                  but has been heavily modified since. */
	DREF DeeIntObject *result;
	twodigits convmultmax, convmult, c;
	size_t size_z;
	digit *pz, *pzstop;
	int i, convwidth;
	ASSERT(radix >= 2);
	ASSERT(radix <= 36);
#if !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS
	if (convwidth_base[radix] == 0) {
		twodigits convmax    = radix;
		log_base_BASE[radix] = (log((double)radix) /
		                        log((double)DIGIT_BASE));
		for (i = 1;;) {
			twodigits next = convmax * radix;
			if (next > DIGIT_BASE)
				break;
			convmax = next;
			++i;
		}
		convmultmax_base[radix] = convmax;
		ASSERT(i > 0);
		convwidth_base[radix] = i;
	}
#endif /* !CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS */
	size_z = (size_t)((end - begin) * log_base_BASE[radix]) + 1;
	result = DeeInt_Alloc(size_z);
	if (result == NULL)
		return NULL;
	result->ob_size = 0;
	convwidth   = convwidth_base[radix];
	convmultmax = convmultmax_base[radix];
	while (begin < end) {
#if 1
		c = 0;
		for (i = 0; i < convwidth && begin < end; ++i, ++begin) {
			digit dig;
			char ch;
parse_ch:
			ch = *begin;
			if (ch >= '0' && ch <= '9')
				dig = ch - '0';
			else if (ch >= 'a' && ch <= 'z')
				dig = 10 + (ch - 'a');
			else if (ch >= 'A' && ch <= 'Z')
				dig = 10 + (ch - 'A');
			else if ((unsigned char)ch >= 0x80) {
				/* Unicode character? */
				uint32_t uni;
				struct unitraits *traits;
				uni    = utf8_readchar((char const **)&begin, end);
				traits = DeeUni_Descriptor(uni);
				/* All any kind of digit/decimal character. - If the caller doesn't
				 * want to support any kind of digit, have `int("²")' evaluate to 2,
				 * then they have to verify that the string only contains ~conventional~
				 * decimals by using `string.isdecimal()'. As far as this check is
				 * concerned, we accept anything that applies to `string.isnumeric()' */
				if (traits->ut_flags & (Dee_UNICODE_FDIGIT | Dee_UNICODE_FDECIMAL))
					dig = traits->ut_digit;
				else if (uni >= 'a' && uni <= 'z')
					dig = 10 + ((uint8_t)uni - (uint8_t)'a');
				else if (uni >= 'A' && uni <= 'Z')
					dig = 10 + ((uint8_t)uni - (uint8_t)'A');
				else {
					if (uni != '\\')
						goto invalid_r;
					goto handle_backslash_in_text;
				}
				--begin; /* Account for the additional `++begin' inside of for-advance */
			} else if (ch != '\\') {
				goto invalid_r;
			} else {
handle_backslash_in_text:
				if (!(radix_and_flags & DEEINT_STRING_FESCAPED))
					goto invalid_r;
				if (begin >= end - 2)
					goto invalid_r;
				if (begin[1] == '\n') {
					begin += 2;
					goto parse_ch;
				}
				if (begin[1] == '\r') {
					begin += 2;
					if (begin != end && *begin == '\n')
						++begin;
					goto parse_ch;
				}
				goto invalid_r;
			}
			if unlikely(dig >= radix)
				goto invalid_r;
			c = (twodigits)(c * radix + dig);
			ASSERT(c < DIGIT_BASE);
		}
#else
		c = (digit)_PyLong_DigitValue[Py_CHARMASK(*begin++)];
		for (i = 1; i < convwidth && begin != end; ++i, ++begin) {
			c = (twodigits)(c * radix + (int)_PyLong_DigitValue[Py_CHARMASK(*begin)]);
			ASSERT(c < DIGIT_BASE);
		}
#endif
		convmult = convmultmax;
		if (i != convwidth) {
			convmult = radix;
			for (; i > 1; --i)
				convmult *= radix;
		}
		pz     = result->ob_digit;
		pzstop = pz + (size_t)result->ob_size;
		for (; pz < pzstop; ++pz) {
			c += (twodigits)*pz * convmult;
			*pz = (digit)(c & DIGIT_MASK);
			c >>= DIGIT_BITS;
		}
		if (c) {
			ASSERT(c < DIGIT_BASE);
			if likely((size_t)result->ob_size < size_z) {
				*pz = (digit)c;
				++result->ob_size;
			} else {
				DREF DeeIntObject *tmp;
				ASSERT((size_t)result->ob_size == size_z);
				tmp = DeeInt_Alloc(size_z + 1);
				if (tmp == NULL) {
					Dee_DecrefDokill(result);
					return NULL;
				}
				memcpyc(tmp->ob_digit,
				        result->ob_digit,
				        size_z,
				        sizeof(digit));
				Dee_DecrefDokill(result);
				result                   = tmp;
				result->ob_digit[size_z] = (digit)c;
				++size_z;
			}
		}
	}
	return result;
invalid_r:
	Dee_DecrefDokill(result);
	return (DREF DeeIntObject *)ITER_DONE;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_FromString(/*utf-8*/ char const *__restrict str,
                  size_t len, uint32_t radix_and_flags) {
	unsigned int radix = radix_and_flags >> DEEINT_STRING_RSHIFT;
	bool negative      = false;
	DREF DeeIntObject *result;
	char *iter, *begin = (char *)str, *end = (char *)str + len;
	digit *dst;
	twodigits number;
	uint8_t num_bits;
	uint8_t bits_per_digit;
	/* Parse a sign prefix. */
	for (;; ++begin) {
		if (begin == end)
			goto invalid;
		if (*begin == '+')
			continue;
		if (*begin == '-') {
			negative = !negative;
			continue;
		}
		if (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
			uint32_t begin_plus_one;
			char *new_begin = begin + 1;
			begin_plus_one  = utf8_readchar((char const **)&new_begin, end);
			if (DeeUni_IsLF(begin_plus_one)) {
				begin = new_begin;
				if (begin_plus_one == '\r' &&
				    *begin == '\n')
					++begin;
				continue;
			}
		}
		break;
	}
	if (!radix) {
		/* Automatically determine the radix. */
		char *old_begin = begin;
		uint32_t leading_zero;
		leading_zero = utf8_readchar((char const **)&begin, end);
		if (DeeUni_IsDecimalX(leading_zero, 0)) {
			if (begin == end) /* Special case: int(0) */
				return_reference_((DeeObject *)&DeeInt_Zero);
			while (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				uint32_t begin_plus_one;
				char *new_begin = begin + 1;
				begin_plus_one  = utf8_readchar((char const **)&new_begin, end);
				if (DeeUni_IsLF(begin_plus_one)) {
					begin = new_begin;
					if (begin_plus_one == '\r' &&
					    *begin == '\n')
						++begin;
					continue;
				}
				break;
			}
			if (*begin == 'x' || *begin == 'X')
				radix = 16, ++begin;
			else if (*begin == 'b' || *begin == 'B')
				radix = 2, ++begin;
			else {
				radix = 8;
			}
		} else {
			begin = old_begin;
			radix = 10;
		}
	}
	if unlikely(begin == end)
		goto invalid;
	ASSERT(radix >= 2);
	if ((radix & (radix - 1)) != 0) {
		result = int_from_nonbinary_string(begin, end, radix, radix_and_flags);
		/* Check for errors. */
		if unlikely(!ITER_ISOK(result)) {
			if unlikely(result == (DREF DeeIntObject *)ITER_DONE)
				goto invalid;
			goto done;
		}
	} else {
		bits_per_digit = 0; /* bits_per_digit = ceil(sqrt(radix)) */
		while ((unsigned int)(1 << bits_per_digit) < radix)
			++bits_per_digit;
		{
			size_t num_digits = 1 + ((len * bits_per_digit) / DIGIT_BITS);
			result            = DeeInt_Alloc(num_digits);
			if unlikely(!result)
				goto done;
			bzeroc(result->ob_digit,
			       num_digits,
			       sizeof(digit));
		}
		dst    = result->ob_digit;
		number = 0, num_bits = 0;
		/* Parse the integer starting with the least significant bits. */
		iter = end;
		while (iter > begin) {
			uint32_t ch;
			digit dig;
			struct unitraits *trt;
			ch  = utf8_readchar_rev((char const **)&iter, begin);
			trt = DeeUni_Descriptor(ch);
			if (trt->ut_flags & UNICODE_FDECIMAL)
				dig = trt->ut_digit;
			else if (ch >= 'a' && ch <= 'z')
				dig = 10 + (digit)(ch - 'a');
			else if (ch >= 'A' && ch <= 'Z')
				dig = 10 + (digit)(ch - 'A');
			else if (DeeUni_IsLF(ch) &&
			         (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				if (iter == begin)
					goto invalid_r;
				if (iter[-1] == '\\') {
					--iter;
					continue;
				}
				if (iter[-1] == '\r' && ch == '\n' &&
				    iter - 1 != begin && iter[-2] == '\\') {
					iter -= 2;
					continue;
				}
				goto invalid_r;
			} else {
				goto invalid_r;
			}
			/* Got the digit. */
			if unlikely(dig >= radix)
				goto invalid_r;
			/* Add the digit to out number buffer. */
			number |= (twodigits)dig << num_bits;
			num_bits += bits_per_digit;
			while (num_bits >= DIGIT_BITS) {
				*dst++ = (digit)(number & DIGIT_MASK);
				number >>= DIGIT_BITS;
				num_bits -= DIGIT_BITS;
			}
		}
		/* Append trailing bits. */
		if (num_bits) {
			ASSERT(num_bits < DIGIT_BITS);
			*dst = (digit)number;
		}
		while (result->ob_size &&
		       !result->ob_digit[result->ob_size - 1])
			--result->ob_size;
	}
	/* Negate the integer if it was prefixed by `-' */
	if (negative)
		result->ob_size = -result->ob_size;
done:
	return (DREF DeeObject *)result;
invalid_r:
	Dee_DecrefDokill(result);
invalid:
	if (radix_and_flags & DEEINT_STRING_FTRY)
		return ITER_DONE;
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid integer %$q",
	                len, str);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_FromAscii(/*ascii*/ char const *__restrict str,
                 size_t len, uint32_t radix_and_flags) {
	unsigned int radix = radix_and_flags >> DEEINT_STRING_RSHIFT;
	bool negative      = false;
	DREF DeeIntObject *result;
	char *iter, *begin = (char *)str, *end = (char *)str + len;
	digit *dst;
	twodigits number;
	uint8_t num_bits;
	uint8_t bits_per_digit;
	/* Parse a sign prefix. */
	for (;; ++begin) {
		if (begin == end)
			goto invalid;
		if (*begin == '+')
			continue;
		if (*begin == '-') {
			negative = !negative;
			continue;
		}
		if (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
			char begin_plus_one = begin[1];
			if (DeeUni_IsLF(begin_plus_one)) {
				begin += 2;
				if (begin_plus_one == '\r' && begin[0] == '\n')
					++begin;
				continue;
			}
		}
		break;
	}
	if (!radix) {
		/* Automatically determine the radix. */
		char leading_zero = *begin;
		if (DeeUni_IsDecimalX(leading_zero, 0)) {
			++begin;
			if (begin == end) /* Special case: int(0) */
				return_reference_((DeeObject *)&DeeInt_Zero);
			while (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				char begin_plus_one = begin[1];
				if (DeeUni_IsLF(begin_plus_one)) {
					begin += 2;
					if (begin_plus_one == '\r' && begin[0] == '\n')
						++begin;
					continue;
				}
				break;
			}
			if (*begin == 'x' || *begin == 'X')
				radix = 16, ++begin;
			else if (*begin == 'b' || *begin == 'B')
				radix = 2, ++begin;
			else {
				radix = 8;
			}
		} else {
			radix = 10;
		}
	}
	if unlikely(begin == end)
		goto invalid;
	ASSERT(radix >= 2);
	if ((radix & (radix - 1)) != 0) {
		result = int_from_nonbinary_string(begin, end, radix, radix_and_flags);
		/* Check for errors. */
		if unlikely(!ITER_ISOK(result)) {
			if unlikely(result == (DREF DeeIntObject *)ITER_DONE)
				goto invalid;
			goto done;
		}
	} else {
		bits_per_digit = 0; /* bits_per_digit = ceil(sqrt(radix)) */
		while ((unsigned int)(1 << bits_per_digit) < radix)
			++bits_per_digit;
		{
			size_t num_digits = 1 + ((len * bits_per_digit) / DIGIT_BITS);
			result            = DeeInt_Alloc(num_digits);
			if unlikely(!result)
				goto done;
			bzeroc(result->ob_digit,
			       num_digits,
			       sizeof(digit));
		}
		dst    = result->ob_digit;
		number = 0, num_bits = 0;
		/* Parse the integer starting with the least significant bits. */
		iter = end;
		while (iter > begin) {
			char ch;
			digit dig;
			ch = *--iter;
			if (ch >= '0' && ch <= '9')
				dig = (digit)(ch - '0');
			else if (ch >= 'a' && ch <= 'z')
				dig = 10 + (digit)(ch - 'a');
			else if (ch >= 'A' && ch <= 'Z')
				dig = 10 + (digit)(ch - 'A');
			else if ((unsigned char)ch >= 0x80) {
				/* Unicode character? */
				uint32_t uni;
				struct unitraits *traits;
				++iter;
				uni    = utf8_readchar_rev((char const **)&iter, end);
				traits = DeeUni_Descriptor(uni);
				/* All any kind of digit/decimal character. - If the caller doesn't
				 * want to support any kind of digit, have `int("²")' evaluate to 2,
				 * then they have to verify that the string only contains ~conventional~
				 * decimals by using `string.isdecimal()'. As far as this check is
				 * concerned, we accept anything that applies to `string.isnumeric()' */
				if (traits->ut_flags & (Dee_UNICODE_FDIGIT | Dee_UNICODE_FDECIMAL))
					dig = traits->ut_digit;
				else if (uni >= 'a' && uni <= 'z')
					dig = 10 + ((uint8_t)uni - (uint8_t)'a');
				else if (uni >= 'A' && uni <= 'Z')
					dig = 10 + ((uint8_t)uni - (uint8_t)'A');
				else if (traits->ut_flags & Dee_UNICODE_FLF)
					goto handle_linefeed_in_text;
				else {
					goto invalid_r;
				}
			} else {
				if (!DeeUni_IsLF(ch))
					goto invalid_r;
handle_linefeed_in_text:
				if (!(radix_and_flags & DEEINT_STRING_FESCAPED))
					goto invalid_r;
				if (iter == begin)
					goto invalid_r;
				if (iter[-1] == '\\') {
					--iter;
					continue;
				}
				if (iter[-1] == '\r' && ch == '\n' &&
				    iter - 1 != begin && iter[-2] == '\\') {
					iter -= 2;
					continue;
				}
				goto invalid_r;
			}
			/* Got the digit. */
			if unlikely(dig >= radix)
				goto invalid_r;
			/* Add the digit to out number buffer. */
			number |= (twodigits)dig << num_bits;
			num_bits += bits_per_digit;
			while (num_bits >= DIGIT_BITS) {
				*dst++ = (digit)(number & DIGIT_MASK);
				number >>= DIGIT_BITS;
				num_bits -= DIGIT_BITS;
			}
		}
		/* Append trailing bits. */
		if (num_bits) {
			ASSERT(num_bits < DIGIT_BITS);
			*dst = (digit)number;
		}
		while (result->ob_size &&
		       !result->ob_digit[result->ob_size - 1])
			--result->ob_size;
	}
	/* Negate the integer if it was prefixed by `-' */
	if (negative)
		result->ob_size = -result->ob_size;
done:
	return (DREF DeeObject *)result;
invalid_r:
	Dee_DecrefDokill(result);
invalid:
	if (radix_and_flags & DEEINT_STRING_FTRY)
		return ITER_DONE;
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid integer %$q",
	                len, str);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL Dee_Atoi64)(/*utf-8*/ char const *__restrict str,
                   size_t len, uint32_t radix_and_flags,
                   int64_t *__restrict value) {
	unsigned int radix = radix_and_flags >> DEEINT_STRING_RSHIFT;
	char *iter, *begin = (char *)str, *end = (char *)str + len;
	bool negative = false;
	uint64_t result;
	/* Parse a sign prefix. */
	for (;; ++begin) {
		if (begin == end)
			goto err_invalid;
		if (*begin == '+')
			continue;
		if (*begin == '-') {
			negative = !negative;
			continue;
		}
		if (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
			uint32_t begin_plus_one;
			char *new_begin = begin + 1;
			begin_plus_one  = utf8_readchar((char const **)&new_begin, end);
			if (DeeUni_IsLF(begin_plus_one)) {
				begin = new_begin;
				if (begin_plus_one == '\r' &&
				    *begin == '\n')
					++begin;
				continue;
			}
		}
		break;
	}
	if unlikely(negative && !(radix_and_flags & DEEATOI_STRING_FSIGNED)) {
		/* Negative value when unsigned was needed. */
		if (radix_and_flags & DEEINT_STRING_FTRY)
			return 1;
		return err_integer_overflow_i(64, false);
	}
	if (!radix) {
		/* Automatically determine the radix. */
		char *old_begin = begin;
		uint32_t leading_zero;
		leading_zero = utf8_readchar((char const **)&begin, end);
		if (DeeUni_IsDecimalX(leading_zero, 0)) {
			if (begin == end) {
				/* Special case: int(0) */
				*value = 0;
				return 0;
			}
			while (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				uint32_t begin_plus_one;
				char *new_begin = begin + 1;
				begin_plus_one  = utf8_readchar((char const **)&new_begin, end);
				if (DeeUni_IsLF(begin_plus_one)) {
					begin = new_begin;
					if (begin_plus_one == '\r' &&
					    *begin == '\n')
						++begin;
					continue;
				}
				break;
			}
			if (*begin == 'x' || *begin == 'X')
				radix = 16, ++begin;
			else if (*begin == 'b' || *begin == 'B')
				radix = 2, ++begin;
			else {
				radix = 8;
			}
		} else {
			begin = old_begin;
			radix = 10;
		}
	}
	if unlikely(begin == end)
		goto err_invalid;
	ASSERT(radix >= 2);
	/* Parse the integer starting with the least significant bits. */
	result = 0;
	iter   = end;
	while (iter > begin) {
		uint32_t ch;
		uint8_t dig;
		struct unitraits *trt;
		ch  = utf8_readchar_rev((char const **)&iter, begin);
		trt = DeeUni_Descriptor(ch);
		if (trt->ut_flags & UNICODE_FDECIMAL)
			dig = trt->ut_digit;
		else if (ch >= 'a' && ch <= 'z')
			dig = 10 + (uint8_t)(ch - 'a');
		else if (ch >= 'A' && ch <= 'Z')
			dig = 10 + (uint8_t)(ch - 'A');
		else if (DeeUni_IsLF(ch) &&
		         (radix_and_flags & DEEINT_STRING_FESCAPED)) {
			if (iter == begin)
				goto err_invalid;
			if (iter[-1] == '\\') {
				--iter;
				continue;
			}
			if (iter[-1] == '\r' && ch == '\n' &&
			    iter - 1 != begin && iter[-2] == '\\') {
				iter -= 2;
				continue;
			}
			goto err_invalid;
		} else {
			goto err_invalid;
		}
		/* Got the digit. */
		if unlikely(dig >= radix)
			goto err_invalid;
		/* Add the digit to out number buffer. */
		if (OVERFLOW_UMUL(result, radix, &result))
			goto err_overflow;
		if (OVERFLOW_UADD(result, dig, &result))
			goto err_overflow;
	}
	/* Negate the integer if it was prefixed by `-' */
	if (negative) {
		if (result > INT64_MAX)
			goto err_overflow;
		result = (uint64_t) - (int64_t)result;
	}
	*value = result;
	return 0;
err_overflow:
	if (radix_and_flags & DEEINT_STRING_FTRY)
		return 1;
	return err_integer_overflow_i(64, !negative);
err_invalid:
	if (radix_and_flags & DEEINT_STRING_FTRY)
		return 1;
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Invalid integer %$q",
	                       len, str);
}

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL Dee_Atoi32)(/*utf-8*/ char const *__restrict str,
                   size_t len, uint32_t radix_and_flags,
                   int32_t *__restrict value) {
	int64_t val64;
	int result = Dee_Atoi64(str, len, radix_and_flags, &val64);
	if (result == 0) {
		if (radix_and_flags & DEEATOI_STRING_FSIGNED) {
			if unlikely(val64 < INT32_MIN || val64 > INT32_MAX) {
				err_integer_overflow_i(32, val64 < 0);
				goto err;
			}
		} else {
			if unlikely((uint64_t)val64 > UINT32_MAX) {
				err_integer_overflow_i(32, true);
				goto err;
			}
		}
		*value = (int32_t)val64;
	}
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL Dee_Atoi16)(/*utf-8*/ char const *__restrict str,
                   size_t len, uint32_t radix_and_flags,
                   int16_t *__restrict value) {
	int64_t val64;
	int result = Dee_Atoi64(str, len, radix_and_flags, &val64);
	if (result == 0) {
		if (radix_and_flags & DEEATOI_STRING_FSIGNED) {
			if unlikely(val64 < INT16_MIN || val64 > INT16_MAX) {
				err_integer_overflow_i(16, val64 < 0);
				goto err;
			}
		} else {
			if unlikely((uint64_t)val64 > UINT16_MAX) {
				err_integer_overflow_i(16, true);
				goto err;
			}
		}
		*value = (int16_t)val64;
	}
	return result;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 4)) int
(DCALL Dee_Atoi8)(/*utf-8*/ char const *__restrict str,
                  size_t len, uint32_t radix_and_flags,
                  int8_t *__restrict value) {
	int64_t val64;
	int result = Dee_Atoi64(str, len, radix_and_flags, &val64);
	if (result == 0) {
		if (radix_and_flags & DEEATOI_STRING_FSIGNED) {
			if unlikely(val64 < INT8_MIN || val64 > INT8_MAX) {
				err_integer_overflow_i(8, val64 < 0);
				goto err;
			}
		} else {
			if unlikely((uint64_t)val64 > UINT8_MAX) {
				err_integer_overflow_i(8, true);
				goto err;
			}
		}
		*value = (int8_t)val64;
	}
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeInt_PrintDecimal(DREF DeeIntObject *__restrict self, uint32_t flags,
                    dformatprinter printer, void *arg) {
	/* !!!DISCLAIMER!!! This function was originally taken from python,
	 *                  but has been heavily modified since. */
	size_t size, buflen, size_a, i, j;
	dssize_t result;
	digit *pout, *pin, rem, tenpow;
	int negative;
	char *buf, *iter;
	size_a   = (size_t)self->ob_size;
	negative = (dssize_t)size_a < 0;
	if (negative)
		size_a = (size_t) - (dssize_t)size_a;
	if unlikely(size_a > SSIZE_MAX / DIGIT_BITS) {
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "int too large to format");
err:
		return -1;
	}
	size = 1 + size_a * DIGIT_BITS / (3 * DeeInt_DECIMAL_SHIFT);
	pout = (digit *)Dee_AMalloc(size * sizeof(digit));
	if (!pout)
		goto err;
	pin  = self->ob_digit;
	size = 0;
	for (i = size_a; i--;) {
		digit hi = pin[i];
		for (j = 0; j < size; j++) {
			twodigits z = (twodigits)pout[j] << DIGIT_BITS | hi;
			hi = (digit)(z / DeeInt_DECIMAL_BASE);
			pout[j] = (digit)(z - (twodigits)hi *
			                      DeeInt_DECIMAL_BASE);
		}
		while (hi) {
			pout[size++] = hi % DeeInt_DECIMAL_BASE;
			hi /= DeeInt_DECIMAL_BASE;
		}
	}
	if (size == 0)
		pout[size++] = 0;
	buflen = 1 + 1 + (size - 1) * DeeInt_DECIMAL_SHIFT;
	tenpow = 10;
	rem    = pout[size - 1];
	while (rem >= tenpow) {
		tenpow *= 10;
		++buflen;
	}
	/* Allocate a string target buffer. */
	buf = (char *)Dee_AMalloc(buflen * sizeof(char));
	if unlikely(!buf)
		goto err_pout;
	iter = buf + buflen;
	for (i = 0; i < size - 1; ++i) {
		rem = pout[i];
		for (j = 0; j < DeeInt_DECIMAL_SHIFT; ++j) {
			*--iter = '0' + rem % 10;
			rem /= 10;
		}
	}
	rem = pout[i];
	do {
		*--iter = '0' + rem % 10;
		rem /= 10;
	} while (rem != 0);
	if (negative) {
		*--iter = '-';
	} else if (flags & DEEINT_PRINT_FSIGN) {
		*--iter = '+';
	}
	result = (*printer)(arg, iter, (buf + buflen) - iter);
	Dee_AFree(buf);
done_pout:
	Dee_AFree(pout);
	return result;
err_pout:
	result = -1;
	goto done_pout;
}

PRIVATE char const int_digits[2][18] = {
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x', 'q' }, /* Lower */
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X', 'Q' }  /* Upper */
};


PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeInt_Print(DeeObject *__restrict self, uint32_t radix_and_flags,
             dformatprinter printer, void *arg) {
	ASSERT_OBJECT_TYPE(self, &DeeInt_Type);
	switch (radix_and_flags >> DEEINT_PRINT_RSHIFT) {

	case 10:
		return DeeInt_PrintDecimal((DeeIntObject *)self, radix_and_flags, printer, arg);

	{
		twodigits number;
		digit *src;
		uint8_t num_bits, dig_bits, dig_mask, dig;
		char *buf, *iter;
		size_t bufsize, num_digits;
		dssize_t result;
		DeeIntObject *me;
		char *digit_chars;
		/* Power-of-2 radices. */
	case 2:
		dig_bits = 1;
		dig_mask = 0x1;
		goto do_print;

	case 4:
		dig_bits = 2;
		dig_mask = 0x3;
		goto do_print;

	case 8:
		dig_bits = 3;
		dig_mask = 0x7;
		goto do_print;

	case 16:
		dig_bits = 4;
		dig_mask = 0xf;
do_print:
		me         = (DeeIntObject *)self;
		num_digits = (size_t)me->ob_size;
#if DEEINT_PRINT_FUPPER == 1
		digit_chars = (char *)int_digits[radix_and_flags & DEEINT_PRINT_FUPPER];
#else /* DEEINT_PRINT_FUPPER == 1 */
		digit_chars = (char *)int_digits[(radix_and_flags & DEEINT_PRINT_FUPPER) ? 1 : 0];
#endif /* DEEINT_PRINT_FUPPER != 1 */
		if ((dssize_t)num_digits <= 0) {
			if (!num_digits) {
				bufsize = 4;
				buf     = (char *)Dee_AMalloc(bufsize * sizeof(char));
				if unlikely(!buf)
					goto err;
				iter    = buf + bufsize;
				*--iter = '0';
				goto do_print_prefix;
			}
			num_digits = (size_t) - (dssize_t)num_digits;
		}
		bufsize = 4 + ((num_digits * DIGIT_BITS) / dig_bits);
		buf     = (char *)Dee_AMalloc(bufsize * sizeof(char));
		if unlikely(!buf)
			goto err;
		iter   = buf + bufsize;
		src    = me->ob_digit;
		number = 0, num_bits = 0;
		do {
			if (num_bits < dig_bits && num_digits) {
				number &= (1 << num_bits) - 1;
				number |= (twodigits)*src++ << num_bits;
				num_bits += DIGIT_BITS;
				if (!--num_digits) {
					/* Strip leading ZERO-digits from the output. */
					while (num_bits && !((number >> (num_bits - 1))&1))
						--num_bits;
				}
			}
			/* Print extracted digits. */
			while (num_bits >= dig_bits) {
				num_bits -= dig_bits;
				dig = number & dig_mask;
				number >>= dig_bits;
				*--iter = digit_chars[dig];
			}
		} while (num_digits);

		/* Print remaining bits. */
		if (num_bits) {
			dig     = number & dig_mask;
			*--iter = digit_chars[dig];
		}
do_print_prefix:
		/* Print the numsys prefix. */
		if (radix_and_flags & DEEINT_PRINT_FNUMSYS) {
			if (dig_bits == 4)
				*--iter = digit_chars[16]; /* x */
			if (dig_bits == 2)
				*--iter = digit_chars[17]; /* q */
			if (dig_bits == 1)
				*--iter = digit_chars[11]; /* b */
			*--iter = '0';
		}
		/* Print the sign prefix. */
		if (me->ob_size < 0) {
			*--iter = '-';
		} else if (radix_and_flags & DEEINT_PRINT_FSIGN) {
			*--iter = '+';
		}
		result = (*printer)(arg, iter, (size_t)((buf + bufsize) - iter));
		Dee_AFree(buf);
		return result;
	}	break;

	default:
		break;
	}
	/* TODO: support for arbitrary values for radix! (in the range 2..36 inclusively)
	 *       After all: the fromstring() function also supports arbitrary values... */
	DeeError_Throwf(&DeeError_NotImplemented,
	                "Unsupported integer radix %u",
	                (unsigned)(radix_and_flags >> DEEINT_PRINT_RSHIFT));
err:
	return -1;
}



PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeInt_TryAs8(DeeObject *__restrict self,
              int8_t *__restrict value) {
	int result;
#if DIGIT_BITS <= 16
	int16_t digval;
	result = DeeInt_TryAs16(self, &digval);
#else /* DIGIT_BITS <= 16 */
	int32_t digval;
	result = DeeInt_TryAs32(self, &digval);
#endif /* DIGIT_BITS > 16 */
	switch (result) {

	case INT_UNSIGNED:
#if DIGIT_BITS <= 16
		if ((uint16_t)digval > 0xff)
			return INT_POS_OVERFLOW;
		*value = (int8_t)(uint8_t)(uint16_t)digval;
#else /* DIGIT_BITS <= 16 */
		if ((uint32_t)digval > 0xff)
			return INT_POS_OVERFLOW;
		*value = (int8_t)(uint8_t)(uint32_t)digval;
#endif /* DIGIT_BITS > 16 */
		break;

	case INT_SIGNED:
		if (digval > INT8_MAX)
			return INT_POS_OVERFLOW;
		if (digval < INT8_MIN)
			return INT_NEG_OVERFLOW;
		*value = (int8_t)digval;
		break;

	default:
		break;
	}
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeInt_TryAs16(DeeObject *__restrict self,
               int16_t *__restrict value) {
#if DIGIT_BITS <= 16
	uint16_t prev, result;
	int sign;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	switch (DeeInt_SIZE(self)) {

	case 0:
		*value = 0;
		return 0;

	case 1:
		*value = DeeInt_DIGIT(self)[0];
		return INT_UNSIGNED;

	case -1:
		*value = -(int16_t)DeeInt_DIGIT(self)[0];
		return INT_SIGNED;

	default: break;
	}
	result = prev = 0, sign = 1;
	i = DeeInt_SIZE(self);
	if (i < 0)
		sign = -1, i = -i;
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | DeeInt_DIGIT(self)[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (sign < 0) {
		if (result <= INT16_MAX) {
			result = (uint16_t)(-(int16_t)result);
		} else if (result == (uint16_t)(0 - (uint16_t)INT16_MIN)) {
			result = (uint16_t)INT16_MIN;
		} else {
overflow:
			return sign > 0 ? INT_POS_OVERFLOW : INT_NEG_OVERFLOW;
		}
		*value = (int16_t)result;
		return INT_SIGNED;
	}
	*value = (int16_t)result;
	return INT_UNSIGNED;
#else /* DIGIT_BITS <= 16 */
	int32_t digval;
	int result = DeeInt_TryAs32(self, &digval);
	switch (result) {

	case INT_UNSIGNED:
		if ((uint32_t)digval > 0xffff)
			return INT_POS_OVERFLOW;
		*value = (int16_t)(uint16_t)(uint32_t)digval;
		break;

	case INT_SIGNED:
		if (digval > INT16_MAX)
			return INT_POS_OVERFLOW;
		if (digval < INT16_MIN)
			return INT_NEG_OVERFLOW;
		*value = (int16_t)digval;
		break;

	default:
		break;
	}
	return result;
#endif /* DIGIT_BITS > 16 */
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeInt_TryAs32(DeeObject *__restrict self,
               int32_t *__restrict value) {
	uint32_t prev, result;
	int sign;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	switch (DeeInt_SIZE(self)) {
	case 0: *value = 0; return 0;
	case 1:
		*value = DeeInt_DIGIT(self)[0];
		return INT_UNSIGNED;
	case -1:
		*value = -(int32_t)DeeInt_DIGIT(self)[0];
		return INT_SIGNED;
	default: break;
	}
	result = prev = 0, sign = 1;
	i = DeeInt_SIZE(self);
	if (i < 0)
		sign = -1, i = -i;
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | DeeInt_DIGIT(self)[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (sign < 0) {
		if (result <= INT32_MAX) {
			result = (uint32_t)(-(int32_t)result);
		} else if (result == (uint32_t)(0 - (uint32_t)INT32_MIN)) {
			result = (uint32_t)INT32_MIN;
		} else {
overflow:
			return sign > 0 ? INT_POS_OVERFLOW : INT_NEG_OVERFLOW;
		}
		*value = (int32_t)result;
		return INT_SIGNED;
	}
	*value = (int32_t)result;
	return INT_UNSIGNED;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeInt_TryAs64(DeeObject *__restrict self,
               int64_t *__restrict value) {
	uint64_t prev, result;
	int sign;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	switch (DeeInt_SIZE(self)) {

	case 0:
		*value = 0;
		return 0;

	case 1:
		*value = DeeInt_DIGIT(self)[0];
		return INT_UNSIGNED;

	case -1:
		*value = -(int64_t)DeeInt_DIGIT(self)[0];
		return INT_SIGNED;

	default: break;
	}
	result = prev = 0, sign = 1;
	i = DeeInt_SIZE(self);
	if (i < 0)
		sign = -1, i = -i;
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | DeeInt_DIGIT(self)[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (sign < 0) {
		if (result <= INT64_MAX) {
			result = (uint64_t)(-(int64_t)result);
		} else if (result == (uint64_t)(0 - (uint64_t)INT64_MIN)) {
			result = (uint64_t)INT64_MIN;
		} else {
overflow:
			return sign > 0 ? INT_POS_OVERFLOW : INT_NEG_OVERFLOW;
		}
		*value = (int64_t)result;
		return INT_SIGNED;
	}
	*value = (int64_t)result;
	return INT_UNSIGNED;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeInt_TryAs128(DeeObject *__restrict self,
                dint128_t *__restrict value) {
	union {
		duint128_t u;
		dint128_t  s;
	} result;
	int sign;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	switch (DeeInt_SIZE(self)) {

	case 0:
		DUINT128_GET64(*value)[DEE_INT128_LS64] = 0;
		DUINT128_GET64(*value)[DEE_INT128_MS64] = 0;
		return 0;

	case 1:
		DUINT128_GET64(*value)[DEE_INT128_LS64] = DeeInt_DIGIT(self)[0];
		DUINT128_GET64(*value)[DEE_INT128_MS64] = 0;
		return INT_UNSIGNED;

	case -1:
		DUINT128_GETS64(*value)[DEE_INT128_LS64] = -(sdigit)DeeInt_DIGIT(self)[0];
		DUINT128_GETS64(*value)[DEE_INT128_MS64] = 0;
		return INT_SIGNED;

	default: break;
	}
	DUINT128_SET(result.u, 0);
	sign = 1;
	i    = DeeInt_SIZE(self);
	if (i < 0)
		sign = -1, i = -i;
	while (--i >= 0) {
		if (DUINT128_SHL_WILL_OVERFLOW(result.u, DIGIT_BITS))
			goto overflow;
		DUINT128_SHL(result.u, DIGIT_BITS);
		DUINT128_OR(result.u, DeeInt_DIGIT(self)[i]);
	}
	if (sign < 0) {
		if (!DINT128_ISMAX(result.s)) {
			DSINT128_TONEG(result.s);
		} else if (DINT128_IS0MMIN(result.s)) {
			DINT128_SETMIN(result.s);
		} else {
overflow:
			return sign > 0 ? INT_POS_OVERFLOW : INT_NEG_OVERFLOW;
		}
		*value = result.s;
		return INT_SIGNED;
	}
	*value = result.s;
	return INT_UNSIGNED;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsS8)(DeeObject *__restrict self,
                       int8_t *__restrict value) {
	int error = DeeInt_TryAs8(self, value);
	if (error == INT_UNSIGNED && *(uint8_t *)value > INT8_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsS16)(DeeObject *__restrict self,
                        int16_t *__restrict value) {
	int error = DeeInt_TryAs16(self, value);
	if (error == INT_UNSIGNED && *(uint16_t *)value > INT16_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsS32)(DeeObject *__restrict self,
                        int32_t *__restrict value) {
	int error = DeeInt_TryAs32(self, value);
	if (error == INT_UNSIGNED && *(uint32_t *)value > INT32_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsS64)(DeeObject *__restrict self,
                        int64_t *__restrict value) {
	int error = DeeInt_TryAs64(self, value);
	if (error == INT_UNSIGNED && *(uint64_t *)value > INT64_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsS128)(DeeObject *__restrict self,
                         dint128_t *__restrict value) {
	int error = DeeInt_TryAs128(self, value);
	if (error == INT_UNSIGNED && DSINT128_ISNEG(*value))
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsU8)(DeeObject *__restrict self,
                       uint8_t *__restrict value) {
	int error = DeeInt_TryAs8(self, (int8_t *)value);
	if (error == INT_SIGNED && *(int8_t *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsU16)(DeeObject *__restrict self,
                        uint16_t *__restrict value) {
	int error = DeeInt_TryAs16(self, (int16_t *)value);
	if (error == INT_SIGNED && *(int16_t *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsU32)(DeeObject *__restrict self,
                        uint32_t *__restrict value) {
	int error = DeeInt_TryAs32(self, (int32_t *)value);
	if (error == INT_SIGNED && *(int32_t *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsU64)(DeeObject *__restrict self,
                        uint64_t *__restrict value) {
	int error = DeeInt_TryAs64(self, (int64_t *)value);
	if (error == INT_SIGNED && *(int64_t *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInt_TryAsU128)(DeeObject *__restrict self,
                         duint128_t *__restrict value) {
	int error = DeeInt_TryAs128(self, (dint128_t *)value);
	if (error == INT_SIGNED && DSINT128_ISNEG(*value))
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_As8)(DeeObject *__restrict self, int8_t *__restrict value) {
	int result = DeeInt_TryAs8(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 8, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_As16)(DeeObject *__restrict self, int16_t *__restrict value) {
	int result = DeeInt_TryAs16(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 16, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_As32)(DeeObject *__restrict self, int32_t *__restrict value) {
	int result = DeeInt_TryAs32(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 32, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_As64)(DeeObject *__restrict self, int64_t *__restrict value) {
	int result = DeeInt_TryAs64(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 64, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_As128)(DeeObject *__restrict self, dint128_t *__restrict value) {
	int result = DeeInt_TryAs128(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 128, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsS8)(DeeObject *__restrict self, int8_t *__restrict value) {
	int error = DeeInt_As8(self, value);
	if (error == INT_UNSIGNED && *(uint8_t *)value > INT8_MAX)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 8, true);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsS16)(DeeObject *__restrict self, int16_t *__restrict value) {
	int error = DeeInt_As16(self, value);
	if (error == INT_UNSIGNED && *(uint16_t *)value > INT16_MAX)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 16, true);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsS32)(DeeObject *__restrict self, int32_t *__restrict value) {
	int error = DeeInt_As32(self, value);
	if (error == INT_UNSIGNED && *(uint32_t *)value > INT32_MAX)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 32, true);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsS64)(DeeObject *__restrict self, int64_t *__restrict value) {
	int error = DeeInt_As64(self, value);
	if (error == INT_UNSIGNED && *(uint64_t *)value > INT64_MAX)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 64, true);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsS128)(DeeObject *__restrict self, dint128_t *__restrict value) {
	int error = DeeInt_As128(self, value);
	if (error == INT_UNSIGNED && DSINT128_ISNEG(*value))
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 128, true);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsU8)(DeeObject *__restrict self, uint8_t *__restrict value) {
	int error = DeeInt_As8(self, (int8_t *)value);
	if (error == INT_SIGNED && *(int8_t *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 8, false);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsU16)(DeeObject *__restrict self, uint16_t *__restrict value) {
	int error = DeeInt_As16(self, (int16_t *)value);
	if (error == INT_SIGNED && *(int16_t *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 16, false);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsU32)(DeeObject *__restrict self, uint32_t *__restrict value) {
	int error = DeeInt_As32(self, (int32_t *)value);
	if (error == INT_SIGNED && *(int32_t *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 32, false);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsU64)(DeeObject *__restrict self, uint64_t *__restrict value) {
	int error = DeeInt_As64(self, (int64_t *)value);
	if (error == INT_SIGNED && *(int64_t *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 64, false);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsU128)(DeeObject *__restrict self, duint128_t *__restrict value) {
	int error = DeeInt_As128(self, (dint128_t *)value);
	if (error == INT_SIGNED && DSINT128_ISNEG(*value))
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 128, false);
}


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsBytes)(DeeObject *__restrict self,
                       void *__restrict dst, size_t length,
                       bool little_endian, bool as_signed) {
	uint8_t *writer;
	twodigits temp;
	size_t i, count, remaining;
	dssize_t incr;
	size_t num_bits;
	uint8_t leading_byte;
	digit last_digit;
	unsigned int last_bits;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeInt_Type);
	count = (size_t)DeeInt_SIZE(self);
	if unlikely(!count) {
		/* Special case: zero. */
		bzero(dst, length);
		return 0;
	}
	leading_byte = 0;
	if ((dssize_t)count < 0) {
		count        = (size_t) - (dssize_t)count;
		leading_byte = 0xff;
		if unlikely(!as_signed) {
			err_integer_overflow((DeeObject *)self, 0, false);
			goto err;
		}
	}
	if (little_endian) {
		incr   = 1;
		writer = (uint8_t *)dst;
	} else {
		incr   = -1;
		writer = (uint8_t *)dst + length - 1;
	}
	temp      = 0;
	num_bits  = 0;
	remaining = length;
	for (i = 0; i < count - 1; ++i) {
		temp |= (twodigits)DeeInt_DIGIT(self)[i] << num_bits;
		num_bits += DIGIT_BITS;
		while (num_bits >= 8) {
			if (!remaining)
				goto err_overflow;
			num_bits -= 8;
			*writer = temp & 0xff;
			writer += incr;
			--remaining;
			temp >>= 8;
			temp &= ((twodigits)1 << num_bits) - 1;
		}
	}
	ASSERT(i == count - 1);
	last_digit = DeeInt_DIGIT(self)[i];
	last_bits  = DIGIT_BITS;
	/* The last bit was read. - Now truncate leading zeros. */
	while (last_bits && !(last_digit & ((digit)1 << (last_bits - 1))))
		--last_bits;
	temp |= (twodigits)last_digit << num_bits;
	num_bits += last_bits;
	/* Write all remaining full bytes. */
	while (num_bits >= 8) {
		if (!remaining)
			goto err_overflow;
		num_bits -= 8;
		*writer = temp & 0xff;
		writer += incr;
		--remaining;
		temp >>= 8;
		temp &= ((twodigits)1 << num_bits) - 1;
	}

	if (num_bits) {
		/* There are still some more remaining bits. */
		uint8_t lead_mask = ((uint8_t)1 << num_bits) - 1;
		if (!remaining)
			goto err_overflow;
#if 1
		*writer = temp & lead_mask;
#else
		*writer = leading_byte & ~lead_mask;
		*writer |= temp & lead_mask;
#endif
		writer += incr;
		--remaining;
	} else if (!remaining && as_signed) {
		/* No space left for a sign bit. */
		goto err_overflow;
	}
	/* Fill in all remaining bytes with the leading byte. */
	memset(little_endian
	       ? (void *)writer
	       : dst,
	       leading_byte,
	       remaining);
#if 1
	if (DeeInt_SIZE(self) < 0) {
		/* The integer is negative. -> We must decrement +
		 * invert all the integer bits that were written. */
		size_t total_bits = num_bits + (length - remaining) * 8;
		if (num_bits)
			total_bits -= 8;
		num_bits = total_bits;
		writer   = (uint8_t *)dst;
		if (!little_endian)
			writer += length - 1;
		while (num_bits >= 8) {
			if ((*writer)-- != 0)
				goto done_decr;
			writer += incr;
			num_bits -= 8;
		}
		if (num_bits) {
			/* Decrement the last partial byte. */
			uint8_t old_byte  = *writer;
			uint8_t byte_mask = ((uint8_t)1 << num_bits) - 1;
			uint8_t new_byte  = (uint8_t)((old_byte & byte_mask) - 1);
			*writer           = old_byte & ~byte_mask;
			*writer |= new_byte & byte_mask;
		}
done_decr:
		/* With the decrement complete, we must now invert all bits. */
		num_bits = total_bits;
		writer   = (uint8_t *)dst;
		if (!little_endian)
			writer += length - 1;
		while (num_bits >= 8) {
			*writer = ~*writer;
			writer += incr;
			num_bits -= 8;
		}
		if (num_bits) {
#if 1
			*writer = ~*writer;
#else
			/* Invert the last partial byte. */
			uint8_t old_byte  = *writer;
			uint8_t byte_mask = ((uint8_t)1 << num_bits) - 1;
			*writer           = (old_byte & ~byte_mask) | (~old_byte & byte_mask);
#endif
		}
	}
#endif
	return 0;
err_overflow:
	err_integer_overflow(self, length * 8, true);
err:
	return -1;
}

PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeInt_FromBytes)(void const *buf, size_t length,
                         bool little_endian, bool as_signed) {
	DREF DeeIntObject *result;
	size_t total_bits;
	size_t total_digits;
	bool is_negative = false;
	total_bits       = length * 8;
	if (as_signed) {
		uint8_t sign_byte;
		uint8_t msb_byte;
		unsigned int msb_topbit;
		if (!length)
			goto return_zero;
		sign_byte = 0x00;
		if (little_endian) {
			if (((uint8_t *)buf)[length - 1] & 0x80) {
				sign_byte   = 0xff;
				is_negative = true;
			}
			/* Strip leading sign bytes. */
			while (((uint8_t *)buf)[length - 1] == sign_byte) {
				if (!--length) {
					if (sign_byte)
						goto return_m1;
					goto return_zero;
				}
				total_bits -= 8;
			}
			msb_byte = ((uint8_t *)buf)[length - 1];
		} else {
			if (((uint8_t *)buf)[0] & 0x80) {
				sign_byte   = 0xff;
				is_negative = true;
			}
			/* Strip leading sign bytes. */
			while (((uint8_t *)buf)[0] == sign_byte) {
				if (!--length) {
					if (sign_byte)
						goto return_m1;
					goto return_zero;
				}
				total_bits -= 8;
				buf = (void *)((uint8_t *)buf + 1);
			}
			msb_byte = ((uint8_t *)buf)[0];
		}
		/* Strip leading bits. */
		msb_topbit = 7;
		while (msb_topbit) {
			/* Find the last 1-bit in MSB */
			uint8_t msb_mask = (1 << msb_topbit);
			if ((msb_byte & msb_mask) != (sign_byte & msb_mask))
				break;
			--msb_topbit;
		}
		total_bits -= (7 - msb_topbit);
	} else {
		uint8_t msb_byte;
		unsigned int msb_topbit;
		if (little_endian) {
			while (length && !((uint8_t *)buf)[length - 1])
				--length, total_bits -= 8;
			if (!length)
				goto return_zero;
			msb_byte = ((uint8_t *)buf)[length - 1];
		} else {
			while (length && !((uint8_t *)buf)[0]) {
				--length;
				total_bits -= 8;
				buf = (void *)((uint8_t *)buf + 1);
			}
			if (!length)
				goto return_zero;
			msb_byte = ((uint8_t *)buf)[0];
		}
		msb_topbit = 7;
		while (msb_topbit) {
			/* Find the last 1-bit in MSB */
			if (msb_byte & (1 << msb_topbit))
				break;
			--msb_topbit;
		}
		total_bits -= (7 - msb_topbit);
	}
	/* At this point, we've determined the exact number
	 * of bits, as well as having extracted the sign bit,
	 * and having stripped unused sign extensions. */
	total_digits = (total_bits + (DIGIT_BITS - 1)) / DIGIT_BITS;
	ASSERT(total_digits >= 1);
	result = DeeInt_Alloc(total_digits);
	if unlikely(!result)
		goto done;
	/* Now to actually fill in integer digit data. */
	{
		twodigits temp     = 0;
		unsigned num_bits  = 0;
		size_t byte_index  = 0;
		size_t digit_index = 0;
		for (; byte_index < length; ++byte_index) {
			if (byte_index == length - 1) {
				unsigned int byte_bits = total_bits % 8;
				uint8_t byte_value;
				if (!byte_bits)
					byte_bits = 8;
				if (little_endian) {
					byte_value = ((uint8_t *)buf)[byte_index];
				} else {
					byte_value = ((uint8_t *)buf)[(length - 1) - byte_index];
				}
				temp |= (digit)(byte_value & (uint8_t)((1 << byte_bits) - 1)) << num_bits;
				num_bits += byte_bits;
			} else {
				if (little_endian) {
					temp |= (digit)((uint8_t *)buf)[byte_index] << num_bits;
				} else {
					temp |= (digit)((uint8_t *)buf)[(length - 1) - byte_index] << num_bits;
				}
				num_bits += 8;
			}
#if DIGIT_BITS >= 8
			if (num_bits >= DIGIT_BITS)
#else /* DIGIT_BITS >= 8 */
			while (num_bits >= DIGIT_BITS)
#endif /* DIGIT_BITS < 8 */
			{
				ASSERT(digit_index < total_digits);
				num_bits -= DIGIT_BITS;
				result->ob_digit[digit_index] = temp & DIGIT_MASK;
				temp >>= DIGIT_BITS;
				temp &= ((twodigits)1 << num_bits) - 1;
				++digit_index;
			}
		}
		ASSERT(num_bits ? (digit_index == total_digits - 1)
		                : (digit_index == total_digits));
		ASSERT(num_bits == total_bits - (digit_index * DIGIT_BITS));
		ASSERT(num_bits < DIGIT_BITS);
		if (num_bits) {
			/* Fill in the last digit with the remaining bits. */
			result->ob_digit[digit_index] = temp & (((digit)1 << num_bits) - 1);
		}
#if 1
		if (is_negative) {
			/* Transform all written bits: `dec();inv();' */
			for (digit_index = 0; digit_index < total_digits - 1; ++digit_index) {
				if ((result->ob_digit[digit_index])-- != 0)
					goto done_decr;
				result->ob_digit[digit_index] &= DIGIT_MASK;
			}
			/* Decrement the remaining number of bits. */
			if (!num_bits)
				num_bits = DIGIT_BITS;
			if ((result->ob_digit[digit_index])-- != 0)
				goto done_decr;
			result->ob_digit[digit_index] &= (((digit)1 << num_bits) - 1);
done_decr:
			/* With the decrement complete, we must now invert all bits. */
			for (digit_index = 0; digit_index < total_digits; ++digit_index)
				result->ob_digit[digit_index] ^= DIGIT_MASK;
			/* Mask non-set bits of the most significant digit. */
			result->ob_digit[total_digits - 1] &= ((digit)1 << num_bits) - 1;
		}
#endif
	}

	/* Finally, fill in the integer size field. */
	result->ob_size = (dssize_t)total_digits;
	if (is_negative)
		result->ob_size = -result->ob_size;
done:
	return (DREF DeeObject *)result;
return_m1:
	return_reference_((DeeObject *)&DeeInt_MinusOne);
return_zero:
	return_reference_((DeeObject *)&DeeInt_Zero);
}





PRIVATE WUNUSED DREF DeeObject *DCALL int_return_zero(void) {
	return_reference_((DeeObject *)&DeeInt_Zero);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
int_new(size_t argc, DeeObject *const *argv) {
	DeeObject *val;
	uint16_t radix = 0;
	if (DeeArg_Unpack(argc, argv, "o|I16u:int", &val, &radix))
		goto err;
	if (DeeString_Check(val)) {
		char *utf8 = DeeString_AsUtf8(val);
		if unlikely(!utf8)
			goto err;
		if unlikely(radix == 1)
			goto err_bad_radix;
		return DeeInt_FromString(utf8,
		                         WSTR_LENGTH(utf8),
		                         DEEINT_STRING(radix,
		                                       DEEINT_STRING_FNORMAL));
	}
	if (DeeBytes_Check(val)) {
		if unlikely(radix == 1)
			goto err_bad_radix;
		return DeeInt_FromAscii((char *)DeeBytes_DATA(val),
		                        DeeBytes_SIZE(val),
		                        DEEINT_STRING(radix,
		                                      DEEINT_STRING_FNORMAL));
	}
	return DeeObject_Int(val);
err_bad_radix:
	DeeError_Throwf(&DeeError_ValueError, "Invalid radix = 1");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL int_bool(DeeObject *__restrict self) {
	return ((DeeIntObject *)self)->ob_size != 0;
}




PRIVATE struct type_math int_math = {
	/* .tp_int32       = */ &DeeInt_As32,
	/* .tp_int64       = */ &DeeInt_As64,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ &DeeObject_NewRef,
	/* .tp_inv         = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&int_inv,
	/* .tp_pos         = */ &DeeObject_NewRef,
	/* .tp_neg         = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&int_neg,
	/* .tp_add         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_add,
	/* .tp_sub         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_sub,
	/* .tp_mul         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_mul,
	/* .tp_div         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_div,
	/* .tp_mod         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_mod,
	/* .tp_shl         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_shl,
	/* .tp_shr         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_shr,
	/* .tp_and         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_and,
	/* .tp_or          = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_or,
	/* .tp_xor         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_xor,
	/* .tp_pow         = */ (DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&int_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&int_dec,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL
};




/* Integer compare. */
PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
int_compareint(DeeIntObject *a, DeeIntObject *b) {
	dssize_t sign;
	if (a->ob_size != b->ob_size) {
		sign = a->ob_size - b->ob_size;
	} else {
		dssize_t i = a->ob_size;
		if (i < 0)
			i = -i;
		while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i])
			;
		if (i < 0)
			sign = 0;
		else {
			sign = ((sdigit)a->ob_digit[i] -
			        (sdigit)b->ob_digit[i]);
			if (a->ob_size < 0)
				sign = -sign;
		}
	}
	return sign /* < 0 ? -1 : sign > 0 ? 1 : 0*/;
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
int_hash(DeeIntObject *__restrict self) {
	dhash_t x;
	dssize_t i;
	int sign;
	i = self->ob_size;
	switch (i) {

	case -1:
		return -(sdigit)self->ob_digit[0];

	case 0:
		return 0;

	case 1:
		return self->ob_digit[0];

	default: break;
	}
	sign = 1, x = 0;
	if (i < 0)
		sign = -1, i = -i;
	while (--i >= 0) {
		x = (x << DIGIT_BITS) | (x >> ((__SIZEOF_POINTER__ * 8) - DIGIT_BITS));
		x += self->ob_digit[i];
	}
	return x * sign;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_eq(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_ne(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_lo(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value < 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_le(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value <= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_gr(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value > 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_ge(DeeObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	some_object = DeeObject_Int(some_object);
	if unlikely(!some_object)
		goto err;
	compare_value = int_compareint((DeeIntObject *)self,
	                               (DeeIntObject *)some_object);
	Dee_Decref(some_object);
	return_bool(compare_value >= 0);
err:
	return NULL;
}


PRIVATE struct type_cmp int_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&int_hash,
	/* .tp_eq   = */ &int_cmp_eq,
	/* .tp_ne   = */ &int_cmp_ne,
	/* .tp_lo   = */ &int_cmp_lo,
	/* .tp_le   = */ &int_cmp_le,
	/* .tp_gr   = */ &int_cmp_gr,
	/* .tp_ge   = */ &int_cmp_ge,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_str(DeeObject *__restrict self) {
#if 0 /* XXX: Locale support? And if so, enable the unicode variant here. */
	struct unicode_printer p = UNICODE_PRINTER_INIT;
	/* Simply print this integer to the printer, using decimal encoding. */
	if (DeeInt_Print(self, DEEINT_PRINT_DEC, &unicode_printer_print, &p) < 0)
		goto err;
	return unicode_printer_pack(&p);
err:
	unicode_printer_fini(&p);
	return NULL;
#else
	struct ascii_printer p = ASCII_PRINTER_INIT;
	/* Simply print this integer to the printer, using decimal encoding. */
	if unlikely(DeeInt_Print(self, DEEINT_PRINT_DEC,
	                         &ascii_printer_print,
	                         &p) < 0)
		goto err;
	return ascii_printer_pack(&p);
err:
	ascii_printer_fini(&p);
	return NULL;
#endif
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tostr_impl(DeeObject *__restrict self, uint32_t flags) {
#if 0 /* XXX: Locale support? And if so, enable the unicode variant here. */
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeInt_Print(self, flags, &unicode_printer_print, &printer) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
#else
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(DeeInt_Print(self, flags,
	                         &ascii_printer_print,
	                         &printer) < 0)
		goto err_printer;
	return ascii_printer_pack(&printer);
err_printer:
	ascii_printer_fini(&printer);
#endif
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tostr(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	uint32_t flags                  = 10 << DEEINT_PRINT_RSHIFT;
	char *flags_str                 = NULL;
	PRIVATE struct keyword kwlist[] = { K(radix), K(mode), KEND };
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|I16us:tostr",
	                    &((uint16_t *)&flags)[0], &flags_str))
		goto err;
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|I16us:tostr",
	                    &((uint16_t *)&flags)[1], &flags_str))
		goto err;
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */
	if (flags_str) {
		char *iter = flags_str;
		for (;;) {
			char ch = *iter++;
			if (!ch)
				break;
			if (ch == 'u' || ch == 'X')
				flags |= DEEINT_PRINT_FUPPER;
			else if (ch == 'n' || ch == '#')
				flags |= DEEINT_PRINT_FNUMSYS;
			else if (ch == 's' || ch == '+')
				flags |= DEEINT_PRINT_FSIGN;
			else {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid integer to flags:?Dstring %q",
				                flags_str);
				goto err;
			}
		}
	}
	return int_tostr_impl(self, flags);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_hex(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":hex"))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(16, DEEINT_PRINT_FNUMSYS));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_bin(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":bin"))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(2, DEEINT_PRINT_FNUMSYS));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_oct(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":oct"))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(8, DEEINT_PRINT_FNUMSYS));
err:
	return NULL;
}


/* Return the number of bits needed to represent
 * `self' as a base-2 (possibly signed) integer.
 * When `self' is negative, and `is_signed' is false,
 * an error is thrown, and (size_t)-1 is returned. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
int_reqbits(DeeIntObject const *__restrict self, bool is_signed) {
	size_t result;
	size_t digit_count;
	digit last_digit;
	digit_count = (size_t)self->ob_size;
	if ((Dee_ssize_t)digit_count < 0) {
		if unlikely(!is_signed)
			goto err_underflow;
		digit_count = (size_t) - (Dee_ssize_t)digit_count;
	}
	while (digit_count && self->ob_digit[digit_count - 1] == 0)
		--digit_count;
	if (!digit_count)
		return 1; /* Special case: `0' */
	/* Account for all of the digits leading up to the last one. */
	result = (digit_count - 1) * DIGIT_BITS;
	last_digit = self->ob_digit[digit_count - 1];
	ASSERT(last_digit != 0);
	if (self->ob_size < 0) {
		++result;
		/* Deal with special case: Integers that require the same number
		 * of bits as unsigned-positive, like they do as signed-negative */
		if (POPCOUNT(last_digit) == 1) {
			size_t i;
			for (i = 0; i < digit_count - 1; ++i) {
				if (self->ob_digit[i] != 0)
					goto not_a_limit_int;
			}
			if (digit_count == 1 && last_digit == 1) {
				/* Special case: `-1' (still requires at least 2 bits to represent) */
			} else {
				--result;
			}
		}
not_a_limit_int:
		last_digit = ~last_digit & DIGIT_MASK;
		do {
			++result;
			last_digit >>= 1;
			last_digit |= (DIGIT_BASE >> 1);
		} while (last_digit != DIGIT_MASK);
	} else {
		do {
			++result;
			last_digit >>= 1;
		} while (last_digit);
		/* When needing to represent a signed integer, we'll be
		 * needing at least one additional, leading sign-bit. */
		if (is_signed)
			++result;
	}
	return result;
err_underflow:
	err_integer_overflow((DeeObject *)self, 0, false);
	return (size_t)-1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tobytes(DeeIntObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	struct keyword kwlist[] = { K(length), K(byteorder), K(signed), KEND };
	size_t length = (size_t)-1;
	DeeObject *byteorder = Dee_None;
	bool is_signed       = false;
	bool encode_little;
	DREF DeeObject *result;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|Iuob:tobytes",
	                    &length, &byteorder, &is_signed))
		goto err;
	if (length == (size_t)-1) {
		/* Automatically determine. */
		length = int_reqbits(self, is_signed);
		if unlikely(length == (size_t)-1)
			goto err;
		/* Round up to the required number of bytes. */
		length = (length + 7) / 8;
	}
	if (DeeNone_Check(byteorder)) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		encode_little = true;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		encode_little = false;
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	} else {
		if (DeeObject_AssertTypeExact(byteorder, &DeeString_Type))
			goto err;
		if (DeeString_EQUALS_ASCII(byteorder, "little"))
			encode_little = true;
		else if (DeeString_EQUALS_ASCII(byteorder, "big"))
			encode_little = false;
		else {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid byteorder %r",
			                byteorder);
			goto err;
		}
	}
	/* Encode integer bytes. */
	result = DeeBytes_NewBufferUninitialized(length);
	if unlikely(!result)
		goto err;
	if (DeeInt_AsBytes((DeeObject *)self,
	                   DeeBytes_DATA(result),
	                   length, encode_little,
	                   is_signed))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_bitcount(DeeIntObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	struct keyword kwlist[] = { K(signed), KEND };
	bool is_signed = false;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|b:bitcount",
	                    &is_signed))
		goto err;
	result = int_reqbits(self, is_signed);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_frombytes(DeeObject *UNUSED(self), size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	struct keyword kwlist[] = { K(bytes), K(byteorder), K(signed), KEND };
	DeeObject *bytes;
	DeeObject *byteorder = Dee_None;
	bool is_signed       = false;
	bool encode_little;
	DeeBuffer buf;
	DREF DeeObject *result;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|ob:frombytes",
	                    &bytes, &byteorder, &is_signed))
		goto err;
	if (DeeNone_Check(byteorder)) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		encode_little = true;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		encode_little = false;
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	} else {
		if (DeeObject_AssertTypeExact(byteorder, &DeeString_Type))
			goto err;
		if (DeeString_EQUALS_ASCII(byteorder, "little"))
			encode_little = true;
		else if (DeeString_EQUALS_ASCII(byteorder, "big"))
			encode_little = false;
		else {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid byteorder %r",
			                byteorder);
			goto err;
		}
	}
	if (DeeObject_GetBuf(bytes, &buf, Dee_BUFFER_FREADONLY))
		goto err;
	result = DeeInt_FromBytes(buf.bb_base,
	                          buf.bb_size,
	                          encode_little,
	                          is_signed);
	DeeObject_PutBuf(bytes, &buf, Dee_BUFFER_FREADONLY);
	return result;
err:
	return NULL;
}

PRIVATE struct type_method tpconst int_class_methods[] = {
	{ "frombytes",
	  (dobjmethod_t)&int_frombytes,
	  DOC("(data:?DBytes,byteorder:?Dstring=!N,signed=!f)->?Dint\n"
	      "@param byteorder The byteorder encoding used by the returned bytes. "
	      "One of $\"little\" (for little-endian), $\"big\" (for big-endian) "
	      "or ?N (for host-endian)\n"
	      "@throw ValueError The given @byteorder string isn't recognized\n"
	      "The inverse of ?#tobytes, decoding a given bytes buffer @bytes to construct an integer"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_sizeof(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	size_t int_size;
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	int_size = (size_t)self->ob_size;
	if ((dssize_t)int_size < 0)
		int_size = (size_t) - (dssize_t)int_size;
	return DeeInt_NewSize(offsetof(DeeIntObject, ob_digit) +
	                      (int_size * sizeof(digit)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_forcecopy(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeIntObject *result;
	size_t int_size;
	if (DeeArg_Unpack(argc, argv, ":__forcecopy__"))
		goto err;
	int_size = (size_t)self->ob_size;
	if ((dssize_t)int_size < 0)
		int_size = (size_t) - (dssize_t)int_size;
	result = (DREF DeeIntObject *)DeeInt_Alloc(int_size);
	if unlikely(!result)
		goto err;
	result->ob_size = self->ob_size; /* Also copy the sign */
	memcpyc(result->ob_digit,
	        self->ob_digit,
	        int_size,
	        sizeof(digit));
	return result;
err:
	return NULL;
}


PRIVATE struct type_method tpconst int_methods[] = {
	{ "tostr",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_tostr,
	  DOC("(radix=!10,mode=!P{})->?Dstring\n"
	      "@throw ValueError The given @mode was not recognized\n"
	      "@throw NotImplemented The given @radix cannot be represented\n"
	      "Convert @this integer to a string, using @radix as base and a "
	      /**/ "character-options set @mode for which the following control "
	      /**/ "characters are recognized\n"
	      "#T{Option|Description~"
	      "$\"u\", $\"X\"|Digits above $10 are printed in upper-case&"
	      "$\"n\", $\"##\"|Prefix the integers with its number system prefix (e.g.: $\"0x\")&"
	      "$\"s\", $\"+\"|Also prepend a sign prefix before positive integers}"),
	  TYPE_METHOD_FKWDS },
	{ "hex",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_hex,
	  DOC("->?Dstring\n"
	      "Short-hand alias for ${this.tostr(16, \"n\")} (s.a. ?#tostr)") },
	{ "bin",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_bin,
	  DOC("->?Dstring\n"
	      "Short-hand alias for ${this.tostr(2, \"n\")} (s.a. ?#tostr)") },
	{ "oct",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_oct,
	  DOC("->?Dstring\n"
	      "Short-hand alias for ${this.tostr(8, \"n\")} (s.a. ?#tostr)") },
	{ "tobytes",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_tobytes,
	  DOC("(length?:?Dint,byteorder:?Dstring=!N,signed=!f)->?DBytes\n"
	      "@param byteorder The byteorder encoding used by the returned bytes. "
	      "One of $\"little\" (for little-endian), $\"big\" (for big-endian) "
	      "or ?N (for host-endian)\n"
	      "@throw IntegerOverflow @signed is ?f and @this integer is negative\n"
	      "@throw ValueError The given @byteorder string isn't recognized\n"
	      "Returns the data of @this integer as a @length bytes long "
	      "writable Bytes object that is disjunct from @this integer.\n"
	      "When @signed is ?f, throw an :IntegerOverflow if @this "
	      "integer is negative. Otherwise use two's complement to encode "
	      "negative integers"),
	  TYPE_METHOD_FKWDS },
	{ "bitcount",
	  (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_bitcount,
	  DOC("(signed=!f)->?Dint\n"
	      "@throw IntegerOverflow @signed is ?f and @this integer is negative\n"
	      "Return the number of bits needed to represent @this integer in base-2"),
	  TYPE_METHOD_FKWDS },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_sizeof,
	  DOC("->?Dint") },
	{ "__forcecopy__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&int_forcecopy,
	  DOC("->?Dint\n"
	      "Internal function to force the creation of a copy of @this "
	      "integer without performing aliasing for known constants.\n"
	      "This function is implementation-specific and used by tests "
	      "in order to ensure that inplace-optimization of certain "
	      "operators functions correctly") },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
int_get_bitcount_impl(DeeIntObject *__restrict self) {
	size_t asize;
	digit dig, mask;
	unsigned int addend;
	if (!self->ob_size)
		return 1;
	asize = (size_t)self->ob_size;
	if ((dssize_t)asize < 0) {
		err_integer_overflow((DeeObject *)self, 0, false);
		goto err;
	}
	for (;;) {
		if unlikely(!asize)
			return 1;
		--asize;
		dig = self->ob_digit[asize];
		if (dig)
			break;
	}
	mask   = 1;
	addend = 1;
	for (; mask < dig; mask = (mask << 1) | 1, ++addend)
		;
	return asize * DIGIT_BITS + addend;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_get_bitcount(DeeIntObject *__restrict self) {
	size_t result = int_get_bitcount_impl(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_get_bytecount(DeeIntObject *__restrict self) {
	size_t bits = int_get_bitcount_impl(self);
	size_t result;
	if unlikely(bits == (size_t)-1)
		goto err;
	result = bits / 8;
	if (bits & 7)
		++result;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE struct type_getset tpconst int_getsets[] = {
	{ "bitcount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&int_get_bitcount,
	  NULL,
	  NULL,
	  DOC("->?Dint\n"
	      "Returns the minimum number of bits that are required "
	      "in order to encode @this integer in unsigned two's complement") },
	{ "bytecount",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&int_get_bytecount,
	  NULL,
	  NULL,
	  DOC("->?Dint\n"
	      "Returns the minimum number of bytes that are required "
	      "in order to encode @this integer in unsigned two's complement") },
	{ NULL }
};


/* The max sequence size is the signed value of SIZE_MAX,
 * because negative values are reserved to indicate error
 * states. */
#if SSIZE_MAX > UINT32_MAX
INTERN DEFINE_UINT64(int_size_max, SSIZE_MAX);
#else /* SSIZE_MAX > UINT32_MAX */
INTERN DEFINE_UINT32(int_size_max, SSIZE_MAX);
#endif /* SSIZE_MAX < UINT32_MAX */

PRIVATE struct type_member tpconst int_class_members[] = {
	TYPE_MEMBER_CONST_DOC("SIZE_MAX", &int_size_max,
	                      "The max value acceptable for sequence sizes, or indices\n"
	                      "Note that despite its name, this constant is not necessarily "
	                      "equal to the well-known C-constant of the same name, accessible "
	                      "in deemon as ${(size_t from ctypes).max}\n"
	                      "Note that this value is guarantied to be sufficiently great, such that "
	                      "a sequence consisting of SIZE_MAX elements, each addressed as its own "
	                      "member, or modifiable index in some array, is impossible to achieve due "
	                      "to memory constraints.\n"
	                      "In this implementation, $SIZE_MAX is ${2**31} on 32-bit hosts, and ${2**63} on 64-bit hosts\n"
	                      "Custom, mutable sequences with sizes greater than this may experience inaccuracies "
	                      "with the default implementation of function such as :Sequence.insert's index-argument "
	                      "potentially not being able to correctly determine if a negative or positive number was given\n"
	                      "Such behavior may be considered a bug, however it falls under the category of doesn't-matter-wont-fix\n"),
	{ NULL }
};


PUBLIC DeeTypeObject DeeInt_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_int),
	/* .tp_doc      = */ DOC("The builtin type for representing and operating "
	                         "with whole numbers of an arbitrary precision\n"
	                         "Note that integers themself are immutable, and that "
	                         "inplace operators will change the pointed-object object\n"

	                         "\n"
	                         "()\n"
	                         "Returns the integer constant $0\n"

	                         "\n"
	                         "(ob)\n"
	                         "@throw NotImplemented The given @ob does not implement ${operator int}\n"
	                         "Converts @ob into an integer\n"

	                         "\n"
	                         "(s:?Dstring,radix=!0)\n"
	                         "(s:?DBytes,radix=!0)\n"
	                         "@throw ValueError The given string @s is not a valid integer\n"
	                         "@throw ValueError The given @radix is invalid\n"
	                         "Convert the given :string or :Bytes object @s into an integer\n"
	                         "When @radix is $0, automatically detect it based on a prefix such as $\"0x\". "
	                         "Otherwise, use @radix as it is provided\n"

	                         "\n"
	                         "str->\n"
	                         "repr->\n"
	                         "Returns @this integer as a decimal-encoded string. Same as ${this.tostr()}\n"

	                         "\n"
	                         "bool->\n"
	                         "Returns ?t if @this integer is non-zero\n"

	                         "\n"
	                         "==->\n"
	                         "!=->\n"
	                         "<->\n"
	                         "<=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "Compare @this integer with @other and return the result\n"

	                         "\n"
	                         "int->\n"
	                         "pos->\n"
	                         "Re-return @this integer\n"

	                         "\n"
	                         "inv->\n"
	                         "Return the result of ${-(this + 1)}. This matches the mathematical "
	                         "equivalent of a bit-wise inversion in 2'th complement arithmetic\n"

	                         "\n"
	                         "neg->\n"
	                         "Return @this integer with its sign prefix inverted\n"

	                         "\n"
	                         "add->\n"
	                         "Return the result of the addition between @this and @other\n"

	                         "\n"
	                         "sub->\n"
	                         "Return the result of subtracting @other from @this\n"

	                         "\n"
	                         "*->\n"
	                         "Multiply @this by @other and return the result\n"

	                         "\n"
	                         "/->\n"
	                         "@throw DivideByZero The given @other is $0\n"
	                         "Divide @this by @other and return the truncated result\n"

	                         "\n"
	                         "%->\n"
	                         "@throw DivideByZero The given @other is $0\n"
	                         "Divide @this by @other and return the remainder\n"

	                         "\n"
	                         "<<(count:?Dint)->\n"
	                         "@throw NegativeShift The given @count is lower than $0\n"
	                         "Shift the bits of @this left a total of @count times\n"

	                         "\n"
	                         ">>(count:?Dint)->\n"
	                         "@throw NegativeShift The given @count is lower than $0\n"
	                         "Shift the bits of @this right a total of @count times. "
	                         "All bits that fall off of the end are discarded\n"

	                         "\n"
	                         "&->\n"
	                         "Return the result of a bit-wise and between @this and @other\n"

	                         "\n"
	                         "|->\n"
	                         "Return the result of a bit-wise or between @this and @other\n"

	                         "\n"
	                         "^->\n"
	                         "Return the result of a bit-wise exclusive-or between @this and @other\n"

	                         "\n"
	                         "**->\n"
	                         "Return @this by the power of @other"),
	/* .tp_flags    = */ TP_FVARIABLE | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ &int_return_zero,
				/* .tp_copy_ctor = */ &DeeObject_NewRef, /* No need to actually copy. - Integers are immutable! */
				/* .tp_deep_ctor = */ &DeeObject_NewRef,
				/* .tp_any_ctor  = */ &int_new,
#if CONFIG_INT_CACHE_MAXCOUNT != 0
				/* .tp_free      = */ &DeeInt_Free
#else /* CONFIG_INT_CACHE_MAXCOUNT != 0 */
				/* .tp_free      = */ NULL
#endif /* CONFIG_INT_CACHE_MAXCOUNT == 0 */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ &int_str,
		/* .tp_repr = */ &int_str,
		/* .tp_bool = */ &int_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &int_math,
	/* .tp_cmp           = */ &int_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ int_methods,
	/* .tp_getsets       = */ int_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ int_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ int_class_members
};

/* Helpful singletons for some often used integers. */
PUBLIC DeeIntObject DeeInt_Zero     = { OBJECT_HEAD_INIT(&DeeInt_Type), 0, { 0 } };
PUBLIC DeeIntObject DeeInt_One      = { OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 1 } };
PUBLIC DeeIntObject DeeInt_MinusOne = { OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 1 } };

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_C */
