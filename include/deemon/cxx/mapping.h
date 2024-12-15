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
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (setdefault)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
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
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__contains__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "__contains__", _Dee_HashSelectC(0x769af591, 0x80f9234f8000b556), 1, args));
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
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate__", _Dee_HashSelectC(0xa424715d, 0xcc3185355ab5e636), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, DeeObject *start) {
		DeeObject *args[2];
		args[0] = cb;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "oo" DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "oo" DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__enumerate_index__)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED Ref<Iterator<_AbstractTuple<Key, Value> > > (__iterkeys__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iterkeys__", _Dee_HashSelectC(0x1cb7a03a, 0x911435bb7713e2b6), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (__bounditem__)(DeeObject *key, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem__)(DeeObject *key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), "ob", key, allow_missing));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasitem__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasitem__", _Dee_HashSelectC(0x53c8b93c, 0x618a29a260a42a16), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__getitem_index__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__getitem_index__)(Dee_ssize_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce),  DEE_PCKdSIZ, key));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__getitem_index__)(size_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce),  DEE_PCKuSIZ, key));
	}
	NONNULL_CXX((1)) void (__delitem_index__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91), 1, args)));
	}
	void (__delitem_index__)(Dee_ssize_t key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91),  DEE_PCKdSIZ, key)));
	}
	void (__delitem_index__)(size_t key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91),  DEE_PCKuSIZ, key)));
	}
	NONNULL_CXX((1, 2)) void (__setitem_index__)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af), 2, args)));
	}
	NONNULL_CXX((2)) void (__setitem_index__)(Dee_ssize_t key, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af),  DEE_PCKdSIZ "o", key, value)));
	}
	NONNULL_CXX((2)) void (__setitem_index__)(size_t key, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af),  DEE_PCKuSIZ "o", key, value)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *key, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), "ob", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t key, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ "o", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ "b", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(size_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem_index__)(size_t key, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ "o", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(size_t key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ "b", key, allow_missing));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasitem_index__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem_index__)(Dee_ssize_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008),  DEE_PCKdSIZ, key));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem_index__)(size_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008),  DEE_PCKuSIZ, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__trygetitem__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem__)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29), 2, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(Dee_ssize_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKdSIZ, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(Dee_ssize_t key, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKdSIZ "o", key, def));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(size_t key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKuSIZ, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_index__)(size_t key, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKuSIZ "o", key, def));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_string__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_string__", _Dee_HashSelectC(0xfdcb52ab, 0x8b10069df9a75d5e), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_string__)(DeeObject *key, DeeObject *def) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_string__", _Dee_HashSelectC(0xfdcb52ab, 0x8b10069df9a75d5e), 2, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__trygetitem_string__)(char const *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_string__", _Dee_HashSelectC(0xfdcb52ab, 0x8b10069df9a75d5e), "s", key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<_AbstractTuple<Key, Value> > (__trygetitem_string__)(char const *key, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_string__", _Dee_HashSelectC(0xfdcb52ab, 0x8b10069df9a75d5e), "so", key, def));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__getitem_string__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem_string__", _Dee_HashSelectC(0xb9e11edc, 0x5cdb5179bbabb2db), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__getitem_string__)(char const *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem_string__", _Dee_HashSelectC(0xb9e11edc, 0x5cdb5179bbabb2db), "s", key));
	}
	NONNULL_CXX((1)) void (__delitem_string__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem_string__", _Dee_HashSelectC(0xb0a69a4b, 0x62351950c3b520b4), 1, args)));
	}
	void (__delitem_string__)(char const *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem_string__", _Dee_HashSelectC(0xb0a69a4b, 0x62351950c3b520b4), "s", key)));
	}
	NONNULL_CXX((1, 2)) void (__setitem_string__)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setitem_string__", _Dee_HashSelectC(0x8f37d691, 0x40a27ff82f6dddfe), 2, args)));
	}
	NONNULL_CXX((2)) void (__setitem_string__)(char const *key, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem_string__", _Dee_HashSelectC(0x8f37d691, 0x40a27ff82f6dddfe), "so", key, value)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_string__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (__bounditem_string__)(DeeObject *key, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_string__)(DeeObject *key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), "ob", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_string__)(char const *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), "s", key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem_string__)(char const *key, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), "so", key, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_string__)(char const *key, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_string__", _Dee_HashSelectC(0x8431c10e, 0xc63ecf02d191745c), "sb", key, allow_missing));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasitem_string__)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasitem_string__", _Dee_HashSelectC(0xd04d2523, 0x60d8d1a2d6427300), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem_string__)(char const *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem_string__", _Dee_HashSelectC(0xd04d2523, 0x60d8d1a2d6427300), "s", key));
	}
	WUNUSED Ref<deemon::int_> (__hash__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__hash__", _Dee_HashSelectC(0xc088645e, 0xbc5b5b1504b9d2d8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__compare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__compare_eq__", _Dee_HashSelectC(0xe8a4d608, 0x1e72244b6194ba57), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__trycompare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__trycompare_eq__", _Dee_HashSelectC(0x7e4181e0, 0x5e7888919a0cdc70), 1, args));
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
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__sub__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__sub__", _Dee_HashSelectC(0xc2239a1e, 0xd91dc2370225ae2f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__and__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__and__", _Dee_HashSelectC(0xac39cb48, 0x2b28cb619a45a71e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__xor__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__xor__", _Dee_HashSelectC(0x7378854c, 0x4a8410f65a74106f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_add__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_add__", _Dee_HashSelectC(0x28e8178a, 0x4185ae98a0cae056), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_sub__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_sub__", _Dee_HashSelectC(0x6df12ae4, 0x8d0a0a61a536e34c), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_and__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
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
	WUNUSED NONNULL_CXX((1)) Ref<Mapping<Key, Value> > (__inplace_or__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
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
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_MAPPING_H */
