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

class Numeric: public Object {
public:
	static Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeNumeric_Type;
	}
	static bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeNumeric_Type);
	}
	static bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeNumeric_Type);
	}
/*[[[deemon (CxxType from rt.gen.cxxapi)(Numeric from deemon).printCxxApi();]]]*/
	WUNUSED deemon::Ref<deemon::int_> int_() {
		return inherit(DeeObject_GetAttrStringHash(this, "int_", _Dee_HashSelect(UINT32_C(0x1cb2a87e), UINT64_C(0x4e04c02fc8858bb1))));
	}
	WUNUSED deemon::Ref<deemon::float_> float_() {
		return inherit(DeeObject_GetAttrStringHash(this, "float_", _Dee_HashSelect(UINT32_C(0x713e72), UINT64_C(0x7bf3842369cf2001))));
	}
	WUNUSED deemon::Ref<deemon::int_> s8() {
		return inherit(DeeObject_GetAttrStringHash(this, "s8", _Dee_HashSelect(UINT32_C(0x8e91d8aa), UINT64_C(0x2c22b46392684555))));
	}
	WUNUSED deemon::Ref<deemon::int_> s16() {
		return inherit(DeeObject_GetAttrStringHash(this, "s16", _Dee_HashSelect(UINT32_C(0xdb5b7281), UINT64_C(0x93349818f68f2f79))));
	}
	WUNUSED deemon::Ref<deemon::int_> s32() {
		return inherit(DeeObject_GetAttrStringHash(this, "s32", _Dee_HashSelect(UINT32_C(0x86be9ab2), UINT64_C(0x113edb000463b661))));
	}
	WUNUSED deemon::Ref<deemon::int_> s64() {
		return inherit(DeeObject_GetAttrStringHash(this, "s64", _Dee_HashSelect(UINT32_C(0x8c789c8a), UINT64_C(0xc14d2d01ad1298ff))));
	}
	WUNUSED deemon::Ref<deemon::int_> s128() {
		return inherit(DeeObject_GetAttrStringHash(this, "s128", _Dee_HashSelect(UINT32_C(0xc961f8bf), UINT64_C(0xd26700f831122265))));
	}
	WUNUSED deemon::Ref<deemon::int_> u8() {
		return inherit(DeeObject_GetAttrStringHash(this, "u8", _Dee_HashSelect(UINT32_C(0x655c23a2), UINT64_C(0x98b4b9518f3f5701))));
	}
	WUNUSED deemon::Ref<deemon::int_> u16() {
		return inherit(DeeObject_GetAttrStringHash(this, "u16", _Dee_HashSelect(UINT32_C(0x4b025b59), UINT64_C(0x14d56899210e228d))));
	}
	WUNUSED deemon::Ref<deemon::int_> u32() {
		return inherit(DeeObject_GetAttrStringHash(this, "u32", _Dee_HashSelect(UINT32_C(0x102d3993), UINT64_C(0x3864575086dc2c74))));
	}
	WUNUSED deemon::Ref<deemon::int_> u64() {
		return inherit(DeeObject_GetAttrStringHash(this, "u64", _Dee_HashSelect(UINT32_C(0x76d7d162), UINT64_C(0x427ddf7b27d8d4c6))));
	}
	WUNUSED deemon::Ref<deemon::int_> u128() {
		return inherit(DeeObject_GetAttrStringHash(this, "u128", _Dee_HashSelect(UINT32_C(0x9d0ba7ea), UINT64_C(0x963f3f15e92b1b7c))));
	}
	WUNUSED deemon::Ref<deemon::int_> signed8() {
		return inherit(DeeObject_GetAttrStringHash(this, "signed8", _Dee_HashSelect(UINT32_C(0x539183ff), UINT64_C(0x630818c691a604db))));
	}
	WUNUSED deemon::Ref<deemon::int_> signed16() {
		return inherit(DeeObject_GetAttrStringHash(this, "signed16", _Dee_HashSelect(UINT32_C(0x39a8ee85), UINT64_C(0xc9dba3957529c8e6))));
	}
	WUNUSED deemon::Ref<deemon::int_> signed32() {
		return inherit(DeeObject_GetAttrStringHash(this, "signed32", _Dee_HashSelect(UINT32_C(0x401ad7d8), UINT64_C(0x8ecce67a523a26b6))));
	}
	WUNUSED deemon::Ref<deemon::int_> signed64() {
		return inherit(DeeObject_GetAttrStringHash(this, "signed64", _Dee_HashSelect(UINT32_C(0xd31a2a48), UINT64_C(0x657452d59b772de9))));
	}
	WUNUSED deemon::Ref<deemon::int_> signed128() {
		return inherit(DeeObject_GetAttrStringHash(this, "signed128", _Dee_HashSelect(UINT32_C(0xfd8c1f06), UINT64_C(0x9bbd99d8421a094d))));
	}
	WUNUSED deemon::Ref<deemon::int_> unsigned8() {
		return inherit(DeeObject_GetAttrStringHash(this, "unsigned8", _Dee_HashSelect(UINT32_C(0xaef1bad7), UINT64_C(0x9e8b78fba41919c5))));
	}
	WUNUSED deemon::Ref<deemon::int_> unsigned16() {
		return inherit(DeeObject_GetAttrStringHash(this, "unsigned16", _Dee_HashSelect(UINT32_C(0xa8fca18e), UINT64_C(0xea7c13aabc6570c))));
	}
	WUNUSED deemon::Ref<deemon::int_> unsigned32() {
		return inherit(DeeObject_GetAttrStringHash(this, "unsigned32", _Dee_HashSelect(UINT32_C(0xc023b118), UINT64_C(0x2864fa6210c799af))));
	}
	WUNUSED deemon::Ref<deemon::int_> unsigned64() {
		return inherit(DeeObject_GetAttrStringHash(this, "unsigned64", _Dee_HashSelect(UINT32_C(0x1d42967f), UINT64_C(0x5ee69a5d04e815db))));
	}
	WUNUSED deemon::Ref<deemon::int_> unsigned128() {
		return inherit(DeeObject_GetAttrStringHash(this, "unsigned128", _Dee_HashSelect(UINT32_C(0x493baf7d), UINT64_C(0xd7193bca5b808b24))));
	}
	WUNUSED deemon::Ref<deemon::int_> swap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "swap16", _Dee_HashSelect(UINT32_C(0x2f6792b), UINT64_C(0x7b6d7b73549438e6))));
	}
	WUNUSED deemon::Ref<deemon::int_> swap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "swap32", _Dee_HashSelect(UINT32_C(0x57c6852f), UINT64_C(0x1451b87dc0b31866))));
	}
	WUNUSED deemon::Ref<deemon::int_> swap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "swap64", _Dee_HashSelect(UINT32_C(0x8208c247), UINT64_C(0xadc0d9b179945b41))));
	}
	WUNUSED deemon::Ref<deemon::int_> swap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "swap128", _Dee_HashSelect(UINT32_C(0x5e498c0e), UINT64_C(0x68da9a8bb7f4ffa8))));
	}
	WUNUSED deemon::Ref<deemon::int_> sswap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "sswap16", _Dee_HashSelect(UINT32_C(0xfe35af66), UINT64_C(0xc653941ca804b260))));
	}
	WUNUSED deemon::Ref<deemon::int_> sswap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "sswap32", _Dee_HashSelect(UINT32_C(0x5ea6d95a), UINT64_C(0xd2c67c37993fd04))));
	}
	WUNUSED deemon::Ref<deemon::int_> sswap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "sswap64", _Dee_HashSelect(UINT32_C(0x8367b75d), UINT64_C(0x6154dad8a5a4d002))));
	}
	WUNUSED deemon::Ref<deemon::int_> sswap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "sswap128", _Dee_HashSelect(UINT32_C(0x99599142), UINT64_C(0x542a3a988d3d57c0))));
	}
	WUNUSED deemon::Ref<deemon::int_> leswap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "leswap16", _Dee_HashSelect(UINT32_C(0x487e8219), UINT64_C(0xd0682b67be9aa983))));
	}
	WUNUSED deemon::Ref<deemon::int_> leswap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "leswap32", _Dee_HashSelect(UINT32_C(0x3c086c2d), UINT64_C(0xf0151857d7ce0991))));
	}
	WUNUSED deemon::Ref<deemon::int_> leswap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "leswap64", _Dee_HashSelect(UINT32_C(0x40aa150e), UINT64_C(0xfd67edaf1f992f3f))));
	}
	WUNUSED deemon::Ref<deemon::int_> leswap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "leswap128", _Dee_HashSelect(UINT32_C(0x98fe10ba), UINT64_C(0xdfa667bb8a2a91f5))));
	}
	WUNUSED deemon::Ref<deemon::int_> beswap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "beswap16", _Dee_HashSelect(UINT32_C(0x32de9fb3), UINT64_C(0x305251e9958a4fd6))));
	}
	WUNUSED deemon::Ref<deemon::int_> beswap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "beswap32", _Dee_HashSelect(UINT32_C(0x8c523765), UINT64_C(0xd715948923e8ba9a))));
	}
	WUNUSED deemon::Ref<deemon::int_> beswap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "beswap64", _Dee_HashSelect(UINT32_C(0xc9db181), UINT64_C(0xbe05bbffa5c0b071))));
	}
	WUNUSED deemon::Ref<deemon::int_> beswap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "beswap128", _Dee_HashSelect(UINT32_C(0x11b2e1b9), UINT64_C(0x7ecdfc744007590f))));
	}
	WUNUSED deemon::Ref<deemon::int_> lesswap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "lesswap16", _Dee_HashSelect(UINT32_C(0xa1df15ab), UINT64_C(0x48dc0636cf758a46))));
	}
	WUNUSED deemon::Ref<deemon::int_> lesswap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "lesswap32", _Dee_HashSelect(UINT32_C(0x493ea22f), UINT64_C(0xd085578fbf82d3f3))));
	}
	WUNUSED deemon::Ref<deemon::int_> lesswap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "lesswap64", _Dee_HashSelect(UINT32_C(0xb10fb0c), UINT64_C(0xd8c8f5f899351128))));
	}
	WUNUSED deemon::Ref<deemon::int_> lesswap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "lesswap128", _Dee_HashSelect(UINT32_C(0x40bfea55), UINT64_C(0xe7e285d29d8821fc))));
	}
	WUNUSED deemon::Ref<deemon::int_> besswap16() {
		return inherit(DeeObject_GetAttrStringHash(this, "besswap16", _Dee_HashSelect(UINT32_C(0x3ec1365f), UINT64_C(0x674f6476ae2fbd81))));
	}
	WUNUSED deemon::Ref<deemon::int_> besswap32() {
		return inherit(DeeObject_GetAttrStringHash(this, "besswap32", _Dee_HashSelect(UINT32_C(0xa712a30c), UINT64_C(0xd83215dce3846b09))));
	}
	WUNUSED deemon::Ref<deemon::int_> besswap64() {
		return inherit(DeeObject_GetAttrStringHash(this, "besswap64", _Dee_HashSelect(UINT32_C(0xc816d1cb), UINT64_C(0xf0d5e2e4170ed996))));
	}
	WUNUSED deemon::Ref<deemon::int_> besswap128() {
		return inherit(DeeObject_GetAttrStringHash(this, "besswap128", _Dee_HashSelect(UINT32_C(0xc0a57da0), UINT64_C(0xc0932a5134b479f0))));
	}
	WUNUSED deemon::Ref<deemon::int_> popcount() {
		return inherit(DeeObject_GetAttrStringHash(this, "popcount", _Dee_HashSelect(UINT32_C(0x9fb087f4), UINT64_C(0x1da7fbc6c1dd86dd))));
	}
	WUNUSED deemon::Ref<deemon::int_> ffs() {
		return inherit(DeeObject_GetAttrStringHash(this, "ffs", _Dee_HashSelect(UINT32_C(0xee0da7ec), UINT64_C(0xb86fa72e64f49d1e))));
	}
	WUNUSED deemon::Ref<deemon::int_> partity() {
		return inherit(DeeObject_GetAttrStringHash(this, "partity", _Dee_HashSelect(UINT32_C(0x2e58034c), UINT64_C(0x667c0ca8cad072de))));
	}
	WUNUSED deemon::Ref<deemon::int_> ctz() {
		return inherit(DeeObject_GetAttrStringHash(this, "ctz", _Dee_HashSelect(UINT32_C(0xfc21ef39), UINT64_C(0xefcd17594063dd43))));
	}
	WUNUSED deemon::Ref<deemon::int_> msb() {
		return inherit(DeeObject_GetAttrStringHash(this, "msb", _Dee_HashSelect(UINT32_C(0xbe7ab876), UINT64_C(0xa4211b762e65939c))));
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_NUMERIC_H */
