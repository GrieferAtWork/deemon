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
#ifndef GUARD_DEEMON_CXX_MAPPING_H
#define GUARD_DEEMON_CXX_MAPPING_H 1

#include "api.h"
/**/

#include "object.h"
#include "tuple.h"
/**/

#include "../format.h"
#include "../map.h"
/**/

DEE_CXX_BEGIN

template<class Key = Object, class Value = Object>
class Mapping
	: public Sequence<_AbstractTuple<Key, Value> >
	, public detail::ItemProxyAccessor<Mapping<Key, Value>, Value>
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeMapping_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeMapping_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeMapping_Type);
	}
public:
	static Ref<Mapping<Key, Value> > of() DEE_CXX_NOTHROW {
		return nonnull(Dee_EmptyMapping);
	}

public:
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::iterator;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::iter;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::begin;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::end;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::operator[];
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::item;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::hasitem;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::getitem;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::bounditem;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::delitem;
	using detail::ItemProxyAccessor<Mapping<Key, Value>, Value>::setitem;

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Mapping from deemon).printCxxApi(templateParameters: {"Key", "Value"});]]]*/
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
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<_AbstractTuple<Key, Value> > > (byhash)(DeeObject *template_) {
		DeeObject *args[1];
		args[0] = template_;
		return inherit(DeeObject_CallAttrStringHash(this, "byhash", _Dee_HashSelectC(0x7b5277ce, 0x773c8074445a28d9), 1, args));
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
	class _Wrap_iterkeys
		: public deemon::detail::ConstGetRefProxy<_Wrap_iterkeys, Iterator<Key> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_iterkeys(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "iterkeys", _Dee_HashSelectC(0x62bd6adc, 0x535ac8ab28094ab3));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "iterkeys", _Dee_HashSelectC(0x62bd6adc, 0x535ac8ab28094ab3)));
		}
	};
	WUNUSED _Wrap_iterkeys (iterkeys)() {
		return this;
	}
	class _Wrap_itervalues
		: public deemon::detail::ConstGetRefProxy<_Wrap_itervalues, Iterator<Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_itervalues(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "itervalues", _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "itervalues", _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a)));
		}
	};
	WUNUSED _Wrap_itervalues (itervalues)() {
		return this;
	}
	class _Wrap_iteritems
		: public deemon::detail::ConstGetRefProxy<_Wrap_iteritems, Iterator<_AbstractTuple<Key, Value> > > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_iteritems(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "iteritems", _Dee_HashSelectC(0x800fa909, 0x455da7590b6a9f8f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "iteritems", _Dee_HashSelectC(0x800fa909, 0x455da7590b6a9f8f)));
		}
	};
	WUNUSED _Wrap_iteritems (iteritems)() {
		return this;
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_first(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)));
		}
	};
	WUNUSED _Wrap_first (first)() {
		return this;
	}
	class _Wrap_last
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_last(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)));
		}
	};
	WUNUSED _Wrap_last (last)() {
		return this;
	}
	class _Wrap_byattr
		: public deemon::detail::ConstGetRefProxy<_Wrap_byattr, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_byattr(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "byattr", _Dee_HashSelectC(0x7f16cf28, 0x58b9e1994d29c7ca));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "byattr", _Dee_HashSelectC(0x7f16cf28, 0x58b9e1994d29c7ca)));
		}
	};
	WUNUSED _Wrap_byattr (byattr)() {
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
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_MAPPING_H */
