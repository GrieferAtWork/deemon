/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
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
	WUNUSED Ref<Sequence<_AbstractTuple<Key, Value> > > (enumerate)() {
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (enumerate)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<_AbstractTuple<Key, Value> > > (enumerate)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<Key, Value> > (enumerate)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (equals)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "equals", _Dee_HashSelectC(0xcf48fdb6, 0x8ff48babe6a36c10), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (union_)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "union", _Dee_HashSelectC(0x23b88b9b, 0x3b416e7d690babb2), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (difference)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "difference", _Dee_HashSelectC(0xe944baff, 0x6add28d83d2e1f6e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (intersection)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "intersection", _Dee_HashSelectC(0xabaa0afa, 0xc72d025e185198b7), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (symmetric_difference)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "symmetric_difference", _Dee_HashSelectC(0x9a1e5057, 0x17b1425a414674d3), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (setold)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setold", _Dee_HashSelectC(0xb02a28d9, 0xe69353d27a45da0c), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::bool_, Value> > (setold_ex)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setold_ex", _Dee_HashSelectC(0xf8b4d68b, 0x73d8fdc770be1ae), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (setnew)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setnew", _Dee_HashSelectC(0xb6040b2, 0xde8a8697e7aca93d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::bool_, Value> > (setnew_ex)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setnew_ex", _Dee_HashSelectC(0x3f694391, 0x104d84a2d9986bc5), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (setdefault)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setdefault", _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc), 2, args));
	}
	NONNULL_CXX((1)) void (update)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "update", _Dee_HashSelectC(0xdf8e9237, 0x41c79529f2460018), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 1, args));
	}
	NONNULL_CXX((1)) void (removekeys)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "removekeys", _Dee_HashSelectC(0x85b72988, 0xb92131e1a60492b4), 1, args)));
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
	WUNUSED Ref<_AbstractTuple<Key, Value> > (popitem)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popitem", _Dee_HashSelectC(0x40b249f3, 0x131a404a88439bc0), 0, NULL));
	}
	WUNUSED Ref<Iterator<_AbstractTuple<Key, Value> > > (__iter__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iter__", _Dee_HashSelectC(0x4ae49b3, 0x29df7f8a609cead7), 0, NULL));
	}
	WUNUSED Ref<deemon::int_> (__size__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__size__", _Dee_HashSelectC(0x543ba3b5, 0xd416117435cce357), 0, NULL));
	}
	WUNUSED Ref<deemon::int_> (__hash__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__hash__", _Dee_HashSelectC(0xc088645e, 0xbc5b5b1504b9d2d8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__getitem__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem__", _Dee_HashSelectC(0x2796c7b1, 0x326672bfc335fb3d), 1, args));
	}
	NONNULL_CXX((1)) void (__delitem__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem__", _Dee_HashSelectC(0x20ba3d50, 0x477c6001247177f), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (__setitem__)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setitem__", _Dee_HashSelectC(0xa12b6584, 0x4f2c202e4a8ee77a), 2, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__contains__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__contains__", _Dee_HashSelectC(0x769af591, 0x80f9234f8000b556), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate__", _Dee_HashSelectC(0xa424715d, 0xcc3185355ab5e636), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<Key, Value> > (__enumerate__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate__", _Dee_HashSelectC(0xa424715d, 0xcc3185355ab5e636), 3, args));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<Key, Value> > > (__enumerate_items__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_items__", _Dee_HashSelectC(0xe0a30d4a, 0x66f45818680638d6), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<_AbstractTuple<Key, Value> > > (__enumerate_items__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_items__", _Dee_HashSelectC(0xe0a30d4a, 0x66f45818680638d6), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__compare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__compare_eq__", _Dee_HashSelectC(0xe8a4d608, 0x1e72244b6194ba57), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__eq__", _Dee_HashSelectC(0x2e15aa28, 0x20311e6561792a00), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__ne__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__ne__", _Dee_HashSelectC(0x485d961, 0xe9453f35f2aef187), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__lo__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__lo__", _Dee_HashSelectC(0xbd689eba, 0xf2a5e28053b056c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__le__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__le__", _Dee_HashSelectC(0xd4e31410, 0xe371879105557498), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__gr__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__gr__", _Dee_HashSelectC(0x8af205e9, 0x3fe2793f689055e5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__ge__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__ge__", _Dee_HashSelectC(0xe467e452, 0xe5ad3ef5f6f17572), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__add__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__add__", _Dee_HashSelectC(0x5cb4d11a, 0x6f33a2bc44b51c54), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__sub__)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "__sub__", _Dee_HashSelectC(0xc2239a1e, 0xd91dc2370225ae2f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__and__)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "__and__", _Dee_HashSelectC(0xac39cb48, 0x2b28cb619a45a71e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__xor__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__xor__", _Dee_HashSelectC(0x7378854c, 0x4a8410f65a74106f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_add__)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_add__", _Dee_HashSelectC(0x28e8178a, 0x4185ae98a0cae056), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_sub__)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_sub__", _Dee_HashSelectC(0x6df12ae4, 0x8d0a0a61a536e34c), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_and__)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_and__", _Dee_HashSelectC(0x346d8846, 0x338c9c23ba4fea0e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_xor__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_xor__", _Dee_HashSelectC(0xea50ca60, 0xd8f62f1d57ea2d1d), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__or__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__or__", _Dee_HashSelectC(0xf95e054c, 0x2bc6caacde6c129e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_or__)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_or__", _Dee_HashSelectC(0x2ebbbb19, 0xe07a9de59b5dfa47), 1, args));
	}
	NONNULL_CXX((1)) void (insert_all)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_all", _Dee_HashSelectC(0xc6123bae, 0x6d5810c3bb999f3c), 1, args)));
	}
	class _Wrap_keys
		: public deemon::detail::ConstGetRefProxy<_Wrap_keys, Set<Key> > {
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
	WUNUSED _Wrap_keys (keys)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_values (values)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_items (items)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_iterkeys (iterkeys)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_itervalues (itervalues)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_iteritems (iteritems)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_byattr (byattr)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_frozen (frozen)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_asseq
		: public deemon::detail::ConstGetRefProxy<_Wrap_asseq, Sequence<_AbstractTuple<Key, Value> > > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_asseq(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "asseq", _Dee_HashSelectC(0x8141f943, 0x32fdf88783293653));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "asseq", _Dee_HashSelectC(0x8141f943, 0x32fdf88783293653)));
		}
	};
	WUNUSED _Wrap_asseq (asseq)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_asset
		: public deemon::detail::ConstGetRefProxy<_Wrap_asset, Set<_AbstractTuple<Key, Value> > > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_asset(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "asset", _Dee_HashSelectC(0xc73352c6, 0x2f5e4e95dc2238f0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "asset", _Dee_HashSelectC(0xc73352c6, 0x2f5e4e95dc2238f0)));
		}
	};
	WUNUSED _Wrap_asset (asset)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_asmap
		: public deemon::detail::ConstGetRefProxy<_Wrap_asmap, Mapping<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_asmap(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "asmap", _Dee_HashSelectC(0x9f7b392, 0xdf8e0a44ad057842));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "asmap", _Dee_HashSelectC(0x9f7b392, 0xdf8e0a44ad057842)));
		}
	};
	WUNUSED _Wrap_asmap (asmap)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_MAPPING_H */
