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
#ifndef GUARD_DEEMON_CXX_LIST_H
#define GUARD_DEEMON_CXX_LIST_H 1

#include "../api.h"
#include "api.h"

#include "../format.h" /* Dee_PCKdSIZ, Dee_PCKuSIZ */
#include "../list.h"   /* DeeList_* */
#include "../object.h"
#include "../types.h"  /* DREF, DeeObject, Dee_ssize_t, _Dee_HashSelectC */
#include "object.h"
#include "sequence.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */

/* Not *that* "remove" */
/*!fixincludes fake_include "../system-features.h" // remove */

DEE_CXX_BEGIN

class Type;
class bool_;
class int_;

template<class T = Object>
class List
	: public Sequence<T>
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeList_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeList_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeList_CheckExact(ob);
	}

public:
	static WUNUSED Ref<List<T> > of() {
		return inherit(DeeList_New());
	}
	static WUNUSED Ref<List<T> > of(size_t n_prealloc) {
		return inherit(DeeList_NewWithHint(n_prealloc));
	}
	static WUNUSED Ref<List<T> > of_vector(size_t objc, DeeObject *const *objv) {
		return inherit(DeeList_NewVector(objc, objv));
	}
	static WUNUSED NONNULL_CXX((1)) Ref<List<T> > ofseq(DeeObject *seq) {
		return inherit(DeeList_FromSequence(seq));
	}

public:
	size_t cerase(size_t index, size_t count = 1) {
		return throw_if_minusone(DeeList_Erase(this, index, count));
	}
	Ref<T> cpop(Dee_ssize_t index = -1) {
		return inherit(DeeList_Pop(this, index));
	}
	bool cclear() DEE_CXX_NOTHROW {
		return DeeList_Clear(this);
	}
	void csort(size_t start = 0, size_t end = (size_t)-1, DeeObject *key = NULL) {
		throw_if_nonzero(DeeList_Sort(this, start, end, key));
	}
	void creverse(size_t start = 0, size_t end = (size_t)-1) DEE_CXX_NOTHROW {
		DeeList_Reverse(this, start, end);
	}
	void cappend(DeeObject *item) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_Append(this, item));
	}
	void cextend(DeeObject *seq) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_AppendSequence(this, seq));
	}
	void cappend(size_t objc, DeeObject *const *objv) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_AppendVector(this, objc, objv));
	}
	void cinsert(size_t index, DeeObject *item) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_Insert(this, index, item));
	}
	void cinsertall(size_t index, DeeObject *seq) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_InsertSequence(this, index, seq));
	}
	void cinsert(size_t index, size_t objc, DeeObject *const *objv) DEE_CXX_NOTHROW {
		throw_if_nonzero(DeeList_InsertVector(this, index, objc, objv));
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(List from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	NONNULL_CXX((1)) void (append)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "append", _Dee_HashSelectC(0x5f19594f, 0x8c2b7c1aba65d5ee), 1, args)));
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
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKdSIZ, total, more));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (reserve)(DeeObject *total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), "o" Dee_PCKuSIZ, total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ, total));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(Dee_ssize_t total, DeeObject *more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ "o", total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKdSIZ, total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(Dee_ssize_t total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKdSIZ Dee_PCKuSIZ, total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ, total));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (reserve)(size_t total, DeeObject *more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ "o", total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, Dee_ssize_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKdSIZ, total, more));
	}
	WUNUSED Ref<deemon::bool_> (reserve)(size_t total, size_t more) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), Dee_PCKuSIZ Dee_PCKuSIZ, total, more));
	}
	WUNUSED Ref<deemon::bool_> (shrink)() {
		return inherit(DeeObject_CallAttrStringHash(this, "shrink", _Dee_HashSelectC(0xd8afa32b, 0xf4785bc46214dc5), 0, NULL));
	}
	NONNULL_CXX((1)) void (extend)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "extend", _Dee_HashSelectC(0x960b75e7, 0xba076858e3adb055), 1, args)));
	}
	NONNULL_CXX((1)) void (resize)(DeeObject *size) {
		DeeObject *args[1];
		args[0] = size;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (resize)(DeeObject *size, DeeObject *filler) {
		DeeObject *args[2];
		args[0] = size;
		args[1] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 2, args)));
	}
	void (resize)(Dee_ssize_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), Dee_PCKdSIZ, size)));
	}
	NONNULL_CXX((2)) void (resize)(Dee_ssize_t size, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), Dee_PCKdSIZ "o", size, filler)));
	}
	void (resize)(size_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), Dee_PCKuSIZ, size)));
	}
	NONNULL_CXX((2)) void (resize)(size_t size, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), Dee_PCKuSIZ "o", size, filler)));
	}
	NONNULL_CXX((1, 2)) void (insert)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), 2, args)));
	}
	NONNULL_CXX((2)) void (insert)(Dee_ssize_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), Dee_PCKdSIZ "o", index, item)));
	}
	NONNULL_CXX((2)) void (insert)(size_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), Dee_PCKuSIZ "o", index, item)));
	}
	NONNULL_CXX((1, 2)) void (insertall)(DeeObject *index, DeeObject *items) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), 2, args)));
	}
	NONNULL_CXX((2)) void (insertall)(Dee_ssize_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), Dee_PCKdSIZ "o", index, items)));
	}
	NONNULL_CXX((2)) void (insertall)(size_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), Dee_PCKuSIZ "o", index, items)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (erase)(DeeObject *index, DeeObject *count) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = count;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), 2, args)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" Dee_PCKdSIZ, index, count)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" Dee_PCKuSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKdSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(Dee_ssize_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKdSIZ "o", index, count)));
	}
	void (erase)(Dee_ssize_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKdSIZ Dee_PCKdSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKdSIZ Dee_PCKuSIZ, index, count)));
	}
	void (erase)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKuSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(size_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKuSIZ "o", index, count)));
	}
	void (erase)(size_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKuSIZ Dee_PCKdSIZ, index, count)));
	}
	void (erase)(size_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), Dee_PCKuSIZ Dee_PCKuSIZ, index, count)));
	}
	WUNUSED Ref<T> (pop)() {
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (pop)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 1, args));
	}
	WUNUSED Ref<T> (pop)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), Dee_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (pop)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), Dee_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xchitem)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(Dee_ssize_t index, DeeObject *item) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57), Dee_PCKdSIZ "o", index, item));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(size_t index, DeeObject *item) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57), Dee_PCKuSIZ "o", index, item));
	}
	void (clear)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "clear", _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c), 0, NULL)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *max) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *max, DeeObject *key) {
		DeeObject *args[5];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		args[4] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 5, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "o" Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "o" Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "o" Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ "o" Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "o" Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "o" Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "o" Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ "o" Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should) {
		DeeObject *args[1];
		args[0] = should;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start) {
		DeeObject *args[2];
		args[0] = should;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, DeeObject *max) {
		DeeObject *args[4];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ "o" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ "o" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKdSIZ Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ "o" Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ "o" Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" Dee_PCKuSIZ Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end, max));
	}
	void (fill)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 0, NULL)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (fill)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (fill)(DeeObject *start, DeeObject *end, DeeObject *filler) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 3, args)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" Dee_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" Dee_PCKdSIZ "o", start, end, filler)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" Dee_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" Dee_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(Dee_ssize_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ "oo", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ Dee_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ Dee_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ Dee_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKdSIZ Dee_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(size_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ "oo", start, end, filler)));
	}
	void (fill)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ Dee_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ Dee_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ Dee_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), Dee_PCKuSIZ Dee_PCKuSIZ "o", start, end, filler)));
	}
	void (reverse)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 0, NULL)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (reverse)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 2, args)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" Dee_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" Dee_PCKuSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKdSIZ "o", start, end)));
	}
	void (reverse)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKdSIZ Dee_PCKdSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKdSIZ Dee_PCKuSIZ, start, end)));
	}
	void (reverse)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKuSIZ "o", start, end)));
	}
	void (reverse)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKuSIZ Dee_PCKdSIZ, start, end)));
	}
	void (reverse)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), Dee_PCKuSIZ Dee_PCKuSIZ, start, end)));
	}
	WUNUSED Ref<deemon::bool_> (sort)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (sort)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (sort)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (sort)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (sort)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (sort)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (sort)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (sort)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (sort)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (sort)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (sort)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (sort)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (sort)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (sort)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKdSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (sort)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (sort)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (sort)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (sort)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (sort)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (sort)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), Dee_PCKuSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (sorted)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	NONNULL_CXX((1)) void (__seq_append__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__seq_append__", _Dee_HashSelectC(0x43ea1331, 0x383f299606f81ebe), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__seq_enumerate__)(DeeObject *cb, DeeObject *start) {
		DeeObject *args[2];
		args[0] = cb;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (__seq_enumerate__)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__seq_enumerate__)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should) {
		DeeObject *args[1];
		args[0] = should;
		return inherit(DeeObject_CallAttrStringHash(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (remove_if)(DeeObject *should, DeeObject *start) {
		DeeObject *args[2];
		args[0] = should;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (remove_if)(DeeObject *should, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (remove_if)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "oo" Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (remove_if)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "oo" Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKdSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKdSIZ Dee_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKuSIZ Dee_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" Dee_PCKuSIZ Dee_PCKuSIZ, should, start, end));
	}
	NONNULL_CXX((1, 2)) void (insert_list)(DeeObject *index, DeeObject *items) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3), 2, args)));
	}
	NONNULL_CXX((2)) void (insert_list)(Dee_ssize_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3), Dee_PCKdSIZ "o", index, items)));
	}
	NONNULL_CXX((2)) void (insert_list)(size_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3), Dee_PCKuSIZ "o", index, items)));
	}
	NONNULL_CXX((1, 2)) void (insert_iter)(DeeObject *index, DeeObject *iter) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = iter;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f), 2, args)));
	}
	NONNULL_CXX((2)) void (insert_iter)(Dee_ssize_t index, DeeObject *iter) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f), Dee_PCKdSIZ "o", index, iter)));
	}
	NONNULL_CXX((2)) void (insert_iter)(size_t index, DeeObject *iter) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f), Dee_PCKuSIZ "o", index, iter)));
	}
	NONNULL_CXX((1)) void (push_front)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "push_front", _Dee_HashSelectC(0xdbec111, 0x9f0f5cc09fc9ec79), 1, args)));
	}
	NONNULL_CXX((1)) void (push_back)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "push_back", _Dee_HashSelectC(0x9e3fe4f0, 0x4276b3cb4d87e17e), 1, args)));
	}
	NONNULL_CXX((1)) void (pop_front)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pop_front", _Dee_HashSelectC(0xbd2087a8, 0x3586ea08234c24c), 1, args)));
	}
	NONNULL_CXX((1)) void (pop_back)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pop_back", _Dee_HashSelectC(0x72cb762e, 0x379e1e780abd2dbc), 1, args)));
	}
	void (shrink_to_fit)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "shrink_to_fit", _Dee_HashSelectC(0xdc684938, 0x94741bfe350f497f), 0, NULL)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (sorted_insert)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (sorted_insert)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), 3, args)));
	}
	NONNULL_CXX((1, 2)) void (sorted_insert)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "oo" Dee_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1, 2)) void (sorted_insert)(DeeObject *item, DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "oo" Dee_PCKuSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKdSIZ, item, start)));
	}
	NONNULL_CXX((1, 3)) void (sorted_insert)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKdSIZ "o", item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKuSIZ, item, start)));
	}
	NONNULL_CXX((1, 3)) void (sorted_insert)(DeeObject *item, size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKuSIZ "o", item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (sorted_insert)(DeeObject *item, size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sorted_insert", _Dee_HashSelectC(0xba6a2807, 0x7fce77259191a683), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end)));
	}
	WUNUSED Ref<Set<T> > (tounique)() {
		return inherit(DeeObject_CallAttrStringHash(this, "tounique", _Dee_HashSelectC(0x4d4b2262, 0xf4c5d8281e2fef9b), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (tounique)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "tounique", _Dee_HashSelectC(0x4d4b2262, 0xf4c5d8281e2fef9b), 1, args));
	}
	class _Wrap_allocated
		: public deemon::detail::ConstGetRefProxy<_Wrap_allocated, deemon::int_>
		, public deemon::detail::ConstSetRefProxy<_Wrap_allocated, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_allocated, deemon::int_>::operator =;
		_Wrap_allocated(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "allocated", _Dee_HashSelectC(0xf026a995, 0x5b116996dc9207df));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "allocated", _Dee_HashSelectC(0xf026a995, 0x5b116996dc9207df)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "allocated", _Dee_HashSelectC(0xf026a995, 0x5b116996dc9207df)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "allocated", _Dee_HashSelectC(0xf026a995, 0x5b116996dc9207df), value);
		}
	};
	WUNUSED _Wrap_allocated (allocated)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_first, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_first, T>::operator =;
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
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_last, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_last, T>::operator =;
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
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Sequence<T> > {
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
	class _Wrap_cached
		: public deemon::detail::ConstGetRefProxy<_Wrap_cached, List<T> > {
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
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_LIST_H */
