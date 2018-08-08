/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
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
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/int.h>
#include <deemon/error.h>
#include <deemon/bool.h>
#include <deemon/none.h>
#include <deemon/arg.h>
#include <deemon/string.h>
#include <deemon/numeric.h>

#include <string.h>
#include <limits.h>
#include <math.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#include "int_logic.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif

DECL_BEGIN

INTERN DREF DeeIntObject *DCALL
#ifdef NDEBUG
DeeInt_Alloc(size_t n_digits)
#else
DeeInt_Alloc_dbg(size_t n_digits, char const *file, int line)
#endif
{
 DREF DeeIntObject *result;
#ifdef NDEBUG
 result = (DREF DeeIntObject *)DeeObject_Malloc(offsetof(DeeIntObject,ob_digit)+
                                                n_digits*sizeof(digit));
#else
 result = (DREF DeeIntObject *)DeeDbgObject_Malloc(offsetof(DeeIntObject,ob_digit)+
                                                   n_digits*sizeof(digit),
                                                   file,
                                                   line);
#endif
 if (result) {
  DeeObject_Init(result,&DeeInt_Type);
  result->ob_size = n_digits;
 }
 return result;
}

/* Create an integer from signed/unsigned LEB data. */
PUBLIC DREF DeeObject *DCALL
DeeInt_NewSleb(uint8_t **__restrict preader) {
 DREF DeeIntObject *result; digit *dst;
 twodigits temp; uint8_t num_bits;
 uint8_t *reader = *preader; size_t num_digits = 1;
 /* Figure out a worst-case for how many digits we'll be needing. */
 while (*reader++ & 0x80) ++num_digits;
 num_digits = ((num_digits*7+(DIGIT_BITS-1))/DIGIT_BITS);
 result = DeeInt_Alloc(num_digits);
 if unlikely(!result) goto done;
 /* Read the integer. */
 reader   = *preader;
 dst      = result->ob_digit;
 num_bits = 6;
 temp     = *reader++ & 0x3f;
 for (;;) {
  while (num_bits < DIGIT_BITS &&
        (reader[-1]&0x80)) {
   /* Set the top-most 7 bits. */
   temp     |= (*reader & 0x7f) << num_bits;
   num_bits += 7;
   if (!(*reader++ & 0x80)) break;
  }
  if (num_bits >= DIGIT_BITS) {
   *dst++    = temp&DIGIT_MASK;
   num_bits -= DIGIT_BITS;
   temp    >>= DIGIT_BITS;
  } else {
   if (!num_bits) { ++dst; break; } /* Simple case: unused. */
   /* Less than one digit. */
   *dst = (digit)temp;
   if (*dst) ++dst;
   break;
  }
 }
 result->ob_size = (size_t)(dst-result->ob_digit);
 /* Check the sign bit. */
 if (**preader & 0x40)
     result->ob_size = -result->ob_size;
 /* Save the new read position. */
 *preader = reader;
done:
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeInt_NewUleb(uint8_t **__restrict preader) {
 DREF DeeIntObject *result; digit *dst;
 twodigits temp; uint8_t num_bits;
 uint8_t *reader = *preader; size_t num_digits = 1;
 /* Figure out a worst-case for how many digits we'll be needing. */
 while (*reader++ & 0x80) ++num_digits;
 num_digits = ((num_digits*7+(DIGIT_BITS-1))/DIGIT_BITS);
 result = DeeInt_Alloc(num_digits);
 if unlikely(!result) goto done;
 /* Read the integer. */
 reader = *preader;
 num_bits = 0,temp = 0;
 dst = result->ob_digit;
 for (;;) {
  while (num_bits < DIGIT_BITS &&
        (reader == *preader || (reader[-1]&0x80))) {
   /* Set the top-most 7 bits. */
   temp     |= (*reader & 0x7f) << num_bits;
   num_bits += 7;
   if (!(*reader++ & 0x80)) break;
  }
  if (num_bits >= DIGIT_BITS) {
   *dst++    = temp&DIGIT_MASK;
   num_bits -= DIGIT_BITS;
   temp    >>= DIGIT_BITS;
  } else {
   if (!num_bits) { ++dst; break; } /* Simple case: unused. */
   /* Less than one digit. */
   *dst = (digit)temp;
   if (*dst) ++dst;
   break;
  }
 }
 result->ob_size = (size_t)(dst-result->ob_digit);
 /* Save the new read position. */
 *preader = reader;
done:
 return (DREF DeeObject *)result;
}


PUBLIC uint8_t *DCALL
DeeInt_GetSleb(DeeObject *__restrict self,
               uint8_t *__restrict writer) {
 twodigits temp; uint8_t num_bits;
 uint8_t *dst = writer; digit *src,*end; size_t size;
 DeeIntObject *me = (DeeIntObject *)self;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeInt_Type);
 size = (size_t)me->ob_size;
 src = me->ob_digit;
 if ((dssize_t)size < 0) {
  /* Negative integer. */
  size = (size_t)-(dssize_t)size;
  /* Special handling for writing the first byte. */
  end      = src+size;
  temp     = *src++;
  num_bits = DIGIT_BITS;
  if (src == end) {
   /* Truncate zero-bits from the most significant digit. */
   while (num_bits &&
       !((temp >> (num_bits-1))&1))
        --num_bits;
   if (num_bits <= 6) {
    *dst++ = 0x40|(uint8_t)temp;
    goto done;
   }
  }
  *dst++   = 0x80|0x40|(temp&0x3f);
  num_bits -= 6;
  temp    >>= 6;
 } else {
  temp = 0,num_bits = 0;
  end = src+size;
 }
 for (;;) {
  if (src != end && num_bits < 7) {
   /* Read one more digit. */
   temp     |= (twodigits)*src++ << num_bits;
   num_bits += DIGIT_BITS;
   if (src == end) {
    /* Truncate zero-bits from the most significant digit. */
    while (num_bits &&
        !((temp >> (num_bits-1))&1))
         --num_bits;
   }
  }
  if (num_bits >= 7) {
   /* Keep on writing digits into the buffer. */
   do {
    *dst++    = 0x80|(temp&0x7f);
    temp    >>= 7;
    num_bits -= 7;
   } while (num_bits >= 7);
  } else {
   /* Last part. */
   if (!num_bits) {
    if (dst == writer) *dst++ = 0;
    /* Clear the continue-bit in the last LEB digit. */
    dst[-1] &= 0x7f;
    break;
   }
   ASSERT(!(temp&0x80));
   *dst++ = (uint8_t)temp;
   break;
  }
 }
done:
 return dst;
}
PUBLIC uint8_t *DCALL
DeeInt_GetUleb(DeeObject *__restrict self,
               uint8_t *__restrict writer) {
 twodigits temp; uint8_t num_bits;
 uint8_t *dst = writer; digit *src,*end;
 DeeIntObject *me = (DeeIntObject *)self;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeInt_Type);
 ASSERT(me->ob_size >= 0);
 temp = 0,num_bits = 0;
 src = me->ob_digit;
 end = src+(size_t)me->ob_size;
 for (;;) {
  if (src != end && num_bits < 7) {
   /* Read one more digit. */
   temp     |= (twodigits)*src++ << num_bits;
   num_bits += DIGIT_BITS;
   if (src == end) {
    /* Truncate zero-bits from the most significant digit. */
    while (num_bits &&
        !((temp >> (num_bits-1))&1))
         --num_bits;
   }
  }
  if (num_bits >= 7) {
   /* Keep on writing digits into the buffer. */
   do {
    *dst++    = 0x80|(temp&0x7f);
    temp    >>= 7;
    num_bits -= 7;
   } while (num_bits >= 7);
  } else {
   /* Last part. */
   if (!num_bits) {
    if (dst == writer) *dst++ = 0;
    /* Clear the continue-bit in the last LEB digit. */
    dst[-1] &= 0x7f;
    break;
   }
   ASSERT(!(temp&0x80));
   *dst++ = (uint8_t)temp;
   break;
  }
 }
/*done:*/
 return dst;
}


#if DIGIT_BITS < 16
PUBLIC DREF DeeObject *DCALL DeeInt_NewS8(int8_t val) {
 DREF DeeIntObject *result; int sign = 1;
 uint8_t abs_val = (uint8_t)val;
 if (val < 0) { sign = -1; abs_val = (uint8_t)0-(uint8_t)val; }
 result = DeeInt_Alloc(1);
 if likely(result) {
  result->ob_size     = sign;
  result->ob_digit[0] = (digit)abs_val;
 }
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL DeeInt_NewU8(uint8_t val) {
 DREF DeeIntObject *result;
 result = DeeInt_Alloc(1);
 if likely(result) {
  result->ob_size     = 1;
  result->ob_digit[0] = (digit)val;
 }
 return (DREF DeeObject *)result;
}
#endif


PUBLIC DREF DeeObject *DCALL DeeInt_NewU16(uint16_t val) {
 DREF DeeIntObject *result;
#if DIGIT_BITS >= 16
 result = DeeInt_Alloc(1);
 if likely(result) {
  result->ob_size     = 1;
  result->ob_digit[0] = (digit)val;
 }
#elif DIGIT_BITS >= 8
 if (!(val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = 1;
   result->ob_digit[0] = (digit)val;
  }
 } else {
  result = DeeInt_Alloc(2);
  if likely(result) {
   result->ob_size = 2;
   result->ob_digit[0] = val & DIGIT_MASK;
   result->ob_digit[1] = val >> DIGIT_BITS;
  }
 }
#else
#error "Not implemented"
#endif
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL DeeInt_NewU32(uint32_t val) {
 DREF DeeIntObject *result;
 size_t req_digits; uint32_t iter;
 if (!(val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = 1;
   result->ob_digit[0] = (digit)val;
  }
 } else {
  for (iter = val,req_digits = 0; iter;
       iter >>= DIGIT_BITS,++req_digits);
  ASSERT(req_digits > 0);
  result = DeeInt_Alloc(req_digits);
  if likely(result) {
   result->ob_size = req_digits;
   for (req_digits = 0; val;
        val >>= DIGIT_BITS,++req_digits)
        result->ob_digit[req_digits] = val & DIGIT_MASK;
  }
 }
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL DeeInt_NewU64(uint64_t val) {
 DREF DeeIntObject *result;
 size_t req_digits; uint64_t iter;
#if __SIZEOF_POINTER__ < 8
 /* When the CPU wasn't designed for 64-bit
  * integers, prefer using 32-bit path. */
 if (val <= UINT32_MAX)
     return DeeInt_NewU32((uint32_t)val);
#endif
 /* NOTE: 32 == Bits required to display everything in the range 0..UINT32_MAX */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
 if (!(val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = 1;
   result->ob_digit[0] = (digit)val;
  }
 } else
#endif
 {
  for (iter = val,req_digits = 0; iter;
       iter >>= DIGIT_BITS,++req_digits);
  ASSERT(req_digits > 0);
  result = DeeInt_Alloc(req_digits);
  if likely(result) {
   result->ob_size = req_digits;
   for (req_digits = 0; val;
        val >>= DIGIT_BITS,++req_digits)
        result->ob_digit[req_digits] = val & DIGIT_MASK;
  }
 }
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL DeeInt_NewS16(int16_t val) {
 DREF DeeIntObject *result; int sign = 1;
 uint16_t abs_val = (uint16_t)val;
 if (val < 0) { sign = -1; abs_val = (uint16_t)0-(uint16_t)val; }
#if DIGIT_BITS >= 16
 result = DeeInt_Alloc(1);
 if likely(result) {
  result->ob_size     = sign;
  result->ob_digit[0] = (digit)abs_val;
 }
#elif DIGIT_BITS >= 8
 if (!(abs_val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = sign;
   result->ob_digit[0] = (digit)abs_val;
  }
 } else {
  result = DeeInt_Alloc(2);
  if likely(result) {
   result->ob_size = 2 * sign;
   result->ob_digit[0] = abs_val & DIGIT_MASK;
   result->ob_digit[1] = abs_val >> DIGIT_BITS;
  }
 }
#else
#error "Not implemented"
#endif
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL DeeInt_NewS32(int32_t val) {
 DREF DeeIntObject *result; int sign = 1;
 size_t req_digits; uint32_t iter,abs_val = (uint32_t)val;
 if (val < 0) { sign = -1; abs_val = (uint32_t)0-(uint32_t)val; }
 if (!(abs_val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = sign;
   result->ob_digit[0] = (digit)abs_val;
  }
 } else {
  for (iter = abs_val,req_digits = 0; iter;
       iter >>= DIGIT_BITS,++req_digits);
  ASSERT(req_digits > 0);
  result = DeeInt_Alloc(req_digits);
  if likely(result) {
   result->ob_size = req_digits * sign;
   for (req_digits = 0; abs_val;
        abs_val >>= DIGIT_BITS,++req_digits)
        result->ob_digit[req_digits] = abs_val & DIGIT_MASK;
  }
 }
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL DeeInt_NewS64(int64_t val) {
 DREF DeeIntObject *result; int sign;
 size_t req_digits; uint64_t iter,abs_val;
#if __SIZEOF_POINTER__ < 8
 /* When the CPU wasn't designed for 64-bit
  * integers, prefer using 32-bit path. */
 if (val >= INT32_MIN && val <= INT32_MAX)
     return DeeInt_NewS32((int32_t)val);
#endif
 sign = 1,abs_val = (uint64_t)val;
 if (val < 0) { sign = -1; abs_val = (uint64_t)0-(uint64_t)val; }
 /* NOTE: 32 == Bits required to display everything in the range 0..MAX(-INT32_MIN,INT32_MAX) */
#if __SIZEOF_POINTER__ >= 8 || DIGIT_BITS > 32
 if (!(abs_val >> DIGIT_BITS)) {
  if (!val) return_reference_((DeeObject *)&DeeInt_Zero);
  /* Fast-path: The integer fits into a single digit. */
  result = DeeInt_Alloc(1);
  if likely(result) {
   result->ob_size     = sign;
   result->ob_digit[0] = (digit)abs_val;
  }
 } else
#endif
 {
  for (iter = abs_val,req_digits = 0; iter;
       iter >>= DIGIT_BITS,++req_digits);
  ASSERT(req_digits > 0);
  result = DeeInt_Alloc(req_digits);
  if likely(result) {
   result->ob_size = req_digits * sign;
   for (req_digits = 0; abs_val;
        abs_val >>= DIGIT_BITS,++req_digits)
        result->ob_digit[req_digits] = abs_val & DIGIT_MASK;
  }
 }
 return (DREF DeeObject *)result;
}


/* 128-bit integer creation. */
PUBLIC DREF DeeObject *DCALL
DeeInt_NewU128(duint128_t val) {
 DREF DeeIntObject *result;
 size_t req_digits; duint128_t iter;
 /* Simplification: When it fits into a 64-bit integer, use that path! */
 if (DUINT128_IS64(val))
     return DeeInt_NewU64(DUINT128_GET64(val)[DEE_INT128_LS64]);
 /* The remainder is basically the same as any other creator, but
  * using special macros implementing some basic 128-bit arithmetic. */
 for (iter = val,req_digits = 0; !DSINT128_ISNUL(iter);
      DUINT128_SHR(iter,DIGIT_BITS),++req_digits);
 ASSERT(req_digits > 0);
 result = DeeInt_Alloc(req_digits);
 if likely(result) {
  result->ob_size = req_digits;
  for (req_digits = 0; !DSINT128_ISNUL(val);
       DUINT128_SHR(val,DIGIT_BITS),++req_digits) {
#if DIGIT_BITS == 30
   result->ob_digit[req_digits] = (digit)(DUINT128_GET32(val)[DEE_INT128_LS32] & DIGIT_MASK);
#else
   result->ob_digit[req_digits] = (digit)(DUINT128_GET16(val)[DEE_INT128_LS16] & DIGIT_MASK);
#endif
  }
 }
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL
DeeInt_NewS128(dint128_t val) {
 DREF DeeIntObject *result; int sign;
 size_t req_digits; duint128_t iter,abs_val;
 if (DSINT128_IS64(val))
     return DeeInt_NewS64(DUINT128_GETS64(val)[DEE_INT128_LS64]);
 /* The remainder is basically the same as any other creator, but
  * using special macros implementing some basic 128-bit arithmetic. */
 sign = 1,*(dint128_t *)&abs_val = val;
 if (DSINT128_ISNEG(val)) { sign = -1; DSINT128_TONEG(abs_val); }
 for (iter = abs_val,req_digits = 0; !DSINT128_ISNUL(iter);
      DUINT128_SHR(iter,DIGIT_BITS),++req_digits);
 ASSERT(req_digits > 0);
 result = DeeInt_Alloc(req_digits);
 if likely(result) {
  result->ob_size = req_digits * sign;
  for (req_digits = 0; !DSINT128_ISNUL(abs_val);
       DUINT128_SHR(abs_val,DIGIT_BITS),++req_digits) {
#if DIGIT_BITS == 30
   result->ob_digit[req_digits] = (digit)(DUINT128_GET32(abs_val)[DEE_INT128_LS32] & DIGIT_MASK);
#else
   result->ob_digit[req_digits] = (digit)(DUINT128_GET16(abs_val)[DEE_INT128_LS16] & DIGIT_MASK);
#endif
  }
 }
 return (DREF DeeObject *)result;
}


#if DIGIT_BITS == 30
#define DeeInt_DECIMAL_SHIFT   9                 /* max(e such that 10**e fits in a digit) */
#define DeeInt_DECIMAL_BASE  ((digit)1000000000) /* 10 ** DECIMAL_SHIFT */
#else
#define DeeInt_DECIMAL_SHIFT   4            /* max(e such that 10**e fits in a digit) */
#define DeeInt_DECIMAL_BASE  ((digit)10000) /* 10 ** DECIMAL_SHIFT */
#endif


PRIVATE double log_base_BASE[37] = {0.0e0,};
PRIVATE int convwidth_base[37] = {0,};
PRIVATE twodigits convmultmax_base[37] = {0,};

LOCAL DREF DeeIntObject *DCALL
int_from_nonbinary_string(char *__restrict begin,
                          char *__restrict end,
                          unsigned int radix,
                          uint32_t radix_and_flags) {
 /* !!!DISCLAIMER!!! This function was originally taken from python,
  *                  but has been heavily modified since. */
 DREF DeeIntObject *result;
 twodigits convmultmax,convmult,c;
 size_t size_z; digit *pz,*pzstop;
 int i,convwidth;
 if (convwidth_base[radix] == 0) {
  twodigits convmax = radix;
  log_base_BASE[radix] = (log((double)radix) /
                          log((double)DIGIT_BASE));
  for (i = 1;;) {
   twodigits next = convmax * radix;
   if (next > DIGIT_BASE) break;
   convmax = next;
   ++i;
  }
  convmultmax_base[radix] = convmax;
  ASSERT(i > 0);
  convwidth_base[radix] = i;
 }
 size_z = (size_t)((end-begin)*log_base_BASE[radix])+1;
 result = DeeInt_Alloc(size_z);
 if (result == NULL)
     return NULL;
 result->ob_size = 0;
 convwidth = convwidth_base[radix];
 convmultmax = convmultmax_base[radix];
 while (begin < end) {
#if 1
  c = 0;
  for (i = 0; i < convwidth && begin != end; ++i,++begin) {
   digit dig; char ch;
parse_ch:
   ch = *begin;
   /* */if (ch >= '0' && ch <= '9') dig = ch-'0';
   else if (ch >= 'a' && ch <= 'z') dig = 10+(ch-'a');
   else if (ch >= 'A' && ch <= 'Z') dig = 10+(ch-'A');
#if 0
   else if (DeeUni_IsDigit(ch)) dig = DeeUni_AsDigit(ch);
#endif
   else if (ch != '\\' || !(radix_and_flags&DEEINT_STRING_FESCAPED)) goto invalid_r;
   else if (begin >= end-2) goto invalid_r;
   else {
    if (begin[1] == '\n') { begin += 2; goto parse_ch; }
    if (begin[1] == '\r') { begin += 2; if (begin != end && *begin == '\n') ++begin; goto parse_ch; }
    goto invalid_r;
   }
   if unlikely(dig >= radix)
      goto invalid_r;
   c = (twodigits)(c*radix+dig);
   ASSERT(c < DIGIT_BASE);
  }
#else
  c = (digit)_PyLong_DigitValue[Py_CHARMASK(*begin++)];
  for (i = 1; i < convwidth && begin != end; ++i,++begin) {
   c = (twodigits)(c*radix+(int)_PyLong_DigitValue[Py_CHARMASK(*begin)]);
   ASSERT(c < DIGIT_BASE);
  }
#endif
  convmult = convmultmax;
  if (i != convwidth) {
   convmult = radix;
   for (; i > 1; --i)
       convmult *= radix;
  }
  pz = result->ob_digit;
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
    tmp = DeeInt_Alloc(size_z+1);
    if (tmp == NULL) {
     Dee_DecrefDokill(result);
     return NULL;
    }
    memcpy(tmp->ob_digit,result->ob_digit,size_z*sizeof(digit));
    Dee_DecrefDokill(result);
    result = tmp;
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


PUBLIC DREF DeeObject *DCALL
DeeInt_FromString(/*utf-8*/char const *__restrict str,
                  size_t len, uint32_t radix_and_flags) {
 unsigned int radix = radix_and_flags >> DEEINT_STRING_RSHIFT;
 bool negative = false; DREF DeeIntObject *result;
 char *iter,*begin = (char *)str,*end = (char *)str+len;
 digit *dst; twodigits number; uint8_t num_bits;
 uint8_t bits_per_digit;
 /* Parse a sign prefix. */
 for (;; ++begin) {
  if (begin == end) goto invalid;
  if (*begin == '+') continue;
  if (*begin == '-') { negative = !negative; continue; }
  if (*begin == '\\' && (radix_and_flags&DEEINT_STRING_FESCAPED)) {
   uint32_t begin_plus_one;
   char *new_begin = begin + 1;
   begin_plus_one = utf8_readchar((char const **)&new_begin,end);
   if (DeeUni_IsLF(begin_plus_one)) {
    begin = new_begin;
    if (begin_plus_one == '\r' &&
       *begin == '\n') ++begin;
    continue;
   }
  }
  break;
 }
 if (!radix) {
  /* Automatically determine the radix. */
  char *old_begin = begin; uint32_t leading_zero;
  leading_zero = utf8_readchar((char const **)&begin,end);
  if (DeeUni_IsDigitX(leading_zero,0)) {
   if (begin == end) /* Special case: int(0) */
       return_reference_((DeeObject *)&DeeInt_Zero);
   while (*begin == '\\' && (radix_and_flags&DEEINT_STRING_FESCAPED)) {
    uint32_t begin_plus_one;
    char *new_begin = begin + 1;
    begin_plus_one = utf8_readchar((char const **)&new_begin,end);
    if (DeeUni_IsLF(begin_plus_one)) {
     begin = new_begin;
     if (begin_plus_one == '\r' &&
        *begin == '\n') ++begin;
     continue;
    }
    break;
   }
   /* */if (*begin == 'x' || *begin == 'X') radix = 16,++begin;
   else if (*begin == 'b' || *begin == 'B') radix = 2,++begin;
   else radix = 8;
  } else {
   begin = old_begin;
   radix = 10;
  }
 }
 if unlikely(begin == end)
    goto invalid;
 ASSERT(radix >= 2);
 if ((radix&(radix-1)) != 0) {
  result = int_from_nonbinary_string(begin,end,radix,radix_and_flags);
  /* Check for errors. */
  if unlikely(!ITER_ISOK(result)) {
   if unlikely(result == (DREF DeeIntObject *)ITER_DONE)
      goto invalid;
   goto done;
  }
 } else {
  bits_per_digit = 0; /* bits_per_digit = ceil(sqrt(radix)) */
  while ((unsigned int)(1 << bits_per_digit) < radix) ++bits_per_digit;
  { size_t num_digits = 1+((len*bits_per_digit)/DIGIT_BITS);
    result = DeeInt_Alloc(num_digits);
    if unlikely(!result) goto done;
    memset(result->ob_digit,0,num_digits*sizeof(digit));
  }
  dst = result->ob_digit;
  number = 0,num_bits = 0;
  /* Parse the integer starting with the least significant bits. */
  iter = end;
  while (iter > begin) {
   uint32_t ch; digit dig;
   struct unitraits *trt;
   ch = utf8_readchar_rev((char const **)&iter,begin);
   trt = DeeUni_Descriptor(ch);
   /* */if (trt->ut_flags & UNICODE_FDIGIT) dig = trt->ut_digit;
   else if (ch >= 'a' && ch <= 'z') dig = 10+(digit)(ch-'a');
   else if (ch >= 'A' && ch <= 'Z') dig = 10+(digit)(ch-'A');
   else if (DeeUni_IsLF(ch) &&
           (radix_and_flags & DEEINT_STRING_FESCAPED)) {
    if (iter == begin) goto invalid_r;
    if (iter[-1] == '\\') { --iter; continue; }
    if (iter[-1] == '\r' && ch == '\n' &&
        iter-1 != begin && iter[-2] == '\\')
    { iter -= 2; continue; }
    goto invalid_r;
   } else {
    goto invalid_r;
   }
   /* Got the digit. */
   if unlikely(dig >= radix)
      goto invalid_r;
   /* Add the digit to out number buffer. */
   number   |= (twodigits)dig << num_bits;
   num_bits += bits_per_digit;
   while (num_bits >= DIGIT_BITS) {
    *dst++    = (digit)(number & DIGIT_MASK);
    number  >>= DIGIT_BITS;
    num_bits -= DIGIT_BITS;
   }
  }
  /* Append trailing bits. */
  if (num_bits) {
   ASSERT(num_bits < DIGIT_BITS);
   *dst = (digit)number;
  }
  while (result->ob_size &&
        !result->ob_digit[result->ob_size-1])
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
 if (radix_and_flags&DEEINT_STRING_FTRY)
     return ITER_DONE;
 DeeError_Throwf(&DeeError_ValueError,
                 "Invalid integer %$q",
                 len,str);
 return NULL;
}

PRIVATE dssize_t DCALL
DeeInt_PrintDecimal(DREF DeeIntObject *__restrict self, uint32_t flags,
                    dformatprinter printer, void *arg) {
 /* !!!DISCLAIMER!!! This function was originally taken from python,
  *                  but has been heavily modified since. */
 size_t size,buflen,size_a,i,j; dssize_t result;
 digit *pout,*pin,rem,tenpow;
 int negative; char *buf,*iter;
 size_a   = (size_t)self->ob_size;
 negative = (dssize_t)size_a < 0;
 if (negative) size_a = (size_t)-(dssize_t)size_a;
 if unlikely(size_a > SSIZE_MAX/DIGIT_BITS) {
  DeeError_Throwf(&DeeError_IntegerOverflow,
                  "int too large to format");
err:
  return -1;
 }
 size = 1+size_a*DIGIT_BITS/(3*DeeInt_DECIMAL_SHIFT);
 pout = (digit *)Dee_AMalloc(size*sizeof(digit));
 if (!pout) goto err;
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
 if (size == 0) pout[size++] = 0;
 buflen = 1+1+(size-1)*DeeInt_DECIMAL_SHIFT;
 tenpow = 10;
 rem = pout[size-1];
 while (rem >= tenpow) { tenpow *= 10; ++buflen; }
 /* Allocate a string target buffer. */
 buf = (char *)Dee_AMalloc(buflen*sizeof(char));
 if unlikely(!buf) goto err_pout;
 iter = buf+buflen;
 for (i = 0; i < size-1; ++i) {
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
 /* */if (negative) *--iter = '-';
 else if (flags&DEEINT_PRINT_FSIGN) *--iter = '+';
 result = (*printer)(arg,iter,(buf+buflen)-iter);
 Dee_AFree(buf);
done_pout:
 Dee_AFree(pout);
 return result;
err_pout: result = -1; goto done_pout;
}

PRIVATE char const int_digits[2][18] = {
    {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','x','q'}, /* Lower */
    {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','X','Q'}  /* Upper */
};


PUBLIC dssize_t DCALL
DeeInt_Print(DREF DeeObject *__restrict self, uint32_t radix_and_flags,
             dformatprinter printer, void *arg) {
 ASSERT_OBJECT_TYPE(self,&DeeInt_Type);
 switch (radix_and_flags >> DEEINT_PRINT_RSHIFT) {
 case 10:
  return DeeInt_PrintDecimal((DeeIntObject *)self,radix_and_flags,printer,arg);
 {
  twodigits number; digit *src;
  uint8_t num_bits,dig_bits,dig_mask,dig;
  char *buf,*iter; size_t bufsize;
  dssize_t result; size_t num_digits;
  DeeIntObject *me; char *digit_chars;
  /* Power-of-2 radices. */
 case 2:  dig_bits = 1,dig_mask = 0x1; goto do_print;
 case 4:  dig_bits = 2,dig_mask = 0x3; goto do_print;
 case 8:  dig_bits = 3,dig_mask = 0x7; goto do_print;
 case 16: dig_bits = 4,dig_mask = 0xf;
do_print:
  me = (DeeIntObject *)self;
  num_digits = (size_t)me->ob_size;
#if DEEINT_PRINT_FUPPER == 1
  digit_chars = (char *)int_digits[radix_and_flags&DEEINT_PRINT_FUPPER];
#else
  digit_chars = (char *)int_digits[(radix_and_flags&DEEINT_PRINT_FUPPER) ? 1 : 0];
#endif
  if ((dssize_t)num_digits <= 0) {
   if (!num_digits) {
    bufsize = 4;
    buf = (char *)Dee_AMalloc(bufsize*sizeof(char));
    if unlikely(!buf) goto err;
    iter = buf+bufsize;
    *--iter = '0';
    goto do_print_prefix;
   }
   num_digits = (size_t)-(dssize_t)num_digits;
  }
  bufsize = 4+((num_digits*DIGIT_BITS)/dig_bits);
  buf = (char *)Dee_AMalloc(bufsize*sizeof(char));
  if unlikely(!buf) goto err;
  iter = buf+bufsize;
  src = me->ob_digit;
  number = 0,num_bits = 0;
  do {
   if (num_bits < dig_bits && num_digits) {
    number   &= (1 << num_bits)-1;
    number   |= (twodigits)*src++ << num_bits;
    num_bits += DIGIT_BITS;
    if (!--num_digits) {
     /* Strip leading ZERO-digits from the output. */
     while (num_bits && !((number >> (num_bits-1))&1)) --num_bits;
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
   dig = number & dig_mask;
   *--iter = digit_chars[dig];
  }
do_print_prefix:
  /* Print the numsys prefix. */
  if (radix_and_flags&DEEINT_PRINT_FNUMSYS) {
   if (dig_bits == 4) *--iter = digit_chars[16]; /* x */
   if (dig_bits == 2) *--iter = digit_chars[17]; /* q */
   if (dig_bits == 1) *--iter = digit_chars[11]; /* b */
   *--iter = '0';
  }
  /* Print the sign prefix. */
  /* */if (me->ob_size < 0) *--iter = '-';
  else if (radix_and_flags&DEEINT_PRINT_FSIGN) *--iter = '+';
  result = (*printer)(arg,iter,(size_t)((buf+bufsize)-iter));
  Dee_AFree(buf);
  return result;
 } break;
 default: break;
 }
 DeeError_Throwf(&DeeError_NotImplemented,
                 "Unsupported integer radix %u",
                (unsigned)(radix_and_flags >> DEEINT_PRINT_RSHIFT));
err:
 return -1;
}


PUBLIC int DCALL
DeeInt_TryGet32(DeeObject *__restrict self, int32_t *__restrict value) {
 uint32_t prev,result; int sign;
 dssize_t i;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeInt_Type);
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
 result = prev = 0,sign = 1;
 i = DeeInt_SIZE(self);
 if (i < 0) sign = -1,i = -i;
 while (--i >= 0) {
  result = (result << DIGIT_BITS) | DeeInt_DIGIT(self)[i];
  if ((result >> DIGIT_BITS) != prev)
       goto overflow;
  prev = result;
 }
 if (sign < 0) {
  if (result <= INT32_MAX) {
   result = (uint32_t)(-(int32_t)result);
  } else if (result == (uint32_t)(0-(uint32_t)INT32_MIN)) {
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
PUBLIC int DCALL
DeeInt_TryGet64(DeeObject *__restrict self, int64_t *__restrict value) {
 uint64_t prev,result; int sign;
 dssize_t i;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeInt_Type);
 switch (DeeInt_SIZE(self)) {
 case 0: *value = 0; return 0;
 case 1:
  *value = DeeInt_DIGIT(self)[0];
  return INT_UNSIGNED;
 case -1:
  *value = -(int64_t)DeeInt_DIGIT(self)[0];
  return INT_SIGNED;
 default: break;
 }
 result = prev = 0,sign = 1;
 i = DeeInt_SIZE(self);
 if (i < 0) sign = -1,i = -i;
 while (--i >= 0) {
  result = (result << DIGIT_BITS) | DeeInt_DIGIT(self)[i];
  if ((result >> DIGIT_BITS) != prev)
       goto overflow;
  prev = result;
 }
 if (sign < 0) {
  if (result <= INT64_MAX) {
   result = (uint64_t)(-(int64_t)result);
  } else if (result == (uint64_t)(0-(uint64_t)INT64_MIN)) {
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

PUBLIC bool (DCALL DeeInt_TryGetS32)(DeeObject *__restrict self,
                                     int32_t *__restrict value) {
 int error = DeeInt_TryGet32(self,value);
 if (error == INT_UNSIGNED && *(uint32_t *)value > INT32_MAX)
     return false;
 return (error != INT_POS_OVERFLOW &&
         error != INT_NEG_OVERFLOW);
}
PUBLIC bool (DCALL DeeInt_TryGetS64)(DeeObject *__restrict self,
                                     int64_t *__restrict value) {
 int error = DeeInt_TryGet64(self,value);
 if (error == INT_UNSIGNED && *(uint64_t *)value > INT32_MAX)
     return false;
 return (error != INT_POS_OVERFLOW &&
         error != INT_NEG_OVERFLOW);
}
PUBLIC bool (DCALL DeeInt_TryGetU32)(DeeObject *__restrict self,
                                     uint32_t *__restrict value) {
 int error = DeeInt_TryGet32(self,(int32_t *)value);
 if (error == INT_SIGNED && *(int32_t *)value < 0)
     return false;
 return (error != INT_POS_OVERFLOW &&
         error != INT_NEG_OVERFLOW);
}
PUBLIC bool (DCALL DeeInt_TryGetU64)(DeeObject *__restrict self,
                                     uint64_t *__restrict value) {
 int error = DeeInt_TryGet64(self,(int64_t *)value);
 if (error == INT_SIGNED && *(int64_t *)value < 0)
     return false;
 return (error != INT_POS_OVERFLOW &&
         error != INT_NEG_OVERFLOW);
}


PUBLIC int DCALL
DeeInt_Get32(DeeObject *__restrict self, int32_t *__restrict value) {
 int result = DeeInt_TryGet32(self,value);
 if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW) {
  err_integer_overflow(self,32,result == INT_POS_OVERFLOW);
  return -1;
 }
 return result;
}
PUBLIC int DCALL
DeeInt_Get64(DeeObject *__restrict self, int64_t *__restrict value) {
 int result = DeeInt_TryGet64(self,value);
 if (result == INT_POS_OVERFLOW || result == INT_NEG_OVERFLOW) {
  err_integer_overflow(self,64,result == INT_POS_OVERFLOW);
  return -1;
 }
 return result;
}

PUBLIC int (DCALL DeeInt_GetS32)(DeeObject *__restrict self, int32_t *__restrict value) {
 int error = DeeInt_Get32(self,value);
 if (error == INT_UNSIGNED && *(uint32_t *)value > INT32_MAX) {
  err_integer_overflow(self,32,true);
  return -1;
 }
 return 0;
}
PUBLIC int (DCALL DeeInt_GetS64)(DeeObject *__restrict self, int64_t *__restrict value) {
 int error = DeeInt_Get64(self,value);
 if (error == INT_UNSIGNED && *(uint64_t *)value > INT64_MAX) {
  err_integer_overflow(self,64,true);
  return -1;
 }
 return 0;
}
PUBLIC int (DCALL DeeInt_GetU32)(DeeObject *__restrict self, uint32_t *__restrict value) {
 int error = DeeInt_Get32(self,(int32_t *)value);
 if (error == INT_SIGNED && *(int32_t *)value < 0) {
  err_integer_overflow(self,32,false);
  return -1;
 }
 return 0;
}
PUBLIC int (DCALL DeeInt_GetU64)(DeeObject *__restrict self, uint64_t *__restrict value) {
 int error = DeeInt_Get64(self,(int64_t *)value);
 if (error == INT_SIGNED && *(int64_t *)value < 0) {
  err_integer_overflow(self,64,false);
  return -1;
 }
 return 0;
}





PRIVATE DREF DeeObject *DCALL int_return_zero(void) {
 return_reference_((DeeObject *)&DeeInt_Zero);
}

PRIVATE DREF DeeObject *DCALL
int_new(size_t argc, DeeObject **__restrict argv) {
 DeeObject *val; uint16_t radix = 0;
 if (DeeArg_Unpack(argc,argv,"o|I16u:int",&val,&radix))
     goto err;
 if (!DeeString_Check(val))
      return DeeObject_Int(val);
 if unlikely(radix == 1) {
  DeeError_Throwf(&DeeError_ValueError,"Invalid radix = 1");
 }
 /* TODO: String encodings? */
 return DeeInt_FromString(DeeString_STR(val),DeeString_SIZE(val),
                          DEEINT_STRING(radix,DEEINT_STRING_FNORMAL));
err:
 return NULL;
}

PRIVATE int DCALL int_bool(DeeObject *__restrict self) {
 return ((DeeIntObject *)self)->ob_size != 0;
}




PRIVATE struct type_math int_math = {
    /* .tp_int32  = */&DeeInt_Get32,
    /* .tp_int64  = */&DeeInt_Get64,
    /* .tp_double = */NULL,
    /* .tp_int    = */&DeeObject_NewRef,
    /* .tp_inv    = */(DeeObject *(DCALL *)(DeeObject *__restrict))&int_inv,
    /* .tp_pos    = */&DeeObject_NewRef,
    /* .tp_neg    = */(DeeObject *(DCALL *)(DeeObject *__restrict))&int_neg,
    /* .tp_add    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_add,
    /* .tp_sub    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_sub,
    /* .tp_mul    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_mul,
    /* .tp_div    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_div,
    /* .tp_mod    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_mod,
    /* .tp_shl    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_shl,
    /* .tp_shr    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_shr,
    /* .tp_and    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_and,
    /* .tp_or     = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_or,
    /* .tp_xor    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_xor,
    /* .tp_pow    = */(DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&int_pow
};




/* Integer compare. */
PRIVATE dssize_t DCALL
int_compareint(DeeIntObject *__restrict a,
               DeeIntObject *__restrict b) {
 dssize_t sign;
 if (a->ob_size != b->ob_size) {
  sign = a->ob_size - b->ob_size;
 } else {
  dssize_t i = a->ob_size;
  if (i < 0) i = -i;
  while (--i >= 0 && a->ob_digit[i] == b->ob_digit[i]);
  if (i < 0) sign = 0;
  else {
   sign = ((sdigit)a->ob_digit[i] -
           (sdigit)b->ob_digit[i]);
   if (a->ob_size < 0) sign = -sign;
  }
 }
 return sign/* < 0 ? -1 : sign > 0 ? 1 : 0*/;
}


PRIVATE dhash_t DCALL
int_hash(DeeIntObject *__restrict self) {
 dhash_t x; dssize_t i; int sign;
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
 sign = 1,x = 0;
 if (i < 0) sign = -1,i = -i;
 while (--i >= 0) {
  x = (x << DIGIT_BITS) | (x >> ((__SIZEOF_POINTER__*8) - DIGIT_BITS));
  x += self->ob_digit[i];
 }
 return x*sign;
}
PRIVATE DREF DeeObject *DCALL
int_cmp_eq(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value == 0);
}
PRIVATE DREF DeeObject *DCALL
int_cmp_ne(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value != 0);
}
PRIVATE DREF DeeObject *DCALL
int_cmp_lo(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value < 0);
}
PRIVATE DREF DeeObject *DCALL
int_cmp_le(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value <= 0);
}
PRIVATE DREF DeeObject *DCALL
int_cmp_gr(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value > 0);
}
PRIVATE DREF DeeObject *DCALL
int_cmp_ge(DeeObject *__restrict self, DeeObject *__restrict some_object) {
 dssize_t compare_value;
 if ((some_object = DeeObject_Int(some_object)) == NULL) return NULL;
 compare_value = int_compareint((DeeIntObject *)self,
                                (DeeIntObject *)some_object);
 Dee_Decref(some_object);
 return_bool(compare_value >= 0);
}


PRIVATE struct type_cmp int_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&int_hash,
    /* .tp_eq   = */&int_cmp_eq,
    /* .tp_ne   = */&int_cmp_ne,
    /* .tp_lo   = */&int_cmp_lo,
    /* .tp_le   = */&int_cmp_le,
    /* .tp_gr   = */&int_cmp_gr,
    /* .tp_ge   = */&int_cmp_ge,
};

PRIVATE DREF DeeObject *DCALL
int_str(DeeObject *__restrict self) {
#if 0 /* XXX: Locale support? And if so, enable the unicode variant here. */
 struct unicode_printer p = UNICODE_PRINTER_INIT;
 /* Simply print this integer to the printer, using decimal encoding. */
 if (DeeInt_Print(self,DEEINT_PRINT_DEC,(dformatprinter)&unicode_printer_print,&p) < 0)
     goto err;
 return unicode_printer_pack(&p);
err:
 unicode_printer_fini(&p);
 return NULL;
#else
 struct ascii_printer p = ASCII_PRINTER_INIT;
 /* Simply print this integer to the printer, using decimal encoding. */
 if (DeeInt_Print(self,DEEINT_PRINT_DEC,
                 (dformatprinter)&ascii_printer_print,
                 &p) < 0)
     goto err;
 return ascii_printer_pack(&p);
err:
 ascii_printer_fini(&p);
 return NULL;
#endif
}

PRIVATE DREF DeeObject *DCALL
int_tostr(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 uint32_t flags = 10 << DEEINT_PRINT_RSHIFT; char *flags_str = NULL;
#ifdef CONFIG_BIG_ENDIAN
 if (DeeArg_Unpack(argc,argv,"|I16us:tostr",&((uint16_t *)&flags)[0],&flags_str))
     goto err;
#else
 if (DeeArg_Unpack(argc,argv,"|I16us:tostr",&((uint16_t *)&flags)[1],&flags_str))
     goto err;
 if (DeeArg_Unpack(argc,argv,"|I16us:tostr",&((uint16_t *)&flags)[1],&flags_str))
     goto err;
#endif
 if (flags_str) {
  char *iter = flags_str;
  for (;;) {
   char ch = *iter++;
   if (!ch) break;
   if (ch == 'u' || ch == 'X') flags |= DEEINT_PRINT_FUPPER;
   else if (ch == 'n' || ch == '#') flags |= DEEINT_PRINT_FNUMSYS;
   else if (ch == 's' || ch == '+') flags |= DEEINT_PRINT_FSIGN;
   else {
    DeeError_Throwf(&DeeError_ValueError,
                    "Invalid integer to string flags %q",
                    flags_str);
    goto err;
   }
  }
 }
 {
#if 0 /* XXX: Locale support? And if so, enable the unicode variant here. */
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  if unlikely(DeeInt_Print(self,flags,(dformatprinter)&unicode_printer_print,&printer) < 0)
     goto err_printer;
  return unicode_printer_pack(&printer);
err_printer:
  unicode_printer_fini(&printer);
#else
  struct ascii_printer printer = ASCII_PRINTER_INIT;
  if unlikely(DeeInt_Print(self,flags,
                          (dformatprinter)&ascii_printer_print,
                          &printer) < 0)
     goto err_printer;
  return ascii_printer_pack(&printer);
err_printer:
  ascii_printer_fini(&printer);
#endif
 }
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
int_hex(DeeObject *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":hex"))
     goto err;
 {
#if 0 /* XXX: Locale support? And if so, enable the unicode variant here. */
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  if unlikely(DeeInt_Print(self,
                           DEEINT_PRINT(16,DEEINT_PRINT_FNUMSYS),
                          (dformatprinter)&unicode_printer_print,
                          &printer) < 0)
     goto err_printer;
  return unicode_printer_pack(&printer);
err_printer:
  unicode_printer_fini(&printer);
#else
  struct ascii_printer printer = ASCII_PRINTER_INIT;
  if unlikely(DeeInt_Print(self,
                           DEEINT_PRINT(16,DEEINT_PRINT_FNUMSYS),
                          (dformatprinter)&ascii_printer_print,
                          &printer) < 0)
     goto err_printer;
  return ascii_printer_pack(&printer);
err_printer:
  ascii_printer_fini(&printer);
#endif
 }
err:
 return NULL;
}

PRIVATE struct type_method int_methods[] = {
    { "tostr", &int_tostr,
      DOC("(int radix=10,string mode=\"\")->string\n"
          "@throw ValueError The given @mode was not recognized\n"
          "@throw NotImplemented The given @radix cannot be represented\n"
          "Convert @this integer to a string, using @radix as base and a "
          "character-options set @mode for which the following control "
          "characters are recognized\n"
          "%{table Option|Description\n"
          "-${\"u\"}, ${\"X\"}|Digits above $10 are printed in upper-case\n"
          "-${\"n\"}, ${\"#\"}|Prefix the integers with its number system prefix (e.g.: ${\"0x\"})\n"
          "-${\"s\"}, ${\"+\"}|Also prepend a sign prefix before positive integers}") },
    { "hex", &int_hex,
      DOC("()->string\n"
          "Short-hand alias for ${this.tostr(16,\"n\")}") },
    { NULL }
};


/* The max sequence size is the signed value of SIZE_MAX,
 * because negative values are reserved to indicate error
 * states. */
#if SSIZE_MAX > UINT32_MAX
DEFINE_UINT64(int_size_max,SSIZE_MAX);
#else
DEFINE_UINT32(int_size_max,SSIZE_MAX);
#endif

PRIVATE struct type_member int_class_members[] = {
    TYPE_MEMBER_CONST_DOC("SIZE_MAX",&int_size_max,
    "The max value acceptable for sequence sizes, or indices\n"
    "Note that despite its name, this constant is not necessarily "
    "equal to the well-known C-constant of the same name, accessible "
    "in deemon as ${(size_t from ctypes).max}\n"
    "Note that this value is guarantied to be sufficiently great, such that "
    "a sequence consisting of SIZE_MAX elements, each addressed as its own "
    "member, or modifyable index in some array, is impossible to achive due "
    "to memory constraints.\n"
    "In this implementation, $SIZE_MAX is ${2**31} on 32-bit hosts, and ${2**63} on 64-bit hosts\n"
    "Custom, mutable sequences with sizes greater than this may expirience inaccuracies "
    "with the default implementation of function such as :sequence.insert's index-argument "
    "potentially not being able to correctly determine if a negative or positive number was given\n"
    "Such behavior may be considered a bug, however it falls under the category of doesn't-matter-wont-fix\n"
    ),
    { NULL }
};


PUBLIC DeeTypeObject DeeInt_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_int),
    /* .tp_doc      = */DOC("The builtin type for representing and operating "
                            "with whole numbers of an arbitrary precision\n"
                            "Note that integers themself are immutable, and that "
                            "inplace operators will change the pointed-object object\n"
                            "\n"
                            "()\n"
                            "Returns the integer constant $0\n"
                            "\n"
                            "(object ob)\n"
                            "@throw NotImplemented The given @ob does not implement ${operator int}\n"
                            "Converts @ob into an integer\n"
                            "\n"
                            "(string s, int radix = 0)\n"
                            "@throw ValueError The given string @s is not a valid integer\n"
                            "@throw ValueError The given @radix is invalid\n"
                            "Convert the given string @s into an integer\n"
                            "When radix is $0, automatically detect it based on a prefix such as ${\"0x\"}. "
                            "Otherwise, use @radix as it is provided\n"
                            "\n"
                            "str->\n"
                            "repr->\n"
                            "Returns @this integer as a decimal-encoded string. Same as ${this.tostr()}\n"
                            "\n"
                            "bool->\n"
                            "Returns :true if @this integer is non-zero\n"
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
                            "<<(int count)->\n"
                            "@throw NegativeShift The given @count is lower than $0\n"
                            "Shift the bits of @this left a total of @count times\n"
                            "\n"
                            ">>(int count)->\n"
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
                            "Return @this by the power of @other\n"
                            "\n"
                            ),
    /* .tp_flags    = */TP_FVARIABLE|TP_FFINAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeNumeric_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&int_return_zero,
                /* .tp_copy_ctor = */&noop_varcopy, /* No need to actually copy. - Integers are immutable! */
                /* .tp_deep_ctor = */&noop_varcopy,
                /* .tp_any_ctor  = */&int_new
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */&int_str,
        /* .tp_repr = */&int_str,
        /* .tp_bool = */&int_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */&int_math,
    /* .tp_cmp           = */&int_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */int_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */int_class_members
};

/* Helpful singletons for some oftenly used integers. */
PUBLIC DeeIntObject DeeInt_Zero = { OBJECT_HEAD_INIT(&DeeInt_Type), 0, { 0 } };
PUBLIC DeeIntObject DeeInt_One = { OBJECT_HEAD_INIT(&DeeInt_Type), 1, { 1 } };
PUBLIC DeeIntObject DeeInt_MinusOne = { OBJECT_HEAD_INIT(&DeeInt_Type), -1, { 1 } };

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_C */
