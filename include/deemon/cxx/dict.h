/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_DICT_H
#define GUARD_DEEMON_CXX_DICT_H 1

#include "api.h"
/**/

#include "mapping.h"
#include "object.h"
#include "tuple.h"
/**/

#include "../dict.h"
#include "../format.h"
/**/

DEE_CXX_BEGIN

template<class Key = Object, class Value = Object>
class Dict
	: public Mapping<Key, Value>
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeDict_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeDict_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeDict_CheckExact(ob);
	}
public:
	static WUNUSED Ref<Dict<Key, Value> > of() DEE_CXX_NOTHROW {
		return inherit(DeeDict_New());
	}
	static WUNUSED NONNULL_CXX((1)) Ref<Dict<Key, Value> > ofseq(DeeObject *seq) DEE_CXX_NOTHROW {
		return inherit(DeeDict_FromSequence(seq));
	}
	static WUNUSED NONNULL_CXX((1)) Ref<Dict<Key, Value> > ofiter(DeeObject *iter) DEE_CXX_NOTHROW {
		return inherit(DeeDict_FromIterator(iter));
	}

public:

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Dict from deemon).printCxxApi(templateParameters: {"Key", "Value"});]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<Value> (get)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "get", _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (get)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "get", _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Value> (pop)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (pop)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 2, args));
	}
	void (clear)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "clear", _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c), 0, NULL)));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (popitem)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popitem", _Dee_HashSelectC(0x40b249f3, 0x131a404a88439bc0), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Value> (setdefault)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "setdefault", _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (setdefault)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "setdefault", _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (setold)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setold", _Dee_HashSelectC(0xb02a28d9, 0xe69353d27a45da0c), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (setnew)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setnew", _Dee_HashSelectC(0xb6040b2, 0xde8a8697e7aca93d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::bool_, Value> > (setold_ex)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setold_ex", _Dee_HashSelectC(0xf8b4d68b, 0x73d8fdc770be1ae), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::bool_, Value> > (setnew_ex)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setnew_ex", _Dee_HashSelectC(0x3f694391, 0x104d84a2d9986bc5), 2, args));
	}
	NONNULL_CXX((1)) void (update)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "update", _Dee_HashSelectC(0xdf8e9237, 0x41c79529f2460018), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<_AbstractTuple<Key, Value> > > (byhash)(DeeObject *template_) {
		DeeObject *args[1];
		args[0] = template_;
		return inherit(DeeObject_CallAttrStringHash(this, "byhash", _Dee_HashSelectC(0x7b5277ce, 0x773c8074445a28d9), 1, args));
	}
	NONNULL_CXX((1)) void (insert_all)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_all", _Dee_HashSelectC(0xc6123bae, 0x6d5810c3bb999f3c), 1, args)));
	}
	class _Wrap_keys
		: public deemon::detail::ConstGetRefProxy<_Wrap_keys, Sequence<Key> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_keys(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "keys", _Dee_HashSelectC(0x97e36be1, 0x654d31bc4825131c));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "keys", _Dee_HashSelectC(0x97e36be1, 0x654d31bc4825131c)));
		}
	};
	WUNUSED _Wrap_keys (keys)() {
		return this;
	}
	class _Wrap_values
		: public deemon::detail::ConstGetRefProxy<_Wrap_values, Sequence<Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_values(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "values", _Dee_HashSelectC(0x33b551c8, 0xf6e3e991b86d1574));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "values", _Dee_HashSelectC(0x33b551c8, 0xf6e3e991b86d1574)));
		}
	};
	WUNUSED _Wrap_values (values)() {
		return this;
	}
	class _Wrap_items
		: public deemon::detail::ConstGetRefProxy<_Wrap_items, Sequence<_AbstractTuple<Key, Value> > > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_items(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "items", _Dee_HashSelectC(0x8858badc, 0xa52ef5f42baafa42));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "items", _Dee_HashSelectC(0x8858badc, 0xa52ef5f42baafa42)));
		}
	};
	WUNUSED _Wrap_items (items)() {
		return this;
	}
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Mapping<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_frozen(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd)));
		}
	};
	WUNUSED _Wrap_frozen (frozen)() {
		return this;
	}
	class _Wrap_max_load_factor
		: public deemon::detail::ConstGetRefProxy<_Wrap_max_load_factor, deemon::float_>
		, public deemon::detail::ConstSetRefProxy<_Wrap_max_load_factor, deemon::float_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_max_load_factor, deemon::float_>::operator =;
		_Wrap_max_load_factor(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "max_load_factor", _Dee_HashSelectC(0xae8f7ef7, 0xa0a4981439b4c20f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "max_load_factor", _Dee_HashSelectC(0xae8f7ef7, 0xa0a4981439b4c20f)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "max_load_factor", _Dee_HashSelectC(0xae8f7ef7, 0xa0a4981439b4c20f)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "max_load_factor", _Dee_HashSelectC(0xae8f7ef7, 0xa0a4981439b4c20f), value);
		}
	};
	WUNUSED _Wrap_max_load_factor (max_load_factor)() {
		return this;
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
	WUNUSED _Wrap___sizeof__ (__sizeof__)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_DICT_H */
