/* NOTE: Deemon's integer object implementation is
 *       heavily based on python's `long' data type.
 *       With that in mind, licensing of deemon's integer
 *       implementation must be GPL-compatible, GPL being
 *       the license that python is restricted by.
 *    >> So to simplify this whole deal: I make no claim of having invented the
 *       way that deemon's (phyton's) arbitrary-length integers are implemented,
 *       with all algorithms found in `int_logic.c' originating from python
 *       before being adjusted to fit deemon's runtime.
 *       To further discourage use of code found here, in favor of the original
 *       creator's work, comments have been removed.
 *       I DID NOT WRITE (most) STUFF IN THIS FILE
 */
#ifndef GUARD_DEEMON_OBJECTS_INT_LOGIC_C
#define GUARD_DEEMON_OBJECTS_INT_LOGIC_C 1

#include "int_logic.h"

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/thread.h>

#include <hybrid/align.h>
#include <hybrid/bit.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>

#include "../runtime/runtime_error.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

#undef shift_t
#define shift_t __SHIFT_TYPE__

DECL_BEGIN

#define KARATSUBA_CUTOFF         70
#define KARATSUBA_SQUARE_CUTOFF (2 * KARATSUBA_CUTOFF)
#define FIVEARY_CUTOFF           8

#if DIGIT_BITS <= 31
#define DeeInt_NewMedian(x) DeeInt_NewInt32(x)
#else /* DIGIT_BITS <= 31 */
#define DeeInt_NewMedian(x) DeeInt_NewInt64(x)
#endif /* DIGIT_BITS > 31 */


#define SWAP(T, a, b)         \
	do {                      \
		T const _temp_ = (a); \
		(a) = (b);            \
		(b) = _temp_;         \
	}	__WHILE0
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MEDIUM_VALUE(x)                               \
	(ASSERT(-1 <= (x)->ob_size && (x)->ob_size <= 1), \
	 (x)->ob_size < 0                                 \
	 ? -(sdigit)(x)->ob_digit[0]                      \
	 : ((x)->ob_size == 0                             \
	    ? (sdigit)0                                   \
	    : (sdigit)(x)->ob_digit[0]))
#define SIGCHECK(...)                     \
	do {                                  \
		if (DeeThread_CheckInterrupt()) { \
			__VA_ARGS__;                  \
		}                                 \
	}	__WHILE0

#define maybe_small_int(x) x
#ifdef CONFIG_SIGNED_RIGHT_SHIFT_ZERO_FILLS
#define ARITHMETIC_RIGHT_SHIFT(type, i, j) \
	((i) < 0 ? -1 - ((-1 - (i)) >> (j)) : (i) >> (j))
#else /* CONFIG_SIGNED_RIGHT_SHIFT_ZERO_FILLS */
#define ARITHMETIC_RIGHT_SHIFT(type, i, j) ((i) >> (j))
#endif /* !CONFIG_SIGNED_RIGHT_SHIFT_ZERO_FILLS */



PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_normalize(/*inherit(always)*/ DREF DeeIntObject *__restrict v) {
	dssize_t j = ABS(v->ob_size);
	dssize_t i = j;
	while (i > 0 && v->ob_digit[i - 1] == 0)
		--i;
	if (i != j)
		v->ob_size = (v->ob_size < 0) ? -i : i;
	return v;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_neg(DeeIntObject *__restrict self) {
	DREF DeeIntObject *result;
	size_t num_digits;
	if (ABS(self->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewMedian(-MEDIUM_VALUE(self));
	num_digits = (size_t)ABS(self->ob_size);
	result     = DeeInt_Alloc(num_digits);
	if (result) {
		memcpyc(result->ob_digit, self->ob_digit,
		        num_digits, sizeof(digit));
		result->ob_size = -self->ob_size;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_neg_inherit(/*inherit(always)*/ DREF DeeIntObject *self) {
	DREF DeeIntObject *result;
	if (!DeeObject_IsShared(self)) {
		self->ob_size = -self->ob_size;
		return self;
	}
	result = int_neg(self);
	Dee_Decref_unlikely(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
x_add(DeeIntObject *a, DeeIntObject *b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t size_b = ABS(b->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit carry = 0;
	if (size_a < size_b) {
		SWAP(DeeIntObject *, a, b);
		SWAP(dssize_t, size_a, size_b);
	}
	z = DeeInt_Alloc(size_a + 1);
	if unlikely(!z)
		goto err;
	for (i = 0; i < size_b; ++i) {
		carry += a->ob_digit[i] + b->ob_digit[i];
		z->ob_digit[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	for (; i < size_a; ++i) {
		carry += a->ob_digit[i];
		z->ob_digit[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	z->ob_digit[i] = carry;
	return int_normalize(z);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
x_sub(DeeIntObject *a, DeeIntObject *b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t size_b = ABS(b->ob_size);
	dssize_t i;
	DeeIntObject *z;
	int sign     = 1;
	digit borrow = 0;
	if (size_a < size_b) {
		sign = -1;
		SWAP(DeeIntObject *, a, b);
		SWAP(dssize_t, size_a, size_b);
	} else if (size_a == size_b) {
		i = size_a;
		while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i])
			;
		if (i < 0)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		if (a->ob_digit[i] < b->ob_digit[i]) {
			DeeIntObject *temp;
			sign = -1;
			temp = a;
			a    = b;
			b    = temp;
		}
		size_a = size_b = i + 1;
	}
	z = DeeInt_Alloc(size_a);
	if unlikely(!z)
		goto err;
	for (i = 0; i < size_b; ++i) {
		borrow = a->ob_digit[i] - b->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	for (; i < size_a; ++i) {
		borrow = a->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	ASSERT(borrow == 0);
	if (sign < 0)
		z->ob_size = -z->ob_size;
	return int_normalize(z);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_add(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *z, *b;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
		z = (DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) +
		                                     MEDIUM_VALUE(b));
		goto done;
	}
	if (a->ob_size < 0) {
		if (b->ob_size < 0) {
			z = x_add(a, b);
			if (likely(z) && z->ob_size)
				z->ob_size = -z->ob_size;
		} else {
			z = x_sub(b, a);
		}
	} else if (b->ob_size < 0) {
		z = x_sub(a, b);
	} else {
		z = x_add(a, b);
	}
done:
	Dee_Decref(b);
	return z;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_sub(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *z, *b;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
		z = (DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) -
		                                     MEDIUM_VALUE(b));
		goto done;
	}
	if (a->ob_size < 0) {
		if (b->ob_size < 0) {
			z = x_sub(a, b);
		} else {
			z = x_add(a, b);
		}
		if (z != NULL /*&& z->ob_size != 0*/)
			z->ob_size = -z->ob_size;
	} else {
		if (b->ob_size < 0) {
			z = x_add(a, b);
		} else {
			z = x_sub(a, b);
		}
	}
done:
	Dee_Decref(b);
	return z;
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_add_int(DeeIntObject *__restrict a, digit b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit carry;
	ASSERT(size_a >= 2);
	z = DeeInt_Alloc(size_a + 1);
	if unlikely(!z)
		goto err;
	carry = a->ob_digit[0] + b;
	z->ob_digit[0] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	for (i = 1; i < size_a; ++i) {
		carry += a->ob_digit[i];
		z->ob_digit[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	z->ob_digit[i] = carry;
	return int_normalize(z);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_add_int2(DeeIntObject *__restrict a, twodigits b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit carry;
	ASSERT(size_a >= 2);
	z = DeeInt_Alloc(size_a + 1);
	if unlikely(!z)
		goto err;
	carry          = a->ob_digit[0] + (b & DIGIT_MASK);
	z->ob_digit[0] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	carry += a->ob_digit[1] + ((b >> DIGIT_BITS) & DIGIT_MASK);
	z->ob_digit[1] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	for (i = 2; i < size_a; ++i) {
		carry += a->ob_digit[i];
		z->ob_digit[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	z->ob_digit[i] = carry;
	return int_normalize(z);
err:
	return NULL;
}

#if (DIGIT_BITS * 2) < 32
PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_add_int3(DeeIntObject *__restrict a, uint32_t b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit carry;
	ASSERT(size_a >= 2);
	if (size_a == 2) {
		uint64_t a_value;
		a_value = a->ob_digit[0];
		a_value |= (uint64_t)a->ob_digit[1] << DIGIT_BITS;
		if (a->ob_size < 0)
			return (DREF DeeIntObject *)DeeInt_NewInt64((-(int64_t)a_value) + (int64_t)b);
		return (DREF DeeIntObject *)DeeInt_NewUInt64(a_value + b);
	}
	ASSERT(size_a >= 3);
	z = DeeInt_Alloc(size_a + 1);
	if unlikely(!z)
		goto err;
	carry          = a->ob_digit[0] + (b & DIGIT_MASK);
	z->ob_digit[0] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	carry += a->ob_digit[1] + ((b >> DIGIT_BITS) & DIGIT_MASK);
	z->ob_digit[1] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	carry += a->ob_digit[2] + ((b >> (DIGIT_BITS * 2)) & DIGIT_MASK);
	z->ob_digit[2] = carry & DIGIT_MASK;
	carry >>= DIGIT_BITS;
	for (i = 3; i < size_a; ++i) {
		carry += a->ob_digit[i];
		z->ob_digit[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	z->ob_digit[i] = carry;
	return int_normalize(z);
err:
	return NULL;
}
#endif /* (DIGIT_BITS * 2) < 32 */


PRIVATE WUNUSED NONNULL((2)) DREF DeeIntObject *DCALL
x_sub_revint(digit a, DeeIntObject *__restrict b) {
	dssize_t size_b = ABS(b->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit borrow;
	ASSERT(size_b >= 2);
	z = DeeInt_Alloc(size_b);
	if unlikely(!z)
		goto err;
	borrow = b->ob_digit[0] - a;
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	for (i = 1; i < size_b; ++i) {
		borrow = b->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	ASSERT(borrow == 0);
	z->ob_size = -z->ob_size;
	return int_normalize(z);
err:
	return NULL;
}

#if (DIGIT_BITS * 2) < 32
PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_sub_int3(DeeIntObject *__restrict a, uint32_t b);

PRIVATE WUNUSED NONNULL((2)) DREF DeeIntObject *DCALL
x_sub_revint3(uint32_t a, DeeIntObject *__restrict b) {
	dssize_t size_b = ABS(b->ob_size);
	DeeIntObject *z;
	digit borrow;
	ASSERT(size_b >= 2);
	if (3 < size_b) {
		z = x_sub_int3(b, a);
		if (z)
			z->ob_size = -z->ob_size;
		return z;
	}
	ASSERT(size_b == 2 || size_b == 3);
	if (3 == size_b) {
		uint64_t b_value;
		b_value = b->ob_digit[2];
		b_value <<= DIGIT_BITS;
		b_value |= b->ob_digit[1];
		b_value <<= DIGIT_BITS;
		b_value |= b->ob_digit[0];
		if (a == b_value)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		if (a < b_value) {
			z = (DeeIntObject *)DeeInt_NewUInt64(b_value - a);
			if (z)
				z->ob_size = -z->ob_size;
			return z;
		}
		return (DeeIntObject *)DeeInt_NewUInt64(a - b_value);
	}
	ASSERT(size_b == 2);
	z = DeeInt_Alloc(3);
	if unlikely(!z)
		goto err;
	borrow = (a & DIGIT_MASK) - b->ob_digit[0];
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = ((a >> DIGIT_BITS) & DIGIT_MASK) - b->ob_digit[1] - borrow;
	z->ob_digit[1] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = ((a >> (DIGIT_BITS * 2)) & DIGIT_MASK) - borrow;
	z->ob_digit[2] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	ASSERT(borrow == 0);
	return int_normalize(z);
err:
	return NULL;
}
#endif /* (DIGIT_BITS * 2) < 32 */


PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_sub_int(DeeIntObject *__restrict a, digit b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit borrow;
	ASSERT(size_a >= 2);
	z = DeeInt_Alloc(size_a);
	if unlikely(!z)
		goto err;
	borrow = a->ob_digit[0] - b;
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	for (i = 1; i < size_a; ++i) {
		borrow = a->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	ASSERT(borrow == 0);
	return int_normalize(z);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_sub_int2(DeeIntObject *__restrict a, twodigits b) {
	dssize_t size_a = ABS(a->ob_size);
	dssize_t i;
	DeeIntObject *z;
	digit borrow;
	ASSERT(size_a >= 2);
	if (size_a == 2) {
		twodigits a_value;
		a_value = a->ob_digit[1];
		a_value <<= DIGIT_BITS;
		a_value |= a->ob_digit[0];
		if (a_value == b)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		if (a_value < b) {
			b -= a_value;
			if (b <= DIGIT_MASK) {
				z = DeeInt_Alloc(1);
				if unlikely(!z)
					goto err;
				z->ob_digit[0] = (digit)b;
				z->ob_size = -1;
			} else {
				z = DeeInt_Alloc(2);
				if unlikely(!z)
					goto err;
				z->ob_digit[0] = b & DIGIT_MASK;
				z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
				z->ob_size = -2;
			}
			return z;
		}
		a_value -= b;
		if (a_value <= DIGIT_MASK) {
			z = DeeInt_Alloc(1);
			if unlikely(!z)
				goto err;
			z->ob_digit[0] = (digit)a_value;
		} else {
			z = DeeInt_Alloc(2);
			if unlikely(!z)
				goto err;
			z->ob_digit[0] = a_value & DIGIT_MASK;
			z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
		}
		return z;
	}
	z = DeeInt_Alloc(size_a);
	if unlikely(!z)
		goto err;
	borrow = a->ob_digit[0] - (b & DIGIT_MASK);
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = a->ob_digit[1] - ((b >> DIGIT_BITS) & DIGIT_MASK) - borrow;
	z->ob_digit[1] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	for (i = 2; i < size_a; ++i) {
		borrow = a->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	ASSERT(borrow == 0);
	return int_normalize(z);
err:
	return NULL;
}

#if (DIGIT_BITS * 2) < 32
PRIVATE WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
x_sub_int3(DeeIntObject *__restrict a, uint32_t b) {
	dssize_t size_a = ABS(a->ob_size);
	digit borrow;
	dssize_t i;
	DeeIntObject *z;
	ASSERT(size_a >= 2);
	if (size_a == 2) {
		z = x_sub_revint3(b, a);
		if (z)
			z->ob_size = -z->ob_size;
		return z;
	}
	if (size_a == 3) {
		uint64_t a_value;
		a_value = a->ob_digit[2];
		a_value <<= DIGIT_BITS;
		a_value |= a->ob_digit[1];
		a_value <<= DIGIT_BITS;
		a_value |= a->ob_digit[0];
		if (a_value == b)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		if (a_value < b) {
			b -= (uint32_t)a_value;
			if (b <= DIGIT_MASK) {
				z = DeeInt_Alloc(1);
				if unlikely(!z)
					goto err;
				z->ob_digit[0] = (digit)b;
				z->ob_size = -1;
			} else if (b <= ((twodigits)1 << (DIGIT_BITS * 2)) - 1) {
				z = DeeInt_Alloc(2);
				if unlikely(!z)
					goto err;
				z->ob_digit[0] = b & DIGIT_MASK;
				z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
				z->ob_size = -2;
			} else {
				z = DeeInt_Alloc(3);
				if unlikely(!z)
					goto err;
				z->ob_digit[0] = b & DIGIT_MASK;
				z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
				z->ob_digit[2] = (b >> (DIGIT_BITS * 2)) & DIGIT_MASK;
				z->ob_size = -3;
			}
			return z;
		}
		a_value -= b;
		if (a_value <= DIGIT_MASK) {
			z = DeeInt_Alloc(1);
			if unlikely(!z)
				goto err;
			z->ob_digit[0] = (digit)a_value;
		} else if (a_value <= (((twodigits)1 << (DIGIT_BITS * 2)) - 1)) {
			z = DeeInt_Alloc(2);
			if unlikely(!z)
				goto err;
			z->ob_digit[0] = a_value & DIGIT_MASK;
			z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
		} else {
			z = DeeInt_Alloc(3);
			if unlikely(!z)
				goto err;
			z->ob_digit[0] = a_value & DIGIT_MASK;
			z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
			z->ob_digit[2] = (a_value >> (DIGIT_BITS * 2)) & DIGIT_MASK;
		}
		return z;
	}
	z = DeeInt_Alloc(size_a);
	if unlikely(!z)
		goto err;
	borrow = a->ob_digit[0] - (b & DIGIT_MASK);
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = a->ob_digit[1] - ((b >> DIGIT_BITS) & DIGIT_MASK) - borrow;
	z->ob_digit[1] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = a->ob_digit[2] - ((b >> (DIGIT_BITS * 2)) & DIGIT_MASK) - borrow;
	z->ob_digit[2] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	for (i = 3; i < size_a; ++i) {
		borrow = a->ob_digit[i] - borrow;
		z->ob_digit[i] = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	ASSERT(borrow == 0);
	return int_normalize(z);
err:
	return NULL;
}
#endif /* (DIGIT_BITS * 2) < 32 */



PRIVATE WUNUSED NONNULL((2)) DREF DeeIntObject *DCALL
x_sub_revint2(twodigits a, DeeIntObject *__restrict b) {
	dssize_t size_b = ABS(b->ob_size);
	DeeIntObject *z;
	digit borrow;
	ASSERT(size_b >= 2);
	if (2 < size_b) {
		z = x_sub_int2(b, a);
		if (z)
			z->ob_size = -z->ob_size;
		return z;
	}
	ASSERT(2 == size_b);
	if ((a & DIGIT_MASK) == b->ob_digit[0]) {
		if (((a >> DIGIT_BITS) & DIGIT_MASK) == b->ob_digit[1])
			return_reference_((DeeIntObject *)DeeInt_Zero);
		if (((a >> DIGIT_BITS) & DIGIT_MASK) < b->ob_digit[1])
			goto do_reverse;
	} else if ((a & DIGIT_MASK) < b->ob_digit[0]) {
do_reverse:
		z = x_sub_int2(b, a);
		if (z)
			z->ob_size = -z->ob_size;
		return z;
	}
	z = DeeInt_Alloc(2);
	if unlikely(!z)
		goto err;
	borrow = (a & DIGIT_MASK) - b->ob_digit[0];
	z->ob_digit[0] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	borrow = ((a >> DIGIT_BITS) & DIGIT_MASK) - b->ob_digit[1] - borrow;
	z->ob_digit[1] = borrow & DIGIT_MASK;
	borrow >>= DIGIT_BITS;
	borrow &= 1;
	ASSERT(borrow == 0);
	return int_normalize(z);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
int_inc(DREF DeeIntObject **__restrict p_self) {
	DREF DeeIntObject *z, *a = *p_self;
	if (!DeeObject_IsShared(a)) {
		size_t i;
		/* Try to do the increment in-line, thus not having to allocate a new integer. */
		if unlikely(a->ob_size == 0) {
			*p_self = (DeeIntObject *)DeeInt_One;
			Dee_Incref(DeeInt_One);
			DeeInt_Destroy(a);
			goto done2;
		}
		if (a->ob_size > 0) {
			for (i = 0; i < (size_t)a->ob_size; ++i) {
				if (a->ob_digit[i]++ != DIGIT_MASK)
					goto done2;
				a->ob_digit[i] = 0;
			}
			z = DeeInt_Alloc(a->ob_size + 1);
			if unlikely(!z)
				goto err;
			bzeroc(z->ob_digit,
			       (size_t)a->ob_size,
			       sizeof(digit));
			z->ob_digit[(size_t)a->ob_size] = 1;
		} else {
			for (i = 0;; ++i) {
				digit oldval;
				ASSERT(i < (size_t)-a->ob_size);
				oldval = a->ob_digit[i];
				if (oldval == 0) {
					a->ob_digit[i] = DIGIT_MASK;
					continue;
				}
				if (oldval == 1 && a->ob_size == -1) {
					ASSERT(i == 0);
					*p_self = (DeeIntObject *)DeeInt_Zero;
					Dee_Incref(DeeInt_Zero);
					DeeInt_Destroy(a);
					goto done2;
				}
				a->ob_digit[i] = oldval - 1;
				goto done2;
			}
		}
		*p_self = z; /* Inherit reference. */
		DeeInt_Destroy(a);
		goto done2;
	}
	if (ABS(a->ob_size) <= 1) {
		z = (DREF DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) + 1);
		goto done;
	}
	if (a->ob_size < 0) {
		z = x_sub_revint((digit)1, a);
	} else {
		z = x_add_int(a, (digit)1);
	}
done:
	if unlikely(!z)
		goto err;
	*p_self = z; /* Inherit reference. */
	Dee_Decref_unlikely(a);
done2:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
int_dec(DREF DeeIntObject **__restrict p_self) {
	DREF DeeIntObject *z, *a = *p_self;
	if (!DeeObject_IsShared(a)) {
		size_t i;
		/* Try to do the decrement in-line, thus not having to allocate a new integer. */
		if unlikely(a->ob_size == 0) {
			*p_self = (DeeIntObject *)DeeInt_MinusOne;
			Dee_Incref(DeeInt_MinusOne);
			DeeInt_Destroy(a);
			goto done2;
		}
		if (a->ob_size > 0) {
			for (i = 0;; ++i) {
				digit oldval;
				ASSERT(i < (size_t)a->ob_size);
				oldval = a->ob_digit[i];
				if (oldval == 0) {
					a->ob_digit[i] = DIGIT_MASK;
					continue;
				}
				if (oldval == 1 && a->ob_size == 1) {
					ASSERT(i == 0);
					*p_self = (DeeIntObject *)DeeInt_Zero;
					Dee_Incref(DeeInt_Zero);
					DeeInt_Destroy(a);
					goto done2;
				}
				a->ob_digit[i] = oldval - 1;
				goto done2;
			}
		} else {
			size_t a_digits = (size_t)-a->ob_size;
			for (i = 0; i < a_digits; ++i) {
				if (a->ob_digit[i]++ != DIGIT_MASK)
					goto done2;
				a->ob_digit[i] = 0;
			}
			z = DeeInt_Alloc(a_digits + 1);
			if unlikely(!z)
				goto err;
			z->ob_size = -z->ob_size;
			bzeroc(z->ob_digit, a_digits, sizeof(digit));
			z->ob_digit[a_digits] = 1;
		}
		*p_self = z; /* Inherit reference. */
		DeeInt_Destroy(a);
		goto done2;
	}
	if (ABS(a->ob_size) <= 1) {
		z = (DREF DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) - 1);
		goto done;
	}
	if (a->ob_size < 0) {
		z = x_add_int(a, (digit)1);
		if (z != NULL /*&& z->ob_size != 0*/)
			z->ob_size = -z->ob_size;
	} else {
		z = x_sub_int(a, (digit)1);
	}
done:
	if unlikely(!z)
		goto err;
	*p_self = z; /* Inherit reference. */
	Dee_Decref(a);
done2:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
DeeInt_AddSDigit(DeeIntObject *__restrict a, sdigit b) {
	DREF DeeIntObject *z;
	if (!b)
		return_reference_(a);
	if (ABS(a->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) + b);
	if (a->ob_size < 0) {
		if (b < 0) {
			z = x_add_int(a, (digit)-b);
			if (likely(z) && z->ob_size)
				z->ob_size = -z->ob_size;
		} else {
			z = x_sub_revint((digit)b, a);
		}
	} else if (b < 0) {
		z = x_sub_int(a, (digit)-b);
	} else {
		z = x_add_int(a, (digit)b);
	}
	return z;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
DeeInt_AddUInt32(DeeIntObject *__restrict a, uint32_t b) {
	DREF DeeIntObject *z;
	if (!b)
		return_reference_(a);
	if (ABS(a->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewInt64((int64_t)MEDIUM_VALUE(a) + (int64_t)b);
	if (a->ob_size < 0) {
		if (b <= DIGIT_MASK) {
			z = x_sub_revint((digit)b, a);
		}
#if (DIGIT_BITS * 2) >= 32
		else {
			z = x_sub_revint2((twodigits)b, a);
		}
#else /* (DIGIT_BITS * 2) >= 32 */
		else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2)) - 1) {
			z = x_sub_revint2((twodigits)b, a);
		} else {
			z = x_sub_revint3((twodigits)b, a);
		}
#endif /* (DIGIT_BITS * 2) < 32 */
	} else if (b <= DIGIT_MASK) {
		z = x_add_int(a, (digit)b);
	}
#if (DIGIT_BITS * 2) >= 32
	else {
		z = x_add_int2(a, (twodigits)b);
	}
#else /* (DIGIT_BITS * 2) >= 32 */
	else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2)) - 1) {
		z = x_add_int2(a, (twodigits)b);
	} else {
		z = x_add_int3(a, b);
	}
#endif /* (DIGIT_BITS * 2) < 32 */
	return z;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
DeeInt_SubSDigit(DeeIntObject *__restrict a, sdigit b) {
	DREF DeeIntObject *z;
	if (ABS(a->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) - b);
	if (a->ob_size < 0) {
		if (b < 0) {
			z = x_sub_int(a, (digit)-b);
		} else {
			z = x_add_int(a, (digit)b);
		}
		if (z != NULL /*&& z->ob_size != 0*/)
			z->ob_size = -z->ob_size;
	} else {
		if (b < 0) {
			z = x_add_int(a, (digit)-b);
		} else {
			z = x_sub_int(a, (digit)b);
		}
	}
	return z;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
DeeInt_SubUInt32(DeeIntObject *__restrict a, uint32_t b) {
	DeeIntObject *z;
	if (ABS(a->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewInt64((int64_t)MEDIUM_VALUE(a) - (int64_t)b);
	if (a->ob_size < 0) {
		if (b <= DIGIT_MASK) {
			z = x_add_int(a, (digit)b);
		}
#if (DIGIT_BITS * 2) >= 32
		else {
			z = x_add_int2(a, (twodigits)b);
		}
#else /* (DIGIT_BITS * 2) >= 32 */
		else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2)) - 1) {
			z = x_add_int2(a, (twodigits)b);
		} else {
			z = x_add_int3(a, b);
		}
#endif /* (DIGIT_BITS * 2) < 32 */
		if (z != NULL /*&& z->ob_size != 0*/)
			z->ob_size = -z->ob_size;
	} else {
		if (b <= DIGIT_MASK) {
			z = x_sub_int(a, (digit)b);
		}
#if (DIGIT_BITS * 2) >= 32
		else {
			z = x_sub_int2(a, (twodigits)b);
		}
#else /* (DIGIT_BITS * 2) >= 32 */
		else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2)) - 1) {
			z = x_sub_int2(a, (twodigits)b);
		} else {
			z = x_sub_int3(a, b);
		}
#endif /* (DIGIT_BITS * 2) < 32 */
	}
	return z;
}


PRIVATE NONNULL((1, 3)) digit DCALL
v_iadd(digit *__restrict x, dssize_t m,
       digit const *__restrict y, dssize_t n) {
	dssize_t i;
	digit carry = 0;
	ASSERT(m >= n);
	for (i = 0; i < n; ++i) {
		carry += x[i] + y[i];
		x[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
		ASSERT((carry & 1) == carry);
	}
	for (; carry && i < m; ++i) {
		carry += x[i];
		x[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
		ASSERT((carry & 1) == carry);
	}
	return carry;
}

PRIVATE NONNULL((1, 3)) digit DCALL
v_isub(digit *__restrict x, dssize_t m,
       digit const *__restrict y, dssize_t n) {
	dssize_t i;
	digit borrow = 0;
	ASSERT(m >= n);
	for (i = 0; i < n; ++i) {
		borrow = x[i] - y[i] - borrow;
		x[i]   = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	for (; borrow && i < m; ++i) {
		borrow = x[i] - borrow;
		x[i]   = borrow & DIGIT_MASK;
		borrow >>= DIGIT_BITS;
		borrow &= 1;
	}
	return borrow;
}

PRIVATE NONNULL((1, 2)) digit DCALL
v_lshift(digit *__restrict z,
         digit const *__restrict a,
         dssize_t m, int d) {
	dssize_t i;
	digit carry = 0;
	ASSERT(0 <= d && d < DIGIT_BITS);
	for (i = 0; i < m; i++) {
		twodigits acc;
		acc   = (twodigits)a[i] << d | carry;
		z[i]  = (digit)acc & DIGIT_MASK;
		carry = (digit)(acc >> DIGIT_BITS);
	}
	return carry;
}

PRIVATE NONNULL((1, 2)) digit DCALL
v_rshift(digit *__restrict z,
         digit const *__restrict a,
         dssize_t m, int d) {
	dssize_t i;
	digit carry = 0;
	digit mask  = ((digit)1 << d) - 1U;
	ASSERT(0 <= d && d < DIGIT_BITS);
	for (i = m; i-- > 0;) {
		twodigits acc;
		acc   = (twodigits)carry << DIGIT_BITS | a[i];
		carry = (digit)acc & mask;
		z[i]  = (digit)(acc >> d);
	}
	return carry;
}

PRIVATE NONNULL((1, 2)) digit DCALL
inplace_divrem1(digit *__restrict pout,
                digit const *__restrict pin,
                dssize_t size, digit n) {
	twodigits rem = 0;
	ASSERT(n > 0 && n <= DIGIT_MASK);
	pin += size;
	pout += size;
	while (--size >= 0) {
		digit hi;
		rem     = (rem << DIGIT_BITS) | *--pin;
		*--pout = hi = (digit)(rem / n);
		rem -= (twodigits)hi * n;
	}
	return (digit)rem;
}

PRIVATE WUNUSED NONNULL((1, 3)) DeeIntObject *DCALL
divrem1(DeeIntObject *__restrict a, digit n, digit *p_rem) {
	DeeIntObject *z;
	dssize_t size = ABS(a->ob_size);
	ASSERT(n > 0 && n <= DIGIT_MASK);
	z = DeeInt_Alloc(size);
	if unlikely(!z)
		goto err;
	*p_rem = inplace_divrem1(z->ob_digit, a->ob_digit, size, n);
	return int_normalize(z);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
x_mul(DeeIntObject *a, DeeIntObject *b) {
	DeeIntObject *z;
	dssize_t i;
	dssize_t size_a = ABS(a->ob_size);
	dssize_t size_b = ABS(b->ob_size);
	z = DeeInt_Alloc(size_a + size_b);
	if unlikely(!z)
		goto err;
	bzeroc(z->ob_digit, z->ob_size, sizeof(digit));
	if (a == b) {
		for (i = 0; i < size_a; ++i) {
			twodigits carry, f = a->ob_digit[i];
			digit *pz    = z->ob_digit + (i << 1);
			digit *pa    = a->ob_digit + i + 1;
			digit *paend = a->ob_digit + size_a;
			SIGCHECK(goto err_z);
			carry = *pz + f * f;
			*pz++ = (digit)(carry & DIGIT_MASK);
			carry >>= DIGIT_BITS;
			ASSERT(carry <= DIGIT_MASK);
			f <<= 1;
			while (pa < paend) {
				carry += *pz + *pa++ * f;
				*pz++ = (digit)(carry & DIGIT_MASK);
				carry >>= DIGIT_BITS;
				ASSERT(carry <= (DIGIT_MASK << 1));
			}
			if (carry) {
				carry += *pz;
				*pz++ = (digit)(carry & DIGIT_MASK);
				carry >>= DIGIT_BITS;
			}
			if (carry)
				*pz += (digit)(carry & DIGIT_MASK);
			ASSERT((carry >> DIGIT_BITS) == 0);
		}
	} else {
		for (i = 0; i < size_a; ++i) {
			twodigits carry = 0, f = a->ob_digit[i];
			digit *pz    = z->ob_digit + i;
			digit *pb    = b->ob_digit;
			digit *pbend = b->ob_digit + size_b;
			SIGCHECK(goto err_z);
			while (pb < pbend) {
				carry += *pz + *pb++ * f;
				*pz++ = (digit)(carry & DIGIT_MASK);
				carry >>= DIGIT_BITS;
				ASSERT(carry <= DIGIT_MASK);
			}
			if (carry)
				*pz += (digit)(carry & DIGIT_MASK);
			ASSERT((carry >> DIGIT_BITS) == 0);
			if (DeeThread_CheckInterrupt())
				goto err_z;
		}
	}
	return int_normalize(z);
err_z:
	Dee_Decref(z);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
kmul_split(DeeIntObject *n, dssize_t size,
           DREF DeeIntObject **__restrict phigh,
           DREF DeeIntObject **__restrict plow) {
	DREF DeeIntObject *hi, *lo;
	dssize_t size_n  = ABS(n->ob_size);
	dssize_t size_lo = MIN(size_n, size);
	dssize_t size_hi = size_n - size_lo;
	hi = DeeInt_Alloc(size_hi);
	if unlikely(!hi)
		goto err;
	lo = DeeInt_Alloc(size_lo);
	if unlikely(!lo)
		goto err_hi;
	memcpyc(lo->ob_digit, n->ob_digit, size_lo, sizeof(digit));
	memcpyc(hi->ob_digit, n->ob_digit + size_lo, size_hi, sizeof(digit));
	*phigh = int_normalize(hi);
	*plow  = int_normalize(lo);
	return 0;
err_hi:
	Dee_Decref(hi);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
k_lopsided_mul(DeeIntObject *a, DeeIntObject *b);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
k_mul(DeeIntObject *a, DeeIntObject *b) {
	dssize_t asize   = ABS(a->ob_size);
	dssize_t bsize   = ABS(b->ob_size);
	DeeIntObject *ah = NULL, *al = NULL;
	DeeIntObject *bh = NULL, *bl = NULL;
	DeeIntObject *t1, *t2, *t3, *ret = NULL;
	dssize_t shift, i;
	if (asize > bsize) {
		t1 = a, a = b, b = t1;
		i = asize, asize = bsize, bsize = i;
	}
	i = a == b ? KARATSUBA_SQUARE_CUTOFF : KARATSUBA_CUTOFF;
	if (asize <= i) {
		if (asize == 0)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		return x_mul(a, b);
	}
	if (2 * asize <= bsize)
		return k_lopsided_mul(a, b);
	shift = bsize >> 1;
	if (kmul_split(a, shift, &ah, &al) < 0)
		goto fail;
	ASSERT(ah->ob_size > 0);
	if (a == b) {
		bh = ah;
		bl = al;
		Dee_Incref(bh);
		Dee_Incref(bl);
	} else if (kmul_split(b, shift, &bh, &bl) < 0) {
		goto fail;
	}
	ret = DeeInt_Alloc(asize + bsize);
	if unlikely(!ret)
		goto fail;
#ifndef NDEBUG
	memset(ret->ob_digit, 0xdf, ret->ob_size * sizeof(digit));
#endif /* !NDEBUG */
	t1 = k_mul(ah, bh);
	if unlikely(!t1)
		goto fail;
	ASSERT(t1->ob_size >= 0);
	ASSERT(2 * shift + t1->ob_size <= ret->ob_size);
	memcpyc(ret->ob_digit + 2 * shift,
	        t1->ob_digit,
	        t1->ob_size,
	        sizeof(digit));
	i = ret->ob_size - 2 * shift - t1->ob_size;
	if (i) {
		bzeroc(ret->ob_digit + 2 * shift + t1->ob_size,
		       i, sizeof(digit));
	}
	t2 = k_mul(al, bl);
	if unlikely(!t2)
		goto fail_t1;
	ASSERT(t2->ob_size >= 0);
	ASSERT(t2->ob_size <= 2 * shift);
	memcpyc(ret->ob_digit,
	        t2->ob_digit,
	        t2->ob_size,
	        sizeof(digit));
	i = 2 * shift - t2->ob_size;
	if (i) {
		bzeroc(ret->ob_digit + t2->ob_size,
		       i, sizeof(digit));
	}
	i = ret->ob_size - shift;
	(void)v_isub(ret->ob_digit + shift, i, t2->ob_digit, t2->ob_size);
	Dee_Decref(t2);
	(void)v_isub(ret->ob_digit + shift, i, t1->ob_digit, t1->ob_size);
	Dee_Decref(t1);
	t1 = x_add(ah, al);
	if unlikely(!t1)
		goto fail;
	Dee_Decref(ah);
	Dee_Decref(al);
	ah = al = NULL;
	if (a == b) {
		t2 = t1;
		Dee_Incref(t2);
	} else {
		t2 = x_add(bh, bl);
		if unlikely(!t2)
			goto fail_t1;
	}
	Dee_Decref(bh);
	Dee_Decref(bl);
	bh = NULL;
	bl = NULL;
	t3 = k_mul(t1, t2);
	Dee_Decref(t1);
	Dee_Decref(t2);
	if unlikely(!t3)
		goto fail;
	ASSERT(t3->ob_size >= 0);
	(void)v_iadd(ret->ob_digit + shift, i, t3->ob_digit, t3->ob_size);
	Dee_Decref(t3);
	return int_normalize(ret);
fail_t1:
	Dee_Decref(t1);
fail:
	Dee_XDecref(ret);
	Dee_XDecref(ah);
	Dee_XDecref(al);
	Dee_XDecref(bh);
	Dee_XDecref(bl);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
k_lopsided_mul(DeeIntObject *a, DeeIntObject *b) {
	dssize_t asize = ABS(a->ob_size);
	dssize_t bsize = ABS(b->ob_size);
	dssize_t nbdone;
	DeeIntObject *ret, *bslice = NULL;
	ASSERT(asize > KARATSUBA_CUTOFF);
	ASSERT(2 * asize <= bsize);
	ret = DeeInt_Alloc(asize + bsize);
	if unlikely(!ret)
		goto err;
	bzeroc(ret->ob_digit, ret->ob_size, sizeof(digit));
	bslice = DeeInt_Alloc(asize);
	if unlikely(!bslice)
		goto err_bslice_fail;
	nbdone = 0;
	while (bsize > 0) {
		DeeIntObject *product;
		dssize_t nbtouse = MIN(bsize, asize);
		memcpyc(bslice->ob_digit,
		        b->ob_digit + nbdone,
		        nbtouse, sizeof(digit));
		bslice->ob_size = nbtouse;
		product         = k_mul(a, bslice);
		if unlikely(!product)
			goto err_bslice_fail;
		(void)v_iadd(ret->ob_digit + nbdone, ret->ob_size - nbdone,
		             product->ob_digit, product->ob_size);
		Dee_Decref(product);
		bsize -= nbtouse;
		nbdone += nbtouse;
		if (DeeThread_CheckInterrupt())
			goto err_bslice_fail;
	}
	Dee_Decref(bslice);
	return int_normalize(ret);
err_bslice_fail:
	Dee_Decref(ret);
	Dee_XDecref(bslice);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_mul(DeeIntObject *a, DeeObject *b_ob) {
	DREF DeeIntObject *z, *b;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
		stwodigits v = (stwodigits)(MEDIUM_VALUE(a)) * MEDIUM_VALUE(b);
		Dee_Decref(b);
		return (DREF DeeIntObject *)DeeInt_NewSTwoDigits(v);
	}
	z = k_mul(a, b);
	if (likely(z) && ((a->ob_size ^ b->ob_size) < 0))
		z = int_neg_inherit(z);
	Dee_Decref(b);
	return z;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeIntObject *DCALL
x_divrem(DeeIntObject *v1, DeeIntObject *w1,
         DeeIntObject **__restrict p_rem);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
int_divrem(DeeIntObject *a,
           DeeIntObject *b,
           DeeIntObject **p_div,
           DeeIntObject **p_rem) {
	DREF DeeIntObject *z;
	dssize_t size_a = ABS(a->ob_size);
	dssize_t size_b = ABS(b->ob_size);
	if (size_b == 0) {
		err_divide_by_zero((DeeObject *)a, (DeeObject *)b);
		goto err;
	}
	if (size_a < size_b ||
	    (size_a == size_b && (a->ob_digit[size_a - 1] <
	                          b->ob_digit[size_b - 1]))) {
		Dee_Incref(DeeInt_Zero);
		Dee_Incref(a);
		*p_div = (DeeIntObject *)DeeInt_Zero;
		*p_rem = a;
		return 0;
	}
	if (size_b == 1) {
		digit rem = 0;
		z = divrem1(a, b->ob_digit[0], &rem);
		if unlikely(!z)
			goto err;
		*p_rem = (DeeIntObject *)DeeInt_NewDigit(rem);
		if unlikely(!*p_rem)
			goto err_z;
	} else {
		z = x_divrem(a, b, p_rem);
		if unlikely(!z)
			goto err;
	}
	if ((a->ob_size < 0) != (b->ob_size < 0)) {
		z = int_neg_inherit(z);
		if unlikely(!z)
			goto err_prem;
	}
	if (a->ob_size < 0 && (*p_rem)->ob_size != 0) {
		*p_rem = int_neg_inherit(*p_rem);
		if unlikely(!*p_rem)
			goto err_z;
	}
	*p_div = maybe_small_int(z);
	return 0;
err_prem:
	Dee_Clear(*p_rem);
	goto err;
err_z:
	Dee_Decref(z);
err:
	return -1;
}


PRIVATE unsigned char const bit_length_table[32] = {
	0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

LOCAL ATTR_CONST WUNUSED int DCALL bits_in_digit(digit d) {
	int d_bits = 0;
	while (d >= 32) {
		d_bits += 6;
		d >>= 6;
	}
	d_bits += (int)bit_length_table[d];
	return d_bits;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeIntObject *DCALL
x_divrem(DeeIntObject *v1, DeeIntObject *w1,
         DeeIntObject **__restrict p_rem) {
	DeeIntObject *v, *w, *a;
	dssize_t i, k, size_v, size_w;
	digit wm1, wm2, carry, q, r, vtop, *v0, *vk, *w0, *ak;
	twodigits vv;
	stwodigits z;
	sdigit zhi;
	int d;
	size_v = ABS(v1->ob_size);
	size_w = ABS(w1->ob_size);
	ASSERT(size_v >= size_w && size_w >= 2);
	v = DeeInt_Alloc(size_v + 1);
	if unlikely(!v)
		goto err;
	w = DeeInt_Alloc(size_w);
	if unlikely(!w)
		goto err_v;
	d     = DIGIT_BITS - bits_in_digit(w1->ob_digit[size_w - 1]);
	carry = v_lshift(w->ob_digit, w1->ob_digit, size_w, d);
	ASSERT(carry == 0);
	carry = v_lshift(v->ob_digit, v1->ob_digit, size_v, d);
	if (carry != 0 || v->ob_digit[size_v - 1] >= w->ob_digit[size_w - 1]) {
		v->ob_digit[size_v] = carry;
		++size_v;
	}
	k = size_v - size_w;
	ASSERT(k >= 0);
	a = DeeInt_Alloc(k);
	if unlikely(!a)
		goto err_v_w;
	v0  = v->ob_digit;
	w0  = w->ob_digit;
	wm1 = w0[size_w - 1];
	wm2 = w0[size_w - 2];
	for (vk = v0 + k, ak = a->ob_digit + k; vk-- > v0;) {
		SIGCHECK(goto err_v_w_a);
		vtop = vk[size_w];
		ASSERT(vtop <= wm1);
		vv = ((twodigits)vtop << DIGIT_BITS) | vk[size_w - 1];
		q  = (digit)(vv / wm1);
		r  = (digit)(vv - (twodigits)wm1 * q); /* r = vv % wm1 */
		while ((twodigits)wm2 * q > (((twodigits)r << DIGIT_BITS) | vk[size_w - 2])) {
			--q, r += wm1;
			if (r >= DIGIT_BASE)
				break;
		}
		ASSERT(q <= DIGIT_BASE);
		zhi = 0;
		for (i = 0; i < size_w; ++i) {
			z     = (sdigit)vk[i] + zhi - (stwodigits)q * (stwodigits)w0[i];
			vk[i] = (digit)z & DIGIT_MASK;
			zhi   = (sdigit)ARITHMETIC_RIGHT_SHIFT(stwodigits, z, DIGIT_BITS);
		}
		ASSERT((sdigit)vtop + zhi == -1 || (sdigit)vtop + zhi == 0);
		if ((sdigit)vtop + zhi < 0) {
			carry = 0;
			for (i = 0; i < size_w; ++i) {
				carry += vk[i] + w0[i];
				vk[i] = carry & DIGIT_MASK;
				carry >>= DIGIT_BITS;
			}
			--q;
		}
		ASSERT(q < DIGIT_BASE);
		*--ak = q;
	}
	carry = v_rshift(w0, v0, size_w, d);
	ASSERT(carry == 0);
	Dee_Decref(v);
	*p_rem = int_normalize(w);
	return int_normalize(a);
err_v_w_a:
	Dee_Decref(a);
err_v_w:
	Dee_Decref(w);
err_v:
	Dee_Decref(v);
err:
	*p_rem = NULL;
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
int_divmod(DeeIntObject *v,
           DeeIntObject *w,
           DREF DeeIntObject **p_div,
           DREF DeeIntObject **p_mod) {
	DREF DeeIntObject *div, *mod;
	if (int_divrem(v, w, &div, &mod) < 0)
		goto err;
	if ((mod->ob_size < 0 && w->ob_size > 0) ||
	    (mod->ob_size > 0 && w->ob_size < 0)) {
		DeeIntObject *temp;
		temp = int_add(mod, (DeeObject *)w);
		Dee_Decref(mod);
		mod = temp;
		if unlikely(!mod)
			goto err_div;
		temp = int_sub(div, DeeInt_One);
		if unlikely(!temp)
			goto err_div_mod;
		Dee_Decref(div);
		div = temp;
	}
	if (p_div != NULL) {
		*p_div = div;
	} else {
		Dee_Decref(div);
	}
	if (p_mod != NULL) {
		*p_mod = mod;
	} else {
		Dee_Decref(mod);
	}
	return 0;
err_div_mod:
	Dee_Decref(mod);
err_div:
	Dee_Decref(div);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_div(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *div, *b;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	if (int_divmod(a, b, &div, NULL) < 0)
		div = NULL;
	Dee_Decref(b);
	return div;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_mod(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *mod, *b;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	if (int_divmod(a, b, NULL, &mod) < 0)
		mod = NULL;
	Dee_Decref(b);
	return mod;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_inv(DeeIntObject *__restrict v) {
	DeeIntObject *x;
	if (ABS(v->ob_size) <= 1)
		return (DREF DeeIntObject *)DeeInt_NewMedian(-(MEDIUM_VALUE(v) + 1));
	x = int_add(v, DeeInt_One);
	if unlikely(!x)
		goto err;
	x->ob_size = -x->ob_size;
	return maybe_small_int(x);
err:
	return NULL;
}

#if 0 /* Not needed... */
#define HAVE_int_inv_inherited
INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
int_inv_inherited(/*inherit(always)*/ DREF DeeIntObject *__restrict v) {
	DREF DeeIntObject *result;
	size_t i, size;
	if unlikely(DeeObject_IsShared(v)) {
		result = int_inv(v);
		Dee_Decref_unlikely(v);
		return result;
	}
	size = ABS(v->ob_size);
	for (i = 0;; ++i) {
		if (i >= size) {
			result = DeeInt_Alloc(size + 1);
			if unlikely(!result)
				goto err_v;
			if (v->ob_size >= 0)
				result->ob_size = -result->ob_size;
			DeeInt_Destroy(v);
			bzeroc(result->ob_digit, size, sizeof(Dee_digit_t));
			result->ob_digit[size] = 1;
			return result;
		}
		if (v->ob_digit[i] < DIGIT_MASK) {
			++v->ob_digit[i];
			break;
		}
		v->ob_digit[i] = 0;
	}
	v->ob_size = -v->ob_size;
	return v;
err_v:
	DeeInt_Destroy(v);
	return NULL;
}
#endif

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_shr(DeeIntObject *a, DeeObject *b) {
	DeeIntObject *z;
	digit lomask, himask;
	dssize_t shiftby, newsize, wordshift, loshift, hishift, i, j;
	if (a->ob_size < 0) {
		DeeIntObject *a1, *a2;
		a1 = int_inv(a);
		if unlikely(!a1)
			goto err;
		a2 = int_shr(a1, b);
		Dee_Decref(a1);
		if unlikely(!a2)
			goto err;
#ifdef HAVE_int_inv_inherited
		z = int_inv_inherited(a2);
#else /* HAVE_int_inv_inherited */
		z = int_inv(a2);
		Dee_Decref(a2);
#endif /* !HAVE_int_inv_inherited */
	} else {
		if (DeeObject_AsSSize(b, &shiftby))
			goto err;
		if (shiftby < 0) {
			err_shift_negative((DeeObject *)a, b, false);
			goto err;
		}
		wordshift = shiftby / DIGIT_BITS;
		newsize   = ABS(a->ob_size) - wordshift;
		if (newsize <= 0)
			return_reference_((DeeIntObject *)DeeInt_Zero);
		loshift = shiftby % DIGIT_BITS;
		hishift = DIGIT_BITS - loshift;
		lomask  = ((digit)1 << hishift) - 1;
		himask  = DIGIT_MASK ^ lomask;
		z       = DeeInt_Alloc(newsize);
		if unlikely(!z)
			goto err;
		if (a->ob_size < 0)
			z->ob_size = -(z->ob_size);
		for (i = 0, j = wordshift; i < newsize; i++, j++) {
			z->ob_digit[i] = (a->ob_digit[j] >> loshift) & lomask;
			if (i + 1 < newsize)
				z->ob_digit[i] |= (a->ob_digit[j + 1] << hishift) & himask;
		}
		z = int_normalize(z);
	}
	return maybe_small_int(z);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_shl(DeeIntObject *a, DeeObject *b) {
	DREF DeeIntObject *result;
	twodigits accum;
	dssize_t shiftby, oldsize, newsize, wordshift, remshift, i, j;
	if (DeeObject_AsSSize(b, &shiftby))
		goto err;
	if (shiftby < 0) {
		err_shift_negative((DeeObject *)a, b, true);
		goto err;
	}
	wordshift = shiftby / DIGIT_BITS;
	remshift  = shiftby - wordshift * DIGIT_BITS;
	oldsize   = ABS(a->ob_size);
	newsize   = oldsize + wordshift;
	if (remshift)
		++newsize;
	result = DeeInt_Alloc(newsize);
	if unlikely(!result)
		goto err;
	if (a->ob_size < 0) {
		ASSERT(result->ob_refcnt == 1);
		result->ob_size = -result->ob_size;
	}
	for (i = 0; i < wordshift; i++)
		result->ob_digit[i] = 0;
	accum = 0;
	for (i = wordshift, j = 0; j < oldsize; i++, j++) {
		accum |= (twodigits)a->ob_digit[j] << remshift;
		result->ob_digit[i] = (digit)(accum & DIGIT_MASK);
		accum >>= DIGIT_BITS;
	}
	if (remshift) {
		result->ob_digit[newsize - 1] = (digit)accum;
	} else {
		ASSERT(!accum);
	}
	result = int_normalize(result);
	return maybe_small_int(result);
err:
	return NULL;
}

PRIVATE NONNULL((1, 2)) void DCALL
v_complement(digit *z, digit const *a, size_t m) {
	size_t i;
	digit carry = 1;
	for (i = 0; i < m; ++i) {
		carry += a[i] ^ DIGIT_MASK;
		z[i] = carry & DIGIT_MASK;
		carry >>= DIGIT_BITS;
	}
	ASSERT(carry == 0);
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeIntObject *DCALL
int_bitwise(DeeIntObject *__restrict a, char op,
            DeeIntObject *__restrict b) {
	int nega, negb, negz;
	DREF DeeIntObject *z;
	size_t size_a, size_b, size_z, i;
	size_a = (size_t)ABS(a->ob_size);
	nega   = a->ob_size < 0;
	if (nega) {
		z = DeeInt_Alloc(size_a);
		if unlikely(!z)
			goto err;
		v_complement(z->ob_digit, a->ob_digit, size_a);
		a = z;
	} else {
		Dee_Incref(a);
	}
	size_b = (size_t)ABS(b->ob_size);
	negb   = b->ob_size < 0;
	if (negb) {
		z = DeeInt_Alloc(size_b);
		if unlikely(!z)
			goto err_a;
		v_complement(z->ob_digit, b->ob_digit, size_b);
		b = z;
	} else {
		Dee_Incref(b);
	}
	if (size_a < size_b) {
		z      = a;
		a      = b;
		b      = z;
		size_z = size_a;
		size_a = size_b;
		size_b = size_z;
		negz   = nega;
		nega   = negb;
		negb   = negz;
	}
	switch (op) {

	case '^':
		negz   = nega ^ negb;
		size_z = size_a;
		break;

	case '&':
		negz   = nega & negb;
		size_z = negb ? size_a : size_b;
		break;

	default:
		negz   = nega | negb;
		size_z = negb ? size_b : size_a;
		break;
	}
	z = DeeInt_Alloc(size_z + negz);
	if unlikely(!z)
		goto err_a_b;
	switch (op) {

	case '&':
		for (i = 0; i < size_b; ++i)
			z->ob_digit[i] = a->ob_digit[i] & b->ob_digit[i];
		break;

	case '^':
		for (i = 0; i < size_b; ++i)
			z->ob_digit[i] = a->ob_digit[i] ^ b->ob_digit[i];
		break;

	default:
		for (i = 0; i < size_b; ++i)
			z->ob_digit[i] = a->ob_digit[i] | b->ob_digit[i];
		break;
	}
	if (op == '^' && negb) {
		for (; i < size_z; ++i)
			z->ob_digit[i] = a->ob_digit[i] ^ DIGIT_MASK;
	} else if (i < size_z) {
		memcpyc(&z->ob_digit[i],
		        &a->ob_digit[i],
		        size_z - i,
		        sizeof(digit));
	}
	if (negz) {
		z->ob_size = -(z->ob_size);
		z->ob_digit[size_z] = DIGIT_MASK;
		v_complement(z->ob_digit, z->ob_digit, size_z + 1);
	}
	Dee_Decref(a);
	Dee_Decref(b);
	z = int_normalize(z);
	return maybe_small_int(z);
err_a_b:
	Dee_Decref(b);
err_a:
	Dee_Decref(a);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_and(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *b;
	DREF DeeIntObject *c;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	c = (DREF DeeIntObject *)int_bitwise(a, '&', b);
	Dee_Decref(b);
	return c;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_xor(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *b;
	DREF DeeIntObject *c;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	c = (DREF DeeIntObject *)int_bitwise(a, '^', b);
	Dee_Decref(b);
	return c;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_or(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *b;
	DREF DeeIntObject *c;
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	c = (DREF DeeIntObject *)int_bitwise(a, '|', b);
	Dee_Decref(b);
	return c;
err:
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pow(DeeIntObject *a, DeeObject *b_ob) {
	DeeIntObject *z, *b;
	dssize_t i, j, k;
	DeeIntObject *table[32] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};
	b = (DeeIntObject *)DeeObject_Int(b_ob);
	if unlikely(!b)
		goto err;
	Dee_Incref(a);
	if (b->ob_size < 0) {
		Dee_Decref(a);
		Dee_Decref(b);
#if 0
		return (*DeeFloat_Type.tp_math->tp_pow)(a, b);
#else
		DERROR_NOTIMPLEMENTED();
		goto err;
#endif
	}
	z = (DeeIntObject *)DeeInt_One;
	Dee_Incref(z);
#define MULT(x, y, result)                   \
	do {                                     \
		DREF DeeIntObject *temp;             \
		temp = int_mul(x, (DeeObject *)(y)); \
		if unlikely(!temp)                   \
			goto err_b_a_table_z;            \
		Dee_XDecref(result);                 \
		result = temp;                       \
	}	__WHILE0

	if (b->ob_size <= FIVEARY_CUTOFF) {
		for (i = b->ob_size - 1; i >= 0; --i) {
			digit bi = b->ob_digit[i];
			for (j = (digit)1 << (DIGIT_BITS - 1); j != 0; j >>= 1) {
				MULT(z, z, z);
				if (bi & j)
					MULT(z, a, z);
			}
		}
	} else {
		Dee_Incref(z);
		table[0] = z;
		for (i = 1; i < (dssize_t)COMPILER_LENOF(table); ++i)
			MULT(table[i - 1], a, table[i]);
		for (i = b->ob_size - 1; i >= 0; --i) {
			digit bi = b->ob_digit[i];
			for (j = DIGIT_BITS - 5; j >= 0; j -= 5) {
				int index = (bi >> j) & 0x1f;
				for (k = 0; k < 5; ++k)
					MULT(z, z, z);
				if (index)
					MULT(z, table[index], z);
			}
		}
	}
out:
	if (b->ob_size > FIVEARY_CUTOFF)
		Dee_XDecrefv(table, COMPILER_LENOF(table));
	Dee_Decref(a);
	Dee_Decref(b);
	return z;
err_b_a_table_z:
	Dee_Clear(z);
	goto out;
err:
	return NULL;
#undef MULT
}


/* Everything below is not derived from python and as such is subject to the following license */
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

#undef HAVE_int_pext_impl
#undef HAVE_int_pdep_impl
#if !defined(__OPTIMIZE_SIZE__) && 1
#define HAVE_int_pext_impl
#define HAVE_int_pdep_impl
#endif

#ifdef HAVE_int_pext_impl
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pext_impl(digit const *self, digit const *mask, size_t common_size) {
	DREF DeeIntObject *result;
	size_t i, result_nbits, result_digits;
	size_t result_index;
	shift_t result_shift;
	for (result_nbits = i = 0; i < common_size; ++i) {
		digit d = mask[i];
		result_nbits += POPCOUNT(d);
	}
	result_digits = CEILDIV(result_nbits, DIGIT_BITS);
	result = DeeInt_Alloc(result_digits);
	if unlikely(!result)
		goto err;
	bzeroc(result->ob_digit, result_digits, sizeof(Dee_digit_t));
	result_index = 0;
	result_shift = 0;
	for (i = 0; i < common_size; ++i) {
		shift_t n;
		digit v, m = mask[i];
		if (!m)
			continue;
		v = self[i];
		v = PEXT(v, m);  /* Value to append to "result" */
		n = POPCOUNT(m); /* # of bits from "v" to append write to "result" */
		ASSERT(n > 0);
		ASSERT(n <= DIGIT_BITS);
		for (;;) {
			shift_t n_copy;
			digit addend;
			n_copy = DIGIT_BITS - result_shift;
			if (n_copy > n)
				n_copy = n;
			addend = v & (((digit)1 << n_copy) - 1);
			addend <<= result_shift;
			result->ob_digit[result_index] |= addend;
			n -= n_copy;
			if (!n) {
				result_shift += n_copy;
				ASSERT(result_shift <= DIGIT_BITS);
				break;
			}
			ASSERT((result_shift + n_copy) == DIGIT_BITS);
			++result_index;
			result_shift = 0;
			v >>= n_copy;
		}
	}
	result = int_normalize(result);
	return result;
err:
	return NULL;
}
#endif /* HAVE_int_pext_impl */

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pext_ex_impl(DeeIntObject *self, DeeIntObject *mask) {
	size_t self_size = (size_t)ABS(self->ob_size);
	size_t mask_size = (size_t)ABS(mask->ob_size);
	bool self_neg = self->ob_size < 0;
	bool mask_neg = mask->ob_size < 0;
	DREF DeeIntObject *result;
	size_t i, result_nbits, result_digits;
	size_t result_index, common_size;
	shift_t result_shift;
	digit mask_carry;
	digit self_carry;
	result_nbits = 0;
	common_size = self_size;
	if (common_size > mask_size)
		common_size = mask_size;
	if (mask_neg) {
		size_t mask_common_size = mask_size;
		if (self_neg)
			mask_common_size = common_size;
		mask_carry = 1;
		for (i = 0; i < mask_common_size; ++i) {
			digit d;
			mask_carry += mask->ob_digit[i] ^ DIGIT_MASK;
			d = mask_carry & DIGIT_MASK;
			mask_carry >>= DIGIT_BITS;
			result_nbits += POPCOUNT(d);
		}
		ASSERT(mask_carry == 0 || mask_carry == 1);
		result_nbits += mask_carry;

		/* Special case when the mask is `-1' (iow: when `~mask == 0') */
		if unlikely(!result_nbits)
			return_reference_(self);

		/* Because the mask is negative, all bits past the mask are copied as-is from "self" */
		if (i < self_size)
			result_nbits += (self_size - i) * DIGIT_BITS;
		if (self_neg)
			++result_nbits; /* need 1 extra bit for the sign when inverting "result" */
	} else {
		for (i = 0; i < mask_size; ++i) {
			digit d = mask->ob_digit[i];
			result_nbits += POPCOUNT(d);
		}
	}
	result_digits = CEILDIV(result_nbits, DIGIT_BITS);
	result = DeeInt_Alloc(result_digits);
	if unlikely(!result)
		goto err;
	bzeroc(result->ob_digit, result_digits, sizeof(Dee_digit_t));
	result_index = 0;
	result_shift = 0;
	mask_carry = 1;
	self_carry = 1;
	for (i = 0; i < common_size; ++i) {
		shift_t n;
		digit v, m = mask->ob_digit[i];
		if (mask_neg) {
			mask_carry += m ^ DIGIT_MASK;
			m = mask_carry & DIGIT_MASK;
			mask_carry >>= DIGIT_BITS;
		}
		if (!m) {
			if (self_neg) {
				self_carry += self->ob_digit[i] ^ DIGIT_MASK;
				self_carry >>= DIGIT_BITS;
			}
			continue;
		}
		v = self->ob_digit[i];
		if (self_neg) {
			self_carry += v ^ DIGIT_MASK;
			v = self_carry & DIGIT_MASK;
			self_carry >>= DIGIT_BITS;
		}
		v = PEXT(v, m);  /* Value to append to "result" */
		n = POPCOUNT(m); /* # of bits from "v" to append write to "result" */
		ASSERT(n > 0);
		for (;;) {
			shift_t n_copy;
			digit addend;
			n_copy = DIGIT_BITS - result_shift;
			if (n_copy > n)
				n_copy = n;
			addend = v & (((digit)1 << n_copy) - 1);
			addend <<= result_shift;
			result->ob_digit[result_index] |= addend;
			n -= n_copy;
			if (!n) {
				result_shift += n_copy;
				ASSERT(result_shift <= DIGIT_BITS);
				break;
			}
			ASSERT((result_shift + n_copy) == DIGIT_BITS);
			++result_index;
			result_shift = 0;
			v >>= n_copy;
		}
	}
	if (mask_neg) {
		if (self_size > common_size) {
			/* Remaining bits of "self" must be copied as-is */
			ASSERT(i == common_size);
#ifndef __OPTIMIZE_SIZE__
			if (!self_neg && result_shift == 0) {
				size_t n_digits = self_size - common_size;
				memcpyc(&result->ob_digit[result_index], &self->ob_digit[common_size],
				        n_digits, sizeof(digit));
				result_index += n_digits;
			} else
#endif /* !__OPTIMIZE_SIZE__ */
			{
				for (; i < self_size; ++i) {
					shift_t n;
					digit v = self->ob_digit[i];
					if (self_neg) {
						self_carry += v ^ DIGIT_MASK;
						v = self_carry & DIGIT_MASK;
						self_carry >>= DIGIT_BITS;
					}
					for (n = DIGIT_BITS;;) {
						shift_t n_copy;
						digit addend;
						n_copy = DIGIT_BITS - result_shift;
						if (n_copy > n)
							n_copy = n;
						addend = v & (((digit)1 << n_copy) - 1);
						addend <<= result_shift;
						result->ob_digit[result_index] |= addend;
						n -= n_copy;
						if (!n) {
							result_shift += n_copy;
							ASSERT(result_shift <= DIGIT_BITS);
							break;
						}
						ASSERT((result_shift + n_copy) == DIGIT_BITS);
						++result_index;
						result_shift = 0;
						v >>= n_copy;
					}
				}
			}
		}
		if (self_neg) {
			digit lastmask;
			ASSERT(result_shift <= DIGIT_BITS);
			if (result_shift == DIGIT_BITS) {
				++result_index;
				result_shift = 0;
			}
			ASSERT(result_index == result_digits - 1);
			ASSERT((result_shift + result_index * DIGIT_BITS) <= result_nbits);
			lastmask = (DIGIT_MASK << result_shift) & DIGIT_MASK;
			result->ob_digit[result_index] |= lastmask;
			v_complement(result->ob_digit, result->ob_digit, (size_t)result->ob_size);
			result->ob_size = -result->ob_size;
		}
	} else if (self_neg && mask_size > common_size) {
		/* Remaining bits of "self" are all 1es */
		size_t remaining_bits;
		ASSERT((result_shift + (result_index * DIGIT_BITS)) <= result_nbits);
		remaining_bits = result_nbits - (result_shift + (result_index * DIGIT_BITS));
		if (result_shift >= DIGIT_BITS) {
			result_shift -= DIGIT_BITS;
			++result_index;
		}
		if (result_shift) {
			size_t first_trailing_digit_bits;
			digit first_trailing_digit_mask;
			first_trailing_digit_bits = DIGIT_BITS - result_shift;
			if (first_trailing_digit_bits > remaining_bits)
				first_trailing_digit_bits = remaining_bits;
			first_trailing_digit_mask = ((digit)1 << first_trailing_digit_bits) - 1;
			first_trailing_digit_mask <<= result_shift;
			result->ob_digit[result_index] |= first_trailing_digit_mask;
			remaining_bits -= first_trailing_digit_bits;
			if (!remaining_bits)
				goto done;
			/*result_shift = 0;*/ /* Unused... */
			++result_index;
		}
		if (remaining_bits >= DIGIT_BITS) {
			size_t n_digits = remaining_bits / DIGIT_BITS;
			Dee_digit_memset(&result->ob_digit[result_index], DIGIT_MASK, n_digits);
			result_index += n_digits;
			remaining_bits -= n_digits * DIGIT_BITS;
		}
		if (remaining_bits) {
			digit lastmask = ((digit)1 << remaining_bits) - 1;
			result->ob_digit[result_index] = lastmask;
		}
	}
done:
	result = int_normalize(result);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pext(DeeIntObject *self, DeeIntObject *mask) {
	DREF DeeIntObject *result;
	/* A negative self/mask makes this algorithm extremely complicated.
	 * A (correct, but unoptimized) reference implementation can be found at:
	 * >> /util/test/deemon-int-pext.dee:correctPExt */
#ifdef HAVE_int_pext_impl
	if likely(self->ob_size >= 0 && mask->ob_size >= 0) {
		size_t common_size = (size_t)mask->ob_size;
		if (common_size > (size_t)self->ob_size)
			common_size = (size_t)self->ob_size;
		result = int_pext_impl(self->ob_digit, mask->ob_digit, common_size);
	} else
#endif /* HAVE_int_pext_impl */
	{
		result = int_pext_ex_impl(self, mask);
	}
	return result;
}



#ifdef HAVE_int_pdep_impl
PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeIntObject *DCALL
int_pdep_impl(digit const *self, size_t common_size,
              digit const *mask, size_t mask_size) {
	DREF DeeIntObject *result;
	size_t i, self_index;
	shift_t self_shift;
	result = DeeInt_Alloc(mask_size);
	if unlikely(!result)
		goto err;
	self_index = 0;
	self_shift = 0;
	for (i = 0; i < mask_size && self_index < common_size; ++i) {
		shift_t m_nbits;
		digit v, m = mask[i];
		if likely(m) {
			m_nbits = POPCOUNT(m);
			v = self[self_index] >> self_shift; /* v_nbits = DIGIT_BITS - self_shift */
			self_shift += m_nbits;              /* v_nbits = DIGIT_BITS - self_shift + m_nbits */
			if (self_shift >= DIGIT_BITS) {
				self_shift -= DIGIT_BITS;       /* v_nbits = DIGIT_BITS - self_shift + m_nbits - DIGIT_BITS */
				++self_index;
				if (self_index < common_size) {
					shift_t v_nbits; /* # of bits already stored in "v" */
					/* >> v_nbits = DIGIT_BITS - self_shift + m_nbits - DIGIT_BITS
					 * >> v_nbits = -self_shift + m_nbits
					 * >> v_nbits = m_nbits - self_shift */
					v_nbits = m_nbits - self_shift;
					v |= self[self_index] << v_nbits;
				}
			}
			v = PDEP(v, m);
		} else {
			v = 0;
		}
		result->ob_digit[i] = v;
	}
#if 1
	result->ob_size = (Dee_ssize_t)i;
#else
	bzeroc(result->ob_digit + i, mask_size - i, sizeof(Dee_digit_t));
#endif
	result = int_normalize(result);
	return result;
err:
	return NULL;
}
#endif /* HAVE_int_pdep_impl */

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pdep_ex_impl(DeeIntObject *self, DeeIntObject *mask) {
#if 0
	size_t self_size = (size_t)ABS(self->ob_size);
	size_t mask_size = (size_t)ABS(mask->ob_size);
	bool self_neg = self->ob_size < 0;
	bool mask_neg = mask->ob_size < 0;
	DREF DeeIntObject *result;
#endif

	/* TODO */
	(void)self;
	(void)mask;
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeIntObject *DCALL
int_pdep(DeeIntObject *self, DeeIntObject *mask) {
	DREF DeeIntObject *result;
	/* A negative self/mask makes this algorithm extremely complicated.
	 * A (correct, but unoptimized) reference implementation can be found at:
	 * >> /util/test/deemon-int-pdep.dee:correctPDep */
#ifdef HAVE_int_pdep_impl
	if likely(self->ob_size >= 0 && mask->ob_size >= 0) {
		size_t common_size = (size_t)mask->ob_size;
		if (common_size > (size_t)self->ob_size)
			common_size = (size_t)self->ob_size;
		result = int_pdep_impl(self->ob_digit, common_size,
		                       mask->ob_digit, (size_t)mask->ob_size);
	} else
#endif /* HAVE_int_pdep_impl */
	{
		result = int_pdep_ex_impl(self, mask);
	}
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_C */
