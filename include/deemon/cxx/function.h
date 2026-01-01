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
	void (optimize)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), 0, NULL)));
	}
	NONNULL_CXX((1)) void (optimize)(DeeObject *tuple) {
		DeeObject *args[1];
		args[0] = tuple;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (optimize)(DeeObject *tuple, DeeObject *kwds) {
		DeeObject *args[2];
		args[0] = tuple;
		args[1] = kwds;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (optimize)(DeeObject *tuple, DeeObject *kwds, DeeObject *async) {
		DeeObject *args[3];
		args[0] = tuple;
		args[1] = kwds;
		args[2] = async;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), 3, args)));
	}
	NONNULL_CXX((1, 2)) void (optimize)(DeeObject *tuple, DeeObject *kwds, bool async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "oob", tuple, kwds, async)));
	}
	NONNULL_CXX((1)) void (optimize)(DeeObject *tuple, bool kwds) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "ob", tuple, kwds)));
	}
	NONNULL_CXX((1, 3)) void (optimize)(DeeObject *tuple, bool kwds, DeeObject *async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "obo", tuple, kwds, async)));
	}
	NONNULL_CXX((1)) void (optimize)(DeeObject *tuple, bool kwds, bool async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "obb", tuple, kwds, async)));
	}
	void (optimize)(bool tuple) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "b", tuple)));
	}
	NONNULL_CXX((2)) void (optimize)(bool tuple, DeeObject *kwds) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "bo", tuple, kwds)));
	}
	NONNULL_CXX((2, 3)) void (optimize)(bool tuple, DeeObject *kwds, DeeObject *async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "boo", tuple, kwds, async)));
	}
	NONNULL_CXX((2)) void (optimize)(bool tuple, DeeObject *kwds, bool async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "bob", tuple, kwds, async)));
	}
	void (optimize)(bool tuple, bool kwds) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "bb", tuple, kwds)));
	}
	NONNULL_CXX((3)) void (optimize)(bool tuple, bool kwds, DeeObject *async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "bbo", tuple, kwds, async)));
	}
	void (optimize)(bool tuple, bool kwds, bool async) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "optimize", _Dee_HashSelectC(0x181ecddb, 0xd8ae351bcec097a8), "bbb", tuple, kwds, async)));
	}
	WUNUSED Ref<deemon::bool_> (optimized)() {
		return inherit(DeeObject_CallAttrStringHash(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (optimized)(DeeObject *tuple) {
		DeeObject *args[1];
		args[0] = tuple;
		return inherit(DeeObject_CallAttrStringHash(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (optimized)(DeeObject *tuple, DeeObject *kwds) {
		DeeObject *args[2];
		args[0] = tuple;
		args[1] = kwds;
		return inherit(DeeObject_CallAttrStringHash(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (optimized)(DeeObject *tuple, bool kwds) {
		return inherit(DeeObject_CallAttrStringHashf(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), "ob", tuple, kwds));
	}
	WUNUSED Ref<deemon::bool_> (optimized)(bool tuple) {
		return inherit(DeeObject_CallAttrStringHashf(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), "b", tuple));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (optimized)(bool tuple, DeeObject *kwds) {
		return inherit(DeeObject_CallAttrStringHashf(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), "bo", tuple, kwds));
	}
	WUNUSED Ref<deemon::bool_> (optimized)(bool tuple, bool kwds) {
		return inherit(DeeObject_CallAttrStringHashf(this, "optimized", _Dee_HashSelectC(0xe5639100, 0x7c60221260961ed8), "bb", tuple, kwds));
	}
	class _Wrap___name__
		: public deemon::detail::ConstGetRefProxy<_Wrap___name__, string> {
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
	WUNUSED _Wrap___name__ (__name__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___doc__ (__doc__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___type__
		: public deemon::detail::ConstGetRefProxy<_Wrap___type__, Type> {
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
	WUNUSED _Wrap___type__ (__type__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___module__ (__module__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___operator__
		: public deemon::detail::ConstGetRefProxy<_Wrap___operator__, deemon::int_> {
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
	WUNUSED _Wrap___operator__ (__operator__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___operatorname__ (__operatorname__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___property__
		: public deemon::detail::ConstGetRefProxy<_Wrap___property__, deemon::int_> {
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
	WUNUSED _Wrap___property__ (__property__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___refs__ (__refs__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___statics__
		: public deemon::detail::ConstGetRefProxy<_Wrap___statics__, Sequence<Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___statics__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__statics__", _Dee_HashSelectC(0x4147b465, 0x61d855a2a9645021));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__statics__", _Dee_HashSelectC(0x4147b465, 0x61d855a2a9645021)));
		}
	};
	WUNUSED _Wrap___statics__ (__statics__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___refsbyname__
		: public deemon::detail::ConstGetRefProxy<_Wrap___refsbyname__, Mapping<Object, Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___refsbyname__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__refsbyname__", _Dee_HashSelectC(0x2d058153, 0x460f882177d836ba));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__refsbyname__", _Dee_HashSelectC(0x2d058153, 0x460f882177d836ba)));
		}
	};
	WUNUSED _Wrap___refsbyname__ (__refsbyname__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___staticsbyname__
		: public deemon::detail::ConstGetRefProxy<_Wrap___staticsbyname__, Mapping<Object, Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___staticsbyname__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__staticsbyname__", _Dee_HashSelectC(0xce5fd2cf, 0x9fe58948bf03feb5));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__staticsbyname__", _Dee_HashSelectC(0xce5fd2cf, 0x9fe58948bf03feb5)));
		}
	};
	WUNUSED _Wrap___staticsbyname__ (__staticsbyname__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___symbols__
		: public deemon::detail::ConstGetRefProxy<_Wrap___symbols__, Mapping<Object, Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___symbols__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__symbols__", _Dee_HashSelectC(0x81659df, 0xf4073184aaae4c4c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__symbols__", _Dee_HashSelectC(0x81659df, 0xf4073184aaae4c4c)));
		}
	};
	WUNUSED _Wrap___symbols__ (__symbols__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___nstatic__
		: public deemon::detail::ConstGetRefProxy<_Wrap___nstatic__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___nstatic__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__nstatic__", _Dee_HashSelectC(0xafdbbdfd, 0x1562fdfd929de686));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__nstatic__", _Dee_HashSelectC(0xafdbbdfd, 0x1562fdfd929de686)));
		}
	};
	WUNUSED _Wrap___nstatic__ (__nstatic__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___kwds__ (__kwds__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___defaults__
		: public deemon::detail::ConstGetRefProxy<_Wrap___defaults__, Sequence<Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___defaults__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__defaults__", _Dee_HashSelectC(0x6b005e3c, 0xed5a4d61445c242a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__defaults__", _Dee_HashSelectC(0x6b005e3c, 0xed5a4d61445c242a)));
		}
	};
	WUNUSED _Wrap___defaults__ (__defaults__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___constants__
		: public deemon::detail::ConstGetRefProxy<_Wrap___constants__, Sequence<Object> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___constants__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__constants__", _Dee_HashSelectC(0x46fdb195, 0x9480d2e376e008de));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__constants__", _Dee_HashSelectC(0x46fdb195, 0x9480d2e376e008de)));
		}
	};
	WUNUSED _Wrap___constants__ (__constants__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___argc_min__
		: public deemon::detail::ConstGetRefProxy<_Wrap___argc_min__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___argc_min__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__argc_min__", _Dee_HashSelectC(0xf2637533, 0x5aba0afe6ddccfc9));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__argc_min__", _Dee_HashSelectC(0xf2637533, 0x5aba0afe6ddccfc9)));
		}
	};
	WUNUSED _Wrap___argc_min__ (__argc_min__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___argc_max__
		: public deemon::detail::ConstGetRefProxy<_Wrap___argc_max__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___argc_max__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__argc_max__", _Dee_HashSelectC(0x27fee104, 0xc4e17b1f57515826));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__argc_max__", _Dee_HashSelectC(0x27fee104, 0xc4e17b1f57515826)));
		}
	};
	WUNUSED _Wrap___argc_max__ (__argc_max__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isyielding
		: public deemon::detail::ConstGetRefProxy<_Wrap_isyielding, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isyielding(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isyielding", _Dee_HashSelectC(0x257a8fe5, 0x8bb98317141d1203));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isyielding", _Dee_HashSelectC(0x257a8fe5, 0x8bb98317141d1203)));
		}
	};
	WUNUSED _Wrap_isyielding (isyielding)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_iscopyable
		: public deemon::detail::ConstGetRefProxy<_Wrap_iscopyable, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_iscopyable(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "iscopyable", _Dee_HashSelectC(0xac76e040, 0x70f159e10652f6e6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "iscopyable", _Dee_HashSelectC(0xac76e040, 0x70f159e10652f6e6)));
		}
	};
	WUNUSED _Wrap_iscopyable (iscopyable)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_hasvarargs
		: public deemon::detail::ConstGetRefProxy<_Wrap_hasvarargs, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_hasvarargs(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "hasvarargs", _Dee_HashSelectC(0x8a6b7de2, 0x8d2c7c3b884d483a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "hasvarargs", _Dee_HashSelectC(0x8a6b7de2, 0x8d2c7c3b884d483a)));
		}
	};
	WUNUSED _Wrap_hasvarargs (hasvarargs)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_hasvarkwds
		: public deemon::detail::ConstGetRefProxy<_Wrap_hasvarkwds, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_hasvarkwds(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "hasvarkwds", _Dee_HashSelectC(0x1ae4ea2a, 0xe5cbc54c861b87a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "hasvarkwds", _Dee_HashSelectC(0x1ae4ea2a, 0xe5cbc54c861b87a)));
		}
	};
	WUNUSED _Wrap_hasvarkwds (hasvarkwds)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isthiscall__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isthiscall__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isthiscall__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isthiscall__", _Dee_HashSelectC(0xef9a3351, 0xd4dba41168bf3c17));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isthiscall__", _Dee_HashSelectC(0xef9a3351, 0xd4dba41168bf3c17)));
		}
	};
	WUNUSED _Wrap___isthiscall__ (__isthiscall__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___hasassembly__
		: public deemon::detail::ConstGetRefProxy<_Wrap___hasassembly__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___hasassembly__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__hasassembly__", _Dee_HashSelectC(0xec18fcb7, 0x2901862357febcc7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__hasassembly__", _Dee_HashSelectC(0xec18fcb7, 0x2901862357febcc7)));
		}
	};
	WUNUSED _Wrap___hasassembly__ (__hasassembly__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___islenient__
		: public deemon::detail::ConstGetRefProxy<_Wrap___islenient__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___islenient__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__islenient__", _Dee_HashSelectC(0x45ac013a, 0x368423b672d44f8a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__islenient__", _Dee_HashSelectC(0x45ac013a, 0x368423b672d44f8a)));
		}
	};
	WUNUSED _Wrap___islenient__ (__islenient__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___hasheapframe__
		: public deemon::detail::ConstGetRefProxy<_Wrap___hasheapframe__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___hasheapframe__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__hasheapframe__", _Dee_HashSelectC(0xba738d07, 0x67cebfda2362f448));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__hasheapframe__", _Dee_HashSelectC(0xba738d07, 0x67cebfda2362f448)));
		}
	};
	WUNUSED _Wrap___hasheapframe__ (__hasheapframe__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___hasfinally__
		: public deemon::detail::ConstGetRefProxy<_Wrap___hasfinally__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___hasfinally__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__hasfinally__", _Dee_HashSelectC(0x32ab2f3f, 0x790a8d5db5dafb1f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__hasfinally__", _Dee_HashSelectC(0x32ab2f3f, 0x790a8d5db5dafb1f)));
		}
	};
	WUNUSED _Wrap___hasfinally__ (__hasfinally__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___isconstructor__
		: public deemon::detail::ConstGetRefProxy<_Wrap___isconstructor__, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___isconstructor__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__isconstructor__", _Dee_HashSelectC(0x8f659298, 0x953b024dbade9f7b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__isconstructor__", _Dee_HashSelectC(0x8f659298, 0x953b024dbade9f7b)));
		}
	};
	WUNUSED _Wrap___isconstructor__ (__isconstructor__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___nlocal__
		: public deemon::detail::ConstGetRefProxy<_Wrap___nlocal__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___nlocal__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__nlocal__", _Dee_HashSelectC(0x255d5513, 0x632aec48256da3a5));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__nlocal__", _Dee_HashSelectC(0x255d5513, 0x632aec48256da3a5)));
		}
	};
	WUNUSED _Wrap___nlocal__ (__nlocal__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___nconst__
		: public deemon::detail::ConstGetRefProxy<_Wrap___nconst__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___nconst__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__nconst__", _Dee_HashSelectC(0x2fd6a3b7, 0xdd836d4e7609d99));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__nconst__", _Dee_HashSelectC(0x2fd6a3b7, 0xdd836d4e7609d99)));
		}
	};
	WUNUSED _Wrap___nconst__ (__nconst__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___nref__
		: public deemon::detail::ConstGetRefProxy<_Wrap___nref__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___nref__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__nref__", _Dee_HashSelectC(0x3cc8ae1b, 0x66c66e5b8fd3a748));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__nref__", _Dee_HashSelectC(0x3cc8ae1b, 0x66c66e5b8fd3a748)));
		}
	};
	WUNUSED _Wrap___nref__ (__nref__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___nexcept__
		: public deemon::detail::ConstGetRefProxy<_Wrap___nexcept__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___nexcept__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__nexcept__", _Dee_HashSelectC(0x275b7f2, 0x186fbb5e6f931993));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__nexcept__", _Dee_HashSelectC(0x275b7f2, 0x186fbb5e6f931993)));
		}
	};
	WUNUSED _Wrap___nexcept__ (__nexcept__)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FUNCTION_H */
