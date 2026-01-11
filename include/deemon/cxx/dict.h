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

public:

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Dict from deemon).printCxxApi(templateParameters: {"Key", "Value"});]]]*/
	void (clear)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "clear", _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c), 0, NULL)));
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
	WUNUSED NONNULL_CXX((1, 2)) Ref<Value> (setdefault)(DeeObject *key, DeeObject *value) {
		DeeObject *args[2];
		args[0] = key;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "setdefault", _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc), 2, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (popitem)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popitem", _Dee_HashSelectC(0x40b249f3, 0x131a404a88439bc0), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (shrink)() {
		return inherit(DeeObject_CallAttrStringHash(this, "shrink", _Dee_HashSelectC(0xd8afa32b, 0xf4785bc46214dc5), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (shrink)(DeeObject *fully) {
		DeeObject *args[1];
		args[0] = fully;
		return inherit(DeeObject_CallAttrStringHash(this, "shrink", _Dee_HashSelectC(0xd8afa32b, 0xf4785bc46214dc5), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (shrink)(bool fully) {
		return inherit(DeeObject_CallAttrStringHashf(this, "shrink", _Dee_HashSelectC(0xd8afa32b, 0xf4785bc46214dc5), "b", fully));
	}
	WUNUSED Ref<deemon::bool_> (reserve)() {
		return inherit(DeeObject_CallAttrStringHash(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total) {
		DeeObject *args[1];
		args[0] = total;
		return inherit(DeeObject_CallAttrStringHash(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (reserve)(DeeObject *total, DeeObject *more) {
		DeeObject *args[2];
		args[0] = total;
		args[1] = more;
		return inherit(DeeObject_CallAttrStringHash(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (reserve)(DeeObject *total, DeeObject *more, DeeObject *weak) {
		DeeObject *args[3];
		args[0] = total;
		args[1] = more;
		args[2] = weak;
		return inherit(DeeObject_CallAttrStringHash(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (reserve)(DeeObject *total, DeeObject *more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "oob", total, more, weak));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKdSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (reserve)(DeeObject *total, Dee_ssize_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKdSIZ "o", total, more, weak));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, Dee_ssize_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKdSIZ "b", total, more, weak));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKuSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (reserve)(DeeObject *total, size_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKuSIZ "o", total, more, weak));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, size_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKuSIZ "b", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ, total));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, DeeObject *more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ "o", total, more));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, DeeObject *more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ "oo", total, more, weak));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, DeeObject *more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ "ob", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKdSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, Dee_ssize_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKdSIZ "o", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, Dee_ssize_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKdSIZ "b", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKuSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, size_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKuSIZ "o", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, size_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKuSIZ "b", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ, total));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(size_t total, DeeObject *more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ "o", total, more));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (reserve)(size_t total, DeeObject *more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ "oo", total, more, weak));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(size_t total, DeeObject *more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ "ob", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKdSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (reserve)(size_t total, Dee_ssize_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKdSIZ "o", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, Dee_ssize_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKdSIZ "b", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKuSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (reserve)(size_t total, size_t more, DeeObject *weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKuSIZ "o", total, more, weak));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, size_t more, bool weak) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKuSIZ "b", total, more, weak));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__seq_xchitem__)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_xchitem__", _Dee_HashSelectC(0x4eea0d1, 0x6238b16fe217a6ed), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<_AbstractTuple<Key, Value> > (__seq_xchitem__)(Dee_ssize_t index, DeeObject *item) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_xchitem__", _Dee_HashSelectC(0x4eea0d1, 0x6238b16fe217a6ed), Dee_PCKdSIZ "o", index, item));
	}
	WUNUSED NONNULL_CXX((2)) Ref<_AbstractTuple<Key, Value> > (__seq_xchitem__)(size_t index, DeeObject *item) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_xchitem__", _Dee_HashSelectC(0x4eea0d1, 0x6238b16fe217a6ed), Dee_PCKuSIZ "o", index, item));
	}
	NONNULL_CXX((1)) void (__seq_erase__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (__seq_erase__)(DeeObject *index, DeeObject *count) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = count;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), 2, args)));
	}
	NONNULL_CXX((1)) void (__seq_erase__)(DeeObject *index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), "o" Dee_PCKdSIZ, index, count)));
	}
	NONNULL_CXX((1)) void (__seq_erase__)(DeeObject *index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), "o" Dee_PCKuSIZ, index, count)));
	}
	void (__seq_erase__)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKdSIZ, index)));
	}
	NONNULL_CXX((2)) void (__seq_erase__)(Dee_ssize_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKdSIZ "o", index, count)));
	}
	void (__seq_erase__)(Dee_ssize_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKdSIZ Dee_PCKdSIZ, index, count)));
	}
	void (__seq_erase__)(Dee_ssize_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKdSIZ Dee_PCKuSIZ, index, count)));
	}
	void (__seq_erase__)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKuSIZ, index)));
	}
	NONNULL_CXX((2)) void (__seq_erase__)(size_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKuSIZ "o", index, count)));
	}
	void (__seq_erase__)(size_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKuSIZ Dee_PCKdSIZ, index, count)));
	}
	void (__seq_erase__)(size_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_erase__", _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e), Dee_PCKuSIZ Dee_PCKuSIZ, index, count)));
	}
	NONNULL_CXX((1, 2)) void (__seq_insert__)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_insert__", _Dee_HashSelectC(0x108c61ac, 0xe94b2ec29ead79d1), 2, args)));
	}
	NONNULL_CXX((2)) void (__seq_insert__)(Dee_ssize_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_insert__", _Dee_HashSelectC(0x108c61ac, 0xe94b2ec29ead79d1), Dee_PCKdSIZ "o", index, item)));
	}
	NONNULL_CXX((2)) void (__seq_insert__)(size_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_insert__", _Dee_HashSelectC(0x108c61ac, 0xe94b2ec29ead79d1), Dee_PCKuSIZ "o", index, item)));
	}
	NONNULL_CXX((1)) void (__seq_append__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_append__", _Dee_HashSelectC(0x43ea1331, 0x383f299606f81ebe), 1, args)));
	}
	NONNULL_CXX((1)) void (__seq_pushfront__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_pushfront__", _Dee_HashSelectC(0xe30e92a5, 0x69ae18cfaba44b5a), 1, args)));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__seq_pop__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_pop__", _Dee_HashSelectC(0xbc856b3, 0x292be45738029ef3), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_pop__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_pop__", _Dee_HashSelectC(0xbc856b3, 0x292be45738029ef3), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__seq_pop__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_pop__", _Dee_HashSelectC(0xbc856b3, 0x292be45738029ef3), Dee_PCKdSIZ, index));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__seq_pop__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_pop__", _Dee_HashSelectC(0xbc856b3, 0x292be45738029ef3), Dee_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should) {
		DeeObject *args[1];
		args[0] = should;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start) {
		DeeObject *args[2];
		args[0] = should;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, DeeObject *end, DeeObject *max) {
		DeeObject *args[4];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "ooo" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "ooo" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "oo" Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ "o" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ "o" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ "o" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ "o" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_removeif__)(DeeObject *should, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_removeif__", _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	void (__seq_reverse__)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), 0, NULL)));
	}
	NONNULL_CXX((1)) void (__seq_reverse__)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (__seq_reverse__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), 2, args)));
	}
	NONNULL_CXX((1)) void (__seq_reverse__)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), "o" Dee_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (__seq_reverse__)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), "o" Dee_PCKuSIZ, start, end)));
	}
	void (__seq_reverse__)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (__seq_reverse__)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKdSIZ "o", start, end)));
	}
	void (__seq_reverse__)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKdSIZ Dee_PCKdSIZ, start, end)));
	}
	void (__seq_reverse__)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKdSIZ Dee_PCKuSIZ, start, end)));
	}
	void (__seq_reverse__)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (__seq_reverse__)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKuSIZ "o", start, end)));
	}
	void (__seq_reverse__)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKuSIZ Dee_PCKdSIZ, start, end)));
	}
	void (__seq_reverse__)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_reverse__", _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6), Dee_PCKuSIZ Dee_PCKuSIZ, start, end)));
	}
	WUNUSED Ref<Iterator<_AbstractTuple<Key, Value> > > (__seq_iter__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_iter__", _Dee_HashSelectC(0x2fb16a47, 0x49d80da3961f157e), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, DeeObject *start) {
		DeeObject *args[2];
		args[0] = cb;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_enumerate__)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<Key, Value> > (__seq_getitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_getitem__", _Dee_HashSelectC(0x4c346166, 0x8b3bf00bdee10ba0), 1, args));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__seq_getitem__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_getitem__", _Dee_HashSelectC(0x4c346166, 0x8b3bf00bdee10ba0), Dee_PCKdSIZ, index));
	}
	WUNUSED Ref<_AbstractTuple<Key, Value> > (__seq_getitem__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_getitem__", _Dee_HashSelectC(0x4c346166, 0x8b3bf00bdee10ba0), Dee_PCKuSIZ, index));
	}
	NONNULL_CXX((1)) void (__seq_delitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_delitem__", _Dee_HashSelectC(0x3b8e2105, 0x11a0e21507457e51), 1, args)));
	}
	void (__seq_delitem__)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_delitem__", _Dee_HashSelectC(0x3b8e2105, 0x11a0e21507457e51), Dee_PCKdSIZ, index)));
	}
	void (__seq_delitem__)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_delitem__", _Dee_HashSelectC(0x3b8e2105, 0x11a0e21507457e51), Dee_PCKuSIZ, index)));
	}
	NONNULL_CXX((1, 2)) void (__seq_setitem__)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_setitem__", _Dee_HashSelectC(0x60928657, 0x54b9696d799c0ea8), 2, args)));
	}
	NONNULL_CXX((2)) void (__seq_setitem__)(Dee_ssize_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_setitem__", _Dee_HashSelectC(0x60928657, 0x54b9696d799c0ea8), Dee_PCKdSIZ "o", index, value)));
	}
	NONNULL_CXX((2)) void (__seq_setitem__)(size_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__seq_setitem__", _Dee_HashSelectC(0x60928657, 0x54b9696d799c0ea8), Dee_PCKuSIZ "o", index, value)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__seq_compare__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_compare__", _Dee_HashSelectC(0xd06efa37, 0x1f9459ac2c6feba4), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Numeric> (__seq_compare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_compare_eq__", _Dee_HashSelectC(0xa3f084a6, 0x857f75efb347d0ac), 1, args));
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_first, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_first, _AbstractTuple<Key, Value> >::operator =;
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
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7), value);
		}
	};
	WUNUSED _Wrap_first (first)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_last
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_last, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_last, _AbstractTuple<Key, Value> >::operator =;
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
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc), value);
		}
	};
	WUNUSED _Wrap_last (last)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_firstkey
		: public deemon::detail::ConstGetRefProxy<_Wrap_firstkey, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_firstkey, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_firstkey, _AbstractTuple<Key, Value> >::operator =;
		_Wrap_firstkey(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "firstkey", _Dee_HashSelectC(0x145fac13, 0x610ea85f4f5a0074));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "firstkey", _Dee_HashSelectC(0x145fac13, 0x610ea85f4f5a0074)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "firstkey", _Dee_HashSelectC(0x145fac13, 0x610ea85f4f5a0074)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "firstkey", _Dee_HashSelectC(0x145fac13, 0x610ea85f4f5a0074), value);
		}
	};
	WUNUSED _Wrap_firstkey (firstkey)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_lastkey
		: public deemon::detail::ConstGetRefProxy<_Wrap_lastkey, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_lastkey, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_lastkey, _AbstractTuple<Key, Value> >::operator =;
		_Wrap_lastkey(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lastkey", _Dee_HashSelectC(0x6dab3145, 0xecff85dd1d641efb));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lastkey", _Dee_HashSelectC(0x6dab3145, 0xecff85dd1d641efb)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "lastkey", _Dee_HashSelectC(0x6dab3145, 0xecff85dd1d641efb)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "lastkey", _Dee_HashSelectC(0x6dab3145, 0xecff85dd1d641efb), value);
		}
	};
	WUNUSED _Wrap_lastkey (lastkey)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_firstvalue
		: public deemon::detail::ConstGetRefProxy<_Wrap_firstvalue, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_firstvalue, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_firstvalue, _AbstractTuple<Key, Value> >::operator =;
		_Wrap_firstvalue(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "firstvalue", _Dee_HashSelectC(0xaade1c33, 0x8dd327b182158e40));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "firstvalue", _Dee_HashSelectC(0xaade1c33, 0x8dd327b182158e40)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "firstvalue", _Dee_HashSelectC(0xaade1c33, 0x8dd327b182158e40)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "firstvalue", _Dee_HashSelectC(0xaade1c33, 0x8dd327b182158e40), value);
		}
	};
	WUNUSED _Wrap_firstvalue (firstvalue)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_lastvalue
		: public deemon::detail::ConstGetRefProxy<_Wrap_lastvalue, _AbstractTuple<Key, Value> >
		, public deemon::detail::ConstSetRefProxy<_Wrap_lastvalue, _AbstractTuple<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_lastvalue, _AbstractTuple<Key, Value> >::operator =;
		_Wrap_lastvalue(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "lastvalue", _Dee_HashSelectC(0xf5e15f5, 0x2e2e12173695c5e7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "lastvalue", _Dee_HashSelectC(0xf5e15f5, 0x2e2e12173695c5e7)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "lastvalue", _Dee_HashSelectC(0xf5e15f5, 0x2e2e12173695c5e7)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "lastvalue", _Dee_HashSelectC(0xf5e15f5, 0x2e2e12173695c5e7), value);
		}
	};
	WUNUSED _Wrap_lastvalue (lastvalue)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_cached
		: public deemon::detail::ConstGetRefProxy<_Wrap_cached, Dict<Key, Value> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_cached(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "cached", _Dee_HashSelectC(0x915e175e, 0xddfd408a14eae4b4));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "cached", _Dee_HashSelectC(0x915e175e, 0xddfd408a14eae4b4)));
		}
	};
	WUNUSED _Wrap_cached (cached)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap___sizeof__ (__sizeof__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___hidxio__
		: public deemon::detail::ConstGetRefProxy<_Wrap___hidxio__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___hidxio__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__hidxio__", _Dee_HashSelectC(0xd8034a31, 0xf03b231c814923af));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__hidxio__", _Dee_HashSelectC(0xd8034a31, 0xf03b231c814923af)));
		}
	};
	WUNUSED _Wrap___hidxio__ (__hidxio__)() DEE_CXX_NOTHROW {
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
	WUNUSED _Wrap_max_load_factor (max_load_factor)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_DICT_H */
