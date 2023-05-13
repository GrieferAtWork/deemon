/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/bit.h>
#include <hybrid/byteorder.h>
#include <hybrid/int128.h>
#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "int_logic.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h> /* CHAR_BIT */
#endif /* CONFIG_HAVE_LIMITS_H */

#ifndef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

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

/* Figure out the most efficient way to shift a 128-bit integer by `DIGIT_BITS', both left and right */
#if DIGIT_BITS < 8 && defined(__hybrid_uint128_shr8)
#define __hybrid_uint128_shr_DIGIT_BITS(var)           __hybrid_uint128_shr8(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS(var)           __hybrid_uint128_shl8(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS_overflows(var) __hybrid_uint128_shl8_overflows(var, DIGIT_BITS)
#elif DIGIT_BITS < 16 && defined(__hybrid_uint128_shr16)
#define __hybrid_uint128_shr_DIGIT_BITS(var)           __hybrid_uint128_shr16(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS(var)           __hybrid_uint128_shl16(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS_overflows(var) __hybrid_uint128_shl16_overflows(var, DIGIT_BITS)
#elif DIGIT_BITS < 32 && defined(__hybrid_uint128_shr32)
#define __hybrid_uint128_shr_DIGIT_BITS(var)               __hybrid_uint128_shr32(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS(var)               __hybrid_uint128_shl32(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS_overflows(var)     __hybrid_uint128_shl32_overflows(var, DIGIT_BITS)
#else /* ... */
#define __hybrid_uint128_shr_DIGIT_BITS(var)           __hybrid_uint128_shr64(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS(var)           __hybrid_uint128_shl64(var, DIGIT_BITS)
#define __hybrid_uint128_shl_DIGIT_BITS_overflows(var) __hybrid_uint128_shl64_overflows(var, DIGIT_BITS)
#endif /* !... */

#if DIGIT_BITS <= 8
#define __hybrid_uint128_least_significant_DIGIT_BITS(var) (digit)(__hybrid_uint128_vec8_significand(var, 0) & DIGIT_MASK)
#elif DIGIT_BITS <= 16
#define __hybrid_uint128_least_significant_DIGIT_BITS(var) (digit)(__hybrid_uint128_vec16_significand(var, 0) & DIGIT_MASK)
#elif DIGIT_BITS <= 32
#define __hybrid_uint128_least_significant_DIGIT_BITS(var) (digit)(__hybrid_uint128_vec32_significand(var, 0) & DIGIT_MASK)
#else /* DIGIT_BITS <= ... */
#define __hybrid_uint128_least_significant_DIGIT_BITS(var) (digit)(__hybrid_uint128_vec64_significand(var, 0) & DIGIT_MASK)
#endif /* DIGIT_BITS > ... */

/* Config option to statically pre-allocate all 8-bit integer constants (that is: [-128,255]). */
#if (!defined(CONFIG_STRING_8BIT_STATIC) && \
     !defined(CONFIG_STRING_8BIT_NORMAL))
#ifdef __OPTIMIZE_SIZE__
#define CONFIG_STRING_8BIT_NORMAL
#else /* __OPTIMIZE_SIZE__ */
#define CONFIG_STRING_8BIT_STATIC
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* ... */


DECL_BEGIN

#ifndef CONFIG_HAVE_memend
#define CONFIG_HAVE_memend
#undef memend
#define memend dee_memend
DeeSystem_DEFINE_memend(dee_memend)
#endif /* !CONFIG_HAVE_memend */

#if CONFIG_INT_CACHE_MAXCOUNT != 0
struct free_int {
	struct free_int *fi_next; /* [0..1] Next free integer. */
};
struct free_int_set {
	struct free_int  *fis_head; /* [0..1][lock(fis_lock)] First free integer object. */
	size_t            fis_size; /* [lock(fis_lock)][<= CONFIG_INT_CACHE_MAXSIZE]
	                             * Amount of free integer objects in this set. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t fis_lock; /* Lock for this free integer set. */
#endif  /* !CONFIG_NO_THREADS */
};

#define free_int_set_available(self)  Dee_atomic_lock_available(&(self)->fis_lock)
#define free_int_set_acquired(self)   Dee_atomic_lock_acquired(&(self)->fis_lock)
#define free_int_set_tryacquire(self) Dee_atomic_lock_tryacquire(&(self)->fis_lock)
#define free_int_set_acquire(self)    Dee_atomic_lock_acquire(&(self)->fis_lock)
#define free_int_set_waitfor(self)    Dee_atomic_lock_waitfor(&(self)->fis_lock)
#define free_int_set_release(self)    Dee_atomic_lock_release(&(self)->fis_lock)

PRIVATE struct free_int_set free_ints[CONFIG_INT_CACHE_MAXCOUNT];

INTERN size_t DCALL
Dee_intcache_clearall(size_t max_clear) {
	size_t i, result = 0;
	struct free_int_set *set;
	for (i = 0; i < COMPILER_LENOF(free_ints); ++i) {
		struct free_int *chain;
		struct free_int *chain_end;
		size_t total_free;
		set = &free_ints[i];
#ifndef CONFIG_NO_THREADS
		while (!free_int_set_tryacquire(set)) {
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
		free_int_set_release(set);
		/* Free all of the extracted chain elements. */
		while (chain) {
			chain_end = chain->fi_next;
			DeeObject_Free(chain);
			chain = chain_end;
		}
		if (result >= max_clear)
			break;
#ifndef CONFIG_NO_THREADS
next_set:
		;
#endif /* !CONFIG_NO_THREADS */
	}
	return result;
}

INTERN NONNULL((1)) void DCALL
DeeInt_Free(DeeIntObject *__restrict self) {
	size_t n_digits;
	n_digits = (size_t)self->ob_size;
	if (self->ob_size < 0)
		n_digits = (size_t)-self->ob_size;
	if (n_digits < CONFIG_INT_CACHE_MAXCOUNT) {
		struct free_int_set *set;
		set = &free_ints[n_digits];
#ifndef CONFIG_NO_THREADS
		while (!free_int_set_tryacquire(set)) {
			if (atomic_read(&set->fis_size) >= CONFIG_INT_CACHE_MAXSIZE)
				goto do_free;
			SCHED_YIELD();
		}
#endif  /* !CONFIG_NO_THREADS */
		COMPILER_READ_BARRIER();
		if (set->fis_size < CONFIG_INT_CACHE_MAXSIZE) {
			((struct free_int *)self)->fi_next = set->fis_head;
			set->fis_head                      = (struct free_int *)self;
			++set->fis_size;
			free_int_set_release(set);
			return;
		}
		free_int_set_release(set);
	}
#ifndef CONFIG_NO_THREADS
do_free:
#endif /* !CONFIG_NO_THREADS */
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
		while (!free_int_set_tryacquire(set)) {
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
			free_int_set_release(set);
			goto init_result;
		}
		free_int_set_release(set);
	}
#ifndef CONFIG_NO_THREADS
do_alloc:
#endif /* !CONFIG_NO_THREADS */
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
Dee_intcache_clearall(size_t UNUSED(max_clear)) {
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

#if defined(CONFIG_STRING_8BIT_STATIC) || defined(__DEEMON__)
typedef struct {
	Dee_OBJECT_HEAD
	Dee_ssize_t ob_size;
	Dee_digit_t ob_digit[1];
} DeeIntObject1Digit;
/*[[[deemon
print("PRIVATE DeeIntObject1Digit eightbit_blob[128 + 256] = {");
for (local i: [-128:256]) {
	print("	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), ",
		i < 0 ? -1 : i > 0 ? 1 : 0, ", { ",
		i.abs, " } }, /" "* ", i, " *" "/");
}
print("};");
]]]*/
PRIVATE DeeIntObject1Digit eightbit_blob[128 + 256] = {
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 128 } }, /* -128 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 127 } }, /* -127 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 126 } }, /* -126 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 125 } }, /* -125 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 124 } }, /* -124 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 123 } }, /* -123 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 122 } }, /* -122 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 121 } }, /* -121 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 120 } }, /* -120 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 119 } }, /* -119 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 118 } }, /* -118 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 117 } }, /* -117 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 116 } }, /* -116 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 115 } }, /* -115 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 114 } }, /* -114 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 113 } }, /* -113 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 112 } }, /* -112 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 111 } }, /* -111 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 110 } }, /* -110 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 109 } }, /* -109 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 108 } }, /* -108 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 107 } }, /* -107 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 106 } }, /* -106 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 105 } }, /* -105 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 104 } }, /* -104 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 103 } }, /* -103 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 102 } }, /* -102 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 101 } }, /* -101 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 100 } }, /* -100 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 99 } }, /* -99 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 98 } }, /* -98 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 97 } }, /* -97 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 96 } }, /* -96 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 95 } }, /* -95 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 94 } }, /* -94 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 93 } }, /* -93 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 92 } }, /* -92 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 91 } }, /* -91 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 90 } }, /* -90 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 89 } }, /* -89 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 88 } }, /* -88 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 87 } }, /* -87 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 86 } }, /* -86 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 85 } }, /* -85 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 84 } }, /* -84 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 83 } }, /* -83 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 82 } }, /* -82 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 81 } }, /* -81 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 80 } }, /* -80 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 79 } }, /* -79 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 78 } }, /* -78 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 77 } }, /* -77 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 76 } }, /* -76 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 75 } }, /* -75 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 74 } }, /* -74 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 73 } }, /* -73 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 72 } }, /* -72 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 71 } }, /* -71 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 70 } }, /* -70 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 69 } }, /* -69 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 68 } }, /* -68 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 67 } }, /* -67 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 66 } }, /* -66 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 65 } }, /* -65 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 64 } }, /* -64 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 63 } }, /* -63 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 62 } }, /* -62 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 61 } }, /* -61 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 60 } }, /* -60 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 59 } }, /* -59 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 58 } }, /* -58 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 57 } }, /* -57 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 56 } }, /* -56 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 55 } }, /* -55 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 54 } }, /* -54 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 53 } }, /* -53 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 52 } }, /* -52 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 51 } }, /* -51 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 50 } }, /* -50 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 49 } }, /* -49 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 48 } }, /* -48 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 47 } }, /* -47 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 46 } }, /* -46 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 45 } }, /* -45 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 44 } }, /* -44 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 43 } }, /* -43 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 42 } }, /* -42 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 41 } }, /* -41 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 40 } }, /* -40 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 39 } }, /* -39 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 38 } }, /* -38 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 37 } }, /* -37 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 36 } }, /* -36 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 35 } }, /* -35 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 34 } }, /* -34 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 33 } }, /* -33 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 32 } }, /* -32 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 31 } }, /* -31 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 30 } }, /* -30 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 29 } }, /* -29 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 28 } }, /* -28 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 27 } }, /* -27 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 26 } }, /* -26 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 25 } }, /* -25 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 24 } }, /* -24 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 23 } }, /* -23 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 22 } }, /* -22 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 21 } }, /* -21 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 20 } }, /* -20 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 19 } }, /* -19 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 18 } }, /* -18 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 17 } }, /* -17 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 16 } }, /* -16 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 15 } }, /* -15 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 14 } }, /* -14 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 13 } }, /* -13 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 12 } }, /* -12 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 11 } }, /* -11 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 10 } }, /* -10 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 9 } }, /* -9 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 8 } }, /* -8 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 7 } }, /* -7 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 6 } }, /* -6 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 5 } }, /* -5 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 4 } }, /* -4 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 3 } }, /* -3 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 2 } }, /* -2 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 1 } }, /* -1 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 0, { 0 } }, /* 0 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 1 } }, /* 1 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 2 } }, /* 2 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 3 } }, /* 3 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 4 } }, /* 4 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 5 } }, /* 5 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 6 } }, /* 6 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 7 } }, /* 7 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 8 } }, /* 8 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 9 } }, /* 9 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 10 } }, /* 10 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 11 } }, /* 11 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 12 } }, /* 12 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 13 } }, /* 13 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 14 } }, /* 14 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 15 } }, /* 15 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 16 } }, /* 16 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 17 } }, /* 17 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 18 } }, /* 18 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 19 } }, /* 19 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 20 } }, /* 20 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 21 } }, /* 21 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 22 } }, /* 22 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 23 } }, /* 23 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 24 } }, /* 24 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 25 } }, /* 25 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 26 } }, /* 26 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 27 } }, /* 27 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 28 } }, /* 28 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 29 } }, /* 29 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 30 } }, /* 30 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 31 } }, /* 31 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 32 } }, /* 32 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 33 } }, /* 33 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 34 } }, /* 34 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 35 } }, /* 35 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 36 } }, /* 36 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 37 } }, /* 37 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 38 } }, /* 38 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 39 } }, /* 39 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 40 } }, /* 40 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 41 } }, /* 41 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 42 } }, /* 42 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 43 } }, /* 43 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 44 } }, /* 44 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 45 } }, /* 45 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 46 } }, /* 46 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 47 } }, /* 47 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 48 } }, /* 48 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 49 } }, /* 49 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 50 } }, /* 50 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 51 } }, /* 51 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 52 } }, /* 52 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 53 } }, /* 53 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 54 } }, /* 54 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 55 } }, /* 55 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 56 } }, /* 56 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 57 } }, /* 57 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 58 } }, /* 58 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 59 } }, /* 59 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 60 } }, /* 60 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 61 } }, /* 61 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 62 } }, /* 62 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 63 } }, /* 63 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 64 } }, /* 64 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 65 } }, /* 65 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 66 } }, /* 66 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 67 } }, /* 67 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 68 } }, /* 68 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 69 } }, /* 69 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 70 } }, /* 70 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 71 } }, /* 71 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 72 } }, /* 72 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 73 } }, /* 73 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 74 } }, /* 74 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 75 } }, /* 75 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 76 } }, /* 76 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 77 } }, /* 77 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 78 } }, /* 78 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 79 } }, /* 79 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 80 } }, /* 80 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 81 } }, /* 81 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 82 } }, /* 82 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 83 } }, /* 83 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 84 } }, /* 84 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 85 } }, /* 85 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 86 } }, /* 86 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 87 } }, /* 87 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 88 } }, /* 88 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 89 } }, /* 89 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 90 } }, /* 90 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 91 } }, /* 91 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 92 } }, /* 92 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 93 } }, /* 93 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 94 } }, /* 94 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 95 } }, /* 95 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 96 } }, /* 96 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 97 } }, /* 97 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 98 } }, /* 98 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 99 } }, /* 99 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 100 } }, /* 100 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 101 } }, /* 101 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 102 } }, /* 102 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 103 } }, /* 103 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 104 } }, /* 104 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 105 } }, /* 105 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 106 } }, /* 106 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 107 } }, /* 107 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 108 } }, /* 108 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 109 } }, /* 109 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 110 } }, /* 110 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 111 } }, /* 111 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 112 } }, /* 112 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 113 } }, /* 113 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 114 } }, /* 114 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 115 } }, /* 115 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 116 } }, /* 116 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 117 } }, /* 117 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 118 } }, /* 118 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 119 } }, /* 119 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 120 } }, /* 120 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 121 } }, /* 121 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 122 } }, /* 122 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 123 } }, /* 123 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 124 } }, /* 124 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 125 } }, /* 125 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 126 } }, /* 126 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 127 } }, /* 127 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 128 } }, /* 128 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 129 } }, /* 129 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 130 } }, /* 130 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 131 } }, /* 131 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 132 } }, /* 132 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 133 } }, /* 133 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 134 } }, /* 134 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 135 } }, /* 135 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 136 } }, /* 136 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 137 } }, /* 137 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 138 } }, /* 138 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 139 } }, /* 139 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 140 } }, /* 140 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 141 } }, /* 141 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 142 } }, /* 142 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 143 } }, /* 143 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 144 } }, /* 144 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 145 } }, /* 145 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 146 } }, /* 146 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 147 } }, /* 147 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 148 } }, /* 148 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 149 } }, /* 149 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 150 } }, /* 150 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 151 } }, /* 151 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 152 } }, /* 152 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 153 } }, /* 153 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 154 } }, /* 154 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 155 } }, /* 155 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 156 } }, /* 156 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 157 } }, /* 157 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 158 } }, /* 158 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 159 } }, /* 159 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 160 } }, /* 160 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 161 } }, /* 161 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 162 } }, /* 162 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 163 } }, /* 163 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 164 } }, /* 164 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 165 } }, /* 165 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 166 } }, /* 166 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 167 } }, /* 167 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 168 } }, /* 168 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 169 } }, /* 169 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 170 } }, /* 170 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 171 } }, /* 171 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 172 } }, /* 172 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 173 } }, /* 173 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 174 } }, /* 174 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 175 } }, /* 175 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 176 } }, /* 176 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 177 } }, /* 177 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 178 } }, /* 178 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 179 } }, /* 179 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 180 } }, /* 180 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 181 } }, /* 181 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 182 } }, /* 182 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 183 } }, /* 183 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 184 } }, /* 184 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 185 } }, /* 185 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 186 } }, /* 186 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 187 } }, /* 187 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 188 } }, /* 188 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 189 } }, /* 189 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 190 } }, /* 190 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 191 } }, /* 191 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 192 } }, /* 192 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 193 } }, /* 193 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 194 } }, /* 194 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 195 } }, /* 195 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 196 } }, /* 196 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 197 } }, /* 197 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 198 } }, /* 198 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 199 } }, /* 199 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 200 } }, /* 200 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 201 } }, /* 201 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 202 } }, /* 202 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 203 } }, /* 203 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 204 } }, /* 204 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 205 } }, /* 205 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 206 } }, /* 206 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 207 } }, /* 207 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 208 } }, /* 208 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 209 } }, /* 209 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 210 } }, /* 210 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 211 } }, /* 211 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 212 } }, /* 212 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 213 } }, /* 213 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 214 } }, /* 214 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 215 } }, /* 215 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 216 } }, /* 216 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 217 } }, /* 217 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 218 } }, /* 218 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 219 } }, /* 219 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 220 } }, /* 220 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 221 } }, /* 221 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 222 } }, /* 222 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 223 } }, /* 223 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 224 } }, /* 224 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 225 } }, /* 225 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 226 } }, /* 226 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 227 } }, /* 227 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 228 } }, /* 228 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 229 } }, /* 229 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 230 } }, /* 230 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 231 } }, /* 231 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 232 } }, /* 232 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 233 } }, /* 233 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 234 } }, /* 234 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 235 } }, /* 235 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 236 } }, /* 236 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 237 } }, /* 237 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 238 } }, /* 238 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 239 } }, /* 239 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 240 } }, /* 240 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 241 } }, /* 241 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 242 } }, /* 242 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 243 } }, /* 243 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 244 } }, /* 244 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 245 } }, /* 245 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 246 } }, /* 246 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 247 } }, /* 247 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 248 } }, /* 248 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 249 } }, /* 249 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 250 } }, /* 250 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 251 } }, /* 251 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 252 } }, /* 252 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 253 } }, /* 253 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 254 } }, /* 254 */
	{ Dee_OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 255 } }, /* 255 */
};
/*[[[end]]]*/
#define eightbit (eightbit_blob + 128)
#endif /* CONFIG_STRING_8BIT_STATIC || __DEEMON__ */

/* Helpful singletons for some often used integers. */
PUBLIC struct _Dee_int_1digit_object DeeInt_MinusOne_Zero_One[3] = {
	{ OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 1 } },
	{ OBJECT_HEAD_INIT(&DeeInt_Type), 0, { 0 } }, /* Technically, zero doesn't needy a digit array */
	{ OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 1 } }
};





/* Create an integer from signed/unsigned LEB data. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_NewSleb(uint8_t const **__restrict p_reader) {
	uint8_t const *reader = *p_reader;
	DREF DeeIntObject *result;
	digit *dst;
	twodigits temp;
	uint8_t num_bits;
	size_t num_digits = 1;
	/* Figure out a worst-case for how many digits we'll be needing. */
	while (*reader++ & 0x80)
		++num_digits;
	num_digits = ((num_digits * 7 + (DIGIT_BITS - 1)) / DIGIT_BITS);
#ifdef CONFIG_STRING_8BIT_STATIC
	if (num_digits == 1) {
		uint8_t first_byte;
		/* See if maybe we can encode the value as one of the 8-bit constants. */
		reader     = *p_reader;
		num_bits   = 6;
		first_byte = *reader++;
		temp       = first_byte & 0x3f;
		while (reader[-1] & 0x80) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			++reader;
		}
		*p_reader = reader;
		if (first_byte & 0x40)
			return DeeInt_NewSTwoDigits(-(stwodigits)temp);
		return DeeInt_NewTwoDigits(temp);
	}
#endif /* CONFIG_STRING_8BIT_STATIC */
	result = DeeInt_Alloc(num_digits);
	if unlikely(!result)
		goto done;

	/* Read the integer. */
	reader   = *p_reader;
	dst      = result->ob_digit;
	num_bits = 6;
	temp     = *reader++ & 0x3f;
	for (;;) {
		while (num_bits < DIGIT_BITS && (reader[-1] & 0x80)) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			++reader;
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
					result = (DeeIntObject *)DeeInt_Zero;
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
	if (**p_reader & 0x40)
		result->ob_size = -result->ob_size;
done2:
	/* Save the new read position. */
	*p_reader = reader;
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_NewUleb(uint8_t const **__restrict p_reader) {
	uint8_t const *reader = *p_reader;
	DREF DeeIntObject *result;
	digit *dst;
	twodigits temp;
	uint8_t num_bits;
	size_t num_digits = 1;
	/* Figure out a worst-case for how many digits we'll be needing. */
	while (*reader++ & 0x80)
		++num_digits;
	num_digits = ((num_digits * 7 + (DIGIT_BITS - 1)) / DIGIT_BITS);
#ifdef CONFIG_STRING_8BIT_STATIC
	if (num_digits == 1) {
		/* See if maybe we can encode the value as one of the 8-bit constants. */
		reader   = *p_reader;
		num_bits = 7;
		temp     = *reader++ & 0x7f;
		while (reader[-1] & 0x80) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			++reader;
		}
		*p_reader = reader;
		return DeeInt_NewTwoDigits(temp);
	}
#endif /* CONFIG_STRING_8BIT_STATIC */
	result = DeeInt_Alloc(num_digits);
	if unlikely(!result)
		goto done;

	/* Read the integer. */
	reader   = *p_reader;
	num_bits = 7;
	temp     = *reader++ & 0x7f;
	dst      = result->ob_digit;
	for (;;) {
		while (num_bits < DIGIT_BITS && (reader[-1] & 0x80)) {
			/* Set the top-most 7 bits. */
			temp |= (twodigits)(*reader & 0x7f) << num_bits;
			num_bits += 7;
			if (!(*reader++ & 0x80)) {
				while (num_bits && !((temp >> (num_bits - 1)) & 1))
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
					result = (DeeIntObject *)DeeInt_Zero;
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
	*p_reader = reader;
done:
	return (DREF DeeObject *)result;
}


/* Write the value of an integer as signed/unsigned LEB data.
 * NOTE: When writing ULEB data, the caller is responsible to ensure that `self' is positive. */
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
		if (src < end && num_bits < 7) {
			/* Read one more digit. */
			temp |= (twodigits)*src++ << num_bits;
			num_bits += DIGIT_BITS;
			if (src >= end) {
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
		if (src < end && num_bits < 7) {
			/* Read one more digit. */
			temp |= (twodigits)*src++ << num_bits;
			num_bits += DIGIT_BITS;
			if (src >= end) {
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


PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewInt8(int8_t val) {
#ifdef CONFIG_STRING_8BIT_STATIC
	return_reference(eightbit + val);
#else /* CONFIG_STRING_8BIT_STATIC */
	DREF DeeIntObject *result;
	int sign        = 1;
	uint8_t abs_val = (uint8_t)val;
	if (val <= 0) {
		if (!val)
			return_reference_(DeeInt_Zero);
		sign    = -1;
		abs_val = (uint8_t)0 - (uint8_t)val;
	}
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = sign;
		result->ob_digit[0] = (digit)abs_val;
	}
	return (DREF DeeObject *)result;
#endif /* !CONFIG_STRING_8BIT_STATIC */
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewUInt8(uint8_t val) {
#ifdef CONFIG_STRING_8BIT_STATIC
	return_reference(eightbit + val);
#else /* CONFIG_STRING_8BIT_STATIC */
	DREF DeeIntObject *result;
	if (!val)
		return_reference_(DeeInt_Zero);
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = 1;
		result->ob_digit[0] = (digit)val;
	}
	return (DREF DeeObject *)result;
#endif /* !CONFIG_STRING_8BIT_STATIC */
}


PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewUInt16(uint16_t val) {
	DREF DeeIntObject *result;
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val <= 0xff)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
#if DIGIT_BITS >= 16
	if (!val)
		return_reference_(DeeInt_Zero);
	result = DeeInt_Alloc(1);
	if likely(result) {
		result->ob_size     = 1;
		result->ob_digit[0] = (digit)val;
	}
#elif DIGIT_BITS >= 8
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_(DeeInt_Zero);
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
#else /* DIGIT_BITS >= ... */
#error "Not implemented"
#endif /* DIGIT_BITS < ... */
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewUInt32(uint32_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	uint32_t iter;
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val <= 0xff)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_(DeeInt_Zero);
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewUInt64(uint64_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	uint64_t iter;

	/* When the CPU wasn't designed for 64-bit
	 * integers, prefer using 32-bit path. */
#if __SIZEOF_POINTER__ < 8
	if (val <= UINT32_MAX)
		return DeeInt_NewUInt32((uint32_t)val);
#else /* __SIZEOF_POINTER__ < 8 */
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val <= 0xff)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
#endif /* __SIZEOF_POINTER__ >= 8 */

		/* NOTE: 32 == Bits required to display everything in the range 0..UINT32_MAX */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
	if (!(val >> DIGIT_BITS)) {
		if (!val)
			return_reference_(DeeInt_Zero);
		/* Fast-path: The integer fits into a single digit. */
		result = DeeInt_Alloc(1);
		if likely(result) {
			result->ob_size     = 1;
			result->ob_digit[0] = (digit)val;
		}
	} else
#endif /* __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32 */
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewInt16(int16_t val) {
	DREF DeeIntObject *result;
	int sign         = 1;
	uint16_t abs_val = (uint16_t)val;
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val >= -128 && val <= 127)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
#if DIGIT_BITS >= 16
	if (val <= 0) {
		if (!val)
			return_reference_(DeeInt_Zero);
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
			return_reference_(DeeInt_Zero);
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
#else /* DIGIT_BITS >= ... */
#error "Not implemented"
#endif /* DIGIT_BITS < ... */
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewInt32(int32_t val) {
	DREF DeeIntObject *result;
	int sign;
	size_t req_digits;
	uint32_t iter, abs_val;
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val >= -128 && val <= 127)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
	sign = 1;
	abs_val = (uint32_t)val;
	if (val < 0) {
		sign    = -1;
		abs_val = (uint32_t)0 - (uint32_t)val;
	}
	if (!(abs_val >> DIGIT_BITS)) {
		if (!val)
			return_reference_(DeeInt_Zero);
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewInt64(int64_t val) {
	DREF DeeIntObject *result;
	int sign;
	size_t req_digits;
	uint64_t iter, abs_val;
#if __SIZEOF_POINTER__ < 8
	/* When the CPU wasn't designed for 64-bit
	 * integers, prefer using 32-bit path. */
	if (val >= INT32_MIN && val <= INT32_MAX)
		return DeeInt_NewInt32((int32_t)val);
#else /* __SIZEOF_POINTER__ < 8 */
#ifdef CONFIG_STRING_8BIT_STATIC
	if (val >= -128 && val <= 127)
		return_reference(eightbit + val);
#endif /* CONFIG_STRING_8BIT_STATIC */
#endif /* __SIZEOF_POINTER__ >= 8 */

	sign = 1;
	abs_val = (uint64_t)val;
	if (val < 0) {
		sign    = -1;
		abs_val = (uint64_t)0 - (uint64_t)val;
	}
	/* NOTE: 32 == Bits required to display everything in the range 0..MAX(-INT32_MIN, INT32_MAX) */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
	if (!(abs_val >> DIGIT_BITS)) {
		if (!val)
			return_reference_(DeeInt_Zero);
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
DeeInt_NewUInt128(Dee_uint128_t val) {
	DREF DeeIntObject *result;
	size_t req_digits;
	Dee_uint128_t iter;

	/* Simplification: When it fits into a 64-bit integer, use that path! */
	if (__hybrid_uint128_is64bit(val))
		return DeeInt_NewUInt64(__hybrid_uint128_get64(val));

	/* The remainder is basically the same as any other creator, but
	 * using special macros implementing some basic 128-bit arithmetic. */
	for (iter = val, req_digits = 0; !__hybrid_uint128_iszero(iter);
	     __hybrid_uint128_shr_DIGIT_BITS(iter), ++req_digits)
		;
	ASSERT(req_digits > 0);
	result = DeeInt_Alloc(req_digits);
	if likely(result) {
		result->ob_size = req_digits;
		for (req_digits = 0; !__hybrid_uint128_iszero(val);
		     __hybrid_uint128_shr_DIGIT_BITS(val), ++req_digits) {
			result->ob_digit[req_digits] = __hybrid_uint128_least_significant_DIGIT_BITS(val);
		}
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeInt_NewInt128(Dee_int128_t val) {
	DREF DeeIntObject *result;
	int sign;
	size_t req_digits;
	union {
		Dee_int128_t  s;
		Dee_uint128_t u;
	} iter, abs_val;

	/* Simplification: When it fits into a 64-bit integer, use that path! */
	if (__hybrid_int128_is64bit(val))
		return DeeInt_NewInt64(__hybrid_int128_get64(val));

	/* The remainder is basically the same as any other creator, but
	 * using special macros implementing some basic 128-bit arithmetic. */
	sign = 1;
	abs_val.s = val;
	if (__hybrid_int128_isneg(val)) {
		sign = -1;
		__hybrid_int128_neg(abs_val.s);
	}
	for (iter = abs_val, req_digits = 0; !__hybrid_int128_iszero(iter.s);
	     __hybrid_uint128_shr_DIGIT_BITS(iter.u), ++req_digits)
		;
	ASSERT(req_digits > 0);
	result = DeeInt_Alloc(req_digits);
	if likely(result) {
		result->ob_size = req_digits * sign;
		for (req_digits = 0; !__hybrid_int128_iszero(abs_val.s);
		     __hybrid_uint128_shr_DIGIT_BITS(abs_val.u), ++req_digits) {
			result->ob_digit[req_digits] = __hybrid_uint128_least_significant_DIGIT_BITS(abs_val.u);
		}
	}
	return (DREF DeeObject *)result;
}


#if DIGIT_BITS == 30
#define DeeInt_DECIMAL_SHIFT 9                   /* max(e such that 10**e fits in a digit) */
#define DeeInt_DECIMAL_BASE  ((digit)1000000000) /* 10 ** DECIMAL_SHIFT */
#else /* DIGIT_BITS == 30 */
#define DeeInt_DECIMAL_SHIFT 4              /* max(e such that 10**e fits in a digit) */
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
#else /* DIGIT_BITS == ... */
#error "Unsupported `DIGIT_BITS'"
#endif /* DIGIT_BITS != ... */
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
#else /* DIGIT_BITS == ... */
#error "Unsupported `DIGIT_BITS'"
#endif /* DIGIT_BITS != ... */
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
#else /* DIGIT_BITS == ... */
#error "Unsupported `DIGIT_BITS'"
#endif /* DIGIT_BITS != ... */
};
#endif /* CONFIG_USE_PRECALCULATED_INT_FROM_STRING_CONSTANTS */

LOCAL WUNUSED DREF DeeIntObject *DCALL
int_from_nonbinary_string(char const *__restrict begin,
                          char const *__restrict end,
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
		goto err;
	result->ob_size = 0;
	convwidth   = convwidth_base[radix];
	convmultmax = convmultmax_base[radix];
	while (begin < end) {
		c = 0;
		for (i = 0; i < convwidth && begin < end; ++i, ++begin) {
			digit dig;
			char ch;
parse_ch:
			ch = *begin;
			if (DeeUni_AsDigit(ch, 16, &dig)) {
				/* ... */
			} else if ((unsigned char)ch >= 0x80) {
				/* Unicode character? */
				uint32_t uni;
				struct unitraits *traits;
				uni    = utf8_readchar((char const **)&begin, end);
				traits = DeeUni_Descriptor(uni);
				if (traits->ut_flags & (UNICODE_ISNUMERIC | UNICODE_ISHEX)) {
					dig = traits->ut_digit_idx;
					if unlikely(dig >= Dee_UNICODE_DIGIT_IDENTITY_COUNT)
						dig = DeeUni_GetNumericIdx8((uint8_t)dig);
				} else {
					if (uni != '\\') {
						if (ch == '_' && !(radix_and_flags & DEEINT_STRING_FNOSEPS))
							goto do_skip_char;
						goto invalid_r;
					}
					goto handle_backslash_in_text;
				}
				--begin; /* Account for the additional `++begin' inside of for-advance */
			} else if (ch != '\\') {
				if (ch == '_' && !(radix_and_flags & DEEINT_STRING_FNOSEPS)) {
do_skip_char:
					++begin;
					goto parse_ch;
				}
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
					if (begin < end && *begin == '\n')
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
				*(digit *)mempcpyc(tmp->ob_digit, result->ob_digit,
				                   size_z, sizeof(digit)) = (digit)c;
				Dee_DecrefDokill(result);
				result = tmp;
				++size_z;
			}
		}
	}
	return result;
invalid_r:
	Dee_DecrefDokill(result);
	return (DREF DeeIntObject *)ITER_DONE;
err:
	return NULL;
}


/* Convert an integer to/from a string.
 * WARNING: The caller is responsible not to pass a radix equal to `1'.
 *          When a radix equal to `0', it is automatically determined from the passed string. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeInt_FromString(/*utf-8*/ char const *__restrict str,
                  size_t len, uint32_t radix_and_flags) {
	unsigned int radix = radix_and_flags >> DEEINT_STRING_RSHIFT;
	bool negative      = false;
	DREF DeeIntObject *result;
	char const *iter;
	char const *begin = str;
	char const *end   = str + len;
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
			char const *new_begin = begin + 1;
			begin_plus_one = utf8_readchar((char const **)&new_begin, end);
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
		char const *old_begin = begin;
		uint32_t leading_zero;
		leading_zero = utf8_readchar((char const **)&begin, end);
		if (DeeUni_AsDigitVal(leading_zero) == 0) {
			if (begin == end) /* Special case: int(0) */
				return_reference_(DeeInt_Zero);
			while (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				uint32_t begin_plus_one;
				char const *new_begin = begin + 1;
				begin_plus_one = utf8_readchar((char const **)&new_begin, end);
				if (DeeUni_IsLF(begin_plus_one)) {
					begin = new_begin;
					if (begin_plus_one == '\r' &&
					    *begin == '\n')
						++begin;
					continue;
				}
				break;
			}
			if (*begin == 'x' || *begin == 'X') {
				radix = 16, ++begin;
			} else if (*begin == 'b' || *begin == 'B') {
				radix = 2, ++begin;
			} else {
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
		dst      = result->ob_digit;
		number   = 0;
		num_bits = 0;

		/* Parse the integer starting with the least significant bits. */
		iter = end;
		while (iter > begin) {
			uint32_t ch;
			digit dig;
			struct unitraits *traits;
			ch     = utf8_readchar_rev((char const **)&iter, begin);
			traits = DeeUni_Descriptor(ch);
			if (traits->ut_flags & (UNICODE_ISNUMERIC | Dee_UNICODE_ISHEX)) {
				dig = traits->ut_digit_idx;
				if unlikely(dig >= Dee_UNICODE_DIGIT_IDENTITY_COUNT)
					dig = DeeUni_GetNumericIdx8((uint8_t)dig);
			} else if (DeeUni_IsLF(ch) && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
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
			} else if (ch == '_' && !(radix_and_flags & DEEINT_STRING_FNOSEPS)) {
				continue;
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
	char const *iter;
	char const *begin = str;
	char const *end = str + len;
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
		if (DeeUni_AsDigitVal(leading_zero) == 0) {
			++begin;
			if (begin == end) /* Special case: int(0) */
				return_reference_(DeeInt_Zero);
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
			if (*begin == 'x' || *begin == 'X') {
				radix = 16;
				++begin;
			} else if (*begin == 'b' || *begin == 'B') {
				radix = 2;
				++begin;
			} else {
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
			if (DeeUni_AsDigit(ch, 16, &dig)) {
				/* ... */
			} else if ((unsigned char)ch >= 0x80) {
				/* Unicode character? */
				uint32_t uni;
				struct unitraits *traits;
				++iter;
				uni    = utf8_readchar_rev((char const **)&iter, end);
				traits = DeeUni_Descriptor(uni);
				/* All any kind of digit/decimal character. - If the caller doesn't
				 * want to support any kind of digit, have `int("²")' evaluate to 2,
				 * then they have to verify that the string only contains ~conventional~
				 * decimals by using `string.isdigit()'. As far as this check is
				 * concerned, we accept anything that applies to `string.isnumeric()' */
				if (traits->ut_flags & (UNICODE_ISNUMERIC | UNICODE_ISHEX)) {
					dig = traits->ut_digit_idx;
					if unlikely(dig >= Dee_UNICODE_DIGIT_IDENTITY_COUNT)
						dig = DeeUni_GetNumericIdx8((uint8_t)dig);
				} else if (traits->ut_flags & Dee_UNICODE_ISLF) {
					goto handle_linefeed_in_text;
				} else {
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
	char const *iter;
	char const *begin = str;
	char const *end   = str + len;
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
			char const *new_begin = begin + 1;
			begin_plus_one = utf8_readchar((char const **)&new_begin, end);
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
		uint32_t leading_zero;
		char const *old_begin = begin;
		leading_zero = utf8_readchar((char const **)&begin, end);
		if (DeeUni_AsDigitVal(leading_zero) == 0) {
			if (begin == end) {
				/* Special case: int(0) */
				*value = 0;
				return 0;
			}
			while (*begin == '\\' && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
				uint32_t begin_plus_one;
				char const *new_begin = begin + 1;
				begin_plus_one = utf8_readchar((char const **)&new_begin, end);
				if (DeeUni_IsLF(begin_plus_one)) {
					begin = new_begin;
					if (begin_plus_one == '\r' &&
					    *begin == '\n')
						++begin;
					continue;
				}
				break;
			}
			if (*begin == 'x' || *begin == 'X') {
				radix = 16;
				++begin;
			} else if (*begin == 'b' || *begin == 'B') {
				radix = 2;
				++begin;
			} else {
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
		struct unitraits *traits;
		ch  = utf8_readchar_rev((char const **)&iter, begin);
		traits = DeeUni_Descriptor(ch);
		if (traits->ut_flags & (UNICODE_ISNUMERIC | UNICODE_ISHEX)) {
			dig = traits->ut_digit_idx;
			if unlikely(dig >= Dee_UNICODE_DIGIT_IDENTITY_COUNT)
				dig = DeeUni_GetNumericIdx8(dig);
		} else if (DeeUni_IsLF(ch) && (radix_and_flags & DEEINT_STRING_FESCAPED)) {
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

/* @return:  0: Successfully parsed an integer.
 * @return: -1: An error occurred. (never returned when `DEEINT_STRING_FTRY' is set)
 * @return:  1: Failed to parse an integer. (returned when `DEEINT_STRING_FTRY' is set) */
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


#define DECIMAL_THOUSANDS_GROUPINGS     3
#define NON_DECIMAL_THOUSANDS_GROUPINGS 4

PRIVATE WUNUSED NONNULL((1, 4)) dssize_t DCALL
DeeInt_PrintDecimal(DREF DeeIntObject *__restrict self, uint32_t flags,
                    size_t precision, dformatprinter printer, void *arg) {
	/* !!!DISCLAIMER!!! This function was originally taken from python,
	 *                  but has been heavily modified since. */
	size_t size, bufsize, size_a, i, j, intlen;
	dssize_t result, temp;
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
	pout = (digit *)Dee_Mallocac(size, sizeof(digit));
	if (!pout)
		goto err;
	pin  = self->ob_digit;
	size = 0;
	for (i = size_a; i--;) {
		digit hi = pin[i];
		for (j = 0; j < size; j++) {
			twodigits z = (twodigits)pout[j] << DIGIT_BITS | hi;
			hi = (digit)(z / DeeInt_DECIMAL_BASE);
			pout[j] = (digit)(z - (twodigits)hi * DeeInt_DECIMAL_BASE);
		}
		while (hi) {
			pout[size++] = hi % DeeInt_DECIMAL_BASE;
			hi /= DeeInt_DECIMAL_BASE;
		}
	}
	if (size == 0)
		pout[size++] = 0;
	bufsize = 1 + 1 + (size - 1) * DeeInt_DECIMAL_SHIFT;
	tenpow = 10;
	rem    = pout[size - 1];
	while (rem >= tenpow) {
		tenpow *= 10;
		++bufsize;
	}
	if (flags & DEEINT_PRINT_FSEPS) {
		/* Allocate a string target buffer. */
		bufsize += bufsize / DECIMAL_THOUSANDS_GROUPINGS;
		buf = (char *)Dee_Mallocac(bufsize, sizeof(char));
		if unlikely(!buf)
			goto err_pout;
		iter = buf + bufsize;
		intlen = 0;
		for (i = 0; i < size - 1; ++i) {
			rem = pout[i];
			for (j = 0; j < DeeInt_DECIMAL_SHIFT; ++j) {
				*--iter = '0' + rem % 10;
				++intlen;
				rem /= 10;
				if ((intlen % DECIMAL_THOUSANDS_GROUPINGS) == 0)
					*--iter = '_';
			}
		}
		rem = pout[i];
		do {
			*--iter = '0' + rem % 10;
			++intlen;
			rem /= 10;
			if ((intlen % DECIMAL_THOUSANDS_GROUPINGS) == 0 && rem != 0)
				*--iter = '_';
		} while (rem != 0);
	} else {
		/* Allocate a string target buffer. */
		buf = (char *)Dee_Mallocac(bufsize, sizeof(char));
		if unlikely(!buf)
			goto err_pout;
		iter = buf + bufsize;
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
		intlen = (size_t)((buf + bufsize) - iter);
	}
	if (precision > intlen) {
		size_t num_leading_zeroes;
		result = 0;
		if (negative || (flags & DEEINT_PRINT_FSIGN)) {
			temp = (*printer)(arg, negative ? "-" : "+", 1);
			if unlikely(temp < 0)
				goto err_temp;
			result = temp;
		}
		num_leading_zeroes = precision - intlen;
		if (flags & DEEINT_PRINT_FSEPS) {
			size_t first_sep_in_int, first_sep_in_pad;
			first_sep_in_int = (size_t)((char *)memend(iter, '_', (size_t)((buf + bufsize) - iter)) - iter);
			ASSERT(first_sep_in_int >= 1 && first_sep_in_int <= DECIMAL_THOUSANDS_GROUPINGS);
			first_sep_in_pad = (num_leading_zeroes - (DECIMAL_THOUSANDS_GROUPINGS -
			                                          first_sep_in_int)) %
			                   DECIMAL_THOUSANDS_GROUPINGS;
			ASSERT(/*first_sep_in_pad >= 0 &&*/ first_sep_in_pad < DECIMAL_THOUSANDS_GROUPINGS);
			if (num_leading_zeroes < first_sep_in_pad)
				goto do_normal_pad;
			if (first_sep_in_pad) {
				temp = DeeFormat_Repeat(printer, arg, '0', first_sep_in_pad);
				if unlikely(temp < 0)
					goto err_temp;
				result += temp;
				temp = (*printer)(arg, "_", 1);
				if unlikely(temp < 0)
					goto err_temp;
				result += temp;
				num_leading_zeroes -= first_sep_in_pad;
			}
			while (num_leading_zeroes >= DECIMAL_THOUSANDS_GROUPINGS) {
#if DECIMAL_THOUSANDS_GROUPINGS == 3
				temp = (*printer)(arg, "000", 3);
#else /* DECIMAL_THOUSANDS_GROUPINGS == 3 */
				temp = DeeFormat_Repeat(printer, arg, '0', DECIMAL_THOUSANDS_GROUPINGS);
#endif /* DECIMAL_THOUSANDS_GROUPINGS != 3 */
				if unlikely(temp < 0)
					goto err_temp;
				result += temp;
				temp = (*printer)(arg, "_", 1);
				if unlikely(temp < 0)
					goto err_temp;
				result += temp;
				num_leading_zeroes -= DECIMAL_THOUSANDS_GROUPINGS;
			}
		}
do_normal_pad:
		temp = DeeFormat_Repeat(printer, arg, '0', num_leading_zeroes);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if likely(result >= 0) {
			temp = (*printer)(arg, iter, (size_t)((buf + bufsize) - iter));
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	} else {
		if (negative) {
			*--iter = '-';
		} else if (flags & DEEINT_PRINT_FSIGN) {
			*--iter = '+';
		}
		intlen = (size_t)((buf + bufsize) - iter);
		result = (*printer)(arg, iter, intlen);
	}
done_buf:
	Dee_Freea(buf);
done_pout:
	Dee_Freea(pout);
	return result;
err_temp:
	result = temp;
	goto done_buf;
err_pout:
	result = -1;
	goto done_pout;
}


#undef DeeInt_Print_USES_UNICODE
#if 0 /* When defined, `DeeInt_Print()' might print non-ascii characters */
#define DeeInt_Print_USES_UNICODE
#endif

/* Print an integer to a given format-printer.
 * Radix must be one of `2', `4', `8', `10' or `16' and
 * if it isn't, a `NotImplemented' error is thrown.
 * This list of supported radices may be extended in the future. */
PUBLIC WUNUSED NONNULL((1, 4)) dssize_t DCALL
DeeInt_Print(DeeObject *__restrict self, uint32_t radix_and_flags,
             size_t precision, dformatprinter printer, void *arg) {
	ASSERT_OBJECT_TYPE(self, &DeeInt_Type);
	switch (radix_and_flags >> DEEINT_PRINT_RSHIFT) {

	case 10:
		return DeeInt_PrintDecimal((DeeIntObject *)self, radix_and_flags,
		                           precision, printer, arg);

		/* Power-of-2 radices. */
	case 2: {
		twodigits number;
		digit *src;
		uint8_t num_bits, dig_bits, dig_mask, dig;
		char *buf, *iter;
		size_t bufsize, num_digits, intlen;
		dssize_t result;
		DeeIntObject *me;
		char const *digit_chars;
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
		digit_chars = DeeAscii_ItoaDigits(radix_and_flags & DEEINT_PRINT_FUPPER);
		if ((dssize_t)num_digits <= 0) {
			if (!num_digits) {
				bufsize = 4;
				buf     = (char *)Dee_Mallocac(bufsize, sizeof(char));
				if unlikely(!buf)
					goto err;
				iter    = buf + bufsize;
				*--iter = '0';
				intlen  = 1;
				goto do_print_prefix;
			}
			num_digits = (size_t) - (dssize_t)num_digits;
		}
		bufsize = (num_digits * DIGIT_BITS) / dig_bits;
		if (radix_and_flags & DEEINT_PRINT_FSEPS)
			bufsize += bufsize / NON_DECIMAL_THOUSANDS_GROUPINGS;
		bufsize += 4;
		buf = (char *)Dee_Mallocac(bufsize, sizeof(char));
		if unlikely(!buf)
			goto err;
		iter     = buf + bufsize;
		src      = me->ob_digit;
		number   = 0;
		num_bits = 0;
		intlen   = 0;
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
				++intlen;
				if (radix_and_flags & DEEINT_PRINT_FSEPS) {
					if ((intlen % NON_DECIMAL_THOUSANDS_GROUPINGS) == 0 && (num_digits || num_bits))
						*--iter = '_';
				}
			}
		} while (num_digits);

		/* Print remaining bits. */
		if (num_bits) {
			dig     = number & dig_mask;
			*--iter = digit_chars[dig];
			++intlen;
		}
do_print_prefix:

		/* Deal with custom precisions */
		if (precision > intlen) {
			dssize_t temp;
			char prefix[3];
			size_t prefix_len = 0;
			size_t num_leading_zeroes;
			if (me->ob_size < 0) {
				prefix[prefix_len++] = '-';
			} else if (radix_and_flags & DEEINT_PRINT_FSIGN) {
				prefix[prefix_len++] = '+';
			}
			if (radix_and_flags & DEEINT_PRINT_FNUMSYS) {
				prefix[prefix_len++] = '0';
				if (dig_bits == 4)
					prefix[prefix_len++] = digit_chars[33]; /* x */
				if (dig_bits == 2)
					prefix[prefix_len++] = digit_chars[25]; /* q */
				if (dig_bits == 1)
					prefix[prefix_len++] = digit_chars[11]; /* b */
			}
			result = 0;
			if (prefix_len) {
				result = (*printer)(arg, prefix, prefix_len);
				if unlikely(result < 0)
					goto done_buf;
			}
			num_leading_zeroes = precision - intlen;
			if (radix_and_flags & DEEINT_PRINT_FSEPS) {
				size_t first_sep_in_int, first_sep_in_pad;
				first_sep_in_int = (size_t)((char *)memend(iter, '_', (size_t)((buf + bufsize) - iter)) - iter);
				ASSERT(first_sep_in_int >= 1 && first_sep_in_int <= NON_DECIMAL_THOUSANDS_GROUPINGS);
				first_sep_in_pad = (num_leading_zeroes - (NON_DECIMAL_THOUSANDS_GROUPINGS -
				                                          first_sep_in_int)) %
				                   NON_DECIMAL_THOUSANDS_GROUPINGS;
				ASSERT(/*first_sep_in_pad >= 0 &&*/ first_sep_in_pad < NON_DECIMAL_THOUSANDS_GROUPINGS);
				if (num_leading_zeroes < first_sep_in_pad)
					goto do_normal_pad;
				if (first_sep_in_pad) {
					temp = DeeFormat_Repeat(printer, arg, '0', first_sep_in_pad);
					if unlikely(temp < 0)
						goto err_temp;
					result += temp;
					temp = (*printer)(arg, "_", 1);
					if unlikely(temp < 0)
						goto err_temp;
					result += temp;
					num_leading_zeroes -= first_sep_in_pad;
				}
				while (num_leading_zeroes >= NON_DECIMAL_THOUSANDS_GROUPINGS) {
#if NON_DECIMAL_THOUSANDS_GROUPINGS == 4
					temp = (*printer)(arg, "0000", 4);
#else /* NON_DECIMAL_THOUSANDS_GROUPINGS == 4 */
					temp = DeeFormat_Repeat(printer, arg, '0', NON_DECIMAL_THOUSANDS_GROUPINGS);
#endif /* NON_DECIMAL_THOUSANDS_GROUPINGS != 4 */
					if unlikely(temp < 0)
						goto err_temp;
					result += temp;
					temp = (*printer)(arg, "_", 1);
					if unlikely(temp < 0)
						goto err_temp;
					result += temp;
					num_leading_zeroes -= NON_DECIMAL_THOUSANDS_GROUPINGS;
				}
			}
do_normal_pad:
			temp = DeeFormat_Repeat(printer, arg, '0', num_leading_zeroes);
			if unlikely(temp < 0) {
err_temp:
				result = temp;
				goto done_buf;
			}
			result += temp;
			temp = (*printer)(arg, iter, (size_t)((buf + bufsize) - iter));
			if unlikely(temp < 0) {
				result = temp;
				goto done_buf;
			}
			result += temp;
			goto done_buf;
		}

		/* Print the radix prefix. */
		if (radix_and_flags & DEEINT_PRINT_FNUMSYS) {
			if (dig_bits == 4)
				*--iter = digit_chars[33]; /* x */
			if (dig_bits == 2)
				*--iter = digit_chars[25]; /* q */
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
		intlen = (size_t)((buf + bufsize) - iter);
		result = (*printer)(arg, iter, intlen);
done_buf:
		Dee_Freea(buf);
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



/* Extract the 32-, 64- or 128-bit value of the given integer.
 * NOTE: In theory, deemon integers can have arbitrarily large
 *       values, however in deemon's C api, we must limit ourself
 *       to only a set number of bits.
 * @return: One of `INT_*' (See above) */
PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeInt_TryGet8Bit(DeeObject *__restrict self,
                  int8_t *__restrict value) {
	int result;
#if DIGIT_BITS <= 16
	int16_t digval;
	result = DeeInt_TryGet16Bit(self, &digval);
#else /* DIGIT_BITS <= 16 */
	int32_t digval;
	result = DeeInt_TryGet32Bit(self, &digval);
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

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeInt_TryGet16Bit(DeeObject *__restrict self,
                   int16_t *__restrict value) {
#if DIGIT_BITS <= 16
	DeeIntObject *me = (DeeIntObject *)self;
	uint16_t prev, result;
	bool negative;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeInt_Type);
	switch (me->ob_size) {
	case 0:
		*value = 0;
		return 0;
	case 1:
		*value = me->ob_digit[0];
		return INT_UNSIGNED;
	case -1:
		*value = -(int16_t)me->ob_digit[0];
		return INT_SIGNED;
	default: break;
	}
	result   = 0;
	prev     = 0;
	negative = false;
	i = me->ob_size;
	if (i < 0) {
		negative = true;
		i        = -i;
	}
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | me->ob_digit[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (negative) {
		if unlikely(result > UINT16_C(0x8000))
			goto overflow;
		*value = -(int16_t)result;
		return INT_SIGNED;
	}
	*value = (int16_t)result;
	return INT_UNSIGNED;
overflow:
	return negative ? INT_NEG_OVERFLOW : INT_POS_OVERFLOW;
#else /* DIGIT_BITS <= 16 */
	int32_t digval;
	int result = DeeInt_TryGet32Bit(self, &digval);
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

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeInt_TryGet32Bit(DeeObject *__restrict self,
                   int32_t *__restrict value) {
	DeeIntObject *me = (DeeIntObject *)self;
	uint32_t prev, result;
	bool negative;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeInt_Type);
	switch (me->ob_size) {
	case 0:
		*value = 0;
		return 0;
	case 1:
		*value = me->ob_digit[0];
		return INT_UNSIGNED;
	case -1:
		*value = -(int32_t)me->ob_digit[0];
		return INT_SIGNED;
	default: break;
	}
	result   = 0;
	prev     = 0;
	negative = false;
	i = me->ob_size;
	if (i < 0) {
		negative = true;
		i        = -i;
	}
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | me->ob_digit[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (negative) {
		if unlikely(result > UINT32_C(0x80000000))
			goto overflow;
		*value = -(int32_t)result;
		return INT_SIGNED;
	}
	*value = (int32_t)result;
	return INT_UNSIGNED;
overflow:
	return negative ? INT_NEG_OVERFLOW : INT_POS_OVERFLOW;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeInt_TryGet64Bit(DeeObject *__restrict self,
                   int64_t *__restrict value) {
	DeeIntObject *me = (DeeIntObject *)self;
	uint64_t prev, result;
	bool negative;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeInt_Type);
	switch (me->ob_size) {
	case 0:
		*value = 0;
		return 0;
	case 1:
		*value = me->ob_digit[0];
		return INT_UNSIGNED;
	case -1:
		*value = -(int64_t)me->ob_digit[0];
		return INT_SIGNED;
	default: break;
	}
	result   = 0;
	prev     = 0;
	negative = false;
	i = me->ob_size;
	if (i < 0) {
		negative = true;
		i        = -i;
	}
	while (--i >= 0) {
		result = (result << DIGIT_BITS) | me->ob_digit[i];
		if ((result >> DIGIT_BITS) != prev)
			goto overflow;
		prev = result;
	}
	if (negative) {
		if unlikely(result > UINT64_C(0x8000000000000000))
			goto overflow;
		*value = -(int64_t)result;
		return INT_SIGNED;
	}
	*value = (int64_t)result;
	return INT_UNSIGNED;
overflow:
	return negative ? INT_NEG_OVERFLOW : INT_POS_OVERFLOW;
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL
DeeInt_TryGet128Bit(DeeObject *__restrict self,
                    Dee_int128_t *__restrict value) {
	DeeIntObject *me = (DeeIntObject *)self;
	union {
		Dee_uint128_t u;
		Dee_int128_t  s;
	} result;
	bool negative;
	dssize_t i;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeInt_Type);
	switch (me->ob_size) {
	case 0:
		__hybrid_int128_setzero(*value);
		return 0;
	case 1:
		__hybrid_int128_vec64_significand(*value, 0) = me->ob_digit[0];
		__hybrid_int128_vec64_significand(*value, 1) = 0;
		return INT_UNSIGNED;
	case -1:
		__hybrid_int128_vec64_significand(*value, 0) = -(sdigit)me->ob_digit[0];
		__hybrid_int128_vec64_significand(*value, 1) = -1;
		return INT_SIGNED;
	default: break;
	}
	__hybrid_uint128_setzero(result.u);
	negative = false;
	i = me->ob_size;
	if (i < 0) {
		negative = true;
		i = -i;
	}
	while (--i >= 0) {
		if (__hybrid_uint128_shl_DIGIT_BITS_overflows(result.u))
			goto overflow;
		__hybrid_uint128_shl_DIGIT_BITS(result.u);
		__hybrid_uint128_or(result.u, me->ob_digit[i]);
	}
	if (negative) {
		static Dee_uint128_t const uint128_ill_pos =
		__HYBRID_UINT128_INIT16N(0x8000, 0x0000, 0x0000, 0x0000,
		                         0x0000, 0x0000, 0x0000, 0x0000);
		if (__hybrid_uint128_gr128(result.u, uint128_ill_pos))
			goto overflow;
		__hybrid_int128_neg(result.s);
		*value = result.s;
		return INT_SIGNED;
	}
	*value = result.s;
	return INT_UNSIGNED;
overflow:
	return negative ? INT_NEG_OVERFLOW : INT_POS_OVERFLOW;
}

/* Similar to the functions above, but explicitly require signed/unsigned 32/64-bit values. */
PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsInt8)(DeeObject *__restrict self,
                         int8_t *__restrict value) {
	int error = DeeInt_TryGet8Bit(self, value);
	if (error == INT_UNSIGNED && *(uint8_t const *)value > INT8_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsInt16)(DeeObject *__restrict self,
                          int16_t *__restrict value) {
	int error = DeeInt_TryGet16Bit(self, value);
	if (error == INT_UNSIGNED && *(uint16_t const *)value > INT16_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsInt32)(DeeObject *__restrict self,
                          int32_t *__restrict value) {
	int error = DeeInt_TryGet32Bit(self, value);
	if (error == INT_UNSIGNED && *(uint32_t const *)value > INT32_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsInt64)(DeeObject *__restrict self,
                          int64_t *__restrict value) {
	int error = DeeInt_TryGet64Bit(self, value);
	if (error == INT_UNSIGNED && *(uint64_t const *)value > INT64_MAX)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsInt128)(DeeObject *__restrict self,
                           Dee_int128_t *__restrict value) {
	int error = DeeInt_TryGet128Bit(self, value);
	if (error == INT_UNSIGNED && __hybrid_int128_isneg(*value))
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsUInt8)(DeeObject *__restrict self,
                          uint8_t *__restrict value) {
	int error = DeeInt_TryGet8Bit(self, (int8_t *)value);
	if (error == INT_SIGNED && *(int8_t const *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsUInt16)(DeeObject *__restrict self,
                           uint16_t *__restrict value) {
	int error = DeeInt_TryGet16Bit(self, (int16_t *)value);
	if (error == INT_SIGNED && *(int16_t const *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsUInt32)(DeeObject *__restrict self,
                           uint32_t *__restrict value) {
	int error = DeeInt_TryGet32Bit(self, (int32_t *)value);
	if (error == INT_SIGNED && *(int32_t const *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsUInt64)(DeeObject *__restrict self,
                           uint64_t *__restrict value) {
	int error = DeeInt_TryGet64Bit(self, (int64_t *)value);
	if (error == INT_SIGNED && *(int64_t const *)value < 0)
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) bool
(DCALL DeeInt_TryAsUInt128)(DeeObject *__restrict self,
                            Dee_uint128_t *__restrict value) {
	int error = DeeInt_TryGet128Bit(self, (Dee_int128_t *)value);
	if (error == INT_SIGNED && __hybrid_int128_isneg(*(Dee_int128_t const *)value))
		return false;
	return (error != INT_POS_OVERFLOW &&
	        error != INT_NEG_OVERFLOW);
}


/* Same as the functions above, but raise an `Error.ValueError.ArithmeticError.IntegerOverflow'
 * for `INT_POS_OVERFLOW' and `INT_NEG_OVERFLOW' and returns -1. */
PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_Get8Bit)(DeeObject *__restrict self, int8_t *__restrict value) {
	int result = DeeInt_TryGet8Bit(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 8, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_Get16Bit)(DeeObject *__restrict self, int16_t *__restrict value) {
	int result = DeeInt_TryGet16Bit(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 16, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_Get32Bit)(DeeObject *__restrict self, int32_t *__restrict value) {
	int result = DeeInt_TryGet32Bit(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 32, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_Get64Bit)(DeeObject *__restrict self, int64_t *__restrict value) {
	int result = DeeInt_TryGet64Bit(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 64, result == INT_POS_OVERFLOW);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_Get128Bit)(DeeObject *__restrict self, Dee_int128_t *__restrict value) {
	int result = DeeInt_TryGet128Bit(self, value);
	if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW)
		goto err_overflow;
	return result;
err_overflow:
	return err_integer_overflow(self, 128, result == INT_POS_OVERFLOW);
}

/* Read the signed/unsigned values from the given integer.
 * @return: 0:  Successfully read the value.
 * @return: -1: An error occurred (Integer overflow). */
PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsInt8)(DeeObject *__restrict self, int8_t *__restrict value) {
	int error = DeeInt_Get8Bit(self, value);
	if (error == INT_UNSIGNED && *value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 8, true);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsInt16)(DeeObject *__restrict self, int16_t *__restrict value) {
	int error = DeeInt_Get16Bit(self, value);
	if (error == INT_UNSIGNED && *value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 16, true);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsInt32)(DeeObject *__restrict self, int32_t *__restrict value) {
	int error = DeeInt_Get32Bit(self, value);
	if (error == INT_UNSIGNED && *value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 32, true);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsInt64)(DeeObject *__restrict self, int64_t *__restrict value) {
	int error = DeeInt_Get64Bit(self, value);
	if (error == INT_UNSIGNED && *value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 64, true);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsInt128)(DeeObject *__restrict self, Dee_int128_t *__restrict value) {
	int error = DeeInt_Get128Bit(self, value);
	if (error == INT_UNSIGNED && __hybrid_int128_isneg(*value))
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 128, true);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsUInt8)(DeeObject *__restrict self, uint8_t *__restrict value) {
	int error = DeeInt_Get8Bit(self, (int8_t *)value);
	if (error == INT_SIGNED && *(int8_t const *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 8, false);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsUInt16)(DeeObject *__restrict self, uint16_t *__restrict value) {
	int error = DeeInt_Get16Bit(self, (int16_t *)value);
	if (error == INT_SIGNED && *(int16_t const *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 16, false);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsUInt32)(DeeObject *__restrict self, uint32_t *__restrict value) {
	int error = DeeInt_Get32Bit(self, (int32_t *)value);
	if (error == INT_SIGNED && *(int32_t const *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 32, false);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsUInt64)(DeeObject *__restrict self, uint64_t *__restrict value) {
	int error = DeeInt_Get64Bit(self, (int64_t *)value);
	if (error == INT_SIGNED && *(int64_t const *)value < 0)
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 64, false);
}

PUBLIC WUNUSED ATTR_OUT(2) NONNULL((1)) int
(DCALL DeeInt_AsUInt128)(DeeObject *__restrict self, Dee_uint128_t *__restrict value) {
	int error = DeeInt_Get128Bit(self, (Dee_int128_t *)value);
	if (error == INT_SIGNED && __hybrid_int128_isneg(*(Dee_int128_t const *)value))
		goto err_overflow;
	return 0;
err_overflow:
	return err_integer_overflow(self, 128, false);
}


/* Convert an integer to a binary-encoded data array. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsBytes)(DeeObject *__restrict self,
                       void *__restrict dst, size_t length,
                       bool little_endian, bool as_signed) {
	DeeIntObject *me = (DeeIntObject *)self;
	uint8_t *writer;
	twodigits temp;
	size_t i, count, remaining;
	dssize_t incr;
	size_t num_bits;
	uint8_t leading_byte;
	digit last_digit;
	unsigned int last_bits;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeInt_Type);
	count = (size_t)me->ob_size;
	if unlikely(!count) {
		/* Special case: zero. */
		bzero(dst, length);
		return 0;
	}
	leading_byte = 0;
	if ((dssize_t)count < 0) {
		count = (size_t)(-(dssize_t)count);
		leading_byte = 0xff;
		if unlikely(!as_signed) {
			err_integer_overflow((DeeObject *)me, 0, false);
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
		temp |= (twodigits)me->ob_digit[i] << num_bits;
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
	last_digit = me->ob_digit[i];
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
	if (me->ob_size < 0) {
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
	err_integer_overflow((DeeObject *)me, length * 8, true);
err:
	return -1;
}

/* Convert a binary-encoded data array into an integer. */
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
	return_reference_((DeeObject *)DeeInt_MinusOne);
return_zero:
	return_reference_(DeeInt_Zero);
}





PRIVATE WUNUSED DREF DeeObject *DCALL int_return_zero(void) {
	return_reference_(DeeInt_Zero);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
int_new(size_t argc, DeeObject *const *argv) {
	DeeObject *val;
	uint16_t radix = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPu16 ":int", &val, &radix))
		goto err;
	if (DeeString_Check(val)) {
		char const *utf8 = DeeString_AsUtf8(val);
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
		return DeeInt_FromAscii((char const *)DeeBytes_DATA(val),
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
int_bool(DeeIntObject *__restrict self) {
	return self->ob_size != 0;
}




PRIVATE struct type_math int_math = {
	/* .tp_int32       = */ &DeeInt_Get32Bit,
	/* .tp_int64       = */ &DeeInt_Get64Bit,
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
		if (i < 0) {
			sign = 0;
		} else {
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
	sign = 1;
	x    = 0;
	if (i < 0) {
		sign = -1;
		i    = -i;
	}
	do {
		--i;
		x = (x << DIGIT_BITS) | (x >> ((__SIZEOF_POINTER__ * 8) - DIGIT_BITS));
		x += self->ob_digit[(size_t)i];
	} while ((size_t)i);
	return x * sign;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_eq(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_ne(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_lo(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value < 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_le(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value <= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_gr(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value > 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
int_cmp_ge(DeeIntObject *self, DeeObject *some_object) {
	dssize_t compare_value;
	DREF DeeIntObject *rhs;
	rhs = (DREF DeeIntObject *)DeeObject_Int(some_object);
	if unlikely(!rhs)
		goto err;
	compare_value = int_compareint(self, rhs);
	Dee_Decref(rhs);
	return_bool(compare_value >= 0);
err:
	return NULL;
}


PRIVATE struct type_cmp int_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&int_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&int_cmp_ge,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tostr_impl(DeeIntObject *__restrict self, uint32_t flags, size_t precision) {
#ifdef DeeInt_Print_USES_UNICODE
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(DeeInt_Print((DeeObject *)self, flags, precision,
	                         &unicode_printer_print,
	                         &printer) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#else /* DeeInt_Print_USES_UNICODE */
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(DeeInt_Print((DeeObject *)self, flags, precision,
	                         &ascii_printer_print,
	                         &printer) < 0)
		goto err_printer;
	return ascii_printer_pack(&printer);
err_printer:
	ascii_printer_fini(&printer);
	return NULL;
#endif /* !DeeInt_Print_USES_UNICODE */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_str(DeeIntObject *__restrict self) {
	return int_tostr_impl(self, DEEINT_PRINT_DEC, 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
int_print(DeeIntObject *__restrict self, dformatprinter printer, void *arg) {
	return DeeInt_Print((DeeObject *)self, DEEINT_PRINT_DEC, 0, printer, arg);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_tostr(DeeIntObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	PRIVATE struct keyword kwlist[] = { K(radix), K(precision), K(mode), KEND };
	size_t precision = 0;
	uint32_t flags_and_radix = 10 << DEEINT_PRINT_RSHIFT;
	char const *flags_str = NULL;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|" UNPu16 UNPuSIZ "s:tostr",
	                    &((uint16_t *)&flags_and_radix)[0], &precision, &flags_str))
		goto err;
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|" UNPu16 UNPuSIZ "s:tostr",
	                    &((uint16_t *)&flags_and_radix)[1], &precision, &flags_str))
		goto err;
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */
	if (flags_str) {
		char const *iter = flags_str;
		for (;;) {
			char ch = *iter++;
			if (!ch)
				break;
			if (ch == 'u' || ch == 'X') {
				flags_and_radix |= DEEINT_PRINT_FUPPER;
			} else if (ch == 'n' || ch == '#') {
				flags_and_radix |= DEEINT_PRINT_FNUMSYS;
			} else if (ch == 's' || ch == '+') {
				flags_and_radix |= DEEINT_PRINT_FSIGN;
			} else if (ch == '_') {
				flags_and_radix |= DEEINT_PRINT_FSEPS;
			} else {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid integer to flags:?Dstring %q",
				                flags_str);
				goto err;
			}
		}
	}
	return int_tostr_impl(self, flags_and_radix, precision);
err:
	return NULL;
}

INTERN struct keyword precision_kwlist[] = { K(precision), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_hex(DeeIntObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t precision = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, precision_kwlist,
	                    "|" UNPuSIZ ":hex", &precision))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(16, DEEINT_PRINT_FNUMSYS), precision);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_bin(DeeIntObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t precision = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, precision_kwlist,
	                    "|" UNPuSIZ ":bin", &precision))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(2, DEEINT_PRINT_FNUMSYS), precision);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_oct(DeeIntObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t precision = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, precision_kwlist,
	                    "|" UNPuSIZ ":oct", &precision))
		goto err;
	return int_tostr_impl(self, DEEINT_PRINT(8, DEEINT_PRINT_FNUMSYS), precision);
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
	if ((dssize_t)digit_count < 0) {
		if unlikely(!is_signed)
			goto err_underflow;
		digit_count = (size_t) - (dssize_t)digit_count;
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
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|" UNPuSIZ "ob:tobytes",
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
		if (DeeString_EQUALS_ASCII(byteorder, "little")) {
			encode_little = true;
		} else if (DeeString_EQUALS_ASCII(byteorder, "big")) {
			encode_little = false;
		} else {
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
		if (DeeString_EQUALS_ASCII(byteorder, "little")) {
			encode_little = true;
		} else if (DeeString_EQUALS_ASCII(byteorder, "big")) {
			encode_little = false;
		} else {
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
	TYPE_KWMETHOD("frombytes", &int_frombytes,
	              "(data:?DBytes,byteorder:?Dstring=!N,signed=!f)->?.\n"
	              "@param byteorder The byteorder encoding used by the returned bytes. "
	              /*            */ "One of $\"little\" (for little-endian), $\"big\" "
	              /*            */ "(for big-endian) or ?N (for host-endian)\n"
	              "@throw ValueError The given @byteorder string isn't recognized\n"
	              "The inverse of ?#tobytes, decoding a given bytes buffer @bytes to "
	              /**/ "construct an integer"),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_sizeof(DeeIntObject *__restrict self) {
	size_t int_size;
	int_size = (size_t)self->ob_size;
	if ((dssize_t)int_size < 0)
		int_size = (size_t)(-(dssize_t)int_size);
	return DeeInt_NewSize(offsetof(DeeIntObject, ob_digit) +
	                      (int_size * sizeof(digit)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_forcecopy(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeIntObject *result;
	size_t int_size;
	if (DeeArg_Unpack(argc, argv, ":__forcecopy__"))
		goto err;
	int_size = (size_t)self->ob_size;
	if ((dssize_t)int_size < 0)
		int_size = (size_t)(-(dssize_t)int_size);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
int_divmod_f(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DREF DeeIntObject *div, *rem;
	DREF DeeIntObject *y;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:divmod", &y))
		goto err;
	y = (DeeIntObject *)DeeObject_Int((DeeObject *)y);
	if unlikely(!y)
		goto err;
	error = int_divmod(self, y, &div, &rem);
	Dee_Decref(y);
	if unlikely(error)
		goto err;
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_divrem;
	DeeTuple_SET(result, 0, (DREF DeeObject *)div); /* Inherit reference */
	DeeTuple_SET(result, 1, (DREF DeeObject *)rem); /* Inherit reference */
	return result;
err_divrem:
	Dee_Decref_likely(div);
	Dee_Decref_likely(rem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_nextafter(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeIntObject *y;
	dssize_t diff;
	if (DeeArg_Unpack(argc, argv, "o:nextafter", &y))
		goto err;
	y = (DREF DeeIntObject *)DeeObject_Int((DeeObject *)y);
	if unlikely(!y)
		goto err;
	diff = int_compareint(self, y);
	Dee_Decref(y);
	Dee_Incref(self);
	if (diff < 0) {
		if unlikely(int_inc(&self))
			goto err_self;
	} else if (diff > 0) {
		if unlikely(int_dec(&self))
			goto err_self;
	}
	return self;
err_self:
	Dee_Decref(self);
err:
	return NULL;
}

#define DEFINE_INT_COMPARE_FUNCTION(name, cmp)                            \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL                    \
	int_##name(DeeIntObject *self, size_t argc, DeeObject *const *argv) { \
		DREF DeeIntObject *y;                                             \
		dssize_t diff;                                                    \
		if (DeeArg_Unpack(argc, argv, "o:" #name, &y))                    \
			goto err;                                                     \
		y = (DREF DeeIntObject *)DeeObject_Int((DeeObject *)y);           \
		if unlikely(!y)                                                   \
			goto err;                                                     \
		diff = int_compareint(self, y);                                   \
		Dee_Decref(y);                                                    \
		return_bool_(diff cmp 0);                                         \
err:                                                                      \
		return NULL;                                                      \
	}
DEFINE_INT_COMPARE_FUNCTION(isgreater, >)
DEFINE_INT_COMPARE_FUNCTION(isgreaterequal, >=)
DEFINE_INT_COMPARE_FUNCTION(isless, <)
DEFINE_INT_COMPARE_FUNCTION(islessequal, <=)
DEFINE_INT_COMPARE_FUNCTION(islessgreater, !=)
#undef DEFINE_INT_COMPARE_FUNCTION

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_isunordered(DeeIntObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *y;
	(void)self;
	if (DeeArg_Unpack(argc, argv, "o:isunordered", &y))
		goto err;
	return DeeObject_GetAttrString(y, "isnan");
err:
	return NULL;
}

DOC_REF(numeric_hex_doc);
DOC_REF(numeric_bin_doc);
DOC_REF(numeric_oct_doc);
DOC_REF(numeric_divmod_doc);
DOC_REF(numeric_nextafter_doc);
DOC_REF(numeric_isgreater_doc);
DOC_REF(numeric_isgreaterequal_doc);
DOC_REF(numeric_isless_doc);
DOC_REF(numeric_islessequal_doc);
DOC_REF(numeric_islessgreater_doc);
DOC_REF(numeric_isunordered_doc);

PRIVATE struct type_method tpconst int_methods[] = {
	TYPE_KWMETHOD(STR_tostr, &int_tostr,
	              "(radix=!10,precision=!0,mode=!P{})->?Dstring\n"
	              "@param precision The minimum number of digits (excluding radix/sign "
	              /*            */ "prefixes) to print. Padding is done using $'0'-chars.\n"
	              "@throw ValueError The given @mode was not recognized\n"
	              "@throw NotImplemented The given @radix cannot be represented\n"
	              "Convert @this integer to a string, using @radix as base and a "
	              /**/ "character-options set @mode for which the following control "
	              /**/ "characters are recognized\n"
	              "#T{Option|Description~"
	              "$\"u\", $\"X\"|Digits above $10 are printed in upper-case&"
	              "$\"n\", $\"##\"|Prefix the integers with its number system prefix (e.g.: $\"0x\")&"
	              "$\"s\", $\"+\"|Also prepend a sign prefix before positive integers&"
	              "$\"_\"|Include canonical thousands/group-separators}"),
	TYPE_KWMETHOD("hex", &int_hex, numeric_hex_doc),
	TYPE_KWMETHOD("bin", &int_bin, numeric_bin_doc),
	TYPE_KWMETHOD("oct", &int_oct, numeric_oct_doc),
	TYPE_KWMETHOD("tobytes", &int_tobytes,
	              "(length?:?.,byteorder:?Dstring=!N,signed=!f)->?DBytes\n"
	              "@param byteorder The byteorder encoding used by the returned bytes. "
	              /*            */ "One of $\"little\" (for little-endian), $\"big\" (for big-endian) "
	              /*            */ "or ?N (for host-endian)\n"
	              "@throw IntegerOverflow @signed is ?f and @this integer is negative\n"
	              "@throw ValueError The given @byteorder string isn't recognized\n"
	              "Returns the data of @this integer as a @length bytes long "
	              /**/ "writable Bytes object that is disjunct from @this integer.\n"
	              "When @signed is ?f, throw an :IntegerOverflow if @this "
	              /**/ "integer is negative. Otherwise use two's complement to encode "
	              /**/ "negative integers"),
	TYPE_KWMETHOD("bitcount", &int_bitcount,
	              "(signed=!f)->?.\n"
	              "@throw IntegerOverflow @signed is ?f and @this integer is negative\n"
	              "Return the number of bits needed to represent @this integer in base-2"),
	TYPE_METHOD("divmod", &int_divmod_f, numeric_divmod_doc),
	TYPE_METHOD("nextafter", &int_nextafter,
	            "(y:?.)->?.\n"
	            "Same as ${this > y ? this - 1 : this < y ? this + 1 : this}"),
	TYPE_METHOD("isgreater", &int_isgreater,
	            "(y:?.)->?Dbool\n"
	            "Same as ${this > y}"),
	TYPE_METHOD("isgreaterequal", &int_isgreaterequal,
	            "(y:?.)->?Dbool\n"
	            "Same as ${this >= y}"),
	TYPE_METHOD("isless", &int_isless,
	            "(y:?.)->?Dbool\n"
	            "Same as ${this < y}"),
	TYPE_METHOD("islessequal", &int_islessequal,
	            "(y:?.)->?Dbool\n"
	            "Same as ${this <= y}"),
	TYPE_METHOD("islessgreater", &int_islessgreater,
	            "(y:?.)->?Dbool\n"
	            "Same as ${this != y}"),
	TYPE_METHOD("isunordered", &int_isunordered,
	            "(y:?X2?.?Dfloat)->?Dbool\n"
	            "Same as ${y is float && y.isnan}"),
	TYPE_METHOD("__forcecopy__", &int_forcecopy,
	            "->?.\n"
	            "Internal function to force the creation of a copy of @this "
	            /**/ "integer without performing aliasing for known constants.\n"
	            "This function is implementation-specific and used by tests "
	            /**/ "in order to ensure that inplace-optimization of certain "
	            /**/ "operators functions correctly"),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_abs(DeeIntObject *__restrict self) {
	if (self->ob_size < 0)
		return (DREF DeeIntObject *)int_neg(self);
	return_reference_(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_return_nonzero(DeeIntObject *__restrict self) {
	return_bool(self->ob_size != 0);
}


INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_popcount(DeeIntObject *__restrict self) {
	size_t result, i;
	if unlikely(self->ob_size < 0)
		goto err_neg;
	result = 0;
	for (i = 0; i < (size_t)self->ob_size; ++i) {
		digit dig = self->ob_digit[i];
		result += POPCOUNT(dig);
	}
	return (DREF DeeIntObject *)DeeInt_NewSize(result);
err_neg:
	err_int_negative((DeeObject *)self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_ffs(DeeIntObject *__restrict self) {
	size_t result, i;
	if unlikely(self->ob_size < 0)
		goto err_neg;
	if unlikely(self->ob_size == 0)
		return_reference_((DeeIntObject *)DeeInt_Zero);
	result = 1;
	for (i = 0;; ++i) {
		digit dig;
		ASSERT(i < (size_t)self->ob_size);
		dig = self->ob_digit[i];
		if (dig) {
			result += CTZ(dig);
			break;
		}
		result += DIGIT_BITS;
	}
	return (DREF DeeIntObject *)DeeInt_NewSize(result);
err_neg:
	err_int_negative((DeeObject *)self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_partity(DeeIntObject *__restrict self) {
	__SHIFT_TYPE__ result;
	size_t i;
	digit sum;
	if unlikely(DeeInt_IsNeg(self))
		goto err_neg;
	sum = 0;
	for (i = 0; i < (size_t)self->ob_size; ++i)
		sum ^= self->ob_digit[i];
	result = PARITY(sum);
	return (DREF DeeIntObject *)DeeInt_NEWU(result);
err_neg:
	err_int_negative((DeeObject *)self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_ctz(DeeIntObject *__restrict self) {
	size_t result, i;
	if unlikely(self->ob_size < 0)
		goto err_neg;
	if unlikely(self->ob_size == 0)
		return_reference_((DeeIntObject *)DeeInt_Zero);
	result = 0;
	for (i = 0;; ++i) {
		digit dig;
		ASSERT(i < (size_t)self->ob_size);
		dig = self->ob_digit[i];
		if (dig) {
			result += CTZ(dig);
			break;
		}
		result += DIGIT_BITS;
	}
	return (DREF DeeIntObject *)DeeInt_NewSize(result);
err_neg:
	err_int_negative((DeeObject *)self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_get_msb(DeeIntObject *__restrict self) {
	size_t result;
	digit msb_digit;
	if unlikely(self->ob_size <= 0)
		goto err_neg_or_zero;
	result    = ((size_t)self->ob_size - 1) * DIGIT_BITS;
	msb_digit = self->ob_digit[(size_t)self->ob_size - 1];
	result += sizeof(digit) * CHAR_BIT;
	result -= CLZ(msb_digit);
	return (DREF DeeIntObject *)DeeInt_NewSize(result);
err_neg_or_zero:
	err_int_negative_or_zero((DeeObject *)self);
	return NULL;
}

/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_m3rd", "-3rd");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_m2nd", "-2nd");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_m1st", "-1st");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_p1st", "1st");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_p2nd", "2nd");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_p3rd", "3rd");
]]]*/
PRIVATE DEFINE_STRING_EX(str_m3rd, "-3rd", 0x8be040f2, 0x2c6687ce52b2d5c);
PRIVATE DEFINE_STRING_EX(str_m2nd, "-2nd", 0xb0b2978a, 0xf0d5319d0acbc254);
PRIVATE DEFINE_STRING_EX(str_m1st, "-1st", 0xbcbf5323, 0x4d73a7bc07d3085e);
PRIVATE DEFINE_STRING_EX(str_p1st, "1st", 0xe4efb48d, 0x421421a9365d165c);
PRIVATE DEFINE_STRING_EX(str_p2nd, "2nd", 0x7c14eb33, 0x424c37e55ef4e885);
PRIVATE DEFINE_STRING_EX(str_p3rd, "3rd", 0xc941bdb2, 0xb6806ce92422760e);
/*[[[end]]]*/

PRIVATE DeeObject *const str_mNTH[3] = { (DeeObject *)&str_m1st, (DeeObject *)&str_m2nd, (DeeObject *)&str_m3rd };
PRIVATE DeeObject *const str_pNTH[3] = { (DeeObject *)&str_p1st, (DeeObject *)&str_p2nd, (DeeObject *)&str_p3rd };

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
int_get_nth(DeeIntObject *__restrict self) {
#ifdef DeeInt_Print_USES_UNICODE
	struct unicode_printer printer;
#else /* DeeInt_Print_USES_UNICODE */
	struct ascii_printer printer;
#endif /* !DeeInt_Print_USES_UNICODE */
	switch (self->ob_size) {
	case -1:
		if (self->ob_digit[0] >= 1 && self->ob_digit[0] <= 3)
			return_reference_(str_mNTH[self->ob_digit[0] - 1]);
		break;
	case 1:
		if (self->ob_digit[0] >= 1 && self->ob_digit[0] <= 3)
			return_reference_(str_pNTH[self->ob_digit[0] - 1]);
		break;
	default:
		break;
	}
#ifdef DeeInt_Print_USES_UNICODE
	unicode_printer_init(&printer);
	if unlikely(DeeInt_Print((DeeObject *)self, DEEINT_PRINT_DEC,
	                         0, &unicode_printer_print, &printer) < 0)
		goto err_printer;
	if unlikely(unicode_printer_print(&printer, "th", 2) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
#else /* DeeInt_Print_USES_UNICODE */
	ascii_printer_init(&printer);
	if unlikely(DeeInt_Print((DeeObject *)self, DEEINT_PRINT_DEC,
	                         0, &ascii_printer_print, &printer) < 0)
		goto err_printer;
	if unlikely(ascii_printer_print(&printer, "th", 2) < 0)
		goto err_printer;
	return ascii_printer_pack(&printer);
err_printer:
	ascii_printer_fini(&printer);
	return NULL;
#endif /* !DeeInt_Print_USES_UNICODE */
}



PRIVATE struct type_getset tpconst int_getsets[] = {
	TYPE_GETTER("__sizeof__", &int_sizeof, "->?."),
	TYPE_GETTER("abs", &int_get_abs, "->?."),
	TYPE_GETTER("trunc", &DeeObject_NewRef, "->?."),
	TYPE_GETTER("floor", &DeeObject_NewRef, "->?."),
	TYPE_GETTER("ceil", &DeeObject_NewRef, "->?."),
	TYPE_GETTER("round", &DeeObject_NewRef, "->?."),
	TYPE_GETTER("isnormal", &int_return_nonzero, "->?Dbool\nSame as {this != 0}"),

	/* Binary property helper functions */
	TYPE_GETTER("popcount", &int_get_popcount,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "Return the number of 1-bits in this integer"),
	TYPE_GETTER("ffs", &int_get_ffs,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "FindFirstSet: same as ?#ctz +1, but returns $0 when ${this == 0}"),
	TYPE_GETTER("partity", &int_get_partity,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "Return $0 or $1 indivative of the even/odd parity of @this. Same as ${this.popcount % 2}"),
	TYPE_GETTER("ctz", &int_get_ctz,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "CountTrailingZeros: return the number of trailing zero-bits:\n"
	            "${"
	            /**/ "local n = this.ctz;\n"
	            /**/ "assert this == (this >> n) << n;"
	            "}"),
	TYPE_GETTER("msb", &int_get_msb,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this <= 0}\n"
	            "MostSignificantBit: return the index of the most significant 1-bit:\n"
	            "${"
	            /**/ "assert (this >> this.msb) == 1;"
	            "}"),

	TYPE_GETTER("nth", &int_get_nth,
	            "->?Dstring\n"
	            "Returns the value of @this ?. as a string (as per ?#op:str), with "
	            "the standard english enumeration suffix applicable to the value of @{this}:\n"
	            "#T{Value|Return~"
	            "$-3|$\"-3rd\"&"
	            "$-2|$\"-2nd\"&"
	            "$-1|$\"-1st\"&"
	            "$1|$\"1st\"&"
	            "$2|$\"2nd\"&"
	            "$3|$\"3rd\"}\n"
	            "All other integer values are returned as ${f\"{this}th\"}"),

	TYPE_GETTER(STR_int, &DeeObject_NewRef,
	            "->?Dint\n"
	            "Always re-return @this"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst int_members[] = {
	TYPE_MEMBER_CONST(STR_isfloat, Dee_False),
	TYPE_MEMBER_CONST("isnan", Dee_False),
	TYPE_MEMBER_CONST("isinf", Dee_False),
	TYPE_MEMBER_CONST("isfinite", Dee_True),
	TYPE_MEMBER_END
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
	                      /**/ "equal to the well-known C-constant of the same name, accessible "
	                      /**/ "in deemon as ${(size_t from ctypes).max}\n"
	                      "Note that this value is guarantied to be sufficiently great, such that "
	                      /**/ "a sequence consisting of SIZE_MAX elements, each addressed as its own "
	                      /**/ "member, or modifiable index in some array, is impossible to achieve due "
	                      /**/ "to memory constraints.\n"
	                      "In this implementation, $SIZE_MAX is ${2**31} on 32-bit hosts, and ${2**63} on 64-bit hosts\n"
	                      "Custom, mutable sequences with sizes greater than this may experience inaccuracies "
	                      /**/ "with the default implementation of function such as :Sequence.insert's index-argument "
	                      /**/ "potentially not being able to correctly determine if a negative or positive number was given\n"
	                      "Such behavior may be considered a bug, however it falls under the category of doesn't-matter-wont-fix\n"),
	TYPE_MEMBER_CONST(STR_isfloat, Dee_False),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeInt_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_int),
	/* .tp_doc      = */ DOC("The builtin type for representing and operating "
	                         /**/ "with whole numbers of an arbitrary precision\n"
	                         "Note that integers themself are immutable, and that "
	                         /**/ "inplace operators will change the pointed-object object\n"
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
	                         /**/ "Otherwise, use @radix as it is provided\n"
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
	                         /**/ "equivalent of a bit-wise inversion in 2'th complement arithmetic\n"
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

	                         "<<(count:?.)->\n"
	                         "@throw NegativeShift The given @count is lower than $0\n"
	                         "Shift the bits of @this left a total of @count times\n"
	                         "\n"

	                         ">>(count:?.)->\n"
	                         "@throw NegativeShift The given @count is lower than $0\n"
	                         "Shift the bits of @this right a total of @count times. "
	                         /**/ "All bits that fall off of the end are discarded\n"
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
				/* .tp_ctor      = */ (dfunptr_t)&int_return_zero,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef, /* No need to actually copy. - Integers are immutable! */
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)&int_new,
#if CONFIG_INT_CACHE_MAXCOUNT != 0
				/* .tp_free      = */ (dfunptr_t)&DeeInt_Free
#else /* CONFIG_INT_CACHE_MAXCOUNT != 0 */
				/* .tp_free      = */ (dfunptr_t)NULL
#endif /* CONFIG_INT_CACHE_MAXCOUNT == 0 */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&int_str,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&int_str,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&int_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&int_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&int_print
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
	/* .tp_members       = */ int_members,
	/* .tp_class_methods = */ int_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ int_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_C */
