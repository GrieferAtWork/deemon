/* NOTE: Deemon's integer object implementation is
 *       heavily based on python's `long' data type.
 *       With that in mind, licensing of deemon's integer
 *       implementation must be GPL-compatible, GPL being
 *       the license that python is restricted by.
 *    >> So to simplify this whole deal: I make no claim of having invented the
 *       way that deemon's (phyton's) arbitrary-length integers are implemented,
 *       with all algorithms found in `int_logic.c' originating from phython
 *       before being adjusted to fit deemon's runtime.
 *       To further discourage use of code found here, in favor of the original
 *       creator's work, comments have been removed.
 *       I DID NOT WRITE STUFF IN THIS FILE
 */
#ifndef GUARD_DEEMON_OBJECTS_INT_LOGIC_C
#define GUARD_DEEMON_OBJECTS_INT_LOGIC_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/int.h>
#include <deemon/error.h>

#include "int_logic.h"
#include "../runtime/runtime_error.h"

#include <string.h>

#ifdef __KOS_SYSTEM_HEADERS__
#include <hybrid/minmax.h>
#endif /* __KOS_SYSTEM_HEADERS__ */

DECL_BEGIN

#ifndef MIN
#define MIN(x,y)     ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)     ((x) < (y) ? (y) : (x))
#endif

#define KARATSUBA_CUTOFF         70
#define KARATSUBA_SQUARE_CUTOFF (2*KARATSUBA_CUTOFF)
#define FIVEARY_CUTOFF           8

#if DIGIT_BITS <= 31
#define DeeInt_NewMedian(x) DeeInt_NewS32(x)
#else
#define DeeInt_NewMedian(x) DeeInt_NewS64(x)
#endif


#define SWAP(T,a,b) do{ T const _temp_ = (a); (a) = (b); (b) = _temp_; }__WHILE0
#define ABS(x)        ((x) < 0 ? -(x) : (x))
#define MEDIUM_VALUE(x) \
  (ASSERT(-1 <= (x)->ob_size && (x)->ob_size <= 1), \
  (x)->ob_size < 0 ? -(sdigit)(x)->ob_digit[0] : \
 ((x)->ob_size == 0 ? (sdigit)0 : (sdigit)(x)->ob_digit[0]))
#define SIGCHECK(...)   /* nothing */
#define maybe_small_int(x) x
#ifdef CONFIG_SIGNED_RIGHT_SHIFT_ZERO_FILLS
#define ARITHMETIC_RIGHT_SHIFT(type,i,j) \
    ((i) < 0 ? -1-((-1-(i)) >> (j)) : (i) >> (j))
#else
#define ARITHMETIC_RIGHT_SHIFT(type,i,j) ((i) >> (j))
#endif



INTERN DREF DeeIntObject *DCALL
int_copy(DeeIntObject const *__restrict self) {
 DREF DeeIntObject *result;
 size_t num_digits;
 num_digits = (size_t)ABS(((DeeIntObject *)self)->ob_size);
 result = (DeeIntObject *)DeeInt_Alloc(num_digits);
 if (result) {
  memcpy(&result->ob_size,&((DeeIntObject *)self)->ob_size,
         (offsetof(DeeIntObject,ob_digit)-
          offsetof(DeeIntObject,ob_size))+
          num_digits*sizeof(digit));
 }
 return result;
}

INTERN DREF DeeIntObject *DCALL
int_normalize(/*inherit(always)*/DREF DeeIntObject *__restrict v) {
 dssize_t j = ABS(v->ob_size);
 dssize_t i = j;
 while (i > 0 && v->ob_digit[i-1] == 0) --i;
 if (i != j) v->ob_size = (v->ob_size < 0) ? -i : i;
 return v;
}

PRIVATE DREF DeeIntObject *DCALL
x_add(DeeIntObject *__restrict a,
      DeeIntObject *__restrict b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t size_b = ABS(b->ob_size);
 dssize_t i; DeeIntObject *z;
 digit carry = 0;
 if (size_a < size_b) {
  SWAP(DeeIntObject *,a,b);
  SWAP(dssize_t,size_a,size_b);
 }
 z = DeeInt_Alloc(size_a+1);
 if (z == NULL) return NULL;
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
}

PRIVATE DREF DeeIntObject *DCALL
x_sub(DeeIntObject *__restrict a,
      DeeIntObject *__restrict b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t size_b = ABS(b->ob_size);
 dssize_t i; DeeIntObject *z;
 int sign = 1; digit borrow = 0;
 if (size_a < size_b) {
  sign = -1;
  SWAP(DeeIntObject *,a,b);
  SWAP(dssize_t,size_a,size_b);
 } else if (size_a == size_b) {
  i = size_a;
  while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i]);
  if (i < 0) return_reference_((DeeIntObject *)&DeeInt_Zero);
  if (a->ob_digit[i] < b->ob_digit[i]) {
   sign = -1;
   { DeeIntObject *temp = a; a = b; b = temp; }
  }
  size_a = size_b = i+1;
 }
 z = DeeInt_Alloc(size_a);
 if (z == NULL) return NULL;
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
 if (sign < 0) z->ob_size = -z->ob_size;
 return int_normalize(z);
}

INTERN DREF DeeObject *DCALL
int_add(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *z;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL)
      return NULL;
 if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
  z = (DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) +
                                       MEDIUM_VALUE(b));
  goto done;
 }
 if (a->ob_size < 0) {
  if (b->ob_size < 0) {
   z = x_add(a,b);
   if (z && z->ob_size)
       z->ob_size = -z->ob_size;
  } else {
   z = x_sub(b,a);
  }
 } else if (b->ob_size < 0) {
  z = x_sub(a,b);
 } else {
  z = x_add(a,b);
 }
done:
 Dee_Decref(b);
 return (DeeObject *)z;
}

INTERN DREF DeeObject *DCALL
int_sub(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *z;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
  z = (DeeIntObject *)DeeInt_NewMedian(MEDIUM_VALUE(a) -
                                       MEDIUM_VALUE(b));
  goto done;
 }
 if (a->ob_size < 0) {
  if (b->ob_size < 0)
       z = x_sub(a,b);
  else z = x_add(a,b);
  if (z != NULL/* && z->ob_size != 0*/)
      z->ob_size = -z->ob_size;
 } else {
  if (b->ob_size < 0)
       z = x_add(a,b);
  else z = x_sub(a,b);
 }
done:
 Dee_Decref(b);
 return (DeeObject *)z;
}



PRIVATE DREF DeeIntObject *DCALL
x_add_int(DeeIntObject *__restrict a, digit b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t i; DeeIntObject *z;
 digit carry;
 ASSERT(size_a >= 2);
 z = DeeInt_Alloc(size_a+1);
 if (z == NULL) return NULL;
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
}

PRIVATE DREF DeeIntObject *DCALL
x_add_int2(DeeIntObject *__restrict a, twodigits b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t i; DeeIntObject *z; digit carry;
 ASSERT(size_a >= 2);
 z = DeeInt_Alloc(size_a+1);
 if (z == NULL) return NULL;
 carry = a->ob_digit[0] + (b & DIGIT_MASK);
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
}

#if (DIGIT_BITS * 2) < 32
PRIVATE DREF DeeIntObject *DCALL
x_add_int3(DeeIntObject *__restrict a, uint32_t b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t i; DeeIntObject *z; digit carry;
 ASSERT(size_a >= 2);
 if (size_a == 2) {
  uint64_t a_value;
  a_value = a->ob_digit[0] | (a->ob_digit[1] << DIGIT_BITS);
  if (a->ob_size < 0)
      return (DREF DeeIntObject *)DeeInt_NewS64((-(int64_t)a_value) + (int64_t)b);
  return (DREF DeeIntObject *)DeeInt_NewU64(a_value + b);
 }
 ASSERT(size_a >= 3);
 z = DeeInt_Alloc(size_a+1);
 if (z == NULL) return NULL;
 carry = a->ob_digit[0] + (b & DIGIT_MASK);
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
}
#endif


PRIVATE DREF DeeIntObject *DCALL
x_sub_revint(digit a, DeeIntObject *__restrict b) {
 dssize_t size_b = ABS(b->ob_size);
 dssize_t i; DeeIntObject *z; digit borrow;
 ASSERT(size_b >= 2);
 z = DeeInt_Alloc(size_b);
 if (z == NULL) return NULL;
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
}

#if (DIGIT_BITS * 2) < 32
PRIVATE DREF DeeIntObject *DCALL
x_sub_int3(DeeIntObject *__restrict a, uint32_t b);
PRIVATE DREF DeeIntObject *DCALL
x_sub_revint3(uint32_t a, DeeIntObject *__restrict b) {
 dssize_t size_b = ABS(b->ob_size);
 DeeIntObject *z; digit borrow;
 ASSERT(size_b >= 2);
 if (3 < size_b) {
  z = x_sub_int3(b,a);
  if (z) z->ob_size = -z->ob_size;
  return z;
 }
 ASSERT(size_b == 2 || size_b == 3);
 if (3 == size_b) {
  uint64_t b_value;
  b_value   = b->ob_digit[2];
  b_value <<= DIGIT_BITS;
  b_value  |= b->ob_digit[1];
  b_value <<= DIGIT_BITS;
  b_value  |= b->ob_digit[0];
  if (a == b_value)
      return_reference_((DeeIntObject *)&DeeInt_Zero);
  if (a < b_value) {
   z = (DeeIntObject *)DeeInt_NewU64(b_value - a);
   if (z) z->ob_size = -z->ob_size;
   return z;
  }
  return (DeeIntObject *)DeeInt_NewU64(a - b_value);
 }
 ASSERT(size_b == 2);
 z = DeeInt_Alloc(3);
 if (z == NULL) return NULL;
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
}
#endif


PRIVATE DREF DeeIntObject *DCALL
x_sub_int(DeeIntObject *__restrict a, digit b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t i; DeeIntObject *z; digit borrow;
 ASSERT(size_a >= 2);
 z = DeeInt_Alloc(size_a);
 if (z == NULL) return NULL;
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
}

PRIVATE DREF DeeIntObject *DCALL
x_sub_int2(DeeIntObject *__restrict a, twodigits b) {
 dssize_t size_a = ABS(a->ob_size);
 dssize_t i; DeeIntObject *z; digit borrow;
 ASSERT(size_a >= 2);
 if (size_a == 2) {
  twodigits a_value;
  a_value   = a->ob_digit[1];
  a_value <<= DIGIT_BITS;
  a_value  |= a->ob_digit[0];
  if (a_value == b)
      return_reference_((DeeIntObject *)&DeeInt_Zero);
  if (a_value < b) {
   b -= a_value;
   if (b <= DIGIT_MASK) {
    z = DeeInt_Alloc(2);
    if unlikely(!z) return NULL;
    z->ob_digit[0] = (digit)b;
    z->ob_size = -1;
   } else {
    z = DeeInt_Alloc(2);
    if unlikely(!z) return NULL;
    z->ob_digit[0] = b & DIGIT_MASK;
    z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
    z->ob_size = -2;
   }
   return z;
  }
  a_value -= b;
  if (a_value <= DIGIT_MASK) {
   z = DeeInt_Alloc(2);
   if unlikely(!z) return NULL;
   z->ob_digit[0] = (digit)a_value;
  } else {
   z = DeeInt_Alloc(2);
   if unlikely(!z) return NULL;
   z->ob_digit[0] = a_value & DIGIT_MASK;
   z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
  }
  return z;
 }
 z = DeeInt_Alloc(size_a);
 if (z == NULL) return NULL;
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
}

#if (DIGIT_BITS * 2) < 32
PRIVATE DREF DeeIntObject *DCALL
x_sub_int3(DeeIntObject *__restrict a, uint32_t b) {
 dssize_t size_a = ABS(a->ob_size);
 digit borrow; dssize_t i; DeeIntObject *z;
 ASSERT(size_a >= 2);
 if (size_a == 2) {
  z = x_sub_revint3(b,a);
  if (z) z->ob_size = -z->ob_size;
  return z;
 }
 if (size_a == 3) {
  uint64_t a_value;
  a_value   = a->ob_digit[2];
  a_value <<= DIGIT_BITS;
  a_value  |= a->ob_digit[1];
  a_value <<= DIGIT_BITS;
  a_value  |= a->ob_digit[0];
  if (a_value == b)
      return_reference_((DeeIntObject *)&DeeInt_Zero);
  if (a_value < b) {
   b -= (uint32_t)a_value;
   if (b <= DIGIT_MASK) {
    z = DeeInt_Alloc(2);
    if unlikely(!z) return NULL;
    z->ob_digit[0] = (digit)b;
    z->ob_size = -1;
   } else if (b <= ((twodigits)1 << (DIGIT_BITS * 2))-1) {
    z = DeeInt_Alloc(2);
    if unlikely(!z) return NULL;
    z->ob_digit[0] = b & DIGIT_MASK;
    z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
    z->ob_size = -2;
   } else {
    z = DeeInt_Alloc(3);
    if unlikely(!z) return NULL;
    z->ob_digit[0] = b & DIGIT_MASK;
    z->ob_digit[1] = (b >> DIGIT_BITS) & DIGIT_MASK;
    z->ob_digit[2] = (b >> (DIGIT_BITS*2)) & DIGIT_MASK;
    z->ob_size = -3;
   }
   return z;
  }
  a_value -= b;
  if (a_value <= DIGIT_MASK) {
   z = DeeInt_Alloc(2);
   if unlikely(!z) return NULL;
   z->ob_digit[0] = (digit)a_value;
  } else if (a_value <= ((twodigits)1 << (DIGIT_BITS * 2))-1) {
   z = DeeInt_Alloc(2);
   if unlikely(!z) return NULL;
   z->ob_digit[0] = a_value & DIGIT_MASK;
   z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
  } else {
   z = DeeInt_Alloc(3);
   if unlikely(!z) return NULL;
   z->ob_digit[0] = a_value & DIGIT_MASK;
   z->ob_digit[1] = (a_value >> DIGIT_BITS) & DIGIT_MASK;
   z->ob_digit[2] = (a_value >> (DIGIT_BITS*2)) & DIGIT_MASK;
  }
  return z;
 }
 z = DeeInt_Alloc(size_a);
 if (z == NULL) return NULL;
 borrow = a->ob_digit[0] - (b & DIGIT_MASK);
 z->ob_digit[0] = borrow & DIGIT_MASK;
 borrow >>= DIGIT_BITS;
 borrow &= 1;
 borrow = a->ob_digit[1] - ((b >> DIGIT_BITS) & DIGIT_MASK) - borrow;
 z->ob_digit[1] = borrow & DIGIT_MASK;
 borrow >>= DIGIT_BITS;
 borrow &= 1;
 borrow = a->ob_digit[2] - ((b >> (DIGIT_BITS*2)) & DIGIT_MASK) - borrow;
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
}
#endif



PRIVATE DREF DeeIntObject *DCALL
x_sub_revint2(twodigits a,
              DeeIntObject *__restrict b) {
 dssize_t size_b = ABS(b->ob_size);
 DeeIntObject *z;
 digit borrow;
 ASSERT(size_b >= 2);
 if (2 < size_b) {
  z = x_sub_int2(b,a);
  if (z) z->ob_size = -z->ob_size;
  return z;
 }
 ASSERT(2 == size_b);
 if ((a & DIGIT_MASK) == b->ob_digit[0]) {
  if (((a >> DIGIT_BITS) & DIGIT_MASK) == b->ob_digit[1])
        return_reference_((DeeIntObject *)&DeeInt_Zero);
  if (((a >> DIGIT_BITS) & DIGIT_MASK) < b->ob_digit[1])
        goto do_reverse;
 } else if ((a & DIGIT_MASK) < b->ob_digit[0]) {
do_reverse:
  z = x_sub_int2(b,a);
  if (z) z->ob_size = -z->ob_size;
  return z;
 }
 z = DeeInt_Alloc(2);
 if (z == NULL) return NULL;
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
}

INTERN DREF DeeObject *DCALL
DeeInt_AddSDigit(DeeIntObject *__restrict a, sdigit b) {
 DeeIntObject *z;
 if (!b) return_reference_((DREF DeeObject *)a);
 if (ABS(a->ob_size) <= 1)
     return DeeInt_NewMedian(MEDIUM_VALUE(a) + b);
 if (a->ob_size < 0) {
  if (b < 0) {
   z = x_add_int(a,(digit)-b);
   if (z && z->ob_size)
       z->ob_size = -z->ob_size;
  } else {
   z = x_sub_revint((digit)b,a);
  }
 } else if (b < 0) {
  z = x_sub_int(a,(digit)-b);
 } else {
  z = x_add_int(a,(digit)b);
 }
 return (DREF DeeObject *)z;
}

INTERN DREF DeeObject *DCALL
DeeInt_AddU32(DeeIntObject *__restrict a, uint32_t b) {
 DeeIntObject *z;
 if (!b) return_reference_((DREF DeeObject *)a);
 if (ABS(a->ob_size) <= 1)
     return DeeInt_NewS64((int64_t)MEDIUM_VALUE(a) + (int64_t)b);
 if (a->ob_size < 0) {
  if (b <= DIGIT_MASK) {
   z = x_sub_revint((digit)b,a);
  }
#if (DIGIT_BITS * 2) >= 32
  else {
   z = x_sub_revint2((twodigits)b,a);
  }
#else
  else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2))-1) {
   z = x_sub_revint2((twodigits)b,a);
  } else {
   z = x_sub_revint3((twodigits)b,a);
  }
#endif
 } else if (b <= DIGIT_MASK) {
  z = x_add_int(a,(digit)b);
 }
#if (DIGIT_BITS * 2) >= 32
 else {
  z = x_add_int2(a,(twodigits)b);
 }
#else
 else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2))-1) {
  z = x_add_int2(a,(twodigits)b);
 } else {
  z = x_add_int3(a,b);
 }
#endif
 return (DREF DeeObject *)z;
}

INTERN DREF DeeObject *DCALL
DeeInt_SubSDigit(DeeIntObject *__restrict a, sdigit b) {
 DeeIntObject *z;
 if (ABS(a->ob_size) <= 1)
     return DeeInt_NewMedian(MEDIUM_VALUE(a) - b);
 if (a->ob_size < 0) {
  if (b < 0)
       z = x_sub_int(a,(digit)-b);
  else z = x_add_int(a,(digit)b);
  if (z != NULL/* && z->ob_size != 0*/)
      z->ob_size = -z->ob_size;
 } else {
  if (b < 0)
       z = x_add_int(a,(digit)-b);
  else z = x_sub_int(a,(digit)b);
 }
 return (DeeObject *)z;
}

INTERN DREF DeeObject *DCALL
DeeInt_SubU32(DeeIntObject *__restrict a, uint32_t b) {
 DeeIntObject *z;
 if (ABS(a->ob_size) <= 1)
     return DeeInt_NewS64((int64_t)MEDIUM_VALUE(a) - (int64_t)b);
 if (a->ob_size < 0) {
  if (b <= DIGIT_MASK) {
   z = x_add_int(a,(digit)b);
  }
#if (DIGIT_BITS * 2) >= 32
  else {
   z = x_add_int2(a,(twodigits)b);
  }
#else
  else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2))-1) {
   z = x_add_int2(a,(twodigits)b);
  } else {
   z = x_add_int3(a,b);
  }
#endif
  if (z != NULL/* && z->ob_size != 0*/)
      z->ob_size = -z->ob_size;
 } else {
  if (b <= DIGIT_MASK) {
   z = x_sub_int(a,(digit)b);
  }
#if (DIGIT_BITS * 2) >= 32
  else {
   z = x_sub_int2(a,(twodigits)b);
  }
#else
  else if (b <= ((uint32_t)1 << (DIGIT_BITS * 2))-1) {
   z = x_sub_int2(a,(twodigits)b);
  } else {
   z = x_sub_int3(a,b);
  }
#endif
 }
 return (DeeObject *)z;
}


PRIVATE digit DCALL
v_iadd(digit       *__restrict x, dssize_t m,
       digit const *__restrict y, dssize_t n) {
 dssize_t i; digit carry = 0;
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

PRIVATE digit DCALL
v_isub(digit       *__restrict x, dssize_t m,
       digit const *__restrict y, dssize_t n) {
 dssize_t i; digit borrow = 0;
 ASSERT(m >= n);
 for (i = 0; i < n; ++i) {
  borrow = x[i] - y[i] - borrow;
  x[i] = borrow & DIGIT_MASK;
  borrow >>= DIGIT_BITS;
  borrow &= 1;
 }
 for (; borrow && i < m; ++i) {
  borrow = x[i] - borrow;
  x[i] = borrow & DIGIT_MASK;
  borrow >>= DIGIT_BITS;
  borrow &= 1;
 }
 return borrow;
}

PRIVATE digit DCALL
v_lshift(digit *__restrict z,
         digit const *__restrict a,
         dssize_t m, int d) {
 dssize_t i; digit carry = 0;
 ASSERT(0 <= d && d < DIGIT_BITS);
 for (i = 0; i < m; i++) {
  twodigits acc = (twodigits)a[i] << d | carry;
  z[i] = (digit)acc & DIGIT_MASK;
  carry = (digit)(acc >> DIGIT_BITS);
 }
 return carry;
}

PRIVATE digit DCALL
v_rshift(digit *__restrict z,
         digit const *__restrict a,
         dssize_t m, int d) {
 dssize_t i;
 digit carry = 0;
 digit mask = ((digit)1 << d) - 1U;
 ASSERT(0 <= d && d < DIGIT_BITS);
 for (i = m; i-- > 0;) {
  twodigits acc = (twodigits)carry << DIGIT_BITS | a[i];
  carry = (digit)acc & mask;
  z[i] = (digit)(acc >> d);
 }
 return carry;
}

PRIVATE digit DCALL
inplace_divrem1(digit *__restrict pout,
                digit const *__restrict pin,
                dssize_t size, digit n) {
 twodigits rem = 0;
 ASSERT(n > 0 && n <= DIGIT_MASK);
 pin += size;
 pout += size;
 while (--size >= 0) {
  digit hi;
  rem = (rem << DIGIT_BITS) | *--pin;
  *--pout = hi = (digit)(rem / n);
  rem -= (twodigits)hi * n;
 }
 return (digit)rem;
}

PRIVATE DeeIntObject *DCALL
divrem1(DeeIntObject *__restrict a, digit n, digit *prem) {
 DeeIntObject *z;
 dssize_t const size = ABS(a->ob_size);
 ASSERT(n > 0 && n <= DIGIT_MASK);
 z = DeeInt_Alloc(size);
 if (z == NULL) return NULL;
 *prem = inplace_divrem1(z->ob_digit,a->ob_digit,size,n);
 return int_normalize(z);
}


PRIVATE DREF DeeIntObject *DCALL
x_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *z; dssize_t i;
 dssize_t size_a = ABS(a->ob_size);
 dssize_t size_b = ABS(b->ob_size);
 if unlikely((z = DeeInt_Alloc(size_a+size_b)) == NULL) return NULL;
 memset(z->ob_digit,0,z->ob_size*sizeof(digit));
 if (a == b) {
  for (i = 0; i < size_a; ++i) {
   twodigits carry,f = a->ob_digit[i];
   digit *pz = z->ob_digit + (i << 1);
   digit *pa = a->ob_digit + i + 1;
   digit *paend = a->ob_digit + size_a;
   SIGCHECK({ Dee_Decref(z); return NULL; });
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
   twodigits carry = 0,f = a->ob_digit[i];
   digit *pz = z->ob_digit + i;
   digit *pb = b->ob_digit;
   digit *pbend = b->ob_digit + size_b;
   SIGCHECK({ Dee_Decref(z); return NULL; });
   while (pb < pbend) {
    carry += *pz + *pb++ * f;
    *pz++ = (digit)(carry & DIGIT_MASK);
    carry >>= DIGIT_BITS;
    ASSERT(carry <= DIGIT_MASK);
   }
   if (carry)
    *pz += (digit)(carry & DIGIT_MASK);
   ASSERT((carry >> DIGIT_BITS) == 0);
  }
 }
 return int_normalize(z);
}

PRIVATE int DCALL
kmul_split(DeeIntObject *__restrict n, dssize_t size,
           DREF DeeIntObject **__restrict phigh,
           DREF DeeIntObject **__restrict plow) {
 DREF DeeIntObject *hi,*lo;
 dssize_t size_lo,size_hi;
 dssize_t const size_n = ABS(n->ob_size);
 size_lo = MIN(size_n,size);
 size_hi = size_n - size_lo;
 if unlikely((hi = DeeInt_Alloc(size_hi)) == NULL) return -1;
 if unlikely((lo = DeeInt_Alloc(size_lo)) == NULL) { Dee_Decref(hi); return -1; }
 memcpy(lo->ob_digit,n->ob_digit,size_lo * sizeof(digit));
 memcpy(hi->ob_digit,n->ob_digit + size_lo,size_hi * sizeof(digit));
 *phigh = int_normalize(hi);
 *plow = int_normalize(lo);
 return 0;
}

PRIVATE DREF DeeIntObject *DCALL
k_lopsided_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b);
PRIVATE DREF DeeIntObject *DCALL
k_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 dssize_t asize = ABS(a->ob_size);
 dssize_t bsize = ABS(b->ob_size);
 DeeIntObject *ah = NULL,*al = NULL;
 DeeIntObject *bh = NULL,*bl = NULL;
 DeeIntObject *t1,*t2,*t3,*ret = NULL;
 dssize_t shift,i;
 if (asize > bsize) {
  t1 = a,a = b,b = t1;
  i = asize,asize = bsize,bsize = i;
 }
 i = a == b ? KARATSUBA_SQUARE_CUTOFF : KARATSUBA_CUTOFF;
 if (asize <= i) {
  if (asize == 0) return_reference_((DeeIntObject *)&DeeInt_Zero);
  return x_mul(a,b);
 }
 if (2*asize <= bsize) return k_lopsided_mul(a, b);
 shift = bsize >> 1;
 if (kmul_split(a,shift,&ah,&al) < 0) goto fail;
 ASSERT(ah->ob_size > 0);
 if (a == b) {
  bh = ah;
  bl = al;
  Dee_Incref(bh);
  Dee_Incref(bl);
 } else if (kmul_split(b,shift,&bh,&bl) < 0) {
  goto fail;
 }
 ret = DeeInt_Alloc(asize+bsize);
 if (ret == NULL) goto fail;
#ifndef NDEBUG
 memset(ret->ob_digit,0xdf,ret->ob_size*sizeof(digit));
#endif
 if ((t1 = k_mul(ah,bh)) == NULL) goto fail;
 ASSERT(t1->ob_size >= 0);
 ASSERT(2*shift + t1->ob_size <= ret->ob_size);
 memcpy(ret->ob_digit+2*shift,t1->ob_digit,t1->ob_size*sizeof(digit));
 i = ret->ob_size-2*shift-t1->ob_size;
 if (i) memset(ret->ob_digit+2*shift+t1->ob_size,0,i*sizeof(digit));
 if ((t2 = k_mul(al,bl)) == NULL) { Dee_Decref(t1); goto fail; }
 ASSERT(t2->ob_size >= 0);
 ASSERT(t2->ob_size <= 2*shift);
 memcpy(ret->ob_digit,t2->ob_digit,t2->ob_size*sizeof(digit));
 i = 2*shift-t2->ob_size;
 if (i) memset(ret->ob_digit+t2->ob_size,0,i*sizeof(digit));
 i = ret->ob_size-shift;
 (void)v_isub(ret->ob_digit+shift,i,t2->ob_digit,t2->ob_size);
 Dee_Decref(t2);
 (void)v_isub(ret->ob_digit+shift,i,t1->ob_digit,t1->ob_size);
 Dee_Decref(t1);
 if ((t1 = x_add(ah,al)) == NULL) goto fail;
 Dee_Decref(ah);
 Dee_Decref(al);
 ah = al = NULL;
 /* */if (a == b) { t2 = t1; Dee_Incref(t2); }
 else if ((t2 = x_add(bh,bl)) == NULL) { Dee_Decref(t1); goto fail; }
 Dee_Decref(bh);
 Dee_Decref(bl);
 bh = bl = NULL;
 t3 = k_mul(t1,t2);
 Dee_Decref(t1);
 Dee_Decref(t2);
 if (t3 == NULL) goto fail;
 ASSERT(t3->ob_size >= 0);
 (void)v_iadd(ret->ob_digit + shift,i,t3->ob_digit,t3->ob_size);
 Dee_Decref(t3);
 return int_normalize(ret);
fail:
 Dee_XDecref(ret);
 Dee_XDecref(ah);
 Dee_XDecref(al);
 Dee_XDecref(bh);
 Dee_XDecref(bl);
 return NULL;
}

PRIVATE DREF DeeIntObject *DCALL
k_lopsided_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 dssize_t const asize = ABS(a->ob_size);
 dssize_t nbdone,bsize = ABS(b->ob_size);
 DeeIntObject *ret,*bslice = NULL;
 ASSERT(asize > KARATSUBA_CUTOFF);
 ASSERT(2*asize <= bsize);
 ret = DeeInt_Alloc(asize+bsize);
 if (ret == NULL) return NULL;
 memset(ret->ob_digit,0,ret->ob_size*sizeof(digit));
 bslice = DeeInt_Alloc(asize);
 if (bslice == NULL) goto fail;
 nbdone = 0;
 while (bsize > 0) {
  DeeIntObject *product;
  dssize_t const nbtouse = MIN(bsize,asize);
  memcpy(bslice->ob_digit,b->ob_digit+nbdone,nbtouse*sizeof(digit));
  bslice->ob_size = nbtouse;
  product = k_mul(a,bslice);
  if (product == NULL) goto fail;
  (void)v_iadd(ret->ob_digit+nbdone,ret->ob_size-nbdone,
               product->ob_digit,product->ob_size);
  Dee_Decref(product);
  bsize -= nbtouse;
  nbdone += nbtouse;
 }
 Dee_Decref(bslice);
 return int_normalize(ret);
fail:
 Dee_Decref(ret);
 Dee_XDecref(bslice);
 return NULL;
}

INTERN DREF DeeObject *DCALL
int_mul(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DREF DeeIntObject *z;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 if (ABS(a->ob_size) <= 1 && ABS(b->ob_size) <= 1) {
  stwodigits v = (stwodigits)(MEDIUM_VALUE(a)) * MEDIUM_VALUE(b);
  Dee_Decref(b);
  return DeeInt_NewSTwoDigits(v);
 }
 z = k_mul(a,b);
 if (z && ((a->ob_size ^ b->ob_size) < 0)) {
  DREF DeeObject *temp = int_neg(z);
  Dee_Decref(z);
  z = (DREF DeeIntObject *)temp;
 }
 Dee_Decref(b);
 return (DeeObject *)z;
}


PRIVATE DeeIntObject *DCALL
x_divrem(DeeIntObject *__restrict v1,
         DeeIntObject *__restrict w1,
         DeeIntObject **__restrict prem);

PRIVATE int DCALL
int_divrem(DeeIntObject *__restrict a,
           DeeIntObject *__restrict b,
           DeeIntObject **pdiv,
           DeeIntObject **prem) {
 DREF DeeIntObject *z;
 dssize_t size_a = ABS(a->ob_size);
 dssize_t size_b = ABS(b->ob_size);
 if (size_b == 0) { err_divide_by_zero((DeeObject *)a,(DeeObject *)b); return -1; }
 if (size_a < size_b || (size_a == size_b && a->ob_digit[size_a-1] < b->ob_digit[size_b-1])) {
  Dee_Incref(&DeeInt_Zero);
  Dee_Incref(a);
  *pdiv = (DeeIntObject *)&DeeInt_Zero;
  *prem = (DeeIntObject *)a;
  return 0;
 }
 if (size_b == 1) {
  digit rem = 0;
  z = divrem1(a,b->ob_digit[0],&rem);
  if (z == NULL) return -1;
  *prem = (DeeIntObject *)DeeInt_NewDigit(rem);
  if (*prem == NULL) { Dee_Decref(z); return -1; }
 } else {
  z = x_divrem(a,b,prem);
  if (z == NULL)
   return -1;
 }
 if ((a->ob_size < 0) != (b->ob_size < 0)) {
  DREF DeeObject *temp = int_neg(z);
  Dee_Decref(z);
  if (!temp) { Dee_Clear(*prem); return -1; }
  z = (DREF DeeIntObject *)temp;
 }
 if (a->ob_size < 0 && (*prem)->ob_size != 0) {
  DREF DeeObject *temp = int_neg(*prem);
  Dee_Decref(*prem);
  *prem = (DeeIntObject *)temp;
  if (temp == NULL) { Dee_Decref(z); return -1; }
 }
 *pdiv = maybe_small_int(z);
 return 0;
}


PRIVATE unsigned char const BitLengthTable[32] = {
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};

LOCAL int DCALL bits_in_digit(digit d) {
 int d_bits = 0;
 while (d >= 32) { d_bits += 6; d >>= 6; }
 d_bits += (int)BitLengthTable[d];
 return d_bits;
}

PRIVATE DeeIntObject *DCALL
x_divrem(DeeIntObject *__restrict v1,
         DeeIntObject *__restrict w1,
         DeeIntObject **__restrict prem) {
 DeeIntObject *v,*w,*a; dssize_t i,k,size_v,size_w;
 digit wm1,wm2,carry,q,r,vtop,*v0,*vk,*w0,*ak;
 twodigits vv; stwodigits z; sdigit zhi; int d;
 size_v = ABS(v1->ob_size);
 size_w = ABS(w1->ob_size);
 ASSERT(size_v >= size_w && size_w >= 2);
 if unlikely((v = DeeInt_Alloc(size_v+1)) == NULL) { *prem = NULL; return NULL; }
 if unlikely((w = DeeInt_Alloc(size_w)) == NULL) { Dee_Decref(v); *prem = NULL; return NULL; }
 d = DIGIT_BITS - bits_in_digit(w1->ob_digit[size_w-1]);
 carry = v_lshift(w->ob_digit,w1->ob_digit,size_w,d);
 ASSERT(carry == 0);
 carry = v_lshift(v->ob_digit,v1->ob_digit,size_v,d);
 if (carry != 0 || v->ob_digit[size_v-1] >= w->ob_digit[size_w-1]) {
  v->ob_digit[size_v] = carry;
  ++size_v;
 }
 k = size_v-size_w;
 ASSERT(k >= 0);
 if ((a = DeeInt_Alloc(k)) == NULL) {
  Dee_Decref(w);
  Dee_Decref(v);
  *prem = NULL;
  return NULL;
 }
 v0 = v->ob_digit;
 w0 = w->ob_digit;
 wm1 = w0[size_w-1];
 wm2 = w0[size_w-2];
 for (vk = v0+k,ak = a->ob_digit + k; vk-- > v0;) {
  SIGCHECK({ Dee_Decref(a); Dee_Decref(w); Dee_Decref(v); *prem = NULL; return NULL; });
  vtop = vk[size_w];
  ASSERT(vtop <= wm1);
  vv = ((twodigits)vtop << DIGIT_BITS) | vk[size_w-1];
  q = (digit)(vv / wm1);
  r = (digit)(vv - (twodigits)wm1 * q); /* r = vv % wm1 */
  while ((twodigits)wm2 * q > (((twodigits)r << DIGIT_BITS) | vk[size_w-2])) {
   --q,r += wm1;
   if (r >= DIGIT_BASE) break;
  }
  ASSERT(q <= DIGIT_BASE);
  zhi = 0;
  for (i = 0; i < size_w; ++i) {
   z = (sdigit)vk[i]+zhi-(stwodigits)q*(stwodigits)w0[i];
   vk[i] = (digit)z & DIGIT_MASK;
   zhi = (sdigit)ARITHMETIC_RIGHT_SHIFT(stwodigits,z,DIGIT_BITS);
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
 carry = v_rshift(w0,v0,size_w,d);
 ASSERT(carry == 0);
 Dee_Decref(v);
 *prem = int_normalize(w);
 return int_normalize(a);
}


PRIVATE int DCALL
l_divmod(DeeIntObject *__restrict v,
         DeeIntObject *__restrict w,
         DREF DeeIntObject **pdiv,
         DREF DeeIntObject **pmod) {
 DREF DeeIntObject *div,*mod;
 if (int_divrem(v,w,&div,&mod) < 0) return -1;
 if ((mod->ob_size < 0 && w->ob_size > 0) ||
     (mod->ob_size > 0 && w->ob_size < 0)) {
  DeeIntObject *temp;
  temp = (DeeIntObject *)int_add(mod,w);
  Dee_Decref(mod);
  mod = temp;
  if (mod == NULL) { Dee_Decref(div); return -1; }
  if ((temp = (DeeIntObject *)int_sub(div,(DeeIntObject *)&DeeInt_One)) == NULL) {
   Dee_Decref(mod);
   Dee_Decref(div);
   return -1;
  }
  Dee_Decref(div);
  div = temp;
 }
 if (pdiv != NULL) *pdiv = div; else Dee_Decref(div);
 if (pmod != NULL) *pmod = mod; else Dee_Decref(mod);
 return 0;
}

INTERN DeeObject *DCALL int_div(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *div;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 if (l_divmod((DeeIntObject *)a,(DeeIntObject *)b,&div,NULL) < 0)
     div = NULL;
 Dee_Decref(b);
 return (DeeObject *)div;
}

INTERN DeeObject *DCALL int_mod(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *mod;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 if (l_divmod((DeeIntObject *)a,(DeeIntObject *)b,NULL,&mod) < 0)
     mod = NULL;
 Dee_Decref(b);
 return (DeeObject *)mod;
}

INTERN DREF DeeObject *DCALL int_inv(DeeIntObject *__restrict v) {
 DeeIntObject *x;
 if (ABS(v->ob_size) <=1)
     return DeeInt_NewMedian(-(MEDIUM_VALUE(v) + 1));
 x = (DeeIntObject *)int_add(v,(DeeIntObject *)&DeeInt_One);
 if (x == NULL) return NULL;
 x->ob_size = -x->ob_size;
 return (DeeObject *)maybe_small_int(x);
}

INTERN DREF DeeObject *DCALL int_neg(DeeIntObject *__restrict v) {
 DeeIntObject *z;
 if (ABS(v->ob_size) <= 1)
     return DeeInt_NewMedian(-MEDIUM_VALUE(v));
 z = (DeeIntObject *)int_copy(v);
 if (z != NULL) z->ob_size = -v->ob_size;
 return (DeeObject *)z;
}

INTERN DREF DeeObject *DCALL
int_shr(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *z = NULL; digit lomask,himask;
 dssize_t shiftby,newsize,wordshift,loshift,hishift,i,j;
 if (a->ob_size < 0) {
  DeeIntObject *a1,*a2;
  a1 = (DeeIntObject *)int_inv(a);
  if (a1 == NULL) goto rshift_error;
  a2 = (DeeIntObject *)int_shr(a1,b);
  Dee_Decref(a1);
  if (a2 == NULL) goto rshift_error;
  z = (DeeIntObject *)int_inv(a2);
  Dee_Decref(a2);
 } else {
  if (DeeObject_AsSSize((DeeObject *)b,&shiftby)) goto rshift_error;
  if (shiftby < 0) {
   err_shift_negative((DeeObject *)a,(DeeObject *)b,false);
   goto rshift_error;
  }
  wordshift = shiftby / DIGIT_BITS;
  newsize = ABS(a->ob_size) - wordshift;
  if (newsize <= 0) return_reference_((DeeObject *)&DeeInt_Zero);
  loshift = shiftby % DIGIT_BITS;
  hishift = DIGIT_BITS - loshift;
  lomask = ((digit)1 << hishift) - 1;
  himask = DIGIT_MASK ^ lomask;
  z = DeeInt_Alloc(newsize);
  if (z == NULL) goto rshift_error;
  if (a->ob_size < 0) z->ob_size = -(z->ob_size);
  for (i = 0,j = wordshift; i < newsize; i++,j++) {
   z->ob_digit[i] = (a->ob_digit[j] >> loshift) & lomask;
   if (i+1 < newsize)
    z->ob_digit[i] |= (a->ob_digit[j+1] << hishift) & himask;
  }
  z = int_normalize(z);
 }
rshift_error:
 return (DeeObject *)maybe_small_int(z);
}

INTERN DREF DeeObject *DCALL int_shl(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeIntObject *z = NULL; twodigits accum;
 dssize_t shiftby,oldsize,newsize,wordshift,remshift,i,j;
 if (DeeObject_AsSSize((DeeObject *)b,&shiftby)) return NULL;
 if (shiftby < 0) {
  err_shift_negative((DeeObject *)a,(DeeObject *)b,true);
  return NULL;
 }
 wordshift = shiftby / DIGIT_BITS;
 remshift = shiftby - wordshift * DIGIT_BITS;
 oldsize = ABS(a->ob_size);
 newsize = oldsize + wordshift;
 if (remshift) ++newsize;
 z = DeeInt_Alloc(newsize);
 if (z == NULL) return NULL;
 if (a->ob_size < 0) { ASSERT(z->ob_refcnt == 1); z->ob_size = -z->ob_size; }
 for (i = 0; i < wordshift; i++) z->ob_digit[i] = 0;
 accum = 0;
 for (i = wordshift,j = 0; j < oldsize; i++,j++) {
  accum |= (twodigits)a->ob_digit[j] << remshift;
  z->ob_digit[i] = (digit)(accum & DIGIT_MASK);
  accum >>= DIGIT_BITS;
 }
 if (remshift) z->ob_digit[newsize-1] = (digit)accum;
 else ASSERT(!accum);
 z = int_normalize(z);
 return (DeeObject *)maybe_small_int(z);
}

PRIVATE void DCALL
v_complement(digit       *__restrict z,
             digit const *__restrict a,
             dssize_t m) {
 dssize_t i; digit carry = 1;
 for (i = 0; i < m; ++i) {
  carry += a[i] ^ DIGIT_MASK;
  z[i] = carry & DIGIT_MASK;
  carry >>= DIGIT_BITS;
 }
 ASSERT(carry == 0);
}


PRIVATE DREF DeeObject *DCALL
int_bitwise(DeeIntObject *__restrict a, char op,
             DeeIntObject *__restrict b) {
 int nega,negb,negz; DeeIntObject *z;
 dssize_t size_a,size_b,size_z,i;
 size_a = ABS(a->ob_size);
 nega = a->ob_size < 0;
 if (nega) {
  if ((z = DeeInt_Alloc(size_a)) == NULL) return NULL;
  v_complement(z->ob_digit,a->ob_digit,size_a),a = z;
 } else {
  Dee_Incref(a);
 }
 size_b = ABS(b->ob_size);
 negb = b->ob_size < 0;
 if (negb) {
  if ((z = DeeInt_Alloc(size_b)) == NULL) { Dee_Decref(a); return NULL; }
  v_complement(z->ob_digit,b->ob_digit,size_b),b = z;
 } else {
  Dee_Incref(b);
 }
 if (size_a < size_b) {
  z = a,a = b,b = z;
  size_z = size_a,size_a = size_b,size_b = size_z;
  negz = nega,nega = negb,negb = negz;
 }
 switch (op) {
 case '^':
  negz = nega ^ negb;
  size_z = size_a;
  break;
 case '&':
  negz = nega & negb;
  size_z = negb ? size_a : size_b;
  break;
 default:
  negz = nega | negb;
  size_z = negb ? size_b : size_a;
  break;
 }
 if ((z = DeeInt_Alloc(size_z+negz)) == NULL) {
  Dee_Decref(a);
  Dee_Decref(b);
  return NULL;
 }
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
  for (; i < size_z; ++i) {
   z->ob_digit[i] = a->ob_digit[i] ^ DIGIT_MASK;
  }
 } else if (i < size_z) {
  memcpy(&z->ob_digit[i],&a->ob_digit[i],(size_z-i)*sizeof(digit));
 }
 if (negz) {
  z->ob_size = -(z->ob_size);
  z->ob_digit[size_z] = DIGIT_MASK;
  v_complement(z->ob_digit,z->ob_digit,size_z+1);
 }
 Dee_Decref(a);
 Dee_Decref(b);
 return (DeeObject *)maybe_small_int(int_normalize(z));
}

INTERN DREF DeeObject *DCALL int_and(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeObject *c;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 c = int_bitwise((DeeIntObject*)a,'&',(DeeIntObject*)b);
 Dee_Decref(b);
 return c;
}
INTERN DREF DeeObject *DCALL int_xor(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeObject *c;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 c = int_bitwise((DeeIntObject*)a,'^',(DeeIntObject*)b);
 Dee_Decref(b);
 return c;
}
INTERN DREF DeeObject *DCALL int_or(DeeIntObject *__restrict a, DeeIntObject *__restrict b) {
 DeeObject *c;
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL) return NULL;
 c = int_bitwise((DeeIntObject*)a,'|',(DeeIntObject*)b);
 Dee_Decref(b);
 return c;
}


INTERN DREF DeeObject *DCALL
int_pow(DeeIntObject *__restrict a,
        DeeIntObject *__restrict b) {
 DeeIntObject *z; dssize_t i,j,k;
 DeeIntObject *table[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
 if ((b = (DeeIntObject *)DeeObject_Int((DeeObject *)b)) == NULL)
      return NULL;
 a = (DeeIntObject *)a;
 Dee_Incref(a);
 if (b->ob_size < 0) {
  Dee_Decref(a);
  Dee_Decref(b);
#if 0
  return (*DeeFloat_Type.tp_math->tp_pow)(a,b);
#else
  DERROR_NOTIMPLEMENTED();
  return NULL;
#endif
 }
 z = (DeeIntObject *)&DeeInt_One;
 Dee_Incref(z);
#define MULT(x,y,result) \
 do { \
  DREF DeeIntObject *temp; \
  temp = (DeeIntObject *)int_mul(x,y); \
  if (temp == NULL) goto err; \
  Dee_XDecref(result); \
  result = temp; \
 } __WHILE0

 if (b->ob_size <= FIVEARY_CUTOFF) {
  for (i = b->ob_size - 1; i >= 0; --i) {
   digit bi = b->ob_digit[i];
   for (j = (digit)1 << (DIGIT_BITS-1); j != 0; j >>= 1) {
    MULT(z,z,z);
    if (bi & j)
        MULT(z,a,z);
   }
  }
 } else {
  Dee_Incref(z);
  table[0] = z;
  for (i = 1; i < 32; ++i)
       MULT(table[i-1],a,table[i]);
  for (i = b->ob_size - 1; i >= 0; --i) {
   digit const bi = b->ob_digit[i];
   for (j = DIGIT_BITS - 5; j >= 0; j -= 5) {
    int const index = (bi >> j) & 0x1f;
    for (k = 0; k < 5; ++k)
        MULT(z,z,z);
    if (index)
        MULT(z,table[index],z);
   }
  }
 }
out:
 if (b->ob_size > FIVEARY_CUTOFF) {
  for (i = 0; i < 32; ++i)
       Dee_XDecref(table[i]);
 }
 Dee_Decref(a);
 Dee_Decref(b);
 return (DeeObject *)z;
err:
 Dee_Clear(z);
 goto out;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_LOGIC_C */
