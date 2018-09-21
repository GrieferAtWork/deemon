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
#ifndef GUARD_DEEMON_INT_H
#define GUARD_DEEMON_INT_H 1

#include "api.h"
#include "object.h"
#include "format.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <hybrid/typecore.h>

/* NOTE: Integer objects have been completely reworked in deemon 2.0,
 *       no longer being split between 8 different types for different
 *       sizes, as well as being becoming immutable objects alongside
 *       the change of disallowing inplace operators on out-of-scope
 *       variables:
 *    >> local my_var = 42;
 *    >> function foo() {
 *    >>     ++my_var; // Illegal since __DEEMON__ >= 200
 *    >>               // Causes a compiler error.
 *    >> }
 *    >> print my_var; // 42
 *    >> foo();
 *    >> print my_var; // 43 (Before 200)
 */

DECL_BEGIN


#ifdef CONFIG_LITTLE_ENDIAN
#define DEE_INT128_MS8       15
#define DEE_INT128_MS16      7
#define DEE_INT128_MS32      3
#define DEE_INT128_MS64      1
#else
#define DEE_INT128_MS8       0
#define DEE_INT128_MS16      0
#define DEE_INT128_MS32      0
#define DEE_INT128_MS64      0
#endif
#define DEE_INT128_LS8  (15-DEE_INT128_MS8)
#define DEE_INT128_LS16 (7-DEE_INT128_MS16)
#define DEE_INT128_LS32 (3-DEE_INT128_MS32)
#define DEE_INT128_LS64 (1-DEE_INT128_MS64)

#ifdef CONFIG_NATIVE_INT128
#define DSINT128_DEC(x)   (--(x))
#define DSINT128_INC(x)   (++(x))
#define DSINT128_INV(x)   ((x) ^= -1)
#define DSINT128_TONEG(x) ((x) = -(x))
#define DSINT128_ISNEG(x) ((x) < 0)
#define DSINT128_ISNUL(x) ((x) == 0)
#define DSINT128_IS64(x)  ((x) >= INT64_MIN && (x) <= INT64_MAX)
#define DUINT128_IS64(x)  ((x) <= UINT64_MAX)
#define DUINT128_OR(x,v)  (void)((x) |= (v))
#define DUINT128_AND(x,v) (void)((x) &= (v))
#define DUINT128_XOR(x,v) (void)((x) ^= (v))
#define DUINT128_SHR(x,n) (*(duint128_t *)&(x) >>= (n))
#define DUINT128_SHL(x,n) (*(duint128_t *)&(x) <<= (n))
#define DUINT128_SET(x,v) (*(duint128_t *)&(x) = (v))
#define DSINT128_SET(x,v) (*(dint128_t *)&(x) = (v))
#define DUINT128_SHL_WILL_OVERFLOW(x,n) \
      (((duint128_t)((duint128_t)(x) << (n)) >> (n)) != (duint128_t)(x))
#else
#define DSINT128_DEC(x)   ((DUINT128_GET64(x)[DEE_INT128_LS64]-- == 0) ? \
                           (void)(--DUINT128_GET64(x)[DEE_INT128_MS64]) : (void)0)
#define DSINT128_INC(x)   ((++DUINT128_GET64(x)[DEE_INT128_LS64] == 0) ? \
                           (void)(++DUINT128_GET64(x)[DEE_INT128_MS64]) : (void)0)
#define DSINT128_INV(x)   (DUINT128_GET64(x)[DEE_INT128_LS64] ^= -1, \
                           DUINT128_GET64(x)[DEE_INT128_MS64] ^= -1)
#define DSINT128_TONEG(x) (DSINT128_DEC(x),DSINT128_INV(x)) /* x = -x  <===>  x = ~(x-1); */
#define DSINT128_ISNEG(x) (DUINT128_GETS8(x)[DEE_INT128_MS8] < 0)
#define DSINT128_ISNUL(x) (!DUINT128_GET64(x)[DEE_INT128_LS64] && \
                           !DUINT128_GET64(x)[DEE_INT128_MS64])
#define DSINT128_IS64(x)  (DUINT128_GET64(x)[DEE_INT128_MS64] == 0 || \
                           DUINT128_GETS64(x)[DEE_INT128_MS64] == -1)
#define DUINT128_IS64(x)  (DUINT128_GET64(x)[DEE_INT128_MS64] == 0)
#define DUINT128_SET(x,v) \
       (DUINT128_GET64(x)[DEE_INT128_MS64] = 0, \
        DUINT128_GET64(x)[DEE_INT128_LS64] = (uint64_t)(v))
#define DSINT128_SET(x,v) \
       (DUINT128_GETS64(x)[DEE_INT128_MS64] = (v) < 0 ? (int64_t)-1 : 0, \
        DUINT128_GETS64(x)[DEE_INT128_LS64] = (int64_t)(v))
#define DUINT128_OR(x,v) \
       (sizeof(v) == 1 ? (void)(DUINT128_GET8(x)[DEE_INT128_LS8] |= (uint8_t)(v)) : \
        sizeof(v) == 2 ? (void)(DUINT128_GET16(x)[DEE_INT128_LS16] |= (uint16_t)(v)) : \
        sizeof(v) == 4 ? (void)(DUINT128_GET32(x)[DEE_INT128_LS32] |= (uint32_t)(v)) : \
                         (void)(DUINT128_GET64(x)[DEE_INT128_LS64] |= (uint64_t)(v)))
#define DUINT128_AND(x,v) \
       (sizeof(v) == 1 ? (void)(DUINT128_GET8(x)[DEE_INT128_LS8] &= (uint8_t)(v)) : \
        sizeof(v) == 2 ? (void)(DUINT128_GET16(x)[DEE_INT128_LS16] &= (uint16_t)(v)) : \
        sizeof(v) == 4 ? (void)(DUINT128_GET32(x)[DEE_INT128_LS32] &= (uint32_t)(v)) : \
                         (void)(DUINT128_GET64(x)[DEE_INT128_LS64] &= (uint64_t)(v)))
#define DUINT128_XOR(x,v) \
       (sizeof(v) == 1 ? (void)(DUINT128_GET8(x)[DEE_INT128_LS8] ^= (uint8_t)(v)) : \
        sizeof(v) == 2 ? (void)(DUINT128_GET16(x)[DEE_INT128_LS16] ^= (uint16_t)(v)) : \
        sizeof(v) == 4 ? (void)(DUINT128_GET32(x)[DEE_INT128_LS32] ^= (uint32_t)(v)) : \
                         (void)(DUINT128_GET64(x)[DEE_INT128_LS64] ^= (uint64_t)(v)))


/* Unsigned shift-right for n <= 64 */
#define DUINT128_SHR(x,n) \
 (DUINT128_GET64(x)[DEE_INT128_LS64] >>= (n), \
  DUINT128_GET64(x)[DEE_INT128_LS64]  |= \
      (DUINT128_GET64(x)[DEE_INT128_MS64] & ((UINT64_C(1) << (n))-1)) << (64-(n)), \
  DUINT128_GET64(x)[DEE_INT128_MS64] >>= (n))
/* Unsigned shift-left for n <= 64 */
#define DUINT128_SHL(x,n) \
 (DUINT128_GET64(x)[DEE_INT128_MS64] <<= (n), \
  DUINT128_GET64(x)[DEE_INT128_MS64] |= \
      (DUINT128_GET64(x)[DEE_INT128_LS64] & ((UINT64_C(1) << (64-(n)))-1)) >> (64-(n)), \
  DUINT128_GET64(x)[DEE_INT128_LS64] <<= (n))
#define DUINT128_SHL_WILL_OVERFLOW(x,n) \
 ((DUINT128_GET64(x)[DEE_INT128_MS64] & (((UINT64_C(1) << (n))-1) << (64-(n)))) != 0)
#endif

#define DINT128_SETMIN(x)  (DUINT128_GET64(x)[DEE_INT128_LS64] = ~(uint64_t)0,DUINT128_GET64(x)[DEE_INT128_MS64] = ~(UINT64_C(1) << 63))
#define DINT128_SETMAX(x)  (DUINT128_GET64(x)[DEE_INT128_LS64] = 0,DUINT128_GET64(x)[DEE_INT128_MS64] = (UINT64_C(1) << 63))
#define DINT128_ISMIN(x)   (DUINT128_GET64(x)[DEE_INT128_LS64] == ~(uint64_t)0 && DUINT128_GET64(x)[DEE_INT128_MS64] == ~(UINT64_C(1) << 63))
#define DINT128_ISMAX(x)   (DUINT128_GET64(x)[DEE_INT128_LS64] == 0 && DUINT128_GET64(x)[DEE_INT128_MS64] == (UINT64_C(1) << 63))
#define DINT128_IS0MMIN(x) (DUINT128_GET64(x)[DEE_INT128_LS64] == 1 && DUINT128_GET64(x)[DEE_INT128_MS64] == (UINT64_C(1) << 63)) /* x == (0 - MIN) */



typedef struct int_object DeeIntObject;

#if __SIZEOF_POINTER__ == 8
#define DIGIT_BITS  30
#else
#define DIGIT_BITS  15
#endif


#if DIGIT_BITS == 30
typedef uint32_t    digit;
typedef int32_t    sdigit;
typedef uint64_t twodigits;
typedef int64_t stwodigits;
#else
typedef uint16_t    digit;
typedef int16_t    sdigit;
typedef uint32_t twodigits;
typedef int32_t stwodigits;
#endif
#define DIGIT_BASE ((digit)1 << DIGIT_BITS)
#define DIGIT_MASK ((digit)(DIGIT_BASE - 1))

struct int_object {
    OBJECT_HEAD
    dssize_t ob_size;
    digit    ob_digit[1]; /* Bit-vector of the integer, split in digits of `DIGIT_BITS' bits each.
                           * Least significant bits come first with individual digits being encoded
                           * in host-endian.
                           * The total number of digits is the absolute value of `ob_size',
                           * which is negative if the value of the integer is too. */
};
#define DeeInt_SIZE(x)  ((DeeIntObject *)REQUIRES_OBJECT(x))->ob_size
#define DeeInt_DIGIT(x) ((DeeIntObject *)REQUIRES_OBJECT(x))->ob_digit


#define DEE_PRIVATE_ABS(value) ((value) < 0 ? -(value) : (value))
#define DEE_PRIVATE_REQ_DIGITS(value) \
   (DEE_PRIVATE_ABS(value) > (1 << DIGIT_BITS) ? 2 : 1)

#define DEFINE_INT_1DIGIT(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[1]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) < 0 ? -1 : (value) > 0 ? 1 : 0, \
 { DEE_PRIVATE_ABS(value)&DIGIT_MASK } \
}
#define DEFINE_UINT_1DIGIT(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[1]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), (value) ? 1 : 0, \
 { (value)&DIGIT_MASK } \
}
#define DEFINE_INT_2DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[2]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 ((value) < 0 ? ((uintmax_t)-(value) > ((uintmax_t)1 << DIGIT_BITS) ? -2 : -1) : \
  (value) > 0 ? ((uintmax_t) (value) > ((uintmax_t)1 << DIGIT_BITS) ?  2 :  1) : 0), \
 { DEE_PRIVATE_ABS(value)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_UINT_2DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[2]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) > ((uintmax_t)1 << DIGIT_BITS) ? 2 : (value) ? 1 : 0, \
 { (value)&DIGIT_MASK, ((value) >> DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_INT_3DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[3]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 ((value) < 0 ? ((uintmax_t)-(value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? -3 : \
                 (uintmax_t)-(value) > ((uintmax_t)1 <<   DIGIT_BITS) ? -2 : -1) : \
  (value) > 0 ? ((uintmax_t) (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ?  3 : \
                 (uintmax_t) (value) > ((uintmax_t)1 << DIGIT_BITS) ? 2 :  1) : 0), \
 { DEE_PRIVATE_ABS(value)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 2*DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_INT_4DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[4]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) < 0 ? ((uintmax_t)-(value) > ((uintmax_t)1 << 3*DIGIT_BITS) ? -4 : \
                (uintmax_t)-(value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? -3 : \
                (uintmax_t)-(value) > ((uintmax_t)1 <<   DIGIT_BITS) ? -2 : -1) : \
 (value) > 0 ? ((uintmax_t) (value) > ((uintmax_t)1 << 3*DIGIT_BITS) ?  4 : \
                (uintmax_t) (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ?  3 : \
                (uintmax_t) (value) > ((uintmax_t)1 <<   DIGIT_BITS) ?  2 : 1) : 0, \
 { DEE_PRIVATE_ABS(value)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 2*DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 3*DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_INT_5DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[5]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) < 0 ? ((uintmax_t)-(value) > ((uintmax_t)1 << 4*DIGIT_BITS) ? -5 : \
                (uintmax_t)-(value) > ((uintmax_t)1 << 3*DIGIT_BITS) ? -4 : \
                (uintmax_t)-(value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? -3 : \
                (uintmax_t)-(value) > ((uintmax_t)1 <<   DIGIT_BITS) ? -2 : -1) : \
 (value) > 0 ? ((uintmax_t) (value) > ((uintmax_t)1 << 4*DIGIT_BITS) ?  5 : \
                (uintmax_t) (value) > ((uintmax_t)1 << 3*DIGIT_BITS) ?  4 : \
                (uintmax_t) (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ?  3 : \
                (uintmax_t) (value) > ((uintmax_t)1 <<   DIGIT_BITS) ?  2 : 1) : 0, \
 { DEE_PRIVATE_ABS(value)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 2*DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 3*DIGIT_BITS)&DIGIT_MASK, \
  (DEE_PRIVATE_ABS(value) >> 4*DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_UINT_3DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[3]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? 3 : \
 (value) > ((uintmax_t)1 << DIGIT_BITS) ? 2 : (value) ? 1 : 0, \
 { (value)&DIGIT_MASK, \
  ((value) >> DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 2*DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_UINT_4DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[4]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) > ((uintmax_t)1 << 3*DIGIT_BITS) ? 4 : \
 (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? 3 : \
 (value) > ((uintmax_t)1 << DIGIT_BITS) ? 2 : \
 (value) ? 1 : 0, \
 { (value)&DIGIT_MASK, \
  ((value) >> DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 2*DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 3*DIGIT_BITS)&DIGIT_MASK } \
}
#define DEFINE_UINT_5DIGITS(name,value) \
struct { \
    OBJECT_HEAD \
    dssize_t _size; \
    digit    _digits[5]; \
} name = { \
 OBJECT_HEAD_INIT(&DeeInt_Type), \
 (value) > ((uintmax_t)1 << 4*DIGIT_BITS) ? 5 : \
 (value) > ((uintmax_t)1 << 3*DIGIT_BITS) ? 4 : \
 (value) > ((uintmax_t)1 << 2*DIGIT_BITS) ? 3 : \
 (value) > ((uintmax_t)1 << DIGIT_BITS) ? 2 : \
 (value) ? 1 : 0, \
 { (value)&DIGIT_MASK, \
  ((value) >> DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 2*DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 3*DIGIT_BITS)&DIGIT_MASK, \
  ((value) >> 4*DIGIT_BITS)&DIGIT_MASK } \
}

#if DIGIT_BITS == 30
#define DEFINE_INT15(name,value)  DEFINE_INT_1DIGIT(name,value)
#define DEFINE_INT16(name,value)  DEFINE_INT_1DIGIT(name,value)
#define DEFINE_INT30(name,value)  DEFINE_INT_1DIGIT(name,value)
#define DEFINE_INT32(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT45(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT48(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT60(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT64(name,value)  DEFINE_INT_3DIGITS(name,value)
#define DEFINE_INT75(name,value)  DEFINE_INT_3DIGITS(name,value)
#define DEFINE_INT90(name,value)  DEFINE_INT_3DIGITS(name,value)
#define DEFINE_UINT15(name,value) DEFINE_UINT_1DIGIT(name,value)
#define DEFINE_UINT16(name,value) DEFINE_UINT_1DIGIT(name,value)
#define DEFINE_UINT30(name,value) DEFINE_UINT_1DIGIT(name,value)
#define DEFINE_UINT32(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT45(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT48(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT60(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT64(name,value) DEFINE_UINT_3DIGITS(name,value)
#define DEFINE_UINT75(name,value) DEFINE_UINT_3DIGITS(name,value)
#define DEFINE_UINT90(name,value) DEFINE_UINT_3DIGITS(name,value)
#else
#define DEFINE_INT15(name,value)  DEFINE_INT_1DIGIT(name,value)
#define DEFINE_INT16(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT30(name,value)  DEFINE_INT_2DIGITS(name,value)
#define DEFINE_INT32(name,value)  DEFINE_INT_3DIGITS(name,value)
#define DEFINE_INT45(name,value)  DEFINE_INT_3DIGITS(name,value)
#define DEFINE_INT48(name,value)  DEFINE_INT_4DIGITS(name,value)
#define DEFINE_INT60(name,value)  DEFINE_INT_4DIGITS(name,value)
#define DEFINE_INT64(name,value)  DEFINE_INT_5DIGITS(name,value)
#define DEFINE_INT75(name,value)  DEFINE_INT_5DIGITS(name,value)
#define DEFINE_UINT15(name,value) DEFINE_UINT_1DIGIT(name,value)
#define DEFINE_UINT16(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT30(name,value) DEFINE_UINT_2DIGITS(name,value)
#define DEFINE_UINT32(name,value) DEFINE_UINT_3DIGITS(name,value)
#define DEFINE_UINT45(name,value) DEFINE_UINT_3DIGITS(name,value)
#define DEFINE_UINT48(name,value) DEFINE_UINT_4DIGITS(name,value)
#define DEFINE_UINT60(name,value) DEFINE_UINT_4DIGITS(name,value)
#define DEFINE_UINT64(name,value) DEFINE_UINT_5DIGITS(name,value)
#define DEFINE_UINT75(name,value) DEFINE_UINT_5DIGITS(name,value)
#endif


DDATDEF DeeTypeObject DeeInt_Type;

/* Builtin constant for special (oftenly used) values. */
#ifdef GUARD_DEEMON_OBJECTS_INT_C
DDATDEF DeeIntObject  DeeInt_Zero;
DDATDEF DeeIntObject  DeeInt_One;
DDATDEF DeeIntObject  DeeInt_MinusOne;
#else
DDATDEF DeeObject     DeeInt_Zero;
DDATDEF DeeObject     DeeInt_One;
DDATDEF DeeObject     DeeInt_MinusOne;
#endif

#define DeeInt_Check(x)      DeeObject_InstanceOfExact(x,&DeeInt_Type) /* `int' is `final' */
#define DeeInt_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeInt_Type)




/* Integer object creation. */
DFUNDEF DREF DeeObject *DCALL DeeInt_NewS16(int16_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewS32(int32_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewS64(int64_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewU16(uint16_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewU32(uint32_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewU64(uint64_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewS128(dint128_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewU128(duint128_t val);
#if DIGIT_BITS < 16
DFUNDEF DREF DeeObject *DCALL DeeInt_NewS8(int8_t val);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewU8(uint8_t val);
#else
#define DeeInt_NewU8  DeeInt_NewU16
#define DeeInt_NewS8  DeeInt_NewS16
#endif

#define DeeInt_NewAutoFit(v) \
 ((v) < 0 ? ((v) >= INT8_MIN ? DeeInt_NewS8((int8_t)(v)) : \
             (v) >= INT16_MIN ? DeeInt_NewS16((int16_t)(v)) : \
             (v) >= INT32_MIN ? DeeInt_NewS32((int32_t)(v)) : \
                                DeeInt_NewS64((int32_t)(v))) \
          : ((v) <= INT8_MIN ? DeeInt_NewU8((uint8_t)(v)) : \
             (v) <= INT16_MIN ? DeeInt_NewU16((uint16_t)(v)) : \
             (v) <= INT32_MIN ? DeeInt_NewU32((uint32_t)(v)) : \
                                DeeInt_NewU64((uint32_t)(v))))


/* Create an integer from signed/unsigned LEB data. */
DFUNDEF DREF DeeObject *DCALL DeeInt_NewSleb(uint8_t **__restrict preader);
DFUNDEF DREF DeeObject *DCALL DeeInt_NewUleb(uint8_t **__restrict preader);

/* Write the value of an integer as signed/unsigned LEB data.
 * NOTE: When writing ULEB data, the caller is responsible to ensure that `self' is positive. */
DFUNDEF uint8_t *DCALL DeeInt_GetSleb(DeeObject *__restrict self, uint8_t *__restrict writer);
DFUNDEF uint8_t *DCALL DeeInt_GetUleb(DeeObject *__restrict self, uint8_t *__restrict writer);

/* Calculate the worst-case required memory for writing a given integer in LEB format. */
#define DEEINT_SLEB_MAXSIZE(self) (((DeeIntObject *)REQUIRES_OBJECT(self))->ob_size < 0 ? \
                                  (((-((DeeIntObject *)REQUIRES_OBJECT(self))->ob_size + 1)*DIGIT_BITS)/7) : \
                                  ((( ((DeeIntObject *)REQUIRES_OBJECT(self))->ob_size + 1)*DIGIT_BITS)/7))
#define DEEINT_ULEB_MAXSIZE(self) ((((size_t)((DeeIntObject *)REQUIRES_OBJECT(self))->ob_size + 1)*DIGIT_BITS)/7)

#define DeeInt_IsNeg(self) (((DeeIntObject *)REQUIRES_OBJECT(self))->ob_size < 0)


#define INT_SIGNED         0  /* The returned integer value is signed. */
#define INT_UNSIGNED       1  /* The returned integer value is unsigned. */
#define INT_POS_OVERFLOW   2  /* ERROR: The returned integer value overflows into the positive. */
#define INT_NEG_OVERFLOW (-2) /* ERROR: The returned integer value overflows into the negative. */

/* Extract the 32-, 64- or 128-bit value of the given integer.
 * NOTE: In theory, deemon integers can have arbitrarily large
 *       values, however in deemon's C api, we must limit ourself
 *       to only a set number of bits.
 * @return: One of `INT_*' (See above) */
DFUNDEF int DCALL DeeInt_TryAs32(DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF int DCALL DeeInt_TryAs64(DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF int DCALL DeeInt_TryAs128(DeeObject *__restrict self, dint128_t *__restrict value);

/* Similar to the functions above, but explicitly require signed/unsigned 32/64-bit values. */
DFUNDEF bool DCALL DeeInt_TryAsS32(DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF bool DCALL DeeInt_TryAsS64(DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF bool DCALL DeeInt_TryAsS128(DeeObject *__restrict self, dint128_t *__restrict value);
DFUNDEF bool DCALL DeeInt_TryAsU32(DeeObject *__restrict self, uint32_t *__restrict value);
DFUNDEF bool DCALL DeeInt_TryAsU64(DeeObject *__restrict self, uint64_t *__restrict value);
DFUNDEF bool DCALL DeeInt_TryAsU128(DeeObject *__restrict self, duint128_t *__restrict value);

/* Same as the functions above, but raise an `Error.ValueError.Arithmetic.IntegerOverflow'
 * for `INT_POS_OVERFLOW' and `INT_NEG_OVERFLOW' and returns -1. */
DFUNDEF int DCALL DeeInt_As32(DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF int DCALL DeeInt_As64(DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF int DCALL DeeInt_As128(DeeObject *__restrict self, dint128_t *__restrict value);

/* Read the signed/unsigned values from the given integer.
 * @return: 0:  Successfully read the value.
 * @return: -1: An error occurred (Integer overflow). */
DFUNDEF int (DCALL DeeInt_AsS32)(DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF int (DCALL DeeInt_AsS64)(DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF int (DCALL DeeInt_AsS128)(DeeObject *__restrict self, dint128_t *__restrict value);
DFUNDEF int (DCALL DeeInt_AsU32)(DeeObject *__restrict self, uint32_t *__restrict value);
DFUNDEF int (DCALL DeeInt_AsU64)(DeeObject *__restrict self, uint64_t *__restrict value);
DFUNDEF int (DCALL DeeInt_AsU128)(DeeObject *__restrict self, duint128_t *__restrict value);


/* Convert an integer to a binary-encoded data array. */
DFUNDEF int
(DCALL DeeInt_AsBytes)(DeeObject *__restrict self,
                       void *__restrict dst, size_t length,
                       bool little_endian, bool as_signed);
/* Convert a binary-encoded data array into an integer. */
DFUNDEF DREF DeeObject *
(DCALL DeeInt_FromBytes)(void const *__restrict buf, size_t length,
                         bool little_endian, bool as_signed);

#define DEE_PRIVATE_TRYGETSINT_4 DeeInt_TryAsS32
#define DEE_PRIVATE_TRYGETSINT_8 DeeInt_TryAsS64
#define DEE_PRIVATE_TRYGETUINT_4 DeeInt_TryAsU32
#define DEE_PRIVATE_TRYGETUINT_8 DeeInt_TryAsU64
#define DEE_PRIVATE_TRYGETSINT(size) DEE_PRIVATE_TRYGETSINT_##size
#define DEE_PRIVATE_TRYGETUINT(size) DEE_PRIVATE_TRYGETUINT_##size
#define DeeInt_TryAsS(size,self,val) DEE_PRIVATE_TRYGETSINT(size)(self,val)
#define DeeInt_TryAsU(size,self,val) DEE_PRIVATE_TRYGETUINT(size)(self,val)

#define DeeInt_TryAsSSize(self,val)    DeeInt_TryAsS(__SIZEOF_SIZE_T__,self,val)
#define DeeInt_TryAsSize(self,val)     DeeInt_TryAsU(__SIZEOF_SIZE_T__,self,val)
#define DeeInt_TryAsIntptr(self,val)   DeeInt_TryAsS(__SIZEOF_POINTER__,self,val)
#define DeeInt_TryAsUIntptr(self,val)  DeeInt_TryAsU(__SIZEOF_POINTER__,self,val)



/* Convert an integer to/from a string.
 * WARNING: The caller is responsible not to pass a radix equal to `1'.
 *          When a radix equal to `0', it is automatically determined from the passed string. */
DFUNDEF DREF DeeObject *DCALL
DeeInt_FromString(/*utf-8*/char const *__restrict str,
                  size_t len, uint32_t radix_and_flags);
DFUNDEF DREF DeeObject *DCALL
DeeInt_FromAscii(/*ascii*/char const *__restrict str,
                 size_t len, uint32_t radix_and_flags);
#define DEEINT_STRING(radix,flags) ((radix) << DEEINT_STRING_RSHIFT | (flags))
#define DEEINT_STRING_RSHIFT   16
#define DEEINT_STRING_FNORMAL  0x0000
#define DEEINT_STRING_FESCAPED 0x0001 /* Decode escaped linefeeds in the given input string. */
#define DEEINT_STRING_FTRY     0x0002 /* Don't throw a ValueError, but return ITER_DONE. */

/* @return:  0: Successfully parsed an integer.
 * @return: -1: An error occurred. (never returned when `DEEINT_STRING_FTRY' is set)
 * @return:  1: Failed to parse an integer. (returned when `DEEINT_STRING_FTRY' is set) */
DFUNDEF int (DCALL Dee_Atoi8)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int8_t *__restrict value);
DFUNDEF int (DCALL Dee_Atoi16)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int16_t *__restrict value);
DFUNDEF int (DCALL Dee_Atoi32)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int32_t *__restrict value);
DFUNDEF int (DCALL Dee_Atoi64)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int64_t *__restrict value);
#define DEEATOI_STRING_FSIGNED 0x0004 /* The generated value is signed. */

#ifdef __INTELLISENSE__
DFUNDEF int (DCALL Dee_Atos8)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int8_t *__restrict value);
DFUNDEF int (DCALL Dee_Atos16)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int16_t *__restrict value);
DFUNDEF int (DCALL Dee_Atos32)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int32_t *__restrict value);
DFUNDEF int (DCALL Dee_Atos64)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, int64_t *__restrict value);
DFUNDEF int (DCALL Dee_Atou8)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, uint8_t *__restrict value);
DFUNDEF int (DCALL Dee_Atou16)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, uint16_t *__restrict value);
DFUNDEF int (DCALL Dee_Atou32)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, uint32_t *__restrict value);
DFUNDEF int (DCALL Dee_Atou64)(/*utf-8*/char const *__restrict str, size_t len, uint32_t radix_and_flags, uint64_t *__restrict value);
#else
#define Dee_Atos8(str,len,radix_and_flags,value)   Dee_Atoi8(str,len,(radix_and_flags)|DEEATOI_STRING_FSIGNED,value)
#define Dee_Atos16(str,len,radix_and_flags,value)  Dee_Atoi16(str,len,(radix_and_flags)|DEEATOI_STRING_FSIGNED,value)
#define Dee_Atos32(str,len,radix_and_flags,value)  Dee_Atoi32(str,len,(radix_and_flags)|DEEATOI_STRING_FSIGNED,value)
#define Dee_Atos64(str,len,radix_and_flags,value)  Dee_Atoi64(str,len,(radix_and_flags)|DEEATOI_STRING_FSIGNED,value)
#define Dee_Atou8(str,len,radix_and_flags,value)   Dee_Atoi8(str,len,radix_and_flags,(int8_t *)(value))
#define Dee_Atou16(str,len,radix_and_flags,value)  Dee_Atoi16(str,len,radix_and_flags,(int16_t *)(value))
#define Dee_Atou32(str,len,radix_and_flags,value)  Dee_Atoi32(str,len,radix_and_flags,(int32_t *)(value))
#define Dee_Atou64(str,len,radix_and_flags,value)  Dee_Atoi64(str,len,radix_and_flags,(int64_t *)(value))
#endif

#ifdef __NO_builtin_choose_expr
#define DEE_PRIVATE_ATOI_FLAGS(T,flags) \
        (((T)-1) < (T)0 ? (flags)|DEEATOI_STRING_FSIGNED : (flags))
#define Dee_TAtoi(T,str,len,radix_and_flags,value) \
        (sizeof(T) <= 1 ? Dee_Atoi8(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int8_t *)(value)) : \
         sizeof(T) <= 2 ? Dee_Atoi16(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int16_t *)(value)) : \
         sizeof(T) <= 4 ? Dee_Atoi32(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int32_t *)(value)) : \
                          Dee_Atoi64(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int64_t *)(value)))
#else /* __NO_builtin_choose_expr */
#define DEE_PRIVATE_ATOI_FLAGS(T,flags) \
        __builtin_choose_expr(((T)-1) < (T)0,(flags)|DEEATOI_STRING_FSIGNED,(flags))
#define Dee_TAtoi(T,str,len,radix_and_flags,value) \
        __builtin_choose_expr(sizeof(T) <= 1,Dee_Atoi8(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int8_t *)(value)), \
        __builtin_choose_expr(sizeof(T) <= 2,Dee_Atoi16(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int16_t *)(value)), \
        __builtin_choose_expr(sizeof(T) <= 4,Dee_Atoi32(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int32_t *)(value)), \
                                             Dee_Atoi64(str,len,DEE_PRIVATE_ATOI_FLAGS(T,radix_and_flags),(int64_t *)(value)))))
#endif /* !__NO_builtin_choose_expr */


/* Print an integer to a given format-printer.
 * Radix must be one of `2', `4', `8', `10' or `16' and
 * if it isn't, a `NotImplemented' error is thrown.
 * This list of supported radices may be extended in the future. */
DFUNDEF dssize_t DCALL
DeeInt_Print(DREF DeeObject *self, uint32_t radix_and_flags,
             dformatprinter printer, void *arg);
#define DEEINT_PRINT(radix,flags) ((radix) << DEEINT_PRINT_RSHIFT | (flags))
#define DEEINT_PRINT_RSHIFT  16
#define DEEINT_PRINT_FNORMAL 0x0000
#define DEEINT_PRINT_FUPPER  0x0001 /* Use uppercase characters for printing digits above `9' */
#define DEEINT_PRINT_FNUMSYS 0x0002 /* Prepend the number system prefix before the integer itself (e.g.: `0x').
                                     * NOTE: If the radix cannot be represented as a prefix, this flag is ignored. */
#define DEEINT_PRINT_FSIGN   0x0004 /* Always prepend a sign, even before for positive numbers. */

#define DEEINT_PRINT_BIN     DEEINT_PRINT(2,DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_OCT     DEEINT_PRINT(8,DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_DEC     DEEINT_PRINT(10,DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_HEX     DEEINT_PRINT(16,DEEINT_PRINT_FNORMAL)


#define DEE_PRIVATE_NEWINT_1   DeeInt_NewS8
#define DEE_PRIVATE_NEWINT_2   DeeInt_NewS16
#define DEE_PRIVATE_NEWINT_4   DeeInt_NewS32
#define DEE_PRIVATE_NEWINT_8   DeeInt_NewS64
#define DEE_PRIVATE_NEWUINT_1  DeeInt_NewU8
#define DEE_PRIVATE_NEWUINT_2  DeeInt_NewU16
#define DEE_PRIVATE_NEWUINT_4  DeeInt_NewU32
#define DEE_PRIVATE_NEWUINT_8  DeeInt_NewU64
#define DEE_PRIVATE_NEWINT(size)  DEE_PRIVATE_NEWINT_##size
#define DEE_PRIVATE_NEWUINT(size) DEE_PRIVATE_NEWUINT_##size

/* Create a new integer object with an input integral value `val' of `size' bytes. */
#define DeeInt_New(size,val)  DEE_PRIVATE_NEWINT(size)(val)
#define DeeInt_Newu(size,val) DEE_PRIVATE_NEWUINT(size)(val)

#define DeeInt_NewChar(val)    DeeInt_New(__SIZEOF_CHAR__,val)
#define DeeInt_NewUChar(val)   DeeInt_Newu(__SIZEOF_CHAR__,val)
#define DeeInt_NewShort(val)   DeeInt_New(__SIZEOF_SHORT__,val)
#define DeeInt_NewUShort(val)  DeeInt_Newu(__SIZEOF_SHORT__,val)
#define DeeInt_NewInt(val)     DeeInt_New(__SIZEOF_INT__,val)
#define DeeInt_NewUInt(val)    DeeInt_Newu(__SIZEOF_INT__,val)
#define DeeInt_NewLong(val)    DeeInt_New(__SIZEOF_LONG__,val)
#define DeeInt_NewULong(val)   DeeInt_Newu(__SIZEOF_LONG__,val)
#ifdef __COMPILER_HAVE_LONGLONG
#define DeeInt_NewLLong(val)   DeeInt_New(__SIZEOF_LONG_LONG__,val)
#define DeeInt_NewULLong(val)  DeeInt_Newu(__SIZEOF_LONG_LONG__,val)
#endif /* __COMPILER_HAVE_LONGLONG */
#define DeeInt_NewSize(val)    DeeInt_Newu(__SIZEOF_SIZE_T__,val)
#define DeeInt_NewHash(val)    DeeInt_Newu(__SIZEOF_POINTER__,val)
#define DeeInt_NewSSize(val)   DeeInt_New(__SIZEOF_SIZE_T__,val)
#define DeeInt_NewPtrdiff(val) DeeInt_New(__SIZEOF_PTRDIFF_T__,val)
#define DeeInt_NewIntptr(val)  DeeInt_New(__SIZEOF_POINTER__,val)
#define DeeInt_NewUIntptr(val) DeeInt_Newu(__SIZEOF_POINTER__,val)

#if DIGIT_BITS == 30
#define DeeInt_NewDigit(val)       DeeInt_Newu(4,val)
#define DeeInt_NewSDigit(val)      DeeInt_New(4,val)
#define DeeInt_NewTwoDigits(val)   DeeInt_Newu(8,val)
#define DeeInt_NewSTwoDigits(val)  DeeInt_New(8,val)
#else
#define DeeInt_NewDigit(val)       DeeInt_Newu(2,val)
#define DeeInt_NewSDigit(val)      DeeInt_New(2,val)
#define DeeInt_NewTwoDigits(val)   DeeInt_Newu(4,val)
#define DeeInt_NewSTwoDigits(val)  DeeInt_New(4,val)
#endif

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeInt_AsS32(self,value)                   __builtin_expect(DeeInt_AsS32(self,value),0)
#define DeeInt_AsS64(self,value)                   __builtin_expect(DeeInt_AsS64(self,value),0)
#define DeeInt_AsU32(self,value)                   __builtin_expect(DeeInt_AsU32(self,value),0)
#define DeeInt_AsU64(self,value)                   __builtin_expect(DeeInt_AsU64(self,value),0)
#define DeeInt_TryAsS32(self,value)                __builtin_expect(DeeInt_TryAsS32(self,value),true)
#define DeeInt_TryAsS64(self,value)                __builtin_expect(DeeInt_TryAsS64(self,value),true)
#define DeeInt_TryAsU32(self,value)                __builtin_expect(DeeInt_TryAsU32(self,value),true)
#define DeeInt_TryAsU64(self,value)                __builtin_expect(DeeInt_TryAsU64(self,value),true)
#define Dee_Atoi8(str,len,radix_and_flags,value)   __builtin_expect(Dee_Atoi8(str,len,radix_and_flags,value),0)
#define Dee_Atoi16(str,len,radix_and_flags,value)  __builtin_expect(Dee_Atoi16(str,len,radix_and_flags,value),0)
#define Dee_Atoi32(str,len,radix_and_flags,value)  __builtin_expect(Dee_Atoi32(str,len,radix_and_flags,value),0)
#define Dee_Atoi64(str,len,radix_and_flags,value)  __builtin_expect(Dee_Atoi64(str,len,radix_and_flags,value),0)
#endif /* !__NO_builtin_expect */
#endif

DECL_END

#endif /* !GUARD_DEEMON_INT_H */
