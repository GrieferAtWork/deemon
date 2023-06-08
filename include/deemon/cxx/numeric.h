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
#ifndef GUARD_DEEMON_CXX_NUMERIC_H
#define GUARD_DEEMON_CXX_NUMERIC_H 1

#include "api.h"
/**/

#include "object.h"
/**/

#include "../numeric.h"

DEE_CXX_BEGIN

class Numeric
	: public Object
	, public detail::MathProxyAccessor<Numeric, Numeric>
{
public:
	using detail::MathProxyAccessor<Numeric, Numeric>::inv;
	using detail::MathProxyAccessor<Numeric, Numeric>::pos;
	using detail::MathProxyAccessor<Numeric, Numeric>::neg;
	using detail::MathProxyAccessor<Numeric, Numeric>::add;
	using detail::MathProxyAccessor<Numeric, Numeric>::sub;
	using detail::MathProxyAccessor<Numeric, Numeric>::mul;
	using detail::MathProxyAccessor<Numeric, Numeric>::div;
	using detail::MathProxyAccessor<Numeric, Numeric>::mod;
	using detail::MathProxyAccessor<Numeric, Numeric>::shl;
	using detail::MathProxyAccessor<Numeric, Numeric>::shr;
	using detail::MathProxyAccessor<Numeric, Numeric>::and_;
	using detail::MathProxyAccessor<Numeric, Numeric>::or_;
	using detail::MathProxyAccessor<Numeric, Numeric>::xor_;
	using detail::MathProxyAccessor<Numeric, Numeric>::pow;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator~;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator+;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator-;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator*;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator/;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator%;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator<<;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator>>;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator&;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator|;
	using detail::MathProxyAccessor<Numeric, Numeric>::operator^;
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeNumeric_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeNumeric_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeNumeric_Type);
	}
/*[[[deemon (CxxType from rt.gen.cxxapi)(Numeric from deemon).printCxxApi();]]]*/
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
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "os", radix, precision));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<string> (tostr)(DeeObject *radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (tostr)(DeeObject *radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "o" DEE_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(Dee_ssize_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ "s", radix, precision));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(Dee_ssize_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKdSIZ DEE_PCKuSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(char const *radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73), "s", radix));
	}
	WUNUSED Ref<string> (tostr)(size_t radix) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ, radix));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ "o", radix, precision));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<string> (tostr)(size_t radix, DeeObject *precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ "oo", radix, precision, mode));
	}
	WUNUSED NONNULL_CXX((2)) Ref<string> (tostr)(size_t radix, DeeObject *precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ "os", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKdSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKdSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, Dee_ssize_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKdSIZ "s", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, char const *precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ "s", radix, precision));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKuSIZ, radix, precision));
	}
	WUNUSED NONNULL_CXX((3)) Ref<string> (tostr)(size_t radix, size_t precision, DeeObject *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKuSIZ "o", radix, precision, mode));
	}
	WUNUSED Ref<string> (tostr)(size_t radix, size_t precision, char const *mode) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tostr", _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73),  DEE_PCKuSIZ DEE_PCKuSIZ "s", radix, precision, mode));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (hex)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "hex", _Dee_HashSelectC(0x82a9ef3f, 0x5b1dd33b4f09e41b),  DEE_PCKuSIZ, precision));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (bin)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bin", _Dee_HashSelectC(0x5a5ee1b4, 0xcd6ee112a9d2e67d),  DEE_PCKuSIZ, precision));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407),  DEE_PCKdSIZ, precision));
	}
	WUNUSED Ref<string> (oct)(size_t precision) {
		return inherit(DeeObject_CallAttrStringHashf(this, "oct", _Dee_HashSelectC(0xed7fe6af, 0x84205b18ca702407),  DEE_PCKuSIZ, precision));
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
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (tobytes)(DeeObject *length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "ob", length, byteorder));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(Dee_ssize_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "b", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(Dee_ssize_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKdSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(bool length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "b", length));
	}
	WUNUSED Ref<Bytes> (tobytes)(char const *length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "s", length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(char const *length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "so", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(char const *length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "sb", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(double length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "f", length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(double length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fo", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(double length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "foo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(double length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(double length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fb", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(double length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fs", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(double length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fso", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(double length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d), "fsb", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "o", length, byteorder));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "oo", length, byteorder, signed_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (tobytes)(size_t length, DeeObject *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "ob", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, bool byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "b", length, byteorder));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "s", length, byteorder));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (tobytes)(size_t length, char const *byteorder, DeeObject *signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "so", length, byteorder, signed_));
	}
	WUNUSED Ref<Bytes> (tobytes)(size_t length, char const *byteorder, bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "tobytes", _Dee_HashSelectC(0xb72dac45, 0x2c82795d5b9c763d),  DEE_PCKuSIZ "sb", length, byteorder, signed_));
	}
	WUNUSED Ref<Numeric> (bitcount)() {
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Numeric> (bitcount)(DeeObject *signed_) {
		DeeObject *args[1];
		args[0] = signed_;
		return inherit(DeeObject_CallAttrStringHash(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), 1, args));
	}
	WUNUSED Ref<Numeric> (bitcount)(bool signed_) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bitcount", _Dee_HashSelectC(0x4cacc37f, 0x8e82ba8252728a35), "b", signed_));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Numeric, Numeric> > (divmod)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<Numeric, Numeric> > (divmod)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<_AbstractTuple<Numeric, Numeric> > (divmod)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6), "f", y));
	}
	WUNUSED Ref<_AbstractTuple<Numeric, Numeric> > (divmod)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "divmod", _Dee_HashSelectC(0xabe5175b, 0x56056310297339c6),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Numeric> (nextafter)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37), 1, args));
	}
	WUNUSED Ref<Numeric> (nextafter)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<Numeric> (nextafter)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37), "f", y));
	}
	WUNUSED Ref<Numeric> (nextafter)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelectC(0xe1e4632d, 0xdb946f5aa5012d37),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelectC(0x4d131c9, 0x7d14fd652371de34),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreaterequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelectC(0x47c340b, 0xf1cf45f4551813b4),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isless)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isless)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isless)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isless)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelectC(0xc093e250, 0x63ee6983aba9e389),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelectC(0x3f6c6174, 0xfaec7676fdaa8cc3),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelectC(0xfc1ab689, 0x16b5b6d91d66c88f),  DEE_PCKuSIZ, y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isunordered)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(Dee_ssize_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb),  DEE_PCKdSIZ, y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb), "f", y));
	}
	WUNUSED Ref<deemon::bool_> (isunordered)(size_t y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isunordered", _Dee_HashSelectC(0x3907db0d, 0xb0d4b15ee21e7ffb),  DEE_PCKuSIZ, y));
	}
	class _Wrap_int_
		: public deemon::detail::ConstGetRefProxy<_Wrap_int_, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_int_(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "int", _Dee_HashSelectC(0xce831ddf, 0xb7ad4ebe928a1ef0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "int", _Dee_HashSelectC(0xce831ddf, 0xb7ad4ebe928a1ef0)));
		}
	};
	WUNUSED _Wrap_int_ (int_)() {
		return this;
	}
	class _Wrap_float_
		: public deemon::detail::ConstGetRefProxy<_Wrap_float_, deemon::float_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_float_(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "float", _Dee_HashSelectC(0x95fb9fe8, 0x19ab2ca7919bffe4));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "float", _Dee_HashSelectC(0x95fb9fe8, 0x19ab2ca7919bffe4)));
		}
	};
	WUNUSED _Wrap_float_ (float_)() {
		return this;
	}
	class _Wrap_s8
		: public deemon::detail::ConstGetRefProxy<_Wrap_s8, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_s8(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "s8", _Dee_HashSelectC(0x8e91d8aa, 0x2c22b46392684555));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s8", _Dee_HashSelectC(0x8e91d8aa, 0x2c22b46392684555)));
		}
	};
	WUNUSED _Wrap_s8 (s8)() {
		return this;
	}
	class _Wrap_s16
		: public deemon::detail::ConstGetRefProxy<_Wrap_s16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_s16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "s16", _Dee_HashSelectC(0xdb5b7281, 0x93349818f68f2f79));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s16", _Dee_HashSelectC(0xdb5b7281, 0x93349818f68f2f79)));
		}
	};
	WUNUSED _Wrap_s16 (s16)() {
		return this;
	}
	class _Wrap_s32
		: public deemon::detail::ConstGetRefProxy<_Wrap_s32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_s32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "s32", _Dee_HashSelectC(0x86be9ab2, 0x113edb000463b661));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s32", _Dee_HashSelectC(0x86be9ab2, 0x113edb000463b661)));
		}
	};
	WUNUSED _Wrap_s32 (s32)() {
		return this;
	}
	class _Wrap_s64
		: public deemon::detail::ConstGetRefProxy<_Wrap_s64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_s64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "s64", _Dee_HashSelectC(0x8c789c8a, 0xc14d2d01ad1298ff));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s64", _Dee_HashSelectC(0x8c789c8a, 0xc14d2d01ad1298ff)));
		}
	};
	WUNUSED _Wrap_s64 (s64)() {
		return this;
	}
	class _Wrap_s128
		: public deemon::detail::ConstGetRefProxy<_Wrap_s128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_s128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "s128", _Dee_HashSelectC(0xc961f8bf, 0xd26700f831122265));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s128", _Dee_HashSelectC(0xc961f8bf, 0xd26700f831122265)));
		}
	};
	WUNUSED _Wrap_s128 (s128)() {
		return this;
	}
	class _Wrap_u8
		: public deemon::detail::ConstGetRefProxy<_Wrap_u8, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_u8(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "u8", _Dee_HashSelectC(0x655c23a2, 0x98b4b9518f3f5701));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u8", _Dee_HashSelectC(0x655c23a2, 0x98b4b9518f3f5701)));
		}
	};
	WUNUSED _Wrap_u8 (u8)() {
		return this;
	}
	class _Wrap_u16
		: public deemon::detail::ConstGetRefProxy<_Wrap_u16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_u16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "u16", _Dee_HashSelectC(0x4b025b59, 0x14d56899210e228d));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u16", _Dee_HashSelectC(0x4b025b59, 0x14d56899210e228d)));
		}
	};
	WUNUSED _Wrap_u16 (u16)() {
		return this;
	}
	class _Wrap_u32
		: public deemon::detail::ConstGetRefProxy<_Wrap_u32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_u32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "u32", _Dee_HashSelectC(0x102d3993, 0x3864575086dc2c74));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u32", _Dee_HashSelectC(0x102d3993, 0x3864575086dc2c74)));
		}
	};
	WUNUSED _Wrap_u32 (u32)() {
		return this;
	}
	class _Wrap_u64
		: public deemon::detail::ConstGetRefProxy<_Wrap_u64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_u64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "u64", _Dee_HashSelectC(0x76d7d162, 0x427ddf7b27d8d4c6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u64", _Dee_HashSelectC(0x76d7d162, 0x427ddf7b27d8d4c6)));
		}
	};
	WUNUSED _Wrap_u64 (u64)() {
		return this;
	}
	class _Wrap_u128
		: public deemon::detail::ConstGetRefProxy<_Wrap_u128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_u128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "u128", _Dee_HashSelectC(0x9d0ba7ea, 0x963f3f15e92b1b7c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u128", _Dee_HashSelectC(0x9d0ba7ea, 0x963f3f15e92b1b7c)));
		}
	};
	WUNUSED _Wrap_u128 (u128)() {
		return this;
	}
	class _Wrap_signed8
		: public deemon::detail::ConstGetRefProxy<_Wrap_signed8, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_signed8(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "signed8", _Dee_HashSelectC(0x539183ff, 0x630818c691a604db));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed8", _Dee_HashSelectC(0x539183ff, 0x630818c691a604db)));
		}
	};
	WUNUSED _Wrap_signed8 (signed8)() {
		return this;
	}
	class _Wrap_signed16
		: public deemon::detail::ConstGetRefProxy<_Wrap_signed16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_signed16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "signed16", _Dee_HashSelectC(0x39a8ee85, 0xc9dba3957529c8e6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed16", _Dee_HashSelectC(0x39a8ee85, 0xc9dba3957529c8e6)));
		}
	};
	WUNUSED _Wrap_signed16 (signed16)() {
		return this;
	}
	class _Wrap_signed32
		: public deemon::detail::ConstGetRefProxy<_Wrap_signed32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_signed32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "signed32", _Dee_HashSelectC(0x401ad7d8, 0x8ecce67a523a26b6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed32", _Dee_HashSelectC(0x401ad7d8, 0x8ecce67a523a26b6)));
		}
	};
	WUNUSED _Wrap_signed32 (signed32)() {
		return this;
	}
	class _Wrap_signed64
		: public deemon::detail::ConstGetRefProxy<_Wrap_signed64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_signed64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "signed64", _Dee_HashSelectC(0xd31a2a48, 0x657452d59b772de9));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed64", _Dee_HashSelectC(0xd31a2a48, 0x657452d59b772de9)));
		}
	};
	WUNUSED _Wrap_signed64 (signed64)() {
		return this;
	}
	class _Wrap_signed128
		: public deemon::detail::ConstGetRefProxy<_Wrap_signed128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_signed128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "signed128", _Dee_HashSelectC(0xfd8c1f06, 0x9bbd99d8421a094d));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed128", _Dee_HashSelectC(0xfd8c1f06, 0x9bbd99d8421a094d)));
		}
	};
	WUNUSED _Wrap_signed128 (signed128)() {
		return this;
	}
	class _Wrap_unsigned8
		: public deemon::detail::ConstGetRefProxy<_Wrap_unsigned8, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_unsigned8(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "unsigned8", _Dee_HashSelectC(0xaef1bad7, 0x9e8b78fba41919c5));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned8", _Dee_HashSelectC(0xaef1bad7, 0x9e8b78fba41919c5)));
		}
	};
	WUNUSED _Wrap_unsigned8 (unsigned8)() {
		return this;
	}
	class _Wrap_unsigned16
		: public deemon::detail::ConstGetRefProxy<_Wrap_unsigned16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_unsigned16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "unsigned16", _Dee_HashSelectC(0xa8fca18e, 0xea7c13aabc6570c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned16", _Dee_HashSelectC(0xa8fca18e, 0xea7c13aabc6570c)));
		}
	};
	WUNUSED _Wrap_unsigned16 (unsigned16)() {
		return this;
	}
	class _Wrap_unsigned32
		: public deemon::detail::ConstGetRefProxy<_Wrap_unsigned32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_unsigned32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "unsigned32", _Dee_HashSelectC(0xc023b118, 0x2864fa6210c799af));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned32", _Dee_HashSelectC(0xc023b118, 0x2864fa6210c799af)));
		}
	};
	WUNUSED _Wrap_unsigned32 (unsigned32)() {
		return this;
	}
	class _Wrap_unsigned64
		: public deemon::detail::ConstGetRefProxy<_Wrap_unsigned64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_unsigned64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "unsigned64", _Dee_HashSelectC(0x1d42967f, 0x5ee69a5d04e815db));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned64", _Dee_HashSelectC(0x1d42967f, 0x5ee69a5d04e815db)));
		}
	};
	WUNUSED _Wrap_unsigned64 (unsigned64)() {
		return this;
	}
	class _Wrap_unsigned128
		: public deemon::detail::ConstGetRefProxy<_Wrap_unsigned128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_unsigned128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "unsigned128", _Dee_HashSelectC(0x493baf7d, 0xd7193bca5b808b24));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned128", _Dee_HashSelectC(0x493baf7d, 0xd7193bca5b808b24)));
		}
	};
	WUNUSED _Wrap_unsigned128 (unsigned128)() {
		return this;
	}
	class _Wrap_swap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_swap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_swap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "swap16", _Dee_HashSelectC(0x2f6792b, 0x7b6d7b73549438e6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap16", _Dee_HashSelectC(0x2f6792b, 0x7b6d7b73549438e6)));
		}
	};
	WUNUSED _Wrap_swap16 (swap16)() {
		return this;
	}
	class _Wrap_swap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_swap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_swap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "swap32", _Dee_HashSelectC(0x57c6852f, 0x1451b87dc0b31866));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap32", _Dee_HashSelectC(0x57c6852f, 0x1451b87dc0b31866)));
		}
	};
	WUNUSED _Wrap_swap32 (swap32)() {
		return this;
	}
	class _Wrap_swap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_swap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_swap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "swap64", _Dee_HashSelectC(0x8208c247, 0xadc0d9b179945b41));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap64", _Dee_HashSelectC(0x8208c247, 0xadc0d9b179945b41)));
		}
	};
	WUNUSED _Wrap_swap64 (swap64)() {
		return this;
	}
	class _Wrap_swap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_swap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_swap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "swap128", _Dee_HashSelectC(0x5e498c0e, 0x68da9a8bb7f4ffa8));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap128", _Dee_HashSelectC(0x5e498c0e, 0x68da9a8bb7f4ffa8)));
		}
	};
	WUNUSED _Wrap_swap128 (swap128)() {
		return this;
	}
	class _Wrap_sswap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_sswap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_sswap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "sswap16", _Dee_HashSelectC(0xfe35af66, 0xc653941ca804b260));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap16", _Dee_HashSelectC(0xfe35af66, 0xc653941ca804b260)));
		}
	};
	WUNUSED _Wrap_sswap16 (sswap16)() {
		return this;
	}
	class _Wrap_sswap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_sswap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_sswap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "sswap32", _Dee_HashSelectC(0x5ea6d95a, 0xd2c67c37993fd04));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap32", _Dee_HashSelectC(0x5ea6d95a, 0xd2c67c37993fd04)));
		}
	};
	WUNUSED _Wrap_sswap32 (sswap32)() {
		return this;
	}
	class _Wrap_sswap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_sswap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_sswap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "sswap64", _Dee_HashSelectC(0x8367b75d, 0x6154dad8a5a4d002));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap64", _Dee_HashSelectC(0x8367b75d, 0x6154dad8a5a4d002)));
		}
	};
	WUNUSED _Wrap_sswap64 (sswap64)() {
		return this;
	}
	class _Wrap_sswap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_sswap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_sswap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "sswap128", _Dee_HashSelectC(0x99599142, 0x542a3a988d3d57c0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap128", _Dee_HashSelectC(0x99599142, 0x542a3a988d3d57c0)));
		}
	};
	WUNUSED _Wrap_sswap128 (sswap128)() {
		return this;
	}
	class _Wrap_leswap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_leswap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_leswap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "leswap16", _Dee_HashSelectC(0x487e8219, 0xd0682b67be9aa983));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap16", _Dee_HashSelectC(0x487e8219, 0xd0682b67be9aa983)));
		}
	};
	WUNUSED _Wrap_leswap16 (leswap16)() {
		return this;
	}
	class _Wrap_leswap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_leswap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_leswap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "leswap32", _Dee_HashSelectC(0x3c086c2d, 0xf0151857d7ce0991));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap32", _Dee_HashSelectC(0x3c086c2d, 0xf0151857d7ce0991)));
		}
	};
	WUNUSED _Wrap_leswap32 (leswap32)() {
		return this;
	}
	class _Wrap_leswap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_leswap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_leswap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "leswap64", _Dee_HashSelectC(0x40aa150e, 0xfd67edaf1f992f3f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap64", _Dee_HashSelectC(0x40aa150e, 0xfd67edaf1f992f3f)));
		}
	};
	WUNUSED _Wrap_leswap64 (leswap64)() {
		return this;
	}
	class _Wrap_leswap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_leswap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_leswap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "leswap128", _Dee_HashSelectC(0x98fe10ba, 0xdfa667bb8a2a91f5));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap128", _Dee_HashSelectC(0x98fe10ba, 0xdfa667bb8a2a91f5)));
		}
	};
	WUNUSED _Wrap_leswap128 (leswap128)() {
		return this;
	}
	class _Wrap_beswap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_beswap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_beswap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "beswap16", _Dee_HashSelectC(0x32de9fb3, 0x305251e9958a4fd6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap16", _Dee_HashSelectC(0x32de9fb3, 0x305251e9958a4fd6)));
		}
	};
	WUNUSED _Wrap_beswap16 (beswap16)() {
		return this;
	}
	class _Wrap_beswap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_beswap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_beswap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "beswap32", _Dee_HashSelectC(0x8c523765, 0xd715948923e8ba9a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap32", _Dee_HashSelectC(0x8c523765, 0xd715948923e8ba9a)));
		}
	};
	WUNUSED _Wrap_beswap32 (beswap32)() {
		return this;
	}
	class _Wrap_beswap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_beswap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_beswap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "beswap64", _Dee_HashSelectC(0xc9db181, 0xbe05bbffa5c0b071));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap64", _Dee_HashSelectC(0xc9db181, 0xbe05bbffa5c0b071)));
		}
	};
	WUNUSED _Wrap_beswap64 (beswap64)() {
		return this;
	}
	class _Wrap_beswap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_beswap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_beswap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "beswap128", _Dee_HashSelectC(0x11b2e1b9, 0x7ecdfc744007590f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap128", _Dee_HashSelectC(0x11b2e1b9, 0x7ecdfc744007590f)));
		}
	};
	WUNUSED _Wrap_beswap128 (beswap128)() {
		return this;
	}
	class _Wrap_lesswap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_lesswap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_lesswap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lesswap16", _Dee_HashSelectC(0xa1df15ab, 0x48dc0636cf758a46));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap16", _Dee_HashSelectC(0xa1df15ab, 0x48dc0636cf758a46)));
		}
	};
	WUNUSED _Wrap_lesswap16 (lesswap16)() {
		return this;
	}
	class _Wrap_lesswap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_lesswap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_lesswap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lesswap32", _Dee_HashSelectC(0x493ea22f, 0xd085578fbf82d3f3));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap32", _Dee_HashSelectC(0x493ea22f, 0xd085578fbf82d3f3)));
		}
	};
	WUNUSED _Wrap_lesswap32 (lesswap32)() {
		return this;
	}
	class _Wrap_lesswap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_lesswap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_lesswap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lesswap64", _Dee_HashSelectC(0xb10fb0c, 0xd8c8f5f899351128));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap64", _Dee_HashSelectC(0xb10fb0c, 0xd8c8f5f899351128)));
		}
	};
	WUNUSED _Wrap_lesswap64 (lesswap64)() {
		return this;
	}
	class _Wrap_lesswap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_lesswap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_lesswap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lesswap128", _Dee_HashSelectC(0x40bfea55, 0xe7e285d29d8821fc));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap128", _Dee_HashSelectC(0x40bfea55, 0xe7e285d29d8821fc)));
		}
	};
	WUNUSED _Wrap_lesswap128 (lesswap128)() {
		return this;
	}
	class _Wrap_besswap16
		: public deemon::detail::ConstGetRefProxy<_Wrap_besswap16, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_besswap16(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "besswap16", _Dee_HashSelectC(0x3ec1365f, 0x674f6476ae2fbd81));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap16", _Dee_HashSelectC(0x3ec1365f, 0x674f6476ae2fbd81)));
		}
	};
	WUNUSED _Wrap_besswap16 (besswap16)() {
		return this;
	}
	class _Wrap_besswap32
		: public deemon::detail::ConstGetRefProxy<_Wrap_besswap32, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_besswap32(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "besswap32", _Dee_HashSelectC(0xa712a30c, 0xd83215dce3846b09));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap32", _Dee_HashSelectC(0xa712a30c, 0xd83215dce3846b09)));
		}
	};
	WUNUSED _Wrap_besswap32 (besswap32)() {
		return this;
	}
	class _Wrap_besswap64
		: public deemon::detail::ConstGetRefProxy<_Wrap_besswap64, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_besswap64(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "besswap64", _Dee_HashSelectC(0xc816d1cb, 0xf0d5e2e4170ed996));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap64", _Dee_HashSelectC(0xc816d1cb, 0xf0d5e2e4170ed996)));
		}
	};
	WUNUSED _Wrap_besswap64 (besswap64)() {
		return this;
	}
	class _Wrap_besswap128
		: public deemon::detail::ConstGetRefProxy<_Wrap_besswap128, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_besswap128(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "besswap128", _Dee_HashSelectC(0xc0a57da0, 0xc0932a5134b479f0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap128", _Dee_HashSelectC(0xc0a57da0, 0xc0932a5134b479f0)));
		}
	};
	WUNUSED _Wrap_besswap128 (besswap128)() {
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
			return DeeObject_GetAttrStringHash(m_self, "ffs", _Dee_HashSelectC(0xee0da7ec, 0xb86fa72e64f49d1e));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ffs", _Dee_HashSelectC(0xee0da7ec, 0xb86fa72e64f49d1e)));
		}
	};
	WUNUSED _Wrap_ffs (ffs)() {
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
	WUNUSED _Wrap_fls (fls)() {
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
			return DeeObject_GetAttrStringHash(m_self, "partity", _Dee_HashSelectC(0x2e58034c, 0x667c0ca8cad072de));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "partity", _Dee_HashSelectC(0x2e58034c, 0x667c0ca8cad072de)));
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
			return DeeObject_GetAttrStringHash(m_self, "ctz", _Dee_HashSelectC(0xfc21ef39, 0xefcd17594063dd43));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ctz", _Dee_HashSelectC(0xfc21ef39, 0xefcd17594063dd43)));
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
			return DeeObject_GetAttrStringHash(m_self, "msb", _Dee_HashSelectC(0xbe7ab876, 0xa4211b762e65939c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "msb", _Dee_HashSelectC(0xbe7ab876, 0xa4211b762e65939c)));
		}
	};
	WUNUSED _Wrap_msb (msb)() {
		return this;
	}
	class _Wrap_abs
		: public deemon::detail::ConstGetRefProxy<_Wrap_abs, Numeric> {
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
	WUNUSED _Wrap_abs (abs)() {
		return this;
	}
	class _Wrap_isfloat
		: public deemon::detail::ConstGetRefProxy<_Wrap_isfloat, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isfloat(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isfloat", _Dee_HashSelectC(0xe3da5546, 0x96c4dbe16a19d65d));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isfloat", _Dee_HashSelectC(0xe3da5546, 0x96c4dbe16a19d65d)));
		}
	};
	WUNUSED _Wrap_isfloat (isfloat)() {
		return this;
	}
	class _Wrap_trunc
		: public deemon::detail::ConstGetRefProxy<_Wrap_trunc, Numeric> {
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
	WUNUSED _Wrap_trunc (trunc)() {
		return this;
	}
	class _Wrap_floor
		: public deemon::detail::ConstGetRefProxy<_Wrap_floor, Numeric> {
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
	WUNUSED _Wrap_floor (floor)() {
		return this;
	}
	class _Wrap_ceil
		: public deemon::detail::ConstGetRefProxy<_Wrap_ceil, Numeric> {
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
	WUNUSED _Wrap_ceil (ceil)() {
		return this;
	}
	class _Wrap_round
		: public deemon::detail::ConstGetRefProxy<_Wrap_round, Numeric> {
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
			return DeeObject_GetAttrStringHash(m_self, "isnan", _Dee_HashSelectC(0x36d136ac, 0x428ff2e9a35f402a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnan", _Dee_HashSelectC(0x36d136ac, 0x428ff2e9a35f402a)));
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
			return DeeObject_GetAttrStringHash(m_self, "isinf", _Dee_HashSelectC(0xc8a63b33, 0xf5f86dfadcc14b6a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isinf", _Dee_HashSelectC(0xc8a63b33, 0xf5f86dfadcc14b6a)));
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
			return DeeObject_GetAttrStringHash(m_self, "isfinite", _Dee_HashSelectC(0x797c83cf, 0x9838bbd14d676c85));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isfinite", _Dee_HashSelectC(0x797c83cf, 0x9838bbd14d676c85)));
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
			return DeeObject_GetAttrStringHash(m_self, "isnormal", _Dee_HashSelectC(0x6a066cc5, 0x112aae54ed86881f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnormal", _Dee_HashSelectC(0x6a066cc5, 0x112aae54ed86881f)));
		}
	};
	WUNUSED _Wrap_isnormal (isnormal)() {
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
	WUNUSED _Wrap_nth (nth)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_NUMERIC_H */
