/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_INT_H
#define GUARD_DEEMON_CXX_INT_H 1

#include "../api.h"
#include "api.h"

#include <hybrid/typecore.h> /* __*_TYPE__ */

#include "../format.h"          /* Dee_PCKdSIZ, Dee_PCKuSIZ */
#include "../int.h"             /* DeeInt_*, Dee_INT_STRING, Dee_INT_STRING_FNORMAL */
#include "../object.h"
#include "../system-features.h" /* ceil, floor, isgreater, isgreaterequal, isless, islessequal, islessgreater, isnormal, isunordered, nextafter, pow, round, strlen, trunc */
#include "../types.h"           /* DREF, DeeObject, Dee_int128_t, Dee_ssize_t, Dee_uint128_t, _Dee_HashSelectC */
#include "numeric.h"
#include "object.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* intN_t, uintN_t */

DEE_CXX_BEGIN

class Type;
class Bytes;
class bool_;

class int_
	: public Numeric
	, public detail::MathProxyAccessor<int_, int_>
{
public:
	using detail::MathProxyAccessor<int_, int_>::inv;
	using detail::MathProxyAccessor<int_, int_>::pos;
	using detail::MathProxyAccessor<int_, int_>::neg;
	using detail::MathProxyAccessor<int_, int_>::add;
	using detail::MathProxyAccessor<int_, int_>::sub;
	using detail::MathProxyAccessor<int_, int_>::mul;
	using detail::MathProxyAccessor<int_, int_>::div;
	using detail::MathProxyAccessor<int_, int_>::mod;
	using detail::MathProxyAccessor<int_, int_>::shl;
	using detail::MathProxyAccessor<int_, int_>::shr;
	using detail::MathProxyAccessor<int_, int_>::and_;
	using detail::MathProxyAccessor<int_, int_>::or_;
	using detail::MathProxyAccessor<int_, int_>::xor_;
	using detail::MathProxyAccessor<int_, int_>::pow;
	using detail::MathProxyAccessor<int_, int_>::operator~;
	using detail::MathProxyAccessor<int_, int_>::operator+;
	using detail::MathProxyAccessor<int_, int_>::operator-;
	using detail::MathProxyAccessor<int_, int_>::operator*;
	using detail::MathProxyAccessor<int_, int_>::operator/;
	using detail::MathProxyAccessor<int_, int_>::operator%;
	using detail::MathProxyAccessor<int_, int_>::operator<<;
	using detail::MathProxyAccessor<int_, int_>::operator>>;
	using detail::MathProxyAccessor<int_, int_>::operator&;
	using detail::MathProxyAccessor<int_, int_>::operator|;
	using detail::MathProxyAccessor<int_, int_>::operator^;
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeInt_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeInt_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeInt_CheckExact(ob);
	}

public:
	static WUNUSED Ref<int_> zero() {
		return nonnull(DeeInt_Zero);
	}
	static WUNUSED Ref<int_> one() {
		return nonnull(DeeInt_One);
	}
	static WUNUSED Ref<int_> minus_one() {
		return nonnull(DeeInt_MinusOne);
	}
	static WUNUSED Ref<int_> ofsign(int sign) {
		return nonnull(DeeInt_FromSign(sign));
	}
	static WUNUSED Ref<int_> of(__INT8_TYPE__ value) {
		return inherit(DeeInt_NewInt8(value));
	}
	static WUNUSED Ref<int_> of(__INT16_TYPE__ value) {
		return inherit(DeeInt_NewInt16(value));
	}
	static WUNUSED Ref<int_> of(__INT32_TYPE__ value) {
		return inherit(DeeInt_NewInt32(value));
	}
	static WUNUSED Ref<int_> of(__INT64_TYPE__ value) {
		return inherit(DeeInt_NewInt64(value));
	}
	static WUNUSED Ref<int_> of(Dee_int128_t value) {
		return inherit(DeeInt_NewInt128(value));
	}
	static WUNUSED Ref<int_> of(__UINT8_TYPE__ value) {
		return inherit(DeeInt_NewUInt8(value));
	}
	static WUNUSED Ref<int_> of(__UINT16_TYPE__ value) {
		return inherit(DeeInt_NewUInt16(value));
	}
	static WUNUSED Ref<int_> of(__UINT32_TYPE__ value) {
		return inherit(DeeInt_NewUInt32(value));
	}
	static WUNUSED Ref<int_> of(__UINT64_TYPE__ value) {
		return inherit(DeeInt_NewUInt64(value));
	}
	static WUNUSED Ref<int_> of(Dee_uint128_t value) {
		return inherit(DeeInt_NewUInt128(value));
	}
#ifdef __FIFTHINT_TYPE__
	static WUNUSED Ref<int_> of(__FIFTHINT_TYPE__ value) {
		return inherit(DeeInt_NEWS(value));
	}
	static WUNUSED Ref<int_> of(__UFIFTHINT_TYPE__ value) {
		return inherit(DeeInt_NEWU(value));
	}
#endif /* __FIFTHINT_TYPE__ */

	static WUNUSED Ref<int_> ofutf8(/*ascii*/ char const *__restrict str, size_t len,
	                                 uint32_t radix_and_flags = Dee_INT_STRING(0, Dee_INT_STRING_FNORMAL)) {
		return inherit(DeeInt_FromString(str, len, radix_and_flags));
	}
	static WUNUSED Ref<int_> ofutf8(/*ascii*/ char const *__restrict str) {
		return inherit(DeeInt_FromString(str, strlen(str), Dee_INT_STRING(0, Dee_INT_STRING_FNORMAL)));
	}

	static WUNUSED Ref<int_> ofascii(/*ascii*/ char const *__restrict str, size_t len,
	                                 uint32_t radix_and_flags = Dee_INT_STRING(0, Dee_INT_STRING_FNORMAL)) {
		return inherit(DeeInt_FromAscii(str, len, radix_and_flags));
	}
	static WUNUSED Ref<int_> ofascii(/*ascii*/ char const *__restrict str) {
		return inherit(DeeInt_FromAscii(str, strlen(str), Dee_INT_STRING(0, Dee_INT_STRING_FNORMAL)));
	}
public:
	/* Integer conversion */
	int_ &cgetval(char &value) {
		throw_if_nonzero(DeeInt_AsChar(this, &value));
		return *this;
	}
	int_ &cgetval(signed char &value) {
		throw_if_nonzero(DeeInt_AsSChar(this, &value));
		return *this;
	}
	int_ &cgetval(unsigned char &value) {
		throw_if_nonzero(DeeInt_AsUChar(this, &value));
		return *this;
	}
	int_ &cgetval(short &value) {
		throw_if_nonzero(DeeInt_AsShort(this, &value));
		return *this;
	}
	int_ &cgetval(unsigned short &value) {
		throw_if_nonzero(DeeInt_AsUShort(this, &value));
		return *this;
	}
	int_ &cgetval(int &value) {
		throw_if_nonzero(DeeInt_AsInt(this, &value));
		return *this;
	}
	int_ &cgetval(unsigned int &value) {
		throw_if_nonzero(DeeInt_AsUInt(this, &value));
		return *this;
	}
	int_ &cgetval(long &value) {
		throw_if_nonzero(DeeInt_AsLong(this, &value));
		return *this;
	}
	int_ &cgetval(unsigned long &value) {
		throw_if_nonzero(DeeInt_AsULong(this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	int_ &cgetval(__LONGLONG &value) {
		throw_if_nonzero(DeeInt_AsLLong(this, &value));
		return *this;
	}
	int_ &cgetval(__ULONGLONG &value) {
		throw_if_nonzero(DeeInt_AsULLong(this, &value));
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	int_ &cgetval(Dee_int128_t &value) {
		throw_if_nonzero(DeeInt_AsInt128(this, &value));
		return *this;
	}
	int_ &cgetval(Dee_uint128_t &value) {
		throw_if_nonzero(DeeInt_AsUInt128(this, &value));
		return *this;
	}

	/* Helper functions to explicitly convert an object to an integral value. */
	template<class T> WUNUSED T casval() {
		T result;
		cgetval(result);
		return result;
	}
	WUNUSED short casshort() {
		return casval<short>();
	}
	WUNUSED unsigned short casushort() {
		return casval<unsigned short>();
	}
	WUNUSED int casint() {
		return casval<int>();
	}
	WUNUSED unsigned int casuint() {
		return casval<unsigned int>();
	}
	WUNUSED long caslong() {
		return casval<long>();
	}
	WUNUSED unsigned long casulong() {
		return casval<unsigned long>();
	}
#ifdef __COMPILER_HAVE_LONGLONG
	WUNUSED __LONGLONG casllong() {
		return casval<__LONGLONG>();
	}
	WUNUSED __ULONGLONG casullong() {
		return casval<__ULONGLONG>();
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	WUNUSED int8_t cass8() {
		return casval<int8_t>();
	}
	WUNUSED int16_t cass16() {
		return casval<int16_t>();
	}
	WUNUSED int32_t cass32() {
		return casval<int32_t>();
	}
	WUNUSED int64_t cass64() {
		return casval<int64_t>();
	}
	WUNUSED Dee_int128_t cass128() {
		return casval<Dee_int128_t>();
	}
	WUNUSED uint8_t casu8() {
		return casval<uint8_t>();
	}
	WUNUSED uint16_t casu16() {
		return casval<uint16_t>();
	}
	WUNUSED uint32_t casu32() {
		return casval<uint32_t>();
	}
	WUNUSED uint64_t casu64() {
		return casval<uint64_t>();
	}
	WUNUSED Dee_uint128_t casu128() {
		return casval<Dee_uint128_t>();
	}
	WUNUSED size_t cassize() {
		return casval<size_t>();
	}
	WUNUSED Dee_ssize_t casssize() {
		return casval<Dee_ssize_t>();
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(int from deemon).printCxxApi(exclude: {"int"});]]]*/
	WUNUSED Ref<string> (tostr)() {
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix) {
		DeeObject *args[1];
		args[0] = radix;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision) {
		DeeObject *args[2];
		args[0] = radix;
		args[1] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision, DeeObject *mode) {
		DeeObject *args[3];
		args[0] = radix;
		args[1] = precision;
		args[2] = mode;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "oos", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" Dee_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKdSIZ Dee_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(size_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), Dee_PCKuSIZ Dee_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (hex)() {
		return inherit(DeeObject_CallAttrStringHash(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (hex)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b), 1, args));
	}
	WUNUSED Ref<string> (hex)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b), Dee_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (hex)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b), Dee_PCKuSIZ, precision));
	}
	WUNUSED Ref<string> (bin)() {
		return inherit(DeeObject_CallAttrStringHash(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (bin)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d), 1, args));
	}
	WUNUSED Ref<string> (bin)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d), Dee_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (bin)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d), Dee_PCKuSIZ, precision));
	}
	WUNUSED Ref<string> (oct)() {
		return inherit(DeeObject_CallAttrStringHash(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (oct)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407), 1, args));
	}
	WUNUSED Ref<string> (oct)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407), Dee_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (oct)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407), Dee_PCKuSIZ, precision));
	}
	WUNUSED Ref<Bytes> (tobytes)() {
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length) {
		DeeObject *args[1];
		args[0] = length;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder) {
		DeeObject *args[2];
		args[0] = length;
		args[1] = byteorder;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder, DeeObject *signed_) {
		DeeObject *args[3];
		args[0] = length;
		args[1] = byteorder;
		args[2] = signed_;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "oob", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "os", length, byteorder));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "oso", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "osb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKdSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(size_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), Dee_PCKuSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<deemon::int_> (bitcount)() {
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bitcount)(DeeObject *signed_) {
		DeeObject *args[1];
		args[0] = signed_;
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), 1, args));
	}
	WUNUSED Ref<deemon::int_> (bitcount)(bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), "b", signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (nextafter)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37), 1, args));
	}
	WUNUSED Ref<deemon::int_> (nextafter)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::int_> (nextafter)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreaterequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isless)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isless)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isless)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f), Dee_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isunordered)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), Dee_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), Dee_PCKuSIZ, y));
	}
	WUNUSED Ref<deemon::int_> (__forcecopy__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__forcecopy__", _Dee_HashSelectC(0xc96f01d0, 0xc90cb18768481fa9), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (pext)(DeeObject *mask) {
		DeeObject *args[1];
		args[0] = mask;
		return inherit(DeeObject_CallAttrStringHash(this, "pext", _Dee_HashSelectC(0xded0a982, 0xe18c6a9927db1215), 1, args));
	}
	WUNUSED Ref<deemon::int_> (pext)(Dee_ssize_t mask) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pext", _Dee_HashSelectC(0xded0a982, 0xe18c6a9927db1215), Dee_PCKdSIZ, mask));
	}
	WUNUSED Ref<deemon::int_> (pext)(size_t mask) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pext", _Dee_HashSelectC(0xded0a982, 0xe18c6a9927db1215), Dee_PCKuSIZ, mask));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (pdep)(DeeObject *mask) {
		DeeObject *args[1];
		args[0] = mask;
		return inherit(DeeObject_CallAttrStringHash(this, "pdep", _Dee_HashSelectC(0x4bc4ac00, 0x570b2c9c158bf091), 1, args));
	}
	WUNUSED Ref<deemon::int_> (pdep)(Dee_ssize_t mask) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pdep", _Dee_HashSelectC(0x4bc4ac00, 0x570b2c9c158bf091), Dee_PCKdSIZ, mask));
	}
	WUNUSED Ref<deemon::int_> (pdep)(size_t mask) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pdep", _Dee_HashSelectC(0x4bc4ac00, 0x570b2c9c158bf091), Dee_PCKuSIZ, mask));
	}
	class _Wrap___sizeof__
		: public deemon::detail::ConstGetRefProxy<_Wrap___sizeof__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___sizeof__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__sizeof__", _Dee_HashSelectC(0x422f56f1, 0x4240f7a183278760));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__sizeof__", _Dee_HashSelectC(0x422f56f1, 0x4240f7a183278760)));
		}
	};
	WUNUSED _Wrap___sizeof__ (__sizeof__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_abs
		: public deemon::detail::ConstGetRefProxy<_Wrap_abs, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_abs(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "abs", _Dee_HashSelectC(0x62947f9, 0x7ce68cbaf722015c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "abs", _Dee_HashSelectC(0x62947f9, 0x7ce68cbaf722015c)));
		}
	};
	WUNUSED _Wrap_abs (abs)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_trunc
		: public deemon::detail::ConstGetRefProxy<_Wrap_trunc, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_trunc(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "trunc", _Dee_HashSelectC(0xbdb1d95d, 0x55dde3e4f51201b3));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "trunc", _Dee_HashSelectC(0xbdb1d95d, 0x55dde3e4f51201b3)));
		}
	};
	WUNUSED _Wrap_trunc (trunc)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_floor
		: public deemon::detail::ConstGetRefProxy<_Wrap_floor, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_floor(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "floor", _Dee_HashSelectC(0x7819c83c, 0x213bace42059b42a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "floor", _Dee_HashSelectC(0x7819c83c, 0x213bace42059b42a)));
		}
	};
	WUNUSED _Wrap_floor (floor)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_ceil
		: public deemon::detail::ConstGetRefProxy<_Wrap_ceil, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ceil(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ceil", _Dee_HashSelectC(0x7ed4e8b7, 0x4ea23d981c754581));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ceil", _Dee_HashSelectC(0x7ed4e8b7, 0x4ea23d981c754581)));
		}
	};
	WUNUSED _Wrap_ceil (ceil)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_round
		: public deemon::detail::ConstGetRefProxy<_Wrap_round, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_round(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "round", _Dee_HashSelectC(0xa33127a2, 0x973d8304b01a5682));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "round", _Dee_HashSelectC(0xa33127a2, 0x973d8304b01a5682)));
		}
	};
	WUNUSED _Wrap_round (round)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isnormal
		: public deemon::detail::ConstGetRefProxy<_Wrap_isnormal, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isnormal(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isnormal", _Dee_HashSelectC(0x6a066cc5, 0x112aae54ed86881f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnormal", _Dee_HashSelectC(0x6a066cc5, 0x112aae54ed86881f)));
		}
	};
	WUNUSED _Wrap_isnormal (isnormal)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_popcount
		: public deemon::detail::ConstGetRefProxy<_Wrap_popcount, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_popcount(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "popcount", _Dee_HashSelectC(0x9fb087f4, 0x1da7fbc6c1dd86dd));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "popcount", _Dee_HashSelectC(0x9fb087f4, 0x1da7fbc6c1dd86dd)));
		}
	};
	WUNUSED _Wrap_popcount (popcount)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_ffs
		: public deemon::detail::ConstGetRefProxy<_Wrap_ffs, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ffs(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ffs", _Dee_HashSelectC(0xee0da7ec, 0xb86fa72e64f49d1e));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ffs", _Dee_HashSelectC(0xee0da7ec, 0xb86fa72e64f49d1e)));
		}
	};
	WUNUSED _Wrap_ffs (ffs)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_fls
		: public deemon::detail::ConstGetRefProxy<_Wrap_fls, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_fls(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "fls", _Dee_HashSelectC(0xc5cbbf7c, 0x60b5b1e4954fc54b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "fls", _Dee_HashSelectC(0xc5cbbf7c, 0x60b5b1e4954fc54b)));
		}
	};
	WUNUSED _Wrap_fls (fls)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_parity
		: public deemon::detail::ConstGetRefProxy<_Wrap_parity, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_parity(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28)));
		}
	};
	WUNUSED _Wrap_parity (parity)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_ctz
		: public deemon::detail::ConstGetRefProxy<_Wrap_ctz, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ctz(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ctz", _Dee_HashSelectC(0xfc21ef39, 0xefcd17594063dd43));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ctz", _Dee_HashSelectC(0xfc21ef39, 0xefcd17594063dd43)));
		}
	};
	WUNUSED _Wrap_ctz (ctz)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_ct1
		: public deemon::detail::ConstGetRefProxy<_Wrap_ct1, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ct1(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ct1", _Dee_HashSelectC(0x528dd1f6, 0x829b7dd417cecd9d));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ct1", _Dee_HashSelectC(0x528dd1f6, 0x829b7dd417cecd9d)));
		}
	};
	WUNUSED _Wrap_ct1 (ct1)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_msb
		: public deemon::detail::ConstGetRefProxy<_Wrap_msb, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_msb(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "msb", _Dee_HashSelectC(0xbe7ab876, 0xa4211b762e65939c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "msb", _Dee_HashSelectC(0xbe7ab876, 0xa4211b762e65939c)));
		}
	};
	WUNUSED _Wrap_msb (msb)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_bitmask
		: public deemon::detail::ConstGetRefProxy<_Wrap_bitmask, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_bitmask(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "bitmask", _Dee_HashSelectC(0x3a160ebf, 0x96033f145c3d0b94));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "bitmask", _Dee_HashSelectC(0x3a160ebf, 0x96033f145c3d0b94)));
		}
	};
	WUNUSED _Wrap_bitmask (bitmask)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_nth
		: public deemon::detail::ConstGetRefProxy<_Wrap_nth, string> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_nth(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "nth", _Dee_HashSelectC(0x8e4eb0c, 0x3c51c11b465d5ad5));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "nth", _Dee_HashSelectC(0x8e4eb0c, 0x3c51c11b465d5ad5)));
		}
	};
	WUNUSED _Wrap_nth (nth)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

inline WUNUSED Ref<int_> of(__INT8_TYPE__ value) {
	return inherit(DeeInt_NewInt8(value));
}
inline WUNUSED Ref<int_> of(__INT16_TYPE__ value) {
	return inherit(DeeInt_NewInt16(value));
}
inline WUNUSED Ref<int_> of(__INT32_TYPE__ value) {
	return inherit(DeeInt_NewInt32(value));
}
inline WUNUSED Ref<int_> of(__INT64_TYPE__ value) {
	return inherit(DeeInt_NewInt64(value));
}
inline WUNUSED Ref<int_> of(Dee_int128_t value) {
	return inherit(DeeInt_NewInt128(value));
}
inline WUNUSED Ref<int_> of(__UINT8_TYPE__ value) {
	return inherit(DeeInt_NewUInt8(value));
}
inline WUNUSED Ref<int_> of(__UINT16_TYPE__ value) {
	return inherit(DeeInt_NewUInt16(value));
}
inline WUNUSED Ref<int_> of(__UINT32_TYPE__ value) {
	return inherit(DeeInt_NewUInt32(value));
}
inline WUNUSED Ref<int_> of(__UINT64_TYPE__ value) {
	return inherit(DeeInt_NewUInt64(value));
}
inline WUNUSED Ref<int_> of(Dee_uint128_t value) {
	return inherit(DeeInt_NewUInt128(value));
}
#ifdef __FIFTHINT_TYPE__
inline WUNUSED Ref<int_> of(__FIFTHINT_TYPE__ value) {
	return inherit(DeeInt_NEWS(value));
}
inline WUNUSED Ref<int_> of(__UFIFTHINT_TYPE__ value) {
	return inherit(DeeInt_NEWU(value));
}
#endif /* __FIFTHINT_TYPE__ */


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_INT_H */
