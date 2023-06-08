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
#ifndef GUARD_DEEMON_CXX_FUNCTION_H
#define GUARD_DEEMON_CXX_FUNCTION_H 1

#include "api.h"
/**/

#include "callable.h"
#include "object.h"
/**/

#include "../format.h"
#include "../code.h"
/**/

#include <hybrid/typecore.h>

DEE_CXX_BEGIN

class Function
	: public Callable
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeFunction_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFunction_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFunction_CheckExact(ob);
	}

public:
	static Ref<Function> of(DeeObject *code, size_t refc,
	                        DeeObject *const *refv) {
		return inherit(DeeFunction_New(code, refc, refv));
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Function from deemon).printCxxApi();]]]*/
	class _Wrap___name__
		: public deemon::detail::ConstGetRefProxy<_Wrap___name__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___name__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__name__", _Dee_HashSelectC(0x27a6cbdf, 0x9004f0806b170f3f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__name__", _Dee_HashSelectC(0x27a6cbdf, 0x9004f0806b170f3f)));
		}
	};
	WUNUSED _Wrap___name__ (__name__)() {
		return this;
	}
	class _Wrap___doc__
		: public deemon::detail::ConstGetRefProxy<_Wrap___doc__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___doc__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__doc__", _Dee_HashSelectC(0xd5eefba, 0x9e1c0e198ad451ff));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__doc__", _Dee_HashSelectC(0xd5eefba, 0x9e1c0e198ad451ff)));
		}
	};
	WUNUSED _Wrap___doc__ (__doc__)() {
		return this;
	}
	class _Wrap___type__
		: public deemon::detail::ConstGetRefProxy<_Wrap___type__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___type__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__type__", _Dee_HashSelectC(0xc25dc337, 0xd3fa545616840a4e));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__type__", _Dee_HashSelectC(0xc25dc337, 0xd3fa545616840a4e)));
		}
	};
	WUNUSED _Wrap___type__ (__type__)() {
		return this;
	}
	class _Wrap___module__
		: public deemon::detail::ConstGetRefProxy<_Wrap___module__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___module__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb)));
		}
	};
	WUNUSED _Wrap___module__ (__module__)() {
		return this;
	}
	class _Wrap___operator__
		: public deemon::detail::ConstGetRefProxy<_Wrap___operator__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___operator__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__operator__", _Dee_HashSelectC(0xce3dee75, 0x9d8d1bba3a878aad));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__operator__", _Dee_HashSelectC(0xce3dee75, 0x9d8d1bba3a878aad)));
		}
	};
	WUNUSED _Wrap___operator__ (__operator__)() {
		return this;
	}
	class _Wrap___operatorname__
		: public deemon::detail::ConstGetRefProxy<_Wrap___operatorname__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___operatorname__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__operatorname__", _Dee_HashSelectC(0x8893f1ad, 0x1bbac41c8a3c45b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__operatorname__", _Dee_HashSelectC(0x8893f1ad, 0x1bbac41c8a3c45b)));
		}
	};
	WUNUSED _Wrap___operatorname__ (__operatorname__)() {
		return this;
	}
	class _Wrap___property__
		: public deemon::detail::ConstGetRefProxy<_Wrap___property__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___property__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__property__", _Dee_HashSelectC(0x1267b4ff, 0x77922bbfee0194ee));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__property__", _Dee_HashSelectC(0x1267b4ff, 0x77922bbfee0194ee)));
		}
	};
	WUNUSED _Wrap___property__ (__property__)() {
		return this;
	}
	class _Wrap___refs__
		: public deemon::detail::ConstGetRefProxy<_Wrap___refs__, Sequence<Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___refs__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__refs__", _Dee_HashSelectC(0x17cd1423, 0x6bbc6d8947e3c6a2));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__refs__", _Dee_HashSelectC(0x17cd1423, 0x6bbc6d8947e3c6a2)));
		}
	};
	WUNUSED _Wrap___refs__ (__refs__)() {
		return this;
	}
	class _Wrap___kwds__
		: public deemon::detail::ConstGetRefProxy<_Wrap___kwds__, Sequence<string> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___kwds__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__kwds__", _Dee_HashSelectC(0xd3926a14, 0xa90825b224a7262b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__kwds__", _Dee_HashSelectC(0xd3926a14, 0xa90825b224a7262b)));
		}
	};
	WUNUSED _Wrap___kwds__ (__kwds__)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FUNCTION_H */
