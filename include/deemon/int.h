/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
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

#include <hybrid/byteorder.h>
#include <hybrid/int128.h>
#include <hybrid/limitcore.h>
#include <hybrid/typecore.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "format.h"
#include "object.h"

/* NOTE: Integer objects have been completely reworked in deemon 2.0,
 *       no longer being split between 8 different types for different
 *       sizes, as well as becoming immutable objects and disallowing
 *       inplace operators on out-of-scope variables:
 * >> local my_var = 42;
 * >> function foo() {
 * >>     ++my_var; // Illegal since __DEEMON__ >= 200
 * >>               // Causes a compiler error.
 * >> }
 * >> print my_var; // 42
 * >> foo();
 * >> print my_var; // 43 (Before 200)
 *
 * Code like this can be migrated as follows:
 * Option 1:
 * >> global my_var = 42; // Change to a global variable
 * >> function foo() {
 * >>     ++my_var;
 * >> }
 * >> print my_var; // 42
 * >> foo();
 * >> print my_var; // 43
 *
 * Option 2:
 * >> import Cell from deemon;
 * >> local my_var = Cell(42); // Change to a cell
 * >> function foo() {
 * >>     ++my_var.value;
 * >> }
 * >> print my_var.value; // 42
 * >> foo();
 * >> print my_var.value; // 43
 */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_int_object int_object
#endif /* DEE_SOURCE */

typedef struct Dee_int_object DeeIntObject;

#if __SIZEOF_POINTER__ >= 8
#define Dee_DIGIT_BITS 30
#else /* __SIZEOF_POINTER__ >= 8 */
#define Dee_DIGIT_BITS 15
#endif /* __SIZEOF_POINTER__ < 8 */


#if Dee_DIGIT_BITS <= 16
#define Dee_SIZEOF_DIGIT     2
#define Dee_SIZEOF_TWODIGITS 4
typedef uint16_t Dee_digit_t;
typedef int16_t Dee_sdigit_t;
typedef uint32_t Dee_twodigits_t;
typedef int32_t Dee_stwodigits_t;
#else /* Dee_DIGIT_BITS <= 16 */
#define Dee_SIZEOF_DIGIT     4
#define Dee_SIZEOF_TWODIGITS 8
typedef uint32_t Dee_digit_t;
typedef int32_t Dee_sdigit_t;
typedef uint64_t Dee_twodigits_t;
typedef int64_t Dee_stwodigits_t;
#endif /* Dee_DIGIT_BITS > 16 */
#define Dee_DIGIT_BASE ((Dee_digit_t)1 << Dee_DIGIT_BITS)
#define Dee_DIGIT_MASK ((Dee_digit_t)(Dee_DIGIT_BASE - 1))


#ifdef DEE_SOURCE
#define DIGIT_BITS Dee_DIGIT_BITS
typedef Dee_digit_t digit;
typedef Dee_sdigit_t sdigit;
typedef Dee_twodigits_t twodigits;
typedef Dee_stwodigits_t stwodigits;
#define DIGIT_BASE Dee_DIGIT_BASE
#define DIGIT_MASK Dee_DIGIT_MASK
#endif /* DEE_SOURCE */

struct Dee_int_object {
	Dee_OBJECT_HEAD
	Dee_ssize_t                          ob_size;   /* Number of used digits (negative of that number for negative integers) */
	COMPILER_FLEXIBLE_ARRAY(Dee_digit_t, ob_digit); /* Bit-vector of the integer, split in digits of `Dee_DIGIT_BITS' bits each.
	                                                 * Least significant bits come first with individual digits being encoded
	                                                 * in host-endian.
	                                                 * The total number of digits is the absolute value of `ob_size',
	                                                 * which is negative if the value of the integer is too. */
};

#define DEE_PRIVATE_ABS(value)              \
	((value) < 0                            \
	 ? ((intmax_t)(value) == __INTMAX_MIN__ \
	    ? __UINTMAX_MAX__                   \
	    : (uintmax_t)(-(intmax_t)(value)))  \
	 : (uintmax_t)(value))
#define Dee_DEFINE_INT_1DIGIT(name, value)          \
	struct {                                        \
		Dee_OBJECT_HEAD                             \
		Dee_ssize_t _size;                          \
		Dee_digit_t _digits[1];                     \
	} name = {                                      \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),         \
		(value) < 0                                 \
		? -1                                        \
		: (value) > 0                               \
		  ? 1                                       \
		  : 0,                                      \
		{ DEE_PRIVATE_ABS(value) & Dee_DIGIT_MASK } \
	}
#define Dee_DEFINE_UINT_1DIGIT(name, value) \
	struct {                                \
		Dee_OBJECT_HEAD                     \
		Dee_ssize_t _size;                  \
		Dee_digit_t _digits[1];             \
	} name = {                              \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type), \
		(value) ? 1 : 0,                    \
		{ (value)&Dee_DIGIT_MASK }          \
	}
#define Dee_DEFINE_INT_2DIGITS(name, value)                                      \
	struct {                                                                     \
		Dee_OBJECT_HEAD                                                          \
		Dee_ssize_t _size;                                                       \
		Dee_digit_t _digits[2];                                                  \
	} name = {                                                                   \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                                      \
		((value) < 0                                                             \
		 ? ((intmax_t)(value) == __INTMAX_MIN__                                  \
		    ? -2                                                                 \
		    : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << Dee_DIGIT_BITS) \
		      ? -2                                                               \
		      : -1)                                                              \
		 : (value) > 0                                                           \
		   ? ((uintmax_t)(value) > ((uintmax_t)1 << Dee_DIGIT_BITS)              \
		      ? 2                                                                \
		      : 1)                                                               \
		   : 0),                                                                 \
		{ DEE_PRIVATE_ABS(value) & Dee_DIGIT_MASK,                               \
		  (DEE_PRIVATE_ABS(value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK }          \
	}
#define Dee_DEFINE_UINT_2DIGITS(name, value)                                     \
	struct {                                                                     \
		Dee_OBJECT_HEAD                                                          \
		Dee_ssize_t _size;                                                       \
		Dee_digit_t _digits[2];                                                  \
	} name = {                                                                   \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                                      \
		(value) > ((uintmax_t)1 << Dee_DIGIT_BITS)                               \
		? 2                                                                      \
		: (value)                                                                \
		  ? 1                                                                    \
		  : 0,                                                                   \
		{ (value)&Dee_DIGIT_MASK, ((value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK } \
	}
#define Dee_DEFINE_INT_3DIGITS(name, value)                                          \
	struct {                                                                         \
		Dee_OBJECT_HEAD                                                              \
		Dee_ssize_t _size;                                                           \
		Dee_digit_t _digits[3];                                                      \
	} name = {                                                                       \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                                          \
		((value) < 0                                                                 \
		 ? ((intmax_t)(value) == __INTMAX_MIN__                                      \
		    ? -3                                                                     \
		    : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS) \
		      ? -3                                                                   \
		      : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << Dee_DIGIT_BITS)   \
		        ? -2                                                                 \
		        : -1)                                                                \
		 : (value) > 0                                                               \
		   ? ((uintmax_t)(value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)              \
		      ? 3                                                                    \
		      : (uintmax_t)(value) > ((uintmax_t)1 << Dee_DIGIT_BITS)                \
		        ? 2                                                                  \
		        : 1)                                                                 \
		   : 0),                                                                     \
		{ DEE_PRIVATE_ABS(value) & Dee_DIGIT_MASK,                                   \
		  (DEE_PRIVATE_ABS(value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,               \
		  (DEE_PRIVATE_ABS(value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK }          \
	}
#define Dee_DEFINE_INT_4DIGITS(name, value)                                           \
	struct {                                                                          \
		Dee_OBJECT_HEAD                                                               \
		Dee_ssize_t _size;                                                            \
		Dee_digit_t _digits[4];                                                       \
	} name = {                                                                        \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                                           \
		(value) < 0                                                                   \
		? ((intmax_t)(value) == __INTMAX_MIN__                                        \
		   ? -4                                                                       \
		   : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)   \
		     ? -4                                                                     \
		     : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS) \
		       ? -3                                                                   \
		       : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << Dee_DIGIT_BITS)   \
		         ? -2                                                                 \
		         : -1)                                                                \
		: (value) > 0                                                                 \
		  ? ((uintmax_t)(value) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)                \
		     ? 4                                                                      \
		     : (uintmax_t)(value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)              \
		       ? 3                                                                    \
		       : (uintmax_t)(value) > ((uintmax_t)1 << Dee_DIGIT_BITS)                \
		         ? 2                                                                  \
		         : 1)                                                                 \
		  : 0,                                                                        \
		{ DEE_PRIVATE_ABS(value) & Dee_DIGIT_MASK,                                    \
		  (DEE_PRIVATE_ABS(value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,                \
		  (DEE_PRIVATE_ABS(value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,            \
		  (DEE_PRIVATE_ABS(value) >> 3 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK }           \
	}
#define Dee_DEFINE_INT_5DIGITS(name, value)                                             \
	struct {                                                                            \
		Dee_OBJECT_HEAD                                                                 \
		Dee_ssize_t _size;                                                              \
		Dee_digit_t _digits[5];                                                         \
	} name = {                                                                          \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                                             \
		(value) < 0                                                                     \
		? ((intmax_t)(value) == __INTMAX_MIN__                                          \
		   ? -5                                                                         \
		   : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 4 * Dee_DIGIT_BITS)     \
		     ? -5                                                                       \
		     : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)   \
		       ? -4                                                                     \
		       : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS) \
		         ? -3                                                                   \
		         : (uintmax_t)(-(intmax_t)(value)) > ((uintmax_t)1 << Dee_DIGIT_BITS)   \
		           ? -2                                                                 \
		           : -1)                                                                \
		: (value) > 0                                                                   \
		  ? ((uintmax_t)(value) > ((uintmax_t)1 << 4 * Dee_DIGIT_BITS)                  \
		     ? 5                                                                        \
		     : (uintmax_t)(value) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)                \
		       ? 4                                                                      \
		       : (uintmax_t)(value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)              \
		         ? 3                                                                    \
		         : (uintmax_t)(value) > ((uintmax_t)1 << Dee_DIGIT_BITS)                \
		           ? 2                                                                  \
		           : 1)                                                                 \
		  : 0,                                                                          \
		{ DEE_PRIVATE_ABS(value) & Dee_DIGIT_MASK,                                      \
		  (DEE_PRIVATE_ABS(value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,                  \
		  (DEE_PRIVATE_ABS(value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,              \
		  (DEE_PRIVATE_ABS(value) >> 3 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,              \
		  (DEE_PRIVATE_ABS(value) >> 4 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK }             \
	}
#define Dee_DEFINE_UINT_3DIGITS(name, value)                 \
	struct {                                                 \
		Dee_OBJECT_HEAD                                      \
		Dee_ssize_t _size;                                   \
		Dee_digit_t _digits[3];                              \
	} name = {                                               \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                  \
		(value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)       \
		? 3                                                  \
		: (value) > ((uintmax_t)1 << Dee_DIGIT_BITS)         \
		  ? 2                                                \
		  : (value)                                          \
		    ? 1                                              \
		    : 0,                                             \
		{ (value)&Dee_DIGIT_MASK,                            \
		  ((value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,      \
		  ((value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK } \
	}
#define Dee_DEFINE_UINT_4DIGITS(name, value)                 \
	struct {                                                 \
		Dee_OBJECT_HEAD                                      \
		Dee_ssize_t _size;                                   \
		Dee_digit_t _digits[4];                              \
	} name = {                                               \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                  \
		(value) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)       \
		? 4                                                  \
		: (value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)     \
		  ? 3                                                \
		  : (value) > ((uintmax_t)1 << Dee_DIGIT_BITS)       \
		    ? 2                                              \
		    : (value)                                        \
		      ? 1                                            \
		      : 0,                                           \
		{ (value)&Dee_DIGIT_MASK,                            \
		  ((value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,      \
		  ((value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,  \
		  ((value) >> 3 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK } \
	}
#define Dee_DEFINE_UINT_5DIGITS(name, value)                 \
	struct {                                                 \
		Dee_OBJECT_HEAD                                      \
		Dee_ssize_t _size;                                   \
		Dee_digit_t _digits[5];                              \
	} name = {                                               \
		Dee_OBJECT_HEAD_INIT(&DeeInt_Type),                  \
		(value) > ((uintmax_t)1 << 4 * Dee_DIGIT_BITS)       \
		? 5                                                  \
		: (value) > ((uintmax_t)1 << 3 * Dee_DIGIT_BITS)     \
		  ? 4                                                \
		  : (value) > ((uintmax_t)1 << 2 * Dee_DIGIT_BITS)   \
		    ? 3                                              \
		    : (value) > ((uintmax_t)1 << Dee_DIGIT_BITS)     \
		      ? 2                                            \
		      : (value)                                      \
		        ? 1                                          \
		        : 0,                                         \
		{ (value)&Dee_DIGIT_MASK,                            \
		  ((value) >> Dee_DIGIT_BITS) & Dee_DIGIT_MASK,      \
		  ((value) >> 2 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,  \
		  ((value) >> 3 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK,  \
		  ((value) >> 4 * Dee_DIGIT_BITS) & Dee_DIGIT_MASK } \
	}


#if Dee_DIGIT_BITS == 30
#define Dee_DEFINE_INT15(name, value)  Dee_DEFINE_INT_1DIGIT(name, value)
#define Dee_DEFINE_INT16(name, value)  Dee_DEFINE_INT_1DIGIT(name, value)
#define Dee_DEFINE_INT30(name, value)  Dee_DEFINE_INT_1DIGIT(name, value)
#define Dee_DEFINE_INT32(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT45(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT48(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT60(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT64(name, value)  Dee_DEFINE_INT_3DIGITS(name, value)
#define Dee_DEFINE_INT75(name, value)  Dee_DEFINE_INT_3DIGITS(name, value)
#define Dee_DEFINE_INT90(name, value)  Dee_DEFINE_INT_3DIGITS(name, value)
#define Dee_DEFINE_UINT15(name, value) Dee_DEFINE_UINT_1DIGIT(name, value)
#define Dee_DEFINE_UINT16(name, value) Dee_DEFINE_UINT_1DIGIT(name, value)
#define Dee_DEFINE_UINT30(name, value) Dee_DEFINE_UINT_1DIGIT(name, value)
#define Dee_DEFINE_UINT32(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT45(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT48(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT60(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT64(name, value) Dee_DEFINE_UINT_3DIGITS(name, value)
#define Dee_DEFINE_UINT75(name, value) Dee_DEFINE_UINT_3DIGITS(name, value)
#define Dee_DEFINE_UINT90(name, value) Dee_DEFINE_UINT_3DIGITS(name, value)
#else /* Dee_DIGIT_BITS == 30 */
#define Dee_DEFINE_INT15(name, value)  Dee_DEFINE_INT_1DIGIT(name, value)
#define Dee_DEFINE_INT16(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT30(name, value)  Dee_DEFINE_INT_2DIGITS(name, value)
#define Dee_DEFINE_INT32(name, value)  Dee_DEFINE_INT_3DIGITS(name, value)
#define Dee_DEFINE_INT45(name, value)  Dee_DEFINE_INT_3DIGITS(name, value)
#define Dee_DEFINE_INT48(name, value)  Dee_DEFINE_INT_4DIGITS(name, value)
#define Dee_DEFINE_INT60(name, value)  Dee_DEFINE_INT_4DIGITS(name, value)
#define Dee_DEFINE_INT64(name, value)  Dee_DEFINE_INT_5DIGITS(name, value)
#define Dee_DEFINE_INT75(name, value)  Dee_DEFINE_INT_5DIGITS(name, value)
#define Dee_DEFINE_UINT15(name, value) Dee_DEFINE_UINT_1DIGIT(name, value)
#define Dee_DEFINE_UINT16(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT30(name, value) Dee_DEFINE_UINT_2DIGITS(name, value)
#define Dee_DEFINE_UINT32(name, value) Dee_DEFINE_UINT_3DIGITS(name, value)
#define Dee_DEFINE_UINT45(name, value) Dee_DEFINE_UINT_3DIGITS(name, value)
#define Dee_DEFINE_UINT48(name, value) Dee_DEFINE_UINT_4DIGITS(name, value)
#define Dee_DEFINE_UINT60(name, value) Dee_DEFINE_UINT_4DIGITS(name, value)
#define Dee_DEFINE_UINT64(name, value) Dee_DEFINE_UINT_5DIGITS(name, value)
#define Dee_DEFINE_UINT75(name, value) Dee_DEFINE_UINT_5DIGITS(name, value)
#endif /* Dee_DIGIT_BITS != 30 */

#ifdef DEE_SOURCE
#define DEFINE_INT_1DIGIT   Dee_DEFINE_INT_1DIGIT
#define DEFINE_UINT_1DIGIT  Dee_DEFINE_UINT_1DIGIT
#define DEFINE_INT_2DIGITS  Dee_DEFINE_INT_2DIGITS
#define DEFINE_UINT_2DIGITS Dee_DEFINE_UINT_2DIGITS
#define DEFINE_INT_3DIGITS  Dee_DEFINE_INT_3DIGITS
#define DEFINE_INT_4DIGITS  Dee_DEFINE_INT_4DIGITS
#define DEFINE_INT_5DIGITS  Dee_DEFINE_INT_5DIGITS
#define DEFINE_UINT_3DIGITS Dee_DEFINE_UINT_3DIGITS
#define DEFINE_UINT_4DIGITS Dee_DEFINE_UINT_4DIGITS
#define DEFINE_UINT_5DIGITS Dee_DEFINE_UINT_5DIGITS
#define DEFINE_INT15        Dee_DEFINE_INT15
#define DEFINE_INT16        Dee_DEFINE_INT16
#define DEFINE_INT30        Dee_DEFINE_INT30
#define DEFINE_INT32        Dee_DEFINE_INT32
#define DEFINE_INT45        Dee_DEFINE_INT45
#define DEFINE_INT48        Dee_DEFINE_INT48
#define DEFINE_INT60        Dee_DEFINE_INT60
#define DEFINE_INT64        Dee_DEFINE_INT64
#define DEFINE_INT75        Dee_DEFINE_INT75
#define DEFINE_UINT15       Dee_DEFINE_UINT15
#define DEFINE_UINT16       Dee_DEFINE_UINT16
#define DEFINE_UINT30       Dee_DEFINE_UINT30
#define DEFINE_UINT32       Dee_DEFINE_UINT32
#define DEFINE_UINT45       Dee_DEFINE_UINT45
#define DEFINE_UINT48       Dee_DEFINE_UINT48
#define DEFINE_UINT60       Dee_DEFINE_UINT60
#define DEFINE_UINT64       Dee_DEFINE_UINT64
#define DEFINE_UINT75       Dee_DEFINE_UINT75
#ifdef Dee_DEFINE_INT90
#define DEFINE_INT90        Dee_DEFINE_INT90
#define DEFINE_UINT90       Dee_DEFINE_UINT90
#endif /* Dee_DEFINE_INT90 */
#endif /* DEE_SOURCE */


/* Check if a given object is an `int'-object */
#define DeeInt_Check(x)      DeeObject_InstanceOfExact(x, &DeeInt_Type) /* `int' is final */
#define DeeInt_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeInt_Type)
DDATDEF DeeTypeObject DeeInt_Type;


/* Builtin constant for special (often used) values. */
struct _Dee_int_1digit_object {
	Dee_OBJECT_HEAD
	Dee_ssize_t ob_size;
	Dee_digit_t ob_digit[1];

	/* Pad to whole pointers. */
#if __SIZEOF_POINTER__ == 4 && Dee_DIGIT_BITS == 15
	uint16_t _ob_pad;
#elif __SIZEOF_POINTER__ == 8 && Dee_DIGIT_BITS == 30
	uint32_t _ob_pad;
#elif __SIZEOF_POINTER__ == 8 && Dee_DIGIT_BITS == 15
	uint16_t _ob_pad[3];
#endif /* ... */
};

DDATDEF struct _Dee_int_1digit_object DeeInt_MinusOne_Zero_One[3];
#define DeeInt_MinusOne ((DeeObject *)&DeeInt_MinusOne_Zero_One[0])
#define DeeInt_Zero     ((DeeObject *)&DeeInt_MinusOne_Zero_One[1])
#define DeeInt_One      ((DeeObject *)&DeeInt_MinusOne_Zero_One[2])

/* Return an integer object for the values `-1', `0' and `1' */
#define DeeInt_FromSign(sign)                                                         \
	(Dee_ASSERTF((sign) >= -1 && (sign) <= 1, "Invalid sign value: %d", (int)(sign)), \
	 (DeeObject *)((DeeInt_MinusOne_Zero_One + 1) + (sign)))

/* Return an integer object for small values */
#define DeeInt_ForSmallInt(val) ((DeeObject *)((DeeInt_MinusOne_Zero_One + 1) + (val)))
#define DeeInt_IsSmallInt(val)  ((val) >= -1 && (val) <= 1)




/* Helpers for performing operations in a (theoretical) `DeeInt_NewSSize()' / `DeeInt_NewSize()' object. */
#define DeeInt_SSize_Hash(lhs) ((Dee_hash_t)(size_t)(lhs))
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_SSize_Compare(Dee_ssize_t lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_SSize_CompareEq(Dee_ssize_t lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_SSize_TryCompareEq(Dee_ssize_t lhs, DeeObject *rhs);
#define DeeInt_Size_Hash(lhs) ((Dee_hash_t)(lhs))
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_Size_Compare(size_t lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_Size_CompareEq(size_t lhs, DeeObject *rhs);
DFUNDEF WUNUSED NONNULL((2)) int DCALL DeeInt_Size_TryCompareEq(size_t lhs, DeeObject *rhs);




/* Integer object creation. */
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewInt8(int8_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewUInt8(uint8_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewInt16(int16_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewInt32(int32_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewInt64(int64_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewUInt16(uint16_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewUInt32(uint32_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewUInt64(uint64_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewInt128(Dee_int128_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewUInt128(Dee_uint128_t val);
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *DCALL DeeInt_NewDouble(double val); /* TODO: Rounding? */

/* Create an integer from signed/unsigned LEB data. */
DFUNDEF WUNUSED ATTR_INOUT(1) DREF /*Int*/ DeeObject *DCALL
DeeInt_NewSleb(__BYTE_TYPE__ const **__restrict p_reader);
DFUNDEF WUNUSED ATTR_INOUT(1) DREF /*Int*/ DeeObject *DCALL
DeeInt_NewUleb(__BYTE_TYPE__ const **__restrict p_reader);

/* Write the value of an integer as signed/unsigned LEB data.
 * NOTE: When writing ULEB data, the caller is responsible to ensure that `self' is positive. */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) __BYTE_TYPE__ *DCALL
DeeInt_GetSleb(/*Int*/ DeeObject *__restrict self,
               __BYTE_TYPE__ *__restrict writer);
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) __BYTE_TYPE__ *DCALL
DeeInt_GetUleb(/*Int*/ DeeObject *__restrict self,
               __BYTE_TYPE__ *__restrict writer);

/* Calculate the worst-case required memory for writing a given integer in LEB format. */
#define DeeInt_GetSlebMaxSize(self)                                                     \
	(((DeeIntObject *)Dee_REQUIRES_OBJECT(self))->ob_size < 0                           \
	 ? (((-((DeeIntObject *)Dee_REQUIRES_OBJECT(self))->ob_size + 1) * DIGIT_BITS) / 7) \
	 : (((((DeeIntObject *)Dee_REQUIRES_OBJECT(self))->ob_size + 1) * DIGIT_BITS) / 7))
#define DeeInt_GetUlebMaxSize(self) \
	((((size_t)((DeeIntObject *)Dee_REQUIRES_OBJECT(self))->ob_size + 1) * DIGIT_BITS) / 7)

#define DeeInt_IsNeg(self) (((DeeIntObject *)Dee_REQUIRES_OBJECT(self))->ob_size < 0)


/* Return values for `DeeInt_TryAs*' */
#ifndef Dee_INT_SIGNED
#define Dee_INT_SIGNED         0  /* The returned integer value is signed. */
#define Dee_INT_UNSIGNED       1  /* The returned integer value is unsigned. */
#endif /* !Dee_INT_SIGNED */
#define Dee_INT_POS_OVERFLOW   2  /* ERROR: The returned integer value overflows into the positive. */
#define Dee_INT_NEG_OVERFLOW (-2) /* ERROR: The returned integer value overflows into the negative. */


#ifdef DEE_SOURCE
#ifndef INT_SIGNED
#define INT_SIGNED       Dee_INT_SIGNED    /* The saved integer value is signed. */
#define INT_UNSIGNED     Dee_INT_UNSIGNED  /* The saved integer value is unsigned. */
#endif /* !INT_SIGNED */
#define INT_POS_OVERFLOW Dee_INT_POS_OVERFLOW /* ERROR: The returned integer value overflows into the positive. */
#define INT_NEG_OVERFLOW Dee_INT_NEG_OVERFLOW /* ERROR: The returned integer value overflows into the negative. */
#endif /* DEE_SOURCE */

/* Extract the 8-, 16-, 32-, 64- or 128-bit value of the given integer.
 * NOTE: In theory, deemon integers can have arbitrarily large values,
 *       however in deemon's C api, we must limit ourself to only a set
 *       number of bits.
 * @return: One of `INT_*' (See above) */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_TryGet8Bit(/*Int*/ DeeObject *__restrict self, int8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_TryGet16Bit(/*Int*/ DeeObject *__restrict self, int16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_TryGet32Bit(/*Int*/ DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_TryGet64Bit(/*Int*/ DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_TryGet128Bit(/*Int*/ DeeObject *__restrict self, Dee_int128_t *__restrict value);

/* Similar to the functions above, but explicitly require signed/unsigned 32/64-bit values. */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsInt8(/*Int*/ DeeObject *__restrict self, int8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsInt16(/*Int*/ DeeObject *__restrict self, int16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsInt32(/*Int*/ DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsInt64(/*Int*/ DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsInt128(/*Int*/ DeeObject *__restrict self, Dee_int128_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsUInt8(/*Int*/ DeeObject *__restrict self, uint8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsUInt16(/*Int*/ DeeObject *__restrict self, uint16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsUInt32(/*Int*/ DeeObject *__restrict self, uint32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsUInt64(/*Int*/ DeeObject *__restrict self, uint64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) bool DCALL DeeInt_TryAsUInt128(/*Int*/ DeeObject *__restrict self, Dee_uint128_t *__restrict value);

/* Same as the functions above, but raise an `Error.ValueError.ArithmeticError.IntegerOverflow'
 * for `INT_POS_OVERFLOW' and `INT_NEG_OVERFLOW' and return `-1'. */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_Get8Bit(/*Int*/ DeeObject *__restrict self, int8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_Get16Bit(/*Int*/ DeeObject *__restrict self, int16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_Get32Bit(/*Int*/ DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_Get64Bit(/*Int*/ DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int DCALL DeeInt_Get128Bit(/*Int*/ DeeObject *__restrict self, Dee_int128_t *__restrict value);

/* Read the signed/unsigned values from the given integer.
 * @return: 0:  Successfully read the value.
 * @return: -1: An error occurred (Integer overflow). */
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsInt8)(/*Int*/ DeeObject *__restrict self, int8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsInt16)(/*Int*/ DeeObject *__restrict self, int16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsInt32)(/*Int*/ DeeObject *__restrict self, int32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsInt64)(/*Int*/ DeeObject *__restrict self, int64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsInt128)(/*Int*/ DeeObject *__restrict self, Dee_int128_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsUInt8)(/*Int*/ DeeObject *__restrict self, uint8_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsUInt16)(/*Int*/ DeeObject *__restrict self, uint16_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsUInt32)(/*Int*/ DeeObject *__restrict self, uint32_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsUInt64)(/*Int*/ DeeObject *__restrict self, uint64_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsUInt128)(/*Int*/ DeeObject *__restrict self, Dee_uint128_t *__restrict value);
DFUNDEF WUNUSED ATTR_OUT(2) NONNULL((1)) int (DCALL DeeInt_AsDouble)(/*Int*/ DeeObject *__restrict self, double *__restrict value);


/* Convert an integer to a binary-encoded data array. */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL DeeInt_AsBytes)(/*Int*/ DeeObject *__restrict self,
                       void *__restrict dst, size_t length,
                       bool little_endian, bool as_signed);

/* Convert a binary-encoded data array into an integer. */
DFUNDEF WUNUSED DREF /*Int*/ DeeObject *
(DCALL DeeInt_FromBytes)(void const *buf, size_t length,
                         bool little_endian, bool as_signed);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DeeInt_AsNativeBytes(self, dst, length, as_signed) DeeInt_AsBytes(self, dst, length, true, as_signed)
#define DeeInt_FromNativeBytes(buf, length, as_signed)     DeeInt_FromBytes(buf, length, true, as_signed)
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define DeeInt_AsNativeBytes(self, dst, length, as_signed) DeeInt_AsBytes(self, dst, length, false, as_signed)
#define DeeInt_FromNativeBytes(buf, length, as_signed)     DeeInt_FromBytes(buf, length, false, as_signed)
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */

#define DEE_PRIVATE_TRYGETSINT_1(self, val)  DeeInt_TryAsInt8(self, (int8_t *)(val))
#define DEE_PRIVATE_TRYGETSINT_2(self, val)  DeeInt_TryAsInt16(self, (int16_t *)(val))
#define DEE_PRIVATE_TRYGETSINT_4(self, val)  DeeInt_TryAsInt32(self, (int32_t *)(val))
#define DEE_PRIVATE_TRYGETSINT_8(self, val)  DeeInt_TryAsInt64(self, (int64_t *)(val))
#define DEE_PRIVATE_TRYGETSINT_16(self, val) DeeInt_TryAsInt128(self, (Dee_int128_t *)(val))
#define DEE_PRIVATE_TRYGETUINT_1(self, val)  DeeInt_TryAsUInt8(self, (uint8_t *)(val))
#define DEE_PRIVATE_TRYGETUINT_2(self, val)  DeeInt_TryAsUInt16(self, (uint16_t *)(val))
#define DEE_PRIVATE_TRYGETUINT_4(self, val)  DeeInt_TryAsUInt32(self, (uint32_t *)(val))
#define DEE_PRIVATE_TRYGETUINT_8(self, val)  DeeInt_TryAsUInt64(self, (uint64_t *)(val))
#define DEE_PRIVATE_TRYGETUINT_16(self, val) DeeInt_TryAsUInt128(self, (Dee_uint128_t *)(val))
#define DEE_PRIVATE_TRYGETSINT(size)         DEE_PRIVATE_TRYGETSINT_##size
#define DEE_PRIVATE_TRYGETUINT(size)         DEE_PRIVATE_TRYGETUINT_##size
#define DeeInt_TryAsIntN(size, self, val)    DEE_PRIVATE_TRYGETSINT(size)(self, val)
#define DeeInt_TryAsUIntN(size, self, val)   DEE_PRIVATE_TRYGETUINT(size)(self, val)

#define DEE_PRIVATE_GETSINT_1(self, val)  DeeInt_AsInt8(self, (int8_t *)(val))
#define DEE_PRIVATE_GETSINT_2(self, val)  DeeInt_AsInt16(self, (int16_t *)(val))
#define DEE_PRIVATE_GETSINT_4(self, val)  DeeInt_AsInt32(self, (int32_t *)(val))
#define DEE_PRIVATE_GETSINT_8(self, val)  DeeInt_AsInt64(self, (int64_t *)(val))
#define DEE_PRIVATE_GETSINT_16(self, val) DeeInt_AsInt128(self, (Dee_int128_t *)(val))
#define DEE_PRIVATE_GETUINT_1(self, val)  DeeInt_AsUInt8(self, (uint8_t *)(val))
#define DEE_PRIVATE_GETUINT_2(self, val)  DeeInt_AsUInt16(self, (uint16_t *)(val))
#define DEE_PRIVATE_GETUINT_4(self, val)  DeeInt_AsUInt32(self, (uint32_t *)(val))
#define DEE_PRIVATE_GETUINT_8(self, val)  DeeInt_AsUInt64(self, (uint64_t *)(val))
#define DEE_PRIVATE_GETUINT_16(self, val) DeeInt_AsUInt128(self, (Dee_uint128_t *)(val))
#define DEE_PRIVATE_GETSINT(size)         DEE_PRIVATE_GETSINT_##size
#define DEE_PRIVATE_GETUINT(size)         DEE_PRIVATE_GETUINT_##size
#define DeeInt_AsIntN(size, self, val)    DEE_PRIVATE_GETSINT(size)(self, val)
#define DeeInt_AsUIntN(size, self, val)   DEE_PRIVATE_GETUINT(size)(self, val)

#ifdef __CHAR_UNSIGNED__
#define DeeInt_AsChar(self, result)    DeeInt_AsUIntN(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(char *, result))
#else /* __CHAR_UNSIGNED__ */
#define DeeInt_AsChar(self, result)    DeeInt_AsIntN(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(char *, result))
#endif /* !__CHAR_UNSIGNED__ */
#define DeeInt_AsSChar(self, result)   DeeInt_AsIntN(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(signed char *, result))
#define DeeInt_AsUChar(self, result)   DeeInt_AsUIntN(__SIZEOF_CHAR__, self, Dee_REQUIRES_TYPE(unsigned char *, result))
#define DeeInt_AsShort(self, result)   DeeInt_AsIntN(__SIZEOF_SHORT__, self, Dee_REQUIRES_TYPE(short *, result))
#define DeeInt_AsUShort(self, result)  DeeInt_AsUIntN(__SIZEOF_SHORT__, self, Dee_REQUIRES_TYPE(unsigned short *, result))
#define DeeInt_AsInt(self, result)     DeeInt_AsIntN(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(int *, result))
#define DeeInt_AsUInt(self, result)    DeeInt_AsUIntN(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(unsigned int *, result))
#define DeeInt_AsLong(self, result)    DeeInt_AsIntN(__SIZEOF_LONG__, self, Dee_REQUIRES_TYPE(long *, result))
#define DeeInt_AsULong(self, result)   DeeInt_AsUIntN(__SIZEOF_LONG__, self, Dee_REQUIRES_TYPE(unsigned long *, result))
#ifdef __SIZEOF_LONG_LONG__
#define DeeInt_AsLLong(self, result)   DeeInt_AsIntN(__SIZEOF_LONG_LONG__, self, Dee_REQUIRES_TYPE(__LONGLONG *, result))
#define DeeInt_AsULLong(self, result)  DeeInt_AsUIntN(__SIZEOF_LONG_LONG__, self, Dee_REQUIRES_TYPE(__ULONGLONG *, result))
#endif /* __SIZEOF_LONG_LONG__ */
#define DeeInt_AsSize(self, result)    DeeInt_AsUIntN(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(size_t *, result))
#define DeeInt_AsSSize(self, result)   DeeInt_AsIntN(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(Dee_ssize_t *, result))
#define DeeInt_AsPtrdiff(self, result) DeeInt_AsIntN(__SIZEOF_PTRDIFF_T__, self, Dee_REQUIRES_TYPE(ptrdiff_t *, result))
#define DeeInt_AsIntptr(self, result)  DeeInt_AsIntN(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(intptr_t *, result))
#define DeeInt_AsUIntptr(self, result) DeeInt_AsUIntN(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(uintptr_t *, result))

#define DeeInt_TryAsSSize(self, val)   DeeInt_TryAsIntN(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(Dee_ssize_t *, val))
#define DeeInt_TryAsSize(self, val)    DeeInt_TryAsUIntN(__SIZEOF_SIZE_T__, self, Dee_REQUIRES_TYPE(size_t *, val))
#define DeeInt_TryAsInt(self, val)     DeeInt_TryAsIntN(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(int *, val))
#define DeeInt_TryAsUInt(self, val)    DeeInt_TryAsIntN(__SIZEOF_INT__, self, Dee_REQUIRES_TYPE(unsigned int *, val))
#define DeeInt_TryAsIntptr(self, val)  DeeInt_TryAsIntN(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(intptr_t *, val))
#define DeeInt_TryAsUIntptr(self, val) DeeInt_TryAsUIntN(__SIZEOF_POINTER__, self, Dee_REQUIRES_TYPE(uintptr_t *, val))


#ifndef __NO_builtin_choose_expr
#define DeeInt_TryAsIntX(self, result)                                                                 \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeInt_TryAsInt8(self, (int8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeInt_TryAsInt16(self, (int16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeInt_TryAsInt32(self, (int32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeInt_TryAsInt64(self, (int64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeInt_TryAsInt128(self, (Dee_int128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#define DeeInt_TryAsUIntX(self, result)                                                                  \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeInt_TryAsUInt8(self, (uint8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeInt_TryAsUInt16(self, (uint16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeInt_TryAsUInt32(self, (uint32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeInt_TryAsUInt64(self, (uint64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeInt_TryAsUInt128(self, (Dee_uint128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#define DeeInt_AsIntX(self, result)                                                                 \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeInt_AsInt8(self, (int8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeInt_AsInt16(self, (int16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeInt_AsInt32(self, (int32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeInt_AsInt64(self, (int64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeInt_AsInt128(self, (Dee_int128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#define DeeInt_AsUIntX(self, result)                                                                  \
	__builtin_choose_expr(sizeof(*(result)) == 1,  DeeInt_AsUInt8(self, (uint8_t *)(result)),         \
	__builtin_choose_expr(sizeof(*(result)) == 2,  DeeInt_AsUInt16(self, (uint16_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 4,  DeeInt_AsUInt32(self, (uint32_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 8,  DeeInt_AsUInt64(self, (uint64_t *)(result)),       \
	__builtin_choose_expr(sizeof(*(result)) == 16, DeeInt_AsUInt128(self, (Dee_uint128_t *)(result)), \
	                                               _Dee_invalid_integer_size())))))
#else /* !__NO_builtin_choose_expr */
#define DeeInt_TryAsIntX(self, result)                                              \
	(sizeof(*(result)) == 1 ?  DeeInt_TryAsInt8(self, (int8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeInt_TryAsInt16(self, (int16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeInt_TryAsInt32(self, (int32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeInt_TryAsInt64(self, (int64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeInt_TryAsInt128(self, (Dee_int128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#define DeeInt_TryAsUIntX(self, result)                                               \
	(sizeof(*(result)) == 1 ?  DeeInt_TryAsUInt8(self, (uint8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeInt_TryAsUInt16(self, (uint16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeInt_TryAsUInt32(self, (uint32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeInt_TryAsUInt64(self, (uint64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeInt_TryAsUInt128(self, (Dee_uint128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#define DeeInt_AsIntX(self, result)                                              \
	(sizeof(*(result)) == 1 ?  DeeInt_AsInt8(self, (int8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeInt_AsInt16(self, (int16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeInt_AsInt32(self, (int32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeInt_AsInt64(self, (int64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeInt_AsInt128(self, (Dee_int128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#define DeeInt_AsUIntX(self, result)                                               \
	(sizeof(*(result)) == 1 ?  DeeInt_AsUInt8(self, (uint8_t *)(result)) :         \
	 sizeof(*(result)) == 2 ?  DeeInt_AsUInt16(self, (uint16_t *)(result)) :       \
	 sizeof(*(result)) == 4 ?  DeeInt_AsUInt32(self, (uint32_t *)(result)) :       \
	 sizeof(*(result)) == 8 ?  DeeInt_AsUInt64(self, (uint64_t *)(result)) :       \
	 sizeof(*(result)) == 16 ? DeeInt_AsUInt128(self, (Dee_uint128_t *)(result)) : \
	                           _Dee_invalid_integer_size())
#endif /* __NO_builtin_choose_expr */



/* Convert an integer to/from a string.
 * WARNING: The caller is responsible not to pass a radix equal to `1'.
 *          When a radix equal to `0', it is automatically determined from the passed string. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Int*/ DeeObject *DCALL
DeeInt_FromString(/*utf-8*/ char const *__restrict str,
                  size_t len, uint32_t radix_and_flags);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Int*/ DeeObject *DCALL
DeeInt_FromAscii(/*ascii*/ char const *__restrict str,
                 size_t len, uint32_t radix_and_flags);
#define DEEINT_STRING(radix, flags) ((radix) << DEEINT_STRING_RSHIFT | (flags))
#define DEEINT_STRING_RSHIFT   16
#define DEEINT_STRING_FNORMAL  0x0000
#define DEEINT_STRING_FESCAPED 0x0001 /* Decode escaped linefeeds in the given input string. */
#define DEEINT_STRING_FTRY     0x0002 /* Don't throw a ValueError, but return ITER_DONE. */
#define DEEINT_STRING_FNOSEPS  0x0004 /* Error out if _-characters are encountered during parsing. */

/* @return:  0: Successfully parsed an integer.
 * @return: -1: An error occurred. (never returned when `DEEINT_STRING_FTRY' is set)
 * @return:  1: Failed to parse an integer. (returned when `DEEINT_STRING_FTRY' is set) */
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atoi8)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int8_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atoi16)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int16_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atoi32)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int32_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atoi64)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int64_t *__restrict value);
#define DEEATOI_STRING_FSIGNED 0x0004 /* The generated value is signed. */

#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atos8)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int8_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atos16)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int16_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atos32)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int32_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atos64)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, int64_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atou8)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, uint8_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atou16)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, uint16_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atou32)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, uint32_t *__restrict value);
DFUNDEF WUNUSED NONNULL((1, 4)) int (DCALL Dee_Atou64)(/*utf-8*/ char const *__restrict str, size_t len, uint32_t radix_and_flags, uint64_t *__restrict value);
#else /* __INTELLISENSE__ */
#define Dee_Atos8(str, len, radix_and_flags, value)  Dee_Atoi8(str, len, (radix_and_flags) | DEEATOI_STRING_FSIGNED, value)
#define Dee_Atos16(str, len, radix_and_flags, value) Dee_Atoi16(str, len, (radix_and_flags) | DEEATOI_STRING_FSIGNED, value)
#define Dee_Atos32(str, len, radix_and_flags, value) Dee_Atoi32(str, len, (radix_and_flags) | DEEATOI_STRING_FSIGNED, value)
#define Dee_Atos64(str, len, radix_and_flags, value) Dee_Atoi64(str, len, (radix_and_flags) | DEEATOI_STRING_FSIGNED, value)
#define Dee_Atou8(str, len, radix_and_flags, value)  Dee_Atoi8(str, len, radix_and_flags, (int8_t *)(value))
#define Dee_Atou16(str, len, radix_and_flags, value) Dee_Atoi16(str, len, radix_and_flags, (int16_t *)(value))
#define Dee_Atou32(str, len, radix_and_flags, value) Dee_Atoi32(str, len, radix_and_flags, (int32_t *)(value))
#define Dee_Atou64(str, len, radix_and_flags, value) Dee_Atoi64(str, len, radix_and_flags, (int64_t *)(value))
#endif /* !__INTELLISENSE__ */

#ifdef __NO_builtin_choose_expr
#define DEE_PRIVATE_ATOI_FLAGS(T, flags) \
	(((T)-1) < (T)0 ? (flags) | DEEATOI_STRING_FSIGNED : (flags))
#define Dee_TAtoi(T, str, len, radix_and_flags, value)                                                       \
	(sizeof(T) <= 1 ? Dee_Atoi8(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int8_t *)(value)) :   \
	 sizeof(T) <= 2 ? Dee_Atoi16(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int16_t *)(value)) : \
	 sizeof(T) <= 4 ? Dee_Atoi32(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int32_t *)(value)) : \
	                  Dee_Atoi64(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int64_t *)(value)))
#define Dee_TAtoiu(str, len, radix_and_flags, value)                                \
	(sizeof(T) <= 1 ? Dee_Atoi8(str, len, (radix_and_flags), (int8_t *)(value)) :   \
	 sizeof(T) <= 2 ? Dee_Atoi16(str, len, (radix_and_flags), (int16_t *)(value)) : \
	 sizeof(T) <= 4 ? Dee_Atoi32(str, len, (radix_and_flags), (int32_t *)(value)) : \
	                  Dee_Atoi64(str, len, (radix_and_flags), (int64_t *)(value)))
#else /* __NO_builtin_choose_expr */
#define DEE_PRIVATE_ATOI_FLAGS(T, flags) \
	__builtin_choose_expr(((T)-1) < (T)0, (flags) | DEEATOI_STRING_FSIGNED, (flags))
#define Dee_TAtoi(T, str, len, radix_and_flags, value)                                                                           \
	__builtin_choose_expr(sizeof(T) <= 1, Dee_Atoi8(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int8_t *)(value)),    \
	__builtin_choose_expr(sizeof(T) <= 2, Dee_Atoi16(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int16_t *)(value)),  \
	__builtin_choose_expr(sizeof(T) <= 4, Dee_Atoi32(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int32_t *)(value)),  \
	                                      Dee_Atoi64(str, len, DEE_PRIVATE_ATOI_FLAGS(T, radix_and_flags), (int64_t *)(value)))))
#define Dee_TAtoiu(str, len, radix_and_flags, value)                                                           \
	__builtin_choose_expr(sizeof(*(value)) <= 1, Dee_Atoi8(str, len, (radix_and_flags), (int8_t *)(value)),    \
	__builtin_choose_expr(sizeof(*(value)) <= 2, Dee_Atoi16(str, len, (radix_and_flags), (int16_t *)(value)),  \
	__builtin_choose_expr(sizeof(*(value)) <= 4, Dee_Atoi32(str, len, (radix_and_flags), (int32_t *)(value)),  \
	                                             Dee_Atoi64(str, len, (radix_and_flags), (int64_t *)(value)))))
#endif /* !__NO_builtin_choose_expr */
#define Dee_TAtois(str, len, radix_and_flags, value)                                                                                    \
	Dee_TAtoiu(str, len, (radix_and_flags) | DEEATOI_STRING_FSIGNED, value)


/* Print an integer to a given format-printer.
 * Radix must be one of `2', `4', `8', `10' or `16' and
 * if it isn't, a `NotImplemented' error is thrown.
 * This list of supported radices may be extended in the future.
 * @param: precision: The minimum number of digits (excluding numsys/sign
 *                    prefixes) to print. Padding is done using '0'-chars */
DFUNDEF WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
DeeInt_Print(/*Int*/ DeeObject *__restrict self, uint32_t radix_and_flags,
             size_t precision, Dee_formatprinter_t printer, void *arg);
#define DEEINT_PRINT(radix, flags) ((radix) << DEEINT_PRINT_RSHIFT | (flags))
#define DEEINT_PRINT_RSHIFT  16
#define DEEINT_PRINT_FNORMAL 0x0000
#define DEEINT_PRINT_FUPPER  0x0001 /* Use uppercase characters for printing digits above `9' */
#define DEEINT_PRINT_FNUMSYS 0x0002 /* Prepend the number system prefix before the integer itself (e.g.: `0x').
                                     * NOTE: If the radix cannot be represented as a prefix, this flag is ignored. */
#define DEEINT_PRINT_FSIGN   0x0004 /* Always prepend a sign, even before for positive numbers. */
#define DEEINT_PRINT_FSEPS   0x0008 /* Include _-characters to denote thousands/group-separators */

#define DEEINT_PRINT_BIN     DEEINT_PRINT(2, DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_OCT     DEEINT_PRINT(8, DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_DEC     DEEINT_PRINT(10, DEEINT_PRINT_FNORMAL)
#define DEEINT_PRINT_HEX     DEEINT_PRINT(16, DEEINT_PRINT_FNORMAL)

#define DeeInt_PrintRepr(self, printer, arg) \
	DeeInt_Print(self, DEEINT_PRINT_DEC, 0, printer, arg)


#define DEE_PRIVATE_NEWINT_1      DeeInt_NewInt8
#define DEE_PRIVATE_NEWINT_2      DeeInt_NewInt16
#define DEE_PRIVATE_NEWINT_4      DeeInt_NewInt32
#define DEE_PRIVATE_NEWINT_8      DeeInt_NewInt64
#define DEE_PRIVATE_NEWINT_16     DeeInt_NewInt128
#define DEE_PRIVATE_NEWUINT_1     DeeInt_NewUInt8
#define DEE_PRIVATE_NEWUINT_2     DeeInt_NewUInt16
#define DEE_PRIVATE_NEWUINT_4     DeeInt_NewUInt32
#define DEE_PRIVATE_NEWUINT_8     DeeInt_NewUInt64
#define DEE_PRIVATE_NEWUINT_16    DeeInt_NewUInt128
#define DEE_PRIVATE_NEWINT(size)  DEE_PRIVATE_NEWINT_##size
#define DEE_PRIVATE_NEWUINT(size) DEE_PRIVATE_NEWUINT_##size

/* Create a new integer object by looking at sizeof(v). */
#ifdef __NO_builtin_choose_expr
#define DeeInt_NEWS(value)                                  \
	(sizeof(value) <= 1 ? DeeInt_NewInt8((int8_t)(value)) :   \
	 sizeof(value) <= 2 ? DeeInt_NewInt16((int16_t)(value)) : \
	 sizeof(value) <= 4 ? DeeInt_NewInt32((int32_t)(value)) : \
	                      DeeInt_NewInt64((int64_t)(value)))
#define DeeInt_NEWU(value)                                   \
	(sizeof(value) <= 1 ? DeeInt_NewUInt8((uint8_t)(value)) :   \
	 sizeof(value) <= 2 ? DeeInt_NewUInt16((uint16_t)(value)) : \
	 sizeof(value) <= 4 ? DeeInt_NewUInt32((uint32_t)(value)) : \
	                      DeeInt_NewUInt64((uint64_t)(value)))
#else /* __NO_builtin_choose_expr */
#define DeeInt_NEWS(value)                                                        \
	__builtin_choose_expr(sizeof(value) <= 1, DeeInt_NewInt8((int8_t)(value)),    \
	__builtin_choose_expr(sizeof(value) <= 2, DeeInt_NewInt16((int16_t)(value)),  \
	__builtin_choose_expr(sizeof(value) <= 4, DeeInt_NewInt32((int32_t)(value)),  \
	                                          DeeInt_NewInt64((int64_t)(value)))))
#define DeeInt_NEWU(value)                                                          \
	__builtin_choose_expr(sizeof(value) <= 1, DeeInt_NewUInt8((uint8_t)(value)),    \
	__builtin_choose_expr(sizeof(value) <= 2, DeeInt_NewUInt16((uint16_t)(value)),  \
	__builtin_choose_expr(sizeof(value) <= 4, DeeInt_NewUInt32((uint32_t)(value)),  \
	                                          DeeInt_NewUInt64((uint64_t)(value)))))
#endif /* !__NO_builtin_choose_expr */



/* Create a new integer object with an input integral value `val' of `size' bytes. */
#define DeeInt_NewIntN(size, val)  DEE_PRIVATE_NEWINT(size)(val)
#define DeeInt_NewUIntN(size, val) DEE_PRIVATE_NEWUINT(size)(val)

#ifndef __CHAR_UNSIGNED__
#define DeeInt_NewChar(val)    DeeInt_NewIntN(__SIZEOF_CHAR__, val)
#else /* !__CHAR_UNSIGNED__ */
#define DeeInt_NewChar(val)    DeeInt_NewUIntN(__SIZEOF_CHAR__, val)
#endif /* __CHAR_UNSIGNED__ */
#define DeeInt_NewSChar(val)   DeeInt_NewIntN(__SIZEOF_CHAR__, val)
#define DeeInt_NewUChar(val)   DeeInt_NewUIntN(__SIZEOF_CHAR__, val)
#define DeeInt_NewShort(val)   DeeInt_NewIntN(__SIZEOF_SHORT__, val)
#define DeeInt_NewUShort(val)  DeeInt_NewUIntN(__SIZEOF_SHORT__, val)
#define DeeInt_NewInt(val)     DeeInt_NewIntN(__SIZEOF_INT__, val)
#define DeeInt_NewUInt(val)    DeeInt_NewUIntN(__SIZEOF_INT__, val)
#define DeeInt_NewLong(val)    DeeInt_NewIntN(__SIZEOF_LONG__, val)
#define DeeInt_NewULong(val)   DeeInt_NewUIntN(__SIZEOF_LONG__, val)
#ifdef __COMPILER_HAVE_LONGLONG
#define DeeInt_NewLLong(val)   DeeInt_NewIntN(__SIZEOF_LONG_LONG__, val)
#define DeeInt_NewULLong(val)  DeeInt_NewUIntN(__SIZEOF_LONG_LONG__, val)
#endif /* __COMPILER_HAVE_LONGLONG */
#define DeeInt_NewSize(val)    DeeInt_NewUIntN(__SIZEOF_SIZE_T__, val)
#define DeeInt_NewHash(val)    DeeInt_NewUIntN(__SIZEOF_POINTER__, val)
#define DeeInt_NewSSize(val)   DeeInt_NewIntN(__SIZEOF_SIZE_T__, val)
#define DeeInt_NewPtrdiff(val) DeeInt_NewIntN(__SIZEOF_PTRDIFF_T__, val)
#define DeeInt_NewIntptr(val)  DeeInt_NewIntN(__SIZEOF_POINTER__, val)
#define DeeInt_NewUIntptr(val) DeeInt_NewUIntN(__SIZEOF_POINTER__, val)

#if DIGIT_BITS <= 16
#define DeeInt_NewDigit(val)      DeeInt_NewUIntN(2, val)
#define DeeInt_NewSDigit(val)     DeeInt_NewIntN(2, val)
#define DeeInt_NewTwoDigits(val)  DeeInt_NewUIntN(4, val)
#define DeeInt_NewSTwoDigits(val) DeeInt_NewIntN(4, val)
#else /* DIGIT_BITS <= 16 */
#define DeeInt_NewDigit(val)      DeeInt_NewUIntN(4, val)
#define DeeInt_NewSDigit(val)     DeeInt_NewIntN(4, val)
#define DeeInt_NewTwoDigits(val)  DeeInt_NewUIntN(8, val)
#define DeeInt_NewSTwoDigits(val) DeeInt_NewIntN(8, val)
#endif /* DIGIT_BITS > 16 */

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeInt_AsInt32(self, value)                  __builtin_expect(DeeInt_AsInt32(self, value), 0)
#define DeeInt_AsInt64(self, value)                  __builtin_expect(DeeInt_AsInt64(self, value), 0)
#define DeeInt_AsUInt32(self, value)                 __builtin_expect(DeeInt_AsUInt32(self, value), 0)
#define DeeInt_AsUInt64(self, value)                 __builtin_expect(DeeInt_AsUInt64(self, value), 0)
#define DeeInt_AsUInt128(self, value)                __builtin_expect(DeeInt_AsUInt128(self, value), 0)
#define DeeInt_TryAsInt32(self, value)               __builtin_expect(DeeInt_TryAsInt32(self, value), true)
#define DeeInt_TryAsInt64(self, value)               __builtin_expect(DeeInt_TryAsInt64(self, value), true)
#define DeeInt_TryAsUInt32(self, value)              __builtin_expect(DeeInt_TryAsUInt32(self, value), true)
#define DeeInt_TryAsUInt64(self, value)              __builtin_expect(DeeInt_TryAsUInt64(self, value), true)
#define DeeInt_TryAsUInt128(self, value)             __builtin_expect(DeeInt_TryAsUInt128(self, value), true)
#define Dee_Atoi8(str, len, radix_and_flags, value)  __builtin_expect(Dee_Atoi8(str, len, radix_and_flags, value), 0)
#define Dee_Atoi16(str, len, radix_and_flags, value) __builtin_expect(Dee_Atoi16(str, len, radix_and_flags, value), 0)
#define Dee_Atoi32(str, len, radix_and_flags, value) __builtin_expect(Dee_Atoi32(str, len, radix_and_flags, value), 0)
#define Dee_Atoi64(str, len, radix_and_flags, value) __builtin_expect(Dee_Atoi64(str, len, radix_and_flags, value), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_INT_H */
