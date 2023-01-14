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
#ifndef GUARD_DEEMON_OBJECTS_NUMERIC_C
#define GUARD_DEEMON_OBJECTS_NUMERIC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include <hybrid/byteswap.h>

#include <math.h> /* FIXME: This needs a feature check! */

#include "../runtime/strings.h"

DECL_BEGIN

INTDEF int DCALL none_i1(void *UNUSED(b));
INTDEF int DCALL none_i2(void *UNUSED(b), void *UNUSED(c));

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asflt(DeeObject *__restrict self) {
	double result;
	if (DeeObject_AsDouble(self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass8(DeeObject *__restrict self) {
	int8_t result;
	if (DeeObject_AsInt8(self, &result))
		goto err;
	return DeeInt_NewS8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass16(DeeObject *__restrict self) {
	int16_t result;
	if (DeeObject_AsInt16(self, &result))
		goto err;
	return DeeInt_NewS16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass32(DeeObject *__restrict self) {
	int32_t result;
	if (DeeObject_AsInt32(self, &result))
		goto err;
	return DeeInt_NewS32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass64(DeeObject *__restrict self) {
	int64_t result;
	if (DeeObject_AsInt64(self, &result))
		goto err;
	return DeeInt_NewS64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_ass128(DeeObject *__restrict self) {
	dint128_t result;
	if (DeeObject_AsInt128(self, &result))
		goto err;
	return DeeInt_NewS128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu8(DeeObject *__restrict self) {
	uint8_t result;
	if (DeeObject_AsUInt8(self, &result))
		goto err;
	return DeeInt_NewU8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu16(DeeObject *__restrict self) {
	uint16_t result;
	if (DeeObject_AsUInt16(self, &result))
		goto err;
	return DeeInt_NewU16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu32(DeeObject *__restrict self) {
	uint32_t result;
	if (DeeObject_AsUInt32(self, &result))
		goto err;
	return DeeInt_NewU32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu64(DeeObject *__restrict self) {
	uint64_t result;
	if (DeeObject_AsUInt64(self, &result))
		goto err;
	return DeeInt_NewU64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_asu128(DeeObject *__restrict self) {
	duint128_t result;
	if (DeeObject_AsUInt128(self, &result))
		goto err;
	return DeeInt_NewU128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed8(DeeObject *__restrict self) {
	int8_t result;
	if unlikely(DeeObject_GetInt8(self, &result) < 0)
		goto err;
	return DeeInt_NewS8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned8(DeeObject *__restrict self) {
	uint8_t result;
	if unlikely(DeeObject_GetInt8(self, (int8_t *)&result) < 0)
		goto err;
	return DeeInt_NewU8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed16(DeeObject *__restrict self) {
	int16_t result;
	if unlikely(DeeObject_GetInt16(self, &result) < 0)
		goto err;
	return DeeInt_NewS16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_GetInt16(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewU16(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed32(DeeObject *__restrict self) {
	int32_t result;
	if unlikely(DeeObject_GetInt32(self, &result) < 0)
		goto err;
	return DeeInt_NewS32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_GetInt32(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewU32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed64(DeeObject *__restrict self) {
	int64_t result;
	if unlikely(DeeObject_GetInt64(self, &result) < 0)
		goto err;
	return DeeInt_NewS64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_GetInt64(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewU64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_signed128(DeeObject *__restrict self) {
	dint128_t result;
	if unlikely(DeeObject_GetInt128(self, &result) < 0)
		goto err;
	return DeeInt_NewS128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_unsigned128(DeeObject *__restrict self) {
	duint128_t result;
	if unlikely(DeeObject_GetInt128(self, (dint128_t *)&result) < 0)
		goto err;
	return DeeInt_NewU128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_GetInt16(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewU16(BSWAP16(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_GetInt32(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewU32(BSWAP32(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_GetInt64(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewU64(BSWAP64(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_swap128(DeeObject *__restrict self) {
	duint128_t result;
	uint64_t temp;
	if unlikely(DeeObject_GetInt128(self, (dint128_t *)&result) < 0)
		goto err;
	temp = DUINT128_GET64(result)[0];
	DUINT128_GET64(result)
	[0] = BSWAP64(DUINT128_GET64(result)[1]);
	DUINT128_GET64(result)
	[1] = BSWAP64(temp);
	return DeeInt_NewU128(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap16(DeeObject *__restrict self) {
	uint16_t result;
	if unlikely(DeeObject_GetInt16(self, (int16_t *)&result) < 0)
		goto err;
	return DeeInt_NewS16((int16_t)BSWAP16(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap32(DeeObject *__restrict self) {
	uint32_t result;
	if unlikely(DeeObject_GetInt32(self, (int32_t *)&result) < 0)
		goto err;
	return DeeInt_NewS32((int64_t)BSWAP32(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap64(DeeObject *__restrict self) {
	uint64_t result;
	if unlikely(DeeObject_GetInt64(self, (int64_t *)&result) < 0)
		goto err;
	return DeeInt_NewS64((int64_t)BSWAP64(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
numeric_sswap128(DeeObject *__restrict self) {
	dint128_t result;
	uint64_t temp;
	if unlikely(DeeObject_GetInt128(self, (dint128_t *)&result) < 0)
		goto err;
	temp = DUINT128_GET64(result)[0];
	DUINT128_GET64(result)
	[0] = BSWAP64(DUINT128_GET64(result)[1]);
	DUINT128_GET64(result)
	[1] = BSWAP64(temp);
	return DeeInt_NewS128(result);
err:
	return NULL;
}


INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_popcount(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_ffs(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_partity(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_ctz(DeeIntObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL int_get_msb(DeeIntObject *__restrict self);

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_popcount(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_popcount(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_ffs(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_ffs(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_partity(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_partity(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_ctz(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_ctz(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeIntObject *DCALL
numeric_get_msb(DeeObject *__restrict self) {
	DREF DeeIntObject *result;
	DREF DeeIntObject *asint;
	asint = (DREF DeeIntObject *)DeeObject_Int(self);
	if unlikely(!asint)
		goto err;
	result = int_get_msb(asint);
	Dee_Decref(asint);
	return result;
err:
	return NULL;
}



PRIVATE struct type_getset tpconst numeric_getsets[] = {
	TYPE_GETTER(STR_int, &DeeObject_Int,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "Return @this number as an integer, truncating all digits after a dot/comma"),
	TYPE_GETTER(STR_float, &numeric_asflt,
	            "->?Dfloat\n"
	            "@throw NotImplemented @this number does not implement ${operator float}\n"
	            "Return @this number as a floating point value"),
	TYPE_GETTER("s8", &numeric_ass8,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${-128 ... 127}"),
	TYPE_GETTER("s16", &numeric_ass16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${-32768 ... 32767}"),
	TYPE_GETTER("s32", &numeric_ass32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${-2147483648 ... 2147483647}"),
	TYPE_GETTER("s64", &numeric_ass64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${-9223372036854775808 ... 9223372036854775807}"),
	TYPE_GETTER("s128", &numeric_ass128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${-170141183460469231731687303715884105728 ... 170141183460469231731687303715884105727}"),
	TYPE_GETTER("u8", &numeric_asu8,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${0x0 ... 0xff}"),
	TYPE_GETTER("u16", &numeric_asu16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffff}"),
	TYPE_GETTER("u32", &numeric_asu32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffff}"),
	TYPE_GETTER("u64", &numeric_asu64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffffffffffff}"),
	TYPE_GETTER("u128", &numeric_asu128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Return @this number as an integer in the range of ${0x0 ... 0xffffffffffffffffffffffffffffffff}"),
	TYPE_GETTER("signed8", &numeric_signed8,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${-128 ... 127}, return the same value as @s8\n"
	            "Otherwise, an integer in the range ${128 ... 255} is returned as ${256 - this}\n"
	            "This is the same behavior as casting an 8-bit integer to becoming signed, by "
	            /**/ "re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed16", &numeric_signed16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${-32768 ... 32767}, return the same value as @s16\n"
	            "Otherwise, an integer in the range ${32768 ... 65535} is returned as ${65536 - this}\n"
	            "This is the same behavior as casting an 16-bit integer to becoming signed, by "
	            /**/ "re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed32", &numeric_signed32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${-2147483648 ... 2147483647}, return the same value as @s32\n"
	            "Otherwise, an integer in the range ${2147483648 ... 4294967295} is returned as ${4294967296 - this}\n"
	            "This is the same behavior as casting an 32-bit integer to becoming signed, by "
	            /**/ "re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed64", &numeric_signed64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${-9223372036854775808 ... 9223372036854775807}, return the same value as @s64\n"
	            "Otherwise, an integer in the range ${9223372036854775808 ... 18446744073709551615} is returned as ${18446744073709551616 - this}\n"
	            "This is the same behavior as casting an 64-bit integer to becoming signed, by "
	            /**/ "re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("signed128", &numeric_signed128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${-170141183460469231731687303715884105728 ... 170141183460469231731687303715884105727}, return the same value as @s128\n"
	            "Otherwise, an integer in the range ${170141183460469231731687303715884105728 ... 340282366920938463463374607431768211455} is returned as ${340282366920938463463374607431768211456 - this}\n"
	            "This is the same behavior as casting an 128-bit integer to becoming signed, by "
	            /**/ "re-interpreting the most significant bit as a sign-bit"),
	TYPE_GETTER("unsigned8", &numeric_unsigned8,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${0 ... 255}, return the same value as @u8\n"
	            "Otherwise, an integer in the range ${-128 ... -1} is returned as ${256 + this}\n"
	            "This is the same behavior as casting an 8-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned16", &numeric_unsigned16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${0 ... 65535}, return the same value as @u16\n"
	            "Otherwise, an integer in the range ${-32768 ... -1} is returned as ${65536 + this}\n"
	            "This is the same behavior as casting an 16-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned32", &numeric_unsigned32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${0 ... 4294967295}, return the same value as @u32\n"
	            "Otherwise, an integer in the range ${-2147483648 ... -1} is returned as ${4294967296 + this}\n"
	            "This is the same behavior as casting an 32-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned64", &numeric_unsigned64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${0 ... 18446744073709551615}, return the same value as @u64\n"
	            "Otherwise, an integer in the range ${-9223372036854775808 ... -1} is returned as ${18446744073709551616 + this}\n"
	            "This is the same behavior as casting an 64-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),
	TYPE_GETTER("unsigned128", &numeric_unsigned128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "When @this number fits the range ${0 ... 340282366920938463463374607431768211455}, return the same value as @u128\n"
	            "Otherwise, an integer in the range ${-170141183460469231731687303715884105728 ... -1} is returned as ${340282366920938463463374607431768211456 + this}\n"
	            "This is the same behavior as casting an 128-bit integer to becoming unsigned, by "
	            /**/ "ignoring the most significant bit from potentially being a sign-bit"),

	TYPE_GETTER("swap16", &numeric_swap16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Take the integer generated by ?#u16 and exchange its low and high 8 bits"),
	TYPE_GETTER("swap32", &numeric_swap32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap16, but ?#u32 is taken as input, and the 8-bit tuples $abcd are re-arranged as $dcba"),
	TYPE_GETTER("swap64", &numeric_swap64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap16, but ?#u64 is taken as input, and the 8-bit tuples $abcdefgh are re-arranged as $hgfedcba"),
	TYPE_GETTER("swap128", &numeric_swap128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap16, but ?#u128 is taken as input, and the 8-bit tuples $abcdefghijklmnop are re-arranged as $ponmlkjihgfedcba"),
	TYPE_GETTER("sswap16", &numeric_sswap16,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Same as ${this.swap16.signed16}"),
	TYPE_GETTER("sswap32", &numeric_sswap32,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Same as ${this.swap32.signed32}"),
	TYPE_GETTER("sswap64", &numeric_sswap64,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Same as ${this.swap64.signed64}"),
	TYPE_GETTER("sswap128", &numeric_sswap128,
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Same as ${this.swap128.signed128}"),

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LE_SEL(cast, swap) cast
#define BE_SEL(cast, swap) swap
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define LE_SEL(cast, swap) swap
#define BE_SEL(cast, swap) cast
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	TYPE_GETTER("leswap16", &LE_SEL(numeric_unsigned16, numeric_swap16),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap16, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned16"),
	TYPE_GETTER("leswap32", &LE_SEL(numeric_unsigned32, numeric_swap32),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap32, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned32"),
	TYPE_GETTER("leswap64", &LE_SEL(numeric_unsigned64, numeric_swap64),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap64, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned64"),
	TYPE_GETTER("leswap128", &LE_SEL(numeric_unsigned128, numeric_swap128),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap128, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#unsigned128"),
	TYPE_GETTER("beswap16", &BE_SEL(numeric_unsigned16, numeric_swap16),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap16, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned16"),
	TYPE_GETTER("beswap32", &BE_SEL(numeric_unsigned32, numeric_swap32),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap32, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned32"),
	TYPE_GETTER("beswap64", &BE_SEL(numeric_unsigned64, numeric_swap64),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap64, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned64"),
	TYPE_GETTER("beswap128", &BE_SEL(numeric_unsigned128, numeric_swap128),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#swap128, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#unsigned128"),
	TYPE_GETTER("lesswap16", &LE_SEL(numeric_signed16, numeric_sswap16),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap16, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed16"),
	TYPE_GETTER("lesswap32", &LE_SEL(numeric_signed32, numeric_sswap32),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap32, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed32"),
	TYPE_GETTER("lesswap64", &LE_SEL(numeric_signed64, numeric_sswap64),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap64, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed64"),
	TYPE_GETTER("lesswap128", &LE_SEL(numeric_signed128, numeric_sswap128),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap128, but a little-endian encoded integer is converted to host-endian\n"
	            "When the host already is little-endian, this is identical to ?#signed128"),
	TYPE_GETTER("besswap16", &BE_SEL(numeric_signed16, numeric_sswap16),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap16, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed16"),
	TYPE_GETTER("besswap32", &BE_SEL(numeric_signed32, numeric_sswap32),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap32, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed32"),
	TYPE_GETTER("besswap64", &BE_SEL(numeric_signed64, numeric_sswap64),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap64, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed64"),
	TYPE_GETTER("besswap128", &BE_SEL(numeric_signed128, numeric_sswap128),
	            "->?Dint\n"
	            "@throw NotImplemented @this number does not implement ${operator int}\n"
	            "@throw IntegerOverflow The value of @this number is outside the requested range\n"
	            "Similar to ?#sswap128, but a big-endian encoded integer is converted to host-endian\n"
	            "When the host already is big-endian, this is identical to ?#signed128"),
#undef LE_SEL
#undef BE_SEL

	/* Binary property helper functions */
	TYPE_GETTER("popcount", &numeric_get_popcount,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "Return the number of 1-bits in this integer"),
	TYPE_GETTER("ffs", &numeric_get_ffs,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "FindFirstSet: same as ?#ctz +1, but returns $0 when ${this == 0}"),
	TYPE_GETTER("partity", &numeric_get_partity,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "Return $0 or $1 indivative of the even/odd parity of @this. Same as ${this.popcount % 2}"),
	TYPE_GETTER("ctz", &numeric_get_ctz,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this <= 0}\n"
	            "CountTrailingZeros: return the number of trailing zero-bits:\n"
	            "${"
	            /**/ "local n = this.ctz;\n"
	            /**/ "assert this == (this >> n) << n;"
	            "}"),
	TYPE_GETTER("msb", &numeric_get_msb,
	            "->?Dint\n"
	            "@throw IntegerOverflow When ${this < 0}\n"
	            "MostSignificantBit: return the index of the most significant 1-bit:\n"
	            "${"
	            /**/ "assert (this >> this.msb) == 1;"
	            "}"),
	TYPE_GETSET_END
};


PUBLIC DeeTypeObject DeeNumeric_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Numeric),
	/* .tp_doc      = */ DOC("Base class for ?Dint and ?Dfloat"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1,
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO: function gcd(other: Numeric): Numeric (GreatestCommonDivisor) */
	/* .tp_getsets       = */ numeric_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_NUMERIC_C */
