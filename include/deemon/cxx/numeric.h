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
	class _Wrap_int_
		: public deemon::detail::ConstGetRefProxy<_Wrap_int_, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_int_(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "int", _Dee_HashSelect(UINT32_C(0xce831ddf), UINT64_C(0xb7ad4ebe928a1ef0)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "int", _Dee_HashSelect(UINT32_C(0xce831ddf), UINT64_C(0xb7ad4ebe928a1ef0))));
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
			return DeeObject_GetAttrStringHash(m_self, "float", _Dee_HashSelect(UINT32_C(0x95fb9fe8), UINT64_C(0x19ab2ca7919bffe4)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "float", _Dee_HashSelect(UINT32_C(0x95fb9fe8), UINT64_C(0x19ab2ca7919bffe4))));
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
			return DeeObject_GetAttrStringHash(m_self, "s8", _Dee_HashSelect(UINT32_C(0x8e91d8aa), UINT64_C(0x2c22b46392684555)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s8", _Dee_HashSelect(UINT32_C(0x8e91d8aa), UINT64_C(0x2c22b46392684555))));
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
			return DeeObject_GetAttrStringHash(m_self, "s16", _Dee_HashSelect(UINT32_C(0xdb5b7281), UINT64_C(0x93349818f68f2f79)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s16", _Dee_HashSelect(UINT32_C(0xdb5b7281), UINT64_C(0x93349818f68f2f79))));
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
			return DeeObject_GetAttrStringHash(m_self, "s32", _Dee_HashSelect(UINT32_C(0x86be9ab2), UINT64_C(0x113edb000463b661)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s32", _Dee_HashSelect(UINT32_C(0x86be9ab2), UINT64_C(0x113edb000463b661))));
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
			return DeeObject_GetAttrStringHash(m_self, "s64", _Dee_HashSelect(UINT32_C(0x8c789c8a), UINT64_C(0xc14d2d01ad1298ff)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s64", _Dee_HashSelect(UINT32_C(0x8c789c8a), UINT64_C(0xc14d2d01ad1298ff))));
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
			return DeeObject_GetAttrStringHash(m_self, "s128", _Dee_HashSelect(UINT32_C(0xc961f8bf), UINT64_C(0xd26700f831122265)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "s128", _Dee_HashSelect(UINT32_C(0xc961f8bf), UINT64_C(0xd26700f831122265))));
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
			return DeeObject_GetAttrStringHash(m_self, "u8", _Dee_HashSelect(UINT32_C(0x655c23a2), UINT64_C(0x98b4b9518f3f5701)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u8", _Dee_HashSelect(UINT32_C(0x655c23a2), UINT64_C(0x98b4b9518f3f5701))));
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
			return DeeObject_GetAttrStringHash(m_self, "u16", _Dee_HashSelect(UINT32_C(0x4b025b59), UINT64_C(0x14d56899210e228d)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u16", _Dee_HashSelect(UINT32_C(0x4b025b59), UINT64_C(0x14d56899210e228d))));
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
			return DeeObject_GetAttrStringHash(m_self, "u32", _Dee_HashSelect(UINT32_C(0x102d3993), UINT64_C(0x3864575086dc2c74)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u32", _Dee_HashSelect(UINT32_C(0x102d3993), UINT64_C(0x3864575086dc2c74))));
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
			return DeeObject_GetAttrStringHash(m_self, "u64", _Dee_HashSelect(UINT32_C(0x76d7d162), UINT64_C(0x427ddf7b27d8d4c6)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u64", _Dee_HashSelect(UINT32_C(0x76d7d162), UINT64_C(0x427ddf7b27d8d4c6))));
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
			return DeeObject_GetAttrStringHash(m_self, "u128", _Dee_HashSelect(UINT32_C(0x9d0ba7ea), UINT64_C(0x963f3f15e92b1b7c)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "u128", _Dee_HashSelect(UINT32_C(0x9d0ba7ea), UINT64_C(0x963f3f15e92b1b7c))));
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
			return DeeObject_GetAttrStringHash(m_self, "signed8", _Dee_HashSelect(UINT32_C(0x539183ff), UINT64_C(0x630818c691a604db)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed8", _Dee_HashSelect(UINT32_C(0x539183ff), UINT64_C(0x630818c691a604db))));
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
			return DeeObject_GetAttrStringHash(m_self, "signed16", _Dee_HashSelect(UINT32_C(0x39a8ee85), UINT64_C(0xc9dba3957529c8e6)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed16", _Dee_HashSelect(UINT32_C(0x39a8ee85), UINT64_C(0xc9dba3957529c8e6))));
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
			return DeeObject_GetAttrStringHash(m_self, "signed32", _Dee_HashSelect(UINT32_C(0x401ad7d8), UINT64_C(0x8ecce67a523a26b6)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed32", _Dee_HashSelect(UINT32_C(0x401ad7d8), UINT64_C(0x8ecce67a523a26b6))));
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
			return DeeObject_GetAttrStringHash(m_self, "signed64", _Dee_HashSelect(UINT32_C(0xd31a2a48), UINT64_C(0x657452d59b772de9)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed64", _Dee_HashSelect(UINT32_C(0xd31a2a48), UINT64_C(0x657452d59b772de9))));
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
			return DeeObject_GetAttrStringHash(m_self, "signed128", _Dee_HashSelect(UINT32_C(0xfd8c1f06), UINT64_C(0x9bbd99d8421a094d)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "signed128", _Dee_HashSelect(UINT32_C(0xfd8c1f06), UINT64_C(0x9bbd99d8421a094d))));
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
			return DeeObject_GetAttrStringHash(m_self, "unsigned8", _Dee_HashSelect(UINT32_C(0xaef1bad7), UINT64_C(0x9e8b78fba41919c5)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned8", _Dee_HashSelect(UINT32_C(0xaef1bad7), UINT64_C(0x9e8b78fba41919c5))));
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
			return DeeObject_GetAttrStringHash(m_self, "unsigned16", _Dee_HashSelect(UINT32_C(0xa8fca18e), UINT64_C(0xea7c13aabc6570c)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned16", _Dee_HashSelect(UINT32_C(0xa8fca18e), UINT64_C(0xea7c13aabc6570c))));
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
			return DeeObject_GetAttrStringHash(m_self, "unsigned32", _Dee_HashSelect(UINT32_C(0xc023b118), UINT64_C(0x2864fa6210c799af)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned32", _Dee_HashSelect(UINT32_C(0xc023b118), UINT64_C(0x2864fa6210c799af))));
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
			return DeeObject_GetAttrStringHash(m_self, "unsigned64", _Dee_HashSelect(UINT32_C(0x1d42967f), UINT64_C(0x5ee69a5d04e815db)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned64", _Dee_HashSelect(UINT32_C(0x1d42967f), UINT64_C(0x5ee69a5d04e815db))));
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
			return DeeObject_GetAttrStringHash(m_self, "unsigned128", _Dee_HashSelect(UINT32_C(0x493baf7d), UINT64_C(0xd7193bca5b808b24)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "unsigned128", _Dee_HashSelect(UINT32_C(0x493baf7d), UINT64_C(0xd7193bca5b808b24))));
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
			return DeeObject_GetAttrStringHash(m_self, "swap16", _Dee_HashSelect(UINT32_C(0x2f6792b), UINT64_C(0x7b6d7b73549438e6)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap16", _Dee_HashSelect(UINT32_C(0x2f6792b), UINT64_C(0x7b6d7b73549438e6))));
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
			return DeeObject_GetAttrStringHash(m_self, "swap32", _Dee_HashSelect(UINT32_C(0x57c6852f), UINT64_C(0x1451b87dc0b31866)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap32", _Dee_HashSelect(UINT32_C(0x57c6852f), UINT64_C(0x1451b87dc0b31866))));
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
			return DeeObject_GetAttrStringHash(m_self, "swap64", _Dee_HashSelect(UINT32_C(0x8208c247), UINT64_C(0xadc0d9b179945b41)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap64", _Dee_HashSelect(UINT32_C(0x8208c247), UINT64_C(0xadc0d9b179945b41))));
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
			return DeeObject_GetAttrStringHash(m_self, "swap128", _Dee_HashSelect(UINT32_C(0x5e498c0e), UINT64_C(0x68da9a8bb7f4ffa8)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "swap128", _Dee_HashSelect(UINT32_C(0x5e498c0e), UINT64_C(0x68da9a8bb7f4ffa8))));
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
			return DeeObject_GetAttrStringHash(m_self, "sswap16", _Dee_HashSelect(UINT32_C(0xfe35af66), UINT64_C(0xc653941ca804b260)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap16", _Dee_HashSelect(UINT32_C(0xfe35af66), UINT64_C(0xc653941ca804b260))));
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
			return DeeObject_GetAttrStringHash(m_self, "sswap32", _Dee_HashSelect(UINT32_C(0x5ea6d95a), UINT64_C(0xd2c67c37993fd04)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap32", _Dee_HashSelect(UINT32_C(0x5ea6d95a), UINT64_C(0xd2c67c37993fd04))));
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
			return DeeObject_GetAttrStringHash(m_self, "sswap64", _Dee_HashSelect(UINT32_C(0x8367b75d), UINT64_C(0x6154dad8a5a4d002)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap64", _Dee_HashSelect(UINT32_C(0x8367b75d), UINT64_C(0x6154dad8a5a4d002))));
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
			return DeeObject_GetAttrStringHash(m_self, "sswap128", _Dee_HashSelect(UINT32_C(0x99599142), UINT64_C(0x542a3a988d3d57c0)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "sswap128", _Dee_HashSelect(UINT32_C(0x99599142), UINT64_C(0x542a3a988d3d57c0))));
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
			return DeeObject_GetAttrStringHash(m_self, "leswap16", _Dee_HashSelect(UINT32_C(0x487e8219), UINT64_C(0xd0682b67be9aa983)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap16", _Dee_HashSelect(UINT32_C(0x487e8219), UINT64_C(0xd0682b67be9aa983))));
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
			return DeeObject_GetAttrStringHash(m_self, "leswap32", _Dee_HashSelect(UINT32_C(0x3c086c2d), UINT64_C(0xf0151857d7ce0991)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap32", _Dee_HashSelect(UINT32_C(0x3c086c2d), UINT64_C(0xf0151857d7ce0991))));
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
			return DeeObject_GetAttrStringHash(m_self, "leswap64", _Dee_HashSelect(UINT32_C(0x40aa150e), UINT64_C(0xfd67edaf1f992f3f)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap64", _Dee_HashSelect(UINT32_C(0x40aa150e), UINT64_C(0xfd67edaf1f992f3f))));
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
			return DeeObject_GetAttrStringHash(m_self, "leswap128", _Dee_HashSelect(UINT32_C(0x98fe10ba), UINT64_C(0xdfa667bb8a2a91f5)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "leswap128", _Dee_HashSelect(UINT32_C(0x98fe10ba), UINT64_C(0xdfa667bb8a2a91f5))));
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
			return DeeObject_GetAttrStringHash(m_self, "beswap16", _Dee_HashSelect(UINT32_C(0x32de9fb3), UINT64_C(0x305251e9958a4fd6)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap16", _Dee_HashSelect(UINT32_C(0x32de9fb3), UINT64_C(0x305251e9958a4fd6))));
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
			return DeeObject_GetAttrStringHash(m_self, "beswap32", _Dee_HashSelect(UINT32_C(0x8c523765), UINT64_C(0xd715948923e8ba9a)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap32", _Dee_HashSelect(UINT32_C(0x8c523765), UINT64_C(0xd715948923e8ba9a))));
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
			return DeeObject_GetAttrStringHash(m_self, "beswap64", _Dee_HashSelect(UINT32_C(0xc9db181), UINT64_C(0xbe05bbffa5c0b071)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap64", _Dee_HashSelect(UINT32_C(0xc9db181), UINT64_C(0xbe05bbffa5c0b071))));
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
			return DeeObject_GetAttrStringHash(m_self, "beswap128", _Dee_HashSelect(UINT32_C(0x11b2e1b9), UINT64_C(0x7ecdfc744007590f)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "beswap128", _Dee_HashSelect(UINT32_C(0x11b2e1b9), UINT64_C(0x7ecdfc744007590f))));
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
			return DeeObject_GetAttrStringHash(m_self, "lesswap16", _Dee_HashSelect(UINT32_C(0xa1df15ab), UINT64_C(0x48dc0636cf758a46)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap16", _Dee_HashSelect(UINT32_C(0xa1df15ab), UINT64_C(0x48dc0636cf758a46))));
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
			return DeeObject_GetAttrStringHash(m_self, "lesswap32", _Dee_HashSelect(UINT32_C(0x493ea22f), UINT64_C(0xd085578fbf82d3f3)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap32", _Dee_HashSelect(UINT32_C(0x493ea22f), UINT64_C(0xd085578fbf82d3f3))));
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
			return DeeObject_GetAttrStringHash(m_self, "lesswap64", _Dee_HashSelect(UINT32_C(0xb10fb0c), UINT64_C(0xd8c8f5f899351128)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap64", _Dee_HashSelect(UINT32_C(0xb10fb0c), UINT64_C(0xd8c8f5f899351128))));
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
			return DeeObject_GetAttrStringHash(m_self, "lesswap128", _Dee_HashSelect(UINT32_C(0x40bfea55), UINT64_C(0xe7e285d29d8821fc)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lesswap128", _Dee_HashSelect(UINT32_C(0x40bfea55), UINT64_C(0xe7e285d29d8821fc))));
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
			return DeeObject_GetAttrStringHash(m_self, "besswap16", _Dee_HashSelect(UINT32_C(0x3ec1365f), UINT64_C(0x674f6476ae2fbd81)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap16", _Dee_HashSelect(UINT32_C(0x3ec1365f), UINT64_C(0x674f6476ae2fbd81))));
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
			return DeeObject_GetAttrStringHash(m_self, "besswap32", _Dee_HashSelect(UINT32_C(0xa712a30c), UINT64_C(0xd83215dce3846b09)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap32", _Dee_HashSelect(UINT32_C(0xa712a30c), UINT64_C(0xd83215dce3846b09))));
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
			return DeeObject_GetAttrStringHash(m_self, "besswap64", _Dee_HashSelect(UINT32_C(0xc816d1cb), UINT64_C(0xf0d5e2e4170ed996)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap64", _Dee_HashSelect(UINT32_C(0xc816d1cb), UINT64_C(0xf0d5e2e4170ed996))));
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
			return DeeObject_GetAttrStringHash(m_self, "besswap128", _Dee_HashSelect(UINT32_C(0xc0a57da0), UINT64_C(0xc0932a5134b479f0)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "besswap128", _Dee_HashSelect(UINT32_C(0xc0a57da0), UINT64_C(0xc0932a5134b479f0))));
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

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_NUMERIC_H */
