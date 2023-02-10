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
#ifndef GUARD_DEEMON_CXX_FLOAT_H
#define GUARD_DEEMON_CXX_FLOAT_H 1

#include "api.h"
/**/

#include "object.h"
#include "numeric.h"
/**/

#include "../format.h"
#include "../float.h"
/**/

DEE_CXX_BEGIN

class float_
	: public Numeric
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeFloat_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFloat_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFloat_CheckExact(ob);
	}

#ifdef CONFIG_HAVE_FPU
public:
	static WUNUSED Ref<float_> of(double value) {
		return inherit(DeeFloat_New(value));
	}

public:
	WUNUSED double cvalue() const DEE_CXX_NOTHROW {
		return DeeFloat_VALUE(this);
	}
#endif /* CONFIG_HAVE_FPU */

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(float from deemon).printCxxApi();]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<deemon::float_> (nextafter)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "nextafter", _Dee_HashSelect(UINT32_C(0xe1e4632d), UINT64_C(0xdb946f5aa5012d37)), 1, args));
	}
	WUNUSED Ref<deemon::float_> (nextafter)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "nextafter", _Dee_HashSelect(UINT32_C(0xe1e4632d), UINT64_C(0xdb946f5aa5012d37)), "f", y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreater", _Dee_HashSelect(UINT32_C(0x4d131c9), UINT64_C(0x7d14fd652371de34)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreater)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreater", _Dee_HashSelect(UINT32_C(0x4d131c9), UINT64_C(0x7d14fd652371de34)), "f", y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isgreaterequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isgreaterequal", _Dee_HashSelect(UINT32_C(0x47c340b), UINT64_C(0xf1cf45f4551813b4)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isgreaterequal)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isgreaterequal", _Dee_HashSelect(UINT32_C(0x47c340b), UINT64_C(0xf1cf45f4551813b4)), "f", y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isless)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "isless", _Dee_HashSelect(UINT32_C(0xc093e250), UINT64_C(0x63ee6983aba9e389)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (isless)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "isless", _Dee_HashSelect(UINT32_C(0xc093e250), UINT64_C(0x63ee6983aba9e389)), "f", y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessequal)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessequal", _Dee_HashSelect(UINT32_C(0x3f6c6174), UINT64_C(0xfaec7676fdaa8cc3)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessequal)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessequal", _Dee_HashSelect(UINT32_C(0x3f6c6174), UINT64_C(0xfaec7676fdaa8cc3)), "f", y));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (islessgreater)(DeeObject *y) {
		DeeObject *args[1];
		args[0] = y;
		return inherit(DeeObject_CallAttrStringHash(this, "islessgreater", _Dee_HashSelect(UINT32_C(0xfc1ab689), UINT64_C(0x16b5b6d91d66c88f)), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (islessgreater)(double y) {
		return inherit(DeeObject_CallAttrStringHashf(this, "islessgreater", _Dee_HashSelect(UINT32_C(0xfc1ab689), UINT64_C(0x16b5b6d91d66c88f)), "f", y));
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
	class _Wrap_abs
		: public deemon::detail::ConstGetRefProxy<_Wrap_abs, deemon::float_> {
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
		: public deemon::detail::ConstGetRefProxy<_Wrap_trunc, deemon::float_> {
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
		: public deemon::detail::ConstGetRefProxy<_Wrap_floor, deemon::float_> {
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
		: public deemon::detail::ConstGetRefProxy<_Wrap_ceil, deemon::float_> {
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
		: public deemon::detail::ConstGetRefProxy<_Wrap_round, deemon::float_> {
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
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FLOAT_H */
