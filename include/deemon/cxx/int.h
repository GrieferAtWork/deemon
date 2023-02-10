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
#ifndef GUARD_DEEMON_CXX_INT_H
#define GUARD_DEEMON_CXX_INT_H 1

#include "api.h"
/**/

#include "numeric.h"
#include "object.h"
/**/

#include "../format.h"
#include "../int.h"
#include "../system-features.h" /* strlen */
/**/

#include <hybrid/typecore.h>

DEE_CXX_BEGIN

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
		return nonnull(&DeeInt_Zero);
	}
	static WUNUSED Ref<int_> one() {
		return nonnull(&DeeInt_One);
	}
	static WUNUSED Ref<int_> minus_one() {
		return nonnull(&DeeInt_MinusOne);
	}
	static WUNUSED Ref<int_> of(__INT8_TYPE__ value) {
		return inherit(DeeInt_NewS8(value));
	}
	static WUNUSED Ref<int_> of(__INT16_TYPE__ value) {
		return inherit(DeeInt_NewS16(value));
	}
	static WUNUSED Ref<int_> of(__INT32_TYPE__ value) {
		return inherit(DeeInt_NewS32(value));
	}
	static WUNUSED Ref<int_> of(__INT64_TYPE__ value) {
		return inherit(DeeInt_NewS64(value));
	}
	static WUNUSED Ref<int_> of(Dee_int128_t value) {
		return inherit(DeeInt_NewS128(value));
	}
	static WUNUSED Ref<int_> of(__UINT8_TYPE__ value) {
		return inherit(DeeInt_NewU8(value));
	}
	static WUNUSED Ref<int_> of(__UINT16_TYPE__ value) {
		return inherit(DeeInt_NewU16(value));
	}
	static WUNUSED Ref<int_> of(__UINT32_TYPE__ value) {
		return inherit(DeeInt_NewU32(value));
	}
	static WUNUSED Ref<int_> of(__UINT64_TYPE__ value) {
		return inherit(DeeInt_NewU64(value));
	}
	static WUNUSED Ref<int_> of(Dee_uint128_t value) {
		return inherit(DeeInt_NewU128(value));
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
	                                 uint32_t radix_and_flags = DEEINT_STRING(0, DEEINT_STRING_FNORMAL)) {
		return inherit(DeeInt_FromString(str, len, radix_and_flags));
	}
	static WUNUSED Ref<int_> ofutf8(/*ascii*/ char const *__restrict str) {
		return inherit(DeeInt_FromString(str, strlen(str), DEEINT_STRING(0, DEEINT_STRING_FNORMAL)));
	}

	static WUNUSED Ref<int_> ofascii(/*ascii*/ char const *__restrict str, size_t len,
	                                 uint32_t radix_and_flags = DEEINT_STRING(0, DEEINT_STRING_FNORMAL)) {
		return inherit(DeeInt_FromAscii(str, len, radix_and_flags));
	}
	static WUNUSED Ref<int_> ofascii(/*ascii*/ char const *__restrict str) {
		return inherit(DeeInt_FromAscii(str, strlen(str), DEEINT_STRING(0, DEEINT_STRING_FNORMAL)));
	}
public:
	/* Integer conversion */
	Object &cgetval(char &value) {
		throw_if_nonzero(DeeInt_AsChar(this, &value));
		return *this;
	}
	Object &cgetval(signed char &value) {
		throw_if_nonzero(DeeInt_AsSChar(this, &value));
		return *this;
	}
	Object &cgetval(unsigned char &value) {
		throw_if_nonzero(DeeInt_AsUChar(this, &value));
		return *this;
	}
	Object &cgetval(short &value) {
		throw_if_nonzero(DeeInt_AsShort(this, &value));
		return *this;
	}
	Object &cgetval(unsigned short &value) {
		throw_if_nonzero(DeeInt_AsUShort(this, &value));
		return *this;
	}
	Object &cgetval(int &value) {
		throw_if_nonzero(DeeInt_AsInt(this, &value));
		return *this;
	}
	Object &cgetval(unsigned int &value) {
		throw_if_nonzero(DeeInt_AsUInt(this, &value));
		return *this;
	}
	Object &cgetval(long &value) {
		throw_if_nonzero(DeeInt_AsLong(this, &value));
		return *this;
	}
	Object &cgetval(unsigned long &value) {
		throw_if_nonzero(DeeInt_AsULong(this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	Object &cgetval(__LONGLONG &value) {
		throw_if_nonzero(DeeInt_AsLLong(this, &value));
		return *this;
	}
	Object &cgetval(__ULONGLONG &value) {
		throw_if_nonzero(DeeInt_AsULLong(this, &value));
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	Object &cgetval(Dee_int128_t &value) {
		throw_if_nonzero(DeeInt_AsS128(this, &value));
		return *this;
	}
	Object &cgetval(Dee_uint128_t &value) {
		throw_if_nonzero(DeeInt_AsU128(this, &value));
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
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix) {
		DeeObject *args[1];
		args[0] = radix;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision) {
		DeeObject *args[2];
		args[0] = radix;
		args[1] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision, DeeObject *mode) {
		DeeObject *args[3];
		args[0] = radix;
		args[1] = precision;
		args[2] = mode;
		return inherit(DeeObject_CallAttrStringHash(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<string> (tostr)(DeeObject *radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "oos", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "os", radix, precision));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "o" DEE_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ "s", radix, precision));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKdSIZ DEE_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(char const *radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)), "s", radix));
	}
	WUNUSED Ref<string> (tostr)(size_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(size_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ "s", radix, precision));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelect(UINT32_C(0x6b502fcb), UINT64_C(0x9cdf9482b472ce73)),  DEE_PCKuSIZ DEE_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (hex)() {
		return inherit(DeeObject_CallAttrStringHash(this, "hex", _Dee_HashSelect(UINT32_C(0x82a9ef3f), UINT64_C(0x5b1dd33b4f09e41b)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (hex)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "hex", _Dee_HashSelect(UINT32_C(0x82a9ef3f), UINT64_C(0x5b1dd33b4f09e41b)), 1, args));
	}
	WUNUSED Ref<string> (hex)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelect(UINT32_C(0x82a9ef3f), UINT64_C(0x5b1dd33b4f09e41b)),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (hex)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelect(UINT32_C(0x82a9ef3f), UINT64_C(0x5b1dd33b4f09e41b)),  DEE_PCKuSIZ, precision));
	}
	WUNUSED Ref<string> (bin)() {
		return inherit(DeeObject_CallAttrStringHash(this, "bin", _Dee_HashSelect(UINT32_C(0x5a5ee1b4), UINT64_C(0xcd6ee112a9d2e67d)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (bin)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "bin", _Dee_HashSelect(UINT32_C(0x5a5ee1b4), UINT64_C(0xcd6ee112a9d2e67d)), 1, args));
	}
	WUNUSED Ref<string> (bin)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelect(UINT32_C(0x5a5ee1b4), UINT64_C(0xcd6ee112a9d2e67d)),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (bin)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelect(UINT32_C(0x5a5ee1b4), UINT64_C(0xcd6ee112a9d2e67d)),  DEE_PCKuSIZ, precision));
	}
	WUNUSED Ref<string> (oct)() {
		return inherit(DeeObject_CallAttrStringHash(this, "oct", _Dee_HashSelect(UINT32_C(0xed7fe6af), UINT64_C(0x84205b18ca702407)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (oct)(DeeObject *precision) {
		DeeObject *args[1];
		args[0] = precision;
		return inherit(DeeObject_CallAttrStringHash(this, "oct", _Dee_HashSelect(UINT32_C(0xed7fe6af), UINT64_C(0x84205b18ca702407)), 1, args));
	}
	WUNUSED Ref<string> (oct)(Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelect(UINT32_C(0xed7fe6af), UINT64_C(0x84205b18ca702407)),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (oct)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelect(UINT32_C(0xed7fe6af), UINT64_C(0x84205b18ca702407)),  DEE_PCKuSIZ, precision));
	}
	WUNUSED Ref<Bytes> (tobytes)() {
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length) {
		DeeObject *args[1];
		args[0] = length;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder) {
		DeeObject *args[2];
		args[0] = length;
		args[1] = byteorder;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder, DeeObject *signed_) {
		DeeObject *args[3];
		args[0] = length;
		args[1] = byteorder;
		args[2] = signed_;
		return inherit(DeeObject_CallAttrStringHash(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (tobytes)(DeeObject *length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "oob", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "ob", length, byteorder));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "os", length, byteorder));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "oso", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "osb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "b", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKdSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(bool length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "b", length));
	}
	WUNUSED Ref<Bytes> (tobytes)(char const *length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "s", length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(char const *length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "so", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(char const *length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)), "sb", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "b", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(size_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelect(UINT32_C(0xb72dac45), UINT64_C(0x2c82795d5b9c763d)),  DEE_PCKuSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<deemon::int_> (bitcount)() {
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelect(UINT32_C(0x4cacc37f), UINT64_C(0x8e82ba8252728a35)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bitcount)(DeeObject *signed_) {
		DeeObject *args[1];
		args[0] = signed_;
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelect(UINT32_C(0x4cacc37f), UINT64_C(0x8e82ba8252728a35)), 1, args));
	}
	WUNUSED Ref<deemon::int_> (bitcount)(bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bitcount", _Dee_HashSelect(UINT32_C(0x4cacc37f), UINT64_C(0x8e82ba8252728a35)), "b", signed_));
	}
	WUNUSED Ref<deemon::int_> (__forcecopy__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__forcecopy__", _Dee_HashSelect(UINT32_C(0xc96f01d0), UINT64_C(0xc90cb18768481fa9)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "divmod", _Dee_HashSelect(UINT32_C(0xabe5175b), UINT64_C(0x56056310297339c6)), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelect(UINT32_C(0xabe5175b), UINT64_C(0x56056310297339c6)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<_AbstractTuple<deemon::int_, deemon::int_> > (divmod)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelect(UINT32_C(0xabe5175b), UINT64_C(0x56056310297339c6)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (nextafter)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "nextafter", _Dee_HashSelect(UINT32_C(0xe1e4632d), UINT64_C(0xdb946f5aa5012d37)), 1, args));
	}
	WUNUSED Ref<deemon::int_> (nextafter)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelect(UINT32_C(0xe1e4632d), UINT64_C(0xdb946f5aa5012d37)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::int_> (nextafter)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelect(UINT32_C(0xe1e4632d), UINT64_C(0xdb946f5aa5012d37)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreater", _Dee_HashSelect(UINT32_C(0x4d131c9), UINT64_C(0x7d14fd652371de34)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelect(UINT32_C(0x4d131c9), UINT64_C(0x7d14fd652371de34)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelect(UINT32_C(0x4d131c9), UINT64_C(0x7d14fd652371de34)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreaterequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreaterequal", _Dee_HashSelect(UINT32_C(0x47c340b), UINT64_C(0xf1cf45f4551813b4)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelect(UINT32_C(0x47c340b), UINT64_C(0xf1cf45f4551813b4)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelect(UINT32_C(0x47c340b), UINT64_C(0xf1cf45f4551813b4)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isless)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isless", _Dee_HashSelect(UINT32_C(0xc093e250), UINT64_C(0x63ee6983aba9e389)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isless)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelect(UINT32_C(0xc093e250), UINT64_C(0x63ee6983aba9e389)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isless)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelect(UINT32_C(0xc093e250), UINT64_C(0x63ee6983aba9e389)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessequal", _Dee_HashSelect(UINT32_C(0x3f6c6174), UINT64_C(0xfaec7676fdaa8cc3)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelect(UINT32_C(0x3f6c6174), UINT64_C(0xfaec7676fdaa8cc3)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelect(UINT32_C(0x3f6c6174), UINT64_C(0xfaec7676fdaa8cc3)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessgreater", _Dee_HashSelect(UINT32_C(0xfc1ab689), UINT64_C(0x16b5b6d91d66c88f)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelect(UINT32_C(0xfc1ab689), UINT64_C(0x16b5b6d91d66c88f)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelect(UINT32_C(0xfc1ab689), UINT64_C(0x16b5b6d91d66c88f)),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isunordered)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isunordered", _Dee_HashSelect(UINT32_C(0x3907db0d), UINT64_C(0xb0d4b15ee21e7ffb)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelect(UINT32_C(0x3907db0d), UINT64_C(0xb0d4b15ee21e7ffb)),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelect(UINT32_C(0x3907db0d), UINT64_C(0xb0d4b15ee21e7ffb)), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelect(UINT32_C(0x3907db0d), UINT64_C(0xb0d4b15ee21e7ffb)),  DEE_PCKuSIZ, y));
	}
	class _Wrap___sizeof__
		: public deemon::detail::ConstGetRefProxy<_Wrap___sizeof__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___sizeof__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__sizeof__", _Dee_HashSelect(UINT32_C(0x422f56f1), UINT64_C(0x4240f7a183278760)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__sizeof__", _Dee_HashSelect(UINT32_C(0x422f56f1), UINT64_C(0x4240f7a183278760))));
		}
	};
	WUNUSED _Wrap___sizeof__ (__sizeof__)() {
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
			return DeeObject_GetAttrStringHash(m_self, "abs", _Dee_HashSelect(UINT32_C(0x62947f9), UINT64_C(0x7ce68cbaf722015c)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "abs", _Dee_HashSelect(UINT32_C(0x62947f9), UINT64_C(0x7ce68cbaf722015c))));
		}
	};
	WUNUSED _Wrap_abs (abs)() {
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
			return DeeObject_GetAttrStringHash(m_self, "trunc", _Dee_HashSelect(UINT32_C(0xbdb1d95d), UINT64_C(0x55dde3e4f51201b3)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "trunc", _Dee_HashSelect(UINT32_C(0xbdb1d95d), UINT64_C(0x55dde3e4f51201b3))));
		}
	};
	WUNUSED _Wrap_trunc (trunc)() {
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
			return DeeObject_GetAttrStringHash(m_self, "floor", _Dee_HashSelect(UINT32_C(0x7819c83c), UINT64_C(0x213bace42059b42a)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "floor", _Dee_HashSelect(UINT32_C(0x7819c83c), UINT64_C(0x213bace42059b42a))));
		}
	};
	WUNUSED _Wrap_floor (floor)() {
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
			return DeeObject_GetAttrStringHash(m_self, "ceil", _Dee_HashSelect(UINT32_C(0x7ed4e8b7), UINT64_C(0x4ea23d981c754581)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ceil", _Dee_HashSelect(UINT32_C(0x7ed4e8b7), UINT64_C(0x4ea23d981c754581))));
		}
	};
	WUNUSED _Wrap_ceil (ceil)() {
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
			return DeeObject_GetAttrStringHash(m_self, "round", _Dee_HashSelect(UINT32_C(0xa33127a2), UINT64_C(0x973d8304b01a5682)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "round", _Dee_HashSelect(UINT32_C(0xa33127a2), UINT64_C(0x973d8304b01a5682))));
		}
	};
	WUNUSED _Wrap_round (round)() {
		return this;
	}
	class _Wrap_isnan
		: public deemon::detail::ConstGetRefProxy<_Wrap_isnan, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isnan(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isnan", _Dee_HashSelect(UINT32_C(0x36d136ac), UINT64_C(0x428ff2e9a35f402a)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnan", _Dee_HashSelect(UINT32_C(0x36d136ac), UINT64_C(0x428ff2e9a35f402a))));
		}
	};
	WUNUSED _Wrap_isnan (isnan)() {
		return this;
	}
	class _Wrap_isinf
		: public deemon::detail::ConstGetRefProxy<_Wrap_isinf, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isinf(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isinf", _Dee_HashSelect(UINT32_C(0xc8a63b33), UINT64_C(0xf5f86dfadcc14b6a)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isinf", _Dee_HashSelect(UINT32_C(0xc8a63b33), UINT64_C(0xf5f86dfadcc14b6a))));
		}
	};
	WUNUSED _Wrap_isinf (isinf)() {
		return this;
	}
	class _Wrap_isfinite
		: public deemon::detail::ConstGetRefProxy<_Wrap_isfinite, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isfinite(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isfinite", _Dee_HashSelect(UINT32_C(0x797c83cf), UINT64_C(0x9838bbd14d676c85)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isfinite", _Dee_HashSelect(UINT32_C(0x797c83cf), UINT64_C(0x9838bbd14d676c85))));
		}
	};
	WUNUSED _Wrap_isfinite (isfinite)() {
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
			return DeeObject_GetAttrStringHash(m_self, "isnormal", _Dee_HashSelect(UINT32_C(0x6a066cc5), UINT64_C(0x112aae54ed86881f)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnormal", _Dee_HashSelect(UINT32_C(0x6a066cc5), UINT64_C(0x112aae54ed86881f))));
		}
	};
	WUNUSED _Wrap_isnormal (isnormal)() {
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
			return DeeObject_GetAttrStringHash(m_self, "popcount", _Dee_HashSelect(UINT32_C(0x9fb087f4), UINT64_C(0x1da7fbc6c1dd86dd)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "popcount", _Dee_HashSelect(UINT32_C(0x9fb087f4), UINT64_C(0x1da7fbc6c1dd86dd))));
		}
	};
	WUNUSED _Wrap_popcount (popcount)() {
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
			return DeeObject_GetAttrStringHash(m_self, "ffs", _Dee_HashSelect(UINT32_C(0xee0da7ec), UINT64_C(0xb86fa72e64f49d1e)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ffs", _Dee_HashSelect(UINT32_C(0xee0da7ec), UINT64_C(0xb86fa72e64f49d1e))));
		}
	};
	WUNUSED _Wrap_ffs (ffs)() {
		return this;
	}
	class _Wrap_partity
		: public deemon::detail::ConstGetRefProxy<_Wrap_partity, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_partity(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "partity", _Dee_HashSelect(UINT32_C(0x2e58034c), UINT64_C(0x667c0ca8cad072de)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "partity", _Dee_HashSelect(UINT32_C(0x2e58034c), UINT64_C(0x667c0ca8cad072de))));
		}
	};
	WUNUSED _Wrap_partity (partity)() {
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
			return DeeObject_GetAttrStringHash(m_self, "ctz", _Dee_HashSelect(UINT32_C(0xfc21ef39), UINT64_C(0xefcd17594063dd43)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ctz", _Dee_HashSelect(UINT32_C(0xfc21ef39), UINT64_C(0xefcd17594063dd43))));
		}
	};
	WUNUSED _Wrap_ctz (ctz)() {
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
			return DeeObject_GetAttrStringHash(m_self, "msb", _Dee_HashSelect(UINT32_C(0xbe7ab876), UINT64_C(0xa4211b762e65939c)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "msb", _Dee_HashSelect(UINT32_C(0xbe7ab876), UINT64_C(0xa4211b762e65939c))));
		}
	};
	WUNUSED _Wrap_msb (msb)() {
		return this;
	}
/*[[[end]]]*/
};

inline WUNUSED Ref<int_> of(__INT8_TYPE__ value) {
	return inherit(DeeInt_NewS8(value));
}
inline WUNUSED Ref<int_> of(__INT16_TYPE__ value) {
	return inherit(DeeInt_NewS16(value));
}
inline WUNUSED Ref<int_> of(__INT32_TYPE__ value) {
	return inherit(DeeInt_NewS32(value));
}
inline WUNUSED Ref<int_> of(__INT64_TYPE__ value) {
	return inherit(DeeInt_NewS64(value));
}
inline WUNUSED Ref<int_> of(Dee_int128_t value) {
	return inherit(DeeInt_NewS128(value));
}
inline WUNUSED Ref<int_> of(__UINT8_TYPE__ value) {
	return inherit(DeeInt_NewU8(value));
}
inline WUNUSED Ref<int_> of(__UINT16_TYPE__ value) {
	return inherit(DeeInt_NewU16(value));
}
inline WUNUSED Ref<int_> of(__UINT32_TYPE__ value) {
	return inherit(DeeInt_NewU32(value));
}
inline WUNUSED Ref<int_> of(__UINT64_TYPE__ value) {
	return inherit(DeeInt_NewU64(value));
}
inline WUNUSED Ref<int_> of(Dee_uint128_t value) {
	return inherit(DeeInt_NewU128(value));
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
